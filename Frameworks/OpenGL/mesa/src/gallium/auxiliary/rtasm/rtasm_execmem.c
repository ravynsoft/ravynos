/**************************************************************************
 *
 * Copyright (C) 1999-2005  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


/**
 * \file exemem.c
 * Functions for allocating executable memory.
 *
 * \author Keith Whitwell
 */


#include "util/compiler.h"
#include "util/simple_mtx.h"
#include "util/u_debug.h"
#include "util/u_thread.h"
#include "util/u_memory.h"

#include "rtasm_execmem.h"

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#if DETECT_OS_WINDOWS
#include <windows.h>
#endif

#if DETECT_OS_UNIX


/*
 * Allocate a large block of memory which can hold code then dole it out
 * in pieces by means of the generic memory manager code.
 */

#include <unistd.h>
#include <sys/mman.h>
#include "util/u_mm.h"

#define EXEC_HEAP_SIZE (10*1024*1024)

static simple_mtx_t exec_mutex = SIMPLE_MTX_INITIALIZER;

static struct mem_block *exec_heap = NULL;
static unsigned char *exec_mem = NULL;


static int
init_heap(void)
{
   if (!exec_heap)
      exec_heap = u_mmInit( 0, EXEC_HEAP_SIZE );

   if (!exec_mem)
      exec_mem = (unsigned char *) mmap(NULL, EXEC_HEAP_SIZE,
         PROT_EXEC | PROT_READ | PROT_WRITE,
         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

   return (exec_mem != MAP_FAILED);
}


void *
rtasm_exec_malloc(size_t size)
{
   struct mem_block *block = NULL;
   void *addr = NULL;

   simple_mtx_lock(&exec_mutex);

   if (!init_heap())
      goto bail;

   if (exec_heap) {
      size = (size + 31) & ~31;  /* next multiple of 32 bytes */
      block = u_mmAllocMem( exec_heap, size, 5, 0 ); /* 5 -> 32-byte alignment */
   }

   if (block)
      addr = exec_mem + block->ofs;
   else
      debug_printf("rtasm_exec_malloc failed\n");

bail:
   simple_mtx_unlock(&exec_mutex);

   return addr;
}


void
rtasm_exec_free(void *addr)
{
   simple_mtx_lock(&exec_mutex);

   if (exec_heap) {
      struct mem_block *block = u_mmFindBlock(exec_heap, (unsigned char *)addr - exec_mem);

      if (block)
         u_mmFreeMem(block);
   }

   simple_mtx_unlock(&exec_mutex);
}


#elif DETECT_OS_WINDOWS


/*
 * Avoid Data Execution Prevention.
 */

void *
rtasm_exec_malloc(size_t size)
{
   return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}


void
rtasm_exec_free(void *addr)
{
   VirtualFree(addr, 0, MEM_RELEASE);
}


#else


/*
 * Just use regular memory.
 */

void *
rtasm_exec_malloc(size_t size)
{
   return MALLOC( size );
}


void
rtasm_exec_free(void *addr)
{
   FREE(addr);
}


#endif
