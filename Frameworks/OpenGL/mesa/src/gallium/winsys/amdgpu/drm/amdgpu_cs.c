/*
 * Copyright © 2008 Jérôme Glisse
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2015 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "amdgpu_cs.h"
#include "util/os_time.h"
#include <inttypes.h>
#include <stdio.h>

#include "amd/common/sid.h"

/* FENCES */

static struct pipe_fence_handle *
amdgpu_fence_create(struct amdgpu_cs *cs)
{
   struct amdgpu_fence *fence = CALLOC_STRUCT(amdgpu_fence);
   struct amdgpu_ctx *ctx = cs->ctx;

   fence->reference.count = 1;
   fence->ws = ctx->ws;
   amdgpu_ctx_reference(&fence->ctx, ctx);
   fence->fence.context = ctx->ctx;
   fence->fence.ip_type = cs->ip_type;
   util_queue_fence_init(&fence->submitted);
   util_queue_fence_reset(&fence->submitted);
   fence->queue_index = cs->queue_index;
   return (struct pipe_fence_handle *)fence;
}

static struct pipe_fence_handle *
amdgpu_fence_import_syncobj(struct radeon_winsys *rws, int fd)
{
   struct amdgpu_winsys *ws = amdgpu_winsys(rws);
   struct amdgpu_fence *fence = CALLOC_STRUCT(amdgpu_fence);
   int r;

   if (!fence)
      return NULL;

   pipe_reference_init(&fence->reference, 1);
   fence->ws = ws;

   r = amdgpu_cs_import_syncobj(ws->dev, fd, &fence->syncobj);
   if (r) {
      FREE(fence);
      return NULL;
   }

   util_queue_fence_init(&fence->submitted);
   fence->imported = true;

   assert(amdgpu_fence_is_syncobj(fence));
   return (struct pipe_fence_handle*)fence;
}

static struct pipe_fence_handle *
amdgpu_fence_import_sync_file(struct radeon_winsys *rws, int fd)
{
   struct amdgpu_winsys *ws = amdgpu_winsys(rws);
   struct amdgpu_fence *fence = CALLOC_STRUCT(amdgpu_fence);

   if (!fence)
      return NULL;

   pipe_reference_init(&fence->reference, 1);
   fence->ws = ws;
   /* fence->ctx == NULL means that the fence is syncobj-based. */

   /* Convert sync_file into syncobj. */
   int r = amdgpu_cs_create_syncobj(ws->dev, &fence->syncobj);
   if (r) {
      FREE(fence);
      return NULL;
   }

   r = amdgpu_cs_syncobj_import_sync_file(ws->dev, fence->syncobj, fd);
   if (r) {
      amdgpu_cs_destroy_syncobj(ws->dev, fence->syncobj);
      FREE(fence);
      return NULL;
   }

   util_queue_fence_init(&fence->submitted);
   fence->imported = true;

   return (struct pipe_fence_handle*)fence;
}

static int amdgpu_fence_export_sync_file(struct radeon_winsys *rws,
					 struct pipe_fence_handle *pfence)
{
   struct amdgpu_winsys *ws = amdgpu_winsys(rws);
   struct amdgpu_fence *fence = (struct amdgpu_fence*)pfence;

   if (amdgpu_fence_is_syncobj(fence)) {
      int fd, r;

      /* Convert syncobj into sync_file. */
      r = amdgpu_cs_syncobj_export_sync_file(ws->dev, fence->syncobj, &fd);
      return r ? -1 : fd;
   }

   util_queue_fence_wait(&fence->submitted);

   /* Convert the amdgpu fence into a fence FD. */
   int fd;
   if (amdgpu_cs_fence_to_handle(ws->dev, &fence->fence,
                                 AMDGPU_FENCE_TO_HANDLE_GET_SYNC_FILE_FD,
                                 (uint32_t*)&fd))
      return -1;

   return fd;
}

static int amdgpu_export_signalled_sync_file(struct radeon_winsys *rws)
{
   struct amdgpu_winsys *ws = amdgpu_winsys(rws);
   uint32_t syncobj;
   int fd = -1;

   int r = amdgpu_cs_create_syncobj2(ws->dev, DRM_SYNCOBJ_CREATE_SIGNALED,
                                     &syncobj);
   if (r) {
      return -1;
   }

   r = amdgpu_cs_syncobj_export_sync_file(ws->dev, syncobj, &fd);
   if (r) {
      fd = -1;
   }

   amdgpu_cs_destroy_syncobj(ws->dev, syncobj);
   return fd;
}

static void amdgpu_fence_submitted(struct pipe_fence_handle *fence,
                                   uint64_t seq_no,
                                   uint64_t *user_fence_cpu_address)
{
   struct amdgpu_fence *afence = (struct amdgpu_fence*)fence;

   afence->fence.fence = seq_no;
   afence->user_fence_cpu_address = user_fence_cpu_address;
   util_queue_fence_signal(&afence->submitted);
}

static void amdgpu_fence_signalled(struct pipe_fence_handle *fence)
{
   struct amdgpu_fence *afence = (struct amdgpu_fence*)fence;

   afence->signalled = true;
   util_queue_fence_signal(&afence->submitted);
}

bool amdgpu_fence_wait(struct pipe_fence_handle *fence, uint64_t timeout,
                       bool absolute)
{
   struct amdgpu_fence *afence = (struct amdgpu_fence*)fence;
   uint32_t expired;
   int64_t abs_timeout;
   uint64_t *user_fence_cpu;
   int r;

   if (afence->signalled)
      return true;

   if (absolute)
      abs_timeout = timeout;
   else
      abs_timeout = os_time_get_absolute_timeout(timeout);

   /* Handle syncobjs. */
   if (amdgpu_fence_is_syncobj(afence)) {
      if (abs_timeout == OS_TIMEOUT_INFINITE)
         abs_timeout = INT64_MAX;

      if (amdgpu_cs_syncobj_wait(afence->ws->dev, &afence->syncobj, 1,
                                 abs_timeout, 0, NULL))
         return false;

      afence->signalled = true;
      return true;
   }

   /* The fence might not have a number assigned if its IB is being
    * submitted in the other thread right now. Wait until the submission
    * is done. */
   if (!util_queue_fence_wait_timeout(&afence->submitted, abs_timeout))
      return false;

   user_fence_cpu = afence->user_fence_cpu_address;
   if (user_fence_cpu) {
      if (*user_fence_cpu >= afence->fence.fence) {
         afence->signalled = true;
         return true;
      }

      /* No timeout, just query: no need for the ioctl. */
      if (!absolute && !timeout)
         return false;
   }

   /* Now use the libdrm query. */
   r = amdgpu_cs_query_fence_status(&afence->fence,
				    abs_timeout,
				    AMDGPU_QUERY_FENCE_TIMEOUT_IS_ABSOLUTE,
				    &expired);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_cs_query_fence_status failed.\n");
      return false;
   }

   if (expired) {
      /* This variable can only transition from false to true, so it doesn't
       * matter if threads race for it. */
      afence->signalled = true;
      return true;
   }
   return false;
}

static bool amdgpu_fence_wait_rel_timeout(struct radeon_winsys *rws,
                                          struct pipe_fence_handle *fence,
                                          uint64_t timeout)
{
   return amdgpu_fence_wait(fence, timeout, false);
}

static struct pipe_fence_handle *
amdgpu_cs_get_next_fence(struct radeon_cmdbuf *rcs)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);
   struct pipe_fence_handle *fence = NULL;

   if (cs->noop)
      return NULL;

   if (cs->next_fence) {
      amdgpu_fence_reference(&fence, cs->next_fence);
      return fence;
   }

   fence = amdgpu_fence_create(cs);
   if (!fence)
      return NULL;

   amdgpu_fence_reference(&cs->next_fence, fence);
   return fence;
}

/* CONTEXTS */

static uint32_t
radeon_to_amdgpu_priority(enum radeon_ctx_priority radeon_priority)
{
   switch (radeon_priority) {
   case RADEON_CTX_PRIORITY_REALTIME:
      return AMDGPU_CTX_PRIORITY_VERY_HIGH;
   case RADEON_CTX_PRIORITY_HIGH:
      return AMDGPU_CTX_PRIORITY_HIGH;
   case RADEON_CTX_PRIORITY_MEDIUM:
      return AMDGPU_CTX_PRIORITY_NORMAL;
   case RADEON_CTX_PRIORITY_LOW:
      return AMDGPU_CTX_PRIORITY_LOW;
   default:
      unreachable("Invalid context priority");
   }
}

static struct radeon_winsys_ctx *amdgpu_ctx_create(struct radeon_winsys *ws,
                                                   enum radeon_ctx_priority priority,
                                                   bool allow_context_lost)
{
   struct amdgpu_ctx *ctx = CALLOC_STRUCT(amdgpu_ctx);
   int r;
   struct amdgpu_bo_alloc_request alloc_buffer = {};
   uint32_t amdgpu_priority = radeon_to_amdgpu_priority(priority);
   amdgpu_bo_handle buf_handle;

   if (!ctx)
      return NULL;

   ctx->ws = amdgpu_winsys(ws);
   ctx->reference.count = 1;
   ctx->allow_context_lost = allow_context_lost;

   r = amdgpu_cs_ctx_create2(ctx->ws->dev, amdgpu_priority, &ctx->ctx);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_cs_ctx_create2 failed. (%i)\n", r);
      goto error_create;
   }

   alloc_buffer.alloc_size = ctx->ws->info.gart_page_size;
   alloc_buffer.phys_alignment = ctx->ws->info.gart_page_size;
   alloc_buffer.preferred_heap = AMDGPU_GEM_DOMAIN_GTT;

   r = amdgpu_bo_alloc(ctx->ws->dev, &alloc_buffer, &buf_handle);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_bo_alloc failed. (%i)\n", r);
      goto error_user_fence_alloc;
   }

   r = amdgpu_bo_cpu_map(buf_handle, (void**)&ctx->user_fence_cpu_address_base);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_bo_cpu_map failed. (%i)\n", r);
      goto error_user_fence_map;
   }

   memset(ctx->user_fence_cpu_address_base, 0, alloc_buffer.alloc_size);
   ctx->user_fence_bo = buf_handle;

   return (struct radeon_winsys_ctx*)ctx;

