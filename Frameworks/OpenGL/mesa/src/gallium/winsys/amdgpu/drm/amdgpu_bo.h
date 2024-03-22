/*
 * Copyright © 2008 Jérôme Glisse
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2015 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AMDGPU_BO_H
#define AMDGPU_BO_H

#include "amdgpu_winsys.h"

#include "pipebuffer/pb_slab.h"

struct amdgpu_sparse_backing_chunk;

/*
 * Sub-allocation information for a real buffer used as backing memory of a
 * sparse buffer.
 */
struct amdgpu_sparse_backing {
   struct list_head list;

   struct amdgpu_bo_real *bo;

   /* Sorted list of free chunks. */
   struct amdgpu_sparse_backing_chunk *chunks;
   uint32_t max_chunks;
   uint32_t num_chunks;
};

struct amdgpu_sparse_commitment {
   struct amdgpu_sparse_backing *backing;
   uint32_t page;
};

enum amdgpu_bo_type {
   AMDGPU_BO_SLAB_ENTRY,
   AMDGPU_BO_SPARSE,
   AMDGPU_BO_REAL,               /* only REAL enums can be present after this */
   AMDGPU_BO_REAL_REUSABLE,      /* only REAL_REUSABLE enums can be present after this */
   AMDGPU_BO_REAL_REUSABLE_SLAB,
};

/* Anything above REAL will use the BO list for REAL. */
#define NUM_BO_LIST_TYPES (AMDGPU_BO_REAL + 1)

/* Base class of the buffer object that other structures inherit. */
struct amdgpu_winsys_bo {
   struct pb_buffer_lean base;
   enum amdgpu_bo_type type:8;
   struct amdgpu_seq_no_fences fences;

   /* This is set when a buffer is returned by buffer_create(), not when the memory is allocated
    * as part of slab BO.
    */
   uint32_t unique_id;

   /* how many command streams, which are being emitted in a separate
    * thread, is this bo referenced in? */
   volatile int num_active_ioctls;
};

/* Real GPU memory allocation managed by the amdgpu kernel driver.
 *
 * There are also types of buffers that are not "real" kernel allocations, such as slab entry
 * BOs, which are suballocated from real BOs, and sparse BOs, which initially only allocate
 * the virtual address range, not memory.
 */
struct amdgpu_bo_real {
   struct amdgpu_winsys_bo b;

   amdgpu_bo_handle bo_handle;
   amdgpu_va_handle va_handle;
   void *cpu_ptr; /* for user_ptr and permanent maps */
   int map_count;
   uint32_t kms_handle;
#if DEBUG
   struct list_head global_list_item;
#endif
   simple_mtx_t map_lock;

   bool is_user_ptr;

   /* Whether buffer_get_handle or buffer_from_handle has been called,
    * it can only transition from false to true. Protected by lock.
    */
   bool is_shared;
};

/* Same as amdgpu_bo_real except this BO isn't destroyed when its reference count drops to 0.
 * Instead it's cached in pb_cache for later reuse.
 */
struct amdgpu_bo_real_reusable {
   struct amdgpu_bo_real b;
   struct pb_cache_entry cache_entry;
};

/* Sparse BO. This only allocates the virtual address range for the BO. The physical storage is
 * allocated on demand by the user using radeon_winsys::buffer_commit with 64KB granularity.
 */
struct amdgpu_bo_sparse {
   struct amdgpu_winsys_bo b;
   amdgpu_va_handle va_handle;

   uint32_t num_va_pages;
   uint32_t num_backing_pages;
   simple_mtx_t commit_lock;

   struct list_head backing;

   /* Commitment information for each page of the virtual memory area. */
   struct amdgpu_sparse_commitment *commitments;
};

/* Suballocated buffer using the slab allocator. This BO is only 1 piece of a larger buffer
 * called slab, which is a buffer that's divided into smaller equal-sized buffers.
 */
struct amdgpu_bo_slab_entry {
   struct amdgpu_winsys_bo b;
   struct pb_slab_entry entry;
};

/* The slab buffer, which is the big backing buffer out of which smaller BOs are suballocated and
 * represented by amdgpu_bo_slab_entry. It's always a real and reusable buffer.
 */
struct amdgpu_bo_real_reusable_slab {
   struct amdgpu_bo_real_reusable b;
   struct pb_slab slab;
   struct amdgpu_bo_slab_entry *entries;
};

static inline bool is_real_bo(struct amdgpu_winsys_bo *bo)
{
   return bo->type >= AMDGPU_BO_REAL;
}

static struct amdgpu_bo_real *get_real_bo(struct amdgpu_winsys_bo *bo)
{
   assert(is_real_bo(bo));
   return (struct amdgpu_bo_real*)bo;
}

static struct amdgpu_bo_real_reusable *get_real_bo_reusable(struct amdgpu_winsys_bo *bo)
{
   assert(bo->type >= AMDGPU_BO_REAL_REUSABLE);
   return (struct amdgpu_bo_real_reusable*)bo;
}

static struct amdgpu_bo_sparse *get_sparse_bo(struct amdgpu_winsys_bo *bo)
{
   assert(bo->type == AMDGPU_BO_SPARSE && bo->base.usage & RADEON_FLAG_SPARSE);
   return (struct amdgpu_bo_sparse*)bo;
}

