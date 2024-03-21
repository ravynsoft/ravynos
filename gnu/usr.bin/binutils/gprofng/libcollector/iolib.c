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
#include <dlfcn.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>

#include "gp-defs.h"
#include "collector.h"
#include "gp-experiment.h"
#include "memmgr.h"

/* TprintfT(<level>,...) definitions.  Adjust per module as needed */
#define DBG_LT0 0 // for high-level configuration, unexpected errors/warnings
#define DBG_LT1 1 // for configuration details, warnings
#define DBG_LT2 2
#define DBG_LT3 3

/* ------------- Data and prototypes for block management --------- */
#define IO_BLK      0 /* Concurrent requests */
#define IO_SEQ      1 /* All requests are sequential, f.e. JAVA_CLASSES */
#define IO_TXT      2 /* Sequential requests. Text strings. */
#define ST_INIT     0 /* Initial state. Not allocated */
#define ST_FREE     1 /* Available			*/
#define ST_BUSY     2 /* Not available		*/

/* IO_BLK, IO_SEQ */
#define NCHUNKS     64

/* IO_TXT */
#define NBUFS  64 /* Number of text buffers */
#define CUR_BUSY(x) ((uint32_t) ((x)>>63))                  /* bit  63    */
#define CUR_INDX(x) ((uint32_t) (((x)>>57) & 0x3fULL))      /* bits 62:57 */
#define CUR_FOFF(x) ((x) & 0x01ffffffffffffffULL)           /* bits 56: 0 */
#define CUR_MAKE(busy, indx, foff) ((((uint64_t)(busy))<<63) | (((uint64_t)(indx))<<57) | ((uint64_t)(foff)) )

typedef struct Buffer
{
  uint8_t *vaddr;
  uint32_t left;    /* bytes left */
  uint32_t state;   /* ST_FREE or ST_BUSY */
} Buffer;

typedef struct DataHandle
{
  Pckt_type kind;           /* obsolete (to be removed) */
  int iotype;               /* IO_BLK, IO_SEQ, IO_TXT */
  int active;
  char fname[MAXPATHLEN];   /* data file name */

  /* IO_BLK, IO_SEQ */
  uint32_t nflow;           /* number of data flows */
  uint32_t *blkstate;       /* block states, nflow*NCHUNKS array */
  uint32_t *blkoff;         /* block offset, nflow*NCHUNKS array */
  uint32_t nchnk;           /* number of active chunks, probably small for IO_BLK */
  uint8_t *chunks[NCHUNKS]; /* chunks (nflow contiguous blocks in virtual memory) */
  uint32_t chblk[NCHUNKS];  /* number of active blocks in a chunk */
  uint32_t nblk;            /* number of blocks in data file */
  int exempt;               /* if exempt from experiment size limit */

  /* IO_TXT */
  Buffer *buffers;          /* array of text buffers */
  uint64_t curpos;          /* current buffer and file offset */
} DataHandle;

#define PROFILE_DATAHNDL_MAX    16
static DataHandle data_hndls[PROFILE_DATAHNDL_MAX];
static int initialized = 0;
static long blksz;          /* Block size. Multiple of page size. Power of two to make (x%blksz)==(x&(blksz-1)) fast. */
static long log2blksz;      /* log2(blksz) to make (x/blksz)==(x>>log2blksz) fast. */
static uint32_t size_limit; /* Experiment size limit */
static uint32_t cur_size;   /* Current experiment size */
static void init ();
static void deleteHandle (DataHandle *hndl);
static int exp_size_ck (int nblocks, char *fname);

/* IO_BLK, IO_SEQ */
static int allocateChunk (DataHandle *hndl, unsigned ichunk);
static uint8_t *getBlock (DataHandle *hndl, unsigned iflow, unsigned ichunk);
static int remapBlock (DataHandle *hndl, unsigned iflow, unsigned ichunk);
static int newBlock (DataHandle *hndl, unsigned iflow, unsigned ichunk);
static void deleteBlock (DataHandle *hndl, unsigned iflow, unsigned ichunk);

/* IO_TXT */
static int is_not_the_log_file (char *fname);
static int mapBuffer (char *fname, Buffer *buf, off64_t foff);
static int newBuffer (DataHandle *hndl, uint64_t pos);
static void writeBuffer (Buffer *buf, int blk_off, char *src, int len);
static void deleteBuffer (Buffer *buf);

/*
 *    Common buffer management routines
 */
static void
init ()
{
  /* set the block size */
  long pgsz = CALL_UTIL (sysconf)(_SC_PAGESIZE);
  blksz = pgsz;
  log2blksz = 16;   /* ensure a minimum size */
  while ((1 << log2blksz) < blksz)
    log2blksz += 1;
  blksz = 1L << log2blksz;  /* ensure that blksz is a power of two */
  TprintfT (DBG_LT1, "iolib init: page size=%ld (0x%lx) blksz=%ld (0x%lx) log2blksz=%ld\n",
	    pgsz, pgsz, (long) blksz, (long) blksz, (long) log2blksz);
  size_limit = 0;
  cur_size = 0;
  initialized = 1;
}

