/*
 * Copyright 2016 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "sp_context.h"
#include "sp_state.h"
#include "sp_image.h"
#include "sp_buffer.h"

static void softpipe_set_shader_images(struct pipe_context *pipe,
                                       enum pipe_shader_type shader,
                                       unsigned start,
                                       unsigned num,
                                       unsigned unbind_num_trailing_slots,
                                       const struct pipe_image_view *images)
{
   struct softpipe_context *softpipe = softpipe_context(pipe);
   unsigned i;
   assert(shader < PIPE_SHADER_TYPES);
   assert(start + num <= ARRAY_SIZE(softpipe->sampler_views[shader]));

   /* set the new images */
   for (i = 0; i < num; i++) {
      int idx = start + i;

      if (images) {
         pipe_resource_reference(&softpipe->tgsi.image[shader]->sp_iview[idx].resource, images[i].resource);
         softpipe->tgsi.image[shader]->sp_iview[idx] = images[i];
      }
      else {
         pipe_resource_reference(&softpipe->tgsi.image[shader]->sp_iview[idx].resource, NULL);
         memset(&softpipe->tgsi.image[shader]->sp_iview[idx], 0, sizeof(struct pipe_image_view));
      }
   }

   for (i = 0; i < unbind_num_trailing_slots; i++) {
      int idx = start + num + i;

      pipe_resource_reference(&softpipe->tgsi.image[shader]->sp_iview[idx].resource, NULL);
      memset(&softpipe->tgsi.image[shader]->sp_iview[idx], 0, sizeof(struct pipe_image_view));
   }
}

static void softpipe_set_shader_buffers(struct pipe_context *pipe,
                                        enum pipe_shader_type shader,
                                        unsigned start,
                                        unsigned num,
                                        const struct pipe_shader_buffer *buffers,
                                        unsigned writable_bitmask)
{
   struct softpipe_context *softpipe = softpipe_context(pipe);
   unsigned i;
   assert(shader < PIPE_SHADER_TYPES);
   assert(start + num <= ARRAY_SIZE(softpipe->buffers[shader]));

   /* set the new images */
   for (i = 0; i < num; i++) {
      int idx = start + i;

      if (buffers) {
         pipe_resource_reference(&softpipe->tgsi.buffer[shader]->sp_bview[idx].buffer, buffers[i].buffer);
         softpipe->tgsi.buffer[shader]->sp_bview[idx] = buffers[i];
      }
      else {
         pipe_resource_reference(&softpipe->tgsi.buffer[shader]->sp_bview[idx].buffer, NULL);
         memset(&softpipe->tgsi.buffer[shader]->sp_bview[idx], 0, sizeof(struct pipe_shader_buffer));
      }
   }
}

void softpipe_init_image_funcs(struct pipe_context *pipe)
{
   pipe->set_shader_images = softpipe_set_shader_images;
   pipe->set_shader_buffers = softpipe_set_shader_buffers;
}
