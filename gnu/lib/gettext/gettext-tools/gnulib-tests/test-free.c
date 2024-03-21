/* Test of free() function.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2020.  */

#include <config.h>

/* Specification.  */
#include <stdlib.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#if defined __linux__
# include <fcntl.h>
# include <stdint.h>
# include <string.h>
# include <sys/mman.h>
#endif

#include "macros.h"

/* The indirection through a volatile function pointer is necessary to prevent
   a GCC optimization.  Without it, when optimizing, GCC would "know" that errno
   is unchanged by calling free(ptr), when ptr was the result of a malloc(...)
   call in the same function.  */
static int
get_errno (void)
{
  volatile int err = errno;
  return err;
}

static int (* volatile get_errno_func) (void) = get_errno;

int
main ()
{
  /* Check that free() preserves errno.  */
  {
    errno = 1789; /* Liberté, égalité, fraternité.  */
    free (NULL);
    ASSERT_NO_STDIO (get_errno_func () == 1789);
  }
  { /* Small memory allocations.  */
    #define N 10000
    void * volatile ptrs[N];
    size_t i;
    for (i = 0; i < N; i++)
      ptrs[i] = malloc (15);
    for (i = 0; i < N; i++)
      {
        errno = 1789;
        free (ptrs[i]);
        ASSERT_NO_STDIO (get_errno_func () == 1789);
      }
    #undef N
  }
  { /* Medium memory allocations.  */
    #define N 1000
    void * volatile ptrs[N];
    size_t i;
    for (i = 0; i < N; i++)
      ptrs[i] = malloc (729);
    for (i = 0; i < N; i++)
      {
        errno = 1789;
        free (ptrs[i]);
        ASSERT_NO_STDIO (get_errno_func () == 1789);
      }
    #undef N
  }
  { /* Large memory allocations.  */
    #define N 10
    void * volatile ptrs[N];
    size_t i;
    for (i = 0; i < N; i++)
      ptrs[i] = malloc (5318153);
    for (i = 0; i < N; i++)
      {
        errno = 1789;
        free (ptrs[i]);
        ASSERT_NO_STDIO (get_errno_func () == 1789);
      }
    #undef N
  }

  /* Test a less common code path.
     When malloc() is based on mmap(), free() can sometimes call munmap().
     munmap() usually succeeds, but fails in a particular situation: when
       - it has to unmap the middle part of a VMA, and
       - the number of VMAs of a process is limited and the limit is
         already reached.
     The latter condition is fulfilled on Linux, when the file
     /proc/sys/vm/max_map_count exists.  This file contains the limit
       - for Linux >= 2.4.19: 65536 (DEFAULT_MAX_MAP_COUNT in linux/include/linux/sched.h)
       - for Linux >= 2.6.31: 65530 (DEFAULT_MAX_MAP_COUNT in linux/include/linux/mm.h).
     But do not test it with glibc < 2.15, since that triggers a glibc internal
     abort: "malloc.c:3551: munmap_chunk: Assertion `ret == 0' failed."
   */
  #if defined __linux__ && !(__GLIBC__ == 2 && __GLIBC_MINOR__ < 15)
  if (open ("/proc/sys/vm/max_map_count", O_RDONLY) >= 0)
    {
      /* Preparations.  */
      size_t pagesize = getpagesize ();
      void *firstpage_backup = malloc (pagesize);
      void *lastpage_backup = malloc (pagesize);
      /* Allocate a large memory area, as a bumper, so that the MAP_FIXED
         allocation later will not overwrite parts of the memory areas
         allocated to ld.so or libc.so.  */
      void *bumper_region =
        mmap (NULL, 0x1000000, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      /* A file descriptor pointing to a regular file.  */
      int fd = open ("test-free", O_RDONLY);

      if (firstpage_backup != NULL && lastpage_backup != NULL
          && bumper_region != (void *)(-1)
          && fd >= 0)
        {
          /* Do a large memory allocation.  */
          size_t big_size = 0x1000000;
          void * volatile ptr = malloc (big_size - 0x100);
          char *ptr_aligned = (char *) ((uintptr_t) ptr & ~(pagesize - 1));
          /* This large memory allocation allocated a memory area
             from ptr_aligned to ptr_aligned + big_size.
             Enlarge this memory area by adding a page before and a page
             after it.  */
          memcpy (firstpage_backup, ptr_aligned, pagesize);
          memcpy (lastpage_backup, ptr_aligned + big_size - pagesize, pagesize);
          if (mmap (ptr_aligned - pagesize, pagesize + big_size + pagesize,
                    PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0)
              != (void *)(-1))
            {
              memcpy (ptr_aligned, firstpage_backup, pagesize);
              memcpy (ptr_aligned + big_size - pagesize, lastpage_backup, pagesize);

              /* Now add as many mappings as we can.
                 Stop at 65536, in order not to crash the machine (in case the
                 limit has been increased by the system administrator).  */
              size_t i;
              for (i = 0; i < 65536; i++)
                if (mmap (NULL, pagesize, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0)
                    == (void *)(-1))
                  break;
              /* Now the number of VMAs of this process has hopefully attained
                 its limit.  */

              errno = 1789;
              /* This call to free() is supposed to call
                   munmap (ptr_aligned, big_size);
                 which increases the number of VMAs by 1, which is supposed
                 to fail.  */
              free (ptr);
              ASSERT_NO_STDIO (get_errno_func () == 1789);
            }
        }
    }
  #endif

  return 0;
}
