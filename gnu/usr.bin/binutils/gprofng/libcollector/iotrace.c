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
 *	IO events
 */
#include "config.h"
#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>

// create() and others are defined in fcntl.h.
// Our 'create' should not have the __nonnull attribute
#undef __nonnull
#define __nonnull(x)
#include <fcntl.h>

#include "gp-defs.h"
#include "collector.h"
#include "gp-experiment.h"
#include "tsd.h"


/* define the packet that will be written out */
typedef struct IOTrace_packet
{ /* IO tracing packet */
  Common_packet comm;
  IOTrace_type iotype;      /* IO type */
  int32_t fd;               /* file descriptor */
  Size_type nbyte;          /* number of bytes */
  hrtime_t requested;       /* time of IO requested */
  int32_t ofd;              /* original file descriptor */
  FileSystem_type fstype;   /* file system type */
  char fname;               /* file name */
} IOTrace_packet;

typedef long long offset_t;

static int open_experiment (const char *);
static int start_data_collection (void);
static int stop_data_collection (void);
static int close_experiment (void);
static int detach_experiment (void);
static int init_io_intf ();

static ModuleInterface module_interface ={
  SP_IOTRACE_FILE,          /* description */
  NULL,                     /* initInterface */
  open_experiment,          /* openExperiment */
  start_data_collection,    /* startDataCollection */
  stop_data_collection,     /* stopDataCollection */
  close_experiment,         /* closeExperiment */
  detach_experiment         /* detachExperiment (fork child) */
};

static CollectorInterface *collector_interface = NULL;
static struct Heap *io_heap = NULL;
static int io_mode = 0;
static CollectorModule io_hndl = COLLECTOR_MODULE_ERR;
static unsigned io_key = COLLECTOR_TSD_INVALID_KEY;

#define CHCK_REENTRANCE(x)   (!io_mode || ((x) = collector_interface->getKey( io_key )) == NULL || (*(x) != 0))
#define RECHCK_REENTRANCE(x) (!io_mode || ((x) = collector_interface->getKey( io_key )) == NULL || (*(x) == 0))
#define PUSH_REENTRANCE(x)   ((*(x))++)
#define POP_REENTRANCE(x)    ((*(x))--)
#define gethrtime collector_interface->getHiResTime


/* interposition function handles */
static int (*__real_open)(const char *path, int oflag, ...) = NULL;
static int (*__real_fcntl)(int fildes, int cmd, ...) = NULL;
static int (*__real_openat)(int fildes, const char *path, int oflag, ...) = NULL;
static int (*__real_close)(int fildes) = NULL;
static FILE *(*__real_fopen)(const char *filename, const char *mode) = NULL;
static int (*__real_fclose)(FILE *stream) = NULL;
static int (*__real_dup)(int fildes) = NULL;
static int (*__real_dup2)(int fildes, int fildes2) = NULL;
static int (*__real_pipe)(int fildes[2]) = NULL;
static int (*__real_socket)(int domain, int type, int protocol) = NULL;
static int (*__real_mkstemp)(char *template) = NULL;
static int (*__real_mkstemps)(char *template, int slen) = NULL;
static int (*__real_creat)(const char *path, mode_t mode) = NULL;
static FILE *(*__real_fdopen)(int fildes, const char *mode) = NULL;
static ssize_t (*__real_read)(int fildes, void *buf, size_t nbyte) = NULL;
static ssize_t (*__real_write)(int fildes, const void *buf, size_t nbyte) = NULL;
static ssize_t (*__real_readv)(int fildes, const struct iovec *iov, int iovcnt) = NULL;
static ssize_t (*__real_writev)(int fildes, const struct iovec *iov, int iovcnt) = NULL;
static size_t (*__real_fread)(void *ptr, size_t size, size_t nitems, FILE *stream) = NULL;
static size_t (*__real_fwrite)(const void *ptr, size_t size, size_t nitems, FILE *stream) = NULL;
static ssize_t (*__real_pread)(int fildes, void *buf, size_t nbyte, off_t offset) = NULL;
static ssize_t (*__real_pwrite)(int fildes, const void *buf, size_t nbyte, off_t offset) = NULL;
static ssize_t (*__real_pwrite64)(int fildes, const void *buf, size_t nbyte, off64_t offset) = NULL;
static char *(*__real_fgets)(char *s, int n, FILE *stream) = NULL;
static int (*__real_fputs)(const char *s, FILE *stream) = NULL;
static int (*__real_fputc)(int c, FILE *stream) = NULL;
static int (*__real_fprintf)(FILE *stream, const char *format, ...) = NULL;
static int (*__real_vfprintf)(FILE *stream, const char *format, va_list ap) = NULL;
static off_t (*__real_lseek)(int fildes, off_t offset, int whence) = NULL;
static offset_t (*__real_llseek)(int fildes, offset_t offset, int whence) = NULL;
static int (*__real_chmod)(const char *path, mode_t mode) = NULL;
static int (*__real_access)(const char *path, int amode) = NULL;
static int (*__real_rename)(const char *old, const char *new) = NULL;
static int (*__real_mkdir)(const char *path, mode_t mode) = NULL;
static int (*__real_getdents)(int fildes, struct dirent *buf, size_t nbyte) = NULL;
static int (*__real_unlink)(const char *path) = NULL;
static int (*__real_fseek)(FILE *stream, long offset, int whence) = NULL;
static void (*__real_rewind)(FILE *stream) = NULL;
static long (*__real_ftell)(FILE *stream) = NULL;
static int (*__real_fgetpos)(FILE *stream, fpos_t *pos) = NULL;
static int (*__real_fsetpos)(FILE *stream, const fpos_t *pos) = NULL;
static int (*__real_fsync)(int fildes) = NULL;
static struct dirent *(*__real_readdir)(DIR *dirp) = NULL;
static int (*__real_flock)(int fd, int operation) = NULL;
static int (*__real_lockf)(int fildes, int function, off_t size) = NULL;
static int (*__real_fflush)(FILE *stream) = NULL;
static int (*__real_open64)(const char *path, int oflag, ...) = NULL;
static int (*__real_open64_2_2)(const char *path, int oflag, ...) = NULL;
static int (*__real_creat64)(const char *path, mode_t mode) = NULL;
static int (*__real_fgetpos64)(FILE *stream, fpos64_t *pos) = NULL;
static int (*__real_fsetpos64)(FILE *stream, const fpos64_t *pos) = NULL;
static FILE *(*__real_fopen_2_17)(const char *filename, const char *mode) = NULL;
static FILE *(*__real_fopen_2_2_5)(const char *filename, const char *mode) = NULL;
static FILE *(*__real_fopen_2_1)(const char *filename, const char *mode) = NULL;
static FILE *(*__real_fopen_2_0)(const char *filename, const char *mode) = NULL;
static int (*__real_fclose_2_17)(FILE *stream) = NULL;
static int (*__real_fclose_2_2_5)(FILE *stream) = NULL;
static int (*__real_fclose_2_1)(FILE *stream) = NULL;
static int (*__real_fclose_2_0)(FILE *stream) = NULL;
static FILE *(*__real_fdopen_2_17)(int fildes, const char *mode) = NULL;
static FILE *(*__real_fdopen_2_2_5)(int fildes, const char *mode) = NULL;
static FILE *(*__real_fdopen_2_1)(int fildes, const char *mode) = NULL;
static FILE *(*__real_fdopen_2_0)(int fildes, const char *mode) = NULL;
static int (*__real_fgetpos_2_17)(FILE *stream, fpos_t *pos) = NULL;
static int (*__real_fgetpos_2_2_5)(FILE *stream, fpos_t *pos) = NULL;
static int (*__real_fgetpos_2_2)(FILE *stream, fpos_t *pos) = NULL;
static int (*__real_fgetpos_2_0)(FILE *stream, fpos_t *pos) = NULL;
static int (*__real_fsetpos_2_17)(FILE *stream, const fpos_t *pos) = NULL;
static int (*__real_fsetpos_2_2_5)(FILE *stream, const fpos_t *pos) = NULL;
static int (*__real_fsetpos_2_2)(FILE *stream, const fpos_t *pos) = NULL;
static int (*__real_fsetpos_2_0)(FILE *stream, const fpos_t *pos) = NULL;
static ssize_t (*__real_pread_2_2)(int fildes, void *buf, size_t nbyte, off_t offset) = NULL;
static ssize_t (*__real_pwrite_2_2)(int fildes, const void *buf, size_t nbyte, off_t offset) = NULL;
static int (*__real_fgetpos64_2_17)(FILE *stream, fpos64_t *pos) = NULL;
static int (*__real_fgetpos64_2_2_5)(FILE *stream, fpos64_t *pos) = NULL;
static int (*__real_fgetpos64_2_2)(FILE *stream, fpos64_t *pos) = NULL;
static int (*__real_fgetpos64_2_1)(FILE *stream, fpos64_t *pos) = NULL;
static int (*__real_fsetpos64_2_17)(FILE *stream, const fpos64_t *pos) = NULL;
static int (*__real_fsetpos64_2_2_5)(FILE *stream, const fpos64_t *pos) = NULL;
static int (*__real_fsetpos64_2_2)(FILE *stream, const fpos64_t *pos) = NULL;
static int (*__real_fsetpos64_2_1)(FILE *stream, const fpos64_t *pos) = NULL;
static ssize_t (*__real_pwrite64_2_2)(int fildes, const void *buf, size_t nbyte, off64_t offset) = NULL;