DataHandle *
__collector_create_handle (char *descp)
{
  int exempt = 0;
  char *desc = descp;
  if (desc[0] == '*')
    {
      desc++;
      exempt = 1;
    }
  if (!initialized)
    init ();

  /* set up header for file, file name, etc. */
  if (*__collector_exp_dir_name == 0)
    {
      __collector_log_write ("<event kind=\"%s\" id=\"%d\">__collector_exp_dir_name==NULL</event>\n",
			     SP_JCMD_CERROR, COL_ERROR_EXPOPEN);
      return NULL;
    }
  char fname[MAXPATHLEN];
  CALL_UTIL (strlcpy)(fname, __collector_exp_dir_name, sizeof (fname));
  CALL_UTIL (strlcat)(fname, "/", sizeof (fname));
  Pckt_type kind = 0;
  int iotype = IO_BLK;
  if (__collector_strcmp (desc, SP_HEAPTRACE_FILE) == 0)
    kind = HEAP_PCKT;
  else if (__collector_strcmp (desc, SP_SYNCTRACE_FILE) == 0)
    kind = SYNC_PCKT;
  else if (__collector_strcmp (desc, SP_IOTRACE_FILE) == 0)
    kind = IOTRACE_PCKT;
  else if (__collector_strcmp (desc, SP_RACETRACE_FILE) == 0)
    kind = RACE_PCKT;
  else if (__collector_strcmp (desc, SP_PROFILE_FILE) == 0)
    kind = PROF_PCKT;
  else if (__collector_strcmp (desc, SP_OMPTRACE_FILE) == 0)
    kind = OMP_PCKT;
  else if (__collector_strcmp (desc, SP_HWCNTR_FILE) == 0)
    kind = HW_PCKT;
  else if (__collector_strcmp (desc, SP_DEADLOCK_FILE) == 0)
    kind = DEADLOCK_PCKT;
  else if (__collector_strcmp (desc, SP_FRINFO_FILE) == 0)
    CALL_UTIL (strlcat)(fname, "data.", sizeof (fname));
  else if (__collector_strcmp (desc, SP_LOG_FILE) == 0)
    iotype = IO_TXT;
  else if (__collector_strcmp (desc, SP_MAP_FILE) == 0)
    iotype = IO_TXT;
  else if (__collector_strcmp (desc, SP_JCLASSES_FILE) == 0)
    iotype = IO_SEQ;
  else
    {
      __collector_log_write ("<event kind=\"%s\" id=\"%d\">iolib unknown file desc %s</event>\n",
			     SP_JCMD_CERROR, COL_ERROR_EXPOPEN, desc);
      return NULL;
    }

  CALL_UTIL (strlcat)(fname, desc, sizeof (fname));
  TprintfT (DBG_LT1, "createHandle calling open on fname = `%s', desc = `%s' %s\n",
	    fname, desc, (exempt == 0 ? "non-exempt" : "exempt"));

  /* allocate a handle -- not mt-safe */
  DataHandle *hndl = NULL;
  for (int i = 0; i < PROFILE_DATAHNDL_MAX; ++i)
    if (data_hndls[i].active == 0)
      {
	hndl = &data_hndls[i];
	break;
      }

  /* out of handles? */
  if (hndl == NULL)
    {
      __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
			     SP_JCMD_CERROR, COL_ERROR_NOHNDL, fname);
      return NULL;
    }

  hndl->kind = kind;
  hndl->nblk = 0;
  hndl->exempt = exempt;
  CALL_UTIL (strlcpy)(hndl->fname, fname, sizeof (hndl->fname));
  int fd = CALL_UTIL (open)(hndl->fname,
			    O_RDWR | O_CREAT | O_TRUNC | O_EXCL,
			    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fd < 0)
    {
      TprintfT (0, "createHandle open failed --  hndl->fname = `%s', SP_LOG_FILE = `%s': %s\n",
		hndl->fname, SP_LOG_FILE, CALL_UTIL (strerror)(errno));
      if (is_not_the_log_file (hndl->fname) == 0)
	{
	  char errbuf[4096];
	  /* If we are trying to create the handle for the log file, write to stderr, not the experiment */
	  CALL_UTIL (snprintf)(errbuf, sizeof (errbuf),
			       "create_handle: COL_ERROR_LOG_OPEN %s: %s\n", hndl->fname, CALL_UTIL (strerror)(errno));
	  CALL_UTIL (write)(2, errbuf, CALL_UTIL (strlen)(errbuf));

	}
      else
	__collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s: create_handle</event>\n",
				 SP_JCMD_CERROR, COL_ERROR_FILEOPN, errno, hndl->fname);
      return NULL;
    }
  CALL_UTIL (close)(fd);

  hndl->iotype = iotype;
  if (hndl->iotype == IO_TXT)
    {
      /* allocate our buffers in virtual memory */
      /* later, we will remap buffers individually to the file */
      uint8_t *memory = (uint8_t*) CALL_UTIL (mmap64_) (0,
	      (size_t) (NBUFS * blksz), PROT_READ | PROT_WRITE,
#if ARCH(SPARC)
	      MAP_SHARED | MAP_ANON,
#else
	      MAP_PRIVATE | MAP_ANON,
#endif
	      -1, (off64_t) 0);
      if (memory == MAP_FAILED)
	{
	  TprintfT (0, "create_handle: can't mmap MAP_ANON (for %s): %s\n", hndl->fname, CALL_UTIL (strerror)(errno));
	  /* see if this is the log file */
	  if (is_not_the_log_file (hndl->fname) == 0)
	    {
	      /* If we are trying to map the log file, write to stderr, not to the experiment */
	      char errbuf[4096];
	      CALL_UTIL (snprintf)(errbuf, sizeof (errbuf),
				   "create_handle: can't mmap MAP_ANON (for %s): %s\n", hndl->fname, CALL_UTIL (strerror)(errno));
	      CALL_UTIL (write)(2, errbuf, CALL_UTIL (strlen)(errbuf));
	    }
	  else  /* write the error message into the experiment */
	    __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">MAP_ANON (for %s); create_handle</event>\n",
				     SP_JCMD_CERROR, COL_ERROR_FILEMAP, errno, hndl->fname);
	  return NULL;
	}
      TprintfT (DBG_LT2, " create_handle IO_TXT data buffer length=%ld (0x%lx) file='%s' memory=%p -- %p\n",
		(long) (NBUFS * blksz), (long) (NBUFS * blksz), hndl->fname,
		memory, memory + (NBUFS * blksz) - 1);

      /* set up an array of buffers, pointing them to the virtual addresses */
      TprintfT (DBG_LT2, "create_handle IO_TXT Buffer structures fname = `%s', NBUFS= %d, size = %ld (0x%lx)\n", fname,
		NBUFS, (long) NBUFS * sizeof (Buffer), (long) NBUFS * sizeof (Buffer));
      hndl->buffers = (Buffer*) __collector_allocCSize (__collector_heap, NBUFS * sizeof (Buffer), 1);
      if (hndl->buffers == NULL)
	{
	  TprintfT (0, "create_handle allocCSize for hndl->buffers failed\n");
	  CALL_UTIL (munmap)(memory, NBUFS * blksz);
	  return NULL;
	}
      for (int i = 0; i < NBUFS; i++)
	{
	  Buffer *buf = &hndl->buffers[i];
	  buf->vaddr = memory + i * blksz;
	  buf->state = ST_FREE;
	}
      /* set the file pointer to the beginning of the file */
      hndl->curpos = CUR_MAKE (0, 0, 0);
    }
  else
    {
      if (hndl->iotype == IO_BLK)
	{
	  long nflow = CALL_UTIL (sysconf)(_SC_NPROCESSORS_ONLN);
	  if (nflow < 16)
	    nflow = 16;
	  hndl->nflow = (uint32_t) nflow;
	}
      else if (hndl->iotype == IO_SEQ)
	hndl->nflow = 1;
      TprintfT (DBG_LT2, "create_handle calling allocCSize blkstate fname=`%s' nflow=%d NCHUNKS=%d size=%ld (0x%lx)\n",
		fname, hndl->nflow, NCHUNKS,
		(long) (hndl->nflow * NCHUNKS * sizeof (uint32_t)),
		(long) (hndl->nflow * NCHUNKS * sizeof (uint32_t)));
      uint32_t *blkstate = (uint32_t*) __collector_allocCSize (__collector_heap, hndl->nflow * NCHUNKS * sizeof (uint32_t), 1);
      if (blkstate == NULL)
	return NULL;
      for (int j = 0; j < hndl->nflow * NCHUNKS; ++j)
	blkstate[j] = ST_INIT;
      hndl->blkstate = blkstate;
      TprintfT (DBG_LT2, "create_handle calling allocCSize blkoff fname=`%s' nflow=%d NCHUNKS=%d size=%ld (0x%lx)\n",
		fname, hndl->nflow, NCHUNKS,
		(long) (hndl->nflow * NCHUNKS * sizeof (uint32_t)),
		(long) (hndl->nflow * NCHUNKS * sizeof (uint32_t)));
      hndl->blkoff = (uint32_t*) __collector_allocCSize (__collector_heap, hndl->nflow * NCHUNKS * sizeof (uint32_t), 1);
      if (hndl->blkoff == NULL)
	return NULL;
      hndl->nchnk = 0;
      for (int j = 0; j < NCHUNKS; ++j)
	{
	  hndl->chunks[j] = NULL;
	  hndl->chblk[j] = 0;
	}
    }
  hndl->active = 1;
  return hndl;
}

