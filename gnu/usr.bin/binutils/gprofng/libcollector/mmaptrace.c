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
 * memory map tracking
 * incorporating former "loadobjects" into more general "map"
 * (including code and data segments and dynamic functions)
 */

#include "config.h"
#include <alloca.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <stdint.h>

#include "gp-defs.h"
#include "collector.h"
#include "gp-experiment.h"
#include "memmgr.h"

/*
 * These are obsolete and unreliable.
 * They are included here only for historical compatibility.
 */
#define MA_SHARED   0x08 /* changes are shared by mapped object */
#define MA_ANON     0x40 /* anonymous memory (e.g. /dev/zero) */
#define MA_ISM      0x80 /* intimate shared mem (shared MMU resources) */
#define MA_BREAK    0x10 /* grown by brk(2) */
#define MA_STACK    0x20 /* grown automatically on stack faults */

typedef struct prmap_t
{
  unsigned long pr_vaddr;   /* virtual address of mapping */
  unsigned long pr_size;    /* size of mapping in bytes */
  char *pr_mapname;         /* name in /proc/<pid>/object */
  int pr_mflags;            /* protection and attribute flags (see below) */
  unsigned long pr_offset;  /* offset into mapped object, if any */
  unsigned long pr_dev;
  unsigned long pr_ino;
  int pr_pagesize;          /* pagesize (bytes) for this mapping */
} prmap_t;

typedef struct MapInfo
{
  struct MapInfo *next;
  unsigned long vaddr;
  unsigned long size;
  char *mapname;    /* name in /proc/<pid>/object */
  char *filename;
  unsigned long offset;
  int mflags;
  int pagesize;
} MapInfo;

typedef struct NameInfo
{
  struct NameInfo *next;
  char *mapname;
  char filename[1];     /* dynamic length file name   */
} NameInfo;

static NameInfo *namemaps = NULL;
static MapInfo mmaps;               /* current memory maps */
static struct DataHandle *map_hndl = NULL;
static char dyntext_fname[MAXPATHLEN];
static void *mapcache = NULL;
static char *maptext = NULL;
static size_t maptext_sz = 4096;    /* initial buffer size */
static int mmap_mode = 0;
static int mmap_initted = 0;
static collector_mutex_t map_lock = COLLECTOR_MUTEX_INITIALIZER;
static collector_mutex_t dyntext_lock = COLLECTOR_MUTEX_INITIALIZER;

/* a reentrance guard for the interposition functions ensures that updates to
   the map cache/file are sequential, with the first doing the final update */
static int reentrance = 0;
#define CHCK_REENTRANCE  (reentrance || mmap_mode <= 0)
#define CURR_REENTRANCE  reentrance
#define PUSH_REENTRANCE  reentrance++
#define POP_REENTRANCE   reentrance--

/* interposition function handles */
static void *(*__real_mmap)(void* start, size_t length, int prot, int flags,
			    int fd, off_t offset) = NULL;
static void *(*__real_mmap64)(void* start, size_t length, int prot, int flags,
			      int fd, off64_t offset) = NULL;
static int (*__real_munmap)(void* start, size_t length) = NULL;
static void *(*__real_dlopen)(const char* pathname, int mode) = NULL;
static void *(*__real_dlopen_2_34)(const char* pathname, int mode) = NULL;
static void *(*__real_dlopen_2_17)(const char* pathname, int mode) = NULL;
static void *(*__real_dlopen_2_2_5)(const char* pathname, int mode) = NULL;
static void *(*__real_dlopen_2_1)(const char* pathname, int mode) = NULL;
static void *(*__real_dlopen_2_0)(const char* pathname, int mode) = NULL;

static int (*__real_dlclose)(void* handle) = NULL;
static int (*__real_dlclose_2_34)(void* handle) = NULL;
static int (*__real_dlclose_2_17)(void* handle) = NULL;
static int (*__real_dlclose_2_2_5)(void* handle) = NULL;
static int (*__real_dlclose_2_0)(void* handle) = NULL;
static void (*collector_heap_record)(int, size_t, void*) = NULL;

/* internal function prototypes */
static int init_mmap_intf ();
static int init_mmap_files ();
static void append_segment_record (char *format, ...);
static void update_map_segments (hrtime_t hrt, int resolve);
static void resolve_mapname (MapInfo *map, char *name);
static void record_segment_map (hrtime_t timestamp, uint64_t loadaddr,
				unsigned long msize, int pagesize, int modeflags,
				long long offset, unsigned check, char *name);
static void record_segment_unmap (hrtime_t timestamp, uint64_t loadaddr);

/* Linux needs handling of the vsyscall page to get its data into the map.xml file */
static void process_vsyscall_page ();

#define MAXVSYSFUNCS 10
static int nvsysfuncs = 0;
static char *sysfuncname[MAXVSYSFUNCS];
static uint64_t sysfuncvaddr[MAXVSYSFUNCS];
static unsigned long sysfuncsize[MAXVSYSFUNCS];

#define MAXDYN 20
static int ndyn = 0;
static char *dynname [MAXDYN];
static void *dynvaddr [MAXDYN];
static unsigned dynsize [MAXDYN];
static char *dynfuncname[MAXDYN];

/*===================================================================*/

/*
 * void __collector_mmap_init_mutex_locks()
 *      Iinitialize mmap mutex locks.
 */
void
__collector_mmap_init_mutex_locks ()
{
  __collector_mutex_init (&map_lock);
  __collector_mutex_init (&dyntext_lock);
}

/* __collector_ext_update_map_segments called by the audit agent
 * Is is also called by dbx/collector when a (possible) map update
 * is intimated, such as after dlopen/dlclose.
 * Required when libcollector.so is not preloaded and interpositions inactive.
 */
int
__collector_ext_update_map_segments (void)
{
  if (!mmap_initted)
    return 0;
  TprintfT (0, "__collector_ext_update_map_segments(%d)\n", CURR_REENTRANCE);
  if (CHCK_REENTRANCE)
    return 0;
  PUSH_REENTRANCE;
  update_map_segments (GETRELTIME (), 1);
  POP_REENTRANCE;
  return 0;
}
/*
 * int __collector_ext_mmap_install()
 *      Install and initialise mmap tracing.
 */
int
__collector_ext_mmap_install (int record)
{
  TprintfT (0, "__collector_ext_mmap_install(mmap_mode=%d)\n", mmap_mode);
  if (NULL_PTR (mmap))
    {
      if (init_mmap_intf ())
	{
	  TprintfT (0, "ERROR: collector mmap tracing initialization failed.\n");
	  return COL_ERROR_EXPOPEN;
	}
    }
  else
    TprintfT (DBG_LT2, "collector mmap tracing: mmap pointer not null\n");

  /* Initialize side door interface with the heap tracing module */
  collector_heap_record = (void(*)(int, size_t, void*))dlsym (RTLD_DEFAULT, "__collector_heap_record");
  if (record)
    {
      map_hndl = __collector_create_handle (SP_MAP_FILE);
      if (map_hndl == NULL)
	return COL_ERROR_MAPOPEN;
      if (init_mmap_files ())
	{
	  TprintfT (0, "ERROR: collector init_mmap_files() failed.\n");
	  return COL_ERROR_EXPOPEN;
	}
    }
  mmaps.next = NULL;
  mapcache = NULL;
  PUSH_REENTRANCE;
  update_map_segments (GETRELTIME (), 1); // initial map
  POP_REENTRANCE;
  mmap_mode = 1;
  mmap_initted = 1;
  process_vsyscall_page ();
  return COL_ERROR_NONE;
}

/*
 * int __collector_ext_mmap_deinstall()
 *	Optionally update final map and stop tracing mmap events.
 */
int
__collector_ext_mmap_deinstall (int update)
{
  if (!mmap_initted)
    return COL_ERROR_NONE;
  mmap_mode = 0;
  if (update)
    {
      /* Final map */
      PUSH_REENTRANCE;
      update_map_segments (GETRELTIME (), 1);
      POP_REENTRANCE;
    }
  TprintfT (0, "__collector_ext_mmap_deinstall(%d)\n", update);
  if (map_hndl != NULL)
    {
      __collector_delete_handle (map_hndl);
      map_hndl = NULL;
    }
  __collector_mutex_lock (&map_lock); // get lock before resetting

  /* Free all memory maps */
  MapInfo *mp;
  for (mp = mmaps.next; mp;)
    {
      MapInfo *next = mp->next;
      __collector_freeCSize (__collector_heap, mp, sizeof (*mp));
      mp = next;
    }
  mmaps.next = NULL;

  /* Free all name maps */
  NameInfo *np;
  for (np = namemaps; np;)
    {
      NameInfo *next = np->next;
      __collector_freeCSize (__collector_heap, np, sizeof (*np) + __collector_strlen (np->filename));
      np = next;
    }
  namemaps = NULL;
  mapcache = __collector_reallocVSize (__collector_heap, mapcache, 0);
  mmaps.next = NULL;
  mapcache = NULL;
  __collector_mutex_unlock (&map_lock);
  TprintfT (0, "__collector_ext_mmap_deinstall done\n");
  return 0;
}

