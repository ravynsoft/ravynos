/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based on amdgpu winsys.
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2015 Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "radv_amdgpu_surface.h"
#include "util/bitset.h"
#include "radv_amdgpu_winsys.h"
#include "radv_private.h"
#include "sid.h"

#include "ac_surface.h"

static int
radv_amdgpu_surface_sanity(const struct ac_surf_info *surf_info, const struct radeon_surf *surf)
{
   unsigned type = RADEON_SURF_GET(surf->flags, TYPE);

   if (!surf->blk_w || !surf->blk_h)
      return -EINVAL;

   switch (type) {
   case RADEON_SURF_TYPE_1D:
      if (surf_info->height > 1)
         return -EINVAL;
      FALLTHROUGH;
   case RADEON_SURF_TYPE_2D:
   case RADEON_SURF_TYPE_CUBEMAP:
      if (surf_info->depth > 1 || surf_info->array_size > 1)
         return -EINVAL;
      break;
   case RADEON_SURF_TYPE_3D:
      if (surf_info->array_size > 1)
         return -EINVAL;
      break;
   case RADEON_SURF_TYPE_1D_ARRAY:
      if (surf_info->height > 1)
         return -EINVAL;
      FALLTHROUGH;
   case RADEON_SURF_TYPE_2D_ARRAY:
      if (surf_info->depth > 1)
         return -EINVAL;
      break;
   default:
      return -EINVAL;
   }
   return 0;
}

static int
radv_amdgpu_winsys_surface_init(struct radeon_winsys *_ws, const struct ac_surf_info *surf_info,
                                struct radeon_surf *surf)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(_ws);
   unsigned mode, type;
   int r;

   r = radv_amdgpu_surface_sanity(surf_info, surf);
   if (r)
      return r;

   type = RADEON_SURF_GET(surf->flags, TYPE);
   mode = RADEON_SURF_GET(surf->flags, MODE);

   struct ac_surf_config config;

   memcpy(&config.info, surf_info, sizeof(config.info));
   config.is_1d = type == RADEON_SURF_TYPE_1D || type == RADEON_SURF_TYPE_1D_ARRAY;
   config.is_3d = type == RADEON_SURF_TYPE_3D;
   config.is_cube = type == RADEON_SURF_TYPE_CUBEMAP;
   config.is_array = type == RADEON_SURF_TYPE_1D_ARRAY || type == RADEON_SURF_TYPE_2D_ARRAY;

   return ac_compute_surface(ws->addrlib, &ws->info, &config, mode, surf);
}

static struct ac_addrlib *
radv_amdgpu_get_addrlib(struct radeon_winsys *rws)
{
   struct radv_amdgpu_winsys *ws = radv_amdgpu_winsys(rws);
   return ws->addrlib;
}

void
radv_amdgpu_surface_init_functions(struct radv_amdgpu_winsys *ws)
{
   ws->base.get_addrlib = radv_amdgpu_get_addrlib;
   ws->base.surface_init = radv_amdgpu_winsys_surface_init;
}