static void
deleteHandle (DataHandle *hndl)
{
  if (hndl->active == 0)
    return;
  hndl->active = 0;

  if (hndl->iotype == IO_BLK || hndl->iotype == IO_SEQ)
    {
      /* Delete all blocks. */
      /* Since access to hndl->active is not synchronized it's still
       * possible that we leave some blocks undeleted.
       */
      for (int j = 0; j < hndl->nflow * NCHUNKS; ++j)
	{
	  uint32_t oldstate = hndl->blkstate[j];
	  if (oldstate != ST_FREE)
	    continue;
	  /* Mark as busy */
	  uint32_t state = __collector_cas_32 (hndl->blkstate + j, oldstate, ST_BUSY);
	  if (state != oldstate)
	    continue;
	  deleteBlock (hndl, j / NCHUNKS, j % NCHUNKS);
	}
    }
  else if (hndl->iotype == IO_TXT)
    {
      /*
       * First, make sure that buffers are in some "coherent" state:
       *
       * At this point, the handle is no longer active.  But some threads
       * might already have passed the active-handle check and are now
       * trying to schedule writes.  So, set the handle pointer to "busy".
       * This will prevent new writes from being scheduled.  Threads that
       * polling will time out.
       */
      hrtime_t timeout = __collector_gethrtime () + 10 * ((hrtime_t) 1000000000);
      volatile uint32_t busy = 0;
      while (1)
	{
	  uint32_t indx;
	  uint64_t opos, npos, foff;
	  int blk_off;
	  /* read the current pointer */
	  opos = hndl->curpos;
	  busy = CUR_BUSY (opos);
	  indx = CUR_INDX (opos);
	  foff = CUR_FOFF (opos);
	  if (busy == 1)
	    {
	      if (__collector_gethrtime () > timeout)
		{
		  TprintfT (0, "deleteHandle ERROR: timeout cleaning up handle for %s\n", hndl->fname);
		  return;
		}
	      continue;
	    }
	  blk_off = foff & (blksz - 1);
	  if (blk_off > 0)
	    foff += blksz - blk_off;
	  npos = CUR_MAKE (1, indx, foff);

	  /* try to update the handle position atomically */
	  if (__collector_cas_64p (&hndl->curpos, &opos, &npos) != opos)
	    continue;

	  /*
	   * If the last buffer won't be filled, account for
	   * the white space at the end so that the buffer will
	   * be deleted properly.
	   */
	  if (blk_off > 0)
	    {
	      Buffer *buf = &hndl->buffers[indx];
	      if (__collector_subget_32 (&buf->left, blksz - blk_off) == 0)
		deleteBuffer (buf);
	    }
	  break;
	}
      /* wait for buffers to be deleted */
      timeout = __collector_gethrtime () + 10 * ((hrtime_t) 1000000000);
      for (int i = 0; i < NBUFS; i++)
	{
	  Buffer *buf = &hndl->buffers[i];
	  while (__collector_cas_32 (&buf->state, ST_FREE, ST_INIT) != ST_FREE)
	    {
	      if (__collector_gethrtime () > timeout)
		{
		  TprintfT (0, "deleteHandle ERROR: timeout waiting for buffer %d for %s\n", i, hndl->fname);
		  return;
		}
	    }
	  CALL_UTIL (munmap)(buf->vaddr, blksz);
	}

      /* free buffer array */
      __collector_freeCSize (__collector_heap, hndl->buffers, NBUFS * sizeof (Buffer));
    }
}

