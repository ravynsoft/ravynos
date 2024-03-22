/*
 * Copyright 2021 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include "vn_ring.h"

#include "venus-protocol/vn_protocol_driver_transport.h"

#include "vn_cs.h"
#include "vn_instance.h"
#include "vn_renderer.h"

static_assert(ATOMIC_INT_LOCK_FREE == 2 && sizeof(atomic_uint) == 4,
              "vn_ring_shared requires lock-free 32-bit atomic_uint");

/* pointers to a ring in a BO */
struct vn_ring_shared {
   const volatile atomic_uint *head;
   volatile atomic_uint *tail;
   volatile atomic_uint *status;
   void *buffer;
   void *extra;
};

struct vn_ring {
   uint64_t id;
   struct vn_instance *instance;
   struct vn_renderer_shmem *shmem;

   uint32_t buffer_size;
   uint32_t buffer_mask;

   struct vn_ring_shared shared;
   uint32_t cur;

   /* This mutex ensures below:
    * - atomic of ring submission
    * - reply shmem resource set and ring submission are paired
    */
   mtx_t mutex;

   /* used for indirect submission of large command (non-VkCommandBuffer) */
   struct vn_cs_encoder upload;

   struct list_head submits;
   struct list_head free_submits;
};

struct vn_ring_submit {
   uint32_t seqno;

   struct list_head head;

   /* BOs to keep alive (TODO make sure shmems are pinned) */
   uint32_t shmem_count;
   struct vn_renderer_shmem *shmems[];
};

struct vn_ring_submission {
   const struct vn_cs_encoder *cs;
   struct vn_ring_submit *submit;

   struct {
      struct vn_cs_encoder cs;
      struct vn_cs_encoder_buffer buffer;
      uint32_t data[64];
   } indirect;
};

static uint32_t
vn_ring_load_head(const struct vn_ring *ring)
{
   /* the renderer is expected to store the head with memory_order_release,
    * forming a release-acquire ordering
    */
   return atomic_load_explicit(ring->shared.head, memory_order_acquire);
}

static void
vn_ring_store_tail(struct vn_ring *ring)
{
   /* the renderer is expected to load the tail with memory_order_acquire,
    * forming a release-acquire ordering
    */
   return atomic_store_explicit(ring->shared.tail, ring->cur,
                                memory_order_release);
}

uint32_t
vn_ring_load_status(const struct vn_ring *ring)
{
   /* must be called and ordered after vn_ring_store_tail for idle status */
   return atomic_load_explicit(ring->shared.status, memory_order_seq_cst);
}

void
vn_ring_unset_status_bits(struct vn_ring *ring, uint32_t mask)
{
   atomic_fetch_and_explicit(ring->shared.status, ~mask,
                             memory_order_seq_cst);
}

static void
vn_ring_write_buffer(struct vn_ring *ring, const void *data, uint32_t size)
{
   assert(ring->cur + size - vn_ring_load_head(ring) <= ring->buffer_size);

   const uint32_t offset = ring->cur & ring->buffer_mask;
   if (offset + size <= ring->buffer_size) {
      memcpy(ring->shared.buffer + offset, data, size);
   } else {
      const uint32_t s = ring->buffer_size - offset;
      memcpy(ring->shared.buffer + offset, data, s);
      memcpy(ring->shared.buffer, data + s, size - s);
   }

   ring->cur += size;
}

static bool
vn_ring_ge_seqno(const struct vn_ring *ring, uint32_t a, uint32_t b)
{
   /* this can return false negative when not called fast enough (e.g., when
    * called once every couple hours), but following calls with larger a's
    * will correct itself
    *
    * TODO use real seqnos?
    */
   if (a >= b)
      return ring->cur >= a || ring->cur < b;
   else
      return ring->cur >= a && ring->cur < b;
}

static void
vn_ring_retire_submits(struct vn_ring *ring, uint32_t seqno)
{
   struct vn_renderer *renderer = ring->instance->renderer;
   list_for_each_entry_safe(struct vn_ring_submit, submit, &ring->submits,
                            head) {
      if (!vn_ring_ge_seqno(ring, seqno, submit->seqno))
         break;

      for (uint32_t i = 0; i < submit->shmem_count; i++)
         vn_renderer_shmem_unref(renderer, submit->shmems[i]);

      list_move_to(&submit->head, &ring->free_submits);
   }
}

bool
vn_ring_get_seqno_status(struct vn_ring *ring, uint32_t seqno)
{
   return vn_ring_ge_seqno(ring, vn_ring_load_head(ring), seqno);
}

