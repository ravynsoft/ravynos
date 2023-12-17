/* malloc.c - dynamic memory allocation for bash. */

/*  Copyright (C) 1985-2021 Free Software Foundation, Inc.

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

/*
 * @(#)nmalloc.c 1 (Caltech) 2/21/82
 *
 *	U of M Modified: 20 Jun 1983 ACT: strange hacks for Emacs
 *
 *	Nov 1983, Mike@BRL, Added support for 4.1C/4.2 BSD.
 *
 * [VERY] old explanation:
 *
 * This is a very fast storage allocator.  It allocates blocks of a small 
 * number of different sizes, and keeps free lists of each size.  Blocks
 * that don't exactly fit are passed up to the next larger size.  In this 
 * implementation, the available sizes are (2^n)-4 (or -16) bytes long.
 * This is designed for use in a program that uses vast quantities of
 * memory, but bombs when it runs out.  To make it a little better, it
 * warns the user when he starts to get near the end.
 *
 * June 84, ACT: modified rcheck code to check the range given to malloc,
 * rather than the range determined by the 2-power used.
 *
 * Jan 85, RMS: calls malloc_warning to issue warning on nearly full.
 * No longer Emacs-specific; can serve as all-purpose malloc for GNU.
 * You should call malloc_init to reinitialize after loading dumped Emacs.
 * Call malloc_stats to get info on memory stats if MALLOC_STATS turned on.
 * realloc knows how to return same block given, just changing its size,
 * if the power of 2 is correct.
 */

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+5).  The
 * smallest allocatable block is 32 bytes. The overhead information will
 * go in the first 16 bytes of the block, and the returned pointer will point
 * to the rest.
 */

/* Define MEMSCRAMBLE to have free() write 0xcf into memory as it's freed, to
   uncover callers that refer to freed memory, and to have malloc() write 0xdf
   into memory as it's allocated to avoid referring to previous contents. */

/* SCO 3.2v4 getcwd and possibly other libc routines fail with MEMSCRAMBLE;
   handled by configure. */

#if defined (HAVE_CONFIG_H)
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#if defined (SHELL)
#  include "bashtypes.h"
#  include "stdc.h"
#else
#  include <sys/types.h>
#endif

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

/* Determine which kind of system this is.  */
#include <signal.h>

#if defined (HAVE_STRING_H)
#  include <string.h>
#else
#  include <strings.h>
#endif
#include <errno.h>
#include <stdio.h>

#if !defined (botch)
#include <stdlib.h>
#endif

#if defined (HAVE_MMAP)
#include <sys/mman.h>
#endif

/* Define getpagesize () if the system does not.  */
#ifndef HAVE_GETPAGESIZE
#  include "getpagesize.h"
#endif

#include "imalloc.h"
#ifdef MALLOC_STATS
#  include "mstats.h"
#endif
#ifdef MALLOC_REGISTER
#  include "table.h"
#endif
#ifdef MALLOC_WATCH
#  include "watch.h"
#endif

#ifdef powerof2
#  undef powerof2
#endif
/* Could also use (((x) & -(x)) == (x)) */
#define powerof2(x)	((((x) - 1) & (x)) == 0)

/* System-specific omissions. */
#ifdef HPUX
#  define NO_VALLOC
#endif

#define MALLOC_PAGESIZE_MIN	4096
#define MALLOC_INCR_PAGES	8192

#define ISALLOC ((char) 0xf7)	/* magic byte that implies allocation */
#define ISFREE ((char) 0x54)	/* magic byte that implies free block */
				/* this is for error checking only */
#define ISMEMALIGN ((char) 0xd6)  /* Stored before the value returned by
				     memalign, with the rest of the word
				     being the distance to the true
				     beginning of the block.  */


/* We have a flag indicating whether memory is allocated, an index in
   nextf[], a size field, and a sentinel value to determine whether or
   not a caller wrote before the start of allocated memory; to realloc()
   memory we either copy mh_nbytes or just change mh_nbytes if there is
   enough room in the block for the new size.  Range checking is always
   done. */
union mhead {
  bits64_t mh_align[2];						/* 16 */
  struct {
    char mi_alloc; 		/* ISALLOC or ISFREE */		/* 1 */
    char mi_index;		/* index in nextf[] */		/* 1 */
    /* Remainder are valid only when block is allocated */
    u_bits16_t mi_magic2;	/* should be == MAGIC2 */	/* 2 */
    u_bits32_t mi_nbytes;	/* # of bytes allocated */	/* 4 */
    char mi_magic8[8];		/* MAGIC1 guard bytes */	/* 8 */
  } minfo;
};
#define mh_alloc	minfo.mi_alloc
#define mh_index	minfo.mi_index
#define mh_nbytes	minfo.mi_nbytes
#define mh_magic2	minfo.mi_magic2
#define mh_magic8	minfo.mi_magic8

#define MAGIC8_NUMBYTES	8
#define MALLOC_SIZE_T		u_bits32_t

#define MOVERHEAD	sizeof(union mhead)

#define MALIGN_MASK	15		/* one less than desired alignment */

/* Guard bytes we write at the end of the allocation, encoding the size. */
typedef union _malloc_guard {
  char s[4];
  u_bits32_t i;
} mguard_t;

/* Access free-list pointer of a block.
   It is stored at block + sizeof (char *).
   This is not a field in the minfo structure member of union mhead
   because we want sizeof (union mhead)
   to describe the overhead for when the block is in use,
   and we do not want the free-list pointer to count in that.  */
/* If we have mmap, this is not used for chunks larger than mmap_threshold,
   since we munmap immediately on free(). */

/* If SIZEOF_CHAR_P == 8, this goes into the mh_magic8 buffer at the end of
   the rest of the struct. This may need adjusting. */
#define CHAIN(a) \
  (*(union mhead **) (sizeof (char *) + (char *) (a)))

/* To implement range checking, we write magic values in at the beginning
   and end of each allocated block, and make sure they are undisturbed
   whenever a free or a realloc occurs. */

/* Written in the bytes before the block's real space (-SIZEOF_CHAR_P bytes) */
#define MAGIC1 0x55
#define MAGIC2 0x5555

#define MSLOP  4		/* 4 bytes extra for u_bits32_t end guard size */

