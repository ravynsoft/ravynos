/**************************************************************************
 *
 * Copyright 2011 Lauri Kasanen
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
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "util/compiler.h"

#include "postprocess/filters.h"
#include "postprocess/pp_private.h"

#include "pipe/p_screen.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_debug.h"
#include "util/u_memory.h"
#include "cso_cache/cso_context.h"

const struct pp_filter_t pp_filters[PP_FILTERS] = {
/*    name			inner	shaders	verts	init			run                       free   */
   { "pp_noblue",		0,	2,	1,	pp_noblue_init,		pp_nocolor,               pp_nocolor_free },
   { "pp_nogreen",		0,	2,	1,	pp_nogreen_init,	pp_nocolor,               pp_nocolor_free },
   { "pp_nored",		0,	2,	1,	pp_nored_init,		pp_nocolor,               pp_nocolor_free },
   { "pp_celshade",		0,	2,	1,	pp_celshade_init,	pp_nocolor,               pp_celshade_free },
   { "pp_jimenezmlaa",		2,	5,	2,	pp_jimenezmlaa_init,	pp_jimenezmlaa,           pp_jimenezmlaa_free },
   { "pp_jimenezmlaa_color",	2,	5,	2,	pp_jimenezmlaa_init_color, pp_jimenezmlaa_color,  pp_jimenezmlaa_free },
};

/** Initialize the post-processing queue. */
struct pp_queue_t *
pp_init(struct pipe_context *pipe, const unsigned int *enabled,
        struct cso_context *cso, struct st_context *st,
        pp_st_invalidate_state_func st_invalidate_state)
{
   unsigned int num_filters = 0;
   unsigned int curpos = 0, i, tmp_req = 0;
   struct pp_queue_t *ppq;

   pp_debug("Initializing the post-processing queue.\n");

   /* How many filters were requested? */
   for (i = 0; i < PP_FILTERS; i++) {
      if (enabled[i])
         num_filters++;
   }
   if (num_filters == 0)
      return NULL;

   ppq = CALLOC(1, sizeof(struct pp_queue_t));

   if (!ppq) {
      pp_debug("Unable to allocate memory for ppq.\n");
      goto error;
   }

   ppq->pp_queue = CALLOC(num_filters, sizeof(pp_func));
   if (ppq->pp_queue == NULL) {
      pp_debug("Unable to allocate memory for pp_queue.\n");
      goto error;
   }

   ppq->shaders = CALLOC(num_filters, sizeof(void *));
   ppq->filters = CALLOC(num_filters, sizeof(unsigned int));

   if ((ppq->shaders == NULL) ||
       (ppq->filters == NULL)) {
      pp_debug("Unable to allocate memory for shaders and filter arrays.\n");
      goto error;
   }

   ppq->p = pp_init_prog(ppq, pipe, cso, st, st_invalidate_state);
   if (ppq->p == NULL) {
      pp_debug("pp_init_prog returned NULL.\n");
      goto error;
   }

   /* Add the enabled filters to the queue, in order */
   curpos = 0;
   for (i = 0; i < PP_FILTERS; i++) {
      if (enabled[i]) {
         ppq->pp_queue[curpos] = pp_filters[i].main;
         tmp_req = MAX2(tmp_req, pp_filters[i].inner_tmps);
         ppq->filters[curpos] = i;

         if (pp_filters[i].shaders) {
            ppq->shaders[curpos] =
               CALLOC(pp_filters[i].shaders + 1, sizeof(void *));
            if (!ppq->shaders[curpos]) {
               pp_debug("Unable to allocate memory for shader list.\n");
               goto error;
            }
         }

         /* Call the initialization function for the filter. */
         if (!pp_filters[i].init(ppq, curpos, enabled[i])) {
            pp_debug("Initialization for filter %u failed.\n", i);
            goto error;
         }           

         curpos++;
      }
   }

   ppq->n_filters = curpos;
   ppq->n_tmp = (curpos > 2 ? 2 : 1);
   ppq->n_inner_tmp = tmp_req;

   ppq->fbos_init = false;

   for (i = 0; i < curpos; i++)
      ppq->shaders[i][0] = ppq->p->passvs;

   pp_debug("Queue successfully allocated. %u filter(s).\n", curpos);
   
   return ppq;

 error:

   if (ppq) {
      /* Assign curpos, since we only need to destroy initialized filters. */
      ppq->n_filters = curpos;

      /* Call the common free function which must handle partial initialization. */
      pp_free(ppq);
   }

   return NULL;
}

/** Free any allocated FBOs (temp buffers). Called after resizing for example. */
void
pp_free_fbos(struct pp_queue_t *ppq)
{

   unsigned int i;

   if (!ppq->fbos_init)
      return;

   for (i = 0; i < ppq->n_tmp; i++) {
      pipe_surface_reference(&ppq->tmps[i], NULL);
      pipe_resource_reference(&ppq->tmp[i], NULL);
   }
   for (i = 0; i < ppq->n_inner_tmp; i++) {
      pipe_surface_reference(&ppq->inner_tmps[i], NULL);
      pipe_resource_reference(&ppq->inner_tmp[i], NULL);
   }
   pipe_surface_reference(&ppq->stencils, NULL);
   pipe_resource_reference(&ppq->stencil, NULL);

   ppq->fbos_init = false;
}

/** 
 * Free the pp queue. Called on context termination and failure in
 * pp_init.
 */
