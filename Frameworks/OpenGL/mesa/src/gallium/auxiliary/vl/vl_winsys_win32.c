/*
 * Copyright © Microsoft Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include "pipe-loader/pipe_loader.h"
#include "pipe/p_screen.h"

#include "util/u_memory.h"
#include "vl/vl_winsys.h"

#include "gallium/winsys/sw/gdi/gdi_sw_winsys.h"
#include "gallium/drivers/d3d12/d3d12_public.h"
#include <unknwn.h>

struct vl_win32_screen
{
   struct vl_screen base;
   LUID *adapter_luid;
};

static void
vl_win32_screen_destroy(struct vl_screen *vscreen)
{
   if (vscreen == NULL)
      return;

   if (vscreen->pscreen)
      vscreen->pscreen->destroy(vscreen->pscreen);

   if (vscreen->dev)
      pipe_loader_release(&vscreen->dev, 1);

   FREE(vscreen);
}

struct vl_screen *
vl_win32_screen_create(LUID *adapter)
{
   struct vl_win32_screen *vscreen = CALLOC_STRUCT(vl_win32_screen);
   if (!vscreen)
      return NULL;

   struct sw_winsys* winsys = gdi_create_sw_winsys();
   if (!winsys)
      goto release_pipe;

   /* If adapter is null, d3d12_create_dxcore_screen will choose one */
   vscreen->base.pscreen = d3d12_create_dxcore_screen(winsys, adapter);

   if (!vscreen->base.pscreen)
      goto release_pipe;

   vscreen->adapter_luid = adapter;

   vscreen->base.destroy = vl_win32_screen_destroy;
   vscreen->base.get_private = NULL;
   vscreen->base.texture_from_drawable = NULL;
   vscreen->base.get_dirty_area = NULL;

   return &vscreen->base;

release_pipe:
   if(winsys)
      winsys->destroy(winsys);
   vl_win32_screen_destroy(&vscreen->base);
   return NULL;
}

struct vl_screen *
vl_win32_screen_create_from_d3d12_device(IUnknown* d3d12_device)
{
   struct vl_win32_screen *vscreen = CALLOC_STRUCT(vl_win32_screen);
   if (!vscreen)
      return NULL;

   struct sw_winsys* winsys = gdi_create_sw_winsys();
   if (!winsys)
      goto release_pipe;

   vscreen->base.pscreen = d3d12_create_dxcore_screen_from_d3d12_device(winsys, d3d12_device, &vscreen->adapter_luid);

   if (!vscreen->base.pscreen)
      goto release_pipe;

   vscreen->base.destroy = vl_win32_screen_destroy;
   vscreen->base.get_private = NULL;
   vscreen->base.texture_from_drawable = NULL;
   vscreen->base.get_dirty_area = NULL;

   return &vscreen->base;

release_pipe:
   if(winsys)
      winsys->destroy(winsys);
   vl_win32_screen_destroy(&vscreen->base);
   return NULL;
}
