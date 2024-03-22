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
 * Buffer management.
 * 
 * A buffer manager does only one basic thing: it creates buffers. Actually,
 * "buffer factory" would probably a more accurate description.
 * 
 * You can chain buffer managers so that you can have a finer grained memory
 * management.
 * 
 * For example, for a simple batch buffer manager you would chain:
 * - the native buffer manager, which provides DMA memory from the graphics
 * memory space;
 * - the fenced buffer manager, which will delay buffer destruction until the 
 * the moment the card finishing processing it. 
 * 
 * \author Jose Fonseca <jfonseca@vmware.com>
 */

#ifndef PB_BUFMGR_H_
#define PB_BUFMGR_H_


#include "pb_buffer.h"


#ifdef __cplusplus
extern "C" {
#endif


struct pb_desc;


/** 
 * Abstract base class for all buffer managers.
 */
struct pb_manager
{
   void
   (*destroy)( struct pb_manager *mgr );

   struct pb_buffer *
   (*create_buffer)( struct pb_manager *mgr, 
	             pb_size size,
	             const struct pb_desc *desc);

   /**
    * Flush all temporary-held buffers.
    * 
    * Used mostly to aid debugging memory issues or to clean up resources when 
    * the drivers are long lived.
    */
   void
   (*flush)( struct pb_manager *mgr );

   bool
   (*is_buffer_busy)( struct pb_manager *mgr,
                      struct pb_buffer *buf );
};

/** 
 * Static sub-allocator based the old memory manager.
 * 
 * It managers buffers of different sizes. It does so by allocating a buffer
 * with the size of the heap, and then using the old mm memory manager to manage
 * that heap. 
 */
struct pb_manager *
mm_bufmgr_create(struct pb_manager *provider, 
                 pb_size size, pb_size align2);

/**
 * Same as mm_bufmgr_create.
 * 
 * Buffer will be release when the manager is destroyed.
 */
struct pb_manager *
mm_bufmgr_create_from_buffer(struct pb_buffer *buffer, 
                             pb_size size, pb_size align2);


/**
 * Slab sub-allocator.
 */
struct pb_manager *
pb_slab_manager_create(struct pb_manager *provider,
                       pb_size bufSize,
                       pb_size slabSize,
                       const struct pb_desc *desc);

/**
 * Allow a range of buffer size, by aggregating multiple slabs sub-allocators 
 * with different bucket sizes.
 */
struct pb_manager *
pb_slab_range_manager_create(struct pb_manager *provider,
                             pb_size minBufSize,
                             pb_size maxBufSize,
                             pb_size slabSize,
                             const struct pb_desc *desc);


/** 
 * Time-based buffer cache.
 *
 * This manager keeps a cache of destroyed buffers during a time interval. 
 */
struct pb_manager *
pb_cache_manager_create(struct pb_manager *provider, 
                        unsigned usecs,
                        float size_factor,
                        unsigned bypass_usage,
                        uint64_t maximum_cache_size);

/**
 * Remove a buffer from the cache, but keep it alive.
 */
void
pb_cache_manager_remove_buffer(struct pb_buffer *buf);

struct pb_fence_ops;

/** 
 * Fenced buffer manager.
 *
 * This manager is just meant for convenience. It wraps the buffers returned
 * by another manager in fenced buffers, so that  
 * 
 * NOTE: the buffer manager that provides the buffers will be destroyed
 * at the same time.
 */
struct pb_manager *
fenced_bufmgr_create(struct pb_manager *provider,
                     struct pb_fence_ops *ops,
                     pb_size max_buffer_size,
                     pb_size max_cpu_total_size);

/** 
 * Debug buffer manager to detect buffer under- and overflows.
 *
 * Under/overflow sizes should be a multiple of the largest alignment
 */
struct pb_manager *
pb_debug_manager_create(struct pb_manager *provider,
                        pb_size underflow_size, pb_size overflow_size); 


#ifdef __cplusplus
}
#endif

#endif /*PB_BUFMGR_H_*/
