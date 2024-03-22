/**************************************************************************
 * 
 * Copyright 2009 VMware, Inc.
 * Copyright 2012 Marek Olšák <maraeo@gmail.com>
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

/* A simple allocator that suballocates memory from a large buffer. */

#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "pipe/p_context.h"
#include "util/u_memory.h"
#include "util/u_math.h"

#include "u_suballoc.h"

/**
 * Create a suballocator.
 *
 * \param flags               bitmask of PIPE_RESOURCE_FLAG_x
 * \param zero_buffer_memory  determines whether the buffer contents should be
 *                            cleared to 0 after the allocation.
 *
 */
void
u_suballocator_init(struct u_suballocator *allocator,
                    struct pipe_context *pipe,
                    unsigned size, unsigned bind,
                    enum pipe_resource_usage usage, unsigned flags,
                    bool zero_buffer_memory)
{
   memset(allocator, 0, sizeof(*allocator));

   allocator->pipe = pipe;
   allocator->size = size;
   allocator->bind = bind;
   allocator->usage = usage;
   allocator->flags = flags;
   allocator->zero_buffer_memory = zero_buffer_memory;
}

void
u_suballocator_destroy(struct u_suballocator *allocator)
{
   pipe_resource_reference(&allocator->buffer, NULL);
}

void
u_suballocator_alloc(struct u_suballocator *allocator, unsigned size,
                     unsigned alignment, unsigned *out_offset,
                     struct pipe_resource **outbuf)
{
   allocator->offset = align(allocator->offset, alignment);

   /* Don't allow allocations larger than the buffer size. */
   if (size > allocator->size)
      goto fail;

   /* Make sure we have enough space in the buffer. */
   if (!allocator->buffer ||
       allocator->offset + size > allocator->size) {
      /* Allocate a new buffer. */
      pipe_resource_reference(&allocator->buffer, NULL);
      allocator->offset = 0;

      struct pipe_resource templ;
      memset(&templ, 0, sizeof(templ));
      templ.target = PIPE_BUFFER;
      templ.format = PIPE_FORMAT_R8_UNORM;
      templ.bind = allocator->bind;
      templ.usage = allocator->usage;
      templ.flags = allocator->flags;
      templ.width0 = allocator->size;
      templ.height0 = 1;
      templ.depth0 = 1;
      templ.array_size = 1;

      struct pipe_screen *screen = allocator->pipe->screen;
      allocator->buffer = screen->resource_create(screen, &templ);
      if (!allocator->buffer)
         goto fail;

      /* Clear the memory if needed. */
      if (allocator->zero_buffer_memory) {
         struct pipe_context *pipe = allocator->pipe;

         if (pipe->clear_buffer) {
            unsigned clear_value = 0;

            pipe->clear_buffer(pipe, allocator->buffer, 0, allocator->size,
                               &clear_value, 4);
         } else {
            struct pipe_transfer *transfer = NULL;
            void *ptr = pipe_buffer_map(pipe, allocator->buffer,
                                        PIPE_MAP_WRITE, &transfer);
            memset(ptr, 0, allocator->size);
            pipe_buffer_unmap(pipe, transfer);
         }
      }
   }

   assert(allocator->offset % alignment == 0);
   assert(allocator->offset < allocator->buffer->width0);
   assert(allocator->offset + size <= allocator->buffer->width0);

   /* Return the buffer. */
   *out_offset = allocator->offset;
   pipe_resource_reference(outbuf, allocator->buffer);

   allocator->offset += size;
   return;

fail:
   pipe_resource_reference(outbuf, NULL);
}