static int
collector_align_pktsize (int sz)
{
  int pktSize = sz;
  if (sz <= 0)
    return sz;
  if ((sz % 8) != 0)
    {
      pktSize = (sz / 8) + 1;
      pktSize *= 8;
    }
  return pktSize;
}

static void
collector_memset (void *s, int c, size_t n)
{
  unsigned char *s1 = s;
  while (n--)
    *s1++ = (unsigned char) c;
}

static size_t
collector_strlen (const char *s)
{
  if (s == NULL)
    return 0;
  int len = -1;
  while (s[++len] != '\0')
    ;
  return len;
}

static size_t
collector_strncpy (char *dst, const char *src, size_t dstsize)
{
  size_t i;
  for (i = 0; i < dstsize; i++)
    {
      dst[i] = src[i];
      if (src[i] == '\0')
	break;
    }
  return i;
}

static char *
collector_strchr (const char *s, int c)
{
  do
    {
      if (*s == (char) c)
	return ((char *) s);
    }
  while (*s++);
  return (NULL);
}

static FileSystem_type
collector_fstype (const char *path)
{
  return UNKNOWNFS_TYPE;
}

void
__collector_module_init (CollectorInterface *_collector_interface)
{
  if (_collector_interface == NULL)
    return;
  collector_interface = _collector_interface;
  Tprintf (0, "iotrace: __collector_module_init\n");
  io_hndl = collector_interface->registerModule (&module_interface);
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
      Tprintf (0, "iotrace: collector_interface is null.\n");
      return COL_ERROR_IOINIT;
    }
  if (io_hndl == COLLECTOR_MODULE_ERR)
    {
      Tprintf (0, "iotrace: handle create failed.\n");
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">data handle not created</event>\n",
				     SP_JCMD_CERROR, COL_ERROR_IOINIT);
      return COL_ERROR_IOINIT;
    }
  TprintfT (0, "iotrace: open_experiment %s\n", exp);
  if (NULL_PTR (fopen))
    init_io_intf ();
  if (io_heap == NULL)
    {
      io_heap = collector_interface->newHeap ();
      if (io_heap == NULL)
	{
	  Tprintf (0, "iotrace: new heap failed.\n");
	  collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">new iotrace heap not created</event>\n",
					 SP_JCMD_CERROR, COL_ERROR_IOINIT);
	  return COL_ERROR_IOINIT;
	}
    }

  const char *params = collector_interface->getParams ();
  while (params)
    {
      if ((params[0] == 'i') && (params[1] == ':'))
	{
	  params += 2;
	  break;
	}
      params = collector_strchr (params, ';');
      if (params)
	params++;
    }
  if (params == NULL)  /* IO data collection not specified */
    return COL_ERROR_IOINIT;

  io_key = collector_interface->createKey (sizeof ( int), NULL, NULL);
  if (io_key == (unsigned) - 1)
    {
      Tprintf (0, "iotrace: TSD key create failed.\n");
      collector_interface->writeLog ("<event kind=\"%s\" id=\"%d\">TSD key not created</event>\n",
				     SP_JCMD_CERROR, COL_ERROR_IOINIT);
      return COL_ERROR_IOINIT;
    }

  collector_interface->writeLog ("<profile name=\"%s\">\n", SP_JCMD_IOTRACE);
  collector_interface->writeLog ("  <profdata fname=\"%s\"/>\n",
				 module_interface.description);
  /* Record IOTrace_packet description */
  IOTrace_packet *pp = NULL;
  collector_interface->writeLog ("  <profpckt kind=\"%d\" uname=\"IO tracing data\">\n", IOTRACE_PCKT);
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
  collector_interface->writeLog ("    <field name=\"IOTYPE\" uname=\"IO trace function type\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->iotype, sizeof (pp->iotype) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"IOFD\" uname=\"File descriptor\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->fd, sizeof (pp->fd) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"IONBYTE\" uname=\"Number of bytes\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->nbyte, sizeof (pp->nbyte) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"IORQST\" uname=\"Time of IO requested\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->requested, sizeof (pp->requested) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"IOOFD\" uname=\"Original file descriptor\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->ofd, sizeof (pp->ofd) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"IOFSTYPE\" uname=\"File system type\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->fstype, sizeof (pp->fstype) == 4 ? "INT32" : "INT64");
  collector_interface->writeLog ("    <field name=\"IOFNAME\" uname=\"File name\" offset=\"%d\" type=\"%s\"/>\n",
				 &pp->fname, "STRING");
  collector_interface->writeLog ("  </profpckt>\n");
  collector_interface->writeLog ("</profile>\n");
  return COL_ERROR_NONE;
}

static int
start_data_collection (void)
{
  io_mode = 1;
  Tprintf (0, "iotrace: start_data_collection\n");
  return 0;
}

static int
stop_data_collection (void)
{
  io_mode = 0;
  Tprintf (0, "iotrace: stop_data_collection\n");
  return 0;
}

static int
close_experiment (void)
{
  io_mode = 0;
  io_key = COLLECTOR_TSD_INVALID_KEY;
  if (io_heap != NULL)
    {
      collector_interface->deleteHeap (io_heap);
      io_heap = NULL;
    }
  Tprintf (0, "iotrace: close_experiment\n");
  return 0;
}

static int
detach_experiment (void)
{
  /* fork child.  Clean up state but don't write to experiment */
  io_mode = 0;
  io_key = COLLECTOR_TSD_INVALID_KEY;
  if (io_heap != NULL)
    {
      collector_interface->deleteHeap (io_heap);
      io_heap = NULL;
    }
  Tprintf (0, "iotrace: detach_experiment\n");
  return 0;
}

static int
init_fopen (void *dlflag)
{
  __real_fopen_2_17 = dlvsym (dlflag, "fopen", "GLIBC_2.17");
  __real_fopen_2_2_5 = dlvsym (dlflag, "fopen", "GLIBC_2.2.5");
  __real_fopen_2_1 = dlvsym (dlflag, "fopen", "GLIBC_2.1");
  __real_fopen_2_0 = dlvsym (dlflag, "fopen", "GLIBC_2.0");
  if (__real_fopen_2_17)
    __real_fopen = __real_fopen_2_17;
  else if (__real_fopen_2_2_5)
    __real_fopen = __real_fopen_2_2_5;
  else if (__real_fopen_2_1)
    __real_fopen = __real_fopen_2_1;
  else if (__real_fopen_2_0)
    __real_fopen = __real_fopen_2_0;
  else 
    __real_fopen = dlsym (dlflag, "fopen");
  return __real_fopen ? 1 : 0;
}