/*
 * void __collector_mmap_fork_child_cleanup()
 *	Perform all necessary cleanup steps in child process after fork().
 */
void
__collector_mmap_fork_child_cleanup ()
{
  /* Initialize all mmap "mutex" locks */
  __collector_mmap_init_mutex_locks ();
  if (!mmap_initted)
    return;
  mmap_mode = 0;
  __collector_delete_handle (map_hndl);
  __collector_mutex_lock (&map_lock); // get lock before resetting

  /* Free all memory maps */
  MapInfo *mp;
  for (mp = mmaps.next; mp;)
    {
      MapInfo *next = mp->next;
      __collector_freeCSize (__collector_heap, mp, sizeof (*mp));
      mp = next;
    }
  mmaps.next = NULL;

  /* Free all name maps */
  NameInfo *np;
  for (np = namemaps; np;)
    {
      NameInfo *next = np->next;
      __collector_freeCSize (__collector_heap, np, sizeof (*np) + __collector_strlen (np->filename));
      np = next;
    }
  namemaps = NULL;
  mapcache = __collector_reallocVSize (__collector_heap, mapcache, 0);
  mmap_initted = 0;
  reentrance = 0;
  __collector_mutex_unlock (&map_lock);
}

static int
init_mmap_files ()
{
  TprintfT (DBG_LT2, "init_mmap_files\n");
  /* also create the headerless dyntext file (if required) */
  CALL_UTIL (snprintf)(dyntext_fname, sizeof (dyntext_fname), "%s/%s",
		       __collector_exp_dir_name, SP_DYNTEXT_FILE);
  if (CALL_UTIL (access)(dyntext_fname, F_OK) != 0)
    {
      int fd = CALL_UTIL (open)(dyntext_fname, O_RDWR | O_CREAT | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      if (fd == -1)
	{
	  char errmsg[256];
	  TprintfT (0, "ERROR: init_mmap_files: open(%s) failed\n",
		    dyntext_fname);
	  __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s: %s</event>\n",
				 SP_JCMD_CERROR, COL_ERROR_DYNOPEN, errno,
				 dyntext_fname, errmsg);
	  return COL_ERROR_DYNOPEN;
	}
      else
	CALL_UTIL (close)(fd);
    }
  return COL_ERROR_NONE;
}

static void
append_segment_record (char *format, ...)
{
  char buf[1024];
  char *bufptr = buf;
  va_list va;
  va_start (va, format);
  int sz = __collector_xml_vsnprintf (bufptr, sizeof (buf), format, va);
  va_end (va);

  if (__collector_expstate != EXP_OPEN && __collector_expstate != EXP_PAUSED)
    {
      TprintfT (0, "append_segment_record: expt neither open nor paused (%d); "
		   "not writing to map.xml\n\t%s", __collector_expstate, buf);
      return;
    }
  if (sz >= sizeof (buf))
    {
      /* Allocate a new buffer */
      sz += 1; /* add the terminating null byte */
      bufptr = (char*) alloca (sz);
      va_start (va, format);
      sz = __collector_xml_vsnprintf (bufptr, sz, format, va);
      va_end (va);
    }
  int rc = __collector_write_string (map_hndl, bufptr, sz);
  if (rc != 0)
    (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\"></event>\n",
				  SP_JCMD_CERROR, COL_ERROR_MAPWRITE);
}

static void
record_segment_map (hrtime_t timestamp, uint64_t loadaddr, unsigned long msize,
		    int pagesize, int modeflags, long long offset,
		    unsigned check, char *name)
{

  TprintfT (DBG_LT2, "record_segment_map(%s @ 0x%llx)\n", name, (long long) loadaddr);
  append_segment_record ("<event kind=\"map\" object=\"segment\" tstamp=\"%u.%09u\" "
			 "vaddr=\"0x%016llX\" size=\"%lu\" pagesz=\"%d\" foffset=\"%c0x%08llX\" "
			 "modes=\"0x%03X\" chksum=\"0x%0X\" name=\"%s\"/>\n",
			 (unsigned) (timestamp / NANOSEC),
			 (unsigned) (timestamp % NANOSEC),
			 loadaddr, msize, pagesize,
			 offset < 0 ? '-' : '+', offset < 0 ? -offset : offset,
			 modeflags, check, name);
}

static void
record_segment_unmap (hrtime_t timestamp, uint64_t loadaddr)
{
  TprintfT (DBG_LT2, "record_segment_unmap(@ 0x%llx)\n", (long long) loadaddr);
  append_segment_record ("<event kind=\"unmap\" tstamp=\"%u.%09u\" vaddr=\"0x%016llX\"/>\n",
			 (unsigned) (timestamp / NANOSEC),
			 (unsigned) (timestamp % NANOSEC), loadaddr);
}

#if WSIZE(64)
#define ELF_EHDR    Elf64_Ehdr
#define ELF_PHDR    Elf64_Phdr
#define ELF_SHDR    Elf64_Shdr
#define ELF_DYN     Elf64_Dyn
#define ELF_AUX     Elf64_auxv_t
#define ELF_SYM     Elf64_Sym
#define ELF_ST_BIND ELF64_ST_BIND
#define ELF_ST_TYPE ELF64_ST_TYPE
#elif WSIZE(32)
#define ELF_EHDR    Elf32_Ehdr
#define ELF_PHDR    Elf32_Phdr
#define ELF_SHDR    Elf32_Shdr
#define ELF_DYN     Elf32_Dyn
#define ELF_AUX     Elf32_auxv_t
#define ELF_SYM     Elf32_Sym
#define ELF_ST_BIND ELF32_ST_BIND
#define ELF_ST_TYPE ELF32_ST_TYPE
#endif

static unsigned
checksum_mapname (MapInfo* map)
{
  unsigned checksum = 0;
  /* only checksum code segments */
  if ((map->mflags & (PROT_EXEC | PROT_READ)) == 0 ||
      (map->mflags & PROT_WRITE) != 0)
    return 0;
  checksum = (unsigned) - 1;
  TprintfT (DBG_LT2, "checksum_mapname checksum = 0x%0X\n", checksum);
  return checksum;
}


