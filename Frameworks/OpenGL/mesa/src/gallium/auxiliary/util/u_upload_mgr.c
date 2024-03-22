/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
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

/* Helper utility for uploading user buffers & other data, and
 * coalescing small buffers into larger ones.
 */

#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "pipe/p_context.h"
#include "util/u_memory.h"
#include "util/u_math.h"

#include "u_upload_mgr.h"


struct u_upload_mgr {
   struct pipe_context *pipe;

   unsigned default_size;  /* Minimum size of the upload buffer, in bytes. */
   unsigned bind;          /* Bitmask of PIPE_BIND_* flags. */
   enum pipe_resource_usage usage;
   unsigned flags;
   unsigned map_flags;     /* Bitmask of PIPE_MAP_* flags. */
   bool map_persistent; /* If persistent mappings are supported. */

   struct pipe_resource *buffer;   /* Upload buffer. */
   struct pipe_transfer *transfer; /* Transfer object for the upload buffer. */
   uint8_t *map;    /* Pointer to the mapped upload buffer. */
   unsigned buffer_size; /* Same as buffer->width0. */
   unsigned offset; /* Aligned offset to the upload buffer, pointing
                     * at the first unused byte. */
   int buffer_private_refcount;
};


struct u_upload_mgr *
u_upload_create(struct pipe_context *pipe, unsigned default_size,
                unsigned bind, enum pipe_resource_usage usage, unsigned flags)
{
   struct u_upload_mgr *upload = CALLOC_STRUCT(u_upload_mgr);
   if (!upload)
      return NULL;

   upload->pipe = pipe;
   upload->default_size = default_size;
   upload->bind = bind;
   upload->usage = usage;
   upload->flags = flags;

   upload->map_persistent =
      pipe->screen->get_param(pipe->screen,
                              PIPE_CAP_BUFFER_MAP_PERSISTENT_COHERENT);

   if (upload->map_persistent) {
      upload->map_flags = PIPE_MAP_WRITE |
                          PIPE_MAP_UNSYNCHRONIZED |
                          PIPE_MAP_PERSISTENT |
                          PIPE_MAP_COHERENT;
   }
   else {
      upload->map_flags = PIPE_MAP_WRITE |
                          PIPE_MAP_UNSYNCHRONIZED |
                          PIPE_MAP_FLUSH_EXPLICIT;
   }

   return upload;
}

struct u_upload_mgr *
u_upload_create_default(struct pipe_context *pipe)
{
   return u_upload_create(pipe, 1024 * 1024,
                          PIPE_BIND_VERTEX_BUFFER |
                          PIPE_BIND_INDEX_BUFFER |
                          PIPE_BIND_CONSTANT_BUFFER,
                          PIPE_USAGE_STREAM, 0);
}

struct u_upload_mgr *
u_upload_clone(struct pipe_context *pipe, struct u_upload_mgr *upload)
{
   struct u_upload_mgr *result = u_upload_create(pipe, upload->default_size,
                                                 upload->bind, upload->usage,
                                                 upload->flags);
   if (!upload->map_persistent && result->map_persistent)
      u_upload_disable_persistent(result);

   return result;
}

void
u_upload_disable_persistent(struct u_upload_mgr *upload)
{
   upload->map_persistent = false;
   upload->map_flags &= ~(PIPE_MAP_COHERENT | PIPE_MAP_PERSISTENT);
   upload->map_flags |= PIPE_MAP_FLUSH_EXPLICIT;
}

static void
upload_unmap_internal(struct u_upload_mgr *upload, bool destroying)
{
   if ((!destroying && upload->map_persistent) || !upload->transfer)
      return;

   struct pipe_box *box = &upload->transfer->box;

   if (!upload->map_persistent && (int) upload->offset > box->x) {
      pipe_buffer_flush_mapped_range(upload->pipe, upload->transfer,
                                     box->x, upload->offset - box->x);
   }

   pipe_buffer_unmap(upload->pipe, upload->transfer);
   upload->transfer = NULL;
   upload->map = NULL;
}


void
u_upload_unmap(struct u_upload_mgr *upload)
{
   upload_unmap_internal(upload, false);
}


static void
u_upload_release_buffer(struct u_upload_mgr *upload)
{
   /* Unmap and unreference the upload buffer. */
   upload_unmap_internal(upload, true);
   if (upload->buffer_private_refcount) {
      /* Subtract the remaining private references before unreferencing
       * the buffer. The mega comment below explains it.
       */
      assert(upload->buffer_private_refcount > 0);
      p_atomic_add(&upload->buffer->reference.count,
                   -upload->buffer_private_refcount);
      upload->buffer_private_refcount = 0;
   }
   pipe_resource_reference(&upload->buffer, NULL);
   upload->buffer_size = 0;
}


void
u_upload_destroy(struct u_upload_mgr *upload)
{
   u_upload_release_buffer(upload);
   FREE(upload);
}