static void
vn_ring_wait_seqno(struct vn_ring *ring, uint32_t seqno)
{
   /* A renderer wait incurs several hops and the renderer might poll
    * repeatedly anyway.  Let's just poll here.
    */
   struct vn_relax_state relax_state =
      vn_relax_init(ring->instance, "ring seqno");
   do {
      if (vn_ring_get_seqno_status(ring, seqno)) {
         vn_relax_fini(&relax_state);
         return;
      }
      vn_relax(&relax_state);
   } while (true);
}

void
vn_ring_wait_all(struct vn_ring *ring)
{
   /* load from tail rather than ring->cur for atomicity */
   const uint32_t pending_seqno =
      atomic_load_explicit(ring->shared.tail, memory_order_relaxed);
   vn_ring_wait_seqno(ring, pending_seqno);
}

static bool
vn_ring_has_space(const struct vn_ring *ring,
                  uint32_t size,
                  uint32_t *out_head)
{
   const uint32_t head = vn_ring_load_head(ring);
   if (likely(ring->cur + size - head <= ring->buffer_size)) {
      *out_head = head;
      return true;
   }

   return false;
}

static uint32_t
vn_ring_wait_space(struct vn_ring *ring, uint32_t size)
{
   assert(size <= ring->buffer_size);

   uint32_t head;
   if (likely(vn_ring_has_space(ring, size, &head)))
      return head;

   {
      VN_TRACE_FUNC();

      /* see the reasoning in vn_ring_wait_seqno */
      struct vn_relax_state relax_state =
         vn_relax_init(ring->instance, "ring space");
      do {
         vn_relax(&relax_state);
         if (vn_ring_has_space(ring, size, &head)) {
            vn_relax_fini(&relax_state);
            return head;
         }
      } while (true);
   }
}

void
vn_ring_get_layout(size_t buf_size,
                   size_t extra_size,
                   struct vn_ring_layout *layout)
{
   /* this can be changed/extended quite freely */
   struct layout {
      alignas(64) uint32_t head;
      alignas(64) uint32_t tail;
      alignas(64) uint32_t status;

      alignas(64) uint8_t buffer[];
   };

   assert(buf_size && util_is_power_of_two_or_zero(buf_size));

   layout->head_offset = offsetof(struct layout, head);
   layout->tail_offset = offsetof(struct layout, tail);
   layout->status_offset = offsetof(struct layout, status);

   layout->buffer_offset = offsetof(struct layout, buffer);
   layout->buffer_size = buf_size;

   layout->extra_offset = layout->buffer_offset + layout->buffer_size;
   layout->extra_size = extra_size;

   layout->shmem_size = layout->extra_offset + layout->extra_size;
}