void
__collector_delete_handle (DataHandle *hndl)
{
  if (hndl == NULL)
    return;
  deleteHandle (hndl);
}

static int
exp_size_ck (int nblocks, char *fname)
{
  if (size_limit == 0)
    return 0;
  /* do an atomic add to the cur_size */
  uint32_t old_size = cur_size;
  uint32_t new_size;
  for (;;)
    {
      new_size = __collector_cas_32 (&cur_size, old_size, old_size + nblocks);
      if (new_size == old_size)
	{
	  new_size = old_size + nblocks;
	  break;
	}
      old_size = new_size;
    }
  TprintfT (DBG_LT2, "exp_size_ck() adding %d block(s); new_size = %d, limit = %d blocks; fname = %s\n",
	    nblocks, new_size, size_limit, fname);

  /* pause the entire collector if we have exceeded the limit */
  if (old_size < size_limit && new_size >= size_limit)
    {
      TprintfT (0, "exp_size_ck() experiment size limit exceeded; new_size = %ld, limit = %ld blocks; fname = %s\n",
		(long) new_size, (long) size_limit, fname);
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\">%ld blocks (each %ld bytes)</event>\n",
				    SP_JCMD_CWARN, COL_ERROR_SIZELIM, (long) size_limit, (long) blksz);
      __collector_pause_m ("size-limit");
      __collector_terminate_expt ();
      return -1;
    }
  return 0;
}

int
__collector_set_size_limit (char *par)
{
  if (!initialized)
    init ();

  int lim = CALL_UTIL (strtol)(par, &par, 0);
  size_limit = (uint32_t) ((uint64_t) lim * 1024 * 1024 / blksz);
  TprintfT (DBG_LT0, "collector_size_limit set to %d MB. = %d blocks\n",
	    lim, size_limit);
  (void) __collector_log_write ("<setting limit=\"%d\"/>\n", lim);
  return COL_ERROR_NONE;
}

/*
 *    IO_BLK and IO_SEQ files
 */

/*
 * Allocate a chunk (nflow blocks) contiguously in virtual memory.
 * Its blocks will be mmapped to the file individually.
 */
static int
allocateChunk (DataHandle *hndl, unsigned ichunk)
{
  /*
   * hndl->chunks[ichunk] is one of:
   *   - NULL (initial value)
   *   - CHUNK_BUSY (transition state when allocating the chunk)
   *   - some address (the allocated chunk)
   */
  uint8_t *CHUNK_BUSY = (uint8_t *) 1;
  hrtime_t timeout = 0;
  while (1)
    {
      if (hndl->chunks[ichunk] > CHUNK_BUSY)
	return 0; /* the chunk has already been allocated */
      /* try to allocate the chunk (change: NULL => CHUNK_BUSY) */
      if (__collector_cas_ptr (&hndl->chunks[ichunk], NULL, CHUNK_BUSY) == NULL)
	{
	  /* allocate virtual memory */
	  uint8_t *newchunk = (uint8_t*) CALL_UTIL (mmap64_) (0,
		  (size_t) (blksz * hndl->nflow), PROT_READ | PROT_WRITE,
#if ARCH(SPARC)
		  MAP_SHARED | MAP_ANON,
#else
		  MAP_PRIVATE | MAP_ANON,
#endif
		  -1, (off64_t) 0);
	  if (newchunk == MAP_FAILED)
	    {
	      deleteHandle (hndl);
	      TprintfT (DBG_LT1, " allocateChunk mmap:  start=0x%x length=%ld (0x%lx), offset=%d ret=%p\n",
			0, (long) (blksz * hndl->nflow),
			(long) (blksz * hndl->nflow), 0, newchunk);
	      TprintfT (0, "allocateChunk: can't mmap MAP_ANON (for %s): %s\n", hndl->fname, CALL_UTIL (strerror) (errno));
	      __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">MAP_ANON (for %s)</event>\n",
				     SP_JCMD_CERROR, COL_ERROR_FILEMAP, errno, hndl->fname);
	      return 1;
	    }

	  /* assign allocated address to our chunk */
	  if (__collector_cas_ptr (&hndl->chunks[ichunk], CHUNK_BUSY, newchunk) != CHUNK_BUSY)
	    {
	      TprintfT (0, "allocateChunk: can't release chunk CAS lock for %s\n", hndl->fname);
	      __collector_log_write ("<event kind=\"%s\" id=\"%d\">couldn't release chunk CAS lock (%s)</event>\n",
				     SP_JCMD_CERROR, COL_ERROR_GENERAL, hndl->fname);
	    }
	  __collector_inc_32 (&hndl->nchnk);
	  return 0;
	}

      /* check for time out */
      if (timeout == 0)
	timeout = __collector_gethrtime () + 10 * ((hrtime_t) 1000000000);
      if (__collector_gethrtime () > timeout)
	{
	  TprintfT (0, "allocateChunk: timeout for %s\n", hndl->fname);
	  __collector_log_write ("<event kind=\"%s\" id=\"%d\">timeout allocating chunk for %s</event>\n",
				 SP_JCMD_CERROR, COL_ERROR_GENERAL, hndl->fname);
	  return 1;
	}
    }
}