error_user_fence_map:
   amdgpu_bo_free(buf_handle);
error_user_fence_alloc:
   amdgpu_cs_ctx_free(ctx->ctx);
error_create:
   FREE(ctx);
   return NULL;
}

static void amdgpu_ctx_destroy(struct radeon_winsys_ctx *rwctx)
{
   struct amdgpu_ctx *ctx = (struct amdgpu_ctx*)rwctx;

   amdgpu_ctx_reference(&ctx, NULL);
}

static void amdgpu_pad_gfx_compute_ib(struct amdgpu_winsys *ws, enum amd_ip_type ip_type,
                                      uint32_t *ib, uint32_t *num_dw, unsigned leave_dw_space)
{
   unsigned pad_dw_mask = ws->info.ip[ip_type].ib_pad_dw_mask;
   unsigned unaligned_dw = (*num_dw + leave_dw_space) & pad_dw_mask;

   if (unaligned_dw) {
      int remaining = pad_dw_mask + 1 - unaligned_dw;

      /* Only pad by 1 dword with the type-2 NOP if necessary. */
      if (remaining == 1 && ws->info.gfx_ib_pad_with_type2) {
         ib[(*num_dw)++] = PKT2_NOP_PAD;
      } else {
         /* Pad with a single NOP packet to minimize CP overhead because NOP is a variable-sized
          * packet. The size of the packet body after the header is always count + 1.
          * If count == -1, there is no packet body. NOP is the only packet that can have
          * count == -1, which is the definition of PKT3_NOP_PAD (count == 0x3fff means -1).
          */
         ib[(*num_dw)++] = PKT3(PKT3_NOP, remaining - 2, 0);
         *num_dw += remaining - 1;
      }
   }
   assert(((*num_dw + leave_dw_space) & pad_dw_mask) == 0);
}

static int amdgpu_submit_gfx_nop(struct amdgpu_ctx *ctx)
{
   struct amdgpu_bo_alloc_request request = {0};
   struct drm_amdgpu_bo_list_in bo_list_in;
   struct drm_amdgpu_cs_chunk_ib ib_in = {0};
   amdgpu_bo_handle buf_handle;
   amdgpu_va_handle va_handle = NULL;
   struct drm_amdgpu_cs_chunk chunks[2];
   void *cpu = NULL;
   uint64_t seq_no;
   uint64_t va;
   int r;

   /* Older amdgpu doesn't report if the reset is complete or not. Detect
    * it by submitting a no-op job. If it reports an error, then assume
    * that the reset is not complete.
    */
   amdgpu_context_handle temp_ctx;
   r = amdgpu_cs_ctx_create2(ctx->ws->dev, AMDGPU_CTX_PRIORITY_NORMAL, &temp_ctx);
   if (r)
      return r;

   request.preferred_heap = AMDGPU_GEM_DOMAIN_VRAM;
   request.alloc_size = 4096;
   request.phys_alignment = 4096;
   r = amdgpu_bo_alloc(ctx->ws->dev, &request, &buf_handle);
   if (r)
      goto destroy_ctx;

   r = amdgpu_va_range_alloc(ctx->ws->dev, amdgpu_gpu_va_range_general,
                 request.alloc_size, request.phys_alignment,
                 0, &va, &va_handle,
                 AMDGPU_VA_RANGE_32_BIT | AMDGPU_VA_RANGE_HIGH);
   if (r)
      goto destroy_bo;
   r = amdgpu_bo_va_op_raw(ctx->ws->dev, buf_handle, 0, request.alloc_size, va,
                           AMDGPU_VM_PAGE_READABLE | AMDGPU_VM_PAGE_WRITEABLE | AMDGPU_VM_PAGE_EXECUTABLE,
                           AMDGPU_VA_OP_MAP);
   if (r)
      goto destroy_bo;

   r = amdgpu_bo_cpu_map(buf_handle, &cpu);
   if (r)
      goto destroy_bo;

   unsigned noop_dw_size = ctx->ws->info.ip[AMD_IP_GFX].ib_pad_dw_mask + 1;
   ((uint32_t*)cpu)[0] = PKT3(PKT3_NOP, noop_dw_size - 2, 0);

   amdgpu_bo_cpu_unmap(buf_handle);

   struct drm_amdgpu_bo_list_entry list;
   amdgpu_bo_export(buf_handle, amdgpu_bo_handle_type_kms, &list.bo_handle);
   list.bo_priority = 0;

   bo_list_in.list_handle = ~0;
   bo_list_in.bo_number = 1;
   bo_list_in.bo_info_size = sizeof(struct drm_amdgpu_bo_list_entry);
   bo_list_in.bo_info_ptr = (uint64_t)(uintptr_t)&list;

   ib_in.ip_type = AMD_IP_GFX;
   ib_in.ib_bytes = noop_dw_size * 4;
   ib_in.va_start = va;

   chunks[0].chunk_id = AMDGPU_CHUNK_ID_BO_HANDLES;
   chunks[0].length_dw = sizeof(struct drm_amdgpu_bo_list_in) / 4;
   chunks[0].chunk_data = (uintptr_t)&bo_list_in;

   chunks[1].chunk_id = AMDGPU_CHUNK_ID_IB;
   chunks[1].length_dw = sizeof(struct drm_amdgpu_cs_chunk_ib) / 4;
   chunks[1].chunk_data = (uintptr_t)&ib_in;

   r = amdgpu_cs_submit_raw2(ctx->ws->dev, temp_ctx, 0, 2, chunks, &seq_no);

destroy_bo:
   if (va_handle)
      amdgpu_va_range_free(va_handle);
   amdgpu_bo_free(buf_handle);
destroy_ctx:
   amdgpu_cs_ctx_free(temp_ctx);

   return r;
}

static void
amdgpu_ctx_set_sw_reset_status(struct radeon_winsys_ctx *rwctx, enum pipe_reset_status status,
                               const char *format, ...)
{
   struct amdgpu_ctx *ctx = (struct amdgpu_ctx*)rwctx;

   /* Don't overwrite the last reset status. */
   if (ctx->sw_status != PIPE_NO_RESET)
      return;

   ctx->sw_status = status;

   if (!ctx->allow_context_lost) {
      va_list args;

      va_start(args, format);
      vfprintf(stderr, format, args);
      va_end(args);

      /* Non-robust contexts are allowed to terminate the process. The only alternative is
       * to skip command submission, which would look like a freeze because nothing is drawn,
       * which looks like a hang without any reset.
       */
      abort();
   }
}

static enum pipe_reset_status
amdgpu_ctx_query_reset_status(struct radeon_winsys_ctx *rwctx, bool full_reset_only,
                              bool *needs_reset, bool *reset_completed)
{
   struct amdgpu_ctx *ctx = (struct amdgpu_ctx*)rwctx;
   int r;

   if (needs_reset)
      *needs_reset = false;
   if (reset_completed)
      *reset_completed = false;

   /* Return a failure due to a GPU hang. */
   uint64_t flags;

   if (full_reset_only && ctx->sw_status == PIPE_NO_RESET) {
      /* If the caller is only interested in full reset (= wants to ignore soft
       * recoveries), we can use the rejected cs count as a quick first check.
       */
      return PIPE_NO_RESET;
   }

   r = amdgpu_cs_query_reset_state2(ctx->ctx, &flags);
   if (r) {
      fprintf(stderr, "amdgpu: amdgpu_cs_query_reset_state2 failed. (%i)\n", r);
      return PIPE_NO_RESET;
   }

   if (flags & AMDGPU_CTX_QUERY2_FLAGS_RESET) {
      if (reset_completed) {
         /* The ARB_robustness spec says:
          *
          *    If a reset status other than NO_ERROR is returned and subsequent
          *    calls return NO_ERROR, the context reset was encountered and
          *    completed. If a reset status is repeatedly returned, the context may
          *    be in the process of resetting.
          *
          * Starting with drm_minor >= 54 amdgpu reports if the reset is complete,
          * so don't do anything special. On older kernels, submit a no-op cs. If it
          * succeeds then assume the reset is complete.
          */
         if (!(flags & AMDGPU_CTX_QUERY2_FLAGS_RESET_IN_PROGRESS))
            *reset_completed = true;

         if (ctx->ws->info.drm_minor < 54 && ctx->ws->info.has_graphics)
            *reset_completed = amdgpu_submit_gfx_nop(ctx) == 0;
      }

      if (needs_reset)
            *needs_reset = flags & AMDGPU_CTX_QUERY2_FLAGS_VRAMLOST;
      if (flags & AMDGPU_CTX_QUERY2_FLAGS_GUILTY)
         return PIPE_GUILTY_CONTEXT_RESET;
      else
         return PIPE_INNOCENT_CONTEXT_RESET;
   }

   /* Return a failure due to SW issues. */
   if (ctx->sw_status != PIPE_NO_RESET) {
      if (needs_reset)
         *needs_reset = true;
      return ctx->sw_status;
   }
   if (needs_reset)
      *needs_reset = false;
   return PIPE_NO_RESET;
}

/* COMMAND SUBMISSION */

