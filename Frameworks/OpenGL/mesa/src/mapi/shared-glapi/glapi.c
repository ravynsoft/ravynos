/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2010 LunarG Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include <string.h>
#include <stdlib.h>
#include "glapi/glapi.h"
#include "u_current.h"
#include "table.h" /* for MAPI_TABLE_NUM_SLOTS */
#include "stub.h"

/*
 * _glapi_Dispatch, _glapi_Context
 * _glapi_tls_Dispatch, _glapi_tls_Context,
 * _glapi_set_context, _glapi_get_context,
 * _glapi_destroy_multithread, _glapi_check_multithread
 * _glapi_set_dispatch, and _glapi_get_dispatch
 * are defined in u_current.c.
 */

/**
 * Return size of dispatch table struct as number of functions (or
 * slots).
 */
unsigned int
_glapi_get_dispatch_table_size(void)
{
   return MAPI_TABLE_NUM_SLOTS;
}

/**
 * Initializes the glapi relocs table (no-op for shared-glapi), and returns the
 * offset of the given function in the dispatch table.
 */
int
_glapi_add_dispatch( const char * function_name )
{
   assert(function_name[0] == 'g' && function_name[1] == 'l');

   const struct mapi_stub *stub = stub_find_public(function_name + 2);
   if (!stub)
      return -1;

   return stub_get_slot(stub);
}

static const struct mapi_stub *
_glapi_get_stub(const char *name)
{
   if (!name || name[0] != 'g' || name[1] != 'l')
      return NULL;
   name += 2;

   return stub_find_public(name);
}

/**
 * Return offset of entrypoint for named function within dispatch table.
 */
int
_glapi_get_proc_offset(const char *funcName)
{
   const struct mapi_stub *stub = _glapi_get_stub(funcName);
   return (stub) ? stub_get_slot(stub) : -1;
}

/**
 * Return pointer to the named function.  If the function name isn't found
 * in the name of static functions, try generating a new API entrypoint on
 * the fly with assembly language.
 */
_glapi_proc
_glapi_get_proc_address(const char *funcName)
{
   const struct mapi_stub *stub = _glapi_get_stub(funcName);
   return (stub) ? (_glapi_proc) stub_get_addr(stub) : NULL;
}

/**
 * Return the name of the function at the given dispatch offset.
 * This is only intended for debugging.
 */
const char *
_glapi_get_proc_name(unsigned int offset)
{
   const struct mapi_stub *stub = stub_find_by_slot(offset);
   return stub ? stub_get_name(stub) : NULL;
}

/** Return pointer to new dispatch table filled with no-op functions */
struct _glapi_table *
_glapi_new_nop_table(unsigned num_entries)
{
   struct _glapi_table *table;

   if (num_entries > MAPI_TABLE_NUM_SLOTS)
      num_entries = MAPI_TABLE_NUM_SLOTS;

   table = malloc(num_entries * sizeof(mapi_func));
   if (table) {
      memcpy(table, table_noop_array, num_entries * sizeof(mapi_func));
   }
   return table;
}

void
_glapi_set_nop_handler(_glapi_nop_handler_proc func)
{
   table_set_noop_handler(func);
}

/**
 * This is a deprecated function which should not be used anymore.
 * It's only present to satisfy linking with older versions of libGL.
 */
unsigned long
_glthread_GetID(void)
{
   return 0;
}

void
_glapi_noop_enable_warnings(unsigned char enable)
{
}

void
_glapi_set_warning_func(_glapi_proc func)
{
}