/*
 * Get the address for block (iflow,ichunk).
 */
static uint8_t *
getBlock (DataHandle *hndl, unsigned iflow, unsigned ichunk)
{
  return hndl->chunks[ichunk] + iflow * blksz;
}

/*
 * Map block (iflow,ichunk) to the next part of the file.
 */
static int
remapBlock (DataHandle *hndl, unsigned iflow, unsigned ichunk)
{
  int rc = 0;
  int fd;
  /* Get the old file nblk and increment it atomically. */
  uint32_t oldblk = hndl->nblk;
  for (;;)
    {
      uint32_t newblk = __collector_cas_32 (&hndl->nblk, oldblk, oldblk + 1);
      if (newblk == oldblk)
	break;
      oldblk = newblk;
    }
  off64_t offset = (off64_t) oldblk * blksz;

  /* 6618470: disable thread cancellation */
  int old_cstate;
  pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_cstate);

  /* Open the file. */
  int iter = 0;
  hrtime_t tso = __collector_gethrtime ();
  for (;;)
    {
      fd = CALL_UTIL (open)(hndl->fname, O_RDWR, 0);
      if (fd < 0)
	{
	  if (errno == EMFILE)
	    {
	      /* too many open files */
	      iter++;
	      if (iter > 1000)
		{
		  /* we've tried 1000 times; kick error back to caller */
		  char errmsg[MAXPATHLEN + 50];
		  hrtime_t teo = __collector_gethrtime ();
		  double deltato = (double) (teo - tso) / 1000000.;
		  (void) CALL_UTIL (snprintf) (errmsg, sizeof (errmsg),
			" t=%lu, %s: open-retries-failed=%d, %3.6f ms.; remap\n",
			(unsigned long) __collector_thr_self (), hndl->fname,
			iter, deltato);
		  __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
					 SP_JCMD_COMMENT, COL_COMMENT_NONE, errmsg);
		  rc = 1;
		  goto exit;
		}
	      /* keep trying */
	      continue;
	    }
	  deleteHandle (hndl);
	  TprintfT (0, "remapBlock: can't open file: %s: %s\n", hndl->fname, STR (CALL_UTIL (strerror)(errno)));
	  __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">t=%lu, %s: remap </event>\n",
				 SP_JCMD_CERROR, COL_ERROR_FILEOPN, errno,
				 (unsigned long) __collector_thr_self (),
				 hndl->fname);
	  rc = 1;
	  goto exit;
	}
      else
	break;
    }

  /* report number of retries of the open due to too many open fd's */
  if (iter > 0)
    {
      char errmsg[MAXPATHLEN + 50];
      hrtime_t teo = __collector_gethrtime ();
      double deltato = (double) (teo - tso) / 1000000.;
      (void) CALL_UTIL (snprintf) (errmsg, sizeof (errmsg),
	      " t=%d, %s: open-retries=%lu, %3.6f ms.; remap\n",
	      (unsigned long) __collector_thr_self (), hndl->fname,
	      iter, deltato);
      __collector_log_write ("<event kind=\"%s\" id=\"%d\">%s</event>\n",
			     SP_JCMD_COMMENT, COL_COMMENT_NONE, errmsg);
    }

  /* Ensure disk space is allocated and the block offset is 0 */
  uint32_t zero = 0;
  int n = CALL_UTIL (pwrite64_) (fd, &zero, sizeof (zero),
				(off64_t) (offset + blksz - sizeof (zero)));
  if (n <= 0)
    {
      deleteHandle (hndl);
      TprintfT (0, "remapBlock: can't pwrite file: %s : errno=%d\n", hndl->fname, errno);
      __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s: remap</event>\n",
			     SP_JCMD_CERROR, COL_ERROR_NOSPACE, errno, hndl->fname);
      CALL_UTIL (close)(fd);
      rc = 1;
      goto exit;
    }
  hndl->blkoff[iflow * NCHUNKS + ichunk] = 0;

  /* Map block to file */
  uint8_t *bptr = getBlock (hndl, iflow, ichunk);
  uint8_t *vaddr = (uint8_t *) CALL_UTIL (mmap64_) ((void*) bptr,
	  (size_t) blksz, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
	  fd, offset);

  if (vaddr != bptr)
    {
      deleteHandle (hndl);
      TprintfT (DBG_LT1, " remapBlock mmap:  start=%p length=%ld (0x%lx) offset=0x%llx ret=%p\n",
		bptr, (long) blksz, (long) blksz, (long long) offset, vaddr);
      TprintfT (0, "remapBlock: can't mmap file: %s : errno=%d\n", hndl->fname, errno);
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s: remap</event>\n",
				    SP_JCMD_CERROR, COL_ERROR_FILEMAP, errno, hndl->fname);
      CALL_UTIL (close)(fd);
      rc = 1;
      goto exit;
    }
  CALL_UTIL (close)(fd);

  if (hndl->exempt == 0)
    exp_size_ck (1, hndl->fname);
  else
    Tprintf (DBG_LT1, "exp_size_ck() bypassed for %d block(s); exempt fname = %s\n",
	       1, hndl->fname);
exit:
  /* Restore the previous cancellation state */
  pthread_setcancelstate (old_cstate, NULL);

  return rc;
}

