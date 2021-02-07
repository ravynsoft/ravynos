/** Implementation of page-related functions for GNUstep
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Created: May 1996

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   <title>NSPage class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#include <stdio.h>
#if __mach__
#include <mach.h>
#endif

#ifdef __CYGWIN__
#include <malloc.h>
#endif

#ifdef _WIN32
#include <malloc.h>
static size_t
getpagesize(void)
{
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return info.dwPageSize;
}
#endif

#ifdef __SOLARIS__
#define getpagesize() sysconf(_SC_PAGESIZE)
#endif

#ifdef __svr4__
#define getpagesize() sysconf(_SC_PAGESIZE)
#endif

#if __mach__
#define getpagesize vm_page_size
#endif

#ifdef __BEOS__
#include <kernel/OS.h>
#define getpagesize()  B_PAGE_SIZE
#endif

/* Cache the size of a memory page here, so we don't have to make the
   getpagesize() system call repeatedly. */
static NSUInteger ns_page_size = 0;

/**
 * Return the number of bytes in a memory page.
 */
NSUInteger
NSPageSize (void)
{
  if (!ns_page_size)
    ns_page_size = getpagesize ();
  return ns_page_size;
}

/**
 * Return log base 2 of the number of bytes in a memory page.
 */
NSUInteger
NSLogPageSize (void)
{
  NSUInteger tmp_page_size = NSPageSize();
  NSUInteger log = 0;

  while (tmp_page_size >>= 1)
    log++;
  return log;
}

/**
 * Round bytes down to the nearest multiple of the memory page size,
 * and return it.
 */
NSUInteger
NSRoundDownToMultipleOfPageSize (NSUInteger bytes)
{
  NSUInteger a = NSPageSize();

  return (bytes / a) * a;
}

/**
 * Round bytes up to the nearest multiple of the memory page size,
 * and return it.
 */
NSUInteger
NSRoundUpToMultipleOfPageSize (NSUInteger bytes)
{
  NSUInteger a = NSPageSize();

  return ((bytes % a) ? ((bytes / a + 1) * a) : bytes);
}

#if __linux__
#include	<sys/sysinfo.h>
#endif

/**
 * Return the number of bytes of real (physical) memory available.
 */
NSUInteger
NSRealMemoryAvailable ()
{
#if __linux__
  struct sysinfo info;

  if ((sysinfo(&info)) != 0)
    return 0;
  return  info.freeram;
#elif defined(_WIN32)
  MEMORYSTATUSEX memory;

  memory.dwLength = sizeof(memory);
  GlobalMemoryStatusEx(&memory);
  return memory.ullAvailPhys;
#elif defined(__BEOS__)
  system_info info;

  if (get_system_info(&info) != B_OK)
    return 0;
  return (info.max_pages - info.used_pages) * B_PAGE_SIZE;
#else
  fprintf (stderr, "NSRealMemoryAvailable() not implemented.\n");
  return 0;
#endif
}

/**
 * Allocate memory for this process and return a pointer to it (or a null
 * pointer on failure). The allocated memory is page aligned and the
 * actual size of memory allocated is a multiple of the page size.
 */
void *
NSAllocateMemoryPages (NSUInteger bytes)
{
  NSUInteger size = NSRoundUpToMultipleOfPageSize (bytes);
  void *where;
#if defined(_WIN32)
  where = VirtualAlloc(NULL, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#elif __mach__
  kern_return_t r;
  r = vm_allocate (mach_task_self(), &where, (vm_size_t) size, 1);
  if (r != KERN_SUCCESS)
    return NULL;
  return where;
#elif	HAVE_POSIX_MEMALIGN
  if (posix_memalign(&where, NSPageSize(), size) != 0)
    return NULL;
#else
  where = valloc (size);
  if (where == NULL)
    return NULL;
#endif
  memset (where, 0, bytes);
  return where;
}

/**
 * Deallocate memory which was previously allocated using the
 * NSAllocateMemoryPages() function.<br />
 * The bytes argument should be the same as the value passed
 * to the NSAllocateMemoryPages() function.
 */
void
NSDeallocateMemoryPages (void *ptr, NSUInteger bytes)
{
#if defined(_WIN32)
  VirtualFree(ptr, 0, MEM_RELEASE);
#elif __mach__
  vm_deallocate (mach_task_self (), ptr, NSRoundUpToMultipleOfPageSize (bytes));
#else
  free (ptr);
#endif
}

/**
 * Perform an efficient large scale copy of data from src to dest.
 * The value bytes specifies the length of the data copied.
 */
void
NSCopyMemoryPages (const void *src, void *dest, NSUInteger bytes)
{
#if __mach__
  kern_return_t r;
  r = vm_copy (mach_task_self(), src, bytes, dest);
  NSParameterAssert (r == KERN_SUCCESS);
#else
  memcpy (dest, src, bytes);
#endif
}