/* How many bytes are actually allocated for a request of size N --
   rounded up to nearest multiple of 16 (alignment) after accounting for
   malloc overhead. */
#define ALLOCATED_BYTES(n) \
	(((n) + MOVERHEAD + MSLOP + MALIGN_MASK) & ~MALIGN_MASK)

#define ASSERT(p) \
  do \
    { \
      if (!(p)) xbotch((PTR_T)0, ERR_ASSERT_FAILED, CPP_STRING(p), file, line); \
    } \
  while (0)

/* Minimum and maximum bucket indices for block splitting (and to bound
   the search for a block to split). */
#define SPLIT_MIN	1		/* 64 */
#define SPLIT_MID	9		/* 16384 */
#define SPLIT_MAX	12		/* 131072 */

/* Minimum and maximum bucket indices for block coalescing. */
#define COMBINE_MIN	1		/* 64 */
#define COMBINE_MAX	(pagebucket - 1)	/* 2048 for 4096-byte pages */

#define LESSCORE_MIN	8		/* 8192 */
#define LESSCORE_FRC	11		/* 65536 */

/* Which bin do we prepopulate with the initial sbrk memory? */
#define PREPOP_BIN	1
#define PREPOP_SIZE	64

#define STARTBUCK	0

/* Should we use mmap for large allocations? */
#if defined (HAVE_MMAP)
#  if defined (MAP_ANON) && !defined (MAP_ANONYMOUS)
#    define MAP_ANONYMOUS MAP_ANON
#  endif
#endif

#if defined (HAVE_MMAP) && defined (MAP_ANONYMOUS)
#  define USE_MMAP	1
#endif

#if defined (USE_MMAP)
#  define MMAP_THRESHOLD	12	/* must be >= SPLIT_MAX, COMBINE_MAX */
#else
#  define MMAP_THRESHOLD	(8 * SIZEOF_LONG)
#endif

/* We don't try to decipher the differences between the Linux-style and
   BSD-style implementations of mremap here; we use the Linux one. */
#if USE_MMAP == 1 && defined (HAVE_MREMAP) && defined (MREMAP_MAYMOVE)
#  define USE_MREMAP 1
#endif

/* usable bins from STARTBUCK..NBUCKETS-1 */

#define NBUCKETS	28

/* Flags for the internal functions. */
#define MALLOC_WRAPPER	0x01	/* wrapper function */
#define MALLOC_INTERNAL	0x02	/* internal function calling another */
#define MALLOC_NOTRACE	0x04	/* don't trace this allocation or free */
#define MALLOC_NOREG	0x08	/* don't register this allocation or free */

/* Future use. */
#define ERR_DUPFREE		0x01
#define ERR_UNALLOC		0x02
#define ERR_UNDERFLOW		0x04	
#define ERR_ASSERT_FAILED	0x08

/* Evaluates to true if NB is appropriate for bucket NU.  NB is adjusted
   appropriately by the caller to account for malloc overhead.  This only
   checks that the recorded size is not too big for the bucket.  We
   can't check whether or not it's in between NU and NU-1 because we
   might have encountered a busy bucket when allocating and moved up to
   the next size. */
#define IN_BUCKET(nb, nu)	((nb) <= binsizes[(nu)])

/* Use this when we want to be sure that NB is in bucket NU. */
#define RIGHT_BUCKET(nb, nu) \
	(((nb) > binsizes[(nu)-1]) && ((nb) <= binsizes[(nu)]))

/* nextf[i] is free list of blocks of size 2**(i + 5)  */

static union mhead *nextf[NBUCKETS];

/* busy[i] is nonzero while allocation or free of block size i is in progress. */

static char busy[NBUCKETS];

static int pagesz;	/* system page size. */
static int pagebucket;	/* bucket for requests a page in size */
static int maxbuck;	/* highest bucket receiving allocation request. */

static char *memtop;	/* top of heap */

static const unsigned long binsizes[NBUCKETS] = {
	32UL, 64UL, 128UL, 256UL, 512UL, 1024UL, 2048UL, 4096UL,
	8192UL, 16384UL, 32768UL, 65536UL, 131072UL, 262144UL, 524288UL,
	1048576UL, 2097152UL, 4194304UL, 8388608UL, 16777216UL, 33554432UL,
	67108864UL, 134217728UL, 268435456UL, 536870912UL, 1073741824UL,
	2147483648UL, 4294967295UL
};

/* binsizes[x] == (1 << ((x) + 5)) */
#define binsize(x)	binsizes[(x)]

#define MAXALLOC_SIZE	binsizes[NBUCKETS-1]

#if !defined (errno)
extern int errno;
#endif

/* Declarations for internal functions */
static PTR_T internal_malloc PARAMS((size_t, const char *, int, int));
static PTR_T internal_realloc PARAMS((PTR_T, size_t, const char *, int, int));
static void internal_free PARAMS((PTR_T, const char *, int, int));
static PTR_T internal_memalign PARAMS((size_t, size_t, const char *, int, int));
#ifndef NO_CALLOC
static PTR_T internal_calloc PARAMS((size_t, size_t, const char *, int, int));
static void internal_cfree PARAMS((PTR_T, const char *, int, int));
#endif
#ifndef NO_VALLOC
static PTR_T internal_valloc PARAMS((size_t, const char *, int, int));
#endif
static PTR_T internal_remap PARAMS((PTR_T, size_t, int, int));

#if defined (botch)
extern void botch ();
#else
static void botch PARAMS((const char *, const char *, int));
#endif
static void xbotch PARAMS((PTR_T, int, const char *, const char *, int));

#if !HAVE_DECL_SBRK
extern char *sbrk ();
#endif /* !HAVE_DECL_SBRK */

#ifdef SHELL
extern int running_trap;
extern int signal_is_trapped PARAMS((int));
#endif

#ifdef MALLOC_STATS
struct _malstats _mstats;
#endif /* MALLOC_STATS */

/* Debugging variables available to applications. */
int malloc_flags = 0;	/* future use */
int malloc_trace = 0;	/* trace allocations and frees to stderr */
int malloc_register = 0;	/* future use */

/* Use a variable in case we want to dynamically adapt it in the future */
int malloc_mmap_threshold = MMAP_THRESHOLD;

#ifdef MALLOC_TRACE
char _malloc_trace_buckets[NBUCKETS];

