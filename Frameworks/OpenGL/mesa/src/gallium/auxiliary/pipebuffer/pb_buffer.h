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

/**
 * \file
 * Generic code for buffers.
 *
 * Behind a pipe buffle handle there can be DMA buffers, client (or user)
 * buffers, regular malloced buffers, etc. This file provides an abstract base
 * buffer handle that allows the driver to cope with all those kinds of buffers
 * in a more flexible way.
 *
 * There is no obligation of a winsys driver to use this library. And a pipe
 * driver should be completly agnostic about it.
 *
 * \author Jose Fonseca <jfonseca@vmware.com>
 */

#ifndef PB_BUFFER_H_
#define PB_BUFFER_H_


#include "util/compiler.h"
#include "util/u_debug.h"
#include "util/u_inlines.h"
#include "pipe/p_defines.h"


#ifdef __cplusplus
extern "C" {
#endif


struct pb_vtbl;
struct pb_validate;
struct pipe_fence_handle;

enum pb_usage_flags {
   PB_USAGE_CPU_READ = (1 << 0),
   PB_USAGE_CPU_WRITE = (1 << 1),
   PB_USAGE_GPU_READ = (1 << 2),
   PB_USAGE_GPU_WRITE = (1 << 3),
   PB_USAGE_DONTBLOCK = (1 << 4),
   PB_USAGE_UNSYNCHRONIZED = (1 << 5),
   /* Persistent mappings may remain across a flush. Note that contrary
    * to OpenGL persistent maps, there is no requirement at the pipebuffer
    * api level to explicitly enforce coherency by barriers or range flushes.
    */
   PB_USAGE_PERSISTENT = (1 << 8)
};

/* For error checking elsewhere */
#define PB_USAGE_ALL (PB_USAGE_CPU_READ | \
                      PB_USAGE_CPU_WRITE | \
                      PB_USAGE_GPU_READ | \
                      PB_USAGE_GPU_WRITE | \
                      PB_USAGE_DONTBLOCK | \
                      PB_USAGE_UNSYNCHRONIZED | \
                      PB_USAGE_PERSISTENT)

#define PB_USAGE_CPU_READ_WRITE  (PB_USAGE_CPU_READ | PB_USAGE_CPU_WRITE)
#define PB_USAGE_GPU_READ_WRITE  (PB_USAGE_GPU_READ | PB_USAGE_GPU_WRITE)
#define PB_USAGE_WRITE           (PB_USAGE_CPU_WRITE | PB_USAGE_GPU_WRITE)


/**
 * Buffer description.
 *
 * Used when allocating the buffer.
 */
struct pb_desc
{
   unsigned alignment;
   enum pb_usage_flags usage;
};


/**
 * 64-bit type for GPU buffer sizes and offsets.
 */
typedef uint64_t pb_size;


/**
 * Base class for all pb_* buffers without the vtbl pointer.
 */
struct pb_buffer_lean
{
   struct pipe_reference  reference;

   /* For internal driver use. It's here so as not to waste space due to
    * type alignment. (pahole)
    */
   uint8_t                placement;

   /* Alignments are powers of two, so store only the bit position.
    *    alignment_log2 = util_logbase2(alignment);
    *    alignment = 1 << alignment_log2;
    */
   uint8_t                alignment_log2;

   /**
    * Used with pb_usage_flags or driver-specific flags, depending on drivers.
    */
   uint16_t               usage;

   pb_size                size;
};


/**
 * Base class for all pb_* buffers with the vtbl pointer.
 */
struct pb_buffer
{
   struct pb_buffer_lean base;

   /**
    * Pointer to the virtual function table.
    *
    * Avoid accessing this table directly. Use the inline functions below
    * instead to avoid mistakes.
    */
   const struct pb_vtbl *vtbl;
};


/**
 * Virtual function table for the buffer storage operations.
 *
 * Note that creation is not done through this table.
 */
struct pb_vtbl
{
   void (*destroy)(void *winsys, struct pb_buffer *buf);

   /**
    * Map the entire data store of a buffer object into the client's address.
    * flags is bitmask of PB_USAGE_CPU_READ/WRITE.
    */
   void *(*map)(struct pb_buffer *buf,
                enum pb_usage_flags flags, void *flush_ctx);