static bool amdgpu_cs_has_user_fence(struct amdgpu_cs *acs)
{
   return acs->ip_type == AMD_IP_GFX ||
          acs->ip_type == AMD_IP_COMPUTE ||
          acs->ip_type == AMD_IP_SDMA;
}

static inline unsigned amdgpu_cs_epilog_dws(struct amdgpu_cs *cs)
{
   if (cs->has_chaining)
      return 4; /* for chaining */

   return 0;
}

static struct amdgpu_cs_buffer *
amdgpu_lookup_buffer(struct amdgpu_cs_context *cs, struct amdgpu_winsys_bo *bo,
                     struct amdgpu_buffer_list *list)
{
   unsigned num_buffers = list->num_buffers;
   struct amdgpu_cs_buffer *buffers = list->buffers;
   unsigned hash = bo->unique_id & (BUFFER_HASHLIST_SIZE-1);
   int i = cs->buffer_indices_hashlist[hash];

   /* not found or found */
   if (i < 0)
      return NULL;

   if (i < num_buffers && buffers[i].bo == bo)
      return &buffers[i];

   /* Hash collision, look for the BO in the list of buffers linearly. */
   for (int i = num_buffers - 1; i >= 0; i--) {
      if (buffers[i].bo == bo) {
         /* Put this buffer in the hash list.
          * This will prevent additional hash collisions if there are
          * several consecutive lookup_buffer calls for the same buffer.
          *
          * Example: Assuming buffers A,B,C collide in the hash list,
          * the following sequence of buffers:
          *         AAAAAAAAAAABBBBBBBBBBBBBBCCCCCCCC
          * will collide here: ^ and here:   ^,
          * meaning that we should get very few collisions in the end. */
         cs->buffer_indices_hashlist[hash] = i & 0x7fff;
         return &buffers[i];
      }
   }
   return NULL;
}

struct amdgpu_cs_buffer *
amdgpu_lookup_buffer_any_type(struct amdgpu_cs_context *cs, struct amdgpu_winsys_bo *bo)
{
   return amdgpu_lookup_buffer(cs, bo, &cs->buffer_lists[get_buf_list_idx(bo)]);
}

static struct amdgpu_cs_buffer *
amdgpu_do_add_buffer(struct amdgpu_cs_context *cs, struct amdgpu_winsys_bo *bo,
                     struct amdgpu_buffer_list *list)
{
   /* New buffer, check if the backing array is large enough. */
   if (list->num_buffers >= list->max_buffers) {
      unsigned new_max =
         MAX2(list->max_buffers + 16, (unsigned)(list->max_buffers * 1.3));
      struct amdgpu_cs_buffer *new_buffers;

      new_buffers = REALLOC(list->buffers, list->max_buffers * sizeof(*new_buffers),
                            new_max * sizeof(*new_buffers));
      if (!new_buffers) {
         fprintf(stderr, "amdgpu_do_add_buffer: allocation failed\n");
         return NULL;
      }

      list->max_buffers = new_max;
      list->buffers = new_buffers;
   }

   int idx = list->num_buffers;
   struct amdgpu_cs_buffer *buffer = &list->buffers[idx];

   memset(buffer, 0, sizeof(*buffer));
   amdgpu_winsys_bo_reference(cs->ws, &buffer->bo, bo);
   list->num_buffers++;

   unsigned hash = bo->unique_id & (BUFFER_HASHLIST_SIZE-1);
   cs->buffer_indices_hashlist[hash] = idx & 0x7fff;
   return buffer;
}

static struct amdgpu_cs_buffer *
amdgpu_lookup_or_add_buffer(struct amdgpu_cs_context *cs, struct amdgpu_winsys_bo *bo,
                            enum amdgpu_bo_type type)
{
   struct amdgpu_buffer_list *list = &cs->buffer_lists[type];
   struct amdgpu_cs_buffer *buffer = amdgpu_lookup_buffer(cs, bo, list);

   return buffer ? buffer : amdgpu_do_add_buffer(cs, bo, list);
}

static struct amdgpu_cs_buffer *
amdgpu_lookup_or_add_slab_buffer(struct amdgpu_cs_context *cs, struct amdgpu_winsys_bo *bo)
{
   struct amdgpu_buffer_list *list = &cs->buffer_lists[AMDGPU_BO_SLAB_ENTRY];
   struct amdgpu_cs_buffer *buffer = amdgpu_lookup_buffer(cs, bo, list);

   if (buffer)
      return buffer;

   struct amdgpu_cs_buffer *real_buffer =
      amdgpu_lookup_or_add_buffer(cs, &get_slab_entry_real_bo(bo)->b, AMDGPU_BO_REAL);
   if (!real_buffer)
      return NULL;

   buffer = amdgpu_do_add_buffer(cs, bo, list);
   if (buffer)
      buffer->slab_real_idx = real_buffer - cs->buffer_lists[AMDGPU_BO_REAL].buffers;
   return buffer;
}

static unsigned amdgpu_cs_add_buffer(struct radeon_cmdbuf *rcs,
                                    struct pb_buffer_lean *buf,
                                    unsigned usage,
                                    enum radeon_bo_domain domains)
{
   /* Don't use the "domains" parameter. Amdgpu doesn't support changing
    * the buffer placement during command submission.
    */
   struct amdgpu_cs_context *cs = (struct amdgpu_cs_context*)rcs->csc;
   struct amdgpu_winsys_bo *bo = (struct amdgpu_winsys_bo*)buf;
   struct amdgpu_cs_buffer *buffer;

   /* Fast exit for no-op calls.
    * This is very effective with suballocators and linear uploaders that
    * are outside of the winsys.
    */
   if (bo == cs->last_added_bo &&
       (usage & cs->last_added_bo_usage) == usage)
      return 0;

   if (bo->type == AMDGPU_BO_SLAB_ENTRY) {
      buffer = amdgpu_lookup_or_add_slab_buffer(cs, bo);
      if (!buffer)
         return 0;

      cs->buffer_lists[AMDGPU_BO_REAL].buffers[buffer->slab_real_idx].usage |=
         usage & ~RADEON_USAGE_SYNCHRONIZED;
   } else if (is_real_bo(bo)) {
      buffer = amdgpu_lookup_or_add_buffer(cs, bo, AMDGPU_BO_REAL);
      if (!buffer)
         return 0;
   } else {
      buffer = amdgpu_lookup_or_add_buffer(cs, bo, AMDGPU_BO_SPARSE);
      if (!buffer)
         return 0;
   }

   buffer->usage |= usage;

   cs->last_added_bo_usage = buffer->usage;
   cs->last_added_bo = bo;
   return 0;
}

static bool amdgpu_ib_new_buffer(struct amdgpu_winsys *ws,
                                 struct amdgpu_ib *main_ib,
                                 struct amdgpu_cs *cs)
{
   struct pb_buffer_lean *pb;
   uint8_t *mapped;
   unsigned buffer_size;

   /* Always create a buffer that is at least as large as the maximum seen IB
    * size, aligned to a power of two (and multiplied by 4 to reduce internal
    * fragmentation if chaining is not available). Limit to 512k dwords, which
    * is the largest power of two that fits into the size field of the
    * INDIRECT_BUFFER packet.
    */
   if (cs->has_chaining)
      buffer_size = 4 * util_next_power_of_two(main_ib->max_ib_size_dw);
   else
      buffer_size = 4 * util_next_power_of_two(4 * main_ib->max_ib_size_dw);

   const unsigned min_size = MAX2(main_ib->max_check_space_size, 8 * 1024 * 4);
   const unsigned max_size = 512 * 1024 * 4;

   buffer_size = MIN2(buffer_size, max_size);
   buffer_size = MAX2(buffer_size, min_size); /* min_size is more important */

   /* Use cached GTT for command buffers. Writing to other heaps is very slow on the CPU.
    * The speed of writing to GTT WC is somewhere between no difference and very slow, while
    * VRAM being very slow a lot more often.
    *
    * Bypass GL2 because command buffers are read only once. Bypassing GL2 has better latency
    * and doesn't have to wait for cached GL2 requests to be processed.
    */
   enum radeon_bo_domain domain = RADEON_DOMAIN_GTT;
   unsigned flags = RADEON_FLAG_NO_INTERPROCESS_SHARING |
                    RADEON_FLAG_GL2_BYPASS;

   if (cs->ip_type == AMD_IP_GFX ||
       cs->ip_type == AMD_IP_COMPUTE ||
       cs->ip_type == AMD_IP_SDMA) {
      /* Avoids hangs with "rendercheck -t cacomposite -f a8r8g8b8" via glamor
       * on Navi 14
       */
      flags |= RADEON_FLAG_32BIT;
   }

   pb = amdgpu_bo_create(ws, buffer_size,
                         ws->info.gart_page_size,
                         domain, flags);
   if (!pb)
      return false;

   mapped = amdgpu_bo_map(&ws->dummy_ws.base, pb, NULL, PIPE_MAP_WRITE);
   if (!mapped) {
      radeon_bo_reference(&ws->dummy_ws.base, &pb, NULL);
      return false;
   }

   radeon_bo_reference(&ws->dummy_ws.base, &main_ib->big_buffer, pb);
   radeon_bo_reference(&ws->dummy_ws.base, &pb, NULL);

   main_ib->gpu_address = amdgpu_bo_get_va(main_ib->big_buffer);
   main_ib->big_buffer_cpu_ptr = mapped;
   main_ib->used_ib_space = 0;

   return true;
}