static void*
dlopen_searchpath (void*(real_dlopen) (const char *, int),
		void *caller_addr, const char *basename, int mode)
{
  TprintfT (DBG_LT2, "dlopen_searchpath(%p, %s, %d)\n", caller_addr, basename, mode);
  Dl_info dl_info;
  if (dladdr (caller_addr, &dl_info) == 0)
    {
      TprintfT (0, "ERROR: dladdr(%p): %s\n", caller_addr, dlerror ());
      return 0;
    }
  TprintfT (DBG_LT2, "dladdr(%p): %p fname=%s\n",
	    caller_addr, dl_info.dli_fbase, dl_info.dli_fname);
  int noload = RTLD_LAZY | RTLD_NOW | RTLD_NOLOAD;
  void *caller_hndl = NULL;
#define WORKAROUND_RTLD_BUG 1
#ifdef WORKAROUND_RTLD_BUG
  // A dynamic linker dlopen bug can result in corruption/closure of open streams
  // XXXX workaround should be removed once linker patches are all available
#if WSIZE(64)
#define MAINBASE 0x400000
#elif WSIZE(32)
#define MAINBASE 0x08048000
#endif
  const char* tmp_path =
	  (dl_info.dli_fbase == (void*) MAINBASE) ? NULL : dl_info.dli_fname;
  caller_hndl = real_dlopen (tmp_path, noload);

#else //XXXX workaround should be removed once linker patches are all available

  caller_hndl = real_dlopen (dl_info.dli_fname, noload);

#endif //XXXX workaround should be removed once linker patches are all available

  if (!caller_hndl)
    {
      TprintfT (0, "ERROR: dlopen(%s,NOLOAD): %s\n", dl_info.dli_fname, dlerror ());
      return NULL;
    }
#if !defined(__MUSL_LIBC)
  Dl_serinfo _info, *info = &_info;
  Dl_serpath *path;

  /* determine search path count and required buffer size */
  dlinfo (caller_hndl, RTLD_DI_SERINFOSIZE, (void *) info);

  /* allocate new buffer and initialize */
  /*
      CR# 7191331
      There is a bug in Linux that causes the first call
      to dlinfo() to return a small value for the dls_size.

      The first call to dlinfo() determines the search path
      count and the required buffer size. The second call to
      dlinfo() tries to obtain the search path information.

      However, the size of the buffer that is returned by
      the first call to the dlinfo() is incorrect (too small).
      The second call to dlinfo() uses the incorrect size to
      allocate memory on the stack and internally uses the memcpy()
      function to copy the search paths to the allocated memory space.
      The length of the search path is much larger than the buffer
      that is allocated on the stack. The memcpy() overwrites some
      of the information that are saved on the stack, specifically,
      it overwrites the "basename" parameter.

      collect crashes right after the second call to dlinfo().

      The search paths are used to locate the shared libraries.
      dlinfo() creates the search paths based on the paths
      that are assigned to LD_LIBRARY_PATH environment variable
      and the standard library paths. The standard library paths
      consists of the /lib and the /usr/lib paths. The
      standard library paths are always included to the search
      paths by dlinfo() even if the LD_LIBRARY_PATH environment
      variable is not defined. Therefore, at the very least the
      dls_cnt is assigned to 2 (/lib and /usr/lib) and dlinfo()
      will never assign dls_cnt to zero. The dls_cnt is the count
      of the potential paths for searching the shared libraries.

      So we need to increase the buffer size before the second
      call to dlinfo(). There are number of ways to increase
      the buffer size. However, none of them can calculate the
      buffer size precisely. Some users on the web have suggested
      to multiply the MAXPATHLEN by dls_cnt for the buffer size.
      The MAXPATHLEN is assigned to 1024 bytes. In my opinion
      this is too much. So I have decided to multiply dls_size
      by dls_cnt for the buffer size since the dls_size is much
      smaller than 1024 bytes.

      I have already confirmed with our user that the workaround
      is working with his real application. Additionally,
      the dlopen_searchpath() function is called only by the
      libcollector init() function when the experiment is started.
      Therefore, allocating some extra bytes on the stack which
      is local to this routine is harmless.
   */

  info = alloca (_info.dls_size * _info.dls_cnt);
  info->dls_size = _info.dls_size;
  info->dls_cnt = _info.dls_cnt;

  /* obtain search path information */
  dlinfo (caller_hndl, RTLD_DI_SERINFO, (void *) info);
  path = &info->dls_serpath[0];

  char pathname[MAXPATHLEN];
  for (unsigned int cnt = 1; cnt <= info->dls_cnt; cnt++, path++)
    {
      __collector_strlcpy (pathname, path->dls_name, sizeof (pathname));
      __collector_strlcat (pathname, "/", sizeof (pathname));
      __collector_strlcat (pathname, basename, sizeof (pathname));
      void* ret = NULL;
#if (ARCH(Intel) && WSIZE(32)) || ARCH(SPARC)
      ret = (real_dlopen) (pathname, mode);
#else
      ret = CALL_REAL (dlopen)(pathname, mode);
#endif
      TprintfT (DBG_LT2, "try %d/%d: %s = %p\n", cnt, info->dls_cnt, pathname, ret);
      if (ret)
	return ret; // success!
    }
#endif
  return NULL;
}

static void
resolve_mapname (MapInfo *map, char *name)
{
  map->filename = "";
  map->mapname = "";
  if (name == NULL || *name == '\0')
    {
      if (map->mflags & MA_STACK)
	map->filename = "<" SP_MAP_STACK ">";
      else if (map->mflags & MA_BREAK)
	map->filename = "<" SP_MAP_HEAP ">";
      else if (map->mflags & MA_ISM)
	map->filename = "<" SP_MAP_SHMEM ">";
      return;
    }
  NameInfo *np;
  for (np = namemaps; np; np = np->next)
    if (__collector_strcmp (np->mapname, name) == 0)
      break;

  if (np == NULL)
    {
      const char *fname;
      fname = name;
      /* Create and link a new name map */
      size_t fnamelen = __collector_strlen (fname) + 1;
      np = (NameInfo*) __collector_allocCSize (__collector_heap, sizeof (NameInfo) + fnamelen, 1);
      if (np == NULL)   // We could not get memory
	return;
      np->mapname = np->filename;
      __collector_strlcpy (np->filename, fname, fnamelen);
      np->next = namemaps;
      namemaps = np;
    }
  map->mapname = np->mapname;
  map->filename = np->filename;
  if (map->filename[0] == (char) 0)
    map->filename = map->mapname;
  TprintfT (DBG_LT2, "resolve_mapname: %s resolved to %s\n", map->mapname, map->filename);
}

static unsigned long
str2ulong (char **ss)
{
  char *s = *ss;
  unsigned long val = 0UL;
  const int base = 16;
  for (;;)
    {
      char c = *s++;
      if (c >= '0' && c <= '9')
	val = val * base + (c - '0');
      else if (c >= 'a' && c <= 'f')
	val = val * base + (c - 'a') + 10;
      else if (c >= 'A' && c <= 'F')
	val = val * base + (c - 'A') + 10;
      else
	break;
    }
  *ss = s - 1;
  return val;
}

