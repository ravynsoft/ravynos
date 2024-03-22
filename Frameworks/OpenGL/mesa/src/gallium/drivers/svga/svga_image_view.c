/**********************************************************
 * Copyright 2022 VMware, Inc.  All rights reserved.
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

#include "pipe/p_defines.h"
#include "util/u_bitmask.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "svga_context.h"
#include "svga_cmd.h"
#include "svga_debug.h"
#include "svga_resource_buffer.h"
#include "svga_resource_texture.h"
#include "svga_surface.h"
#include "svga_sampler_view.h"
#include "svga_format.h"


/**
 * Create a uav object for the specified shader image view
 */
SVGA3dUAViewId
svga_create_uav_image(struct svga_context *svga,
                      const struct pipe_image_view *image)
{
   struct svga_screen *ss = svga_screen(svga->pipe.screen);
   SVGA3dSurfaceFormat svga_format;
   SVGA3dUAViewDesc desc;
   SVGA3dUAViewId uaViewId;

   assert(image);

   /* Make sure the translated svga format supports uav */
   svga_format = svga_translate_format(ss, image->format,
                                       PIPE_BIND_SHADER_IMAGE);
   if (svga_format == SVGA3D_FORMAT_INVALID)
      return SVGA3D_INVALID_ID;

   struct pipe_resource *res = image->resource;
   struct svga_winsys_surface *surf;
   unsigned resourceDim;

   /* resolve target to resource dimension */
   resourceDim = svga_resource_type(res->target);

   memset(&desc, 0, sizeof(desc));

   if (resourceDim == SVGA3D_RESOURCE_BUFFER) {
      unsigned block_width, block_height, bytes_per_block;

      svga_format_size(svga_format, &block_width, &block_height,
                       &bytes_per_block);
      surf = svga_buffer_handle(svga, res, PIPE_BIND_SHADER_IMAGE);
      desc.buffer.firstElement = image->u.buf.offset / bytes_per_block;
      desc.buffer.numElements = image->u.buf.size / bytes_per_block;

      /* mark this buffer as being used in uav */
      struct svga_buffer *sbuf = svga_buffer(res);
      sbuf->uav = true;
   }
   else if (resourceDim == SVGA3D_RESOURCE_TEXTURE1D ||
            resourceDim == SVGA3D_RESOURCE_TEXTURE2D) {

      struct svga_texture *tex = svga_texture(res);
      surf = tex->handle;
      desc.tex.mipSlice = image->u.tex.level;
      desc.tex.firstArraySlice = image->u.tex.first_layer;
      desc.tex.arraySize = image->u.tex.last_layer - image->u.tex.first_layer + 1;
   }
   else {
      assert(resourceDim == SVGA3D_RESOURCE_TEXTURE3D);

      struct svga_texture *tex = svga_texture(res);
      surf = tex->handle;
      desc.tex3D.mipSlice = image->u.tex.level;
      desc.tex3D.firstW = image->u.tex.first_layer;
      desc.tex3D.wSize = image->u.tex.last_layer - image->u.tex.first_layer + 1;
   }

   uaViewId = svga_create_uav(svga, &desc, svga_format, resourceDim, surf);
   if (uaViewId == SVGA3D_INVALID_ID)
      return uaViewId;

   SVGA_DBG(DEBUG_IMAGE, "%s: resource=0x%x dim=%d format=%d uaViewId=%d\n",
            __func__, res, resourceDim, svga_format, uaViewId);

   return uaViewId;
}


/**
 * Set shader images
 */