static bool amdgpu_get_new_ib(struct amdgpu_winsys *ws,
                              struct radeon_cmdbuf *rcs,
                              struct amdgpu_ib *main_ib,
                              struct amdgpu_cs *cs)
{
   /* Small IBs are better than big IBs, because the GPU goes idle quicker
    * and there is less waiting for buffers and fences. Proof:
    *   http://www.phoronix.com/scan.php?page=article&item=mesa-111-si&num=1
    */
   struct drm_amdgpu_cs_chunk_ib *chunk_ib = &cs->csc->chunk_ib[IB_MAIN];
   /* This is the minimum size of a contiguous IB. */
   unsigned ib_size = 4 * 1024 * 4;

   /* Always allocate at least the size of the biggest cs_check_space call,
    * because precisely the last call might have requested this size.
    */
   ib_size = MAX2(ib_size, main_ib->max_check_space_size);

   if (!cs->has_chaining) {
      ib_size = MAX2(ib_size,
                     4 * MIN2(util_next_power_of_two(main_ib->max_ib_size_dw),
                              IB_MAX_SUBMIT_DWORDS));
   }

   main_ib->max_ib_size_dw = main_ib->max_ib_size_dw - main_ib->max_ib_size_dw / 32;

   rcs->prev_dw = 0;
   rcs->num_prev = 0;
   rcs->current.cdw = 0;
   rcs->current.buf = NULL;

   /* Allocate a new buffer for IBs if the current buffer is all used. */
   if (!main_ib->big_buffer ||
       main_ib->used_ib_space + ib_size > main_ib->big_buffer->size) {
      if (!amdgpu_ib_new_buffer(ws, main_ib, cs))
         return false;
   }

   chunk_ib->va_start = main_ib->gpu_address + main_ib->used_ib_space;
   chunk_ib->ib_bytes = 0;
   /* ib_bytes is in dwords and the conversion to bytes will be done before
    * the CS ioctl. */
   main_ib->ptr_ib_size = &chunk_ib->ib_bytes;
   main_ib->is_chained_ib = false;

   amdgpu_cs_add_buffer(rcs, main_ib->big_buffer,
                        RADEON_USAGE_READ | RADEON_PRIO_IB, 0);

   rcs->current.buf = (uint32_t*)(main_ib->big_buffer_cpu_ptr + main_ib->used_ib_space);

   cs->csc->ib_main_addr = rcs->current.buf;

   ib_size = main_ib->big_buffer->size - main_ib->used_ib_space;
   rcs->current.max_dw = ib_size / 4 - amdgpu_cs_epilog_dws(cs);
   return true;
}

static void amdgpu_set_ib_size(struct radeon_cmdbuf *rcs, struct amdgpu_ib *ib)
{
   if (ib->is_chained_ib) {
      *ib->ptr_ib_size = rcs->current.cdw |
                         S_3F2_CHAIN(1) | S_3F2_VALID(1) |
                         S_3F2_PRE_ENA(((struct amdgpu_cs*)ib)->preamble_ib_bo != NULL);
   } else {
      *ib->ptr_ib_size = rcs->current.cdw;
   }
}

static void amdgpu_ib_finalize(struct amdgpu_winsys *ws, struct radeon_cmdbuf *rcs,
                               struct amdgpu_ib *ib, enum amd_ip_type ip_type)
{
   amdgpu_set_ib_size(rcs, ib);
   ib->used_ib_space += rcs->current.cdw * 4;
   ib->used_ib_space = align(ib->used_ib_space, ws->info.ip[ip_type].ib_alignment);
   ib->max_ib_size_dw = MAX2(ib->max_ib_size_dw, rcs->prev_dw + rcs->current.cdw);
}

static bool amdgpu_init_cs_context(struct amdgpu_winsys *ws,
                                   struct amdgpu_cs_context *cs,
                                   enum amd_ip_type ip_type)
{
   for (unsigned i = 0; i < ARRAY_SIZE(cs->chunk_ib); i++) {
      cs->chunk_ib[i].ip_type = ip_type;
      cs->chunk_ib[i].flags = 0;

      if (ip_type == AMD_IP_GFX || ip_type == AMD_IP_COMPUTE) {
         /* The kernel shouldn't invalidate L2 and vL1. The proper place for cache invalidation
          * is the beginning of IBs because completion of an IB doesn't care about the state of
          * GPU caches, only the beginning of an IB does. Draw calls from multiple IBs can be
          * executed in parallel, so draw calls from the current IB can finish after the next IB
          * starts drawing, and so the cache flush at the end of IBs is usually late and thus
          * useless.
          */
         cs->chunk_ib[i].flags |= AMDGPU_IB_FLAG_TC_WB_NOT_INVALIDATE;
      }
   }

   cs->chunk_ib[IB_PREAMBLE].flags |= AMDGPU_IB_FLAG_PREAMBLE;
   cs->last_added_bo = NULL;
   return true;
}

static void cleanup_fence_list(struct amdgpu_fence_list *fences)
{
   for (unsigned i = 0; i < fences->num; i++)
      amdgpu_fence_reference(&fences->list[i], NULL);
   fences->num = 0;
}

static void amdgpu_cs_context_cleanup(struct amdgpu_winsys *ws, struct amdgpu_cs_context *cs)
{
   for (unsigned i = 0; i < ARRAY_SIZE(cs->buffer_lists); i++) {
      struct amdgpu_cs_buffer *buffers = cs->buffer_lists[i].buffers;
      unsigned num_buffers = cs->buffer_lists[i].num_buffers;

      for (unsigned j = 0; j < num_buffers; j++)
         amdgpu_winsys_bo_reference(ws, &buffers[j].bo, NULL);

      cs->buffer_lists[i].num_buffers = 0;
   }

   cs->seq_no_dependencies.valid_fence_mask = 0;
   cleanup_fence_list(&cs->fence_dependencies);
   cleanup_fence_list(&cs->syncobj_dependencies);
   cleanup_fence_list(&cs->syncobj_to_signal);
   amdgpu_fence_reference(&cs->fence, NULL);
   cs->last_added_bo = NULL;
}

static void amdgpu_destroy_cs_context(struct amdgpu_winsys *ws, struct amdgpu_cs_context *cs)
{
   amdgpu_cs_context_cleanup(ws, cs);
   for (unsigned i = 0; i < ARRAY_SIZE(cs->buffer_lists); i++)
      FREE(cs->buffer_lists[i].buffers);
   FREE(cs->fence_dependencies.list);
   FREE(cs->syncobj_dependencies.list);
   FREE(cs->syncobj_to_signal.list);
}


static enum amd_ip_type amdgpu_cs_get_ip_type(struct radeon_cmdbuf *rcs)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);
   return cs->ip_type;
}


static bool
amdgpu_cs_create(struct radeon_cmdbuf *rcs,
                 struct radeon_winsys_ctx *rwctx,
                 enum amd_ip_type ip_type,
                 void (*flush)(void *ctx, unsigned flags,
                               struct pipe_fence_handle **fence),
                 void *flush_ctx)
{
   struct amdgpu_ctx *ctx = (struct amdgpu_ctx*)rwctx;
   struct amdgpu_cs *cs;

   cs = CALLOC_STRUCT(amdgpu_cs);
   if (!cs) {
      return false;
   }

   util_queue_fence_init(&cs->flush_completed);

   cs->ws = ctx->ws;
   cs->ctx = ctx;
   cs->flush_cs = flush;
   cs->flush_data = flush_ctx;
   cs->ip_type = ip_type;
   cs->noop = ctx->ws->noop_cs;
   cs->has_chaining = ctx->ws->info.gfx_level >= GFX7 &&
                      (ip_type == AMD_IP_GFX || ip_type == AMD_IP_COMPUTE);

   /* Compute the queue index by counting the IPs that have queues. */
   assert(ip_type < ARRAY_SIZE(ctx->ws->info.ip));
   assert(ctx->ws->info.ip[ip_type].num_queues);
   cs->queue_index = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(ctx->ws->info.ip); i++) {
      if (!ctx->ws->info.ip[i].num_queues)
         continue;

      if (i == ip_type)
         break;

      cs->queue_index++;
   }

   assert(cs->queue_index < AMDGPU_MAX_QUEUES);

   struct amdgpu_cs_fence_info fence_info;
   fence_info.handle = cs->ctx->user_fence_bo;
   fence_info.offset = cs->ip_type * 4;
   amdgpu_cs_chunk_fence_info_to_data(&fence_info, (void*)&cs->fence_chunk);

   if (!amdgpu_init_cs_context(ctx->ws, &cs->csc1, ip_type)) {
      FREE(cs);
      return false;
   }

   if (!amdgpu_init_cs_context(ctx->ws, &cs->csc2, ip_type)) {
      amdgpu_destroy_cs_context(ctx->ws, &cs->csc1);
      FREE(cs);
      return false;
   }

   memset(cs->buffer_indices_hashlist, -1, sizeof(cs->buffer_indices_hashlist));

   /* Set the first submission context as current. */
   rcs->csc = cs->csc = &cs->csc1;
   cs->cst = &cs->csc2;

   /* Assign to both amdgpu_cs_context; only csc will use it. */
   cs->csc1.buffer_indices_hashlist = cs->buffer_indices_hashlist;
   cs->csc2.buffer_indices_hashlist = cs->buffer_indices_hashlist;

   cs->csc1.ws = ctx->ws;
   cs->csc2.ws = ctx->ws;

   rcs->priv = cs;

   if (!amdgpu_get_new_ib(ctx->ws, rcs, &cs->main_ib, cs)) {
      amdgpu_destroy_cs_context(ctx->ws, &cs->csc2);
      amdgpu_destroy_cs_context(ctx->ws, &cs->csc1);
      FREE(cs);
      rcs->priv = NULL;
      return false;
   }

   p_atomic_inc(&ctx->ws->num_cs);
   return true;
}

