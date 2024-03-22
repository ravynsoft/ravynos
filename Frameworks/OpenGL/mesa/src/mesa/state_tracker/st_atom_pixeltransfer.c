/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
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

/* Authors:
 *   Brian Paul
 */

#include "st_context.h"
#include "st_sampler_view.h"
#include "st_texture.h"

#include "util/u_inlines.h"
#include "util/u_pack_color.h"


/**
 * Update the pixelmap texture with the contents of the R/G/B/A pixel maps.
 */
static void
load_color_map_texture(struct gl_context *ctx, struct pipe_resource *pt)
{
   struct st_context *st = st_context(ctx);
   struct pipe_context *pipe = st->pipe;
   struct pipe_transfer *transfer;
   const GLuint rSize = ctx->PixelMaps.RtoR.Size;
   const GLuint gSize = ctx->PixelMaps.GtoG.Size;
   const GLuint bSize = ctx->PixelMaps.BtoB.Size;
   const GLuint aSize = ctx->PixelMaps.AtoA.Size;
   const uint texSize = pt->width0;
   uint *dest;
   uint i, j;

   dest = (uint *) pipe_texture_map(pipe,
                                     pt, 0, 0, PIPE_MAP_WRITE,
                                     0, 0, texSize, texSize, &transfer);

   /* Pack four 1D maps into a 2D texture:
    * R map is placed horizontally, indexed by S, in channel 0
    * G map is placed vertically, indexed by T, in channel 1
    * B map is placed horizontally, indexed by S, in channel 2
    * A map is placed vertically, indexed by T, in channel 3
    */
   for (i = 0; i < texSize; i++) {
      for (j = 0; j < texSize; j++) {
         union util_color uc;
         int k = (i * texSize + j);
         float rgba[4];
         rgba[0] = ctx->PixelMaps.RtoR.Map[j * rSize / texSize];
         rgba[1] = ctx->PixelMaps.GtoG.Map[i * gSize / texSize];
         rgba[2] = ctx->PixelMaps.BtoB.Map[j * bSize / texSize];
         rgba[3] = ctx->PixelMaps.AtoA.Map[i * aSize / texSize];
         util_pack_color(rgba, pt->format, &uc);
         *(dest + k) = uc.ui[0];
      }
   }

   pipe_texture_unmap(pipe, transfer);
}

/**
 * Upload the pixel transfer color map texture.
 */
void
st_update_pixel_transfer(struct st_context *st)
{
   struct gl_context *ctx = st->ctx;

   if (ctx->Pixel.MapColorFlag) {
      /* create the colormap/texture now if not already done */
      if (!st->pixel_xfer.pixelmap_texture) {
         st->pixel_xfer.pixelmap_texture = st_create_color_map_texture(ctx);
         st->pixel_xfer.pixelmap_sampler_view =
            st_create_texture_sampler_view(st->pipe,
                                           st->pixel_xfer.pixelmap_texture);
      }
      load_color_map_texture(ctx, st->pixel_xfer.pixelmap_texture);
   }
}
