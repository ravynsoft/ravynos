/* Iteration over virtual memory areas.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2011-2017.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* On Solaris in 32-bit mode, when gnulib module 'largefile' is in use,
   prevent a compilation error
     "Cannot use procfs in the large file compilation environment"
   while also preventing <sys/types.h> from not defining off_t.
   On Android, when targeting Android 4.4 or older with a GCC toolchain,
   prevent a compilation error
     "error: call to 'mmap' declared with attribute error: mmap is not
      available with _FILE_OFFSET_BITS=64 when using GCC until android-21.
      Either raise your minSdkVersion, disable _FILE_OFFSET_BITS=64, or
      switch to Clang."
   The files that we access in this compilation unit are less than 2 GB
   large.  */
#if defined __sun && !defined _LP64 && _FILE_OFFSET_BITS == 64
# undef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS 32
#endif
#ifdef __ANDROID__
# undef _FILE_OFFSET_BITS
#endif

/* Specification.  */
#include "vma-iter.h"

#include <errno.h> /* errno */
#include <stdlib.h> /* size_t */
#include <fcntl.h> /* open, O_RDONLY */
#include <unistd.h> /* getpagesize, lseek, read, close, getpid */

#if defined __linux__ || defined __ANDROID__
# include <limits.h> /* PATH_MAX */
#endif

#if defined __linux__ || defined __ANDROID__ || defined __FreeBSD_kernel__ || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __minix /* || defined __CYGWIN__ */
# include <sys/types.h>
# include <sys/mman.h> /* mmap, munmap */
#endif
#if defined __minix
# include <string.h> /* memcpy */
#endif

#if defined __FreeBSD__ || defined __FreeBSD_kernel__ /* FreeBSD, GNU/kFreeBSD */
# include <sys/types.h>
# include <sys/mman.h> /* mmap, munmap */
# include <sys/param.h> /* prerequisite of <sys/user.h> */
# include <sys/user.h> /* struct kinfo_vmentry */
# include <sys/sysctl.h> /* sysctl */
#endif
#if defined __NetBSD__ || defined __OpenBSD__ /* NetBSD, OpenBSD */
# include <sys/types.h>
# include <sys/mman.h> /* mmap, munmap */
# include <sys/sysctl.h> /* sysctl, struct kinfo_vmentry */
#endif

#if defined _AIX /* AIX */
# include <string.h> /* memcpy */
# include <sys/types.h>
# include <sys/mman.h> /* mmap, munmap */
# include <sys/procfs.h> /* prmap_t */
#endif

#if defined __sgi || defined __osf__ /* IRIX, OSF/1 */
# include <string.h> /* memcpy */
# include <sys/types.h>
# include <sys/mman.h> /* mmap, munmap */
# include <sys/procfs.h> /* PIOC*, prmap_t */
#endif

#if defined __sun /* Solaris */
# include <string.h> /* memcpy */
# include <sys/types.h>
# include <sys/mman.h> /* mmap, munmap */
/* Try to use the newer ("structured") /proc filesystem API, if supported.  */
# define _STRUCTURED_PROC 1
# include <sys/procfs.h> /* prmap_t, optionally PIOC* */
#endif

#if HAVE_PSTAT_GETPROCVM /* HP-UX */
# include <sys/pstat.h> /* pstat_getprocvm */
#endif

#if defined __APPLE__ && defined __MACH__ /* Mac OS X */
# include <mach/mach.h>
#endif

#if defined __GNU__ /* GNU/Hurd */
# include <mach/mach.h>
#endif

#if defined _WIN32 || defined __CYGWIN__ /* Windows */
# include <windows.h>
#endif

#if defined __BEOS__ || defined __HAIKU__ /* BeOS, Haiku */
# include <OS.h>
#endif

#if HAVE_MQUERY /* OpenBSD */
# include <sys/types.h>
# include <sys/mman.h> /* mquery */
#endif


/* Support for reading text files in the /proc file system.  */

#if defined __linux__ || defined __ANDROID__ || defined __FreeBSD_kernel__ || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __minix /* || defined __CYGWIN__ */

/* Buffered read-only streams.
   We cannot use <stdio.h> here, because fopen() calls malloc(), and a malloc()
   call may call mmap() and thus pre-allocate available memory.
   Also, we cannot use multiple read() calls, because if the buffer size is
   smaller than the file's contents:
     - On NetBSD, the second read() call would return 0, thus making the file
       appear truncated.
     - On DragonFly BSD, the first read() call would fail with errno = EFBIG.
     - On all platforms, if some other thread is doing memory allocations or
       deallocations between two read() calls, there is a high risk that the
       result of these two read() calls don't fit together, and as a
       consequence we will parse garbage and either omit some VMAs or return
       VMAs with nonsensical addresses.
   So use mmap(), and ignore the resulting VMA.  */

# if defined __linux__ || defined __ANDROID__
  /* On Linux, if the file does not entirely fit into the buffer, the read()
     function stops before the line that would come out truncated.  The
     maximum size of such a line is 73 + PATH_MAX bytes.  To be sure that we
     have read everything, we must verify that at least that many bytes are
     left when read() returned.  */
#  define MIN_LEFTOVER (73 + PATH_MAX)
# else
#  define MIN_LEFTOVER 1
# endif

# ifdef TEST
/* During testing, we want to run into the hairy cases.  */
#  define STACK_ALLOCATED_BUFFER_SIZE 32
# else
#  if MIN_LEFTOVER < 1024
#   define STACK_ALLOCATED_BUFFER_SIZE 1024
#  else
    /* There is no point in using a stack-allocated buffer if it is too small anyway.  */
#   define STACK_ALLOCATED_BUFFER_SIZE 1
#  endif
# endif

struct rofile
  {
    size_t position;
    size_t filled;
    int eof_seen;
    /* These fields deal with allocation of the buffer.  */
    char *buffer;
    char *auxmap;
    size_t auxmap_length;
    unsigned long auxmap_start;
    unsigned long auxmap_end;
    char stack_allocated_buffer[STACK_ALLOCATED_BUFFER_SIZE];
  };