static int
newBlock (DataHandle *hndl, unsigned iflow, unsigned ichunk)
{
  if (allocateChunk (hndl, ichunk) != 0)
    return 1;
  if (remapBlock (hndl, iflow, ichunk) != 0)
    return 1;

  /* Update the number of active blocks */
  __collector_inc_32 (hndl->chblk + ichunk);
  return 0;
}

static void
deleteBlock (DataHandle *hndl, unsigned iflow, unsigned ichunk)
{
  uint8_t *bptr = getBlock (hndl, iflow, ichunk);
  CALL_UTIL (munmap)((void*) bptr, blksz);
  hndl->blkstate[iflow * NCHUNKS + ichunk] = ST_INIT;

  /* Update the number of active blocks */
  __collector_dec_32 (hndl->chblk + ichunk);
}

int
__collector_write_record (DataHandle *hndl, Common_packet *pckt)
{
  if (hndl == NULL || !hndl->active)
    return 1;
  /* fill in the fields of the common packet structure */
  if (pckt->type == 0)
    pckt->type = hndl->kind;
  if (pckt->tstamp == 0)
    pckt->tstamp = __collector_gethrtime ();
  if (pckt->lwp_id == 0)
    pckt->lwp_id = __collector_lwp_self ();
  if (pckt->thr_id == 0)
    pckt->thr_id = __collector_thr_self ();
  if (pckt->cpu_id == 0)
    pckt->cpu_id = CALL_UTIL (getcpuid)();
  if (pckt->tsize == 0)
    pckt->tsize = sizeof (Common_packet);
  TprintfT (DBG_LT3, "collector_write_record to %s, type:%d tsize:%d\n",
	    hndl->fname, pckt->type, pckt->tsize);
  return __collector_write_packet (hndl, (CM_Packet*) pckt);
}

int
__collector_write_packet (DataHandle *hndl, CM_Packet *pckt)
{
  if (hndl == NULL || !hndl->active)
    return 1;

  /* if the experiment is not open, there should be no writes */
  if (__collector_expstate != EXP_OPEN)
    {
#ifdef DEBUG
      char *xstate;
      switch (__collector_expstate)
	{
	case EXP_INIT:
	  xstate = "EXP_INIT";
	  break;
	case EXP_OPEN:
	  xstate = "EXP_OPEN";
	  break;
	case EXP_PAUSED:
	  xstate = "EXP_PAUSED";
	  break;
	case EXP_CLOSED:
	  xstate = "EXP_CLOSED";
	  break;
	default:
	  xstate = "Unknown";
	  break;
	}
      TprintfT (0, "collector_write_packet: write to %s while experiment state is %s\n",
		hndl->fname, xstate);
#endif
      return 1;
    }
  int recsz = pckt->tsize;
  if (recsz > blksz)
    {
      TprintfT (0, "collector_write_packet: packet too long: %d (max %ld)\n", recsz, blksz);
      return 1;
    }
  collector_thread_t tid = __collector_no_threads ? __collector_lwp_self ()
						  : __collector_thr_self ();
  unsigned iflow = (unsigned) (((unsigned long) tid) % hndl->nflow);

  /* Acquire block */
  uint32_t *sptr = &hndl->blkstate[iflow * NCHUNKS];
  uint32_t state = ST_BUSY;
  unsigned ichunk;
  for (ichunk = 0; ichunk < NCHUNKS; ++ichunk)
    {
      uint32_t oldstate = sptr[ichunk];
      if (oldstate == ST_BUSY)
	continue;
      /* Mark as busy */
      state = __collector_cas_32 (sptr + ichunk, oldstate, ST_BUSY);
      if (state == oldstate)
	break;
      if (state == ST_BUSY)
	continue;
      /* It's possible the state changed from ST_INIT to ST_FREE */
      oldstate = state;
      state = __collector_cas_32 (sptr + ichunk, oldstate, ST_BUSY);
      if (state == oldstate)
	break;
    }

  if (state == ST_BUSY || ichunk == NCHUNKS)
    {
      /* We are out of blocks for this data flow.
       * We might switch to another flow but for now report and return.
       */
      TprintfT (0, "collector_write_packet: all %d blocks on flow %d for %s are busy\n",
		NCHUNKS, iflow, hndl->fname);
      return 1;
    }

  if (state == ST_INIT && newBlock (hndl, iflow, ichunk) != 0)
      return 1;
  uint8_t *bptr = getBlock (hndl, iflow, ichunk);
  uint32_t blkoff = hndl->blkoff[iflow * NCHUNKS + ichunk];
  if (blkoff + recsz > blksz)
    {
      /* The record doesn't fit. Close the block */
      if (blkoff < blksz)
	{
	  Common_packet *closed = (Common_packet *) (bptr + blkoff);
	  closed->type = CLOSED_PCKT;
	  closed->tsize = blksz - blkoff; /* redundant */
	}
      if (remapBlock (hndl, iflow, ichunk) != 0)
	return 1;
      blkoff = hndl->blkoff[iflow * NCHUNKS + ichunk];
    }
  if (blkoff + recsz < blksz)
    {
      /* Set the empty padding */
      Common_packet *empty = (Common_packet *) (bptr + blkoff + recsz);
      empty->type = EMPTY_PCKT;
      empty->tsize = blksz - blkoff - recsz;
    }
  __collector_memcpy (bptr + blkoff, pckt, recsz);

  /* Release block */
  if (hndl->active == 0)
    {
      deleteBlock (hndl, iflow, ichunk);
      return 0;
    }
  hndl->blkoff[iflow * NCHUNKS + ichunk] += recsz;
  sptr[ichunk] = ST_FREE;
  return 0;
}