static int
init_io_intf ()
{
  void *dlflag;
  int rc = 0;
  /* if we detect recursion/reentrance, SEGV so we can get a stack */
  static int init_io_intf_started;
  static int init_io_intf_finished;
  init_io_intf_started++;
  if (!init_io_intf_finished && init_io_intf_started >= 3)
    {
      /* pull the plug if recursion occurs... */
      abort ();
    }

  /* lookup fprint to print fatal error message */
  void *ptr = dlsym (RTLD_NEXT, "fprintf");
  if (ptr)
    __real_fprintf = (int (*)(FILE*, const char*, ...)) ptr;
  else
    abort ();

  dlflag = RTLD_NEXT;
  if (init_fopen (dlflag) == 0)
    {
      if (init_fopen (RTLD_DEFAULT))
	dlflag = RTLD_DEFAULT;
      else
	{
	  CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fopen\n");
	  rc = COL_ERROR_IOINIT;
	}
    }

  __real_fgetpos64_2_17 = dlvsym (dlflag, "fgetpos64", "GLIBC_2.17");
  __real_fgetpos64_2_2_5 = dlvsym (dlflag, "fgetpos64", "GLIBC_2.2.5");
  __real_fgetpos64_2_2 = dlvsym (dlflag, "fgetpos64", "GLIBC_2.2");
  __real_fgetpos64_2_1 = dlvsym (dlflag, "fgetpos64", "GLIBC_2.1");
  if (__real_fgetpos64_2_17)
    __real_fgetpos64 = __real_fgetpos64_2_17;
  else if (__real_fgetpos64_2_2_5)
    __real_fgetpos64 = __real_fgetpos64_2_2_5;
  else if (__real_fgetpos64_2_2)
    __real_fgetpos64 = __real_fgetpos64_2_2;
  else if (__real_fgetpos64_2_1)
    __real_fgetpos64 = __real_fgetpos64_2_1;
  else
    __real_fgetpos64 = dlsym (dlflag, "fgetpos64");

  __real_fsetpos64_2_17 = dlvsym (dlflag, "fsetpos64", "GLIBC_2.17");
  __real_fsetpos64_2_2_5 = dlvsym (dlflag, "fsetpos64", "GLIBC_2.2.5");
  __real_fsetpos64_2_2 = dlvsym (dlflag, "fsetpos64", "GLIBC_2.2");
  __real_fsetpos64_2_1 = dlvsym (dlflag, "fsetpos64", "GLIBC_2.1");
  if (__real_fsetpos64_2_17)
    __real_fsetpos64 = __real_fsetpos64_2_17;
  else if (__real_fsetpos64_2_2_5)
    __real_fsetpos64 = __real_fsetpos64_2_2_5;
  else if (__real_fsetpos64_2_2)
    __real_fsetpos64 = __real_fsetpos64_2_2;
  else if (__real_fsetpos64_2_1)
    __real_fsetpos64 = __real_fsetpos64_2_1;
  else
    __real_fsetpos64 = dlsym (dlflag, "fsetpos64");

  __real_pread_2_2 = dlvsym (dlflag, "pread", "GLIBC_2.2");
  if (__real_pread_2_2)
    __real_pread = __real_pread_2_2;
  else
    __real_pread = dlsym (dlflag, "pread");
  if (__real_pread == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT pread\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_pwrite_2_2 = dlvsym (dlflag, "pwrite", "GLIBC_2.2");
  if (__real_pwrite_2_2)
    __real_pwrite = __real_pwrite_2_2;
  else
    __real_pwrite = dlsym (dlflag, "pwrite");
  if (__real_pwrite == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT pwrite\n");
      rc = COL_ERROR_IOINIT;
    }


  __real_pwrite64_2_2 = dlvsym (dlflag, "pwrite64", "GLIBC_2.2");
  if (__real_pwrite64_2_2)
    __real_pwrite64 = __real_pwrite64_2_2;
  else
    __real_pwrite64 = dlsym (dlflag, "pwrite64");

  __real_fclose_2_17 = dlvsym (dlflag, "fclose", "GLIBC_2.17");
  __real_fclose_2_2_5 = dlvsym (dlflag, "fclose", "GLIBC_2.2.5");
  __real_fclose_2_1 = dlvsym (dlflag, "fclose", "GLIBC_2.1");
  __real_fclose_2_0 = dlvsym (dlflag, "fclose", "GLIBC_2.0");
  if (__real_fclose_2_17)
    __real_fclose = __real_fclose_2_17;
  else if (__real_fclose_2_2_5)
    __real_fclose = __real_fclose_2_2_5;
  else if (__real_fclose_2_1)
    __real_fclose = __real_fclose_2_1;
  else if (__real_fclose_2_0)
    __real_fclose = __real_fclose_2_0;
  else
    __real_fclose = dlsym (dlflag, "fclose");
  if (__real_fclose == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fclose\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fdopen_2_17 = dlvsym (dlflag, "fdopen", "GLIBC_2.17");
  __real_fdopen_2_2_5 = dlvsym (dlflag, "fdopen", "GLIBC_2.2.5");
  __real_fdopen_2_1 = dlvsym (dlflag, "fdopen", "GLIBC_2.1");
  __real_fdopen_2_0 = dlvsym (dlflag, "fdopen", "GLIBC_2.0");
  if (__real_fdopen_2_17)
    __real_fdopen = __real_fdopen_2_17;
  else if (__real_fdopen_2_2_5)
    __real_fdopen = __real_fdopen_2_2_5;
  else if (__real_fdopen_2_1)
    __real_fdopen = __real_fdopen_2_1;
  else if (__real_fdopen_2_0)
    __real_fdopen = __real_fdopen_2_0;
  else
    __real_fdopen = dlsym (dlflag, "fdopen");
  if (__real_fdopen == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fdopen\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fgetpos_2_17 = dlvsym (dlflag, "fgetpos", "GLIBC_2.17");
  __real_fgetpos_2_2_5 = dlvsym (dlflag, "fgetpos", "GLIBC_2.2.5");
  __real_fgetpos_2_2 = dlvsym (dlflag, "fgetpos", "GLIBC_2.2");
  __real_fgetpos_2_0 = dlvsym (dlflag, "fgetpos", "GLIBC_2.0");
  if (__real_fgetpos_2_17)
    __real_fgetpos = __real_fgetpos_2_17;
  else if (__real_fgetpos_2_2_5)
    __real_fgetpos = __real_fgetpos_2_2_5;
  else if (__real_fgetpos_2_2)
    __real_fgetpos = __real_fgetpos_2_2;
  else if (__real_fgetpos_2_0)
    __real_fgetpos = __real_fgetpos_2_0;
  else
    __real_fgetpos = dlsym (dlflag, "fgetpos");
  if (__real_fgetpos == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fgetpos\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fsetpos_2_17 = dlvsym (dlflag, "fsetpos", "GLIBC_2.17");
  __real_fsetpos_2_2_5 = dlvsym (dlflag, "fsetpos", "GLIBC_2.2.5");
  __real_fsetpos_2_2 = dlvsym (dlflag, "fsetpos", "GLIBC_2.2");
  __real_fsetpos_2_0 = dlvsym (dlflag, "fsetpos", "GLIBC_2.0");
  if (__real_fsetpos_2_17)
    __real_fsetpos = __real_fsetpos_2_17;
  else if (__real_fsetpos_2_2_5)
    __real_fsetpos = __real_fsetpos_2_2_5;
  else if (__real_fsetpos_2_2)
    __real_fsetpos = __real_fsetpos_2_2;
  else if (__real_fsetpos_2_0)
    __real_fsetpos = __real_fsetpos_2_0;
  else
    __real_fsetpos = dlsym (dlflag, "fsetpos");
  if (__real_fsetpos == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fsetpos\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_open = (int (*)(const char*, int, ...))dlsym (dlflag, "open");
  if (__real_open == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT open\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_open64_2_2 = dlvsym (dlflag, "open64", "GLIBC_2.2");
  if (__real_open64_2_2)
    __real_open64 = __real_open64_2_2;
  else
    __real_open64 = dlsym (dlflag, "open64");

  __real_fcntl = (int (*)(int, int, ...))dlsym (dlflag, "fcntl");
  if (__real_fcntl == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fcntl\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_openat = (int (*)(int, const char*, int, ...))dlsym (dlflag, "openat");
  if (__real_openat == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT openat\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_close = (int (*)(int))dlsym (dlflag, "close");
  if (__real_close == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT close\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_dup = (int (*)(int))dlsym (dlflag, "dup");
  if (__real_dup == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT dup\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_dup2 = (int (*)(int, int))dlsym (dlflag, "dup2");
  if (__real_dup2 == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT dup2\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_pipe = (int (*)(int[]))dlsym (dlflag, "pipe");
  if (__real_pipe == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT pipe\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_socket = (int (*)(int, int, int))dlsym (dlflag, "socket");
  if (__real_socket == NULL)
    {
      __real_socket = (int (*)(int, int, int))dlsym (RTLD_NEXT, "socket");
      if (__real_socket == NULL)
	{
#if 0
	  CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERXXX_IOINIT socket\n");
	  rc = COL_ERROR_IOINIT;
#endif
	}
    }

  __real_mkstemp = (int (*)(char*))dlsym (dlflag, "mkstemp");
  if (__real_mkstemp == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT mkstemp\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_mkstemps = (int (*)(char*, int))dlsym (dlflag, "mkstemps");
  if (__real_mkstemps == NULL)
    {
#if 0
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERXXX_IOINIT mkstemps\n");
      rc = COL_ERROR_IOINIT;
#endif
    }

  __real_creat = (int (*)(const char*, mode_t))dlsym (dlflag, "creat");
  if (__real_creat == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT creat\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_creat64 = (int (*)(const char*, mode_t))dlsym (dlflag, "creat64");
  if (__real_creat64 == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT creat64\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_read = (ssize_t (*)(int, void*, size_t))dlsym (dlflag, "read");
  if (__real_read == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT read\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_write = (ssize_t (*)(int, const void*, size_t))dlsym (dlflag, "write");
  if (__real_write == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT write\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_readv = (ssize_t (*)(int, const struct iovec*, int))dlsym (dlflag, "readv");
  if (__real_readv == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT readv\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_writev = (ssize_t (*)(int, const struct iovec*, int))dlsym (dlflag, "writev");
  if (__real_writev == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT writev\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fread = (size_t (*)(void*, size_t, size_t, FILE*))dlsym (dlflag, "fread");
  if (__real_fread == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fread\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fwrite = (size_t (*)(const void*, size_t, size_t, FILE*))dlsym (dlflag, "fwrite");
  if (__real_fwrite == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fwrite\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_pread = (ssize_t (*)(int, void*, size_t, off_t))dlsym (dlflag, "pread");
  if (__real_pread == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT pread\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_pwrite = (ssize_t (*)(int, const void*, size_t, off_t))dlsym (dlflag, "pwrite");
  if (__real_pwrite == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT pwrite\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_pwrite64 = (ssize_t (*)(int, const void*, size_t, off64_t))dlsym (dlflag, "pwrite64");
  if (__real_pwrite64 == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT pwrite64\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fgets = (char* (*)(char*, int, FILE*))dlsym (dlflag, "fgets");
  if (__real_fgets == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fgets\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fputs = (int (*)(const char*, FILE*))dlsym (dlflag, "fputs");
  if (__real_fputs == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fputs\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fputc = (int (*)(int, FILE*))dlsym (dlflag, "fputc");
  if (__real_fputc == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fputc\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_vfprintf = (int (*)(FILE*, const char*, va_list))dlsym (dlflag, "vfprintf");
  if (__real_vfprintf == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT vfprintf\n");
      rc = COL_ERROR_IOINIT;
    }


  __real_lseek = (off_t (*)(int, off_t, int))dlsym (dlflag, "lseek");
  if (__real_lseek == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT lseek\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_llseek = (offset_t (*)(int, offset_t, int))dlsym (dlflag, "llseek");
  if (__real_llseek == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT llseek\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_chmod = (int (*)(const char*, mode_t))dlsym (dlflag, "chmod");
  if (__real_chmod == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT chmod\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_access = (int (*)(const char*, int))dlsym (dlflag, "access");
  if (__real_access == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT access\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_rename = (int (*)(const char*, const char*))dlsym (dlflag, "rename");
  if (__real_rename == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT rename\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_mkdir = (int (*)(const char*, mode_t))dlsym (dlflag, "mkdir");
  if (__real_mkdir == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT mkdir\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_getdents = (int (*)(int, struct dirent*, size_t))dlsym (dlflag, "getdents");
  if (__real_getdents == NULL)
    {
#if 0
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERXXX_IOINIT getdents\n");
      rc = COL_ERROR_IOINIT;
#endif
    }

  __real_unlink = (int (*)(const char*))dlsym (dlflag, "unlink");
  if (__real_unlink == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT unlink\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fseek = (int (*)(FILE*, long, int))dlsym (dlflag, "fseek");
  if (__real_fseek == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fseek\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_rewind = (void (*)(FILE*))dlsym (dlflag, "rewind");
  if (__real_rewind == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT rewind\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_ftell = (long (*)(FILE*))dlsym (dlflag, "ftell");
  if (__real_ftell == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT ftell\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fsync = (int (*)(int))dlsym (dlflag, "fsync");
  if (__real_fsync == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fsync\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_readdir = (struct dirent * (*)(DIR*))dlsym (dlflag, "readdir");
  if (__real_readdir == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT readdir\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_flock = (int (*)(int, int))dlsym (dlflag, "flock");
  if (__real_flock == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT flock\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_lockf = (int (*)(int, int, off_t))dlsym (dlflag, "lockf");
  if (__real_lockf == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT lockf\n");
      rc = COL_ERROR_IOINIT;
    }

  __real_fflush = (int (*)(FILE*))dlsym (dlflag, "fflush");
  if (__real_fflush == NULL)
    {
      CALL_REAL (fprintf)(stderr, "iotrace_init COL_ERROR_IOINIT fflush\n");
      rc = COL_ERROR_IOINIT;
    }

  init_io_intf_finished++;
  return rc;
}

static void
write_io_packet (int fd, ssize_t ret, hrtime_t reqt, int iotype)
{
  IOTrace_packet iopkt;
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = gethrtime ();
  iopkt.requested = reqt;
  iopkt.iotype = iotype;
  iopkt.fd = fd;
  iopkt.nbyte = ret;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl,
			iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
}

/*------------------------------------------------------------- open */
int
open (const char *path, int oflag, ...)
{
  int *guard;
  int fd;
  void *packet;
  IOTrace_packet *iopkt;
  mode_t mode;
  va_list ap;
  size_t sz;
  unsigned pktSize;

  va_start (ap, oflag);
  mode = va_arg (ap, mode_t);
  va_end (ap);

  if (NULL_PTR (open))
    init_io_intf ();

  if (CHCK_REENTRANCE (guard) || path == NULL)
    return CALL_REAL (open)(path, oflag, mode);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fd = CALL_REAL (open)(path, oflag, mode);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (path);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (fd != -1)
	iopkt->iotype = OPEN_TRACE;
      else
	iopkt->iotype = OPEN_TRACE_ERROR;
      iopkt->fd = fd;
      iopkt->fstype = collector_fstype (path);
      collector_strncpy (&(iopkt->fname), path, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: open cannot allocate memory\n");
      return -1;
    }
  POP_REENTRANCE (guard);
  return fd;
}

/*------------------------------------------------------------- open64 */
static int
gprofng_open64 (int(real_open64) (const char *, int, ...),
		const char *path, int oflag, mode_t mode)
{
  int *guard;
  int fd;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;
  if (CHCK_REENTRANCE (guard) || path == NULL)
    return real_open64 (path, oflag, mode);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fd = real_open64 (path, oflag, mode);
  if (RECHCK_REENTRANCE (guard) || path == NULL)
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (path);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (fd != -1)
	iopkt->iotype = OPEN_TRACE;
      else
	iopkt->iotype = OPEN_TRACE_ERROR;
      iopkt->fd = fd;
      iopkt->fstype = collector_fstype (path);
      collector_strncpy (&(iopkt->fname), path, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl,
		iopkt->comm.tstamp, FRINFO_FROM_STACK_ARG, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: open64 cannot allocate memory\n");
      return -1;
    }
  POP_REENTRANCE (guard);
  return fd;
}

#define DCL_OPEN64(dcl_f) \
  int dcl_f (const char *path, int oflag, ...) \
  { \
    if (__real_open64 == NULL) \
      init_io_intf (); \
    mode_t mode; \
    va_list ap; \
    va_start (ap, oflag); \
    mode = va_arg (ap, mode_t); \
    va_end (ap); \
    return gprofng_open64 (__real_open64, path, oflag, mode); \
  }

DCL_FUNC_VER (DCL_OPEN64, open64_2_2, open64@GLIBC_2.2)
DCL_OPEN64 (open64)


#define F_ERROR_ARG     0
#define F_INT_ARG       1
#define F_LONG_ARG      2
#define F_VOID_ARG      3

/*
 * The following macro is not defined in the
 * older versions of Linux.
 * #define F_DUPFD_CLOEXEC	1030
 *
 * Instead use the command that is defined below
 * until we start compiling mpmt on the newer
 * versions of Linux.
 */
#define TMP_F_DUPFD_CLOEXEC 1030

/*------------------------------------------------------------- fcntl */
int
fcntl (int fildes, int cmd, ...)
{
  int *guard;
  int fd = 0;
  IOTrace_packet iopkt;
  long long_arg = 0;
  int int_arg = 0;
  int which_arg = F_ERROR_ARG;
  va_list ap;
  switch (cmd)
    {
    case F_DUPFD:
    case TMP_F_DUPFD_CLOEXEC:
    case F_SETFD:
    case F_SETFL:
    case F_SETOWN:
    case F_SETSIG:
    case F_SETLEASE:
    case F_NOTIFY:
    case F_SETLK:
    case F_SETLKW:
    case F_GETLK:
      va_start (ap, cmd);
      long_arg = va_arg (ap, long);
      va_end (ap);
      which_arg = F_LONG_ARG;
      break;
    case F_GETFD:
    case F_GETFL:
    case F_GETOWN:
    case F_GETLEASE:
    case F_GETSIG:
      which_arg = F_VOID_ARG;
      break;
    }
  if (NULL_PTR (fcntl))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    {
      switch (which_arg)
	{
	case F_INT_ARG:
	  return CALL_REAL (fcntl)(fildes, cmd, int_arg);
	case F_LONG_ARG:
	  return CALL_REAL (fcntl)(fildes, cmd, long_arg);
	case F_VOID_ARG:
	  return CALL_REAL (fcntl)(fildes, cmd);
	case F_ERROR_ARG:
	  Tprintf (0, "iotrace: ERROR: Unsupported fcntl command\n");
	  return -1;
	}
      return -1;
    }
  if (cmd != F_DUPFD && cmd != TMP_F_DUPFD_CLOEXEC)
    {
      switch (which_arg)
	{
	case F_INT_ARG:
	  return CALL_REAL (fcntl)(fildes, cmd, int_arg);
	case F_LONG_ARG:
	  return CALL_REAL (fcntl)(fildes, cmd, long_arg);
	case F_VOID_ARG:
	  return CALL_REAL (fcntl)(fildes, cmd);
	case F_ERROR_ARG:
	  Tprintf (0, "iotrace: ERROR: Unsupported fcntl command\n");
	  return -1;
	}
      return -1;
    }
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  switch (cmd)
    {
    case F_DUPFD:
    case TMP_F_DUPFD_CLOEXEC:
      fd = CALL_REAL (fcntl)(fildes, cmd, long_arg);
      break;
    }
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (fd != -1)
    iopkt.iotype = OPEN_TRACE;
  else
    iopkt.iotype = OPEN_TRACE_ERROR;
  iopkt.fd = fd;
  iopkt.ofd = fildes;
  iopkt.fstype = UNKNOWNFS_TYPE;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return fd;
}

/*------------------------------------------------------------- openat */
int
openat (int fildes, const char *path, int oflag, ...)
{
  int *guard;
  int fd;
  void *packet;
  IOTrace_packet *iopkt;
  mode_t mode;
  va_list ap;
  size_t sz;
  unsigned pktSize;

  va_start (ap, oflag);
  mode = va_arg (ap, mode_t);
  va_end (ap);
  if (NULL_PTR (openat))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || path == NULL)
    return CALL_REAL (openat)(fildes, path, oflag, mode);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fd = CALL_REAL (openat)(fildes, path, oflag, mode);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (path);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (fd != -1)
	iopkt->iotype = OPEN_TRACE;
      else
	iopkt->iotype = OPEN_TRACE_ERROR;
      iopkt->fd = fd;
      iopkt->fstype = collector_fstype (path);
      collector_strncpy (&(iopkt->fname), path, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: openat cannot allocate memory\n");
      return -1;
    }
  POP_REENTRANCE (guard);
  return fd;
}

/*------------------------------------------------------------- creat */
int
creat (const char *path, mode_t mode)
{
  int *guard;
  int fd;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;
  if (NULL_PTR (creat))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || path == NULL)
    return CALL_REAL (creat)(path, mode);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fd = CALL_REAL (creat)(path, mode);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (path);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (fd != -1)
	iopkt->iotype = OPEN_TRACE;
      else
	iopkt->iotype = OPEN_TRACE_ERROR;
      iopkt->fd = fd;
      iopkt->fstype = collector_fstype (path);
      collector_strncpy (&(iopkt->fname), path, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: creat cannot allocate memory\n");
      return -1;
    }
  POP_REENTRANCE (guard);
  return fd;
}

/*------------------------------------------------------------- creat64 */
#if WSIZE(32) && !defined(__USE_LARGEFILE64)
int
creat64 (const char *path, mode_t mode)
{
  int *guard;
  int fd;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;

  if (NULL_PTR (creat64))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || path == NULL)
    return CALL_REAL (creat64)(path, mode);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fd = CALL_REAL (creat64)(path, mode);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (path);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (fd != -1)
	iopkt->iotype = OPEN_TRACE;
      else
	iopkt->iotype = OPEN_TRACE_ERROR;
      iopkt->fd = fd;
      iopkt->fstype = collector_fstype (path);
      collector_strncpy (&(iopkt->fname), path, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: creat64 cannot allocate memory\n");
      return -1;
    }
  POP_REENTRANCE (guard);
  return fd;
}
#endif

/*------------------------------------------------------------- mkstemp */
int
mkstemp (char *template)
{
  int *guard;
  int fd;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;
  if (NULL_PTR (mkstemp))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || template == NULL)
    return CALL_REAL (mkstemp)(template);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fd = CALL_REAL (mkstemp)(template);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (template);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (fd != -1)
	iopkt->iotype = OPEN_TRACE;
      else
	iopkt->iotype = OPEN_TRACE_ERROR;
      iopkt->fd = fd;
      iopkt->fstype = collector_fstype (template);
      collector_strncpy (&(iopkt->fname), template, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: mkstemp cannot allocate memory\n");
      return -1;
    }
  POP_REENTRANCE (guard);
  return fd;
}

/*------------------------------------------------------------- mkstemps */
int
mkstemps (char *template, int slen)
{
  int *guard;
  int fd;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;
  if (NULL_PTR (mkstemps))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || template == NULL)
    return CALL_REAL (mkstemps)(template, slen);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fd = CALL_REAL (mkstemps)(template, slen);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (template);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (fd != -1)
	iopkt->iotype = OPEN_TRACE;
      else
	iopkt->iotype = OPEN_TRACE_ERROR;
      iopkt->fd = fd;
      iopkt->fstype = collector_fstype (template);
      collector_strncpy (&(iopkt->fname), template, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: mkstemps cannot allocate memory\n");
      return -1;
    }
  POP_REENTRANCE (guard);
  return fd;
}

/*------------------------------------------------------------- close */
int
close (int fildes)
{
  int *guard;
  int stat;
  IOTrace_packet iopkt;
  if (NULL_PTR (close))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (close)(fildes);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  stat = CALL_REAL (close)(fildes);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return stat;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (stat == 0)
    iopkt.iotype = CLOSE_TRACE;
  else
    iopkt.iotype = CLOSE_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return stat;
}

/*------------------------------------------------------------- fopen */
static FILE*
gprofng_fopen (FILE*(real_fopen) (), const char *filename, const char *mode)
{
  int *guard;
  FILE *fp = NULL;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;
  if (CHCK_REENTRANCE (guard) || filename == NULL)
    return real_fopen (filename, mode);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();

  fp = real_fopen (filename, mode);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fp;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (filename);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (fp != NULL)
	{
	  iopkt->iotype = OPEN_TRACE;
	  iopkt->fd = fileno (fp);
	}
      else
	{
	  iopkt->iotype = OPEN_TRACE_ERROR;
	  iopkt->fd = -1;
	}
      iopkt->fstype = collector_fstype (filename);
      collector_strncpy (&(iopkt->fname), filename, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl,
			iopkt->comm.tstamp, FRINFO_FROM_STACK_ARG, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: fopen cannot allocate memory\n");
      return NULL;
    }
  POP_REENTRANCE (guard);
  return fp;
}

#define DCL_FOPEN(dcl_f) \
  FILE *dcl_f (const char *filename, const char *mode) \
  { \
    if (__real_fopen == NULL) \
      init_io_intf (); \
    return gprofng_fopen (__real_fopen, filename, mode); \
  }

DCL_FUNC_VER (DCL_FOPEN, fopen_2_17, fopen@GLIBC_2.17)
DCL_FUNC_VER (DCL_FOPEN, fopen_2_2_5, fopen@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_FOPEN, fopen_2_1, fopen@GLIBC_2.1)
DCL_FUNC_VER (DCL_FOPEN, fopen_2_0, fopen@GLIBC_2.0)
DCL_FOPEN (fopen)

/*------------------------------------------------------------- fclose */
static int
gprofng_fclose (int(real_fclose) (), FILE *stream)
{
  int *guard;
  int stat;
  IOTrace_packet iopkt;
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return real_fclose (stream);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  stat = real_fclose (stream);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return stat;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (stat == 0)
    iopkt.iotype = CLOSE_TRACE;
  else
    iopkt.iotype = CLOSE_TRACE_ERROR;
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl,
			iopkt.comm.tstamp, FRINFO_FROM_STACK_ARG, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return stat;
}

#define DCL_FCLOSE(dcl_f) \
  int dcl_f (FILE *stream) \
  { \
    if (__real_fclose == NULL) \
      init_io_intf (); \
    return gprofng_fclose (__real_fclose, stream); \
  }

DCL_FUNC_VER (DCL_FCLOSE, fclose_2_17, fclose@GLIBC_2.17)
DCL_FUNC_VER (DCL_FCLOSE, fclose_2_2_5, fclose@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_FCLOSE, fclose_2_1, fclose@GLIBC_2.1)
DCL_FUNC_VER (DCL_FCLOSE, fclose_2_0, fclose@GLIBC_2.0)
DCL_FCLOSE (fclose)

/*------------------------------------------------------------- fflush */
int
fflush (FILE *stream)
{
  int *guard;
  int stat;
  IOTrace_packet iopkt;
  if (NULL_PTR (fflush))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (fflush)(stream);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  stat = CALL_REAL (fflush)(stream);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return stat;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (stat == 0)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  if (stream != NULL)
    iopkt.fd = fileno (stream);
  else
    iopkt.fd = -1;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return stat;
}

/*------------------------------------------------------------- fdopen */
static FILE*
gprofng_fdopen (FILE*(real_fdopen) (), int fildes, const char *mode)
{
  int *guard;
  FILE *fp = NULL;
  IOTrace_packet iopkt;
  if (NULL_PTR (fdopen))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return real_fdopen (fildes, mode);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fp = real_fdopen (fildes, mode);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fp;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (fp != NULL)
    iopkt.iotype = OPEN_TRACE;
  else
    iopkt.iotype = OPEN_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.fstype = UNKNOWNFS_TYPE;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl,
			iopkt.comm.tstamp, FRINFO_FROM_STACK_ARG, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return fp;
}

#define DCL_FDOPEN(dcl_f) \
  FILE *dcl_f (int fildes, const char *mode) \
  { \
    if (__real_fdopen == NULL) \
      init_io_intf (); \
    return gprofng_fdopen (__real_fdopen, fildes, mode); \
  }

DCL_FUNC_VER (DCL_FDOPEN, fdopen_2_17, fdopen@GLIBC_2.17)
DCL_FUNC_VER (DCL_FDOPEN, fdopen_2_2_5, fdopen@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_FDOPEN, fdopen_2_1, fdopen@GLIBC_2.1)
DCL_FUNC_VER (DCL_FDOPEN, fdopen_2_0, fdopen@GLIBC_2.0)
DCL_FDOPEN (fdopen)

/*------------------------------------------------------------- dup */
int
dup (int fildes)
{
  int *guard;
  int fd;
  IOTrace_packet iopkt;
  if (NULL_PTR (dup))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (dup)(fildes);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fd = CALL_REAL (dup)(fildes);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (fd != -1)
    iopkt.iotype = OPEN_TRACE;
  else
    iopkt.iotype = OPEN_TRACE_ERROR;

  iopkt.fd = fd;
  iopkt.ofd = fildes;
  iopkt.fstype = UNKNOWNFS_TYPE;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return fd;
}

/*------------------------------------------------------------- dup2 */
int
dup2 (int fildes, int fildes2)
{
  int *guard;
  int fd;
  IOTrace_packet iopkt;
  if (NULL_PTR (dup2))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (dup2)(fildes, fildes2);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fd = CALL_REAL (dup2)(fildes, fildes2);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (fd != -1)
    iopkt.iotype = OPEN_TRACE;
  else
    iopkt.iotype = OPEN_TRACE_ERROR;
  iopkt.fd = fd;
  iopkt.ofd = fildes;
  iopkt.fstype = UNKNOWNFS_TYPE;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return fd;
}

/*------------------------------------------------------------- pipe */
int
pipe (int fildes[2])
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (pipe))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (pipe)(fildes);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (pipe)(fildes);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret != -1)
    iopkt.iotype = OPEN_TRACE;
  else
    iopkt.iotype = OPEN_TRACE_ERROR;
  iopkt.fd = fildes[0];
  iopkt.fstype = UNKNOWNFS_TYPE;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret != -1)
    iopkt.iotype = OPEN_TRACE;
  else
    iopkt.iotype = OPEN_TRACE_ERROR;
  iopkt.fd = fildes[1];
  iopkt.fstype = UNKNOWNFS_TYPE;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- socket */
int
socket (int domain, int type, int protocol)
{
  int *guard;
  int fd;
  IOTrace_packet iopkt;
  if (NULL_PTR (socket))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (socket)(domain, type, protocol);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  fd = CALL_REAL (socket)(domain, type, protocol);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return fd;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (fd != -1)
    iopkt.iotype = OPEN_TRACE;
  else
    iopkt.iotype = OPEN_TRACE_ERROR;
  iopkt.fd = fd;
  iopkt.fstype = UNKNOWNFS_TYPE;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return fd;
}

/*------------------------------------------------------------- read */
ssize_t
read (int fildes, void *buf, size_t nbyte)
{
  int *guard;
  ssize_t ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (read))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (read)(fildes, buf, nbyte);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (read)(fildes, buf, nbyte);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret >= 0)
    iopkt.iotype = READ_TRACE;
  else
    iopkt.iotype = READ_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.nbyte = ret;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- write */
ssize_t
write (int fildes, const void *buf, size_t nbyte)
{
  int *guard;
  ssize_t ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (write))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (write)(fildes, buf, nbyte);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (write)(fildes, buf, nbyte);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret >= 0)
    iopkt.iotype = WRITE_TRACE;
  else
    iopkt.iotype = WRITE_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.nbyte = ret;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- readv */
ssize_t
readv (int fildes, const struct iovec *iov, int iovcnt)
{
  int *guard;
  ssize_t ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (readv))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (readv)(fildes, iov, iovcnt);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (readv)(fildes, iov, iovcnt);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret >= 0)
    iopkt.iotype = READ_TRACE;
  else
    iopkt.iotype = READ_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.nbyte = ret;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- writev */
ssize_t
writev (int fildes, const struct iovec *iov, int iovcnt)
{
  int *guard;
  ssize_t ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (writev))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (writev)(fildes, iov, iovcnt);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (writev)(fildes, iov, iovcnt);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret >= 0)
    iopkt.iotype = WRITE_TRACE;
  else
    iopkt.iotype = WRITE_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.nbyte = ret;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- fread */
size_t
fread (void *ptr, size_t size, size_t nitems, FILE *stream)
{
  int *guard;
  size_t ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (fread))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return CALL_REAL (fread)(ptr, size, nitems, stream);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (fread)(ptr, size, nitems, stream);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ferror (stream) == 0)
    {
      iopkt.iotype = READ_TRACE;
      iopkt.nbyte = ret * size;
    }
  else
    {
      iopkt.iotype = READ_TRACE_ERROR;
      iopkt.nbyte = 0;
    }
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- fwrite */
size_t
fwrite (const void *ptr, size_t size, size_t nitems, FILE *stream)
{
  int *guard;
  size_t ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (fwrite))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return CALL_REAL (fwrite)(ptr, size, nitems, stream);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (fwrite)(ptr, size, nitems, stream);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ferror (stream) == 0)
    {
      iopkt.iotype = WRITE_TRACE;
      iopkt.nbyte = ret * size;
    }
  else
    {
      iopkt.iotype = WRITE_TRACE_ERROR;
      iopkt.nbyte = 0;
    }
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- pread */
static ssize_t
gprofng_pread (ssize_t(real_pread) (int, void *, size_t, off_t),
	       int fildes, void *buf, size_t nbyte, off_t offset)
{
  int *guard;
  ssize_t ret;
  IOTrace_packet iopkt;
  if (CHCK_REENTRANCE (guard))
    return real_pread (fildes, buf, nbyte, offset);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = real_pread (fildes, buf, nbyte, offset);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret >= 0)
    iopkt.iotype = READ_TRACE;
  else
    iopkt.iotype = READ_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.nbyte = ret;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl,
			iopkt.comm.tstamp, FRINFO_FROM_STACK_ARG, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) &iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

#define DCL_PREAD(dcl_f) \
  ssize_t dcl_f (int fildes, void *buf, size_t nbyte, off_t offset) \
  { \
    if (__real_pread == NULL) \
      init_io_intf (); \
    return gprofng_pread (__real_pread, fildes, buf, nbyte, offset); \
  }

DCL_FUNC_VER (DCL_PREAD, pread_2_2, pread@GLIBC_2.2)
DCL_PREAD (pread)

/*------------------------------------------------------------- pwrite */

#if !defined(__MUSL_LIBC)
// map interposed symbol versions

SYMVER_ATTRIBUTE (__collector_pwrite_2_2, pwrite@GLIBC_2.2)
int
__collector_pwrite_2_2 (int fildes, const void *buf, size_t nbyte, off_t offset)
{
  int *guard;
  if (NULL_PTR (pwrite_2_2))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (pwrite_2_2)(fildes, buf, nbyte, offset);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ssize_t ret = CALL_REAL (pwrite_2_2)(fildes, buf, nbyte, offset);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  write_io_packet (fildes, ret, reqt, ret >= 0 ? WRITE_TRACE : WRITE_TRACE_ERROR);
  POP_REENTRANCE (guard);
  return ret;
}

#endif
ssize_t
pwrite (int fildes, const void *buf, size_t nbyte, off_t offset)
{
  int *guard;
  if (NULL_PTR (pwrite))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (pwrite)(fildes, buf, nbyte, offset);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ssize_t ret = CALL_REAL (pwrite)(fildes, buf, nbyte, offset);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  write_io_packet (fildes, ret, reqt, ret >= 0 ? WRITE_TRACE : WRITE_TRACE_ERROR);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- pwrite64 */
#if WSIZE(32) && !defined(__USE_FILE_OFFSET64)
#if !defined(__MUSL_LIBC)
// map interposed symbol versions

SYMVER_ATTRIBUTE (__collector_pwrite64_2_2, pwrite64@GLIBC_2.2)
ssize_t
__collector_pwrite64_2_2 (int fildes, const void *buf, size_t nbyte, off64_t offset)
{
  int *guard;
  if (NULL_PTR (pwrite64_2_2))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (pwrite64_2_2)(fildes, buf, nbyte, offset);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ssize_t ret = CALL_REAL (pwrite64_2_2)(fildes, buf, nbyte, offset);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  write_io_packet (fildes, ret, reqt, ret >= 0 ? WRITE_TRACE : WRITE_TRACE_ERROR);
  POP_REENTRANCE (guard);
  return ret;
}
#endif

ssize_t
pwrite64 (int fildes, const void *buf, size_t nbyte, off64_t offset)
{
  int *guard;
  if (NULL_PTR (pwrite64))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (pwrite64)(fildes, buf, nbyte, offset);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ssize_t ret = CALL_REAL (pwrite64)(fildes, buf, nbyte, offset);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  write_io_packet (fildes, ret, reqt, ret >= 0 ? WRITE_TRACE : WRITE_TRACE_ERROR);
  POP_REENTRANCE (guard);
  return ret;
}
#endif

/*------------------------------------------------------------- fgets */
char*
fgets (char *s, int n, FILE *stream)
{
  int *guard;
  char *ptr;
  IOTrace_packet iopkt;
  if (NULL_PTR (fgets))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return CALL_REAL (fgets)(s, n, stream);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ptr = CALL_REAL (fgets)(s, n, stream);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ptr;
    }
  int error = errno;
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ptr != NULL)
    {
      iopkt.iotype = READ_TRACE;
      iopkt.nbyte = collector_strlen (ptr);
    }
  else if (ptr == NULL && error != EAGAIN && error != EBADF && error != EINTR &&
	   error != EIO && error != EOVERFLOW && error != ENOMEM && error != ENXIO)
    {
      iopkt.iotype = READ_TRACE;
      iopkt.nbyte = 0;
    }
  else
    iopkt.iotype = READ_TRACE_ERROR;
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ptr;
}

/*------------------------------------------------------------- fputs */
int
fputs (const char *s, FILE *stream)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (fputs))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return CALL_REAL (fputs)(s, stream);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (fputs)(s, stream);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret != EOF)
    {
      iopkt.iotype = WRITE_TRACE;
      iopkt.nbyte = ret;
    }
  else
    {
      iopkt.iotype = WRITE_TRACE_ERROR;
      iopkt.nbyte = 0;
    }
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- fputc */
int
fputc (int c, FILE *stream)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (fputc))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return CALL_REAL (fputc)(c, stream);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (fputc)(c, stream);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret != EOF)
    {
      iopkt.iotype = WRITE_TRACE;
      iopkt.nbyte = ret;
    }
  else
    {
      iopkt.iotype = WRITE_TRACE_ERROR;
      iopkt.nbyte = 0;
    }
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- fprintf */
int
fprintf (FILE *stream, const char *format, ...)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  va_list ap;
  va_start (ap, format);
  if (NULL_PTR (fprintf))
    init_io_intf ();
  if (NULL_PTR (vfprintf))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    {
      ret = CALL_REAL (vfprintf)(stream, format, ap);
      va_end (ap);
      return ret;
    }
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (vfprintf)(stream, format, ap);
  va_end (ap);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret >= 0)
    iopkt.iotype = WRITE_TRACE;
  else
    iopkt.iotype = WRITE_TRACE_ERROR;
  iopkt.fd = fileno (stream);
  iopkt.nbyte = ret;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- vfprintf */
int
vfprintf (FILE *stream, const char *format, va_list ap)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (vfprintf))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return CALL_REAL (vfprintf)(stream, format, ap);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (vfprintf)(stream, format, ap);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret >= 0)
    iopkt.iotype = WRITE_TRACE;
  else
    iopkt.iotype = WRITE_TRACE_ERROR;
  iopkt.fd = fileno (stream);
  iopkt.nbyte = ret;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- lseek */
off_t
lseek (int fildes, off_t offset, int whence)
{
  int *guard;
  off_t ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (lseek))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (lseek)(fildes, offset, whence);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (lseek)(fildes, offset, whence);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret != -1)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- llseek */
offset_t
llseek (int fildes, offset_t offset, int whence)
{
  int *guard;
  offset_t ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (llseek))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (llseek)(fildes, offset, whence);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (llseek)(fildes, offset, whence);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret != -1)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- chmod */
int
chmod (const char *path, mode_t mode)
{
  int *guard;
  int ret;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;
  if (NULL_PTR (chmod))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || path == NULL)
    return CALL_REAL (chmod)(path, mode);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (chmod)(path, mode);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (path);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (ret != -1)
	iopkt->iotype = OTHERIO_TRACE;
      else
	iopkt->iotype = OTHERIO_TRACE_ERROR;
      collector_strncpy (&(iopkt->fname), path, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: chmod cannot allocate memory\n");
      return 0;
    }
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- access */
int
access (const char *path, int amode)
{
  int *guard;
  int ret;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;
  if (NULL_PTR (access))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || path == NULL)
    return CALL_REAL (access)(path, amode);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (access)(path, amode);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (path);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (ret != -1)
	iopkt->iotype = OTHERIO_TRACE;
      else
	iopkt->iotype = OTHERIO_TRACE_ERROR;
      collector_strncpy (&(iopkt->fname), path, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: access cannot allocate memory\n");
      return 0;
    }
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- rename */
int
rename (const char *old, const char *new)
{
  int *guard;
  int ret;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;
  if (NULL_PTR (rename))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || new == NULL)
    return CALL_REAL (rename)(old, new);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (rename)(old, new);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (new);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (ret != -1)
	iopkt->iotype = OTHERIO_TRACE;
      else
	iopkt->iotype = OTHERIO_TRACE_ERROR;
      collector_strncpy (&(iopkt->fname), new, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: rename cannot allocate memory\n");
      return 0;
    }
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- mkdir */
int
mkdir (const char *path, mode_t mode)
{
  int *guard;
  int ret;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;
  if (NULL_PTR (mkdir))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || path == NULL)
    return CALL_REAL (mkdir)(path, mode);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (mkdir)(path, mode);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (path);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (ret != -1)
	iopkt->iotype = OTHERIO_TRACE;
      else
	iopkt->iotype = OTHERIO_TRACE_ERROR;
      collector_strncpy (&(iopkt->fname), path, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: mkdir cannot allocate memory\n");
      return 0;
    }
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- getdents */
int
getdents (int fildes, struct dirent *buf, size_t nbyte)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (getdents))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (getdents)(fildes, buf, nbyte);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (getdents)(fildes, buf, nbyte);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret != -1)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- unlink */
int
unlink (const char *path)
{
  int *guard;
  int ret;
  void *packet;
  IOTrace_packet *iopkt;
  size_t sz;
  unsigned pktSize;
  if (NULL_PTR (unlink))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || path == NULL)
    return CALL_REAL (unlink)(path);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (unlink)(path);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  sz = collector_strlen (path);
  pktSize = sizeof (IOTrace_packet) + sz;
  pktSize = collector_align_pktsize (pktSize);
  Tprintf (DBG_LT1, "iotrace allocating %u from io_heap\n", pktSize);
  packet = collector_interface->allocCSize (io_heap, pktSize, 1);
  if (packet != NULL)
    {
      iopkt = (IOTrace_packet *) packet;
      collector_memset (iopkt, 0, pktSize);
      iopkt->comm.tsize = pktSize;
      iopkt->comm.tstamp = grnt;
      iopkt->requested = reqt;
      if (ret != -1)
	iopkt->iotype = OTHERIO_TRACE;
      else
	iopkt->iotype = OTHERIO_TRACE_ERROR;
      collector_strncpy (&(iopkt->fname), path, sz);
      iopkt->comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt->comm.tstamp, FRINFO_FROM_STACK, &iopkt);
      collector_interface->writeDataRecord (io_hndl, (Common_packet*) iopkt);
      collector_interface->freeCSize (io_heap, packet, pktSize);
    }
  else
    {
      Tprintf (0, "iotrace: ERROR: unlink cannot allocate memory\n");
      return 0;
    }
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- fseek */
int
fseek (FILE *stream, long offset, int whence)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (fseek))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return CALL_REAL (fseek)(stream, offset, whence);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (fseek)(stream, offset, whence);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret != -1)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- rewind */
void
rewind (FILE *stream)
{
  int *guard;
  IOTrace_packet iopkt;
  if (NULL_PTR (rewind))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    {
      CALL_REAL (rewind)(stream);
      return;
    }
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  CALL_REAL (rewind)(stream);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  iopkt.iotype = OTHERIO_TRACE;
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
}

/*------------------------------------------------------------- ftell */
long
ftell (FILE *stream)
{
  int *guard;
  long ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (ftell))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return CALL_REAL (ftell)(stream);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (ftell)(stream);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret != -1)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- fgetpos */
static int
gprofng_fgetpos (int(real_fgetpos) (FILE *stream, fpos_t *pos),
		 FILE *stream, fpos_t *pos)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return real_fgetpos (stream, pos);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = real_fgetpos (stream, pos);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret == 0)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl,
			iopkt.comm.tstamp, FRINFO_FROM_STACK_ARG, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) &iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

#define DCL_FGETPOS(dcl_f) \
  int dcl_f (FILE *stream, fpos_t *pos) \
  { \
    if (__real_fgetpos == NULL) \
      init_io_intf (); \
    return gprofng_fgetpos (__real_fgetpos, stream, pos); \
  }

DCL_FUNC_VER (DCL_FGETPOS, fgetpos_2_17, fgetpos@GLIBC_2.17)
DCL_FUNC_VER (DCL_FGETPOS, fgetpos_2_2_5, fgetpos@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_FGETPOS, fgetpos_2_2, fgetpos@GLIBC_2.2)
DCL_FUNC_VER (DCL_FGETPOS, fgetpos_2_0, fgetpos@GLIBC_2.0)
DCL_FGETPOS (fgetpos)

/*------------------------------------------------------------- fgetpos64 */
static int
gprofng_fgetpos64 (int(real_fgetpos64) (), FILE *stream, fpos64_t *pos)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return real_fgetpos64 (stream, pos);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = real_fgetpos64 (stream, pos);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret == 0)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl,
			iopkt.comm.tstamp, FRINFO_FROM_STACK_ARG, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) &iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

#define DCL_FGETPOS64(dcl_f) \
  int dcl_f (FILE *stream, fpos64_t *pos) \
  { \
    if (__real_fgetpos64 == NULL) \
      init_io_intf (); \
    return gprofng_fgetpos64 (__real_fgetpos64, stream, pos); \
  }

DCL_FUNC_VER (DCL_FGETPOS64, fgetpos64_2_17, fgetpos64@GLIBC_2.17)
DCL_FUNC_VER (DCL_FGETPOS64, fgetpos64_2_2_5, fgetpos64@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_FGETPOS64, fgetpos64_2_2, fgetpos64@GLIBC_2.2)
DCL_FUNC_VER (DCL_FGETPOS64, fgetpos64_2_1, fgetpos64@GLIBC_2.1)
DCL_FGETPOS64 (fgetpos64)

/*------------------------------------------------------------- fsetpos */
static int
gprofng_fsetpos (int(real_fsetpos) (FILE *, const fpos_t *),
		 FILE *stream, const fpos_t *pos)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return real_fsetpos (stream, pos);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = real_fsetpos (stream, pos);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret == 0)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl,
			iopkt.comm.tstamp, FRINFO_FROM_STACK_ARG, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) &iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

#define DCL_FSETPOS(dcl_f) \
  int dcl_f (FILE *stream, const fpos_t *pos) \
  { \
    if (__real_fsetpos == NULL) \
      init_io_intf (); \
    return gprofng_fsetpos (__real_fsetpos, stream, pos); \
  }

DCL_FUNC_VER (DCL_FSETPOS, fsetpos_2_17, fsetpos@GLIBC_2.17)
DCL_FUNC_VER (DCL_FSETPOS, fsetpos_2_2_5, fsetpos@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_FSETPOS, fsetpos_2_2, fsetpos@GLIBC_2.2)
DCL_FUNC_VER (DCL_FSETPOS, fsetpos_2_0, fsetpos@GLIBC_2.0)
DCL_FSETPOS (fsetpos)

/*------------------------------------------------------------- fsetpos64 */
static int
gprofng_fsetpos64 (int(real_fsetpos64) (FILE *, const fpos64_t *),
		   FILE *stream, const fpos64_t *pos)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (CHCK_REENTRANCE (guard) || stream == NULL)
    return real_fsetpos64 (stream, pos);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = real_fsetpos64 (stream, pos);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret == 0)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fileno (stream);
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl,
			iopkt.comm.tstamp, FRINFO_FROM_STACK_ARG, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

#define DCL_FSETPOS64(dcl_f) \
  int dcl_f (FILE *stream, const fpos64_t *pos) \
  { \
    if (__real_fsetpos64 == NULL) \
      init_io_intf (); \
    return gprofng_fsetpos64 (__real_fsetpos64, stream, pos); \
  }

DCL_FUNC_VER (DCL_FSETPOS64, fsetpos64_2_17, fsetpos64@GLIBC_2.17)
DCL_FUNC_VER (DCL_FSETPOS64, fsetpos64_2_2_5, fsetpos64@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_FSETPOS64, fsetpos64_2_2, fsetpos64@GLIBC_2.2)
DCL_FUNC_VER (DCL_FSETPOS64, fsetpos64_2_1, fsetpos64@GLIBC_2.1)
DCL_FSETPOS64 (fsetpos64)

/*------------------------------------------------------------- fsync */
int
fsync (int fildes)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (fsync))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (fsync)(fildes);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (fsync)(fildes);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret == 0)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- readdir */
struct dirent*
readdir (DIR *dirp)
{
  int *guard;
  struct dirent *ptr;
  IOTrace_packet iopkt;
  if (NULL_PTR (readdir))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (readdir)(dirp);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ptr = CALL_REAL (readdir)(dirp);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ptr;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof ( IOTrace_packet));
  iopkt.comm.tsize = sizeof ( IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ptr != NULL)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ptr;
}

