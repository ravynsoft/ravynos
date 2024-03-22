/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2010  VMware, Inc.  All Rights Reserved.
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
 */


/**
 * No-op dispatch table.
 *
 * This file defines a special dispatch table which is loaded with no-op
 * functions.
 *
 * Mesa can register a "no-op handler function" which will be called in
 * the event that a no-op function is called.
 *
 * In the past, the dispatch table was loaded with pointers to a single
 * no-op function.  But that broke on Windows because the GL entrypoints
 * use __stdcall convention.  __stdcall means the callee cleans up the
 * stack.  So one no-op function can't properly clean up the stack.  This
 * would lead to crashes.
 *
 * Another benefit of unique no-op functions is we can accurately report
 * the function's name in an error message.
 */


#include <stdlib.h>
#include <string.h>
#include "glapi/glapi_priv.h"


void
_glapi_noop_enable_warnings(unsigned char enable)
{
}

void
_glapi_set_warning_func(_glapi_proc func)
{
}


/**
 * We'll jump though this function pointer whenever a no-op function
 * is called.
 */
static _glapi_nop_handler_proc nop_handler = NULL;


/**
 * Register the no-op handler call-back function.
 */
void
_glapi_set_nop_handler(_glapi_nop_handler_proc func)
{
   nop_handler = func;
}


/**
 * Called by each of the no-op GL entrypoints.
 */
static void
nop(const char *func)
{
   if (nop_handler)
      nop_handler(func);
}


/**
 * This is called if the user somehow calls an unassigned GL dispatch function.
 */
static GLint
NoOpUnused(void)
{
   nop("unused GL entry point");
   return 0;
}

/*
 * Defines for the glapitemp.h functions.
 */
#define KEYWORD1 static
#define KEYWORD1_ALT static
#define KEYWORD2 GLAPIENTRY
#define NAME(func)  NoOp##func
#define DISPATCH(func, args, msg)  nop(#func);
#define RETURN_DISPATCH(func, args, msg)  nop(#func); return 0


/*
 * Defines for the table of no-op entry points.
 */
#define TABLE_ENTRY(name) (_glapi_proc) NoOp##name
#define DISPATCH_TABLE_NAME __glapi_noop_table
#define UNUSED_TABLE_NAME __unused_noop_functions

#include "glapitemp.h"


/** Return pointer to new dispatch table filled with no-op functions */
struct _glapi_table *
_glapi_new_nop_table(unsigned num_entries)
{
   struct _glapi_table *table = malloc(num_entries * sizeof(_glapi_proc));
   if (table) {
      memcpy(table, __glapi_noop_table,
             num_entries * sizeof(_glapi_proc));
   }
   return table;
}