/*
 *    IO_TXT files
 *
 *      IO_TXT covers the case where many threads are trying to write text messages
 *      sequentially (atomically) to a file.  Examples include SP_LOG_FILE and SP_MAP_FILE.
 *
 *      The file is not written directly, but by writing to mmapped virtual memory.
 *      The granularity of the mapping is a "Buffer".  There may be as many as
 *      NBUFS buffers at any one time.
 *
 *      The current position of the file is handled via hndl->curpos.
 *
 *        * It is accessed atomically with 64-bit CAS instructions.
 *
 *        * This 64-bit word encapsulates:
 *          - busy: a bit to lock access to hndl->curpos
 *          - indx: an index indicating which Buffer to use for the current position
 *          - foff: the file offset
 *
 *        * The contents are accessed with:
 *          - unpack macros: CUR_BUSY CUR_INDX CUR_FOFF
 *          -   pack macro : CUR_MAKE
 *
 *      Conceptually, what happens when a thread wants to write a message is:
 *      - acquire the hndl->curpos "busy" lock
 *        . acquire and map new Buffers if needed to complete the message
 *        . update the file offset
 *        . release the lock
 *      - write to the corresponding buffers
 *
 *      Each Buffer has a buf->left field that tracks how many more bytes
 *      need to be written to the Buffer.  After a thread writes to a Buffer,
 *      it decrements buf->left atomically.  When buf->left reaches 0, the
 *      Buffer (mapping) is deleted, freeing the Buffer for a new mapping.
 *
 *      The actual implementation has some twists:
 *
 *      * If the entire text message fits into the current Buffer -- that is,
 *        no new Buffers are needed -- the thread does not acquire the lock.
 *        It simply updates hndl->curpos atomically to the new file offset.
 *
 *      * There are various timeouts to prevent hangs in case of abnormalities.
 */
static int
is_not_the_log_file (char *fname)
{
  if (CALL_UTIL (strstr)(fname, SP_LOG_FILE) == NULL)
    return 1;
  return 0;
}

static int
mapBuffer (char *fname, Buffer *buf, off64_t foff)
{
  int rc = 0;
  /* open fname */
  int fd = CALL_UTIL (open)(fname, O_RDWR, 0);
  if (fd < 0)
    {
      TprintfT (0, "mapBuffer ERROR: can't open file: %s\n", fname);
      if (is_not_the_log_file (fname))
	__collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s: mapBuffer</event>\n",
			       SP_JCMD_CERROR, COL_ERROR_FILEOPN, errno, fname);
      return 1;
    }
  TprintfT (DBG_LT2, "mapBuffer pwrite file %s at 0x%llx\n", fname, (long long) foff);

  /* ensure disk space is allocated */
  char nl = '\n';
  int n = CALL_UTIL (pwrite64_) (fd, &nl, sizeof (nl),
				 (off64_t) (foff + blksz - sizeof (nl)));
  if (n <= 0)
    {
      TprintfT (0, "mapBuffer ERROR: can't pwrite file %s at 0x%llx\n", fname,
		(long long) (foff + blksz - sizeof (nl)));
      if (is_not_the_log_file (fname))
	__collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s: mapBuffer</event>\n",
			       SP_JCMD_CERROR, COL_ERROR_FILETRNC, errno, fname);
      rc = 1;
      goto exit;
    }
  /* mmap buf->vaddr to fname at foff */
  uint8_t *vaddr = CALL_UTIL (mmap64_) (buf->vaddr, (size_t) blksz,
	  PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, foff);
  if (vaddr != buf->vaddr)
    {
      TprintfT (DBG_LT1, " mapBuffer mmap:  start=%p length=%ld (0x%lx) offset=0x%llx ret=%p\n",
		buf->vaddr, blksz, blksz, (long long) foff, vaddr);
      TprintfT (0, "mapBuffer ERROR: can't mmap %s:  vaddr=%p  size=%ld (0x%lx)  ret=%p off=0x%llx errno=%d\n",
		fname, buf->vaddr, blksz, blksz, vaddr, (long long) foff, errno);
      if (is_not_the_log_file (fname))
	__collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s: mapBuffer</event>\n",
			       SP_JCMD_CERROR, COL_ERROR_FILEMAP, errno, fname);
      rc = 1;
    }
  else
    buf->left = blksz;
exit:
  CALL_UTIL (close)(fd);

  /* Should we check buffer size?  Let's not since:
   * - IO_TXT is typically not going to be that big
   * - we want log.xml to be treated specially
   */
  /* exp_size_ck( 1, fname ); */
  return rc;
}

static int
newBuffer (DataHandle *hndl, uint64_t foff)
{
  /* find a ST_FREE buffer and mark it ST_BUSY */
  int ibuf;
  for (ibuf = 0; ibuf < NBUFS; ibuf++)
    if (__collector_cas_32 (&hndl->buffers[ibuf].state, ST_FREE, ST_BUSY) == ST_FREE)
      break;
  if (ibuf >= NBUFS)
    {
      TprintfT (0, "newBuffer ERROR: all buffers busy for %s\n", hndl->fname);
      return -1;
    }
  Buffer *nbuf = hndl->buffers + ibuf;

  /* map buffer */
  if (mapBuffer (hndl->fname, nbuf, foff) != 0)
    {
      nbuf->state = ST_FREE;
      ibuf = -1;
      goto exit;
    }
exit:
  return ibuf;
}

static void
writeBuffer (Buffer *buf, int blk_off, char *src, int len)
{
  __collector_memcpy (buf->vaddr + blk_off, src, len);
  if (__collector_subget_32 (&buf->left, len) == 0)
    deleteBuffer (buf);
}

static void
deleteBuffer (Buffer *buf)
{
  buf->state = ST_FREE;
}