static bool
amdgpu_cs_setup_preemption(struct radeon_cmdbuf *rcs, const uint32_t *preamble_ib,
                           unsigned preamble_num_dw)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);
   struct amdgpu_winsys *ws = cs->ws;
   struct amdgpu_cs_context *csc[2] = {&cs->csc1, &cs->csc2};
   unsigned size = align(preamble_num_dw * 4, ws->info.ip[AMD_IP_GFX].ib_alignment);
   struct pb_buffer_lean *preamble_bo;
   uint32_t *map;

   /* Create the preamble IB buffer. */
   preamble_bo = amdgpu_bo_create(ws, size, ws->info.ip[AMD_IP_GFX].ib_alignment,
                                  RADEON_DOMAIN_VRAM,
                                  RADEON_FLAG_NO_INTERPROCESS_SHARING |
                                  RADEON_FLAG_GTT_WC |
                                  RADEON_FLAG_READ_ONLY);
   if (!preamble_bo)
      return false;

   map = (uint32_t*)amdgpu_bo_map(&ws->dummy_ws.base, preamble_bo, NULL,
                                  PIPE_MAP_WRITE | RADEON_MAP_TEMPORARY);
   if (!map) {
      radeon_bo_reference(&ws->dummy_ws.base, &preamble_bo, NULL);
      return false;
   }

   /* Upload the preamble IB. */
   memcpy(map, preamble_ib, preamble_num_dw * 4);

   /* Pad the IB. */
   amdgpu_pad_gfx_compute_ib(ws, cs->ip_type, map, &preamble_num_dw, 0);
   amdgpu_bo_unmap(&ws->dummy_ws.base, preamble_bo);

   for (unsigned i = 0; i < 2; i++) {
      csc[i]->chunk_ib[IB_PREAMBLE].va_start = amdgpu_bo_get_va(preamble_bo);
      csc[i]->chunk_ib[IB_PREAMBLE].ib_bytes = preamble_num_dw * 4;

      csc[i]->chunk_ib[IB_MAIN].flags |= AMDGPU_IB_FLAG_PREEMPT;
   }

   assert(!cs->preamble_ib_bo);
   cs->preamble_ib_bo = preamble_bo;

   amdgpu_cs_add_buffer(rcs, cs->preamble_ib_bo,
                        RADEON_USAGE_READ | RADEON_PRIO_IB, 0);
   return true;
}

static bool amdgpu_cs_validate(struct radeon_cmdbuf *rcs)
{
   return true;
}

static bool amdgpu_cs_check_space(struct radeon_cmdbuf *rcs, unsigned dw)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);
   struct amdgpu_ib *main_ib = &cs->main_ib;

   assert(rcs->current.cdw <= rcs->current.max_dw);

   unsigned projected_size_dw = rcs->prev_dw + rcs->current.cdw + dw;

   if (projected_size_dw > IB_MAX_SUBMIT_DWORDS)
      return false;

   if (rcs->current.max_dw - rcs->current.cdw >= dw)
      return true;

   unsigned cs_epilog_dw = amdgpu_cs_epilog_dws(cs);
   unsigned need_byte_size = (dw + cs_epilog_dw) * 4;
   /* 125% of the size for IB epilog. */
   unsigned safe_byte_size = need_byte_size + need_byte_size / 4;
   main_ib->max_check_space_size = MAX2(main_ib->max_check_space_size, safe_byte_size);
   main_ib->max_ib_size_dw = MAX2(main_ib->max_ib_size_dw, projected_size_dw);

   if (!cs->has_chaining)
      return false;

   /* Allocate a new chunk */
   if (rcs->num_prev >= rcs->max_prev) {
      unsigned new_max_prev = MAX2(1, 2 * rcs->max_prev);
      struct radeon_cmdbuf_chunk *new_prev;

      new_prev = REALLOC(rcs->prev,
                         sizeof(*new_prev) * rcs->max_prev,
                         sizeof(*new_prev) * new_max_prev);
      if (!new_prev)
         return false;

      rcs->prev = new_prev;
      rcs->max_prev = new_max_prev;
   }

   if (!amdgpu_ib_new_buffer(cs->ws, main_ib, cs))
      return false;

   assert(main_ib->used_ib_space == 0);
   uint64_t va = main_ib->gpu_address;

   /* This space was originally reserved. */
   rcs->current.max_dw += cs_epilog_dw;

   /* Pad with NOPs but leave 4 dwords for INDIRECT_BUFFER. */
   amdgpu_pad_gfx_compute_ib(cs->ws, cs->ip_type, rcs->current.buf, &rcs->current.cdw, 4);

   radeon_emit(rcs, PKT3(PKT3_INDIRECT_BUFFER, 2, 0));
   radeon_emit(rcs, va);
   radeon_emit(rcs, va >> 32);
   uint32_t *new_ptr_ib_size = &rcs->current.buf[rcs->current.cdw++];

   assert((rcs->current.cdw & cs->ws->info.ip[cs->ip_type].ib_pad_dw_mask) == 0);
   assert(rcs->current.cdw <= rcs->current.max_dw);

   amdgpu_set_ib_size(rcs, main_ib);
   main_ib->ptr_ib_size = new_ptr_ib_size;
   main_ib->is_chained_ib = true;

   /* Hook up the new chunk */
   rcs->prev[rcs->num_prev].buf = rcs->current.buf;
   rcs->prev[rcs->num_prev].cdw = rcs->current.cdw;
   rcs->prev[rcs->num_prev].max_dw = rcs->current.cdw; /* no modifications */
   rcs->num_prev++;

   rcs->prev_dw += rcs->current.cdw;
   rcs->current.cdw = 0;

   rcs->current.buf = (uint32_t*)(main_ib->big_buffer_cpu_ptr + main_ib->used_ib_space);
   rcs->current.max_dw = main_ib->big_buffer->size / 4 - cs_epilog_dw;

   amdgpu_cs_add_buffer(rcs, main_ib->big_buffer,
                        RADEON_USAGE_READ | RADEON_PRIO_IB, 0);

   return true;
}

static unsigned amdgpu_cs_get_buffer_list(struct radeon_cmdbuf *rcs,
                                          struct radeon_bo_list_item *list)
{
    struct amdgpu_cs_context *cs = amdgpu_cs(rcs)->csc;
    struct amdgpu_buffer_list *real_buffers = &cs->buffer_lists[AMDGPU_BO_REAL];
    unsigned num_real_buffers = real_buffers->num_buffers;

    if (list) {
        for (unsigned i = 0; i < num_real_buffers; i++) {
            list[i].bo_size = real_buffers->buffers[i].bo->base.size;
            list[i].vm_address =
               amdgpu_va_get_start_addr(get_real_bo(real_buffers->buffers[i].bo)->va_handle);
            list[i].priority_usage = real_buffers->buffers[i].usage;
        }
    }
    return num_real_buffers;
}

static void add_fence_to_list(struct amdgpu_fence_list *fences,
                              struct amdgpu_fence *fence)
{
   unsigned idx = fences->num++;

   if (idx >= fences->max) {
      unsigned size;
      const unsigned increment = 8;

      fences->max = idx + increment;
      size = fences->max * sizeof(fences->list[0]);
      fences->list = realloc(fences->list, size);
      /* Clear the newly-allocated elements. */
      memset(fences->list + idx, 0,
             increment * sizeof(fences->list[0]));
   }
   amdgpu_fence_reference(&fences->list[idx], (struct pipe_fence_handle*)fence);
}

static void amdgpu_cs_add_fence_dependency(struct radeon_cmdbuf *rcs,
                                           struct pipe_fence_handle *pfence)
{
   struct amdgpu_cs *acs = amdgpu_cs(rcs);
   struct amdgpu_cs_context *cs = acs->csc;
   struct amdgpu_fence *fence = (struct amdgpu_fence*)pfence;

   util_queue_fence_wait(&fence->submitted);

   if (!fence->imported) {
      /* Ignore idle fences. This will only check the user fence in memory. */
      if (!amdgpu_fence_wait((void *)fence, 0, false)) {
         add_seq_no_to_list(acs->ws, &cs->seq_no_dependencies, fence->queue_index,
                            fence->queue_seq_no);
      }
   } else if (amdgpu_fence_is_syncobj(fence))
      add_fence_to_list(&cs->syncobj_dependencies, fence);
   else
      add_fence_to_list(&cs->fence_dependencies, fence);
}

static void amdgpu_add_bo_fences_to_dependencies(struct amdgpu_cs *acs,
                                                 struct amdgpu_seq_no_fences *dependencies,
                                                 uint_seq_no new_queue_seq_no,
                                                 struct amdgpu_buffer_list *list)
{
   struct amdgpu_winsys *ws = acs->ws;
   unsigned queue_index = acs->queue_index;
   unsigned num_buffers = list->num_buffers;

   for (unsigned i = 0; i < num_buffers; i++) {
      struct amdgpu_cs_buffer *buffer = &list->buffers[i];
      struct amdgpu_winsys_bo *bo = buffer->bo;

      /* Add BO fences from queues other than 'queue_index' to dependencies. */
      if (buffer->usage & RADEON_USAGE_SYNCHRONIZED) {
         u_foreach_bit(other_queue_idx, bo->fences.valid_fence_mask & ~BITFIELD_BIT(queue_index)) {
            add_seq_no_to_list(ws, dependencies, other_queue_idx,
                               bo->fences.seq_no[other_queue_idx]);
         }
      }

      /* Also set the fence in the BO. */
      bo->fences.seq_no[queue_index] = new_queue_seq_no;
      bo->fences.valid_fence_mask |= BITFIELD_BIT(queue_index);
   }
}