/*------------------------------------------------------------- flock */
int
flock (int fd, int operation)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (flock))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (flock)(fd, operation);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (flock)(fd, operation);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret == 0)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fd;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}

/*------------------------------------------------------------- lockf */
int
lockf (int fildes, int function, off_t size)
{
  int *guard;
  int ret;
  IOTrace_packet iopkt;
  if (NULL_PTR (lockf))
    init_io_intf ();
  if (CHCK_REENTRANCE (guard))
    return CALL_REAL (lockf)(fildes, function, size);
  PUSH_REENTRANCE (guard);
  hrtime_t reqt = gethrtime ();
  ret = CALL_REAL (lockf)(fildes, function, size);
  if (RECHCK_REENTRANCE (guard))
    {
      POP_REENTRANCE (guard);
      return ret;
    }
  hrtime_t grnt = gethrtime ();
  collector_memset (&iopkt, 0, sizeof (IOTrace_packet));
  iopkt.comm.tsize = sizeof (IOTrace_packet);
  iopkt.comm.tstamp = grnt;
  iopkt.requested = reqt;
  if (ret == 0)
    iopkt.iotype = OTHERIO_TRACE;
  else
    iopkt.iotype = OTHERIO_TRACE_ERROR;
  iopkt.fd = fildes;
  iopkt.comm.frinfo = collector_interface->getFrameInfo (io_hndl, iopkt.comm.tstamp, FRINFO_FROM_STACK, &iopkt);
  collector_interface->writeDataRecord (io_hndl, (Common_packet*) & iopkt);
  POP_REENTRANCE (guard);
  return ret;
}