/* These should really go into a header file. */
extern void mtrace_alloc PARAMS((const char *, PTR_T, size_t, const char *, int));
extern void mtrace_free PARAMS((PTR_T, int, const char *, int));
#endif

#if !defined (botch)
static void
botch (s, file, line)
     const char *s;
     const char *file;
     int line;
{
  fprintf (stderr, _("malloc: failed assertion: %s\n"), s);
  (void)fflush (stderr);
  abort ();
}
#endif

/* print the file and line number that caused the assertion failure and
   call botch() to do whatever the application wants with the information */
static void
xbotch (mem, e, s, file, line)
     PTR_T mem;
     int e;
     const char *s;
     const char *file;
     int line;
{
  fprintf (stderr, _("\r\nmalloc: %s:%d: assertion botched\r\n"),
			file ? file : _("unknown"), line);
#ifdef MALLOC_REGISTER
  if (mem != NULL && malloc_register)
    mregister_describe_mem (mem, stderr);
#endif
  (void)fflush (stderr);
  botch(s, file, line);
}

/* Coalesce two adjacent free blocks off the free list for size NU - 1,
   as long as we can find two adjacent free blocks.  nextf[NU -1] is
   assumed to not be busy; the caller (morecore()) checks for this. 
   BUSY[NU] must be set to 1. */
static void
bcoalesce (nu)
     register int nu;
{
  register union mhead *mp, *mp1, *mp2;
  register int nbuck;
  unsigned long siz;

  nbuck = nu - 1;
  if (nextf[nbuck] == 0 || busy[nbuck])
    return;

  busy[nbuck] = 1;
  siz = binsize (nbuck);

  mp2 = mp1 = nextf[nbuck];
  mp = CHAIN (mp1);
  while (mp && mp != (union mhead *)((char *)mp1 + siz))
    {
      mp2 = mp1;
      mp1 = mp;
      mp = CHAIN (mp);
    }

  if (mp == 0)
    {
      busy[nbuck] = 0;
      return;
    }

  /* OK, now we have mp1 pointing to the block we want to add to nextf[NU].
     CHAIN(mp2) must equal mp1.  Check that mp1 and mp are adjacent. */
  if (mp2 != mp1 && CHAIN(mp2) != mp1)
    {
      busy[nbuck] = 0;
      xbotch ((PTR_T)0, 0, "bcoalesce: CHAIN(mp2) != mp1", (char *)NULL, 0);
    }

#ifdef MALLOC_DEBUG
  if (CHAIN (mp1) != (union mhead *)((char *)mp1 + siz))
    {
      busy[nbuck] = 0;
      return;	/* not adjacent */
    }
#endif

  /* Since they are adjacent, remove them from the free list */
  if (mp1 == nextf[nbuck])
    nextf[nbuck] = CHAIN (mp);
  else
    CHAIN (mp2) = CHAIN (mp);
  busy[nbuck] = 0;

#ifdef MALLOC_STATS
  _mstats.tbcoalesce++;
  _mstats.ncoalesce[nbuck]++;
#endif

  /* And add the combined two blocks to nextf[NU]. */
  mp1->mh_alloc = ISFREE;
  mp1->mh_index = nu;
  CHAIN (mp1) = nextf[nu];
  nextf[nu] = mp1;
}

/* Split a block at index > NU (but less than SPLIT_MAX) into a set of
   blocks of the correct size, and attach them to nextf[NU].  nextf[NU]
   is assumed to be empty.  Must be called with signals blocked (e.g.,
   by morecore()).  BUSY[NU] must be set to 1. */
static void
bsplit (nu)
     register int nu;
{
  register union mhead *mp;
  int nbuck, nblks, split_max;
  unsigned long siz;

  split_max = (maxbuck > SPLIT_MAX) ? maxbuck : SPLIT_MAX;

  if (nu >= SPLIT_MID)
    {
      for (nbuck = split_max; nbuck > nu; nbuck--)
	{
	  if (busy[nbuck] || nextf[nbuck] == 0)
	    continue;
	  break;
	}
    }
  else
    {
      for (nbuck = nu + 1; nbuck <= split_max; nbuck++)
	{
	  if (busy[nbuck] || nextf[nbuck] == 0)
	    continue;
	  break;
	}
    }

  if (nbuck > split_max || nbuck <= nu)
    return;

  /* XXX might want to split only if nextf[nbuck] has >= 2 blocks free
     and nbuck is below some threshold. */

  /* Remove the block from the chain of larger blocks. */
  busy[nbuck] = 1;
  mp = nextf[nbuck];
  nextf[nbuck] = CHAIN (mp);
  busy[nbuck] = 0;

#ifdef MALLOC_STATS
  _mstats.tbsplit++;
  _mstats.nsplit[nbuck]++;
#endif

  /* Figure out how many blocks we'll get. */
  siz = binsize (nu);
  nblks = binsize (nbuck) / siz;

  /* Split the block and put it on the requested chain. */
  nextf[nu] = mp;
  while (1)
    {
      mp->mh_alloc = ISFREE;
      mp->mh_index = nu;
      if (--nblks <= 0) break;
      CHAIN (mp) = (union mhead *)((char *)mp + siz);
      mp = (union mhead *)((char *)mp + siz);
    }
  CHAIN (mp) = 0;
}

/* Take the memory block MP and add it to a chain < NU.  NU is the right bucket,
   but is busy.  This avoids memory orphaning. */
static void
xsplit (mp, nu)
     union mhead *mp;
     int nu;
{
  union mhead *nh;
  int nbuck, nblks, split_max;
  unsigned long siz;

  nbuck = nu - 1;
  while (nbuck >= SPLIT_MIN && busy[nbuck])
    nbuck--;
  if (nbuck < SPLIT_MIN)
    return;

#ifdef MALLOC_STATS
  _mstats.tbsplit++;
  _mstats.nsplit[nu]++;
#endif

  /* Figure out how many blocks we'll get. */
  siz = binsize (nu);			/* original block size */
  nblks = siz / binsize (nbuck);	/* should be 2 most of the time */

  /* And add it to nextf[nbuck] */
  siz = binsize (nbuck);		/* XXX - resetting here */
  nh = mp;
  while (1)
    {
      mp->mh_alloc = ISFREE;
      mp->mh_index = nbuck;
      if (--nblks <= 0) break;
      CHAIN (mp) = (union mhead *)((char *)mp + siz);
      mp = (union mhead *)((char *)mp + siz);
    }
  busy[nbuck] = 1;
  CHAIN (mp) = nextf[nbuck];
  nextf[nbuck] = nh;
  busy[nbuck] = 0;
}