void
pp_free(struct pp_queue_t *ppq)
{
   unsigned int i, j;

   if (!ppq)
      return;

   pp_free_fbos(ppq);

   if (ppq->p) {
      if (ppq->p->pipe && ppq->filters && ppq->shaders) {
         for (i = 0; i < ppq->n_filters; i++) {
            unsigned int filter = ppq->filters[i];

            if (ppq->shaders[i] == NULL) {
               continue;
            }

            /*
             * Common shader destruction code for all postprocessing
             * filters.
             */
            for (j = 0; j < pp_filters[filter].shaders; j++) {
               if (ppq->shaders[i][j] == NULL) {
                  /* We reached the end of initialized shaders. */
                  break;
               }

               if (ppq->shaders[i][j] == ppq->p->passvs) {
                  continue;
               }

               assert(ppq);
               assert(ppq->p);
               assert(ppq->p->pipe);
 
               if (j >= pp_filters[filter].verts) {
                  assert(ppq->p->pipe->delete_fs_state);
                  ppq->p->pipe->delete_fs_state(ppq->p->pipe,
                                                ppq->shaders[i][j]);
                  ppq->shaders[i][j] = NULL;
               } else {
                  assert(ppq->p->pipe->delete_vs_state);
                  ppq->p->pipe->delete_vs_state(ppq->p->pipe,
                                                ppq->shaders[i][j]);
                  ppq->shaders[i][j] = NULL;
               }
            }

            /* Finally call each filter type's free functionality. */
            pp_filters[filter].free(ppq, i);
         }
      }

      FREE(ppq->p);
   }

   /*
    * Handle partial initialization for common resource destruction
    * in the create path.
    */
   FREE(ppq->filters);
   FREE(ppq->shaders);
   FREE(ppq->pp_queue);
  
   FREE(ppq);

   pp_debug("Queue taken down.\n");
}

/** Internal debug function. Should be available to final users. */
void
pp_debug(const char *fmt, ...)
{
   va_list ap;

   if (!debug_get_bool_option("PP_DEBUG", false))
      return;

   va_start(ap, fmt);
   _debug_vprintf(fmt, ap);
   va_end(ap);
}

/** Allocate the temp FBOs. Called on makecurrent and resize. */
void
pp_init_fbos(struct pp_queue_t *ppq, unsigned int w,
             unsigned int h)
{

   struct pp_program *p = ppq->p;  /* The lazy will inherit the earth */

   unsigned int i;
   struct pipe_resource tmp_res;

   if (ppq->fbos_init)
      return;

   pp_debug("Initializing FBOs, size %ux%u\n", w, h);
   pp_debug("Requesting %u temps and %u inner temps\n", ppq->n_tmp,
            ppq->n_inner_tmp);

   memset(&tmp_res, 0, sizeof(tmp_res));
   tmp_res.target = PIPE_TEXTURE_2D;
   tmp_res.format = p->surf.format = PIPE_FORMAT_B8G8R8A8_UNORM;
   tmp_res.width0 = w;
   tmp_res.height0 = h;
   tmp_res.depth0 = 1;
   tmp_res.array_size = 1;
   tmp_res.last_level = 0;
   tmp_res.bind = PIPE_BIND_RENDER_TARGET;

   if (!p->screen->is_format_supported(p->screen, tmp_res.format,
                                       tmp_res.target, 1, 1, tmp_res.bind))
      pp_debug("Temp buffers' format fail\n");

   for (i = 0; i < ppq->n_tmp; i++) {
      ppq->tmp[i] = p->screen->resource_create(p->screen, &tmp_res);
      ppq->tmps[i] = p->pipe->create_surface(p->pipe, ppq->tmp[i], &p->surf);

      if (!ppq->tmp[i] || !ppq->tmps[i])
         goto error;
   }

   for (i = 0; i < ppq->n_inner_tmp; i++) {
      ppq->inner_tmp[i] = p->screen->resource_create(p->screen, &tmp_res);
      ppq->inner_tmps[i] = p->pipe->create_surface(p->pipe,
                                                   ppq->inner_tmp[i],
                                                   &p->surf);

      if (!ppq->inner_tmp[i] || !ppq->inner_tmps[i])
         goto error;
   }

   tmp_res.bind = PIPE_BIND_DEPTH_STENCIL;

   tmp_res.format = p->surf.format = PIPE_FORMAT_S8_UINT_Z24_UNORM;

   if (!p->screen->is_format_supported(p->screen, tmp_res.format,
                                       tmp_res.target, 1, 1, tmp_res.bind)) {

      tmp_res.format = p->surf.format = PIPE_FORMAT_Z24_UNORM_S8_UINT;

      if (!p->screen->is_format_supported(p->screen, tmp_res.format,
                                          tmp_res.target, 1, 1, tmp_res.bind))
         pp_debug("Temp Sbuffer format fail\n");
   }

   ppq->stencil = p->screen->resource_create(p->screen, &tmp_res);
   ppq->stencils = p->pipe->create_surface(p->pipe, ppq->stencil, &p->surf);
   if (!ppq->stencil || !ppq->stencils)
      goto error;

   p->framebuffer.width = w;
   p->framebuffer.height = h;

   p->viewport.scale[0] = p->viewport.translate[0] = (float) w / 2.0f;
   p->viewport.scale[1] = p->viewport.translate[1] = (float) h / 2.0f;
   p->viewport.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
   p->viewport.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
   p->viewport.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
   p->viewport.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;

   ppq->fbos_init = true;

   return;

 error:
   pp_debug("Failed to allocate temp buffers!\n");
}