static void amdgpu_cs_add_syncobj_signal(struct radeon_cmdbuf *rws,
                                         struct pipe_fence_handle *fence)
{
   struct amdgpu_cs *acs = amdgpu_cs(rws);
   struct amdgpu_cs_context *cs = acs->csc;

   assert(amdgpu_fence_is_syncobj((struct amdgpu_fence *)fence));

   add_fence_to_list(&cs->syncobj_to_signal, (struct amdgpu_fence*)fence);
}

/* Add backing of sparse buffers to the buffer list.
 *
 * This is done late, during submission, to keep the buffer list short before
 * submit, and to avoid managing fences for the backing buffers.
 */
static bool amdgpu_add_sparse_backing_buffers(struct amdgpu_cs_context *cs)
{
   unsigned num_sparse_buffers = cs->buffer_lists[AMDGPU_BO_SPARSE].num_buffers;

   for (unsigned i = 0; i < num_sparse_buffers; ++i) {
      struct amdgpu_cs_buffer *buffer = &cs->buffer_lists[AMDGPU_BO_SPARSE].buffers[i];
      struct amdgpu_bo_sparse *bo = get_sparse_bo(buffer->bo);

      simple_mtx_lock(&bo->commit_lock);

      list_for_each_entry(struct amdgpu_sparse_backing, backing, &bo->backing, list) {
         /* We can directly add the buffer here, because we know that each
          * backing buffer occurs only once.
          */
         struct amdgpu_cs_buffer *real_buffer =
            amdgpu_do_add_buffer(cs, &backing->bo->b, &cs->buffer_lists[AMDGPU_BO_REAL]);
         if (!real_buffer) {
            fprintf(stderr, "%s: failed to add buffer\n", __func__);
            simple_mtx_unlock(&bo->commit_lock);
            return false;
         }

         real_buffer->usage = buffer->usage;
      }

      simple_mtx_unlock(&bo->commit_lock);
   }

   return true;
}