struct vn_ring *
vn_ring_create(struct vn_instance *instance,
               const struct vn_ring_layout *layout)
{
   VN_TRACE_FUNC();

   const VkAllocationCallbacks *alloc = &instance->base.base.alloc;

   struct vn_ring *ring = vk_zalloc(alloc, sizeof(*ring), VN_DEFAULT_ALIGN,
                                    VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   if (!ring)
      return NULL;

   ring->id = (uintptr_t)ring;
   ring->instance = instance;
   ring->shmem =
      vn_renderer_shmem_create(instance->renderer, layout->shmem_size);
   if (!ring->shmem) {
      if (VN_DEBUG(INIT))
         vn_log(instance, "failed to allocate/map ring shmem");
      vk_free(alloc, ring);
      return NULL;
   }

   void *shared = ring->shmem->mmap_ptr;
   memset(shared, 0, layout->shmem_size);

   assert(layout->buffer_size &&
          util_is_power_of_two_or_zero(layout->buffer_size));
   ring->buffer_size = layout->buffer_size;
   ring->buffer_mask = ring->buffer_size - 1;

   ring->shared.head = shared + layout->head_offset;
   ring->shared.tail = shared + layout->tail_offset;
   ring->shared.status = shared + layout->status_offset;
   ring->shared.buffer = shared + layout->buffer_offset;
   ring->shared.extra = shared + layout->extra_offset;

   mtx_init(&ring->mutex, mtx_plain);

   vn_cs_encoder_init(&ring->upload, instance,
                      VN_CS_ENCODER_STORAGE_SHMEM_ARRAY, 1 * 1024 * 1024);

   list_inithead(&ring->submits);
   list_inithead(&ring->free_submits);

   const struct VkRingMonitorInfoMESA monitor_info = {
      .sType = VK_STRUCTURE_TYPE_RING_MONITOR_INFO_MESA,
      .maxReportingPeriodMicroseconds = VN_WATCHDOG_REPORT_PERIOD_US,
   };
   const struct VkRingCreateInfoMESA info = {
      .sType = VK_STRUCTURE_TYPE_RING_CREATE_INFO_MESA,
      .pNext = &monitor_info,
      .resourceId = ring->shmem->res_id,
      .size = layout->shmem_size,
      .idleTimeout = 5ull * 1000 * 1000,
      .headOffset = layout->head_offset,
      .tailOffset = layout->tail_offset,
      .statusOffset = layout->status_offset,
      .bufferOffset = layout->buffer_offset,
      .bufferSize = layout->buffer_size,
      .extraOffset = layout->extra_offset,
      .extraSize = layout->extra_size,
   };

   uint32_t create_ring_data[64];
   struct vn_cs_encoder local_enc = VN_CS_ENCODER_INITIALIZER_LOCAL(
      create_ring_data, sizeof(create_ring_data));
   vn_encode_vkCreateRingMESA(&local_enc, 0, ring->id, &info);
   vn_renderer_submit_simple(instance->renderer, create_ring_data,
                             vn_cs_encoder_get_len(&local_enc));

   return ring;
}

void
vn_ring_destroy(struct vn_ring *ring)
{
   VN_TRACE_FUNC();

   const VkAllocationCallbacks *alloc = &ring->instance->base.base.alloc;

   uint32_t destroy_ring_data[4];
   struct vn_cs_encoder local_enc = VN_CS_ENCODER_INITIALIZER_LOCAL(
      destroy_ring_data, sizeof(destroy_ring_data));
   vn_encode_vkDestroyRingMESA(&local_enc, 0, ring->id);
   vn_renderer_submit_simple(ring->instance->renderer, destroy_ring_data,
                             vn_cs_encoder_get_len(&local_enc));

   vn_ring_retire_submits(ring, ring->cur);
   assert(list_is_empty(&ring->submits));

   list_for_each_entry_safe(struct vn_ring_submit, submit,
                            &ring->free_submits, head)
      vk_free(alloc, submit);

   vn_cs_encoder_fini(&ring->upload);
   vn_renderer_shmem_unref(ring->instance->renderer, ring->shmem);

   mtx_destroy(&ring->mutex);

   vk_free(alloc, ring);
}

uint64_t
vn_ring_get_id(struct vn_ring *ring)
{
   return ring->id;
}

static struct vn_ring_submit *
vn_ring_get_submit(struct vn_ring *ring, uint32_t shmem_count)
{
   const VkAllocationCallbacks *alloc = &ring->instance->base.base.alloc;
   const uint32_t min_shmem_count = 2;
   struct vn_ring_submit *submit;

   /* TODO this could be simplified if we could omit shmem_count */
   if (shmem_count <= min_shmem_count &&
       !list_is_empty(&ring->free_submits)) {
      submit =
         list_first_entry(&ring->free_submits, struct vn_ring_submit, head);
      list_del(&submit->head);
   } else {
      const size_t submit_size = offsetof(
         struct vn_ring_submit, shmems[MAX2(shmem_count, min_shmem_count)]);
      submit = vk_alloc(alloc, submit_size, VN_DEFAULT_ALIGN,
                        VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
   }

   return submit;
}

static bool
vn_ring_submit_internal(struct vn_ring *ring,
                        struct vn_ring_submit *submit,
                        const struct vn_cs_encoder *cs,
                        uint32_t *seqno)
{
   /* write cs to the ring */
   assert(!vn_cs_encoder_is_empty(cs));

   /* avoid -Wmaybe-unitialized */
   uint32_t cur_seqno = 0;

   for (uint32_t i = 0; i < cs->buffer_count; i++) {
      const struct vn_cs_encoder_buffer *buf = &cs->buffers[i];
      cur_seqno = vn_ring_wait_space(ring, buf->committed_size);
      vn_ring_write_buffer(ring, buf->base, buf->committed_size);
   }

   vn_ring_store_tail(ring);
   const VkRingStatusFlagsMESA status = vn_ring_load_status(ring);
   if (status & VK_RING_STATUS_FATAL_BIT_MESA) {
      vn_log(NULL, "vn_ring_submit abort on fatal");
      abort();
   }

   vn_ring_retire_submits(ring, cur_seqno);

   submit->seqno = ring->cur;
   list_addtail(&submit->head, &ring->submits);

   *seqno = submit->seqno;

   /* notify renderer to wake up ring if idle */
   return status & VK_RING_STATUS_IDLE_BIT_MESA;
}

static const struct vn_cs_encoder *
vn_ring_submission_get_cs(struct vn_ring *ring,
                          struct vn_ring_submission *submit,
                          const struct vn_cs_encoder *cs,
                          bool direct)
{
   if (direct)
      return cs;

   STACK_ARRAY(VkCommandStreamDescriptionMESA, descs, cs->buffer_count);

   uint32_t desc_count = 0;
   for (uint32_t i = 0; i < cs->buffer_count; i++) {
      const struct vn_cs_encoder_buffer *buf = &cs->buffers[i];
      if (buf->committed_size) {
         descs[desc_count++] = (VkCommandStreamDescriptionMESA){
            .resourceId = buf->shmem->res_id,
            .offset = buf->offset,
            .size = buf->committed_size,
         };
      }
   }

   const size_t exec_size = vn_sizeof_vkExecuteCommandStreamsMESA(
      desc_count, descs, NULL, 0, NULL, 0);
   void *exec_data = submit->indirect.data;
   if (exec_size > sizeof(submit->indirect.data)) {
      const VkAllocationCallbacks *alloc = &ring->instance->base.base.alloc;
      exec_data = vk_alloc(alloc, exec_size, VN_DEFAULT_ALIGN,
                           VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
      if (!exec_data) {
         STACK_ARRAY_FINISH(descs);
         return NULL;
      }
   }

   submit->indirect.buffer = VN_CS_ENCODER_BUFFER_INITIALIZER(exec_data);
   submit->indirect.cs =
      VN_CS_ENCODER_INITIALIZER(&submit->indirect.buffer, exec_size);
   vn_encode_vkExecuteCommandStreamsMESA(&submit->indirect.cs, 0, desc_count,
                                         descs, NULL, 0, NULL, 0);
   vn_cs_encoder_commit(&submit->indirect.cs);

   STACK_ARRAY_FINISH(descs);

   return &submit->indirect.cs;
}

static struct vn_ring_submit *
vn_ring_submission_get_ring_submit(struct vn_ring *ring,
                                   const struct vn_cs_encoder *cs,
                                   struct vn_renderer_shmem *extra_shmem,
                                   bool direct)
{
   struct vn_renderer *renderer = ring->instance->renderer;
   const uint32_t shmem_count =
      (direct ? 0 : cs->buffer_count) + (extra_shmem ? 1 : 0);
   struct vn_ring_submit *submit = vn_ring_get_submit(ring, shmem_count);
   if (!submit)
      return NULL;

   submit->shmem_count = shmem_count;
   if (!direct) {
      for (uint32_t i = 0; i < cs->buffer_count; i++) {
         submit->shmems[i] =
            vn_renderer_shmem_ref(renderer, cs->buffers[i].shmem);
      }
   }
   if (extra_shmem) {
      submit->shmems[shmem_count - 1] =
         vn_renderer_shmem_ref(renderer, extra_shmem);
   }

   return submit;
}

static inline void
vn_ring_submission_cleanup(struct vn_ring *ring,
                           struct vn_ring_submission *submit)
{
   const VkAllocationCallbacks *alloc = &ring->instance->base.base.alloc;
   if (submit->cs == &submit->indirect.cs &&
       submit->indirect.buffer.base != submit->indirect.data)
      vk_free(alloc, submit->indirect.buffer.base);
}

static VkResult
vn_ring_submission_prepare(struct vn_ring *ring,
                           struct vn_ring_submission *submit,
                           const struct vn_cs_encoder *cs,
                           struct vn_renderer_shmem *extra_shmem,
                           bool direct)
{
   submit->cs = vn_ring_submission_get_cs(ring, submit, cs, direct);
   if (!submit->cs)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   submit->submit =
      vn_ring_submission_get_ring_submit(ring, cs, extra_shmem, direct);
   if (!submit->submit) {
      vn_ring_submission_cleanup(ring, submit);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   return VK_SUCCESS;
}

static inline bool
vn_ring_submission_can_direct(const struct vn_ring *ring,
                              const struct vn_cs_encoder *cs)
{
   return vn_cs_encoder_get_len(cs) <= (ring->buffer_size >> 4);
}

static struct vn_cs_encoder *
vn_ring_cs_upload_locked(struct vn_ring *ring, const struct vn_cs_encoder *cs)
{
   VN_TRACE_FUNC();
   assert(cs->storage_type == VN_CS_ENCODER_STORAGE_POINTER &&
          cs->buffer_count == 1);
   const void *cs_data = cs->buffers[0].base;
   const size_t cs_size = cs->total_committed_size;
   assert(cs_size == vn_cs_encoder_get_len(cs));

   struct vn_cs_encoder *upload = &ring->upload;
   vn_cs_encoder_reset(upload);

   if (!vn_cs_encoder_reserve(upload, cs_size))
      return NULL;

   vn_cs_encoder_write(upload, cs_size, cs_data, cs_size);
   vn_cs_encoder_commit(upload);

   return upload;
}

static VkResult
vn_ring_submit_locked(struct vn_ring *ring,
                      const struct vn_cs_encoder *cs,
                      struct vn_renderer_shmem *extra_shmem,
                      uint32_t *ring_seqno)
{
   const bool direct = vn_ring_submission_can_direct(ring, cs);
   if (!direct && cs->storage_type == VN_CS_ENCODER_STORAGE_POINTER) {
      cs = vn_ring_cs_upload_locked(ring, cs);
      if (!cs)
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      assert(cs->storage_type != VN_CS_ENCODER_STORAGE_POINTER);
   }

   struct vn_ring_submission submit;
   VkResult result =
      vn_ring_submission_prepare(ring, &submit, cs, extra_shmem, direct);
   if (result != VK_SUCCESS)
      return result;

   uint32_t seqno;
   const bool notify =
      vn_ring_submit_internal(ring, submit.submit, submit.cs, &seqno);
   if (notify) {
      uint32_t notify_ring_data[8];
      struct vn_cs_encoder local_enc = VN_CS_ENCODER_INITIALIZER_LOCAL(
         notify_ring_data, sizeof(notify_ring_data));
      vn_encode_vkNotifyRingMESA(&local_enc, 0, ring->id, seqno, 0);
      vn_renderer_submit_simple(ring->instance->renderer, notify_ring_data,
                                vn_cs_encoder_get_len(&local_enc));
   }

   vn_ring_submission_cleanup(ring, &submit);

   if (ring_seqno)
      *ring_seqno = seqno;

   return VK_SUCCESS;
}

VkResult
vn_ring_submit_command_simple(struct vn_ring *ring,
                              const struct vn_cs_encoder *cs)
{
   mtx_lock(&ring->mutex);
   VkResult result = vn_ring_submit_locked(ring, cs, NULL, NULL);
   mtx_unlock(&ring->mutex);

   return result;
}

static inline void
vn_ring_set_reply_shmem_locked(struct vn_ring *ring,
                               struct vn_renderer_shmem *shmem,
                               size_t offset,
                               size_t size)
{

   uint32_t set_reply_command_stream_data[16];
   struct vn_cs_encoder local_enc = VN_CS_ENCODER_INITIALIZER_LOCAL(
      set_reply_command_stream_data, sizeof(set_reply_command_stream_data));
   const struct VkCommandStreamDescriptionMESA stream = {
      .resourceId = shmem->res_id,
      .offset = offset,
      .size = size,
   };
   vn_encode_vkSetReplyCommandStreamMESA(&local_enc, 0, &stream);
   vn_cs_encoder_commit(&local_enc);
   vn_ring_submit_locked(ring, &local_enc, NULL, NULL);
}

void
vn_ring_submit_command(struct vn_ring *ring,
                       struct vn_ring_submit_command *submit)
{
   assert(!vn_cs_encoder_is_empty(&submit->command));

   vn_cs_encoder_commit(&submit->command);

   size_t reply_offset = 0;
   if (submit->reply_size) {
      submit->reply_shmem = vn_instance_reply_shmem_alloc(
         ring->instance, submit->reply_size, &reply_offset);
      if (!submit->reply_shmem)
         return;
   }

   mtx_lock(&ring->mutex);
   if (submit->reply_size) {
      vn_ring_set_reply_shmem_locked(ring, submit->reply_shmem, reply_offset,
                                     submit->reply_size);
   }
   submit->ring_seqno_valid =
      VK_SUCCESS == vn_ring_submit_locked(ring, &submit->command,
                                          submit->reply_shmem,
                                          &submit->ring_seqno);
   mtx_unlock(&ring->mutex);

   if (submit->reply_size) {
      if (likely(submit->ring_seqno_valid)) {
         void *reply_ptr = submit->reply_shmem->mmap_ptr + reply_offset;
         submit->reply =
            VN_CS_DECODER_INITIALIZER(reply_ptr, submit->reply_size);
         vn_ring_wait_seqno(ring, submit->ring_seqno);
      } else {
         vn_renderer_shmem_unref(ring->instance->renderer,
                                 submit->reply_shmem);
         submit->reply_shmem = NULL;
      }
   }
}

void
vn_ring_free_command_reply(struct vn_ring *ring,
                           struct vn_ring_submit_command *submit)
{
   assert(submit->reply_shmem);
   vn_renderer_shmem_unref(ring->instance->renderer, submit->reply_shmem);
}
