/**************************************************************************
 *
 * Copyright 2009-2010 VMware, Inc.  All Rights Reserved.
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

/**
 * @file
 * Framebuffer utility functions.
 *  
 * @author Brian Paul
 */


#include "pipe/p_screen.h"
#include "pipe/p_state.h"
#include "pipe/p_defines.h"
#include "util/u_inlines.h"

#include "util/u_memory.h"
#include "util/u_framebuffer.h"


/**
 * Compare pipe_framebuffer_state objects.
 * \return TRUE if same, FALSE if different
 */
bool
util_framebuffer_state_equal(const struct pipe_framebuffer_state *dst,
                             const struct pipe_framebuffer_state *src)
{
   unsigned i;

   if (dst->width != src->width ||
       dst->height != src->height)
      return false;

   if (dst->samples != src->samples ||
       dst->layers  != src->layers)
      return false;

   if (dst->nr_cbufs != src->nr_cbufs) {
      return false;
   }

   for (i = 0; i < src->nr_cbufs; i++) {
      if (dst->cbufs[i] != src->cbufs[i]) {
         return false;
      }
   }

   if (dst->zsbuf != src->zsbuf) {
      return false;
   }

   if (dst->resolve != src->resolve) {
      return false;
   }

   return true;
}


/**
 * Copy framebuffer state from src to dst, updating refcounts.
 */
void
util_copy_framebuffer_state(struct pipe_framebuffer_state *dst,
                            const struct pipe_framebuffer_state *src)
{
   unsigned i;

   if (src) {
      dst->width = src->width;
      dst->height = src->height;

      dst->samples = src->samples;
      dst->layers  = src->layers;

      for (i = 0; i < src->nr_cbufs; i++)
         pipe_surface_reference(&dst->cbufs[i], src->cbufs[i]);

      /* Set remaining dest cbuf pointers to NULL */
      for ( ; i < ARRAY_SIZE(dst->cbufs); i++)
         pipe_surface_reference(&dst->cbufs[i], NULL);

      dst->nr_cbufs = src->nr_cbufs;

      pipe_surface_reference(&dst->zsbuf, src->zsbuf);
      pipe_resource_reference(&dst->resolve, src->resolve);
   } else {
      dst->width = 0;
      dst->height = 0;

      dst->samples = 0;
      dst->layers  = 0;

      for (i = 0 ; i < ARRAY_SIZE(dst->cbufs); i++)
         pipe_surface_reference(&dst->cbufs[i], NULL);

      dst->nr_cbufs = 0;

      pipe_surface_reference(&dst->zsbuf, NULL);
      pipe_resource_reference(&dst->resolve, NULL);
   }
}


void
util_unreference_framebuffer_state(struct pipe_framebuffer_state *fb)
{
   unsigned i;

   for (i = 0; i < fb->nr_cbufs; i++) {
      pipe_surface_reference(&fb->cbufs[i], NULL);
   }

   pipe_surface_reference(&fb->zsbuf, NULL);
   pipe_resource_reference(&fb->resolve, NULL);

   fb->samples = fb->layers = 0;
   fb->width = fb->height = 0;
   fb->nr_cbufs = 0;
}


/* Where multiple sizes are allowed for framebuffer surfaces, find the
 * minimum width and height of all bound surfaces.
 */
bool
util_framebuffer_min_size(const struct pipe_framebuffer_state *fb,
                          unsigned *width,
                          unsigned *height)
{
   unsigned w = ~0;
   unsigned h = ~0;
   unsigned i;

   for (i = 0; i < fb->nr_cbufs; i++) {
      if (!fb->cbufs[i])
         continue;

      w = MIN2(w, fb->cbufs[i]->width);
      h = MIN2(h, fb->cbufs[i]->height);
   }

   if (fb->zsbuf) {
      w = MIN2(w, fb->zsbuf->width);
      h = MIN2(h, fb->zsbuf->height);
   }

   if (w == ~0u) {
      *width = 0;
      *height = 0;
      return false;
   }
   else {
      *width = w;
      *height = h;
      return true;
   }
}


/**
 * Return the number of layers set in the framebuffer state.
 */
unsigned
util_framebuffer_get_num_layers(const struct pipe_framebuffer_state *fb)
{
	unsigned i, num_layers = 0;

	/**
	 * In the case of ARB_framebuffer_no_attachment
	 * we obtain the number of layers directly from
	 * the framebuffer state.
	 */
	if (!(fb->nr_cbufs || fb->zsbuf))
		return fb->layers;

	for (i = 0; i < fb->nr_cbufs; i++) {
		if (fb->cbufs[i]) {
			unsigned num = fb->cbufs[i]->u.tex.last_layer -
				       fb->cbufs[i]->u.tex.first_layer + 1;
			num_layers = MAX2(num_layers, num);
		}
	}
	if (fb->zsbuf) {
		unsigned num = fb->zsbuf->u.tex.last_layer -
			       fb->zsbuf->u.tex.first_layer + 1;
		num_layers = MAX2(num_layers, num);
	}
	return num_layers;
}


/**
 * Return the number of MSAA samples.
 */
unsigned
util_framebuffer_get_num_samples(const struct pipe_framebuffer_state *fb)
{
   unsigned i;

   /**
    * In the case of ARB_framebuffer_no_attachment
    * we obtain the number of samples directly from
    * the framebuffer state.
    *
    * NOTE: fb->samples may wind up as zero due to memset()'s on internal
    *       driver structures on their initialization and so we take the
    *       MAX here to ensure we have a valid number of samples. However,
    *       if samples is legitimately not getting set somewhere
    *       multi-sampling will evidently break.
    */
   if (!(fb->nr_cbufs || fb->zsbuf))
      return MAX2(fb->samples, 1);

   /**
    * If a driver doesn't advertise PIPE_CAP_SURFACE_SAMPLE_COUNT,
    * pipe_surface::nr_samples will always be 0.
    */
   for (i = 0; i < fb->nr_cbufs; i++) {
      if (fb->cbufs[i]) {
         return MAX3(1, fb->cbufs[i]->texture->nr_samples,
                     fb->cbufs[i]->nr_samples);
      }
   }
   if (fb->zsbuf) {
      return MAX3(1, fb->zsbuf->texture->nr_samples,
                  fb->zsbuf->nr_samples);
   }

   return MAX2(fb->samples, 1);
}


/**
 * Flip the sample location state along the Y axis.
 */
void
util_sample_locations_flip_y(struct pipe_screen *screen, unsigned fb_height,
                             unsigned samples, uint8_t *locations)
{
   unsigned row, i, shift, grid_width, grid_height;
   uint8_t new_locations[
      PIPE_MAX_SAMPLE_LOCATION_GRID_SIZE *
      PIPE_MAX_SAMPLE_LOCATION_GRID_SIZE * 32];

   screen->get_sample_pixel_grid(screen, samples, &grid_width, &grid_height);

   shift = fb_height % grid_height;

   for (row = 0; row < grid_height; row++) {
      unsigned row_size = grid_width * samples;
      for (i = 0; i < row_size; i++) {
         unsigned dest_row = grid_height - row - 1;
         /* this relies on unsigned integer wraparound behaviour */
         dest_row = (dest_row - shift) % grid_height;
         new_locations[dest_row * row_size + i] = locations[row * row_size + i];
      }
   }

   memcpy(locations, new_locations, grid_width * grid_height * samples);
}