static void amdgpu_cs_submit_ib(void *job, void *gdata, int thread_index)
{
   struct amdgpu_cs *acs = (struct amdgpu_cs*)job;
   struct amdgpu_winsys *ws = acs->ws;
   struct amdgpu_cs_context *cs = acs->cst;
   int i, r;
   uint64_t seq_no = 0;
   bool has_user_fence = amdgpu_cs_has_user_fence(acs);

   simple_mtx_lock(&ws->bo_fence_lock);
   struct amdgpu_queue *queue = &ws->queues[acs->queue_index];
   uint_seq_no prev_seq_no = queue->latest_seq_no;

   /* Generate a per queue sequence number. The logic is similar to the kernel side amdgpu seqno,
    * but the values aren't related.
    */
   uint_seq_no next_seq_no = prev_seq_no + 1;

   /* Wait for the oldest fence to signal. This should always check the user fence, then wait
    * via the ioctl. We have to do this because we are going to release the oldest fence and
    * replace it with the latest fence in the ring.
    */
   struct pipe_fence_handle **oldest_fence =
      &queue->fences[next_seq_no % AMDGPU_FENCE_RING_SIZE];

   if (*oldest_fence) {
      if (!amdgpu_fence_wait(*oldest_fence, 0, false)) {
         /* Take the reference because the fence can be released by other threads after we
          * unlock the mutex.
          */
         struct pipe_fence_handle *tmp_fence = NULL;
         amdgpu_fence_reference(&tmp_fence, *oldest_fence);

         /* Unlock the mutex before waiting. */
         simple_mtx_unlock(&ws->bo_fence_lock);
         amdgpu_fence_wait(tmp_fence, OS_TIMEOUT_INFINITE, false);
         amdgpu_fence_reference(&tmp_fence, NULL);
         simple_mtx_lock(&ws->bo_fence_lock);
      }

      /* Remove the idle fence from the ring. */
      amdgpu_fence_reference(oldest_fence, NULL);
   }

   /* We'll accumulate sequence numbers in this structure. It automatically keeps only the latest
    * sequence number per queue and removes all older ones.
    */
   struct amdgpu_seq_no_fences seq_no_dependencies;
   memcpy(&seq_no_dependencies, &cs->seq_no_dependencies, sizeof(seq_no_dependencies));

   /* Add a fence dependency on the previous IB if the IP has multiple physical queues to
    * make it appear as if it had only 1 queue, or if the previous IB comes from a different
    * context. The reasons are:
    * - Our BO fence tracking only supports 1 queue per IP.
    * - IBs from different contexts must wait for each other and can't execute in a random order.
    */
   struct amdgpu_fence *prev_fence =
      (struct amdgpu_fence*)queue->fences[prev_seq_no % AMDGPU_FENCE_RING_SIZE];

   if (prev_fence && (ws->info.ip[acs->ip_type].num_queues > 1 || queue->last_ctx != acs->ctx))
      add_seq_no_to_list(ws, &seq_no_dependencies, acs->queue_index, prev_seq_no);

   /* Since the kernel driver doesn't synchronize execution between different
    * rings automatically, we have to add fence dependencies manually. This gathers sequence
    * numbers from BOs and sets the next sequence number in the BOs.
    */
   for (unsigned i = 0; i < ARRAY_SIZE(cs->buffer_lists); i++) {
      amdgpu_add_bo_fences_to_dependencies(acs, &seq_no_dependencies, next_seq_no,
                                           &cs->buffer_lists[i]);
   }

#if 0 /* Debug code. */
   printf("submit queue=%u, seq_no=%u\n", acs->queue_index, next_seq_no);

   /* Wait for all previous fences. This can be used when BO fence tracking doesn't work. */
   for (unsigned i = 0; i < AMDGPU_MAX_QUEUES; i++) {
      if (i == acs->queue_index)
         continue;

      struct pipe_fence_handle *fence = queue->fences[ws->queues[i].latest_seq_no % AMDGPU_FENCE_RING_SIZE];
      if (!fence) {
         if (i <= 1)
            printf("      queue %u doesn't have any fence at seq_no %u\n", i, ws->queues[i].latest_seq_no);
         continue;
      }

      bool valid = seq_no_dependencies.valid_fence_mask & BITFIELD_BIT(i);
      uint_seq_no old = seq_no_dependencies.seq_no[i];
      add_seq_no_to_list(ws, &seq_no_dependencies, i, ws->queues[i].latest_seq_no);
      uint_seq_no new = seq_no_dependencies.seq_no[i];

      if (!valid)
         printf("   missing dependency on queue=%u, seq_no=%u\n", i, new);
      else if (old != new)
         printf("   too old dependency on queue=%u, old=%u, new=%u\n", i, old, new);
      else
         printf("   has dependency on queue=%u, seq_no=%u\n", i, old);
   }
#endif

   /* Convert the sequence numbers we gathered to fence dependencies. */
   u_foreach_bit(i, seq_no_dependencies.valid_fence_mask) {
      struct pipe_fence_handle **fence = get_fence_from_ring(ws, &seq_no_dependencies, i);

      if (fence) {
         /* If it's idle, don't add it to the list of dependencies. */
         if (amdgpu_fence_wait(*fence, 0, false))
            amdgpu_fence_reference(fence, NULL);
         else
            add_fence_to_list(&cs->fence_dependencies, (struct amdgpu_fence*)*fence);
      }
   }

   /* Finally, add the IB fence into the winsys queue. */
   amdgpu_fence_reference(&queue->fences[next_seq_no % AMDGPU_FENCE_RING_SIZE], cs->fence);
   queue->latest_seq_no = next_seq_no;
   ((struct amdgpu_fence*)cs->fence)->queue_seq_no = next_seq_no;

   /* Update the last used context in the queue. */
   amdgpu_ctx_reference(&queue->last_ctx, acs->ctx);
   simple_mtx_unlock(&ws->bo_fence_lock);

   struct drm_amdgpu_bo_list_entry *bo_list = NULL;
   unsigned num_bo_handles = 0;
   unsigned initial_num_real_buffers = cs->buffer_lists[AMDGPU_BO_REAL].num_buffers;

#if DEBUG
   /* Prepare the buffer list. */
   if (ws->debug_all_bos) {
      /* The buffer list contains all buffers. This is a slow path that
       * ensures that no buffer is missing in the BO list.
       */
      bo_list = alloca(ws->num_buffers * sizeof(struct drm_amdgpu_bo_list_entry));
      struct amdgpu_bo_real *bo;

      simple_mtx_lock(&ws->global_bo_list_lock);
      LIST_FOR_EACH_ENTRY(bo, &ws->global_bo_list, global_list_item) {
         bo_list[num_bo_handles].bo_handle = bo->kms_handle;
         bo_list[num_bo_handles].bo_priority = 0;
         ++num_bo_handles;
      }
      simple_mtx_unlock(&ws->global_bo_list_lock);
   } else
#endif
   {
      if (!amdgpu_add_sparse_backing_buffers(cs)) {
         fprintf(stderr, "amdgpu: amdgpu_add_sparse_backing_buffers failed\n");
         r = -ENOMEM;
         goto cleanup;
      }

      unsigned num_real_buffers = cs->buffer_lists[AMDGPU_BO_REAL].num_buffers;
      bo_list = alloca((num_real_buffers + 2) * sizeof(struct drm_amdgpu_bo_list_entry));

      for (i = 0; i < num_real_buffers; ++i) {
         struct amdgpu_cs_buffer *buffer = &cs->buffer_lists[AMDGPU_BO_REAL].buffers[i];

         bo_list[num_bo_handles].bo_handle = get_real_bo(buffer->bo)->kms_handle;
         bo_list[num_bo_handles].bo_priority =
            (util_last_bit(buffer->usage & RADEON_ALL_PRIORITIES) - 1) / 2;
         ++num_bo_handles;
      }
   }

   if (acs->ip_type == AMD_IP_GFX)
      ws->gfx_bo_list_counter += cs->buffer_lists[AMDGPU_BO_REAL].num_buffers;

   struct drm_amdgpu_cs_chunk chunks[8];
   unsigned num_chunks = 0;

   /* BO list */
   struct drm_amdgpu_bo_list_in bo_list_in;
   bo_list_in.operation = ~0;
   bo_list_in.list_handle = ~0;
   bo_list_in.bo_number = num_bo_handles;
   bo_list_in.bo_info_size = sizeof(struct drm_amdgpu_bo_list_entry);
   bo_list_in.bo_info_ptr = (uint64_t)(uintptr_t)bo_list;

   chunks[num_chunks].chunk_id = AMDGPU_CHUNK_ID_BO_HANDLES;
   chunks[num_chunks].length_dw = sizeof(struct drm_amdgpu_bo_list_in) / 4;
   chunks[num_chunks].chunk_data = (uintptr_t)&bo_list_in;
   num_chunks++;

   /* Fence dependencies. */
   unsigned num_dependencies = cs->fence_dependencies.num;
   if (num_dependencies) {
      struct drm_amdgpu_cs_chunk_dep *dep_chunk =
         alloca(num_dependencies * sizeof(*dep_chunk));

      for (unsigned i = 0; i < num_dependencies; i++) {
         struct amdgpu_fence *fence =
            (struct amdgpu_fence*)cs->fence_dependencies.list[i];

         assert(util_queue_fence_is_signalled(&fence->submitted));
         amdgpu_cs_chunk_fence_to_dep(&fence->fence, &dep_chunk[i]);
      }

      chunks[num_chunks].chunk_id = AMDGPU_CHUNK_ID_DEPENDENCIES;
      chunks[num_chunks].length_dw = sizeof(dep_chunk[0]) / 4 * num_dependencies;
      chunks[num_chunks].chunk_data = (uintptr_t)dep_chunk;
      num_chunks++;
   }

   /* Syncobj dependencies. */
   unsigned num_syncobj_dependencies = cs->syncobj_dependencies.num;
   if (num_syncobj_dependencies) {
      struct drm_amdgpu_cs_chunk_sem *sem_chunk =
         alloca(num_syncobj_dependencies * sizeof(sem_chunk[0]));

      for (unsigned i = 0; i < num_syncobj_dependencies; i++) {
         struct amdgpu_fence *fence =
            (struct amdgpu_fence*)cs->syncobj_dependencies.list[i];

         if (!amdgpu_fence_is_syncobj(fence))
            continue;

         assert(util_queue_fence_is_signalled(&fence->submitted));
         sem_chunk[i].handle = fence->syncobj;
      }

      chunks[num_chunks].chunk_id = AMDGPU_CHUNK_ID_SYNCOBJ_IN;
      chunks[num_chunks].length_dw = sizeof(sem_chunk[0]) / 4 * num_syncobj_dependencies;
      chunks[num_chunks].chunk_data = (uintptr_t)sem_chunk;
      num_chunks++;
   }

   /* Syncobj signals. */
   unsigned num_syncobj_to_signal = cs->syncobj_to_signal.num;
   if (num_syncobj_to_signal) {
      struct drm_amdgpu_cs_chunk_sem *sem_chunk =
         alloca(num_syncobj_to_signal * sizeof(sem_chunk[0]));

      for (unsigned i = 0; i < num_syncobj_to_signal; i++) {
         struct amdgpu_fence *fence =
            (struct amdgpu_fence*)cs->syncobj_to_signal.list[i];

         assert(amdgpu_fence_is_syncobj(fence));
         sem_chunk[i].handle = fence->syncobj;
      }

      chunks[num_chunks].chunk_id = AMDGPU_CHUNK_ID_SYNCOBJ_OUT;
      chunks[num_chunks].length_dw = sizeof(sem_chunk[0]) / 4
                                     * num_syncobj_to_signal;
      chunks[num_chunks].chunk_data = (uintptr_t)sem_chunk;
      num_chunks++;
   }

   if (ws->info.has_fw_based_shadowing && acs->mcbp_fw_shadow_chunk.shadow_va) {
      chunks[num_chunks].chunk_id = AMDGPU_CHUNK_ID_CP_GFX_SHADOW;
      chunks[num_chunks].length_dw = sizeof(struct drm_amdgpu_cs_chunk_cp_gfx_shadow) / 4;
      chunks[num_chunks].chunk_data = (uintptr_t)&acs->mcbp_fw_shadow_chunk;
      num_chunks++;
   }

   /* Fence */
   if (has_user_fence) {
      chunks[num_chunks].chunk_id = AMDGPU_CHUNK_ID_FENCE;
      chunks[num_chunks].length_dw = sizeof(struct drm_amdgpu_cs_chunk_fence) / 4;
      chunks[num_chunks].chunk_data = (uintptr_t)&acs->fence_chunk;
      num_chunks++;
   }

   /* IB */
   if (cs->chunk_ib[IB_PREAMBLE].ib_bytes) {
      chunks[num_chunks].chunk_id = AMDGPU_CHUNK_ID_IB;
      chunks[num_chunks].length_dw = sizeof(struct drm_amdgpu_cs_chunk_ib) / 4;
      chunks[num_chunks].chunk_data = (uintptr_t)&cs->chunk_ib[IB_PREAMBLE];
      num_chunks++;
   }

   /* IB */
   cs->chunk_ib[IB_MAIN].ib_bytes *= 4; /* Convert from dwords to bytes. */
   chunks[num_chunks].chunk_id = AMDGPU_CHUNK_ID_IB;
   chunks[num_chunks].length_dw = sizeof(struct drm_amdgpu_cs_chunk_ib) / 4;
   chunks[num_chunks].chunk_data = (uintptr_t)&cs->chunk_ib[IB_MAIN];
   num_chunks++;

   if (cs->secure) {
      cs->chunk_ib[IB_PREAMBLE].flags |= AMDGPU_IB_FLAGS_SECURE;
      cs->chunk_ib[IB_MAIN].flags |= AMDGPU_IB_FLAGS_SECURE;
   } else {
      cs->chunk_ib[IB_PREAMBLE].flags &= ~AMDGPU_IB_FLAGS_SECURE;
      cs->chunk_ib[IB_MAIN].flags &= ~AMDGPU_IB_FLAGS_SECURE;
   }

   bool noop = acs->noop;

   if (noop && acs->ip_type == AMD_IP_GFX) {
      /* Reduce the IB size and fill it with NOP to make it like an empty IB. */
      unsigned noop_dw_size = ws->info.ip[AMD_IP_GFX].ib_pad_dw_mask + 1;
      assert(cs->chunk_ib[IB_MAIN].ib_bytes / 4 >= noop_dw_size);

      cs->ib_main_addr[0] = PKT3(PKT3_NOP, noop_dw_size - 2, 0);
      cs->chunk_ib[IB_MAIN].ib_bytes = noop_dw_size * 4;
      noop = false;
   }

   assert(num_chunks <= ARRAY_SIZE(chunks));

   if (unlikely(acs->ctx->sw_status != PIPE_NO_RESET)) {
      r = -ECANCELED;
   } else if (unlikely(noop)) {
      r = 0;
   } else {
      /* Submit the command buffer.
       *
       * The kernel returns -ENOMEM with many parallel processes using GDS such as test suites
       * quite often, but it eventually succeeds after enough attempts. This happens frequently
       * with dEQP using NGG streamout.
       */
      r = 0;

      do {
         /* Wait 1 ms and try again. */
         if (r == -ENOMEM)
            os_time_sleep(1000);

         r = amdgpu_cs_submit_raw2(ws->dev, acs->ctx->ctx, 0, num_chunks, chunks, &seq_no);
      } while (r == -ENOMEM);

      if (!r) {
         /* Success. */
         uint64_t *user_fence = NULL;

         /* Need to reserve 4 QWORD for user fence:
          *   QWORD[0]: completed fence
          *   QWORD[1]: preempted fence
          *   QWORD[2]: reset fence
          *   QWORD[3]: preempted then reset
          */
         if (has_user_fence)
            user_fence = acs->ctx->user_fence_cpu_address_base + acs->ip_type * 4;
         amdgpu_fence_submitted(cs->fence, seq_no, user_fence);
      }
   }

cleanup:
   if (unlikely(r)) {
      amdgpu_ctx_set_sw_reset_status((struct radeon_winsys_ctx*)acs->ctx,
                                     PIPE_GUILTY_CONTEXT_RESET,
                                     "amdgpu: The CS has been rejected (%i).\n", r);
   }

   /* If there was an error, signal the fence, because it won't be signalled
    * by the hardware. */
   if (r || noop)
      amdgpu_fence_signalled(cs->fence);

   if (unlikely(ws->info.has_fw_based_shadowing && acs->mcbp_fw_shadow_chunk.flags && r == 0))
      acs->mcbp_fw_shadow_chunk.flags = 0;

   cs->error_code = r;

   /* Only decrement num_active_ioctls for those buffers where we incremented it. */
   for (i = 0; i < initial_num_real_buffers; i++)
      p_atomic_dec(&cs->buffer_lists[AMDGPU_BO_REAL].buffers[i].bo->num_active_ioctls);

   unsigned num_slab_buffers = cs->buffer_lists[AMDGPU_BO_SLAB_ENTRY].num_buffers;
   for (i = 0; i < num_slab_buffers; i++)
      p_atomic_dec(&cs->buffer_lists[AMDGPU_BO_SLAB_ENTRY].buffers[i].bo->num_active_ioctls);

   unsigned num_sparse_buffers = cs->buffer_lists[AMDGPU_BO_SPARSE].num_buffers;
   for (i = 0; i < num_sparse_buffers; i++)
      p_atomic_dec(&cs->buffer_lists[AMDGPU_BO_SPARSE].buffers[i].bo->num_active_ioctls);

   amdgpu_cs_context_cleanup(ws, cs);
}

