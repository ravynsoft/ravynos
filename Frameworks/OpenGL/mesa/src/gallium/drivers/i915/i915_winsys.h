/**************************************************************************
 *
 * Copyright © 2009 Jakob Bornecrantz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef I915_WINSYS_H
#define I915_WINSYS_H

#include "util/compiler.h"

struct i915_winsys;
struct i915_winsys_buffer;
struct i915_winsys_batchbuffer;
struct pipe_resource;
struct pipe_fence_handle;
struct winsys_handle;

enum i915_winsys_buffer_usage {
   /* use on textures */
   I915_USAGE_RENDER = 0x01,
   I915_USAGE_SAMPLER = 0x02,
   I915_USAGE_2D_TARGET = 0x04,
   I915_USAGE_2D_SOURCE = 0x08,
   /* use on vertex */
   I915_USAGE_VERTEX = 0x10
};

enum i915_winsys_buffer_type {
   I915_NEW_TEXTURE,
   I915_NEW_SCANOUT, /**< a texture used for scanning out from */
   I915_NEW_VERTEX
};

/* These need to be in sync with the definitions of libdrm-intel! */
enum i915_winsys_buffer_tile { I915_TILE_NONE, I915_TILE_X, I915_TILE_Y };

enum i915_winsys_flush_flags {
   I915_FLUSH_ASYNC = 0,
   I915_FLUSH_END_OF_FRAME = 1
};

struct i915_winsys_batchbuffer {

   struct i915_winsys *iws;

   /**
    * Values exported to speed up the writing the batchbuffer,
    * instead of having to go trough a accesor function for
    * each dword written.
    */
   /*{@*/
   uint8_t *map;
   uint8_t *ptr;
   size_t size;

   size_t relocs;
   /*@}*/
};

struct i915_winsys {

   unsigned pci_id; /**< PCI ID for the device */

   /**
    * Batchbuffer functions.
    */
   /*@{*/
   /**
    * Create a new batchbuffer.
    */
   struct i915_winsys_batchbuffer *(*batchbuffer_create)(
      struct i915_winsys *iws);

   /**
    * Validate buffers for usage in this batchbuffer.
    * Does space-checking and asorted other book-keeping.
    *
    * @batch
    * @buffers array to buffers to validate
    * @num_of_buffers size of the passed array
    */
   bool (*validate_buffers)(struct i915_winsys_batchbuffer *batch,
                            struct i915_winsys_buffer **buffers,
                            int num_of_buffers);

   /**
    * Emit a relocation to a buffer.
    * Target position in batchbuffer is the same as ptr.
    *
    * @batch
    * @reloc buffer address to be inserted into target.
    * @usage how is the hardware going to use the buffer.
    * @offset add this to the reloc buffers address
    * @target buffer where to write the address, null for batchbuffer.
    * @fenced relocation needs a fence.
    */
   int (*batchbuffer_reloc)(struct i915_winsys_batchbuffer *batch,
                            struct i915_winsys_buffer *reloc,
                            enum i915_winsys_buffer_usage usage,
                            unsigned offset, bool fenced);

   /**
    * Flush a bufferbatch.
    */
   void (*batchbuffer_flush)(struct i915_winsys_batchbuffer *batch,
                             struct pipe_fence_handle **fence,
                             enum i915_winsys_flush_flags flags);

   /**
    * Destroy a batchbuffer.
    */
   void (*batchbuffer_destroy)(struct i915_winsys_batchbuffer *batch);
   /*@}*/

   /**
    * Buffer functions.
    */
   /*@{*/
   /**
    * Create a buffer.
    */
   struct i915_winsys_buffer *(*buffer_create)(
      struct i915_winsys *iws, unsigned size,
      enum i915_winsys_buffer_type type);

   /**
    * Create a tiled buffer.
    *
    * *stride, height are in bytes. The winsys tries to allocate the buffer with
    * the tiling mode provide in *tiling. If tiling is no possible, *tiling will
    * be set to I915_TILE_NONE. The calculated stride (incorporateing hw/kernel
    * requirements) is always returned in *stride.
    */
   struct i915_winsys_buffer *(*buffer_create_tiled)(
      struct i915_winsys *iws, unsigned *stride, unsigned height,
      enum i915_winsys_buffer_tile *tiling, enum i915_winsys_buffer_type type);

   /**
    * Creates a buffer from a handle.
    * Used to implement pipe_screen::resource_from_handle.
    * Also provides the stride information needed for the
    * texture via the stride argument.
    */
   struct i915_winsys_buffer *(*buffer_from_handle)(
      struct i915_winsys *iws, struct winsys_handle *whandle, unsigned height,
      enum i915_winsys_buffer_tile *tiling, unsigned *stride);

   /**
    * Used to implement pipe_screen::resource_get_handle.
    * The winsys might need the stride information.
    */
   bool (*buffer_get_handle)(struct i915_winsys *iws,
                             struct i915_winsys_buffer *buffer,
                             struct winsys_handle *whandle, unsigned stride);

   /**
    * Map a buffer.
    */
   void *(*buffer_map)(struct i915_winsys *iws,
                       struct i915_winsys_buffer *buffer, bool write);

   /**
    * Unmap a buffer.
    */
   void (*buffer_unmap)(struct i915_winsys *iws,
                        struct i915_winsys_buffer *buffer);

   /**
    * Write to a buffer.
    *
    * Arguments follows pipe_buffer_write.
    */
   int (*buffer_write)(struct i915_winsys *iws, struct i915_winsys_buffer *dst,
                       size_t offset, size_t size, const void *data);

   void (*buffer_destroy)(struct i915_winsys *iws,
                          struct i915_winsys_buffer *buffer);

   /**
    * Check if a buffer is busy.
    */
   bool (*buffer_is_busy)(struct i915_winsys *iws,
                          struct i915_winsys_buffer *buffer);
   /*@}*/

   /**
    * Fence functions.
    */
   /*@{*/
   /**
    * Reference fence and set ptr to fence.
    */
   void (*fence_reference)(struct i915_winsys *iws,
                           struct pipe_fence_handle **ptr,
                           struct pipe_fence_handle *fence);

   /**
    * Check if a fence has finished.
    */
   int (*fence_signalled)(struct i915_winsys *iws,
                          struct pipe_fence_handle *fence);

   /**
    * Wait on a fence to finish.
    */
   int (*fence_finish)(struct i915_winsys *iws,
                       struct pipe_fence_handle *fence);
   /*@}*/

   /**
    * Retrieve the aperture size (in MiB) of the device.
    */
   int (*aperture_size)(struct i915_winsys *iws);

   /**
    * Destroy the winsys.
    */
   void (*destroy)(struct i915_winsys *iws);

   /**
    * Get FD if the winsys provides one
    */
   int (*get_fd)(struct i915_winsys *iws);
};

#endif