int
__collector_write_string (DataHandle *hndl, char *src, int len)
{
  if (hndl == NULL || !hndl->active)
    return 1;
  if (len <= 0)
    return 0;

  hrtime_t timeout = __collector_gethrtime () + 20 * ((hrtime_t) 1000000000);
  volatile uint32_t busy = 0;
  while (1)
    {
      uint32_t indx;
      uint64_t opos, foff, base;
      int blk_off, buf_indices[NBUFS], ibuf, nbufs;

      /* read and decode the current pointer */
      opos = hndl->curpos;
      busy = CUR_BUSY (opos);
      indx = CUR_INDX (opos);
      foff = CUR_FOFF (opos);
      if (busy == 1)
	{
	  if (__collector_gethrtime () > timeout)
	    {
	      /*
	       * E.g., if another thread deleted the handle
	       * after we checked hndl->active.
	       */
	      TprintfT (0, "__collector_write_string ERROR: timeout writing length=%d to text file: %s\n", len, hndl->fname);
	      return 1;
	    }
	  continue;
	}

      /* initial block offset */
      blk_off = foff & (blksz - 1);

      /* number of new buffers to map */
      int lastbuf = ((foff + len - 1) >> log2blksz); /* last block file index we will write */
      int firstbuf = ((foff - 1) >> log2blksz); /* last block file index we have written */
      nbufs = lastbuf - firstbuf;
      TprintfT (DBG_LT2, "__collector_write_string firstbuf = %d, lastbuf = %d, nbufs = %d, log2blksz = %ld\n",
		firstbuf, lastbuf, nbufs, log2blksz);
      if (nbufs >= NBUFS)
	{
	  Tprintf (0, "__collector_write_string ERROR: string of length %d too long to be written to text file: %s\n", len, hndl->fname);
	  return 1;
	}

      /* things are simple if we don't need new buffers */
      if (nbufs == 0)
	{
	  /* try to update the handle position atomically */
	  uint64_t npos = CUR_MAKE (0, indx, foff + len);
	  if (__collector_cas_64p (&hndl->curpos, &opos, &npos) != opos)
	    continue;

	  /* success!  copy our string and we're done */
	  TprintfT (DBG_LT2, "__collector_write_string writeBuffer[%d]: vaddr = %p, len = %d, foff = %lld, '%s'\n",
		    indx, hndl->buffers[indx].vaddr, len, (long long) foff, src);
	  writeBuffer (&hndl->buffers[indx], foff & (blksz - 1), src, len);
	  break;
	}

      /* initialize the new signal mask */
      sigset_t new_mask;
      sigset_t old_mask;
      CALL_UTIL (sigfillset)(&new_mask);

      /* 6618470: disable thread cancellation */
      int old_cstate;
      pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &old_cstate);
      /* block all signals */
      CALL_UTIL (sigprocmask)(SIG_SETMASK, &new_mask, &old_mask);

      /* but if we need new buffers, "lock" the handle pointer */
      uint64_t lpos = CUR_MAKE (1, indx, foff);
      if (__collector_cas_64p (&hndl->curpos, &opos, &lpos) != opos)
	{
	  /* restore signal mask */
	  CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
	  /* Restore the previous cancellation state */
	  pthread_setcancelstate (old_cstate, NULL);
	  continue;
	}

      /* map new buffers */
      base = ((foff - 1) & ~(blksz - 1)); /* last buffer to have been mapped */
      for (ibuf = 0; ibuf < nbufs; ibuf++)
	{
	  base += blksz;
	  buf_indices[ibuf] = newBuffer (hndl, base);
	  if (buf_indices[ibuf] < 0)
	    break;
	}

      /* "unlock" the handle pointer */
      uint64_t npos = CUR_MAKE (0, indx, foff);
      if (ibuf == nbufs)
	npos = CUR_MAKE (0, buf_indices[nbufs - 1], foff + len);
      if (__collector_cas_64p (&hndl->curpos, &lpos, &npos) != lpos)
	{
	  TprintfT (0, "__collector_write_string ERROR: file handle corrupted: %s\n", hndl->fname);
	  /*
	   * At this point, the handle is apparently corrupted and
	   * presumably locked.  No telling what's going on.  Still
	   * let's proceed and write our data and let a later thread
	   * raise an error if it encounters one.
	   */
	}

      /* restore signal mask */
      CALL_UTIL (sigprocmask)(SIG_SETMASK, &old_mask, NULL);
      /* Restore the previous cancellation state */
      pthread_setcancelstate (old_cstate, NULL);

      /* if we couldn't map all the buffers we needed, don't write any part of the string */
      if (ibuf < nbufs)
	{
	  TprintfT (0, "__collector_write_string ERROR: can't map new buffer: %s\n", hndl->fname);
	  return 1;
	}

      /* write any data to the old block */
      if (blk_off > 0)
	{
	  TprintfT (DBG_LT2, "__collector_write_string partial writeBuffer[%d]: len=%ld, foff = %d '%s'\n",
		    indx, blksz - blk_off, blk_off, src);
	  writeBuffer (&hndl->buffers[indx], blk_off, src, blksz - blk_off);
	  src += blksz - blk_off;
	  len -= blksz - blk_off;
	}

      /* write data to the new blocks */
      for (ibuf = 0; ibuf < nbufs; ibuf++)
	{
	  int clen = blksz;
	  if (clen > len)
	    clen = len;
	  TprintfT (DBG_LT2, "__collector_write_string continue writeBuffer[%d]: len= %d, %s",
		    ibuf, clen, src);
	  writeBuffer (&hndl->buffers[buf_indices[ibuf]], 0, src, clen);
	  src += clen;
	  len -= clen;
	}
      break;
    }
  return 0;
}