static void
update_map_segments (hrtime_t hrt, int resolve)
{
  size_t filesz;
  if (__collector_mutex_trylock (&map_lock))
    {
      TprintfT (0, "WARNING: update_map_segments(resolve=%d) BUSY\n", resolve);
      return;
    }
  TprintfT (DBG_LT2, "\n");
  TprintfT (DBG_LT2, "begin update_map_segments(hrt, %d)\n", resolve);

  // Note: there is similar code to read /proc/$PID/map[s] in
  // perfan/er_kernel/src/KSubExp.cc KSubExp::write_subexpt_map()
  const char* proc_map = "/proc/self/maps";
  size_t bufsz = maptext_sz;
  int done = 0;
  filesz = 0;
  int map_fd = CALL_UTIL (open)(proc_map, O_RDONLY);
  while (!done)
    {
      bufsz *= 2;
      maptext = __collector_reallocVSize (__collector_heap, maptext, bufsz);
      TprintfT (DBG_LT2, "  update_map_segments: Loop for bufsize=%ld\n",
		(long) bufsz);
      for (;;)
	{
	  int n = CALL_UTIL (read)(map_fd, maptext + filesz, bufsz - filesz);
	  TprintfT (DBG_LT2, "    update_map_segments: __collector_read(bufp=%p nbyte=%ld)=%d\n",
		    maptext + filesz, (long) ( bufsz - filesz), n);
	  if (n < 0)
	    {
	      TprintfT (0, "ERROR: update_map_segments: read(maps): errno=%d\n", errno);
	      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s</event>\n",
					    SP_JCMD_CERROR, COL_ERROR_MAPREAD, errno, proc_map);
	      CALL_UTIL (close)(map_fd);
	      __collector_mutex_unlock (&map_lock);
	      return;
	    }
	  else if (n == 0)
	    {
	      done = 1;
	      break;
	    }
	  filesz += n;
	  if (filesz >= bufsz) /* Buffer too small */
	    break;
	}
    }
  CALL_UTIL (close)(map_fd);
  maptext_sz = filesz;

  int mapcache_entries = 0;
  char *str, *str1;
  for (str = maptext;; str = str1)
    {
      for (str1 = str; str1 - maptext < filesz; str1++)
	{
	  if (*str1 == '\n')
	    {
	      *str1 = (char) 0;
	      break;
	    }
	}
      if (str1 - maptext >= filesz)
	break;
      str1++;
      mapcache_entries++;
      mapcache = __collector_reallocVSize (__collector_heap, mapcache,
					   sizeof (prmap_t) * mapcache_entries);
      prmap_t *map = ((prmap_t *) mapcache) + (mapcache_entries - 1);
      map->pr_vaddr = str2ulong (&str);
      str++;
      unsigned long eaddr = str2ulong (&str);
      str++;
      map->pr_size = eaddr - map->pr_vaddr;
      map->pr_mflags = 0;
      map->pr_mflags += (*str++ == 'r' ? PROT_READ : 0);
      map->pr_mflags += (*str++ == 'w' ? PROT_WRITE : 0);
      map->pr_mflags += (*str++ == 'x' ? PROT_EXEC : 0);
      map->pr_mflags += (*str++ == 's' ? MA_SHARED : 0);
      str++;
      map->pr_offset = str2ulong (&str);
      str++;
      map->pr_dev = str2ulong (&str) * 0x100;
      str++;
      map->pr_dev += str2ulong (&str);
      str++;
      map->pr_ino = str2ulong (&str);
      if (map->pr_dev == 0)
	map->pr_mflags |= MA_ANON;
      while (*str == ' ')
	str++;
      map->pr_mapname = str;
      map->pr_pagesize = 4096;
    }

  /* Compare two maps and record all differences */
  unsigned nidx = 0;
  MapInfo *prev = &mmaps;
  MapInfo *oldp = mmaps.next;
  for (;;)
    {
      prmap_t *newp = nidx < mapcache_entries ?
	      (prmap_t*) mapcache + nidx : NULL;
      if (oldp == NULL && newp == NULL)
	break;

      /* If two maps are equal proceed to the next pair */
      if (oldp && newp &&
	  oldp->vaddr == newp->pr_vaddr &&
	  oldp->size == newp->pr_size &&
	  __collector_strcmp (oldp->mapname, newp->pr_mapname) == 0)
	{
	  prev = oldp;
	  oldp = oldp->next;
	  nidx++;
	  continue;
	}
      /* Check if we need to unload the old map first */
      if (newp == NULL || (oldp && oldp->vaddr <= newp->pr_vaddr))
	{
	  if (oldp != NULL)
	    {
	      /* Don't record MA_ANON maps except MA_STACK and MA_BREAK */
	      if ((!(oldp->mflags & MA_ANON) || (oldp->mflags & (MA_STACK | MA_BREAK))))
		record_segment_unmap (hrt, oldp->vaddr);
	      /* Remove and free map */
	      prev->next = oldp->next;
	      MapInfo *tmp = oldp;
	      oldp = oldp->next;
	      __collector_freeCSize (__collector_heap, tmp, sizeof (*tmp));
	    }
	}
      else
	{
	  MapInfo *map = (MapInfo*) __collector_allocCSize (__collector_heap, sizeof (MapInfo), 1);
	  if (map == NULL)
	    {
	      __collector_mutex_unlock (&map_lock);
	      return;
	    }
	  map->vaddr = newp->pr_vaddr;
	  map->size = newp->pr_size;
	  map->offset = newp->pr_offset;
	  map->mflags = newp->pr_mflags;
	  map->pagesize = newp->pr_pagesize;
	  resolve_mapname (map, newp->pr_mapname);

	  /* Insert new map */
	  map->next = prev->next;
	  prev->next = map;
	  prev = map;

	  /* Don't record MA_ANON maps except MA_STACK and MA_BREAK */
	  if (!(newp->pr_mflags & MA_ANON) || (newp->pr_mflags & (MA_STACK | MA_BREAK)))
	    {
	      unsigned checksum = checksum_mapname (map);
	      record_segment_map (hrt, map->vaddr, map->size,
				  map->pagesize, map->mflags,
				  map->offset, checksum, map->filename);
	    }
	  nidx++;
	}
    }
  TprintfT (DBG_LT2, "update_map_segments: done\n\n");
  __collector_mutex_unlock (&map_lock);
} /* update_map_segments */

/*
 *    Map addr to a segment. Cope with split segments.
 */
int
__collector_check_segment_internal (unsigned long addr, unsigned long *base,
				    unsigned long *end, int maxnretries, int MA_FLAGS)
{
  int number_of_tries = 0;
retry:
  ;

  unsigned long curbase = 0;
  unsigned long curfoff = 0;
  unsigned long cursize = 0;

  MapInfo *mp;
  for (mp = mmaps.next; mp; mp = mp->next)
    {

      if (curbase + cursize == mp->vaddr &&
	  curfoff + cursize == mp->offset &&
	  ((mp->mflags & MA_FLAGS) == MA_FLAGS
	   || __collector_strncmp (mp->mapname, "[vdso]", 6) == 0
	   || __collector_strncmp (mp->mapname, "[vsyscall]", 10) == 0
	   ))
	cursize = mp->vaddr + mp->size - curbase;
      else if (addr < mp->vaddr)
	break;
      else if ((mp->mflags & MA_FLAGS) != MA_FLAGS
	       && __collector_strncmp (mp->mapname, "[vdso]", 6)
	       && __collector_strncmp (mp->mapname, "[vsyscall]", 10))
	{
	  curbase = 0;
	  curfoff = 0;
	  cursize = 0;
	}
      else
	{
	  curbase = mp->vaddr;
	  curfoff = mp->offset;
	  cursize = mp->size;
	}
    }

  if (addr >= curbase && addr < curbase + cursize)
    {
      *base = curbase;
      *end = curbase + cursize;
      return 1;
    }

  /*
   * 21275311 Unwind failure in native stack for java application running on jdk8 on x86
   *
   * On JDK8, we've observed cases where Java-compiled methods end up
   * in virtual address segments that were "dead zones" (mflags&PROT_READ==0) at
   * the time of the last update_map_segments() but are now "live".  So if we
   * fail to find a segment, let's call update_map_segments and then retry
   * before giving up.
   */
  if (number_of_tries < maxnretries)
    {
      number_of_tries++;
      __collector_ext_update_map_segments ();
      goto retry;
    }
  *base = 0;
  *end = 0;
  return 0;
}

/**
 * Check if address belongs to a readable and executable segment
 * @param addr
 * @param base
 * @param end
 * @param maxnretries
 * @return 1 - yes, 0 - no
 */
int
__collector_check_segment (unsigned long addr, unsigned long *base,
			   unsigned long *end, int maxnretries)
{
  int MA_FLAGS = PROT_READ | PROT_EXEC;
  int res = __collector_check_segment_internal (addr, base, end, maxnretries, MA_FLAGS);
  return res;
}

/**
 * Check if address belongs to a readable segment
 * @param addr
 * @param base
 * @param end
 * @param maxnretries
 * @return 1 - yes, 0 - no
 */
int
__collector_check_readable_segment( unsigned long addr, unsigned long *base, unsigned long *end, int maxnretries )
{
    int MA_FLAGS = PROT_READ;
    int res = __collector_check_segment_internal(addr, base, end, maxnretries, MA_FLAGS);
    return res;
}

static ELF_AUX *auxv = NULL;

