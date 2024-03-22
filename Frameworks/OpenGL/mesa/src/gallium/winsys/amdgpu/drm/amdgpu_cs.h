/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2015 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AMDGPU_CS_H
#define AMDGPU_CS_H

#include "amdgpu_bo.h"
#include "util/u_memory.h"
#include "drm-uapi/amdgpu_drm.h"

/* Smaller submits means the GPU gets busy sooner and there is less
 * waiting for buffers and fences. Proof:
 *   http://www.phoronix.com/scan.php?page=article&item=mesa-111-si&num=1
 */
#define IB_MAX_SUBMIT_DWORDS (20 * 1024)

struct amdgpu_ctx {
   struct pipe_reference reference;
   struct amdgpu_winsys *ws;
   amdgpu_context_handle ctx;
   amdgpu_bo_handle user_fence_bo;
   uint64_t *user_fence_cpu_address_base;

   /* If true, report lost contexts and skip command submission.
    * If false, terminate the process.
    */
   bool allow_context_lost;

   /* Lost context status due to ioctl and allocation failures. */
   enum pipe_reset_status sw_status;
};

struct amdgpu_cs_buffer {
   struct amdgpu_winsys_bo *bo;
   unsigned slab_real_idx; /* index of underlying real BO, used by slab buffers only */
   unsigned usage;
};

enum ib_type {
   IB_PREAMBLE,
   IB_MAIN,
   IB_NUM,
};

struct amdgpu_ib {
   /* A buffer out of which new IBs are allocated. */
   struct pb_buffer_lean   *big_buffer;
   uint8_t                 *big_buffer_cpu_ptr;
   uint64_t                gpu_address;
   unsigned                used_ib_space;

   /* The maximum seen size from cs_check_space. If the driver does
    * cs_check_space and flush, the newly allocated IB should have at least
    * this size.
    */
   unsigned                max_check_space_size;

   unsigned                max_ib_size_dw;
   /* ptr_ib_size initially points to cs->csc->chunk_ib->ib_bytes.
    * If in amdgpu_cs_check_space() ib chaining is required, then ptr_ib_size will point
    * to indirect buffer packet size field.
    */
   uint32_t                *ptr_ib_size;
   bool                    is_chained_ib;
};

struct amdgpu_fence_list {
   struct pipe_fence_handle    **list;
   unsigned                    num;
   unsigned                    max;
};

struct amdgpu_buffer_list {
   unsigned                    max_buffers;
   unsigned                    num_buffers;
   struct amdgpu_cs_buffer     *buffers;
};

struct amdgpu_cs_context {
   struct drm_amdgpu_cs_chunk_ib chunk_ib[IB_NUM];
   uint32_t                    *ib_main_addr; /* the beginning of IB before chaining */

   struct amdgpu_winsys *ws;

   /* Buffers. */
   struct amdgpu_buffer_list   buffer_lists[NUM_BO_LIST_TYPES];
   int16_t                     *buffer_indices_hashlist;

   struct amdgpu_winsys_bo     *last_added_bo;
   unsigned                    last_added_bo_usage;

   struct amdgpu_seq_no_fences seq_no_dependencies;
   struct amdgpu_fence_list    fence_dependencies;
   struct amdgpu_fence_list    syncobj_dependencies;
   struct amdgpu_fence_list    syncobj_to_signal;

   struct pipe_fence_handle    *fence;

   /* the error returned from cs_flush for non-async submissions */
   int                         error_code;

   /* TMZ: will this command be submitted using the TMZ flag */
   bool secure;
};

/* This high limit is needed for viewperf2020/catia. */
#define BUFFER_HASHLIST_SIZE 32768

struct amdgpu_cs {
   struct amdgpu_ib main_ib; /* must be first because this is inherited */
   struct amdgpu_winsys *ws;
   struct amdgpu_ctx *ctx;

   /*
    * Ensure a 64-bit alignment for drm_amdgpu_cs_chunk_fence.
    */
   struct drm_amdgpu_cs_chunk_fence fence_chunk;
   enum amd_ip_type ip_type;
   unsigned queue_index;

   /* We flip between these two CS. While one is being consumed
    * by the kernel in another thread, the other one is being filled
    * by the pipe driver. */
   struct amdgpu_cs_context csc1;
   struct amdgpu_cs_context csc2;
   /* The currently-used CS. */
   struct amdgpu_cs_context *csc;
   /* The CS being currently-owned by the other thread. */
   struct amdgpu_cs_context *cst;
   /* buffer_indices_hashlist[hash(bo)] returns -1 if the bo
    * isn't part of any buffer lists or the index where the bo could be found.
    * Since 1) hash collisions of 2 different bo can happen and 2) we use a
    * single hashlist for the 3 buffer list, this is only a hint.
    * amdgpu_lookup_buffer uses this hint to speed up buffers look up.
    */
   int16_t buffer_indices_hashlist[BUFFER_HASHLIST_SIZE];