/* Return the allocated buffer size or 0 if it failed. */
static unsigned
u_upload_alloc_buffer(struct u_upload_mgr *upload, unsigned min_size)
{
   struct pipe_screen *screen = upload->pipe->screen;
   struct pipe_resource buffer;
   unsigned size;

   /* Release the old buffer, if present:
    */
   u_upload_release_buffer(upload);

   /* Allocate a new one:
    */
   size = align(MAX2(upload->default_size, min_size), 4096);

   memset(&buffer, 0, sizeof buffer);
   buffer.target = PIPE_BUFFER;
   buffer.format = PIPE_FORMAT_R8_UNORM; /* want TYPELESS or similar */
   buffer.bind = upload->bind;
   buffer.usage = upload->usage;
   buffer.flags = upload->flags | PIPE_RESOURCE_FLAG_SINGLE_THREAD_USE;
   buffer.width0 = size;
   buffer.height0 = 1;
   buffer.depth0 = 1;
   buffer.array_size = 1;

   if (upload->map_persistent) {
      buffer.flags |= PIPE_RESOURCE_FLAG_MAP_PERSISTENT |
                      PIPE_RESOURCE_FLAG_MAP_COHERENT;
   }

   upload->buffer = screen->resource_create(screen, &buffer);
   if (upload->buffer == NULL)
      return 0;

   /* Since atomic operations are very very slow when 2 threads are not
    * sharing the same L3 cache (which happens on AMD Zen), eliminate all
    * atomics in u_upload_alloc as follows:
    *
    * u_upload_alloc has to return a buffer reference to the caller.
    * Instead of atomic_inc for every call, it does all possible future
    * increments in advance here. The maximum number of times u_upload_alloc
    * can be called per upload buffer is "size", because the minimum
    * allocation size is 1, thus u_upload_alloc can only return "size" number
    * of suballocations at most, so we will never need more. This is
    * the number that is added to reference.count here.
    *
    * buffer_private_refcount tracks how many buffer references we can return
    * without using atomics. If the buffer is full and there are still
    * references left, they are atomically subtracted from reference.count
    * before the buffer is unreferenced.
    *
    * This technique can increase CPU performance by 10%.
    *
    * The caller of u_upload_alloc_buffer will consume min_size bytes,
    * so init the buffer_private_refcount to 1 + size - min_size, instead
    * of size to avoid overflowing reference.count when size is huge.
    */
   upload->buffer_private_refcount = 1 + (size - min_size);
   assert(upload->buffer_private_refcount < INT32_MAX / 2);
   p_atomic_add(&upload->buffer->reference.count, upload->buffer_private_refcount);

   /* Map the new buffer. */
   upload->map = pipe_buffer_map_range(upload->pipe, upload->buffer,
                                       0, size, upload->map_flags,
                                       &upload->transfer);
   if (upload->map == NULL) {
      u_upload_release_buffer(upload);
      return 0;
   }

   upload->buffer_size = size;
   upload->offset = 0;
   return size;
}

void
u_upload_alloc(struct u_upload_mgr *upload,
               unsigned min_out_offset,
               unsigned size,
               unsigned alignment,
               unsigned *out_offset,
               struct pipe_resource **outbuf,
               void **ptr)
{
   unsigned buffer_size = upload->buffer_size;
   unsigned offset = MAX2(min_out_offset, upload->offset);

   offset = align(offset, alignment);

   /* Make sure we have enough space in the upload buffer
    * for the sub-allocation.
    */
   if (unlikely(offset + size > buffer_size)) {
      /* Allocate a new buffer and set the offset to the smallest one. */
      offset = align(min_out_offset, alignment);
      buffer_size = u_upload_alloc_buffer(upload, offset + size);

      if (unlikely(!buffer_size)) {
         *out_offset = ~0;
         pipe_resource_reference(outbuf, NULL);
         *ptr = NULL;
         return;
      }
   }

   if (unlikely(!upload->map)) {
      upload->map = pipe_buffer_map_range(upload->pipe, upload->buffer,
                                          offset,
                                          buffer_size - offset,
                                          upload->map_flags,
                                          &upload->transfer);
      if (unlikely(!upload->map)) {
         upload->transfer = NULL;
         *out_offset = ~0;
         pipe_resource_reference(outbuf, NULL);
         *ptr = NULL;
         return;
      }

      upload->map -= offset;
   }

   assert(offset < buffer_size);
   assert(offset + size <= buffer_size);
   assert(size);

   /* Emit the return values: */
   *ptr = upload->map + offset;
   *out_offset = offset;

   if (*outbuf != upload->buffer) {
      pipe_resource_reference(outbuf, NULL);
      *outbuf = upload->buffer;
      assert (upload->buffer_private_refcount > 0);
      upload->buffer_private_refcount--;
   }

   upload->offset = offset + size;
}

void
u_upload_data(struct u_upload_mgr *upload,
              unsigned min_out_offset,
              unsigned size,
              unsigned alignment,
              const void *data,
              unsigned *out_offset,
              struct pipe_resource **outbuf)
{
   uint8_t *ptr;

   u_upload_alloc(upload, min_out_offset, size, alignment,
                  out_offset, outbuf,
                  (void**)&ptr);
   if (ptr)
      memcpy(ptr, data, size);
}