static void
process_vsyscall_page ()
{
  TprintfT (DBG_LT2, "process_vsyscall_page()\n");
  if (ndyn != 0)
    {
      /* We've done this one in this process, and cached the results */
      /* use the cached results */
      for (int i = 0; i < ndyn; i++)
	{
	  append_segment_record ("<event kind=\"map\" object=\"dynfunc\" name=\"%s\" "
				 "vaddr=\"0x%016lX\" size=\"%u\" funcname=\"%s\" />\n",
				 dynname[i], dynvaddr[i], dynsize[i], dynfuncname[i]);
	  TprintfT (DBG_LT2, "process_vsyscall_page: append_segment_record map dynfunc='%s' vaddr=0x%016lX size=%ld funcname='%s' -- from cache\n",
		    dynname[i], (unsigned long) dynvaddr[i],
		    (long) dynsize[i], dynfuncname[i]);
	}
    }
  if (nvsysfuncs != 0)
    {
      /* We've done this one in this process, and cached the results */
      /* use the cached results */
      hrtime_t hrt = GETRELTIME ();
      for (int i = 0; i < nvsysfuncs; i++)
	{
	  append_segment_record ("<event kind=\"map\" object=\"function\" tstamp=\"%u.%09u\" "
				 "vaddr=\"0x%016lX\" size=\"%u\" name=\"%s\" />\n",
				 (unsigned) (hrt / NANOSEC), (unsigned) (hrt % NANOSEC),
				 (unsigned long) sysfuncvaddr[i], (unsigned) sysfuncsize[i], sysfuncname[i]);
	  TprintfT (DBG_LT2, "process_vsyscall_page: append_segment_record map function='%s' vaddr=0x%016lX size=%ld -- from cache\n",
		    sysfuncname[i], (unsigned long) sysfuncvaddr[i], (long) sysfuncsize[i]);
	}
    }
  if (ndyn + nvsysfuncs != 0)
    return;

  /* After fork we can't rely on environ as it might have
   * been moved by putenv(). Use the pointer saved by the parent.
   */
  if (auxv == NULL)
    {
      char **envp = (char**) environ;
      if (envp == NULL)
	return;
      while (*envp++ != NULL);
      auxv = (ELF_AUX*) envp;
    }
  TprintfT (DBG_LT2, "process_vsyscall_page, auxv = ox%p\n", auxv);

  ELF_AUX *ap;
#ifdef DEBUG
  for (ap = auxv; ap->a_type != AT_NULL; ap++)
    TprintfT (DBG_LT2, "process_vsyscall_page: ELF_AUX: "
	      " a_type = 0x%016llx %10lld   "
	      " a_un.a_val = 0x%016llx %10lld\n",
	      (long long) ap->a_type, (long long) ap->a_type,
	      (long long) ap->a_un.a_val, (long long) ap->a_un.a_val);
#endif

  // find the first ELF_AUX of type AT_SYSINFO_EHDR
  ELF_EHDR *ehdr = NULL;
  for (ap = auxv; ap->a_type != AT_NULL; ap++)
    {
      if (ap->a_type == AT_SYSINFO_EHDR)
	{
	  // newer Linuxes do not have a_ptr field, they just have a_val
	  ehdr = (ELF_EHDR*)(intptr_t) ap->a_un.a_val;
	  if (ehdr != NULL)
	    break;
	}
    }

  // If one is found
  if (ehdr != NULL)
    {
      char *mapName = "SYSINFO_EHDR";
      MapInfo *mp;
      for (mp = mmaps.next; mp; mp = mp->next)
	{
	  if ((unsigned long) ehdr == mp->vaddr)
	    {
	      mp->mflags |= PROT_EXEC;
	      if (mp->mapname && mp->mapname[0])
		mapName = mp->mapname;
	      break;
	    }
	}

      // Find the dynsym section and record all symbols
      char *base = (char*) ehdr;
      ELF_SHDR *shdr = (ELF_SHDR*) (base + ehdr->e_shoff);
      int i;

#if 0
      TprintfT (DBG_LT2, "process_vsyscall_page: ehdr: EI_CLASS=%lld  EI_DATA=%lld EI_OSABI=%lld e_type=%lld e_machine=%lld e_version=%lld\n"
		"  e_entry   =0x%016llx %10lld  e_phoff     =0x%016llx %10lld\n"
		"  e_shoff   =0x%016llx %10lld  e_flags     =0x%016llx %10lld\n"
		"  e_ehsize  =0x%016llx %10lld  e_phentsize =0x%016llx %10lld\n"
		"  e_phnum   =0x%016llx %10lld  e_shentsize =0x%016llx %10lld\n"
		"  e_shnum   =0x%016llx %10lld  e_shstrndx  =0x%016llx %10lld\n",
		(long long) ehdr->e_ident[EI_CLASS], (long long) ehdr->e_ident[EI_DATA], (long long) ehdr->e_ident[EI_OSABI],
		(long long) ehdr->e_type, (long long) ehdr->e_machine, (long long) ehdr->e_version,
		(long long) ehdr->e_entry, (long long) ehdr->e_entry,
		(long long) ehdr->e_phoff, (long long) ehdr->e_phoff,
		(long long) ehdr->e_shoff, (long long) ehdr->e_shoff,
		(long long) ehdr->e_flags, (long long) ehdr->e_flags,
		(long long) ehdr->e_ehsize, (long long) ehdr->e_ehsize,
		(long long) ehdr->e_phentsize, (long long) ehdr->e_phentsize,
		(long long) ehdr->e_phnum, (long long) ehdr->e_phnum,
		(long long) ehdr->e_shentsize, (long long) ehdr->e_shentsize,
		(long long) ehdr->e_shnum, (long long) ehdr->e_shnum,
		(long long) ehdr->e_shstrndx, (long long) ehdr->e_shstrndx);
      for (i = 1; i < ehdr->e_shnum; i++)
	{
	  TprintfT (DBG_LT2, "process_vsyscall_page: SECTION=%d sh_name=%lld '%s'\n"
		    "  sh_type       =0x%016llx %10lld\n"
		    "  sh_flags      =0x%016llx %10lld\n"
		    "  sh_addr       =0x%016llx %10lld\n"
		    "  sh_offset     =0x%016llx %10lld\n"
		    "  sh_size       =0x%016llx %10lld\n"
		    "  sh_link       =0x%016llx %10lld\n"
		    "  sh_info       =0x%016llx %10lld\n"
		    "  sh_addralign  =0x%016llx %10lld\n"
		    "  sh_entsize    =0x%016llx %10lld\n",
		    i, (long long) shdr[i].sh_name, base + shdr[ehdr->e_shstrndx].sh_offset + shdr[i].sh_name,
		    (long long) shdr[i].sh_type, (long long) shdr[i].sh_type,
		    (long long) shdr[i].sh_flags, (long long) shdr[i].sh_flags,
		    (long long) shdr[i].sh_addr, (long long) shdr[i].sh_addr,
		    (long long) shdr[i].sh_offset, (long long) shdr[i].sh_offset,
		    (long long) shdr[i].sh_size, (long long) shdr[i].sh_size,
		    (long long) shdr[i].sh_link, (long long) shdr[i].sh_link,
		    (long long) shdr[i].sh_info, (long long) shdr[i].sh_info,
		    (long long) shdr[i].sh_addralign, (long long) shdr[i].sh_addralign,
		    (long long) shdr[i].sh_entsize, (long long) shdr[i].sh_entsize);
	}
#endif

      int dynSec = -1;
      for (i = 1; i < ehdr->e_shnum; i++)
	if (shdr[i].sh_type == SHT_DYNSYM)
	  {
	    dynSec = i;
	    break;
	  }
      if (dynSec != -1)
	{
	  char *symbase = base + shdr[shdr[dynSec].sh_link].sh_offset;
	  ELF_SYM *symbols = (ELF_SYM*) (base + shdr[dynSec].sh_offset);
	  int nextSec = 0;
	  int n = shdr[dynSec].sh_size / shdr[dynSec].sh_entsize;
	  for (i = 0; i < n; i++)
	    {
	      ELF_SYM *sym = symbols + i;
	      TprintfT (DBG_LT2, "process_vsyscall_page: symbol=%d st_name=%lld '%s'\n"
			"  st_size     = 0x%016llx %10lld\n"
			"  st_value    = 0x%016llx %10lld\n"
			"  st_shndx    = 0x%016llx %10lld\n"
			"  st_info     = 0x%016llx %10lld\n",
			i, (long long) sym->st_name, symbase + sym->st_name,
			(long long) sym->st_size, (long long) sym->st_size,
			(long long) sym->st_value, (long long) sym->st_value,
			(long long) sym->st_shndx, (long long) sym->st_shndx,
			(long long) sym->st_info, (long long) sym->st_info);
	      if (sym->st_shndx <= 0 || sym->st_size <= 0 ||
		  ELF_ST_BIND (sym->st_info) != STB_GLOBAL || ELF_ST_TYPE (sym->st_info) != STT_FUNC)
		continue;
	      if (nextSec == 0)
		nextSec = sym->st_shndx;
	      else if (nextSec > sym->st_shndx)
		nextSec = sym->st_shndx;
	    }
	  if (nextSec == 0)
	    ehdr = NULL;

	  while (nextSec != 0)
	    {
	      int curSec = nextSec;
	      char *bgn = base + shdr[curSec].sh_offset;
	      char *end = bgn + shdr[curSec].sh_size;
	      for (i = 0; i < n; i++)
		{
		  ELF_SYM *sym = symbols + i;
		  if (sym->st_shndx <= 0 || sym->st_size <= 0 ||
		      ELF_ST_BIND (sym->st_info) != STB_GLOBAL || ELF_ST_TYPE (sym->st_info) != STT_FUNC)
		    continue;
		  if (sym->st_shndx > curSec)
		    {
		      if (nextSec == curSec)
			nextSec = sym->st_shndx;
		      else if (nextSec > sym->st_shndx)
			nextSec = sym->st_shndx;
		      nextSec = sym->st_shndx;
		      continue;
		    }
		  if (sym->st_shndx != curSec)
		    continue;
		  long long st_delta = (sym->st_value >= shdr[sym->st_shndx].sh_addr) ?
			  (sym->st_value - shdr[sym->st_shndx].sh_addr) : -1;
		  char *st_value = bgn + st_delta;
		  if (st_delta >= 0 && st_value + sym->st_size <= end)
		    {
		      append_segment_record ("<event kind=\"map\" object=\"dynfunc\" name=\"%s\" "
					     "vaddr=\"0x%016lX\" size=\"%u\" funcname=\"%s\" />\n",
					     mapName, (void*) st_value, sym->st_size, symbase + sym->st_name);

		      TprintfT (DBG_LT2, "process_vsyscall_page: append_segment_record map dynfunc='%s' vaddr=%016lX size=%ld funcname='%s'\n",
				mapName, (unsigned long) st_value,
				(long) sym->st_size, symbase + sym->st_name);

		      /* now cache this for a subsequent experiment */
		      if (ndyn >= MAXDYN)
			__collector_log_write ("<event kind=\"%s\" id=\"%d\">MAXDYN=%d</event>\n",
					       SP_JCMD_CERROR, COL_ERROR_MAPCACHE, MAXDYN);
		      else
			{
			  dynname [ndyn] = CALL_UTIL (libc_strdup)(mapName);
			  dynvaddr [ndyn] = (void *) st_value;
			  dynsize [ndyn] = (unsigned) sym->st_size;
			  dynfuncname[ndyn] = CALL_UTIL (libc_strdup)(symbase + sym->st_name);
			  TprintfT (DBG_LT2, "process_vsyscall_page: cached entry %d  map function='%s' vaddr=0x%016lX size=%ld '%s'\n",
				    ndyn, dynname[ndyn], (unsigned long) dynvaddr[ndyn],
				    (long) dynsize[ndyn], dynfuncname[ndyn]);
			  ndyn++;
			}
		    }
		}
	      __collector_int_func_load (DFUNC_KERNEL, mapName, NULL,
					 (void*) (base + shdr[curSec].sh_offset), shdr[curSec].sh_size, 0, NULL);

	      /* now cache this function for a subsequent experiment */
	      if (nvsysfuncs >= MAXVSYSFUNCS)
		__collector_log_write ("<event kind=\"%s\" id=\"%d\">MAXVSYSFUNCS=%d</event>\n",
				       SP_JCMD_CERROR, COL_ERROR_MAPCACHE, MAXVSYSFUNCS);
	      else
		{
		  sysfuncname[nvsysfuncs] = CALL_UTIL (libc_strdup)(mapName);
		  sysfuncvaddr[nvsysfuncs] = (unsigned long) (base + shdr[curSec].sh_offset);
		  sysfuncsize[nvsysfuncs] = (unsigned long) (shdr[curSec].sh_size);
		  TprintfT (DBG_LT2, "process_vsyscall_page: cached entry %d  map function='%s' vaddr=0x%016lX size=%ld\n",
			    nvsysfuncs, sysfuncname[nvsysfuncs],
			    (unsigned long) sysfuncvaddr[nvsysfuncs],
			    (long) sysfuncsize[nvsysfuncs]);
		  nvsysfuncs++;
		}
	      TprintfT (DBG_LT2, "process_vsyscall_page: collector_int_func_load='%s' vaddr=0x%016lX size=%ld\n",
			mapName, (unsigned long) (base + shdr[curSec].sh_offset),
			(long) shdr[curSec].sh_size);
	      if (curSec == nextSec)
		break;
	    }
	}
    }

#if WSIZE(32)
  unsigned long vsysaddr = (unsigned long) 0xffffe000;
#elif WSIZE(64)
  unsigned long vsysaddr = (unsigned long) 0xffffffffff600000;
#endif
  // Make sure the vsyscall map has PROT_EXEC
  MapInfo *mp;
  for (mp = mmaps.next; mp; mp = mp->next)
    {
      TprintfT (DBG_LT2, "MapInfo: vaddr=0x%016llx [size=%lld] mflags=0x%llx offset=%lld pagesize=%lld\n"
		"  mapname='%s'   filename='%s'\n",
		(unsigned long long) mp->vaddr, (long long) mp->size,
		(long long) mp->mflags, (long long) mp->offset, (long long) mp->pagesize,
		mp->mapname ? mp->mapname : "NULL",
		mp->filename ? mp->filename : "NULL");
      if (vsysaddr == mp->vaddr)
	mp->mflags |= PROT_EXEC;
      if ((unsigned long) ehdr == (unsigned long) mp->vaddr)
	continue;
      if (__collector_strncmp (mp->mapname, "[vdso]", 6) == 0
	  || __collector_strncmp (mp->mapname, "[vsyscall]", 10) == 0)
	{
	  /*
	   * On rubbia ( 2.6.9-5.ELsmp #1 SMP 32-bit ) access to ehdr causes SEGV.
	   * There doesn't seem to be a way to reliably determine the actual presence
	   * of the page: even when /proc reports it's there it can't be accessed.
	   * We will have to put up with <Unknown> on some Linuxes until this is resolved.
	  __collector_int_func_load(DFUNC_KERNEL, mp->mapname, NULL, (void*) mp->vaddr, mp->size, 0, NULL);
	   */
	  hrtime_t hrt = GETRELTIME ();
	  append_segment_record (
				 "<event kind=\"map\" object=\"function\" tstamp=\"%u.%09u\" "
				 "vaddr=\"0x%016lX\" size=\"%u\" name=\"%s\" />\n",
				 (unsigned) (hrt / NANOSEC), (unsigned) (hrt % NANOSEC),
				 (unsigned long) mp->vaddr, (unsigned) mp->size, mp->mapname);
	  TprintfT (DBG_LT2, "process_vsyscall_page: append_segment_record map function = %s, vaddr = 0x%016lX, size = %u\n",
		    mp->mapname, (unsigned long) mp->vaddr, (unsigned) mp->size);

	  /* now cache this function for a subsequent experiment */
	  if (nvsysfuncs >= MAXVSYSFUNCS)
	    __collector_log_write ("<event kind=\"%s\" id=\"%d\">MAXVSYSFUNCS=%d</event>\n",
				   SP_JCMD_CERROR, COL_ERROR_MAPCACHE, MAXVSYSFUNCS);
	  else
	    {
	      sysfuncname[nvsysfuncs] = CALL_UTIL (libc_strdup)(mp->mapname);
	      sysfuncvaddr[nvsysfuncs] = mp->vaddr;
	      sysfuncsize[nvsysfuncs] = (unsigned long) mp->size;
	      TprintfT (DBG_LT2, "process_vsyscall_page: cached entry %d  map function='%s' vaddr=0x%016lX size=%ld\n",
			nvsysfuncs, sysfuncname[nvsysfuncs],
			(unsigned long) sysfuncvaddr[nvsysfuncs],
			(long) sysfuncsize[nvsysfuncs]);
	      nvsysfuncs++;

	    }
	}
    }
}

