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
 *	Heap tracing events
 */

#include "config.h"
#include <dlfcn.h>

#include "gp-defs.h"
#include "collector.h"
#include "gp-experiment.h"
#include "data_pckts.h"
#include "tsd.h"

/* define the packets to be written out */
typedef struct Heap_packet
{ /* Malloc/free tracing packet */
  Common_packet comm;
  Heap_type  mtype;     /* subtype of packet */
  Size_type  size;      /* size of malloc/realloc request */
  Vaddr_type vaddr;     /* vaddr given to free or returned from malloc/realloc */
  Vaddr_type ovaddr;    /* Previous vaddr given to realloc */
} Heap_packet;

static int init_heap_intf ();
static int open_experiment (const char *);
static int start_data_collection (void);
static int stop_data_collection (void);
static int close_experiment (void);
static int detach_experiment (void);

static ModuleInterface module_interface = {
  SP_HEAPTRACE_FILE, /* description */
  NULL, /* initInterface */
  open_experiment, /* openExperiment */
  start_data_collection, /* startDataCollection */
  stop_data_collection, /* stopDataCollection */
  close_experiment, /* closeExperiment */
  detach_experiment /* detachExperiment (fork child) */
};

static CollectorInterface *collector_interface = NULL;
static int heap_mode = 0;
static CollectorModule heap_hndl = COLLECTOR_MODULE_ERR;
static unsigned heap_key = COLLECTOR_TSD_INVALID_KEY;

#define CHCK_REENTRANCE(x)  ( !heap_mode || ((x) = collector_interface->getKey( heap_key )) == NULL || (*(x) != 0) )
#define PUSH_REENTRANCE(x)  ((*(x))++)
#define POP_REENTRANCE(x)   ((*(x))--)
#define gethrtime collector_interface->getHiResTime

static void *(*__real_malloc)(size_t) = NULL;
static void (*__real_free)(void *);
static void *(*__real_realloc)(void *, size_t);
static void *(*__real_memalign)(size_t, size_t);
static void *(*__real_calloc)(size_t, size_t);
static void *(*__real_valloc)(size_t);
static char *(*__real_strchr)(const char *, int);

void *__libc_malloc (size_t);
void __libc_free (void *);
void *__libc_realloc (void *, size_t);

static void
collector_memset (void *s, int c, size_t n)
{
  unsigned char *s1 = s;
  while (n--)
    *s1++ = (unsigned char) c;
}

void
__collector_module_init (CollectorInterface *_collector_interface)
{
  if (_collector_interface == NULL)
    return;
  collector_interface = _collector_interface;
  Tprintf (0, "heaptrace: __collector_module_init\n");
  heap_hndl = collector_interface->registerModule (&module_interface);

  /* Initialize next module */
  ModuleInitFunc next_init = (ModuleInitFunc) dlsym (RTLD_NEXT, "__collector_module_init");
  if (next_init != NULL)
    next_init (_collector_interface);
  return;
}

