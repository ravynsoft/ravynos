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

#include "d3d12_wgl_public.h"
#include "d3d12/d3d12_public.h"

#include "stw_device.h"
#include "stw_winsys.h"

#include "pipe/p_screen.h"
#include "util/u_memory.h"

struct pipe_screen *
d3d12_wgl_create_screen(struct sw_winsys *winsys, HDC hDC)
{
   LUID *adapter_luid = NULL, local_luid;
   if (stw_dev && stw_dev->callbacks.pfnGetAdapterLuid) {
      stw_dev->callbacks.pfnGetAdapterLuid(hDC, &local_luid);
      adapter_luid = &local_luid;
   }
   return d3d12_create_dxgi_screen(winsys, adapter_luid);
}

void
d3d12_wgl_present(struct pipe_screen *screen,
                  struct pipe_context *ctx,
                  struct pipe_resource *res,
                  HDC hDC)
{
   screen->flush_frontbuffer(screen, ctx, res, 0, 0, hDC, NULL);
}

unsigned
d3d12_wgl_get_pfd_flags(struct pipe_screen *screen)
{
   (void)screen;
   return stw_pfd_gdi_support | stw_pfd_double_buffer;
}