/*
 * collector API for dynamic functions
 */
void collector_func_load () __attribute__ ((weak, alias ("__collector_func_load")));
void
__collector_func_load (char *name, char *alias, char *sourcename,
		       void *vaddr, int size, int lntsize, DT_lineno *lntable)
{
  __collector_int_func_load (DFUNC_API, name, sourcename,
			     vaddr, size, lntsize, lntable);
}

void collector_func_unload () __attribute__ ((weak, alias ("__collector_func_unload")));
void
__collector_func_unload (void *vaddr)
{
  __collector_int_func_unload (DFUNC_API, vaddr);
}

/* routines for handling dynamic functions */
static void
rwrite (int fd, void *buf, size_t nbyte)
{
  size_t left = nbyte;
  size_t res;
  char *ptr = (char*) buf;
  while (left > 0)
    {
      res = CALL_UTIL (write)(fd, ptr, left);
      if (res == -1)
	{
	  TprintfT (0, "ERROR: rwrite(%s) failed: errno=%d\n", dyntext_fname, errno);
	  (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s</event>\n",
					SP_JCMD_CERROR, COL_ERROR_DYNWRITE, errno, dyntext_fname);
	  return;
	}
      left -= res;
      ptr += res;
    }
}

void
__collector_int_func_load (dfunc_mode_t mode, char *name, char *sourcename,
			   void *vaddr, int size, int lntsize, DT_lineno *lntable)
{
  char name_buf[32];
  int slen;
  static char pad[16];
  int padn;
  if (!mmap_initted)
    return;
  hrtime_t hrt = GETRELTIME ();

  if (name == NULL)
    {
      /* generate a name based on vaddr */
      CALL_UTIL (snprintf)(name_buf, sizeof (name_buf), "0x%lx", (unsigned long) vaddr);
      name = name_buf;
    }

  switch (mode)
    {
    case DFUNC_API:
    case DFUNC_KERNEL:
      append_segment_record ("<event kind=\"map\" object=\"function\" tstamp=\"%u.%09u\" "
			     "vaddr=\"0x%016lX\" size=\"%u\" name=\"%s\" />\n",
			     (unsigned) (hrt / NANOSEC), (unsigned) (hrt % NANOSEC),
			     (unsigned long) vaddr, (unsigned) size, name);
      break;
    case DFUNC_JAVA:
      append_segment_record ("<event kind=\"map\" object=\"jcm\" tstamp=\"%u.%09u\" "
			     "vaddr=\"0x%016lX\" size=\"%u\" methodId=\"%s\" />\n",
			     (unsigned) (hrt / NANOSEC), (unsigned) (hrt % NANOSEC),
			     (unsigned long) vaddr, (unsigned) size, name);
      break;
    default:
      return;
    }

  /* 21275311 Unwind failure in native stack for java application running on jdk8 on x86
   * Check:
   *   - function starts in a known segment (base1 != 0)
   *   - function ends in the same segment (base1==base2 && end1==end2)
   * If not, then call update_map_segments().
   */
  unsigned long base1, end1, base2, end2;
  __collector_check_segment ((unsigned long) vaddr, &base1, &end1, 0);
  if (base1)
    __collector_check_segment (((unsigned long) vaddr)+((unsigned long) size), &base2, &end2, 0);
  if (base1 == 0 || base1 != base2 || end1 != end2)
    __collector_ext_update_map_segments ();

  /* Write a copy of actual code to the "dyntext" file */
  DT_header dt_hdr;
  dt_hdr.type = DT_HEADER;
  dt_hdr.size = sizeof (dt_hdr);
  dt_hdr.time = hrt;
  unsigned long t = (unsigned long) vaddr; /* to suppress a warning from gcc */
  dt_hdr.vaddr = (uint64_t) t;

  DT_code dt_code;
  dt_code.type = DT_CODE;
  void *code = vaddr;
  if (vaddr != NULL && size > 0)
    {
      dt_code.size = sizeof (dt_code) + ((size + 0xf) & ~0xf);
      if (mode == DFUNC_KERNEL)
	{
	  /* Some Linuxes don't accept vaddrs from the vsyscall
	   * page in write(). Make a copy.
	   */
	  code = alloca (size);
	  __collector_memcpy (code, vaddr, size);
	}
    }
  else
    dt_code.size = 0;

  DT_srcfile dt_src;
  dt_src.type = DT_SRCFILE;
  if (sourcename)
    {
      slen = CALL_UTIL (strlen)(sourcename) + 1;
      dt_src.size = slen ? sizeof (dt_src) + ((slen + 0xf) & ~0xf) : 0;
    }
  else
    {
      slen = 0;
      dt_src.size = 0;
    }

  DT_ltable dt_ltbl;
  dt_ltbl.type = DT_LTABLE;
  if (lntable != NULL && lntsize > 0)
    dt_ltbl.size = sizeof (dt_ltbl) + lntsize * sizeof (DT_lineno);
  else
    dt_ltbl.size = 0;

  int fd = CALL_UTIL (open)(dyntext_fname, O_RDWR | O_APPEND);
  if (fd == -1)
    {
      TprintfT (0, "ERROR: __collector_int_func_load: open(%s) failed: errno=%d\n",
		dyntext_fname, errno);
      (void) __collector_log_write ("<event kind=\"%s\" id=\"%d\" ec=\"%d\">%s</event>\n",
				    SP_JCMD_CERROR, COL_ERROR_DYNOPEN, errno, dyntext_fname);
      return;
    }

  /* Lock the whole file */
  __collector_mutex_lock (&dyntext_lock);
  rwrite (fd, &dt_hdr, sizeof (dt_hdr));
  if (dt_code.size)
    {
      padn = dt_code.size - sizeof (dt_code) - size;
      rwrite (fd, &dt_code, sizeof (dt_code));
      rwrite (fd, code, size);
      rwrite (fd, &pad, padn);
    }
  if (dt_src.size)
    {
      padn = dt_src.size - sizeof (dt_src) - slen;
      rwrite (fd, &dt_src, sizeof (dt_src));
      rwrite (fd, sourcename, slen);
      rwrite (fd, &pad, padn);
    }
  if (dt_ltbl.size)
    {
      rwrite (fd, &dt_ltbl, sizeof (dt_ltbl));
      rwrite (fd, lntable, dt_ltbl.size - sizeof (dt_ltbl));
    }

    /* Unlock the file */
    __collector_mutex_unlock( &dyntext_lock );
    CALL_UTIL(close( fd ) );
}