static int
open_experiment (const char *exp)
{
  if (collector_interface == NULL)
    {
      Tprintf (0, "heaptrace: collector_interface is null.\n");
      return COL_ERROR_HEAPINIT;
    }
  if (heap_hndl == COLLECTOR_MODULE_ERR)
    {
      Tprintf (0, "heaptrace: handle create failed.\n");
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">data handle not created</event>\n",
				     SP_JCMD_CERROR, COL_ERROR_HEAPINIT);
      return COL_ERROR_HEAPINIT;
    }
  TprintfT (0, "heaptrace: open_experiment %s\n", exp);
  if (NULL_PTR (malloc))
    init_heap_intf ();

  const char *params = collector_interface->getParams ();
  while (params)
    {
      if ((params[0] == 'H') && (params[1] == ':'))
	{
	  params += 2;
	  break;
	}
      params = CALL_REAL (strchr)(params, ';');
      if (params)
	params++;
    }
  if (params == NULL)   /* Heap data collection not specified */
    return COL_ERROR_HEAPINIT;

  heap_key = collector_interface->createKey (sizeof ( int), NULL, NULL);
  if (heap_key == (unsigned) - 1)
    {
      Tprintf (0, "heaptrace: TSD key create failed.\n");
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">TSD key not created</event>\n",
				     SP_JCMD_CERROR, COL_ERROR_HEAPINIT);
      return COL_ERROR_HEAPINIT;
    }
  collector_interface->writeLog ("<profile name=\"%s\">\n", SP_JCMD_HEAPTRACE);
  collector_interface->writeLog ("  <profdata fname=\"%s\"/>\n",
				 module_interface.description);

  /* Record Heap_packet description */
  Heap_packet *pp = NULL;
  collector_interface->writeLog ("  <profpckt kind=\"%d\" uname=\"Heap tracing data\">\n", HEAP_PCKT);
  collector_interface->writeLog ("    <field name=\"LWPID\" uname=\"Lightweight process id\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->comm.lwp_id, sizeof (pp->comm.lwp_id) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"THRID\" uname=\"Thread number\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->comm.thr_id, sizeof (pp->comm.thr_id) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"CPUID\" uname=\"CPU id\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->comm.cpu_id, sizeof (pp->comm.cpu_id) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"TSTAMP\" uname=\"High resolution timestamp\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->comm.tstamp, sizeof (pp->comm.tstamp) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"FRINFO\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->comm.frinfo, sizeof (pp->comm.frinfo) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"HTYPE\" uname=\"Heap trace function type\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->mtype, sizeof (pp->mtype) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"HSIZE\" uname=\"Memory size\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->size, sizeof (pp->size) == 4 ? "UINT32" : "UINT64");
  collector_interface->writeLog ("    <field name=\"HVADDR\" uname=\"Memory address\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->vaddr, sizeof (pp->vaddr) == 4 ? "UINT32" : "UINT64");
  collector_interface->writeLog ("    <field name=\"HOVADDR\" uname=\"Previous memory address\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->ovaddr, sizeof (pp->ovaddr) == 4 ? "UINT32" : "UINT64");
  collector_interface->writeLog ("  </profpckt>\n");
  collector_interface->writeLog ("</profile>\n");
  return COL_ERROR_NONE;
}

static int
start_data_collection (void)
{
  heap_mode = 1;
  Tprintf (0, "heaptrace: start_data_collection\n");
  return 0;
}

static int
stop_data_collection (void)
{
  heap_mode = 0;
  Tprintf (0, "heaptrace: stop_data_collection\n");
  return 0;
}

static int
close_experiment (void)
{
  heap_mode = 0;
  heap_key = COLLECTOR_TSD_INVALID_KEY;
  Tprintf (0, "heaptrace: close_experiment\n");
  return 0;
}

static int
detach_experiment (void)
/* fork child.  Clean up state but don't write to experiment */
{
  heap_mode = 0;
  heap_key = COLLECTOR_TSD_INVALID_KEY;
  Tprintf (0, "heaptrace: detach_experiment\n");
  return 0;
}

static int in_init_heap_intf = 0; // Flag that we are in init_heap_intf()

static int
init_heap_intf ()
{
  void *dlflag;
  in_init_heap_intf = 1;
  __real_malloc = (void*(*)(size_t))dlsym (RTLD_NEXT, "malloc");
  if (__real_malloc == NULL)
    {
      /* We are probably dlopened after libthread/libc,
       * try to search in the previously loaded objects
       */
      __real_malloc = (void*(*)(size_t))dlsym (RTLD_DEFAULT, "malloc");
      if (__real_malloc == NULL)
	{
	  Tprintf (0, "heaptrace: ERROR: real malloc not found\n");
	  in_init_heap_intf = 0;
	  return 1;
	}
      Tprintf (DBG_LT1, "heaptrace: real malloc found with RTLD_DEFAULT\n");
      dlflag = RTLD_DEFAULT;
    }
  else
    {
      Tprintf (DBG_LT1, "heaptrace: real malloc found with RTLD_NEXT\n");
      dlflag = RTLD_NEXT;
    }
  __real_free = (void(*)(void *))dlsym (dlflag, "free");
  __real_realloc = (void*(*)(void *, size_t))dlsym (dlflag, "realloc");
  __real_memalign = (void*(*)(size_t, size_t))dlsym (dlflag, "memalign");
  __real_calloc = (void*(*)(size_t, size_t))dlsym (dlflag, "calloc");
  __real_valloc = (void*(*)(size_t))dlsym (dlflag, "valloc");
  __real_strchr = (char*(*)(const char *, int))dlsym (dlflag, "strchr");
  Tprintf (0, "heaptrace: init_heap_intf done\n");
  in_init_heap_intf = 0;
  return 0;
}

/*------------------------------------------------------------- malloc */

void *
malloc (size_t size)
{
  void *ret;
  int *guard;
  Heap_packet hpacket;
  /* Linux startup workaround  */
  if (!heap_mode)
    {
      void *ppp = (void *) __libc_malloc (size);
      Tprintf (DBG_LT4, "heaptrace: LINUX malloc(%ld, %p)...\n", (long) size, ppp);
      return ppp;
    }
  if (NULL_PTR (malloc))
    init_heap_intf ();
  if (CHCK_REENTRANCE (guard))
    {
      ret = (void *) CALL_REAL (malloc)(size);
      Tprintf (DBG_LT4, "heaptrace: real malloc(%ld) = %p\n", (long) size, ret);
      return ret;
    }
  PUSH_REENTRANCE (guard);

  ret = (void *) CALL_REAL (malloc)(size);
  collector_memset (&hpacket, 0, sizeof ( Heap_packet));
  hpacket.comm.tsize = sizeof ( Heap_packet);
  hpacket.comm.tstamp = gethrtime ();
  hpacket.mtype = MALLOC_TRACE;
  hpacket.size = (Size_type) size;
  hpacket.vaddr = (intptr_t) ret;
  hpacket.comm.frinfo = collector_interface->getFrameInfo (heap_hndl, hpacket.comm.tstamp, FRINFO_FROM_STACK, &hpacket);
  collector_interface->writeDataRecord (heap_hndl, (Common_packet*) & hpacket);
  POP_REENTRANCE (guard);
  return (void *) ret;
}

/*------------------------------------------------------------- free */

void
free (void *ptr)
{
  int *guard;
  Heap_packet hpacket;
  /* Linux startup workaround  */
  if (!heap_mode)
    {
      // Tprintf(DBG_LT4,"heaptrace: LINUX free(%p)...\n",ptr);
      __libc_free (ptr);
      return;
    }
  if (NULL_PTR (malloc))
    init_heap_intf ();
  if (CHCK_REENTRANCE (guard))
    {
      CALL_REAL (free)(ptr);
      return;
    }
  if (ptr == NULL)
    return;
  PUSH_REENTRANCE (guard);

  /* Get a timestamp before 'free' to enforce consistency */
  hrtime_t ts = gethrtime ();
  CALL_REAL (free)(ptr);
  collector_memset (&hpacket, 0, sizeof ( Heap_packet));
  hpacket.comm.tsize = sizeof ( Heap_packet);
  hpacket.comm.tstamp = ts;
  hpacket.mtype = FREE_TRACE;
  hpacket.vaddr = (intptr_t) ptr;
  hpacket.comm.frinfo = collector_interface->getFrameInfo (heap_hndl, hpacket.comm.tstamp, FRINFO_FROM_STACK, &hpacket);
  collector_interface->writeDataRecord (heap_hndl, (Common_packet*) & hpacket);
  POP_REENTRANCE (guard);
  return;
}

/*------------------------------------------------------------- realloc */
void *
realloc (void *ptr, size_t size)
{
  void *ret;
  int *guard;
  Heap_packet hpacket;

  /* Linux startup workaround  */
  if (!heap_mode)
    {
      void * ppp = (void *) __libc_realloc (ptr, size);
      Tprintf (DBG_LT4, "heaptrace: LINUX realloc(%ld, %p->%p)...\n",
	       (long) size, ptr, ppp);
      return ppp;
    }
  if (NULL_PTR (realloc))
    init_heap_intf ();
  if (CHCK_REENTRANCE (guard))
    {
      ret = (void *) CALL_REAL (realloc)(ptr, size);
      return ret;
    }
  PUSH_REENTRANCE (guard);
  hrtime_t ts = gethrtime ();
  ret = (void *) CALL_REAL (realloc)(ptr, size);
  collector_memset (&hpacket, 0, sizeof ( Heap_packet));
  hpacket.comm.tsize = sizeof ( Heap_packet);
  hpacket.comm.tstamp = ts;
  hpacket.mtype = REALLOC_TRACE;
  hpacket.size = (Size_type) size;
  hpacket.vaddr = (intptr_t) ret;
  hpacket.comm.frinfo = collector_interface->getFrameInfo (heap_hndl, hpacket.comm.tstamp, FRINFO_FROM_STACK, &hpacket);
  collector_interface->writeDataRecord (heap_hndl, (Common_packet*) & hpacket);
  POP_REENTRANCE (guard);
  return (void *) ret;
}

/*------------------------------------------------------------- memalign */
void *
memalign (size_t align, size_t size)
{
  void *ret;
  int *guard;
  Heap_packet hpacket;
  if (NULL_PTR (memalign))
    init_heap_intf ();
  if (CHCK_REENTRANCE (guard))
    {
      ret = (void *) CALL_REAL (memalign)(align, size);
      return ret;
    }
  PUSH_REENTRANCE (guard);
  ret = (void *) CALL_REAL (memalign)(align, size);
  collector_memset (&hpacket, 0, sizeof ( Heap_packet));
  hpacket.comm.tsize = sizeof ( Heap_packet);
  hpacket.comm.tstamp = gethrtime ();
  hpacket.mtype = MALLOC_TRACE;
  hpacket.size = (Size_type) size;
  hpacket.vaddr = (intptr_t) ret;
  hpacket.comm.frinfo = collector_interface->getFrameInfo (heap_hndl, hpacket.comm.tstamp, FRINFO_FROM_STACK, &hpacket);
  collector_interface->writeDataRecord (heap_hndl, (Common_packet*) & hpacket);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- valloc */

void *
valloc (size_t size)
{
  void *ret;
  int *guard;
  Heap_packet hpacket;
  if (NULL_PTR (memalign))
    init_heap_intf ();
  if (CHCK_REENTRANCE (guard))
    {
      ret = (void *) CALL_REAL (valloc)(size);
      return ret;
    }
  PUSH_REENTRANCE (guard);
  ret = (void *) CALL_REAL (valloc)(size);
  collector_memset (&hpacket, 0, sizeof ( Heap_packet));
  hpacket.comm.tsize = sizeof ( Heap_packet);
  hpacket.comm.tstamp = gethrtime ();
  hpacket.mtype = MALLOC_TRACE;
  hpacket.size = (Size_type) size;
  hpacket.vaddr = (intptr_t) ret;
  hpacket.comm.frinfo = collector_interface->getFrameInfo (heap_hndl, hpacket.comm.tstamp, FRINFO_FROM_STACK, &hpacket);
  collector_interface->writeDataRecord (heap_hndl, (Common_packet*) & hpacket);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- calloc */
void *
calloc (size_t size, size_t esize)
{
  void *ret;
  int *guard;
  Heap_packet hpacket;
  if (NULL_PTR (calloc))
    {
      if (in_init_heap_intf != 0)
	return NULL; // Terminate infinite loop
      init_heap_intf ();
    }
  if (CHCK_REENTRANCE (guard))
    {
      ret = (void *) CALL_REAL (calloc)(size, esize);
      return ret;
    }
  PUSH_REENTRANCE (guard);
  ret = (void *) CALL_REAL (calloc)(size, esize);
  collector_memset (&hpacket, 0, sizeof ( Heap_packet));
  hpacket.comm.tsize = sizeof ( Heap_packet);
  hpacket.comm.tstamp = gethrtime ();
  hpacket.mtype = MALLOC_TRACE;
  hpacket.size = (Size_type) (size * esize);
  hpacket.vaddr = (intptr_t) ret;
  hpacket.comm.frinfo = collector_interface->getFrameInfo (heap_hndl, hpacket.comm.tstamp, FRINFO_FROM_STACK, &hpacket);
  collector_interface->writeDataRecord (heap_hndl, (Common_packet*) & hpacket);
  POP_REENTRANCE (guard);
  return ret;
}

/* __collector_heap_record is used to record java allocations/deallocations.
 * It uses the same facilities as regular heap tracing for now.
 */
void
__collector_heap_record (int mtype, size_t size, void *vaddr)
{
  int *guard;
  Heap_packet hpacket;
  if (CHCK_REENTRANCE (guard))
    return;
  PUSH_REENTRANCE (guard);
  collector_memset (&hpacket, 0, sizeof ( Heap_packet));
  hpacket.comm.tsize = sizeof ( Heap_packet);
  hpacket.comm.tstamp = gethrtime ();
  hpacket.mtype = mtype;
  hpacket.size = (Size_type) size;
  hpacket.vaddr = (intptr_t) vaddr;
  hpacket.comm.frinfo = collector_interface->getFrameInfo (heap_hndl, hpacket.comm.tstamp, FRINFO_FROM_STACK, &hpacket);
  collector_interface->writeDataRecord (heap_hndl, (Common_packet*) & hpacket);
  POP_REENTRANCE (guard);
  return;
}