   void (*unmap)(struct pb_buffer *buf);

   enum pipe_error (*validate)(struct pb_buffer *buf,
                               struct pb_validate *vl,
                               enum pb_usage_flags flags);

   void (*fence)(struct pb_buffer *buf,
                 struct pipe_fence_handle *fence);

   /**
    * Get the base buffer and the offset.
    *
    * A buffer can be subdivided in smaller buffers. This method should return
    * the underlaying buffer, and the relative offset.
    *
    * Buffers without an underlaying base buffer should return themselves, with
    * a zero offset.
    *
    * Note that this will increase the reference count of the base buffer.
    */
   void (*get_base_buffer)(struct pb_buffer *buf,
                           struct pb_buffer **base_buf,
                           pb_size *offset);
};



/* Accessor functions for pb->vtbl:
 */
static inline void *
pb_map(struct pb_buffer *buf, enum pb_usage_flags flags, void *flush_ctx)
{
   assert(buf);
   if (!buf)
      return NULL;
   assert(pipe_is_referenced(&buf->base.reference));
   return buf->vtbl->map(buf, flags, flush_ctx);
}


static inline void
pb_unmap(struct pb_buffer *buf)
{
   assert(buf);
   if (!buf)
      return;
   assert(pipe_is_referenced(&buf->base.reference));
   buf->vtbl->unmap(buf);
}


static inline void
pb_get_base_buffer(struct pb_buffer *buf,
                   struct pb_buffer **base_buf,
                   pb_size *offset)
{
   assert(buf);
   if (!buf) {
      base_buf = NULL;
      offset = NULL;
      return;
   }
   assert(pipe_is_referenced(&buf->base.reference));
   assert(buf->vtbl->get_base_buffer);
   buf->vtbl->get_base_buffer(buf, base_buf, offset);
   assert(*base_buf);
   assert(*offset < (*base_buf)->base.size);
}


static inline enum pipe_error
pb_validate(struct pb_buffer *buf, struct pb_validate *vl,
            enum pb_usage_flags flags)
{
   assert(buf);
   if (!buf)
      return PIPE_ERROR;
   assert(buf->vtbl->validate);
   return buf->vtbl->validate(buf, vl, flags);
}


static inline void
pb_fence(struct pb_buffer *buf, struct pipe_fence_handle *fence)
{
   assert(buf);
   if (!buf)
      return;
   assert(buf->vtbl->fence);
   buf->vtbl->fence(buf, fence);
}


static inline void
pb_destroy(void *winsys, struct pb_buffer *buf)
{
   assert(buf);
   if (!buf)
      return;

   /* we can't assert(!pipe_is_referenced(&buf->reference)) because the winsys
    * might have means to revive a buf whose refcount reaches 0, such as when
    * destroy and import race against each other
    */
   buf->vtbl->destroy(winsys, buf);
}


static inline void
pb_reference(struct pb_buffer **dst,
             struct pb_buffer *src)
{
   struct pb_buffer *old = *dst;

   if (pipe_reference(&(*dst)->base.reference, &src->base.reference))
      pb_destroy(NULL, old);
   *dst = src;
}

static inline void
pb_reference_with_winsys(void *winsys,
                         struct pb_buffer **dst,
                         struct pb_buffer *src)
{
   struct pb_buffer *old = *dst;

   if (pipe_reference(&(*dst)->base.reference, &src->base.reference))
      pb_destroy(winsys, old);
   *dst = src;
}

/**
 * Utility function to check whether the provided alignment is consistent with
 * the requested or not.
 */
static inline bool
pb_check_alignment(uint32_t requested, uint32_t provided)
{
   if (!requested)
      return true;
   if (requested > provided)
      return false;
   if (provided % requested != 0)
      return false;
   return true;
}


/**
 * Utility function to check whether the provided alignment is consistent with
 * the requested or not.
 */
static inline bool
pb_check_usage(unsigned requested, unsigned provided)
{
   return (requested & provided) == requested ? true : false;
}

#ifdef __cplusplus
}
#endif

#endif /*PB_BUFFER_H_*/