void
__collector_int_func_unload (dfunc_mode_t mode, void *vaddr)
{
  if (!mmap_initted)
    return;
  hrtime_t hrt = GETRELTIME ();
  if (mode == DFUNC_API)
    append_segment_record ("<event kind=\"unmap\" tstamp=\"%u.%09u\" vaddr=\"0x%016lX\"/>\n",
			   (unsigned) (hrt / NANOSEC), (unsigned) (hrt % NANOSEC), (unsigned long) vaddr);
  else if (mode == DFUNC_JAVA)
    /* note that the "vaddr" is really a method id, not an address */
    append_segment_record ("<event kind=\"unmap\" tstamp=\"%u.%09u\" methodId=\"0x%016lX\"/>\n",
			   (unsigned) (hrt / NANOSEC), (unsigned) (hrt % NANOSEC), (unsigned long) vaddr);
  else
    return;
}

/*
 * int init_mmap_intf()
 *      Set up interposition (if not already done).
 */
static int
init_mmap_intf ()
{
  if (__collector_dlsym_guard)
    return 1;
  void *dlflag = RTLD_NEXT;
  __real_mmap = dlsym (dlflag, "mmap");
  if (__real_mmap == NULL)
    {

      /* We are probably dlopened after libthread/libc,
       * try to search in the previously loaded objects
       */
      __real_mmap = dlsym (RTLD_DEFAULT, "mmap");
      if (__real_mmap == NULL)
	{
	  TprintfT (0, "ERROR: collector real mmap not found\n");
	  return 1;
	}
      TprintfT (DBG_LT2, "collector real mmap found with RTLD_DEFAULT\n");
      dlflag = RTLD_DEFAULT;
    }

  __real_mmap64 = dlsym (dlflag, "mmap64");
  __real_munmap = dlsym (dlflag, "munmap");

  // dlopen/dlmopen/dlclose are in libdl.so
  __real_dlopen_2_34 = dlvsym (dlflag, "dlopen", "GLIBC_2.34");
  __real_dlopen_2_17 = dlvsym (dlflag, "dlopen", "GLIBC_2.17");
  __real_dlopen_2_2_5 = dlvsym (dlflag, "dlopen", "GLIBC_2.2.5");
  __real_dlopen_2_1 = dlvsym (dlflag, "dlopen", "GLIBC_2.1");
  __real_dlopen_2_0 = dlvsym (dlflag, "dlopen", "GLIBC_2.0");
  if (__real_dlopen_2_34)
    __real_dlopen = __real_dlopen_2_34;
  else if (__real_dlopen_2_17)
    __real_dlopen = __real_dlopen_2_17;
  else if (__real_dlopen_2_2_5)
    __real_dlopen = __real_dlopen_2_2_5;
  else if (__real_dlopen_2_1)
    __real_dlopen = __real_dlopen_2_1;
  else if (__real_dlopen_2_0)
    __real_dlopen = __real_dlopen_2_0;
  else
    __real_dlopen = dlsym (dlflag, "dlopen");
    
  __real_dlclose_2_34 = dlvsym (dlflag, "dlclose", "GLIBC_2.34");
  __real_dlclose_2_17 = dlvsym (dlflag, "dlclose", "GLIBC_2.17");
  __real_dlclose_2_2_5 = dlvsym (dlflag, "dlclose", "GLIBC_2.2.5");
  __real_dlclose_2_0 = dlvsym (dlflag, "dlclose", "GLIBC_2.0");
  if (__real_dlclose_2_34)
    __real_dlclose = __real_dlclose_2_34;
  else if (__real_dlclose_2_17)
    __real_dlclose = __real_dlclose_2_17;
  else if (__real_dlclose_2_2_5)
    __real_dlclose = __real_dlclose_2_2_5;
  else if (__real_dlclose_2_0)
    __real_dlclose = __real_dlclose_2_0;
  else
    __real_dlclose = dlsym (dlflag, "dlclose");

#define PR_FUNC(f)  TprintfT (DBG_LT2, " mmaptrace.c: " #f ": @%p\n", f)
  PR_FUNC (__real_dlclose);
  PR_FUNC (__real_dlclose_2_0);
  PR_FUNC (__real_dlclose_2_17);
  PR_FUNC (__real_dlclose_2_2_5);
  PR_FUNC (__real_dlclose_2_34);
  PR_FUNC (__real_dlopen);
  PR_FUNC (__real_dlopen_2_0);
  PR_FUNC (__real_dlopen_2_1);
  PR_FUNC (__real_dlopen_2_17);
  PR_FUNC (__real_dlopen_2_2_5);
  PR_FUNC (__real_dlopen_2_34);
  PR_FUNC (__real_mmap);
  PR_FUNC (__real_mmap64);
  PR_FUNC (__real_munmap);

  return 0;
}