void
_malloc_block_signals (setp, osetp)
     sigset_t *setp, *osetp;
{
#ifdef HAVE_POSIX_SIGNALS
  sigfillset (setp);
  sigemptyset (osetp);
  sigprocmask (SIG_BLOCK, setp, osetp);
#else
#  if defined (HAVE_BSD_SIGNALS)
  *osetp = sigsetmask (-1);
#  endif
#endif
}

void
_malloc_unblock_signals (setp, osetp)
     sigset_t *setp, *osetp;
{
#ifdef HAVE_POSIX_SIGNALS
  sigprocmask (SIG_SETMASK, osetp, (sigset_t *)NULL);
#else
#  if defined (HAVE_BSD_SIGNALS)
  sigsetmask (*osetp);
#  endif
#endif
}

#if defined (USE_LESSCORE)
/* Return some memory to the system by reducing the break.  This is only
   called with NU > pagebucket, so we're always assured of giving back
   more than one page of memory. */  
static void
lesscore (nu)			/* give system back some memory */
     register int nu;		/* size index we're discarding  */
{
  long siz;

  siz = binsize (nu);
  /* Should check for errors here, I guess. */
  sbrk (-siz);
  memtop -= siz;

#ifdef MALLOC_STATS
  _mstats.nsbrk++;
  _mstats.tsbrk -= siz;
  _mstats.nlesscore[nu]++;
#endif
}
#endif /* USE_LESSCORE */

