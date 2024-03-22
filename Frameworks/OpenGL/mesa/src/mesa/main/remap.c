/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2009 Chia-I Wu <olv@0xlab.org>
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
 */


/**
 * \file remap.c
 * Remap table management.
 *
 * Entries in the dispatch table are either static or dynamic.  The
 * dispatch table is shared by mesa core and glapi.  When they are
 * built separately, it is possible that a static entry in mesa core
 * is dynamic, or assigned a different static offset, in glapi.  The
 * remap table is in charge of mapping a static entry in mesa core to
 * a dynamic entry, or the corresponding static entry, in glapi.
 */

#include <stdbool.h>
#include <string.h>
#include "remap.h"

#include "glapi/glapi.h"

#define MAX_ENTRY_POINTS 16

#define need_MESA_remap_table
#include "main/remap_helper.h"
#include "errors.h"


/* this is global for quick access */
int driDispatchRemapTable[driDispatchRemapTable_size];



/**
 * Initialize the remap table.  This is called in one_time_init().
 * The remap table needs to be initialized before calling the
 * CALL/GET/SET macros defined in main/dispatch.h.
 */
void
_mesa_init_remap_table(void)
{
   static bool initialized = false;
   GLint i;

   if (initialized)
      return;
   initialized = true;

   for (i = 0; i < driDispatchRemapTable_size; i++) {
      /* sanity check */
      assert(i == MESA_remap_table_functions[i].remap_index);
      const char *name = _mesa_function_pool + MESA_remap_table_functions[i].pool_index;

      /* store the dispatch offset in driDispatchRemapTable, for use by
       * _gloffset_* macros.
       */
      driDispatchRemapTable[i] = _glapi_add_dispatch(name);
      if (driDispatchRemapTable[i] < 0) {
         _mesa_warning(NULL, "failed to remap %s", name);
      }
   }
}
