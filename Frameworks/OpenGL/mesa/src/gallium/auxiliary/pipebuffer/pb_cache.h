/**************************************************************************
 *
 * Copyright 2007-2008 VMware, Inc.
 * Copyright 2015 Advanced Micro Devices, Inc.
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
 * IN NO EVENT SHALL AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef PB_CACHE_H
#define PB_CACHE_H

#include "pb_buffer.h"
#include "util/simple_mtx.h"
#include "util/list.h"
#include "util/u_thread.h"

/**
 * Statically inserted into the driver-specific buffer structure.
 */
struct pb_cache_entry
{
   struct list_head head;
   unsigned start_ms; /**< Cached start time */
   unsigned bucket_index;
};

struct pb_cache
{
   /* The cache is divided into buckets for minimizing cache misses.
    * The driver controls which buffer goes into which bucket.
    */
   struct list_head *buckets;

   simple_mtx_t mutex;
   void *winsys;
   uint64_t cache_size;
   uint64_t max_cache_size;
   unsigned num_heaps;
   unsigned msecs;
   int64_t msecs_base_time;
   unsigned num_buffers;
   unsigned bypass_usage;
   float size_factor;
   unsigned offsetof_pb_cache_entry; /* offsetof(driver_bo, pb_cache_entry) */

   void (*destroy_buffer)(void *winsys, struct pb_buffer_lean *buf);
   bool (*can_reclaim)(void *winsys, struct pb_buffer_lean *buf);
};

void pb_cache_add_buffer(struct pb_cache *mgr, struct pb_cache_entry *entry);
struct pb_buffer_lean *pb_cache_reclaim_buffer(struct pb_cache *mgr, pb_size size,
                                          unsigned alignment, unsigned usage,
                                          unsigned bucket_index);
unsigned pb_cache_release_all_buffers(struct pb_cache *mgr);
void pb_cache_init_entry(struct pb_cache *mgr, struct pb_cache_entry *entry,
                         struct pb_buffer_lean *buf, unsigned bucket_index);
void pb_cache_init(struct pb_cache *mgr, unsigned num_heaps,
                   unsigned usecs, float size_factor,
                   unsigned bypass_usage, uint64_t maximum_cache_size,
                   unsigned offsetof_pb_cache_entry, void *winsys,
                   void (*destroy_buffer)(void *winsys, struct pb_buffer_lean *buf),
                   bool (*can_reclaim)(void *winsys, struct pb_buffer_lean *buf));
void pb_cache_deinit(struct pb_cache *mgr);

#endif
