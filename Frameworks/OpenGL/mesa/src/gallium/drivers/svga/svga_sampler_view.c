/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
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

#include "svga_cmd.h"

#include "pipe/p_state.h"
#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "util/u_thread.h"
#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "svga_format.h"
#include "svga_screen.h"
#include "svga_context.h"
#include "svga_resource_texture.h"
#include "svga_sampler_view.h"
#include "svga_debug.h"
#include "svga_surface.h"


void
svga_debug_describe_sampler_view(char *buf, const struct svga_sampler_view *sv)
{
   char res[128];
   debug_describe_resource(res, sv->texture);
   sprintf(buf, "svga_sampler_view<%s,[%u,%u]>",
                res, sv->min_lod, sv->max_lod);
}


struct svga_sampler_view *
svga_get_tex_sampler_view(struct pipe_context *pipe,
			  struct pipe_resource *pt,
                          unsigned min_lod, unsigned max_lod)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_screen *ss = svga_screen(pipe->screen);
   struct svga_texture *tex = svga_texture(pt);
   struct svga_sampler_view *sv = NULL;
   SVGA3dSurface1Flags flags = SVGA3D_SURFACE_HINT_TEXTURE;
   SVGA3dSurfaceFormat format = svga_translate_format(ss, pt->format,
                                                      PIPE_BIND_SAMPLER_VIEW);
   bool view = true;

   assert(pt);
   assert(min_lod <= max_lod);
   assert(max_lod <= pt->last_level);
   assert(!svga_have_vgpu10(svga));

   /* Is a view needed */
   {
      /*
       * Can't control max lod. For first level views and when we only
       * look at one level we disable mip filtering to achive the same
       * results as a view.
       */
      if (min_lod == 0 && max_lod >= pt->last_level)
         view = false;

      if (ss->debug.no_sampler_view)
         view = false;

      if (ss->debug.force_sampler_view)
         view = true;
   }

   /* First try the cache */
   if (view) {
      mtx_lock(&ss->tex_mutex);
      if (tex->cached_view &&
          tex->cached_view->min_lod == min_lod &&
          tex->cached_view->max_lod == max_lod) {
         svga_sampler_view_reference(&sv, tex->cached_view);
         mtx_unlock(&ss->tex_mutex);
         SVGA_DBG(DEBUG_VIEWS, "svga: Sampler view: reuse %p, %u %u, last %u\n",
                              pt, min_lod, max_lod, pt->last_level);
         svga_validate_sampler_view(svga_context(pipe), sv);
         return sv;
      }
      mtx_unlock(&ss->tex_mutex);
   }

   sv = CALLOC_STRUCT(svga_sampler_view);
   if (!sv)
      return NULL;

   pipe_reference_init(&sv->reference, 1);

   /* Note: we're not refcounting the texture resource here to avoid
    * a circular dependency.
    */
   sv->texture = pt;

   sv->min_lod = min_lod;
   sv->max_lod = max_lod;

   /* No view needed just use the whole texture */
   if (!view) {
      SVGA_DBG(DEBUG_VIEWS,
               "svga: Sampler view: no %p, mips %u..%u, nr %u, size (%ux%ux%u), last %u\n",
               pt, min_lod, max_lod,
               max_lod - min_lod + 1,
               pt->width0,
               pt->height0,
               pt->depth0,
               pt->last_level);
      sv->key.cachable = 0;
      sv->handle = tex->handle;
      debug_reference(&sv->reference,
                      (debug_reference_descriptor)svga_debug_describe_sampler_view, 0);
      return sv;
   }

   SVGA_DBG(DEBUG_VIEWS,
            "svga: Sampler view: yes %p, mips %u..%u, nr %u, size (%ux%ux%u), last %u\n",
            pt, min_lod, max_lod,
            max_lod - min_lod + 1,
            pt->width0,
            pt->height0,
            pt->depth0,
            pt->last_level);

   sv->age = tex->age;
   sv->handle = svga_texture_view_surface(svga, tex,
                                          PIPE_BIND_SAMPLER_VIEW,
                                          flags, format,
                                          min_lod,
                                          max_lod - min_lod + 1,
                                          -1, 1, -1, false,
                                          &sv->key);

   if (!sv->handle) {
      sv->key.cachable = 0;
      sv->handle = tex->handle;
      debug_reference(&sv->reference,
                      (debug_reference_descriptor)
                      svga_debug_describe_sampler_view, 0);
      return sv;
   }

   mtx_lock(&ss->tex_mutex);
   svga_sampler_view_reference(&tex->cached_view, sv);
   mtx_unlock(&ss->tex_mutex);

   debug_reference(&sv->reference,
                   (debug_reference_descriptor)
                   svga_debug_describe_sampler_view, 0);

   return sv;
}


void
svga_validate_sampler_view(struct svga_context *svga,
                           struct svga_sampler_view *v)
{
   struct svga_texture *tex = svga_texture(v->texture);
   unsigned numFaces;
   unsigned age = 0;
   int i;
   unsigned k;

   assert(svga);
   assert(!svga_have_vgpu10(svga));

   if (v->handle == tex->handle)
      return;

   age = tex->age;

   if (tex->b.target == PIPE_TEXTURE_CUBE)
      numFaces = 6;
   else
      numFaces = 1;

   for (i = v->min_lod; i <= v->max_lod; i++) {
      for (k = 0; k < numFaces; k++) {
         assert(i < ARRAY_SIZE(tex->view_age));
         if (v->age < tex->view_age[i])
            svga_texture_copy_handle(svga,
                                     tex->handle, 0, 0, 0, i, k,
                                     v->handle, 0, 0, 0, i - v->min_lod, k,
                                     u_minify(tex->b.width0, i),
                                     u_minify(tex->b.height0, i),
                                     u_minify(tex->b.depth0, i));
      }
   }

   v->age = age;
}


void
svga_destroy_sampler_view_priv(struct svga_sampler_view *v)
{
   struct svga_texture *tex = svga_texture(v->texture);

   if (v->handle != tex->handle) {
      struct svga_screen *ss = svga_screen(v->texture->screen);
      SVGA_DBG(DEBUG_DMA, "unref sid %p (sampler view)\n", v->handle);
      svga_screen_surface_destroy(ss, &v->key,
                                  svga_was_texture_rendered_to(tex),
                                  &v->handle);
   }

   /* Note: we're not refcounting the texture resource here to avoid
    * a circular dependency.
    */
   v->texture = NULL;

   FREE(v);
}
