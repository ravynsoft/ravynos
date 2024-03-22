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

#include "pipe/p_screen.h"

#include "util/u_memory.h"
#include "vl/vl_winsys.h"

#include "target-helpers/sw_helper_public.h"
#include "gallium/winsys/sw/kms-dri/kms_dri_sw_winsys.h"

static void
vl_vgem_drm_screen_destroy(struct vl_screen *vscreen)
{
   if (vscreen) {
      if (vscreen->pscreen)
         vscreen->pscreen->destroy(vscreen->pscreen);
      FREE(vscreen);
   }
}

struct vl_screen *
vl_vgem_drm_screen_create(int fd)
{
   struct vl_screen *vscreen = CALLOC_STRUCT(vl_screen);
   if (!vscreen)
      return NULL;

   struct sw_winsys *winsys = kms_dri_create_winsys(fd);
   if (winsys == NULL)
      goto release_pipe;

   vscreen->pscreen = sw_screen_create(winsys);
   if (!vscreen->pscreen)
      goto release_pipe;

   vscreen->destroy = vl_vgem_drm_screen_destroy;
   vscreen->texture_from_drawable = NULL;
   vscreen->get_dirty_area = NULL;
   vscreen->get_timestamp = NULL;
   vscreen->set_next_timestamp = NULL;
   vscreen->get_private = NULL;
   return vscreen;

release_pipe:
   vl_vgem_drm_screen_destroy(vscreen);
   return NULL;
}