/*------------------------------------------------------------- mmap */
void *
mmap (void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
  int err = 0;
  if (NULL_PTR (mmap))
    err = init_mmap_intf ();
  if (err)
    return MAP_FAILED;

  /* hrtime_t hrt = GETRELTIME(); */
  void *ret = CALL_REAL (mmap)(start, length, prot, flags, fd, offset);

  if (!CHCK_REENTRANCE && (ret != MAP_FAILED) && collector_heap_record != NULL)
    {
      PUSH_REENTRANCE;
      /* write a separate record for mmap tracing */
      collector_heap_record (MMAP_TRACE, length, ret);
      POP_REENTRANCE;
    }
  TprintfT (DBG_LT2, "libcollector.mmap(%p, %ld, %d, %d, %d, 0x%lld) = %p\n",
	    start, (long) length, prot, flags, fd, (long long) offset, ret);
  return ret;
}

/*------------------------------------------------------------- mmap64 */
#if WSIZE(32) && !defined(__USE_FILE_OFFSET64)

void *
mmap64 (void *start, size_t length, int prot, int flags, int fd, off64_t offset)
{
  if (NULL_PTR (mmap64))
    init_mmap_intf ();

  /* hrtime_t hrt = GETRELTIME(); */
  void *ret = CALL_REAL (mmap64)(start, length, prot, flags, fd, offset);
  if (!CHCK_REENTRANCE && (ret != MAP_FAILED) && collector_heap_record != NULL)
    {
      PUSH_REENTRANCE;
      /* write a separate record for mmap tracing */
      collector_heap_record (MMAP_TRACE, length, ret);
      POP_REENTRANCE;
    }
  TprintfT (DBG_LT2, "libcollector.mmap64(%p, %ld, %d, %d, %d, 0x%lld) = %p\n",
	    start, (long) length, prot, flags, fd, (long long) offset, ret);
  return ret;
}
#endif /* WSIZE(32) */

/*------------------------------------------------------------- munmap */
int
munmap (void *start, size_t length)
{
  if (NULL_PTR (munmap))
    init_mmap_intf ();

  /* hrtime_t hrt = GETRELTIME(); */
  int rc = CALL_REAL (munmap)(start, length);
  if (!CHCK_REENTRANCE && (rc == 0) && collector_heap_record != NULL)
    {
      PUSH_REENTRANCE;
      /* write a separate record for mmap tracing */
      collector_heap_record (MUNMAP_TRACE, length, start);
      POP_REENTRANCE;
    }
  TprintfT (DBG_LT2, "libcollector.munmap(%p, %ld) = %d\n", start, (long) length, rc);
  return rc;
}


/*------------------------------------------------------------- dlopen */
static void *
gprofng_dlopen (void*(real_dlopen) (const char *, int),
		void *caller, const char *pathname, int mode)
{
  const char * real_pathname = pathname;
  char new_pathname[MAXPATHLEN];
  int origin_offset = 0;
  TprintfT (DBG_LT2, "dlopen: pathname=%s, mode=%d\n", pathname ? pathname : "NULL", mode);
  if (pathname && __collector_strStartWith (pathname, "$ORIGIN/") == 0)
    origin_offset = 8;
  else if (pathname && __collector_strStartWith (pathname, "${ORIGIN}/") == 0)
    origin_offset = 10;
  if (origin_offset)
    {
      Dl_info dl_info;
      if (caller && dladdr (caller, &dl_info) != 0)
	{
	  TprintfT (DBG_LT2, "dladdr(%p): %p fname=%s\n",
		    caller, dl_info.dli_fbase, dl_info.dli_fname);
	  new_pathname[0] = '\0';
	  const char *p = __collector_strrchr (dl_info.dli_fname, '/');
	  if (p)
	    __collector_strlcpy (new_pathname, dl_info.dli_fname,
				 (p - dl_info.dli_fname + 2) < MAXPATHLEN ?
				   (p - dl_info.dli_fname + 2) : MAXPATHLEN);
	  __collector_strlcat (new_pathname, pathname + origin_offset,
			   MAXPATHLEN - CALL_UTIL (strlen)(new_pathname));
	  real_pathname = new_pathname;
	}
      else
	TprintfT (0, "ERROR: dladdr(%p): %s\n", caller, dlerror ());
    }
  TprintfT (DBG_LT2, "libcollector.dlopen(%s,%d) interposing\n",
	    pathname ? pathname : "", mode);
  void *ret = NULL;

  // set guard for duration of handling dlopen, since want to ensure
  // new mappings are resolved after the actual dlopen has occurred
  PUSH_REENTRANCE;
  hrtime_t hrt = GETRELTIME ();

  if (caller && real_pathname && !__collector_strchr (real_pathname, '/'))
    ret = dlopen_searchpath (real_dlopen, caller, real_pathname, mode);

  if (!ret)
    ret = real_dlopen (real_pathname, mode);
  TprintfT (DBG_LT2, "libcollector -- dlopen(%s) returning %p\n", pathname, ret);

  /* Don't call update if dlopen failed: preserve dlerror() */
  if (ret && (mmap_mode > 0) && !(mode & RTLD_NOLOAD))
    update_map_segments (hrt, 1);
  TprintfT (DBG_LT2, "libcollector -- dlopen(%s) returning %p\n", pathname, ret);
  POP_REENTRANCE;
  return ret;
}

#define DCL_DLOPEN(dcl_f) \
void *dcl_f (const char *pathname, int mode) \
  { \
    if (__real_dlopen == NULL) \
      init_mmap_intf (); \
    void *caller = __builtin_return_address (0); \
    return gprofng_dlopen (__real_dlopen, caller, pathname, mode); \
  }

DCL_FUNC_VER (DCL_DLOPEN, dlopen_2_34, dlopen@GLIBC_2.34)
DCL_FUNC_VER (DCL_DLOPEN, dlopen_2_17, dlopen@GLIBC_2.17)
DCL_FUNC_VER (DCL_DLOPEN, dlopen_2_2_5, dlopen@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_DLOPEN, dlopen_2_1, dlopen@GLIBC_2.1)
DCL_FUNC_VER (DCL_DLOPEN, dlopen_2_0, dlopen@GLIBC_2.0)
DCL_DLOPEN (dlopen)

/*------------------------------------------------------------- dlclose */
static int
gprofng_dlclose (int (real_dlclose) (void *), void *handle)
{
  hrtime_t hrt = GETRELTIME ();
  if (!CHCK_REENTRANCE)
    {
      PUSH_REENTRANCE;
      update_map_segments (hrt, 1);
      POP_REENTRANCE;
      hrt = GETRELTIME ();
    }
  int ret = real_dlclose (handle);

  /* Don't call update if dlclose failed: preserve dlerror() */
  if (!ret && !CHCK_REENTRANCE)
    {
      PUSH_REENTRANCE;
      update_map_segments (hrt, 1);
      POP_REENTRANCE;
    }
  TprintfT (DBG_LT2, "gprofng_dlclose @%p (%p) returning %d\n", real_dlclose,
	    handle, ret);
  return ret;
}

#define DCL_DLCLOSE(dcl_f) \
int dcl_f (void *handle) \
  { \
    if (__real_dlclose == NULL) \
      init_mmap_intf (); \
    return gprofng_dlclose (__real_dlclose, handle); \
  }

DCL_FUNC_VER (DCL_DLCLOSE, dlclose_2_34, dlclose@GLIBC_2.34)
DCL_FUNC_VER (DCL_DLCLOSE, dlclose_2_17, dlclose@GLIBC_2.17)
DCL_FUNC_VER (DCL_DLCLOSE, dlclose_2_2_5, dlclose@GLIBC_2.2.5)
DCL_FUNC_VER (DCL_DLCLOSE, dlclose_2_0, dlclose@GLIBC_2.0)
DCL_DLCLOSE (dlclose)
