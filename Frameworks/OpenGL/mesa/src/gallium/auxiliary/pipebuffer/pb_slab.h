/*
 * Copyright 2016 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS, AUTHORS
 * AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/**
 * \file
 *
 * Helper library for carving out smaller allocations (called "(slab) entries")
 * from larger buffers (called "slabs").
 *
 * The library supports maintaining separate heaps (e.g. VRAM vs. GTT). The
 * meaning of each heap is treated as opaque by this library.
 *
 * The library allows delaying the re-use of an entry, i.e. an entry may be
 * freed by calling \ref pb_slab_free even while the corresponding buffer
 * region is still in use by the GPU. A callback function is called to
 * determine when it is safe to allocate the entry again; the user of this
 * library is expected to maintain the required fences or similar.
 */

#ifndef PB_SLAB_H
#define PB_SLAB_H

#include "pb_buffer.h"
#include "util/simple_mtx.h"
#include "util/list.h"
#include "util/u_thread.h"

struct pb_slab;
struct pb_slabs;
struct pb_slab_group;

/* Descriptor of a slab entry.
 *
 * The user of this utility library is expected to embed this in a larger
 * structure that describes a buffer object.
 */
struct pb_slab_entry
{
   struct list_head head;
   struct pb_slab *slab; /* the slab that contains this buffer */
};

/* Descriptor of a slab from which many entries are carved out.
 *
 * The user of this utility library is expected to embed this in a larger
 * structure that describes a buffer object.
 */
struct pb_slab
{
   struct list_head head;

   struct list_head free; /* list of free pb_slab_entry structures */
   unsigned num_free; /* number of entries in free list */
   unsigned num_entries; /* total number of entries */
   unsigned group_index; /* index into pb_slabs::groups */
   unsigned entry_size;
};

/* Callback function that is called when a new slab needs to be allocated
 * for fulfilling allocation requests of the given size from the given heap.
 *
 * The callback must allocate a pb_slab structure and the desired number
 * of entries. All entries that belong to the slab must be added to the free
 * list. Entries' pb_slab_entry structures must be initialized with the given
 * group_index.
 *
 * The callback may call pb_slab functions.
 */
typedef struct pb_slab *(slab_alloc_fn)(void *priv,
                                        unsigned heap,
                                        unsigned entry_size,
                                        unsigned group_index);

/* Callback function that is called when all entries of a slab have been freed.
 *
 * The callback must free the slab and all its entries. It must not call any of
 * the pb_slab functions, or a deadlock (recursive mutex lock) may occur.
 */
typedef void (slab_free_fn)(void *priv, struct pb_slab *);

/* Callback function to determine whether a given entry can already be reused.
 */
typedef bool (slab_can_reclaim_fn)(void *priv, struct pb_slab_entry *);

/* Manager of slab allocations. The user of this utility library should embed
 * this in a structure somewhere and call pb_slab_init/deinit at init/shutdown
 * time.
 */
struct pb_slabs
{
   simple_mtx_t mutex;

   unsigned min_order;
   unsigned num_orders;
   unsigned num_heaps;
   bool allow_three_fourths_allocations;

   /* One group per (heap, order, three_fourth_allocations). */
   struct pb_slab_group *groups;

   /* List of entries waiting to be reclaimed, i.e. they have been passed to
    * pb_slab_free, but may not be safe for re-use yet. The tail points at
    * the most-recently freed entry.
    */
   struct list_head reclaim;

   void *priv;
   slab_can_reclaim_fn *can_reclaim;
   slab_alloc_fn *slab_alloc;
   slab_free_fn *slab_free;
};

struct pb_slab_entry *
pb_slab_alloc_reclaimed(struct pb_slabs *slabs, unsigned size, unsigned heap, bool reclaim_all);

struct pb_slab_entry *
pb_slab_alloc(struct pb_slabs *slabs, unsigned size, unsigned heap);

void
pb_slab_free(struct pb_slabs* slabs, struct pb_slab_entry *entry);

unsigned
pb_slabs_reclaim(struct pb_slabs *slabs);

bool
pb_slabs_init(struct pb_slabs *slabs,
              unsigned min_order, unsigned max_order,
              unsigned num_heaps, bool allow_three_fourth_allocations,
              void *priv,
              slab_can_reclaim_fn *can_reclaim,
              slab_alloc_fn *slab_alloc,
              slab_free_fn *slab_free);

void
pb_slabs_deinit(struct pb_slabs *slabs);

#endif