   /* Flush CS. */
   void (*flush_cs)(void *ctx, unsigned flags, struct pipe_fence_handle **fence);
   void *flush_data;
   bool noop;
   bool has_chaining;

   struct util_queue_fence flush_completed;
   struct pipe_fence_handle *next_fence;
   struct pb_buffer_lean *preamble_ib_bo;

   struct drm_amdgpu_cs_chunk_cp_gfx_shadow mcbp_fw_shadow_chunk;
};

struct amdgpu_fence {
   struct pipe_reference reference;
   /* If ctx == NULL, this fence is syncobj-based. */
   uint32_t syncobj;

   struct amdgpu_winsys *ws;
   struct amdgpu_ctx *ctx;  /* submission context */
   struct amdgpu_cs_fence fence;
   uint64_t *user_fence_cpu_address;

   /* If the fence has been submitted. This is unsignalled for deferred fences
    * (cs->next_fence) and while an IB is still being submitted in the submit
    * thread. */
   struct util_queue_fence submitted;

   volatile int signalled;              /* bool (int for atomicity) */
   bool imported;
   uint8_t queue_index;       /* for non-imported fences */
   uint_seq_no queue_seq_no;  /* winsys-generated sequence number */
};

static inline bool amdgpu_fence_is_syncobj(struct amdgpu_fence *fence)
{
   return fence->ctx == NULL;
}

static inline void amdgpu_ctx_reference(struct amdgpu_ctx **dst, struct amdgpu_ctx *src)
{
   struct amdgpu_ctx *old_dst = *dst;

   if (pipe_reference(old_dst ? &old_dst->reference : NULL,
                      src ? &src->reference : NULL)) {
      amdgpu_cs_ctx_free(old_dst->ctx);
      amdgpu_bo_free(old_dst->user_fence_bo);
      FREE(old_dst);
   }
   *dst = src;
}

static inline void amdgpu_fence_reference(struct pipe_fence_handle **dst,
                                          struct pipe_fence_handle *src)
{
   struct amdgpu_fence **adst = (struct amdgpu_fence **)dst;
   struct amdgpu_fence *asrc = (struct amdgpu_fence *)src;

   if (pipe_reference(&(*adst)->reference, &asrc->reference)) {
      struct amdgpu_fence *fence = *adst;

      if (amdgpu_fence_is_syncobj(fence))
         amdgpu_cs_destroy_syncobj(fence->ws->dev, fence->syncobj);
      else
         amdgpu_ctx_reference(&fence->ctx, NULL);

      util_queue_fence_destroy(&fence->submitted);
      FREE(fence);
   }
   *adst = asrc;
}

struct amdgpu_cs_buffer *
amdgpu_lookup_buffer_any_type(struct amdgpu_cs_context *cs, struct amdgpu_winsys_bo *bo);

static inline struct amdgpu_cs *
amdgpu_cs(struct radeon_cmdbuf *rcs)
{
   struct amdgpu_cs *cs = (struct amdgpu_cs*)rcs->priv;
   assert(cs);
   return cs;
}

#define get_container(member_ptr, container_type, container_member) \
   (container_type *)((char *)(member_ptr) - offsetof(container_type, container_member))

static inline bool
amdgpu_bo_is_referenced_by_cs(struct amdgpu_cs *cs,
                              struct amdgpu_winsys_bo *bo)
{
   return amdgpu_lookup_buffer_any_type(cs->csc, bo) != NULL;
}

static inline unsigned get_buf_list_idx(struct amdgpu_winsys_bo *bo)
{
   /* AMDGPU_BO_REAL_REUSABLE* maps to AMDGPU_BO_REAL. */
   static_assert(ARRAY_SIZE(((struct amdgpu_cs_context*)NULL)->buffer_lists) == NUM_BO_LIST_TYPES, "");
   return MIN2(bo->type, AMDGPU_BO_REAL);
}

static inline bool
amdgpu_bo_is_referenced_by_cs_with_usage(struct amdgpu_cs *cs,
                                         struct amdgpu_winsys_bo *bo,
                                         unsigned usage)
{
   struct amdgpu_cs_buffer *buffer = amdgpu_lookup_buffer_any_type(cs->csc, bo);

   return buffer && (buffer->usage & usage) != 0;
}

bool amdgpu_fence_wait(struct pipe_fence_handle *fence, uint64_t timeout,
                       bool absolute);
void amdgpu_cs_sync_flush(struct radeon_cmdbuf *rcs);
void amdgpu_cs_init_functions(struct amdgpu_screen_winsys *ws);

#endif
