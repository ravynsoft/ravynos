/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
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
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#include "st_context.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "st_atom.h"
#include "st_program.h"
#include "st_util.h"

#include "cso_cache/cso_context.h"
#include "util/u_framebuffer.h"
#include "main/framebuffer.h"


/**
 * Update the sample locations
 */
static void
update_sample_locations(struct st_context *st)
{
   struct gl_framebuffer *fb = st->ctx->DrawBuffer;

   if (!st->ctx->Extensions.ARB_sample_locations)
      return;

   if (fb->ProgrammableSampleLocations) {
      unsigned grid_width, grid_height, size, pixel, sample_index;
      unsigned samples = st->state.fb_num_samples;
      bool sample_location_pixel_grid = fb->SampleLocationPixelGrid;
      uint8_t locations[
         PIPE_MAX_SAMPLE_LOCATION_GRID_SIZE *
         PIPE_MAX_SAMPLE_LOCATION_GRID_SIZE * 32];

      st->screen->get_sample_pixel_grid(st->screen, samples,
                                        &grid_width, &grid_height);
      size = grid_width * grid_height * samples;

      /**
       * when a dimension is greater than MAX_SAMPLE_LOCATION_GRID_SIZE,
       * st->ctx->Driver.GetSamplePixelGrid() returns 1 for both dimensions.
       */
      if (grid_width > MAX_SAMPLE_LOCATION_GRID_SIZE ||
          grid_height > MAX_SAMPLE_LOCATION_GRID_SIZE)
         sample_location_pixel_grid = false;

      for (pixel = 0; pixel < grid_width * grid_height; pixel++) {
         for (sample_index = 0; sample_index < samples; sample_index++) {
            int table_index = sample_index;
            float x = 0.5f, y = 0.5f;
            uint8_t loc;
            if (sample_location_pixel_grid)
               table_index = pixel * samples + sample_index;
            if (fb->SampleLocationTable) {
               x = fb->SampleLocationTable[table_index*2];
               y = fb->SampleLocationTable[table_index*2+1];
            }
            if (st->state.fb_orientation == Y_0_BOTTOM)
               y = 1.0 - y;

            loc = roundf(CLAMP(x * 16.0f, 0.0f, 15.0f));
            loc |= (int)roundf(CLAMP(y * 16.0f, 0.0f, 15.0f)) << 4;
            locations[pixel * samples + sample_index] = loc;
         }
      }

      util_sample_locations_flip_y(
         st->screen, st->state.fb_height, samples, locations);

      if (!st->state.enable_sample_locations ||
          st->state.sample_locations_samples != samples ||
          memcmp(locations, st->state.sample_locations, size) != 0) {
         st->pipe->set_sample_locations( st->pipe, size, locations);
         
         st->state.sample_locations_samples = samples;
         memcpy(st->state.sample_locations, locations, size);
      }
   } else if (st->state.enable_sample_locations) {
      st->pipe->set_sample_locations(st->pipe, 0, NULL);
   }

   st->state.enable_sample_locations = fb->ProgrammableSampleLocations;
}


/* Update the sample mask and locations for MSAA.
 */
void
st_update_sample_state(struct st_context *st)
{
   unsigned sample_mask = 0xffffffff;
   unsigned sample_count = st->state.fb_num_samples;

   if (_mesa_is_multisample_enabled(st->ctx) && sample_count > 1) {
      /* unlike in gallium/d3d10 the mask is only active if msaa is enabled */
      if (st->ctx->Multisample.SampleCoverage) {
         unsigned nr_bits = (unsigned)
            (st->ctx->Multisample.SampleCoverageValue * (float) sample_count);
         /* there's lot of ways how to do this. We just use first few bits,
          * since we have no knowledge of sample positions here. When
          * app-supplied mask though is used too might need to be smarter.
          * Also, there's an interface restriction here in theory it is
          * encouraged this mask not be the same at each pixel.
          */
         sample_mask = (1 << nr_bits) - 1;
         if (st->ctx->Multisample.SampleCoverageInvert)
            sample_mask = ~sample_mask;
      }
      if (st->ctx->Multisample.SampleMask)
         sample_mask &= st->ctx->Multisample.SampleMaskValue;
   }

   cso_set_sample_mask(st->cso_context, sample_mask);

   update_sample_locations(st);
}


void
st_update_sample_shading(struct st_context *st)
{
   struct gl_program *fp = st->ctx->FragmentProgram._Current;

   if (!fp || !st->ctx->Extensions.ARB_sample_shading)
      return;

   cso_set_min_samples(st->cso_context,
                       _mesa_get_min_invocations_per_fragment(st->ctx, fp));
}