static void
svga_set_shader_images(struct pipe_context *pipe,
                       enum pipe_shader_type shader,
                       unsigned start,
                       unsigned num,
                       unsigned unbind_num_trailing_slots,
                       const struct pipe_image_view *images)
{
   struct svga_context *svga = svga_context(pipe);
   const struct pipe_image_view *img = images;

   assert(svga_have_gl43(svga));

   assert(start + num <= SVGA_MAX_IMAGES);

   if (images) {
      for (unsigned i = start; i < start + num; i++, img++) {
         struct svga_image_view *cur_image_view = &svga->curr.image_views[shader][i];

         if (img) {
            cur_image_view->desc = *img;
            if (img->resource == NULL) {
               /* Use a dummy resource if the image view is created with a NULL resource */
               if (svga->dummy_resource == NULL) {
                  struct svga_screen *ss = svga_screen(svga->pipe.screen);
                  struct pipe_resource templ;
                  struct pipe_resource *res;
                  templ.target = PIPE_BUFFER;
                  templ.format = PIPE_FORMAT_R8_UNORM;
                  templ.bind = PIPE_BIND_SHADER_BUFFER;
                  templ.width0 = 64;
                  templ.height0 = 1;
                  templ.depth0 = 1;
                  templ.array_size = 1;
                  res = ss->screen.resource_create(&ss->screen, &templ);
                  pipe_resource_reference(&svga->dummy_resource, res);
               }
               pipe_resource_reference(&cur_image_view->resource,
                                       svga->dummy_resource);
            }
            else {
               pipe_resource_reference(&cur_image_view->resource,
                                       img->resource);
            }
         }
         else {
            pipe_resource_reference(&cur_image_view->resource, NULL);
         }
         cur_image_view->uav_index = -1;
      }
   }

   /* unbind trailing slots */
   for (unsigned j = 0, i = start + num; j < unbind_num_trailing_slots;
        i++, j++) {
      struct svga_image_view *cur_image_view = &svga->curr.image_views[shader][i];
      cur_image_view->uav_index = -1;
      pipe_resource_reference(&cur_image_view->resource, NULL);
   }

   /* number of bound image views */
   svga->curr.num_image_views[shader] = start + num;

#ifdef DEBUG
   SVGA_DBG(DEBUG_UAV, "%s: num_image_views=%d start=%d num=%d unbind_num_trailing_slots=%d\n",
            __func__, svga->curr.num_image_views[shader], start, num,
            unbind_num_trailing_slots);

   for (unsigned i = start; i < start + num; i++) {
      struct svga_image_view *cur_image_view = &svga->curr.image_views[shader][i];
      struct pipe_image_view *img = &cur_image_view->desc;
      if (img->resource) {
         if (img->resource->target == PIPE_BUFFER) {
            SVGA_DBG(DEBUG_UAV, "   buffer res=0x%x format=%d offset=%d size=%d\n",
                     img->resource, img->format,
                     img->u.buf.offset, img->u.buf.size);
         }
         else {
            SVGA_DBG(DEBUG_UAV,
               "   texture res=0x%x format=%d first_layer=%d last_layer=%d level=%d\n",
                     img->resource, img->format, img->u.tex.first_layer,
                     img->u.tex.last_layer, img->u.tex.level);
         }
      }
      else {
         SVGA_DBG(DEBUG_UAV, "   res=NULL\n");
      }
   }

   SVGA_DBG(DEBUG_UAV, "\n");
#endif

   /* purge any unused uav objects */
   svga_destroy_uav(svga);

   svga->dirty |= SVGA_NEW_IMAGE_VIEW;
}


/**
 *  Initialize shader images gallium interface
 */
void
svga_init_shader_image_functions(struct svga_context *svga)
{
   if (svga_have_gl43(svga)) {
      svga->pipe.set_shader_images = svga_set_shader_images;
   }

   /* Initialize shader image views */
   for (unsigned shader = 0; shader < PIPE_SHADER_TYPES; ++shader) {
      struct svga_image_view *hw_image_views =
         &svga->state.hw_draw.image_views[shader][0];
      struct svga_image_view *cur_image_views =
         &svga->curr.image_views[shader][0];

      for (unsigned i = 0; i < ARRAY_SIZE(svga->curr.image_views[shader]);
           i++, hw_image_views++, cur_image_views++) {
         hw_image_views->resource = NULL;
         cur_image_views->resource = NULL;
      }
   }
   memset(svga->state.hw_draw.num_image_views, 0,
          sizeof(svga->state.hw_draw.num_image_views));
}


/**
 * Cleanup shader image state
 */
void
svga_cleanup_shader_image_state(struct svga_context *svga)
{
   if (!svga_have_gl43(svga))
      return;

   svga_destroy_uav(svga);
}


/**
 * Validate shader image view resources to ensure any pending changes to
 * texture buffers are emitted before they are referenced in image views.
 * The helper function also rebinds the image view resources if the rebind flag
 * is specified.
 */
enum pipe_error
svga_validate_image_view_resources(struct svga_context *svga,
                                   unsigned count,
                                   struct svga_image_view *images,
                                   bool rebind)
{
   assert(svga_have_gl43(svga));

   struct svga_winsys_surface *surf;
   enum pipe_error ret;
   unsigned i;

   for (i = 0; i < count; i++) {
      struct pipe_resource *res = images[i].resource;
      if (res) {
         assert(res == images[i].desc.resource);
         if (res->target == PIPE_BUFFER) {
            struct svga_buffer *sbuf = svga_buffer(res);

            surf = svga_buffer_handle(svga, res, PIPE_BIND_SHADER_IMAGE);
            /* Mark buffer surface as RENDERED */
            svga_set_buffer_rendered_to(sbuf->bufsurf);
         } else {
            struct svga_texture *tex = svga_texture(res);

            surf = tex->handle;
            /* Mark texture as RENDERED */
            svga_set_texture_rendered_to(tex);
         }

         assert(surf);
         if (rebind) {
            ret = svga->swc->resource_rebind(svga->swc, surf, NULL,
                                             SVGA_RELOC_READ|SVGA_RELOC_WRITE);
            if (ret != PIPE_OK)
               return ret;
         }
      }
   }

   return PIPE_OK;
}