/* Make sure the previous submission is completed. */
void amdgpu_cs_sync_flush(struct radeon_cmdbuf *rcs)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);

   /* Wait for any pending ioctl of this CS to complete. */
   util_queue_fence_wait(&cs->flush_completed);
}

static int amdgpu_cs_flush(struct radeon_cmdbuf *rcs,
                           unsigned flags,
                           struct pipe_fence_handle **fence)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);
   struct amdgpu_winsys *ws = cs->ws;
   int error_code = 0;
   uint32_t ib_pad_dw_mask = ws->info.ip[cs->ip_type].ib_pad_dw_mask;

   rcs->current.max_dw += amdgpu_cs_epilog_dws(cs);

   /* Pad the IB according to the mask. */
   switch (cs->ip_type) {
   case AMD_IP_SDMA:
      if (ws->info.gfx_level <= GFX6) {
         while (rcs->current.cdw & ib_pad_dw_mask)
            radeon_emit(rcs, 0xf0000000); /* NOP packet */
      } else {
         while (rcs->current.cdw & ib_pad_dw_mask)
            radeon_emit(rcs, SDMA_NOP_PAD);
      }
      break;
   case AMD_IP_GFX:
   case AMD_IP_COMPUTE:
      amdgpu_pad_gfx_compute_ib(ws, cs->ip_type, rcs->current.buf, &rcs->current.cdw, 0);
      if (cs->ip_type == AMD_IP_GFX)
         ws->gfx_ib_size_counter += (rcs->prev_dw + rcs->current.cdw) * 4;
      break;
   case AMD_IP_UVD:
   case AMD_IP_UVD_ENC:
      while (rcs->current.cdw & ib_pad_dw_mask)
         radeon_emit(rcs, 0x80000000); /* type2 nop packet */
      break;
   case AMD_IP_VCN_JPEG:
      if (rcs->current.cdw % 2)
         assert(0);
      while (rcs->current.cdw & ib_pad_dw_mask) {
         radeon_emit(rcs, 0x60000000); /* nop packet */
         radeon_emit(rcs, 0x00000000);
      }
      break;
   case AMD_IP_VCN_DEC:
      while (rcs->current.cdw & ib_pad_dw_mask)
         radeon_emit(rcs, 0x81ff); /* nop packet */
      break;
   default:
      break;
   }

   if (rcs->current.cdw > rcs->current.max_dw) {
      fprintf(stderr, "amdgpu: command stream overflowed\n");
   }

   /* If the CS is not empty or overflowed.... */
   if (likely(radeon_emitted(rcs, 0) &&
       rcs->current.cdw <= rcs->current.max_dw &&
       !(flags & RADEON_FLUSH_NOOP))) {
      struct amdgpu_cs_context *cur = cs->csc;

      /* Set IB sizes. */
      amdgpu_ib_finalize(ws, rcs, &cs->main_ib, cs->ip_type);

      /* Create a fence. */
      amdgpu_fence_reference(&cur->fence, NULL);
      if (cs->next_fence) {
         /* just move the reference */
         cur->fence = cs->next_fence;
         cs->next_fence = NULL;
      } else {
         cur->fence = amdgpu_fence_create(cs);
      }
      if (fence)
         amdgpu_fence_reference(fence, cur->fence);

      for (unsigned i = 0; i < ARRAY_SIZE(cur->buffer_lists); i++) {
         unsigned num_buffers = cur->buffer_lists[i].num_buffers;
         struct amdgpu_cs_buffer *buffers = cur->buffer_lists[i].buffers;

         for (unsigned j = 0; j < num_buffers; j++)
            p_atomic_inc(&buffers[j].bo->num_active_ioctls);
      }

      amdgpu_cs_sync_flush(rcs);

      /* Swap command streams. "cst" is going to be submitted. */
      rcs->csc = cs->csc = cs->cst;
      cs->cst = cur;

      /* Submit. */
      util_queue_add_job(&ws->cs_queue, cs, &cs->flush_completed,
                         amdgpu_cs_submit_ib, NULL, 0);

      if (flags & RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION)
         cs->csc->secure = !cs->cst->secure;
      else
         cs->csc->secure = cs->cst->secure;

      if (!(flags & PIPE_FLUSH_ASYNC)) {
         amdgpu_cs_sync_flush(rcs);
         error_code = cur->error_code;
      }
   } else {
      if (flags & RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION)
         cs->csc->secure = !cs->csc->secure;
      amdgpu_cs_context_cleanup(ws, cs->csc);
   }

   memset(cs->csc->buffer_indices_hashlist, -1, sizeof(cs->buffer_indices_hashlist));

   amdgpu_get_new_ib(ws, rcs, &cs->main_ib, cs);

   if (cs->preamble_ib_bo) {
      amdgpu_cs_add_buffer(rcs, cs->preamble_ib_bo,
                           RADEON_USAGE_READ | RADEON_PRIO_IB, 0);
   }

   if (cs->ip_type == AMD_IP_GFX)
      ws->num_gfx_IBs++;
   else if (cs->ip_type == AMD_IP_SDMA)
      ws->num_sdma_IBs++;

   return error_code;
}

static void amdgpu_cs_destroy(struct radeon_cmdbuf *rcs)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);

   if (!cs)
      return;

   amdgpu_cs_sync_flush(rcs);
   util_queue_fence_destroy(&cs->flush_completed);
   p_atomic_dec(&cs->ws->num_cs);
   radeon_bo_reference(&cs->ws->dummy_ws.base, &cs->preamble_ib_bo, NULL);
   radeon_bo_reference(&cs->ws->dummy_ws.base, &cs->main_ib.big_buffer, NULL);
   FREE(rcs->prev);
   amdgpu_destroy_cs_context(cs->ws, &cs->csc1);
   amdgpu_destroy_cs_context(cs->ws, &cs->csc2);
   amdgpu_fence_reference(&cs->next_fence, NULL);
   FREE(cs);
}

static bool amdgpu_bo_is_referenced(struct radeon_cmdbuf *rcs,
                                    struct pb_buffer_lean *_buf,
                                    unsigned usage)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);
   struct amdgpu_winsys_bo *bo = (struct amdgpu_winsys_bo*)_buf;

   return amdgpu_bo_is_referenced_by_cs_with_usage(cs, bo, usage);
}

static void amdgpu_cs_set_mcbp_reg_shadowing_va(struct radeon_cmdbuf *rcs,uint64_t regs_va,
                                                                   uint64_t csa_va)
{
   struct amdgpu_cs *cs = amdgpu_cs(rcs);
   cs->mcbp_fw_shadow_chunk.shadow_va = regs_va;
   cs->mcbp_fw_shadow_chunk.csa_va = csa_va;
   cs->mcbp_fw_shadow_chunk.gds_va = 0;
   cs->mcbp_fw_shadow_chunk.flags = AMDGPU_CS_CHUNK_CP_GFX_SHADOW_FLAGS_INIT_SHADOW;
}

static void amdgpu_winsys_fence_reference(struct radeon_winsys *rws,
                                          struct pipe_fence_handle **dst,
                                          struct pipe_fence_handle *src)
{
   amdgpu_fence_reference(dst, src);
}

void amdgpu_cs_init_functions(struct amdgpu_screen_winsys *ws)
{
   ws->base.ctx_create = amdgpu_ctx_create;
   ws->base.ctx_destroy = amdgpu_ctx_destroy;
   ws->base.ctx_set_sw_reset_status = amdgpu_ctx_set_sw_reset_status;
   ws->base.ctx_query_reset_status = amdgpu_ctx_query_reset_status;
   ws->base.cs_create = amdgpu_cs_create;
   ws->base.cs_setup_preemption = amdgpu_cs_setup_preemption;
   ws->base.cs_destroy = amdgpu_cs_destroy;
   ws->base.cs_add_buffer = amdgpu_cs_add_buffer;
   ws->base.cs_validate = amdgpu_cs_validate;
   ws->base.cs_check_space = amdgpu_cs_check_space;
   ws->base.cs_get_buffer_list = amdgpu_cs_get_buffer_list;
   ws->base.cs_flush = amdgpu_cs_flush;
   ws->base.cs_get_next_fence = amdgpu_cs_get_next_fence;
   ws->base.cs_is_buffer_referenced = amdgpu_bo_is_referenced;
   ws->base.cs_sync_flush = amdgpu_cs_sync_flush;
   ws->base.cs_add_fence_dependency = amdgpu_cs_add_fence_dependency;
   ws->base.cs_add_syncobj_signal = amdgpu_cs_add_syncobj_signal;
   ws->base.cs_get_ip_type = amdgpu_cs_get_ip_type;
   ws->base.fence_wait = amdgpu_fence_wait_rel_timeout;
   ws->base.fence_reference = amdgpu_winsys_fence_reference;
   ws->base.fence_import_syncobj = amdgpu_fence_import_syncobj;
   ws->base.fence_import_sync_file = amdgpu_fence_import_sync_file;
   ws->base.fence_export_sync_file = amdgpu_fence_export_sync_file;
   ws->base.export_signalled_sync_file = amdgpu_export_signalled_sync_file;

   if (ws->aws->info.has_fw_based_shadowing)
      ws->base.cs_set_mcbp_reg_shadowing_va = amdgpu_cs_set_mcbp_reg_shadowing_va;
}
