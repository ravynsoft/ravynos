/**************************************************************************
 *
 * Copyright 2010 Younes Manton.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "util/simple_mtx.h"
#include "util/u_handle_table.h"
#include "util/u_thread.h"
#include "vdpau_private.h"

static struct handle_table *htab = NULL;
static simple_mtx_t htab_lock = SIMPLE_MTX_INITIALIZER;

bool vlCreateHTAB(void)
{
   bool ret;

   /* Make sure handle table handles match VDPAU handles. */
   assert(sizeof(unsigned) <= sizeof(vlHandle));
   simple_mtx_lock(&htab_lock);
   if (!htab)
      htab = handle_table_create();
   ret = htab != NULL;
   simple_mtx_unlock(&htab_lock);
   return ret;
}

void vlDestroyHTAB(void)
{
   simple_mtx_lock(&htab_lock);
   if (htab && !handle_table_get_first_handle(htab)) {
      handle_table_destroy(htab);
      htab = NULL;
   }
   simple_mtx_unlock(&htab_lock);
}

vlHandle vlAddDataHTAB(void *data)
{
   vlHandle handle = 0;

   assert(data);
   simple_mtx_lock(&htab_lock);
   if (htab)
      handle = handle_table_add(htab, data);
   simple_mtx_unlock(&htab_lock);
   return handle;
}

void* vlGetDataHTAB(vlHandle handle)
{
   void *data = NULL;

   assert(handle);
   simple_mtx_lock(&htab_lock);
   if (htab)
      data = handle_table_get(htab, handle);
   simple_mtx_unlock(&htab_lock);
   return data;
}

void vlRemoveDataHTAB(vlHandle handle)
{
   simple_mtx_lock(&htab_lock);
   if (htab)
      handle_table_remove(htab, handle);
   simple_mtx_unlock(&htab_lock);
}