/* Ask system for more memory; add to NEXTF[NU].  BUSY[NU] must be set to 1. */  
static void
morecore (nu)
     register int nu;		/* size index to get more of  */
{
  register union mhead *mp;
  register int nblks;
  register long siz;
  long sbrk_amt;		/* amount to get via sbrk() */
  sigset_t set, oset;
  int blocked_sigs;

  /* Block all signals in case we are executed from a signal handler. */
  blocked_sigs = 0;
#ifdef SHELL
#  if defined (SIGCHLD)
  if (running_trap || signal_is_trapped (SIGINT) || signal_is_trapped (SIGCHLD))
#  else
  if (running_trap || signal_is_trapped (SIGINT))
#  endif
#endif
    {
      _malloc_block_signals (&set, &oset);
      blocked_sigs = 1;
    }

  siz = binsize (nu);	/* size of desired block for nextf[nu] */

  if (siz < 0)
    goto morecore_done;		/* oops */

#ifdef MALLOC_STATS
  _mstats.nmorecore[nu]++;
#endif

  /* Try to split a larger block here, if we're within the range of sizes
     to split. */
  if (nu >= SPLIT_MIN && nu <= malloc_mmap_threshold)
    {
      bsplit (nu);
      if (nextf[nu] != 0)
	goto morecore_done;
    }

  /* Try to coalesce two adjacent blocks from the free list on nextf[nu - 1],
     if we can, and we're within the range of the block coalescing limits. */
  if (nu >= COMBINE_MIN && nu < COMBINE_MAX && nu <= malloc_mmap_threshold && busy[nu - 1] == 0 && nextf[nu - 1])
    {
      bcoalesce (nu);
      if (nextf[nu] != 0)
	goto morecore_done;
    }

  /* Take at least a page, and figure out how many blocks of the requested
     size we're getting. */
  if (siz <= pagesz)
    {
      sbrk_amt = pagesz;
      nblks = sbrk_amt / siz;
    }
  else
    {
      /* We always want to request an integral multiple of the page size
	 from the kernel, so let's compute whether or not `siz' is such
	 an amount.  If it is, we can just request it.  If not, we want
	 the smallest integral multiple of pagesize that is larger than
	 `siz' and will satisfy the request. */
      sbrk_amt = siz & (pagesz - 1);
      if (sbrk_amt == 0)
	sbrk_amt = siz;
      else
	sbrk_amt = siz + pagesz - sbrk_amt;
      nblks = 1;
    }

#if defined (USE_MMAP)
  if (nu > malloc_mmap_threshold)
    {
      mp = (union mhead *)mmap (0, sbrk_amt, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
      if ((void *)mp == MAP_FAILED)
	goto morecore_done;
      nextf[nu] = mp;
      mp->mh_alloc = ISFREE;
      mp->mh_index = nu;
      CHAIN (mp) = 0;
#ifdef MALLOC_STATS
      _mstats.nmmap++;
      _mstats.tmmap += sbrk_amt;
#endif
      goto morecore_done;
    }
#endif
	

#ifdef MALLOC_STATS
  _mstats.nsbrk++;
  _mstats.tsbrk += sbrk_amt;
#endif

  mp = (union mhead *) sbrk (sbrk_amt);

  /* Totally out of memory. */
  if ((long)mp == -1)
    goto morecore_done;

  memtop += sbrk_amt;

  /* shouldn't happen, but just in case -- require 8- or 16-byte alignment */
  if ((long)mp & MALIGN_MASK)
    {
      mp = (union mhead *) (((long)mp + MALIGN_MASK) & ~MALIGN_MASK);
      nblks--;
    }

  /* save new header and link the nblks blocks together */
  nextf[nu] = mp;
  while (1)
    {
      mp->mh_alloc = ISFREE;
      mp->mh_index = nu;
      if (--nblks <= 0) break;
      CHAIN (mp) = (union mhead *)((char *)mp + siz);
      mp = (union mhead *)((char *)mp + siz);
    }
  CHAIN (mp) = 0;

morecore_done:
  if (blocked_sigs)
    _malloc_unblock_signals (&set, &oset);
}

static void
malloc_debug_dummy ()
{
  write (1, "malloc_debug_dummy\n", 19);
}

static int
pagealign ()
{
  register int nunits;
  register union mhead *mp;
  long sbrk_needed;
  char *curbrk;

  pagesz = getpagesize ();
  if (pagesz < MALLOC_PAGESIZE_MIN)
    pagesz = MALLOC_PAGESIZE_MIN;

  /* OK, how much do we need to allocate to make things page-aligned?
     Some of this partial page will be wasted space, but we'll use as
     much as we can.  Once we figure out how much to advance the break
     pointer, go ahead and do it. */
  memtop = curbrk = sbrk (0);
  sbrk_needed = pagesz - ((long)curbrk & (pagesz - 1));	/* sbrk(0) % pagesz */
  if (sbrk_needed < 0)
    sbrk_needed += pagesz;

  /* Now allocate the wasted space. */
  if (sbrk_needed)
    {
#ifdef MALLOC_STATS
      _mstats.nsbrk++;
      _mstats.tsbrk += sbrk_needed;
#endif
      curbrk = sbrk (sbrk_needed);
      if ((long)curbrk == -1)
	return -1;
      memtop += sbrk_needed;

      /* Take the memory which would otherwise be wasted and populate the most
	 popular bin (3 == 64 bytes) with it.  Add whatever we need to curbrk
	 to make things 64-byte aligned, compute how many 64-byte chunks we're
	 going to get, and set up the bin. */
      curbrk += sbrk_needed & (PREPOP_SIZE - 1);
      sbrk_needed -= sbrk_needed & (PREPOP_SIZE - 1);
      nunits = sbrk_needed / PREPOP_SIZE;

      if (nunits > 0)
	{
	  mp = (union mhead *)curbrk;

	  nextf[PREPOP_BIN] = mp;
	  while (1)
	    {
	      mp->mh_alloc = ISFREE;
	      mp->mh_index = PREPOP_BIN;
	      if (--nunits <= 0) break;
	      CHAIN(mp) = (union mhead *)((char *)mp + PREPOP_SIZE);
	      mp = (union mhead *)((char *)mp + PREPOP_SIZE);
	    }
	  CHAIN(mp) = 0;
	}
    }

  /* compute which bin corresponds to the page size. */
  for (nunits = 7; nunits < NBUCKETS; nunits++)
    if (pagesz <= binsize(nunits))
      break;
  pagebucket = nunits;

  return 0;
}
    
static PTR_T
internal_malloc (n, file, line, flags)		/* get a block */
     size_t n;
     const char *file;
     int line, flags;
{
  register union mhead *p;
  register int nunits;
  register char *m, *z;
  MALLOC_SIZE_T nbytes;
  mguard_t mg;

  /* Get the system page size and align break pointer so future sbrks will
     be page-aligned.  The page size must be at least 1K -- anything
     smaller is increased. */
  if (pagesz == 0)
    if (pagealign () < 0)
      return ((PTR_T)NULL);
 
  /* Figure out how many bytes are required, rounding up to the nearest
     multiple of 8, then figure out which nextf[] area to use.  Try to
     be smart about where to start searching -- if the number of bytes
     needed is greater than the page size, we can start at pagebucket. */
#if SIZEOF_SIZE_T == 8
  if (ALLOCATED_BYTES(n) > MAXALLOC_SIZE)
    return ((PTR_T) NULL);
#endif
  nbytes = ALLOCATED_BYTES(n);
  nunits = (nbytes <= (pagesz >> 1)) ? STARTBUCK : pagebucket;
  for ( ; nunits < NBUCKETS; nunits++)
    if (nbytes <= binsize(nunits))
      break;

  /* Silently reject too-large requests. XXX - can increase this if HAVE_MMAP */
  if (nunits >= NBUCKETS)
    return ((PTR_T) NULL);

  /* In case this is reentrant use of malloc from signal handler,
     pick a block size that no other malloc level is currently
     trying to allocate.  That's the easiest harmless way not to
     interfere with the other level of execution.  */
#ifdef MALLOC_STATS
  if (busy[nunits]) _mstats.nrecurse++;
#endif
  while (busy[nunits]) nunits++;
  busy[nunits] = 1;

  if (nunits > maxbuck)
    maxbuck = nunits;

  /* If there are no blocks of the appropriate size, go get some */
  if (nextf[nunits] == 0)
    morecore (nunits);

  /* Get one block off the list, and set the new list head */
  if ((p = nextf[nunits]) == NULL)
    {
      busy[nunits] = 0;
      return NULL;
    }
  nextf[nunits] = CHAIN (p);
  busy[nunits] = 0;

  /* Check for free block clobbered */
  /* If not for this check, we would gobble a clobbered free chain ptr
     and bomb out on the NEXT allocate of this size block */
  if (p->mh_alloc != ISFREE || p->mh_index != nunits)
    xbotch ((PTR_T)(p+1), 0, _("malloc: block on free list clobbered"), file, line);

  /* Fill in the info, and set up the magic numbers for range checking. */
  p->mh_alloc = ISALLOC;
  p->mh_magic2 = MAGIC2;
  p->mh_nbytes = n;

  /* Begin guard */
  MALLOC_MEMSET ((char *)p->mh_magic8, MAGIC1, MAGIC8_NUMBYTES);

  /* End guard */
  mg.i = n;
  z = mg.s;
  m = (char *) (p + 1) + n;
  *m++ = *z++, *m++ = *z++, *m++ = *z++, *m++ = *z++;

#ifdef MEMSCRAMBLE
  if (n)
    MALLOC_MEMSET ((char *)(p + 1), 0xdf, n);	/* scramble previous contents */
#endif
#ifdef MALLOC_STATS
  _mstats.nmalloc[nunits]++;
  _mstats.tmalloc[nunits]++;
  _mstats.nmal++;
  _mstats.bytesreq += n;
#endif /* MALLOC_STATS */

#ifdef MALLOC_TRACE
  if (malloc_trace && (flags & MALLOC_NOTRACE) == 0)
    mtrace_alloc ("malloc", p + 1, n, file, line);
  else if (_malloc_trace_buckets[nunits])
    mtrace_alloc ("malloc", p + 1, n, file, line);
#endif

#ifdef MALLOC_REGISTER
  if (malloc_register && (flags & MALLOC_NOREG) == 0)
    mregister_alloc ("malloc", p + 1, n, file, line);
#endif

#ifdef MALLOC_WATCH
  if (_malloc_nwatch > 0)
    _malloc_ckwatch (p + 1, file, line, W_ALLOC, n);
#endif

#if defined (MALLOC_DEBUG)
  z = (char *) (p + 1);
  /* Check alignment of returned pointer */
  if ((unsigned long)z & MALIGN_MASK)
    fprintf (stderr, "malloc: %s:%d: warning: request for %d bytes not aligned on %d byte boundary\r\n",
	file ? file : _("unknown"), line, p->mh_nbytes, MALIGN_MASK+1);
#endif

  return (PTR_T) (p + 1);
}

static void
internal_free (mem, file, line, flags)
     PTR_T mem;
     const char *file;
     int line, flags;
{
  register union mhead *p;
  register char *ap, *z;
  register int nunits;
  register MALLOC_SIZE_T nbytes;
  MALLOC_SIZE_T ubytes;		/* caller-requested size */
  mguard_t mg;

  if ((ap = (char *)mem) == 0)
    return;

  p = (union mhead *) ap - 1;

  if (p->mh_alloc == ISMEMALIGN)
    {
      ap -= p->mh_nbytes;
      p = (union mhead *) ap - 1;
    }

#if defined (MALLOC_TRACE) || defined (MALLOC_REGISTER) || defined (MALLOC_WATCH)
  if (malloc_trace || malloc_register || _malloc_nwatch > 0)
    ubytes = p->mh_nbytes;
#endif

  if (p->mh_alloc != ISALLOC)
    {
      if (p->mh_alloc == ISFREE)
	xbotch (mem, ERR_DUPFREE,
		_("free: called with already freed block argument"), file, line);
      else
	xbotch (mem, ERR_UNALLOC,
		_("free: called with unallocated block argument"), file, line);
    }

  ASSERT (p->mh_magic2 == MAGIC2);

  nunits = p->mh_index;
  nbytes = ALLOCATED_BYTES(p->mh_nbytes);
  /* The MAGIC8_NUMBYTES bytes before the memory handed to the user are now
     used for a simple check to catch things like p[-1] = 'x'.
     We sanity-check the value of mh_nbytes against the size of the blocks
     in the appropriate bucket before we use it.  This can still cause problems
     and obscure errors if mh_nbytes is wrong but still within range; the
     checks against the size recorded at the end of the chunk will probably
     fail then. Using MALLOC_REGISTER will help here, since it saves the
     original number of bytes requested. */

  if (IN_BUCKET(nbytes, nunits) == 0)
    xbotch (mem, ERR_UNDERFLOW,
	    _("free: underflow detected; mh_nbytes out of range"), file, line);
  {
    int i;
    for (i = 0, z = p->mh_magic8; i < MAGIC8_NUMBYTES; i++)
      if (*z++ != MAGIC1)
	xbotch (mem, ERR_UNDERFLOW,
		_("free: underflow detected; magic8 corrupted"), file, line);
  }

  ap += p->mh_nbytes;
  z = mg.s;
  *z++ = *ap++, *z++ = *ap++, *z++ = *ap++, *z++ = *ap++;  
  if (mg.i != p->mh_nbytes)
    xbotch (mem, ERR_ASSERT_FAILED, _("free: start and end chunk sizes differ"), file, line);

#if defined (USE_MMAP)
  if (nunits > malloc_mmap_threshold)
    {
      munmap (p, binsize (nunits));
#if defined (MALLOC_STATS)
      _mstats.nlesscore[nunits]++;
#endif
      goto free_return;
    }
#endif

#if defined (USE_LESSCORE)
  /* We take care of the mmap case and munmap above */
  if (nunits >= LESSCORE_MIN && ((char *)p + binsize(nunits) == memtop))
    {
      /* If above LESSCORE_FRC, give back unconditionally.  This should be set
	 high enough to be infrequently encountered.  If between LESSCORE_MIN
	 and LESSCORE_FRC, call lesscore if the bucket is marked as busy or if
	 there's already a block on the free list. */
      if ((nunits >= LESSCORE_FRC) || busy[nunits] || nextf[nunits] != 0)
	{
	  lesscore (nunits);
	  /* keeps the tracing and registering code in one place */
	  goto free_return;
	}
    }
#endif /* USE_LESSCORE */

#ifdef MEMSCRAMBLE
  if (p->mh_nbytes)
    MALLOC_MEMSET (mem, 0xcf, p->mh_nbytes);
#endif

  ASSERT (nunits < NBUCKETS);

  if (busy[nunits] == 1)
    {
      xsplit (p, nunits);	/* split block and add to different chain */
      goto free_return;
    }

  p->mh_alloc = ISFREE;
  /* Protect against signal handlers calling malloc.  */
  busy[nunits] = 1;
  /* Put this block on the free list.  */
  CHAIN (p) = nextf[nunits];
  nextf[nunits] = p;
  busy[nunits] = 0;

free_return:
  ;		/* Empty statement in case this is the end of the function */

#ifdef MALLOC_STATS
  _mstats.nmalloc[nunits]--;
  _mstats.nfre++;
#endif /* MALLOC_STATS */

#ifdef MALLOC_TRACE
  if (malloc_trace && (flags & MALLOC_NOTRACE) == 0)
    mtrace_free (mem, ubytes, file, line);
  else if (_malloc_trace_buckets[nunits])
    mtrace_free (mem, ubytes, file, line);
#endif

#ifdef MALLOC_REGISTER
  if (malloc_register && (flags & MALLOC_NOREG) == 0)
    mregister_free (mem, ubytes, file, line);
#endif

#ifdef MALLOC_WATCH
  if (_malloc_nwatch > 0)
    _malloc_ckwatch (mem, file, line, W_FREE, ubytes);
#endif
}

#if USE_MREMAP == 1
/* Assume the caller (internal_realloc) has already performed the sanity and
   overflow tests. Basically we kill the old guard information, determine the
   new size, call mremap with the new size, and add the bookkeeping and guard
   information back in. */
static PTR_T
internal_remap (mem, n, nunits, flags)
     PTR_T mem;
     register size_t n;
     int nunits;
     int flags;
{
  register union mhead *p, *np;
  char *m, *z;
  mguard_t mg;
  MALLOC_SIZE_T nbytes;

  if (nunits >= NBUCKETS)	/* Uh oh */
    return ((PTR_T) NULL);

  p = (union mhead *)mem - 1;

  m = (char *)mem + p->mh_nbytes;
  z = mg.s;
  *m++ = 0;  *m++ = 0;  *m++ = 0;  *m++ = 0;	/* erase guard */

  nbytes = ALLOCATED_BYTES(n);

  busy[nunits] = 1;
  np = (union mhead *)mremap (p, binsize (p->mh_index), binsize (nunits), MREMAP_MAYMOVE);
  busy[nunits] = 0;
  if (np == MAP_FAILED)
    return (PTR_T)NULL;

  if (np != p)
    {
      np->mh_alloc = ISALLOC;
      np->mh_magic2 = MAGIC2;
      MALLOC_MEMSET ((char *)np->mh_magic8, MAGIC1, MAGIC8_NUMBYTES);
    }
  np->mh_index = nunits;
  np->mh_nbytes = n;

  mg.i = n;
  z = mg.s;
  m = (char *)(np + 1) + n;
  *m++ = *z++, *m++ = *z++, *m++ = *z++, *m++ = *z++;

  return ((PTR_T)(np + 1));
}
#endif

static PTR_T
internal_realloc (mem, n, file, line, flags)
     PTR_T mem;
     register size_t n;
     const char *file;
     int line, flags;
{
  register union mhead *p;
  register MALLOC_SIZE_T tocopy;
  register MALLOC_SIZE_T nbytes;
  register int newunits, nunits;
  register char *m, *z;
  mguard_t mg;

#ifdef MALLOC_STATS
  _mstats.nrealloc++;
#endif

  if (n == 0)
    {
      internal_free (mem, file, line, MALLOC_INTERNAL);
      return (NULL);
    }
  if ((p = (union mhead *) mem) == 0)
    return internal_malloc (n, file, line, MALLOC_INTERNAL);

  p--;
  nunits = p->mh_index;
  ASSERT (nunits < NBUCKETS);

  if (p->mh_alloc != ISALLOC)
    xbotch (mem, ERR_UNALLOC,
	    _("realloc: called with unallocated block argument"), file, line);

  ASSERT (p->mh_magic2 == MAGIC2);
  nbytes = ALLOCATED_BYTES(p->mh_nbytes);
  /* Since the sizeof(u_bits32_t) bytes before the memory handed to the user
     are now used for the number of bytes allocated, a simple check of
     mh_magic2 is no longer sufficient to catch things like p[-1] = 'x'.
     We sanity-check the value of mh_nbytes against the size of the blocks
     in the appropriate bucket before we use it.  This can still cause problems
     and obscure errors if mh_nbytes is wrong but still within range; the
     checks against the size recorded at the end of the chunk will probably
     fail then.  Using MALLOC_REGISTER will help here, since it saves the
     original number of bytes requested. */
  if (IN_BUCKET(nbytes, nunits) == 0)
    xbotch (mem, ERR_UNDERFLOW,
	    _("realloc: underflow detected; mh_nbytes out of range"), file, line);
  {
    int i;
    for (i = 0, z = p->mh_magic8; i < MAGIC8_NUMBYTES; i++)
      if (*z++ != MAGIC1)
	xbotch (mem, ERR_UNDERFLOW,
		_("realloc: underflow detected; magic8 corrupted"), file, line);

  }

  m = (char *)mem + (tocopy = p->mh_nbytes);
  z = mg.s;
  *z++ = *m++, *z++ = *m++, *z++ = *m++, *z++ = *m++;
  if (mg.i != p->mh_nbytes)
    xbotch (mem, ERR_ASSERT_FAILED, _("realloc: start and end chunk sizes differ"), file, line);

#ifdef MALLOC_WATCH
  if (_malloc_nwatch > 0)
    _malloc_ckwatch (p + 1, file, line, W_REALLOC, n);
#endif
#ifdef MALLOC_STATS
  _mstats.bytesreq += (n < tocopy) ? 0 : n - tocopy;
#endif

  /* If we're reallocating to the same size as previously, return now */
  if (n == p->mh_nbytes)
    return mem;

#if SIZEOF_SIZE_T == 8
  if (ALLOCATED_BYTES(n) > MAXALLOC_SIZE)
    return ((PTR_T) NULL);
#endif
  /* See if desired size rounds to same power of 2 as actual size. */
  nbytes = ALLOCATED_BYTES(n);

  /* If ok, use the same block, just marking its size as changed.  */
  if (RIGHT_BUCKET(nbytes, nunits) || RIGHT_BUCKET(nbytes, nunits-1))
    {
      /* Compensate for increment above. */
      m -= 4;

      *m++ = 0;  *m++ = 0;  *m++ = 0;  *m++ = 0;
      m = (char *)mem + (p->mh_nbytes = n);

      mg.i = n;
      z = mg.s;
      *m++ = *z++, *m++ = *z++, *m++ = *z++, *m++ = *z++;      

      return mem;
    }

  if (n < tocopy)
    tocopy = n;

#ifdef MALLOC_STATS
  _mstats.nrcopy++;
#endif

#if USE_MREMAP == 1
  /* If we are using mmap and have mremap, we use it here. Make sure that
     the old size and new size are above the threshold where we use mmap */
  if (nbytes > p->mh_nbytes)
    newunits = nunits;
  else
    newunits = (nbytes <= (pagesz >> 1)) ? STARTBUCK : pagebucket;
  for ( ; newunits < NBUCKETS; newunits++)
    if (nbytes <= binsize(newunits))
     break;

  if (nunits > malloc_mmap_threshold && newunits > malloc_mmap_threshold)
    {
      m = internal_remap (mem, n, newunits, MALLOC_INTERNAL);
      if (m == 0)
        return 0;
    }
  else
#endif /* USE_MREMAP */
    {
  if ((m = internal_malloc (n, file, line, MALLOC_INTERNAL|MALLOC_NOTRACE|MALLOC_NOREG)) == 0)
    return 0;
  FASTCOPY (mem, m, tocopy);
  internal_free (mem, file, line, MALLOC_INTERNAL);
    }

#ifdef MALLOC_TRACE
  if (malloc_trace && (flags & MALLOC_NOTRACE) == 0)
    mtrace_alloc ("realloc", m, n, file, line);
  else if (_malloc_trace_buckets[nunits])
    mtrace_alloc ("realloc", m, n, file, line);
#endif

#ifdef MALLOC_REGISTER
  if (malloc_register && (flags & MALLOC_NOREG) == 0)
    mregister_alloc ("realloc", m, n, file, line);
#endif

#ifdef MALLOC_WATCH
  if (_malloc_nwatch > 0)
    _malloc_ckwatch (m, file, line, W_RESIZED, n);
#endif

  return m;
}

static PTR_T
internal_memalign (alignment, size, file, line, flags)
     size_t alignment;
     size_t size;
     const char *file;
     int line, flags;
{
  register char *ptr;
  register char *aligned;
  register union mhead *p;

  ptr = internal_malloc (size + alignment, file, line, MALLOC_INTERNAL);

  if (ptr == 0)
    return 0;
  /* If entire block has the desired alignment, just accept it.  */
  if (((long) ptr & (alignment - 1)) == 0)
    return ptr;
  /* Otherwise, get address of byte in the block that has that alignment.  */
  aligned = (char *) (((long) ptr + alignment - 1) & (~alignment + 1));

  /* Store a suitable indication of how to free the block,
     so that free can find the true beginning of it.  */
  p = (union mhead *) aligned - 1;
  p->mh_nbytes = aligned - ptr;
  p->mh_alloc = ISMEMALIGN;

  return aligned;
}

int
posix_memalign (memptr, alignment, size)
     void **memptr;
     size_t alignment, size;
{
  void *mem;

  /* Perform posix-mandated error checking here */
  if ((alignment % sizeof (void *) != 0) || alignment == 0)
    return EINVAL;
  else if (powerof2 (alignment) == 0)
    return EINVAL;

  mem = internal_memalign (alignment, size, (char *)0, 0, 0);
  if (mem != 0)
    {
      *memptr = mem;
      return 0;
    }
  return ENOMEM;
}

size_t
malloc_usable_size (mem)
     void *mem;
{
  register union mhead *p;
  register char *ap;

  if ((ap = (char *)mem) == 0)
    return 0;

  /* Find the true start of the memory block to discover which bin */
  p = (union mhead *) ap - 1;

  if (p->mh_alloc == ISMEMALIGN)
    {
      ap -= p->mh_nbytes;
      p = (union mhead *) ap - 1;
    }

  /* return 0 if ISFREE */
  if (p->mh_alloc == ISFREE)
    return 0;
  
  /* Since we use bounds checking, the usable size is the last requested size. */
  return (p->mh_nbytes);
}

#if !defined (NO_VALLOC)
/* This runs into trouble with getpagesize on HPUX, and Multimax machines.
   Patching out seems cleaner than the ugly fix needed.  */
static PTR_T
internal_valloc (size, file, line, flags)
     size_t size;
     const char *file;
     int line, flags;
{
  return internal_memalign (getpagesize (), size, file, line, flags|MALLOC_INTERNAL);
}
#endif /* !NO_VALLOC */

#ifndef NO_CALLOC
static PTR_T
internal_calloc (n, s, file, line, flags)
     size_t n, s;
     const char *file;
     int line, flags;
{
  size_t total;
  PTR_T result;

  total = n * s;
  result = internal_malloc (total, file, line, flags|MALLOC_INTERNAL);
  if (result)
    memset (result, 0, total);
  return result;  
}

static void
internal_cfree (p, file, line, flags)
     PTR_T p;
     const char *file;
     int line, flags;
{
  internal_free (p, file, line, flags|MALLOC_INTERNAL);
}
#endif /* !NO_CALLOC */

#ifdef MALLOC_STATS
int
malloc_free_blocks (size)
     int size;
{
  int nfree;
  register union mhead *p;

  nfree = 0;
  for (p = nextf[size]; p; p = CHAIN (p))
    nfree++;

  return nfree;
}
#endif

#if defined (MALLOC_WRAPFUNCS)
PTR_T
sh_malloc (bytes, file, line)
     size_t bytes;
     const char *file;
     int line;
{
  return internal_malloc (bytes, file, line, MALLOC_WRAPPER);
}

PTR_T
sh_realloc (ptr, size, file, line)
     PTR_T ptr;
     size_t size;
     const char *file;
     int line;
{
  return internal_realloc (ptr, size, file, line, MALLOC_WRAPPER);
}

void
sh_free (mem, file, line)
     PTR_T mem;
     const char *file;
     int line;
{
  internal_free (mem, file, line, MALLOC_WRAPPER);
}

PTR_T
sh_memalign (alignment, size, file, line)
     size_t alignment;
     size_t size;
     const char *file;
     int line;
{
  return internal_memalign (alignment, size, file, line, MALLOC_WRAPPER);
}

#ifndef NO_CALLOC
PTR_T
sh_calloc (n, s, file, line)
     size_t n, s;
     const char *file;
     int line;
{
  return internal_calloc (n, s, file, line, MALLOC_WRAPPER);
}

void
sh_cfree (mem, file, line)
     PTR_T mem;
     const char *file;
     int line;
{
  internal_cfree (mem, file, line, MALLOC_WRAPPER);
}
#endif

#ifndef NO_VALLOC
PTR_T
sh_valloc (size, file, line)
     size_t size;
     const char *file;
     int line;
{
  return internal_valloc (size, file, line, MALLOC_WRAPPER);
}
#endif /* !NO_VALLOC */

#endif /* MALLOC_WRAPFUNCS */

/* Externally-available functions that call their internal counterparts. */

PTR_T
malloc (size)
     size_t size;
{
  return internal_malloc (size, (char *)NULL, 0, 0);
}

PTR_T
realloc (mem, nbytes)
     PTR_T mem;
     size_t nbytes;
{
  return internal_realloc (mem, nbytes, (char *)NULL, 0, 0);
}

void
free (mem)
     PTR_T mem;
{
  internal_free (mem,  (char *)NULL, 0, 0);
}

PTR_T
memalign (alignment, size)
     size_t alignment;
     size_t size;
{
  return internal_memalign (alignment, size, (char *)NULL, 0, 0);
}

#ifndef NO_VALLOC
PTR_T
valloc (size)
     size_t size;
{
  return internal_valloc (size, (char *)NULL, 0, 0);
}
#endif

#ifndef NO_CALLOC
PTR_T
calloc (n, s)
     size_t n, s;
{
  return internal_calloc (n, s, (char *)NULL, 0, 0);
}

void
cfree (mem)
     PTR_T mem;
{
  internal_cfree (mem, (char *)NULL, 0, 0);
}
#endif