static struct amdgpu_bo_slab_entry *get_slab_entry_bo(struct amdgpu_winsys_bo *bo)
{
   assert(bo->type == AMDGPU_BO_SLAB_ENTRY);
   return (struct amdgpu_bo_slab_entry*)bo;
}

static inline struct amdgpu_bo_real_reusable_slab *get_bo_from_slab(struct pb_slab *slab)
{
   return container_of(slab, struct amdgpu_bo_real_reusable_slab, slab);
}

static struct amdgpu_bo_real *get_slab_entry_real_bo(struct amdgpu_winsys_bo *bo)
{
   assert(bo->type == AMDGPU_BO_SLAB_ENTRY);
   return &get_bo_from_slab(((struct amdgpu_bo_slab_entry*)bo)->entry.slab)->b.b;
}

/* Given a sequence number "fences->seq_no[queue_index]", return a pointer to a non-NULL fence
 * pointer in the queue ring corresponding to that sequence number if the fence is non-NULL.
 * If the fence is not present in the ring (= is idle), return NULL. If it returns a non-NULL
 * pointer and the caller finds the fence to be idle, it's recommended to use the returned pointer
 * to set the fence to NULL in the ring, which is why we return a pointer to a pointer.
 */
static inline struct pipe_fence_handle **
get_fence_from_ring(struct amdgpu_winsys *ws, struct amdgpu_seq_no_fences *fences,
                    unsigned queue_index)
{
   /* The caller should check if the BO has a fence. */
   assert(queue_index < AMDGPU_MAX_QUEUES);
   assert(fences->valid_fence_mask & BITFIELD_BIT(queue_index));

   uint_seq_no buffer_seq_no = fences->seq_no[queue_index];
   uint_seq_no latest_seq_no = ws->queues[queue_index].latest_seq_no;
   bool fence_present = latest_seq_no - buffer_seq_no < AMDGPU_FENCE_RING_SIZE;

   if (fence_present) {
      struct pipe_fence_handle **fence =
         &ws->queues[queue_index].fences[buffer_seq_no % AMDGPU_FENCE_RING_SIZE];

      if (*fence)
         return fence;
   }

   /* If the sequence number references a fence that is not present, it's guaranteed to be idle
    * because the winsys always waits for the oldest fence when it removes it from the ring.
    */
   fences->valid_fence_mask &= ~BITFIELD_BIT(queue_index);
   return NULL;
}

static inline uint_seq_no pick_latest_seq_no(struct amdgpu_winsys *ws, unsigned queue_index,
                                             uint_seq_no n1, uint_seq_no n2)
{
   uint_seq_no latest = ws->queues[queue_index].latest_seq_no;

   /* Since sequence numbers can wrap around, we need to pick the later number that's logically
    * before "latest". The trick is to subtract "latest + 1" to underflow integer such
    * that "latest" becomes UINT*_MAX, and then just return the maximum.
    */
   uint_seq_no s1 = n1 - latest - 1;
   uint_seq_no s2 = n2 - latest - 1;

   return s1 >= s2 ? n1 : n2;
}

static inline void add_seq_no_to_list(struct amdgpu_winsys *ws, struct amdgpu_seq_no_fences *fences,
                                      unsigned queue_index, uint_seq_no seq_no)
{
   if (fences->valid_fence_mask & BITFIELD_BIT(queue_index)) {
      fences->seq_no[queue_index] = pick_latest_seq_no(ws, queue_index, seq_no,
                                                       fences->seq_no[queue_index]);
   } else {
      fences->seq_no[queue_index] = seq_no;
      fences->valid_fence_mask |= BITFIELD_BIT(queue_index);
   }
}

bool amdgpu_bo_can_reclaim(struct amdgpu_winsys *ws, struct pb_buffer_lean *_buf);
struct pb_buffer_lean *amdgpu_bo_create(struct amdgpu_winsys *ws,
                                   uint64_t size,
                                   unsigned alignment,
                                   enum radeon_bo_domain domain,
                                   enum radeon_bo_flag flags);
void amdgpu_bo_destroy(struct amdgpu_winsys *ws, struct pb_buffer_lean *_buf);
void *amdgpu_bo_map(struct radeon_winsys *rws,
                    struct pb_buffer_lean *buf,
                    struct radeon_cmdbuf *rcs,
                    enum pipe_map_flags usage);
void amdgpu_bo_unmap(struct radeon_winsys *rws, struct pb_buffer_lean *buf);
void amdgpu_bo_init_functions(struct amdgpu_screen_winsys *ws);

bool amdgpu_bo_can_reclaim_slab(void *priv, struct pb_slab_entry *entry);
struct pb_slab *amdgpu_bo_slab_alloc(void *priv, unsigned heap, unsigned entry_size,
                                     unsigned group_index);
void amdgpu_bo_slab_free(struct amdgpu_winsys *ws, struct pb_slab *slab);
uint64_t amdgpu_bo_get_va(struct pb_buffer_lean *buf);

static inline
struct amdgpu_winsys_bo *amdgpu_winsys_bo(struct pb_buffer_lean *bo)
{
   return (struct amdgpu_winsys_bo *)bo;
}

static inline
void amdgpu_winsys_bo_reference(struct amdgpu_winsys *ws,
                                struct amdgpu_winsys_bo **dst,
                                struct amdgpu_winsys_bo *src)
{
   radeon_bo_reference(&ws->dummy_ws.base,
                       (struct pb_buffer_lean**)dst, (struct pb_buffer_lean*)src);
}

#endif