/* Open a read-only file stream.  */
static int
rof_open (struct rofile *rof, const char *filename)
{
  int fd;
  unsigned long pagesize;
  size_t size;

  fd = open (filename, O_RDONLY | O_CLOEXEC);
  if (fd < 0)
    return -1;
  rof->position = 0;
  rof->eof_seen = 0;
  /* Try the static buffer first.  */
  pagesize = 0;
  rof->buffer = rof->stack_allocated_buffer;
  size = sizeof (rof->stack_allocated_buffer);
  rof->auxmap = NULL;
  rof->auxmap_start = 0;
  rof->auxmap_end = 0;
  for (;;)
    {
      /* Attempt to read the contents in a single system call.  */
      if (size > MIN_LEFTOVER)
        {
          int n = read (fd, rof->buffer, size);
          if (n < 0 && errno == EINTR)
            goto retry;
# if defined __DragonFly__
          if (!(n < 0 && errno == EFBIG))
# endif
            {
              if (n <= 0)
                /* Empty file.  */
                goto fail1;
              if (n + MIN_LEFTOVER <= size)
                {
                  /* The buffer was sufficiently large.  */
                  rof->filled = n;
# if defined __linux__ || defined __ANDROID__
                  /* On Linux, the read() call may stop even if the buffer was
                     large enough.  We need the equivalent of full_read().  */
                  for (;;)
                    {
                      n = read (fd, rof->buffer + rof->filled, size - rof->filled);
                      if (n < 0 && errno == EINTR)
                        goto retry;
                      if (n < 0)
                        /* Some error.  */
                        goto fail1;
                      if (n + MIN_LEFTOVER > size - rof->filled)
                        /* Allocate a larger buffer.  */
                        break;
                      if (n == 0)
                        {
                          /* Reached the end of file.  */
                          close (fd);
                          return 0;
                        }
                      rof->filled += n;
                    }
# else
                  close (fd);
                  return 0;
# endif
                }
            }
        }
      /* Allocate a larger buffer.  */
      if (pagesize == 0)
        {
          pagesize = getpagesize ();
          size = pagesize;
          while (size <= MIN_LEFTOVER)
            size = 2 * size;
        }
      else
        {
          size = 2 * size;
          if (size == 0)
            /* Wraparound.  */
            goto fail1;
          if (rof->auxmap != NULL)
            munmap (rof->auxmap, rof->auxmap_length);
        }
      rof->auxmap = (void *) mmap ((void *) 0, size, PROT_READ | PROT_WRITE,
                                   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      if (rof->auxmap == (void *) -1)
        {
          close (fd);
          return -1;
        }
      rof->auxmap_length = size;
      rof->auxmap_start = (unsigned long) rof->auxmap;
      rof->auxmap_end = rof->auxmap_start + size;
      rof->buffer = (char *) rof->auxmap;
     retry:
      /* Restart.  */
      if (lseek (fd, 0, SEEK_SET) < 0)
        {
          close (fd);
          fd = open (filename, O_RDONLY | O_CLOEXEC);
          if (fd < 0)
            goto fail2;
        }
    }
 fail1:
  close (fd);
 fail2:
  if (rof->auxmap != NULL)
    munmap (rof->auxmap, rof->auxmap_length);
  return -1;
}

/* Return the next byte from a read-only file stream without consuming it,
   or -1 at EOF.  */
static int
rof_peekchar (struct rofile *rof)
{
  if (rof->position == rof->filled)
    {
      rof->eof_seen = 1;
      return -1;
    }
  return (unsigned char) rof->buffer[rof->position];
}

/* Return the next byte from a read-only file stream, or -1 at EOF.  */
static int
rof_getchar (struct rofile *rof)
{
  int c = rof_peekchar (rof);
  if (c >= 0)
    rof->position++;
  return c;
}

/* Parse an unsigned hexadecimal number from a read-only file stream.  */
static int
rof_scanf_lx (struct rofile *rof, unsigned long *valuep)
{
  unsigned long value = 0;
  unsigned int numdigits = 0;
  for (;;)
    {
      int c = rof_peekchar (rof);
      if (c >= '0' && c <= '9')
        value = (value << 4) + (c - '0');
      else if (c >= 'A' && c <= 'F')
        value = (value << 4) + (c - 'A' + 10);
      else if (c >= 'a' && c <= 'f')
        value = (value << 4) + (c - 'a' + 10);
      else
        break;
      rof_getchar (rof);
      numdigits++;
    }
  if (numdigits == 0)
    return -1;
  *valuep = value;
  return 0;
}

/* Close a read-only file stream.  */
static void
rof_close (struct rofile *rof)
{
  if (rof->auxmap != NULL)
    munmap (rof->auxmap, rof->auxmap_length);
}

#endif


/* Support for reading the info from a text file in the /proc file system.  */

#if defined __linux__ || defined __ANDROID__ || (defined __FreeBSD_kernel__ && !defined __FreeBSD__) /* || defined __CYGWIN__ */
/* GNU/kFreeBSD mounts /proc as linprocfs, which looks like a Linux /proc
   file system.  */

static int
vma_iterate_proc (vma_iterate_callback_fn callback, void *data)
{
  struct rofile rof;

  /* Open the current process' maps file.  It describes one VMA per line.  */
  if (rof_open (&rof, "/proc/self/maps") >= 0)
    {
      unsigned long auxmap_start = rof.auxmap_start;
      unsigned long auxmap_end = rof.auxmap_end;

      for (;;)
        {
          unsigned long start, end;
          unsigned int flags;
          int c;

          /* Parse one line.  First start and end.  */
          if (!(rof_scanf_lx (&rof, &start) >= 0
                && rof_getchar (&rof) == '-'
                && rof_scanf_lx (&rof, &end) >= 0))
            break;
          /* Then the flags.  */
          do
            c = rof_getchar (&rof);
          while (c == ' ');
          flags = 0;
          if (c == 'r')
            flags |= VMA_PROT_READ;
          c = rof_getchar (&rof);
          if (c == 'w')
            flags |= VMA_PROT_WRITE;
          c = rof_getchar (&rof);
          if (c == 'x')
            flags |= VMA_PROT_EXECUTE;
          while (c = rof_getchar (&rof), c != -1 && c != '\n')
            ;

          if (start <= auxmap_start && auxmap_end - 1 <= end - 1)
            {
              /* Consider [start,end-1] \ [auxmap_start,auxmap_end-1]
                 = [start,auxmap_start-1] u [auxmap_end,end-1].  */
              if (start < auxmap_start)
                if (callback (data, start, auxmap_start, flags))
                  break;
              if (auxmap_end - 1 < end - 1)
                if (callback (data, auxmap_end, end, flags))
                  break;
            }
          else
            {
              if (callback (data, start, end, flags))
                break;
            }
        }
      rof_close (&rof);
      return 0;
    }

  return -1;
}

#elif defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__

static int
vma_iterate_proc (vma_iterate_callback_fn callback, void *data)
{
  struct rofile rof;

  /* Open the current process' maps file.  It describes one VMA per line.  */
  if (rof_open (&rof, "/proc/curproc/map") >= 0)
    {
      unsigned long auxmap_start = rof.auxmap_start;
      unsigned long auxmap_end = rof.auxmap_end;

      for (;;)
        {
          unsigned long start, end;
          unsigned int flags;
          int c;

          /* Parse one line.  First start.  */
          if (!(rof_getchar (&rof) == '0'
                && rof_getchar (&rof) == 'x'
                && rof_scanf_lx (&rof, &start) >= 0))
            break;
          while (c = rof_peekchar (&rof), c == ' ' || c == '\t')
            rof_getchar (&rof);
          /* Then end.  */
          if (!(rof_getchar (&rof) == '0'
                && rof_getchar (&rof) == 'x'
                && rof_scanf_lx (&rof, &end) >= 0))
            break;
# if defined __FreeBSD__ || defined __DragonFly__
          /* Then the resident pages count.  */
          do
            c = rof_getchar (&rof);
          while (c == ' ');
          do
            c = rof_getchar (&rof);
          while (c != -1 && c != '\n' && c != ' ');
          /* Then the private resident pages count.  */
          do
            c = rof_getchar (&rof);
          while (c == ' ');
          do
            c = rof_getchar (&rof);
          while (c != -1 && c != '\n' && c != ' ');
          /* Then some kernel address.  */
          do
            c = rof_getchar (&rof);
          while (c == ' ');
          do
            c = rof_getchar (&rof);
          while (c != -1 && c != '\n' && c != ' ');
# endif
          /* Then the flags.  */
          do
            c = rof_getchar (&rof);
          while (c == ' ');
          flags = 0;
          if (c == 'r')
            flags |= VMA_PROT_READ;
          c = rof_getchar (&rof);
          if (c == 'w')
            flags |= VMA_PROT_WRITE;
          c = rof_getchar (&rof);
          if (c == 'x')
            flags |= VMA_PROT_EXECUTE;
          while (c = rof_getchar (&rof), c != -1 && c != '\n')
            ;

          if (start <= auxmap_start && auxmap_end - 1 <= end - 1)
            {
              /* Consider [start,end-1] \ [auxmap_start,auxmap_end-1]
                 = [start,auxmap_start-1] u [auxmap_end,end-1].  */
              if (start < auxmap_start)
                if (callback (data, start, auxmap_start, flags))
                  break;
              if (auxmap_end - 1 < end - 1)
                if (callback (data, auxmap_end, end, flags))
                  break;
            }
          else
            {
              if (callback (data, start, end, flags))
                break;
            }
        }
      rof_close (&rof);
      return 0;
    }

  return -1;
}

#elif defined __minix

static int
vma_iterate_proc (vma_iterate_callback_fn callback, void *data)
{
  char fnamebuf[6+10+4+1];
  char *fname;
  struct rofile rof;

  /* Construct fname = sprintf (fnamebuf+i, "/proc/%u/map", getpid ()).  */
  fname = fnamebuf + sizeof (fnamebuf) - (4 + 1);
  memcpy (fname, "/map", 4 + 1);
  {
    unsigned int value = getpid ();
    do
      *--fname = (value % 10) + '0';
    while ((value = value / 10) > 0);
  }
  fname -= 6;
  memcpy (fname, "/proc/", 6);

  /* Open the current process' maps file.  It describes one VMA per line.  */
  if (rof_open (&rof, fname) >= 0)
    {
      unsigned long auxmap_start = rof.auxmap_start;
      unsigned long auxmap_end = rof.auxmap_end;

      for (;;)
        {
          unsigned long start, end;
          unsigned int flags;
          int c;

          /* Parse one line.  First start and end.  */
          if (!(rof_scanf_lx (&rof, &start) >= 0
                && rof_getchar (&rof) == '-'
                && rof_scanf_lx (&rof, &end) >= 0))
            break;
          /* Then the flags.  */
          do
            c = rof_getchar (&rof);
          while (c == ' ');
          flags = 0;
          if (c == 'r')
            flags |= VMA_PROT_READ;
          c = rof_getchar (&rof);
          if (c == 'w')
            flags |= VMA_PROT_WRITE;
          c = rof_getchar (&rof);
          if (c == 'x')
            flags |= VMA_PROT_EXECUTE;
          while (c = rof_getchar (&rof), c != -1 && c != '\n')
            ;

          if (start <= auxmap_start && auxmap_end - 1 <= end - 1)
            {
              /* Consider [start,end-1] \ [auxmap_start,auxmap_end-1]
                 = [start,auxmap_start-1] u [auxmap_end,end-1].  */
              if (start < auxmap_start)
                if (callback (data, start, auxmap_start, flags))
                  break;
              if (auxmap_end - 1 < end - 1)
                if (callback (data, auxmap_end, end, flags))
                  break;
            }
          else
            {
              if (callback (data, start, end, flags))
                break;
            }
        }
      rof_close (&rof);
      return 0;
    }

  return -1;
}

#else

static inline int
vma_iterate_proc (vma_iterate_callback_fn callback, void *data)
{
  return -1;
}

#endif


/* Support for reading the info from the BSD sysctl() system call.  */

#if (defined __FreeBSD__ || defined __FreeBSD_kernel__) && defined KERN_PROC_VMMAP /* FreeBSD >= 7.1 */

static int
vma_iterate_bsd (vma_iterate_callback_fn callback, void *data)
{
  /* Documentation: https://www.freebsd.org/cgi/man.cgi?sysctl(3)  */
  int info_path[] = { CTL_KERN, KERN_PROC, KERN_PROC_VMMAP, getpid () };
  size_t len;
  size_t pagesize;
  size_t memneed;
  void *auxmap;
  unsigned long auxmap_start;
  unsigned long auxmap_end;
  char *mem;
  char *p;
  char *p_end;

  len = 0;
  if (sysctl (info_path, 4, NULL, &len, NULL, 0) < 0)
    return -1;
  /* Allow for small variations over time.  In a multithreaded program
     new VMAs can be allocated at any moment.  */
  len = 2 * len + 200;
  /* Allocate memneed bytes of memory.
     We cannot use alloca here, because not much stack space is guaranteed.
     We also cannot use malloc here, because a malloc() call may call mmap()
     and thus pre-allocate available memory.
     So use mmap(), and ignore the resulting VMA.  */
  pagesize = getpagesize ();
  memneed = len;
  memneed = ((memneed - 1) / pagesize + 1) * pagesize;
  auxmap = (void *) mmap ((void *) 0, memneed, PROT_READ | PROT_WRITE,
                          MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (auxmap == (void *) -1)
    return -1;
  auxmap_start = (unsigned long) auxmap;
  auxmap_end = auxmap_start + memneed;
  mem = (char *) auxmap;
  if (sysctl (info_path, 4, mem, &len, NULL, 0) < 0)
    {
      munmap (auxmap, memneed);
      return -1;
    }
  p = mem;
  p_end = mem + len;
  while (p < p_end)
    {
      struct kinfo_vmentry *kve = (struct kinfo_vmentry *) p;
      unsigned long start = kve->kve_start;
      unsigned long end = kve->kve_end;
      unsigned int flags = 0;
      if (kve->kve_protection & KVME_PROT_READ)
        flags |= VMA_PROT_READ;
      if (kve->kve_protection & KVME_PROT_WRITE)
        flags |= VMA_PROT_WRITE;
      if (kve->kve_protection & KVME_PROT_EXEC)
        flags |= VMA_PROT_EXECUTE;
      if (start <= auxmap_start && auxmap_end - 1 <= end - 1)
        {
          /* Consider [start,end-1] \ [auxmap_start,auxmap_end-1]
             = [start,auxmap_start-1] u [auxmap_end,end-1].  */
          if (start < auxmap_start)
            if (callback (data, start, auxmap_start, flags))
              break;
          if (auxmap_end - 1 < end - 1)
            if (callback (data, auxmap_end, end, flags))
              break;
        }
      else
        {
          if (callback (data, start, end, flags))
            break;
        }
      p += kve->kve_structsize;
    }
  munmap (auxmap, memneed);
  return 0;
}

#elif defined __NetBSD__ && defined VM_PROC_MAP /* NetBSD >= 8.0 */

static int
vma_iterate_bsd (vma_iterate_callback_fn callback, void *data)
{
  /* Documentation: https://man.netbsd.org/man/sysctl+7  */
  unsigned int entry_size =
    /* If we wanted to have the path of each entry, we would need
       sizeof (struct kinfo_vmentry).  But we need only the non-string
       parts of each entry.  */
    offsetof (struct kinfo_vmentry, kve_path);
  int info_path[] = { CTL_VM, VM_PROC, VM_PROC_MAP, getpid (), entry_size };
  size_t len;
  size_t pagesize;
  size_t memneed;
  void *auxmap;
  unsigned long auxmap_start;
  unsigned long auxmap_end;
  char *mem;
  char *p;
  char *p_end;

  len = 0;
  if (sysctl (info_path, 5, NULL, &len, NULL, 0) < 0)
    return -1;
  /* Allow for small variations over time.  In a multithreaded program
     new VMAs can be allocated at any moment.  */
  len = 2 * len + 10 * entry_size;
  /* But the system call rejects lengths > 1 MB.  */
  if (len > 0x100000)
    len = 0x100000;
  /* And the system call causes a kernel panic if the length is not a multiple
     of entry_size.  */
  len = (len / entry_size) * entry_size;
  /* Allocate memneed bytes of memory.
     We cannot use alloca here, because not much stack space is guaranteed.
     We also cannot use malloc here, because a malloc() call may call mmap()
     and thus pre-allocate available memory.
     So use mmap(), and ignore the resulting VMA.  */
  pagesize = getpagesize ();
  memneed = len;
  memneed = ((memneed - 1) / pagesize + 1) * pagesize;
  auxmap = (void *) mmap ((void *) 0, memneed, PROT_READ | PROT_WRITE,
                          MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (auxmap == (void *) -1)
    return -1;
  auxmap_start = (unsigned long) auxmap;
  auxmap_end = auxmap_start + memneed;
  mem = (char *) auxmap;
  if (sysctl (info_path, 5, mem, &len, NULL, 0) < 0
      || len > 0x100000 - entry_size)
    {
      /* sysctl failed, or the list of VMAs is possibly truncated.  */
      munmap (auxmap, memneed);
      return -1;
    }
  p = mem;
  p_end = mem + len;
  while (p < p_end)
    {
      struct kinfo_vmentry *kve = (struct kinfo_vmentry *) p;
      unsigned long start = kve->kve_start;
      unsigned long end = kve->kve_end;
      unsigned int flags = 0;
      if (kve->kve_protection & KVME_PROT_READ)
        flags |= VMA_PROT_READ;
      if (kve->kve_protection & KVME_PROT_WRITE)
        flags |= VMA_PROT_WRITE;
      if (kve->kve_protection & KVME_PROT_EXEC)
        flags |= VMA_PROT_EXECUTE;
      if (start <= auxmap_start && auxmap_end - 1 <= end - 1)
        {
          /* Consider [start,end-1] \ [auxmap_start,auxmap_end-1]
             = [start,auxmap_start-1] u [auxmap_end,end-1].  */
          if (start < auxmap_start)
            if (callback (data, start, auxmap_start, flags))
              break;
          if (auxmap_end - 1 < end - 1)
            if (callback (data, auxmap_end, end, flags))
              break;
        }
      else
        {
          if (callback (data, start, end, flags))
            break;
        }
      p += entry_size;
    }
  munmap (auxmap, memneed);
  return 0;
}

#elif defined __OpenBSD__ && defined KERN_PROC_VMMAP /* OpenBSD >= 5.7 */

static int
vma_iterate_bsd (vma_iterate_callback_fn callback, void *data)
{
  /* Documentation: https://man.openbsd.org/sysctl.2  */
  int info_path[] = { CTL_KERN, KERN_PROC_VMMAP, getpid () };
  size_t len;
  size_t pagesize;
  size_t memneed;
  void *auxmap;
  unsigned long auxmap_start;
  unsigned long auxmap_end;
  char *mem;
  char *p;
  char *p_end;

  len = 0;
  if (sysctl (info_path, 3, NULL, &len, NULL, 0) < 0)
    return -1;
  /* Allow for small variations over time.  In a multithreaded program
     new VMAs can be allocated at any moment.  */
  len = 2 * len + 10 * sizeof (struct kinfo_vmentry);
  /* But the system call rejects lengths > 64 KB.  */
  if (len > 0x10000)
    len = 0x10000;
  /* And the system call rejects lengths that are not a multiple of
     sizeof (struct kinfo_vmentry).  */
  len = (len / sizeof (struct kinfo_vmentry)) * sizeof (struct kinfo_vmentry);
  /* Allocate memneed bytes of memory.
     We cannot use alloca here, because not much stack space is guaranteed.
     We also cannot use malloc here, because a malloc() call may call mmap()
     and thus pre-allocate available memory.
     So use mmap(), and ignore the resulting VMA.  */
  pagesize = getpagesize ();
  memneed = len;
  memneed = ((memneed - 1) / pagesize + 1) * pagesize;
  auxmap = (void *) mmap ((void *) 0, memneed, PROT_READ | PROT_WRITE,
                          MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (auxmap == (void *) -1)
    return -1;
  auxmap_start = (unsigned long) auxmap;
  auxmap_end = auxmap_start + memneed;
  mem = (char *) auxmap;
  if (sysctl (info_path, 3, mem, &len, NULL, 0) < 0
      || len > 0x10000 - sizeof (struct kinfo_vmentry))
    {
      /* sysctl failed, or the list of VMAs is possibly truncated.  */
      munmap (auxmap, memneed);
      return -1;
    }
  p = mem;
  p_end = mem + len;
  while (p < p_end)
    {
      struct kinfo_vmentry *kve = (struct kinfo_vmentry *) p;
      unsigned long start = kve->kve_start;
      unsigned long end = kve->kve_end;
      unsigned int flags = 0;
      if (kve->kve_protection & KVE_PROT_READ)
        flags |= VMA_PROT_READ;
      if (kve->kve_protection & KVE_PROT_WRITE)
        flags |= VMA_PROT_WRITE;
      if (kve->kve_protection & KVE_PROT_EXEC)
        flags |= VMA_PROT_EXECUTE;
      if (start <= auxmap_start && auxmap_end - 1 <= end - 1)
        {
          /* Consider [start,end-1] \ [auxmap_start,auxmap_end-1]
             = [start,auxmap_start-1] u [auxmap_end,end-1].  */
          if (start < auxmap_start)
            if (callback (data, start, auxmap_start, flags))
              break;
          if (auxmap_end - 1 < end - 1)
            if (callback (data, auxmap_end, end, flags))
              break;
        }
      else
        {
          if (start != end)
            if (callback (data, start, end, flags))
              break;
        }
      p += sizeof (struct kinfo_vmentry);
    }
  munmap (auxmap, memneed);
  return 0;
}

#else

static inline int
vma_iterate_bsd (vma_iterate_callback_fn callback, void *data)
{
  return -1;
}

#endif


int
vma_iterate (vma_iterate_callback_fn callback, void *data)
{
#if defined __linux__ || defined __ANDROID__ || defined __FreeBSD_kernel__ || defined __FreeBSD__ || defined __DragonFly__ || defined __NetBSD__ || defined __minix /* || defined __CYGWIN__ */

# if defined __FreeBSD__
  /* On FreeBSD with procfs (but not GNU/kFreeBSD, which uses linprocfs), the
     function vma_iterate_proc does not return the virtual memory areas that
     were created by anonymous mmap.  See
     <https://svnweb.freebsd.org/base/head/sys/fs/procfs/procfs_map.c?view=markup>
     So use vma_iterate_proc only as a fallback.  */
  int retval = vma_iterate_bsd (callback, data);
  if (retval == 0)
      return 0;

  return vma_iterate_proc (callback, data);
# else
  /* On the other platforms, try the /proc approach first, and the sysctl()
     as a fallback.  */
  int retval = vma_iterate_proc (callback, data);
  if (retval == 0)
      return 0;

  return vma_iterate_bsd (callback, data);
# endif

#elif defined _AIX /* AIX */

  /* On AIX, there is a /proc/$pic/map file, that contains records of type
     prmap_t, defined in <sys/procfs.h>.  In older versions of AIX, it lists
     only the virtual memory areas that are connected to a file, not the
     anonymous ones.  But at least since AIX 7.1, it is well usable.  */

  size_t pagesize;
  char fnamebuf[6+10+4+1];
  char *fname;
  int fd;
  size_t memneed;

  pagesize = getpagesize ();

  /* Construct fname = sprintf (fnamebuf+i, "/proc/%u/map", getpid ()).  */
  fname = fnamebuf + sizeof (fnamebuf) - (4+1);
  memcpy (fname, "/map", 4+1);
  {
    unsigned int value = getpid ();
    do
      *--fname = (value % 10) + '0';
    while ((value = value / 10) > 0);
  }
  fname -= 6;
  memcpy (fname, "/proc/", 6);

  fd = open (fname, O_RDONLY | O_CLOEXEC);
  if (fd < 0)
    return -1;

  /* The contents of /proc/<pid>/map contains a number of prmap_t entries,
     then an entirely null prmap_t entry, then a heap of NUL terminated
     strings.
     Documentation: https://www.ibm.com/docs/en/aix/7.1?topic=files-proc-file
     We read the entire contents, but look only at the prmap_t entries and
     ignore the tail part.  */

  for (memneed = 2 * pagesize; ; memneed = 2 * memneed)
    {
      /* Allocate memneed bytes of memory.
         We cannot use alloca here, because not much stack space is guaranteed.
         We also cannot use malloc here, because a malloc() call may call mmap()
         and thus pre-allocate available memory.
         So use mmap(), and ignore the resulting VMA if it occurs among the
         resulting VMAs.  (Normally it doesn't, because it was allocated after
         the open() call.)  */
      void *auxmap;
      unsigned long auxmap_start;
      unsigned long auxmap_end;
      ssize_t nbytes;

      auxmap = (void *) mmap ((void *) 0, memneed, PROT_READ | PROT_WRITE,
                              MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      if (auxmap == (void *) -1)
        {
          close (fd);
          return -1;
        }
      auxmap_start = (unsigned long) auxmap;
      auxmap_end = auxmap_start + memneed;

      /* Read the contents of /proc/<pid>/map in a single system call.
         This guarantees a consistent result (no duplicated or omitted
         entries).  */
     retry:
      do
        nbytes = read (fd, auxmap, memneed);
      while (nbytes < 0 && errno == EINTR);
      if (nbytes <= 0)
        {
          munmap (auxmap, memneed);
          close (fd);
          return -1;
        }
      if (nbytes == memneed)
        {
          /* Need more memory.  */
          munmap (auxmap, memneed);
          if (lseek (fd, 0, SEEK_SET) < 0)
            {
              close (fd);
              return -1;
            }
        }
      else
        {
          if (read (fd, (char *) auxmap + nbytes, 1) > 0)
            {
              /* Oops, we had a short read.  Retry.  */
              if (lseek (fd, 0, SEEK_SET) < 0)
                {
                  munmap (auxmap, memneed);
                  close (fd);
                  return -1;
                }
              goto retry;
            }

          /* We now have the entire contents of /proc/<pid>/map in memory.  */
          prmap_t* maps = (prmap_t *) auxmap;

          /* The entries are not sorted by address.  Therefore
             1. Extract the relevant information into an array.
             2. Sort the array in ascending order.
             3. Invoke the callback.  */
          typedef struct
            {
              uintptr_t start;
              uintptr_t end;
              unsigned int flags;
            }
          vma_t;
          /* Since 2 * sizeof (vma_t) <= sizeof (prmap_t), we can reuse the
             same memory.  */
          vma_t *vmas = (vma_t *) auxmap;

          vma_t *vp = vmas;
          {
            prmap_t* mp;
            for (mp = maps;;)
              {
                unsigned long start, end;

                start = (unsigned long) mp->pr_vaddr;
                end = start + mp->pr_size;
                if (start == 0 && end == 0 && mp->pr_mflags == 0)
                  break;
                /* Discard empty VMAs and kernel VMAs.  */
                if (start < end && (mp->pr_mflags & MA_KERNTEXT) == 0)
                  {
                    unsigned int flags;
                    flags = 0;
                    if (mp->pr_mflags & MA_READ)
                      flags |= VMA_PROT_READ;
                    if (mp->pr_mflags & MA_WRITE)
                      flags |= VMA_PROT_WRITE;
                    if (mp->pr_mflags & MA_EXEC)
                      flags |= VMA_PROT_EXECUTE;

                    if (start <= auxmap_start && auxmap_end - 1 <= end - 1)
                      {
                        /* Consider [start,end-1] \ [auxmap_start,auxmap_end-1]
                           = [start,auxmap_start-1] u [auxmap_end,end-1].  */
                        if (start < auxmap_start)
                          {
                            vp->start = start;
                            vp->end = auxmap_start;
                            vp->flags = flags;
                            vp++;
                          }
                        if (auxmap_end - 1 < end - 1)
                          {
                            vp->start = auxmap_end;
                            vp->end = end;
                            vp->flags = flags;
                            vp++;
                          }
                      }
                    else
                      {
                        vp->start = start;
                        vp->end = end;
                        vp->flags = flags;
                        vp++;
                      }
                  }
                mp++;
              }
          }

          size_t nvmas = vp - vmas;
          /* Sort the array in ascending order.
             Better not call qsort(), since it may call malloc().
             Insertion-sort is OK in this case, despite its worst-case running
             time of O(N²), since the number of VMAs will rarely be larger than
             1000.  */
          {
            size_t i;
            for (i = 1; i < nvmas; i++)
              {
                /* Invariant: Here vmas[0..i-1] is sorted.  */
                size_t j;
                for (j = i; j > 0 && vmas[j - 1].start > vmas[j].start; j--)
                  {
                    vma_t tmp = vmas[j - 1];
                    vmas[j - 1] = vmas[j];
                    vmas[j] = tmp;
                  }
                /* Invariant: Here vmas[0..i] is sorted.  */
              }
          }

          /* Invoke the callback.  */
          {
            size_t i;
            for (i = 0; i < nvmas; i++)
              {
                vma_t *vpi = &vmas[i];
                if (callback (data, vpi->start, vpi->end, vpi->flags))
                  break;
              }
          }

          munmap (auxmap, memneed);
          break;
        }
    }

  close (fd);
  return 0;

#elif defined __sgi || defined __osf__ /* IRIX, OSF/1 */

  size_t pagesize;
  char fnamebuf[6+10+1];
  char *fname;
  int fd;
  int nmaps;
  size_t memneed;
# if HAVE_MAP_ANONYMOUS
#  define zero_fd -1
#  define map_flags MAP_ANONYMOUS
# else
  int zero_fd;
#  define map_flags 0
# endif
  void *auxmap;
  unsigned long auxmap_start;
  unsigned long auxmap_end;
  prmap_t* maps;
  prmap_t* mp;

  pagesize = getpagesize ();

  /* Construct fname = sprintf (fnamebuf+i, "/proc/%u", getpid ()).  */
  fname = fnamebuf + sizeof (fnamebuf) - 1;
  *fname = '\0';
  {
    unsigned int value = getpid ();
    do
      *--fname = (value % 10) + '0';
    while ((value = value / 10) > 0);
  }
  fname -= 6;
  memcpy (fname, "/proc/", 6);

  fd = open (fname, O_RDONLY | O_CLOEXEC);
  if (fd < 0)
    return -1;

  if (ioctl (fd, PIOCNMAP, &nmaps) < 0)
    goto fail2;

  memneed = (nmaps + 10) * sizeof (prmap_t);
  /* Allocate memneed bytes of memory.
     We cannot use alloca here, because not much stack space is guaranteed.
     We also cannot use malloc here, because a malloc() call may call mmap()
     and thus pre-allocate available memory.
     So use mmap(), and ignore the resulting VMA.  */
  memneed = ((memneed - 1) / pagesize + 1) * pagesize;
# if !HAVE_MAP_ANONYMOUS
  zero_fd = open ("/dev/zero", O_RDONLY | O_CLOEXEC, 0644);
  if (zero_fd < 0)
    goto fail2;
# endif
  auxmap = (void *) mmap ((void *) 0, memneed, PROT_READ | PROT_WRITE,
                          map_flags | MAP_PRIVATE, zero_fd, 0);
# if !HAVE_MAP_ANONYMOUS
  close (zero_fd);
# endif
  if (auxmap == (void *) -1)
    goto fail2;
  auxmap_start = (unsigned long) auxmap;
  auxmap_end = auxmap_start + memneed;
  maps = (prmap_t *) auxmap;

  if (ioctl (fd, PIOCMAP, maps) < 0)
    goto fail1;

  for (mp = maps;;)
    {
      unsigned long start, end;
      unsigned int flags;

      start = (unsigned long) mp->pr_vaddr;
      end = start + mp->pr_size;
      if (start == 0 && end == 0)
        break;
      flags = 0;
      if (mp->pr_mflags & MA_READ)
        flags |= VMA_PROT_READ;
      if (mp->pr_mflags & MA_WRITE)
        flags |= VMA_PROT_WRITE;
      if (mp->pr_mflags & MA_EXEC)
        flags |= VMA_PROT_EXECUTE;
      mp++;
      if (start <= auxmap_start && auxmap_end - 1 <= end - 1)
        {
          /* Consider [start,end-1] \ [auxmap_start,auxmap_end-1]
             = [start,auxmap_start-1] u [auxmap_end,end-1].  */
          if (start < auxmap_start)
            if (callback (data, start, auxmap_start, flags))
              break;
          if (auxmap_end - 1 < end - 1)
            if (callback (data, auxmap_end, end, flags))
              break;
        }
      else
        {
          if (callback (data, start, end, flags))
            break;
        }
    }
  munmap (auxmap, memneed);
  close (fd);
  return 0;

 fail1:
  munmap (auxmap, memneed);
 fail2:
  close (fd);
  return -1;

#elif defined __sun /* Solaris */

  /* Note: Solaris <sys/procfs.h> defines a different type prmap_t with
     _STRUCTURED_PROC than without! Here's a table of sizeof(prmap_t):
                                  32-bit   64-bit
         _STRUCTURED_PROC = 0       32       56
         _STRUCTURED_PROC = 1       96      104
     Therefore, if the include files provide the newer API, prmap_t has
     the bigger size, and thus you MUST use the newer API.  And if the
     include files provide the older API, prmap_t has the smaller size,
     and thus you MUST use the older API.  */

# if defined PIOCNMAP && defined PIOCMAP
  /* We must use the older /proc interface.  */

  size_t pagesize;
  char fnamebuf[6+10+1];
  char *fname;
  int fd;
  int nmaps;
  size_t memneed;
#  if HAVE_MAP_ANONYMOUS
#   define zero_fd -1
#   define map_flags MAP_ANONYMOUS
#  else /* Solaris <= 7 */
  int zero_fd;
#   define map_flags 0
#  endif
  void *auxmap;
  unsigned long auxmap_start;
  unsigned long auxmap_end;
  prmap_t* maps;
  prmap_t* mp;

  pagesize = getpagesize ();

  /* Construct fname = sprintf (fnamebuf+i, "/proc/%u", getpid ()).  */
  fname = fnamebuf + sizeof (fnamebuf) - 1;
  *fname = '\0';
  {
    unsigned int value = getpid ();
    do
      *--fname = (value % 10) + '0';
    while ((value = value / 10) > 0);
  }
  fname -= 6;
  memcpy (fname, "/proc/", 6);

  fd = open (fname, O_RDONLY | O_CLOEXEC);
  if (fd < 0)
    return -1;

  if (ioctl (fd, PIOCNMAP, &nmaps) < 0)
    goto fail2;

  memneed = (nmaps + 10) * sizeof (prmap_t);
  /* Allocate memneed bytes of memory.
     We cannot use alloca here, because not much stack space is guaranteed.
     We also cannot use malloc here, because a malloc() call may call mmap()
     and thus pre-allocate available memory.
     So use mmap(), and ignore the resulting VMA.  */
  memneed = ((memneed - 1) / pagesize + 1) * pagesize;
#  if !HAVE_MAP_ANONYMOUS
  zero_fd = open ("/dev/zero", O_RDONLY | O_CLOEXEC, 0644);
  if (zero_fd < 0)
    goto fail2;
#  endif
  auxmap = (void *) mmap ((void *) 0, memneed, PROT_READ | PROT_WRITE,
                          map_flags | MAP_PRIVATE, zero_fd, 0);
#  if !HAVE_MAP_ANONYMOUS
  close (zero_fd);
#  endif
  if (auxmap == (void *) -1)
    goto fail2;
  auxmap_start = (unsigned long) auxmap;
  auxmap_end = auxmap_start + memneed;
  maps = (prmap_t *) auxmap;

  if (ioctl (fd, PIOCMAP, maps) < 0)
    goto fail1;

  for (mp = maps;;)
    {
      unsigned long start, end;
      unsigned int flags;

      start = (unsigned long) mp->pr_vaddr;
      end = start + mp->pr_size;
      if (start == 0 && end == 0)
        break;
      flags = 0;
      if (mp->pr_mflags & MA_READ)
        flags |= VMA_PROT_READ;
      if (mp->pr_mflags & MA_WRITE)
        flags |= VMA_PROT_WRITE;
      if (mp->pr_mflags & MA_EXEC)
        flags |= VMA_PROT_EXECUTE;
      mp++;
      if (start <= auxmap_start && auxmap_end - 1 <= end - 1)
        {
          /* Consider [start,end-1] \ [auxmap_start,auxmap_end-1]
             = [start,auxmap_start-1] u [auxmap_end,end-1].  */
          if (start < auxmap_start)
            if (callback (data, start, auxmap_start, flags))
              break;
          if (auxmap_end - 1 < end - 1)
            if (callback (data, auxmap_end, end, flags))
              break;
        }
      else
        {
          if (callback (data, start, end, flags))
            break;
        }
    }
  munmap (auxmap, memneed);
  close (fd);
  return 0;

 fail1:
  munmap (auxmap, memneed);
 fail2:
  close (fd);
  return -1;

# else
  /* We must use the newer /proc interface.
     Documentation:
     https://docs.oracle.com/cd/E23824_01/html/821-1473/proc-4.html
     The contents of /proc/<pid>/map consists of records of type
     prmap_t.  These are different in 32-bit and 64-bit processes,
     but here we are fortunately accessing only the current process.  */

  size_t pagesize;
  char fnamebuf[6+10+4+1];
  char *fname;
  int fd;
  int nmaps;
  size_t memneed;
#  if HAVE_MAP_ANONYMOUS
#   define zero_fd -1
#   define map_flags MAP_ANONYMOUS
#  else /* Solaris <= 7 */
  int zero_fd;
#   define map_flags 0
#  endif
  void *auxmap;
  unsigned long auxmap_start;
  unsigned long auxmap_end;
  prmap_t* maps;
  prmap_t* maps_end;
  prmap_t* mp;

  pagesize = getpagesize ();

  /* Construct fname = sprintf (fnamebuf+i, "/proc/%u/map", getpid ()).  */
  fname = fnamebuf + sizeof (fnamebuf) - 1 - 4;
  memcpy (fname, "/map", 4 + 1);
  {
    unsigned int value = getpid ();
    do
      *--fname = (value % 10) + '0';
    while ((value = value / 10) > 0);
  }
  fname -= 6;
  memcpy (fname, "/proc/", 6);

  fd = open (fname, O_RDONLY | O_CLOEXEC);
  if (fd < 0)
    return -1;

  {
    struct stat statbuf;
    if (fstat (fd, &statbuf) < 0)
      goto fail2;
    nmaps = statbuf.st_size / sizeof (prmap_t);
  }

  memneed = (nmaps + 10) * sizeof (prmap_t);
  /* Allocate memneed bytes of memory.
     We cannot use alloca here, because not much stack space is guaranteed.
     We also cannot use malloc here, because a malloc() call may call mmap()
     and thus pre-allocate available memory.
     So use mmap(), and ignore the resulting VMA.  */
  memneed = ((memneed - 1) / pagesize + 1) * pagesize;
#  if !HAVE_MAP_ANONYMOUS
  zero_fd = open ("/dev/zero", O_RDONLY | O_CLOEXEC, 0644);
  if (zero_fd < 0)
    goto fail2;
#  endif
  auxmap = (void *) mmap ((void *) 0, memneed, PROT_READ | PROT_WRITE,
                          map_flags | MAP_PRIVATE, zero_fd, 0);
#  if !HAVE_MAP_ANONYMOUS
  close (zero_fd);
#  endif
  if (auxmap == (void *) -1)
    goto fail2;
  auxmap_start = (unsigned long) auxmap;
  auxmap_end = auxmap_start + memneed;
  maps = (prmap_t *) auxmap;

  /* Read up to memneed bytes from fd into maps.  */
  {
    size_t remaining = memneed;
    size_t total_read = 0;
    char *ptr = (char *) maps;

    do
      {
        size_t nread = read (fd, ptr, remaining);
        if (nread == (size_t)-1)
          {
            if (errno == EINTR)
              continue;
            goto fail1;
          }
        if (nread == 0)
          /* EOF */
          break;
        total_read += nread;
        ptr += nread;
        remaining -= nread;
      }
    while (remaining > 0);

    nmaps = (memneed - remaining) / sizeof (prmap_t);
    maps_end = maps + nmaps;
  }

  for (mp = maps; mp < maps_end; mp++)
    {
      unsigned long start, end;
      unsigned int flags;

      start = (unsigned long) mp->pr_vaddr;
      end = start + mp->pr_size;
      flags = 0;
      if (mp->pr_mflags & MA_READ)
        flags |= VMA_PROT_READ;
      if (mp->pr_mflags & MA_WRITE)
        flags |= VMA_PROT_WRITE;
      if (mp->pr_mflags & MA_EXEC)
        flags |= VMA_PROT_EXECUTE;
      if (start <= auxmap_start && auxmap_end - 1 <= end - 1)
        {
          /* Consider [start,end-1] \ [auxmap_start,auxmap_end-1]
             = [start,auxmap_start-1] u [auxmap_end,end-1].  */
          if (start < auxmap_start)
            if (callback (data, start, auxmap_start, flags))
              break;
          if (auxmap_end - 1 < end - 1)
            if (callback (data, auxmap_end, end, flags))
              break;
        }
      else
        {
          if (callback (data, start, end, flags))
            break;
        }
    }
  munmap (auxmap, memneed);
  close (fd);
  return 0;

 fail1:
  munmap (auxmap, memneed);
 fail2:
  close (fd);
  return -1;

# endif

#elif HAVE_PSTAT_GETPROCVM /* HP-UX */

  unsigned long pagesize = getpagesize ();
  int i;

  for (i = 0; ; i++)
    {
      struct pst_vm_status info;
      int ret = pstat_getprocvm (&info, sizeof (info), 0, i);
      if (ret < 0)
        return -1;
      if (ret == 0)
        break;
      {
        unsigned long start = info.pst_vaddr;
        unsigned long end = start + info.pst_length * pagesize;
        unsigned int flags = 0;
        if (info.pst_permission & PS_PROT_READ)
          flags |= VMA_PROT_READ;
        if (info.pst_permission & PS_PROT_WRITE)
          flags |= VMA_PROT_WRITE;
        if (info.pst_permission & PS_PROT_EXECUTE)
          flags |= VMA_PROT_EXECUTE;

        if (callback (data, start, end, flags))
          break;
      }
    }

#elif defined __APPLE__ && defined __MACH__ /* Mac OS X */

  task_t task = mach_task_self ();
  vm_address_t address;
  vm_size_t size;

  for (address = VM_MIN_ADDRESS;; address += size)
    {
      int more;
      mach_port_t object_name;
      unsigned int flags;
      /* In Mac OS X 10.5, the types vm_address_t, vm_offset_t, vm_size_t have
         32 bits in 32-bit processes and 64 bits in 64-bit processes. Whereas
         mach_vm_address_t and mach_vm_size_t are always 64 bits large.
         Mac OS X 10.5 has three vm_region like methods:
           - vm_region. It has arguments that depend on whether the current
             process is 32-bit or 64-bit. When linking dynamically, this
             function exists only in 32-bit processes. Therefore we use it only
             in 32-bit processes.
           - vm_region_64. It has arguments that depend on whether the current
             process is 32-bit or 64-bit. It interprets a flavor
             VM_REGION_BASIC_INFO as VM_REGION_BASIC_INFO_64, which is
             dangerous since 'struct vm_region_basic_info_64' is larger than
             'struct vm_region_basic_info'; therefore let's write
             VM_REGION_BASIC_INFO_64 explicitly.
           - mach_vm_region. It has arguments that are 64-bit always. This
             function is useful when you want to access the VM of a process
             other than the current process.
         In 64-bit processes, we could use vm_region_64 or mach_vm_region.
         I choose vm_region_64 because it uses the same types as vm_region,
         resulting in less conditional code.  */
# if defined __aarch64__ || defined __ppc64__ || defined __x86_64__
      struct vm_region_basic_info_64 info;
      mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT_64;

      more = (vm_region_64 (task, &address, &size, VM_REGION_BASIC_INFO_64,
                            (vm_region_info_t)&info, &info_count, &object_name)
              == KERN_SUCCESS);
# else
      struct vm_region_basic_info info;
      mach_msg_type_number_t info_count = VM_REGION_BASIC_INFO_COUNT;

      more = (vm_region (task, &address, &size, VM_REGION_BASIC_INFO,
                         (vm_region_info_t)&info, &info_count, &object_name)
              == KERN_SUCCESS);
# endif
      if (object_name != MACH_PORT_NULL)
        mach_port_deallocate (mach_task_self (), object_name);
      if (!more)
        break;
      flags = 0;
      if (info.protection & VM_PROT_READ)
        flags |= VMA_PROT_READ;
      if (info.protection & VM_PROT_WRITE)
        flags |= VMA_PROT_WRITE;
      if (info.protection & VM_PROT_EXECUTE)
        flags |= VMA_PROT_EXECUTE;
      if (callback (data, address, address + size, flags))
        break;
    }
  return 0;

#elif defined __GNU__ /* GNU/Hurd */

  /* The Hurd has a /proc/self/maps that looks like the Linux one, but it
     lacks the VMAs created through anonymous mmap.  Therefore use the Mach
     API.
     Documentation:
     https://www.gnu.org/software/hurd/gnumach-doc/Memory-Attributes.html */

  task_t task = mach_task_self ();
  vm_address_t address;
  vm_size_t size;

  for (address = 0;; address += size)
    {
      vm_prot_t protection;
      vm_prot_t max_protection;
      vm_inherit_t inheritance;
      boolean_t shared;
      memory_object_name_t object_name;
      vm_offset_t offset;
      unsigned int flags;

      if (!(vm_region (task, &address, &size, &protection, &max_protection,
                         &inheritance, &shared, &object_name, &offset)
            == KERN_SUCCESS))
        break;
      mach_port_deallocate (task, object_name);
      flags = 0;
      if (protection & VM_PROT_READ)
        flags |= VMA_PROT_READ;
      if (protection & VM_PROT_WRITE)
        flags |= VMA_PROT_WRITE;
      if (protection & VM_PROT_EXECUTE)
        flags |= VMA_PROT_EXECUTE;
      if (callback (data, address, address + size, flags))
        break;
    }
  return 0;

#elif defined _WIN32 || defined __CYGWIN__
  /* Windows platform.  Use the native Windows API.  */

  MEMORY_BASIC_INFORMATION info;
  uintptr_t address = 0;

  while (VirtualQuery ((void*)address, &info, sizeof(info)) == sizeof(info))
    {
      if (info.State != MEM_FREE)
        /* Ignore areas where info.State has the value MEM_RESERVE or,
           equivalently, info.Protect has the undocumented value 0.
           This is needed, so that on Cygwin, areas used by malloc() are
           distinguished from areas reserved for future malloc().  */
        if (info.State != MEM_RESERVE)
          {
            uintptr_t start, end;
            unsigned int flags;

            start = (uintptr_t)info.BaseAddress;
            end = start + info.RegionSize;
            switch (info.Protect & ~(PAGE_GUARD|PAGE_NOCACHE))
              {
              case PAGE_READONLY:
                flags = VMA_PROT_READ;
                break;
              case PAGE_READWRITE:
              case PAGE_WRITECOPY:
                flags = VMA_PROT_READ | VMA_PROT_WRITE;
                break;
              case PAGE_EXECUTE:
                flags = VMA_PROT_EXECUTE;
                break;
              case PAGE_EXECUTE_READ:
                flags = VMA_PROT_READ | VMA_PROT_EXECUTE;
                break;
              case PAGE_EXECUTE_READWRITE:
              case PAGE_EXECUTE_WRITECOPY:
                flags = VMA_PROT_READ | VMA_PROT_WRITE | VMA_PROT_EXECUTE;
                break;
              case PAGE_NOACCESS:
              default:
                flags = 0;
                break;
              }

            if (callback (data, start, end, flags))
              break;
          }
      address = (uintptr_t)info.BaseAddress + info.RegionSize;
    }
  return 0;

#elif defined __BEOS__ || defined __HAIKU__
  /* Use the BeOS specific API.  */

  area_info info;
  ssize_t cookie;

  cookie = 0;
  while (get_next_area_info (0, &cookie, &info) == B_OK)
    {
      unsigned long start, end;
      unsigned int flags;

      start = (unsigned long) info.address;
      end = start + info.size;
      flags = 0;
      if (info.protection & B_READ_AREA)
        flags |= VMA_PROT_READ | VMA_PROT_EXECUTE;
      if (info.protection & B_WRITE_AREA)
        flags |= VMA_PROT_WRITE;

      if (callback (data, start, end, flags))
        break;
    }
  return 0;

#elif HAVE_MQUERY /* OpenBSD */

# if defined __OpenBSD__
  /* Try sysctl() first.  It is more efficient than the mquery() loop below
     and also provides the flags.  */
  {
    int retval = vma_iterate_bsd (callback, data);
    if (retval == 0)
      return 0;
  }
# endif

  {
    uintptr_t pagesize;
    uintptr_t address;
    int /*bool*/ address_known_mapped;

    pagesize = getpagesize ();
    /* Avoid calling mquery with a NULL first argument, because this argument
       value has a specific meaning.  We know the NULL page is unmapped.  */
    address = pagesize;
    address_known_mapped = 0;
    for (;;)
      {
        /* Test whether the page at address is mapped.  */
        if (address_known_mapped
            || mquery ((void *) address, pagesize, 0, MAP_FIXED, -1, 0)
               == (void *) -1)
          {
            /* The page at address is mapped.
               This is the start of an interval.  */
            uintptr_t start = address;
            uintptr_t end;

            /* Find the end of the interval.  */
            end = (uintptr_t) mquery ((void *) address, pagesize, 0, 0, -1, 0);
            if (end == (uintptr_t) (void *) -1)
              end = 0; /* wrap around */
            address = end;

            /* It's too complicated to find out about the flags.
               Just pass 0.  */
            if (callback (data, start, end, 0))
              break;

            if (address < pagesize) /* wrap around? */
              break;
          }
        /* Here we know that the page at address is unmapped.  */
        {
          uintptr_t query_size = pagesize;

          address += pagesize;

          /* Query larger and larger blocks, to get through the unmapped address
             range with few mquery() calls.  */
          for (;;)
            {
              if (2 * query_size > query_size)
                query_size = 2 * query_size;
              if (address + query_size - 1 < query_size) /* wrap around? */
                {
                  address_known_mapped = 0;
                  break;
                }
              if (mquery ((void *) address, query_size, 0, MAP_FIXED, -1, 0)
                  == (void *) -1)
                {
                  /* Not all the interval [address .. address + query_size - 1]
                     is unmapped.  */
                  address_known_mapped = (query_size == pagesize);
                  break;
                }
              /* The interval [address .. address + query_size - 1] is
                 unmapped.  */
              address += query_size;
            }
          /* Reduce the query size again, to determine the precise size of the
             unmapped interval that starts at address.  */
          while (query_size > pagesize)
            {
              query_size = query_size / 2;
              if (address + query_size - 1 >= query_size)
                {
                  if (mquery ((void *) address, query_size, 0, MAP_FIXED, -1, 0)
                      != (void *) -1)
                    {
                      /* The interval [address .. address + query_size - 1] is
                         unmapped.  */
                      address += query_size;
                      address_known_mapped = 0;
                    }
                  else
                    address_known_mapped = (query_size == pagesize);
                }
            }
          /* Here again query_size = pagesize, and
             either address + pagesize - 1 < pagesize, or
             mquery ((void *) address, pagesize, 0, MAP_FIXED, -1, 0) fails.
             So, the unmapped area ends at address.  */
        }
        if (address + pagesize - 1 < pagesize) /* wrap around? */
          break;
      }
    return 0;
  }

#else

  /* Not implemented.  */
  return -1;

#endif
}


#ifdef TEST

#include <stdio.h>

/* Output the VMAs of the current process in a format similar to the Linux
   /proc/$pid/maps file.  */

static int
vma_iterate_callback (void *data, uintptr_t start, uintptr_t end,
                      unsigned int flags)
{
  printf ("%08lx-%08lx %c%c%c\n",
          (unsigned long) start, (unsigned long) end,
          flags & VMA_PROT_READ ? 'r' : '-',
          flags & VMA_PROT_WRITE ? 'w' : '-',
          flags & VMA_PROT_EXECUTE ? 'x' : '-');
  return 0;
}

int
main ()
{
  vma_iterate (vma_iterate_callback, NULL);

  /* Let the user interactively look at the /proc file system.  */
  sleep (10);

  return 0;
}

/*
 * Local Variables:
 * compile-command: "gcc -ggdb -DTEST -Wall -I.. vma-iter.c"
 * End:
 */

#endif /* TEST */
