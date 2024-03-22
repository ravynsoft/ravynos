/**********************************************************
 * Copyright 2008-2012 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "util/u_debug.h"

#include "svga_resource.h"
#include "svga_resource_buffer.h"
#include "svga_resource_texture.h"
#include "svga_context.h"
#include "svga_screen.h"
#include "svga_format.h"


/**
 * This is the primary driver entrypoint for allocating graphics memory
 * (vertex/index/constant buffers, textures, etc)
 */
static struct pipe_resource *
svga_resource_create(struct pipe_screen *screen,
                     const struct pipe_resource *template)
{
   struct pipe_resource *r;

   if (template->target == PIPE_BUFFER)
      r = svga_buffer_create(screen, template);
   else
      r = svga_texture_create(screen, template);

   if (!r) {
      struct svga_screen *svgascreen = svga_screen(screen);
      svgascreen->hud.num_failed_allocations++;
   }

   return r;
}


static struct pipe_resource *
svga_resource_from_handle(struct pipe_screen * screen,
                          const struct pipe_resource *template,
                          struct winsys_handle *whandle,
                          unsigned usage)
{
   if (template->target == PIPE_BUFFER)
      return NULL;
   else
      return svga_texture_from_handle(screen, template, whandle);
}


/**
 * Check if a resource (texture, buffer) of the given size
 * and format can be created.
 * \Return TRUE if OK, FALSE if too large.
 */
static bool
svga_can_create_resource(struct pipe_screen *screen,
                         const struct pipe_resource *res)
{
   struct svga_screen *svgascreen = svga_screen(screen);
   struct svga_winsys_screen *sws = svgascreen->sws;
   SVGA3dSurfaceFormat format;
   SVGA3dSize base_level_size;
   uint32 numMipLevels;
   uint32 arraySize;
   uint32 numSamples;

   if (res->target == PIPE_BUFFER) {
      format = SVGA3D_BUFFER;
      base_level_size.width = res->width0;
      base_level_size.height = 1;
      base_level_size.depth = 1;
      numMipLevels = 1;
      arraySize = 1;
      numSamples = 0;

   } else {
      if (res->target == PIPE_TEXTURE_CUBE)
         assert(res->array_size == 6);

      format = svga_translate_format(svgascreen, res->format, res->bind);
      if (format == SVGA3D_FORMAT_INVALID)
         return false;

      base_level_size.width = res->width0;
      base_level_size.height = res->height0;
      base_level_size.depth = res->depth0;
      numMipLevels = res->last_level + 1;
      arraySize = res->array_size;
      numSamples = res->nr_samples;
   }

   return sws->surface_can_create(sws, format, base_level_size, 
                                  arraySize, numMipLevels, numSamples);
}


void
svga_init_resource_functions(struct svga_context *svga)
{
   svga->pipe.buffer_map = svga_buffer_transfer_map;
   svga->pipe.texture_map = svga_texture_transfer_map;
   svga->pipe.transfer_flush_region = svga_buffer_transfer_flush_region;
   svga->pipe.buffer_unmap = svga_buffer_transfer_unmap;
   svga->pipe.texture_unmap = svga_texture_transfer_unmap;
   svga->pipe.buffer_subdata = u_default_buffer_subdata;
   svga->pipe.texture_subdata = u_default_texture_subdata;

   if (svga_have_vgpu10(svga)) {
      svga->pipe.generate_mipmap = svga_texture_generate_mipmap;
   } else {
      svga->pipe.generate_mipmap = NULL;
   }
}

void
svga_init_screen_resource_functions(struct svga_screen *is)
{
   is->screen.resource_create = svga_resource_create;
   is->screen.resource_from_handle = svga_resource_from_handle;
   is->screen.resource_get_handle = svga_resource_get_handle;
   is->screen.resource_destroy = svga_resource_destroy;
   is->screen.can_create_resource = svga_can_create_resource;
}
