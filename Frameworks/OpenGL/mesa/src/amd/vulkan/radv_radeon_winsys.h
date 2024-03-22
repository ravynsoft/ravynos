/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * Based on radeon_winsys.h which is:
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 * Copyright 2010 Marek Olšák <maraeo@gmail.com>
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef RADV_RADEON_WINSYS_H
#define RADV_RADEON_WINSYS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/u_math.h"
#include "util/u_memory.h"
#include <vulkan/vulkan.h>
#include "amd_family.h"

struct radeon_info;
struct ac_surf_info;
struct radeon_surf;
struct vk_sync_type;
struct vk_sync_wait;
struct vk_sync_signal;

enum radeon_bo_domain { /* bitfield */
                        RADEON_DOMAIN_GTT = 2,
                        RADEON_DOMAIN_VRAM = 4,
                        RADEON_DOMAIN_VRAM_GTT = RADEON_DOMAIN_VRAM | RADEON_DOMAIN_GTT,
                        RADEON_DOMAIN_GDS = 8,
                        RADEON_DOMAIN_OA = 16,
};

enum radeon_bo_flag { /* bitfield */
                      RADEON_FLAG_GTT_WC = (1 << 0),
                      RADEON_FLAG_CPU_ACCESS = (1 << 1),
                      RADEON_FLAG_NO_CPU_ACCESS = (1 << 2),
                      RADEON_FLAG_VIRTUAL = (1 << 3),
                      RADEON_FLAG_VA_UNCACHED = (1 << 4),
                      RADEON_FLAG_IMPLICIT_SYNC = (1 << 5),
                      RADEON_FLAG_NO_INTERPROCESS_SHARING = (1 << 6),
                      RADEON_FLAG_READ_ONLY = (1 << 7),
                      RADEON_FLAG_32BIT = (1 << 8),
                      RADEON_FLAG_PREFER_LOCAL_BO = (1 << 9),
                      RADEON_FLAG_ZERO_VRAM = (1 << 10),
                      RADEON_FLAG_REPLAYABLE = (1 << 11),
                      RADEON_FLAG_DISCARDABLE = (1 << 12),
};

enum radeon_ctx_priority {
   RADEON_CTX_PRIORITY_INVALID = -1,
   RADEON_CTX_PRIORITY_LOW = 0,
   RADEON_CTX_PRIORITY_MEDIUM,
   RADEON_CTX_PRIORITY_HIGH,
   RADEON_CTX_PRIORITY_REALTIME,
};

enum radeon_ctx_pstate {
   RADEON_CTX_PSTATE_NONE = 0,
   RADEON_CTX_PSTATE_STANDARD,
   RADEON_CTX_PSTATE_MIN_SCLK,
   RADEON_CTX_PSTATE_MIN_MCLK,
   RADEON_CTX_PSTATE_PEAK,
};

enum radeon_value_id {
   RADEON_ALLOCATED_VRAM,
   RADEON_ALLOCATED_VRAM_VIS,
   RADEON_ALLOCATED_GTT,
   RADEON_TIMESTAMP,
   RADEON_NUM_BYTES_MOVED,
   RADEON_NUM_EVICTIONS,
   RADEON_NUM_VRAM_CPU_PAGE_FAULTS,
   RADEON_VRAM_USAGE,
   RADEON_VRAM_VIS_USAGE,
   RADEON_GTT_USAGE,
   RADEON_GPU_TEMPERATURE,
   RADEON_CURRENT_SCLK,
   RADEON_CURRENT_MCLK,
};

enum radv_reset_status {
   RADV_NO_RESET,
   RADV_GUILTY_CONTEXT_RESET,
   RADV_INNOCENT_CONTEXT_RESET,
};

struct radeon_cmdbuf {
   /* These are uint64_t to tell the compiler that buf can't alias them.
    * If they're uint32_t the generated code needs to redundantly
    * store and reload them between buf writes. */
   uint64_t cdw;         /* Number of used dwords. */
   uint64_t max_dw;      /* Maximum number of dwords. */
   uint64_t reserved_dw; /* Number of dwords reserved through radeon_check_space() */
   uint32_t *buf;        /* The base pointer of the chunk. */
};

#define RADEON_SURF_TYPE_MASK     0xFF
#define RADEON_SURF_TYPE_SHIFT    0
#define RADEON_SURF_TYPE_1D       0
#define RADEON_SURF_TYPE_2D       1
#define RADEON_SURF_TYPE_3D       2
#define RADEON_SURF_TYPE_CUBEMAP  3
#define RADEON_SURF_TYPE_1D_ARRAY 4
#define RADEON_SURF_TYPE_2D_ARRAY 5
#define RADEON_SURF_MODE_MASK     0xFF
#define RADEON_SURF_MODE_SHIFT    8

#define RADEON_SURF_GET(v, field) (((v) >> RADEON_SURF_##field##_SHIFT) & RADEON_SURF_##field##_MASK)
#define RADEON_SURF_SET(v, field) (((v)&RADEON_SURF_##field##_MASK) << RADEON_SURF_##field##_SHIFT)
#define RADEON_SURF_CLR(v, field) ((v) & ~(RADEON_SURF_##field##_MASK << RADEON_SURF_##field##_SHIFT))

enum radeon_bo_layout {
   RADEON_LAYOUT_LINEAR = 0,
   RADEON_LAYOUT_TILED,
   RADEON_LAYOUT_SQUARETILED,

   RADEON_LAYOUT_UNKNOWN
};

/* Tiling info for display code, DRI sharing, and other data. */
struct radeon_bo_metadata {
   /* Tiling flags describing the texture layout for display code
    * and DRI sharing.
    */
   union {
      struct {
         enum radeon_bo_layout microtile;
         enum radeon_bo_layout macrotile;
         unsigned pipe_config;
         unsigned bankw;
         unsigned bankh;
         unsigned tile_split;
         unsigned mtilea;
         unsigned num_banks;
         unsigned stride;
         bool scanout;
      } legacy;

      struct {
         /* surface flags */
         unsigned swizzle_mode : 5;
         bool scanout;
         uint32_t dcc_offset_256b;
         uint32_t dcc_pitch_max;
         bool dcc_independent_64b_blocks;
         bool dcc_independent_128b_blocks;
         unsigned dcc_max_compressed_block_size;
      } gfx9;
   } u;

   /* Additional metadata associated with the buffer, in bytes.
    * The maximum size is 64 * 4. This is opaque for the winsys & kernel.
    * Supported by amdgpu only.
    */
   uint32_t size_metadata;
   uint32_t metadata[64];
};

struct radeon_winsys_ctx;

struct radeon_winsys_bo {
   uint64_t va;
   /* buffer is created with AMDGPU_GEM_CREATE_VM_ALWAYS_VALID */
   bool is_local;
   bool vram_no_cpu_access;
   /* buffer is added to the BO list of all submissions */
   bool use_global_list;
   enum radeon_bo_domain initial_domain;
};

struct radv_winsys_submit_info {
   enum amd_ip_type ip_type;
   int queue_index;
   unsigned cs_count;
   unsigned initial_preamble_count;
   unsigned continue_preamble_count;
   unsigned postamble_count;
   struct radeon_cmdbuf **cs_array;
   struct radeon_cmdbuf **initial_preamble_cs;
   struct radeon_cmdbuf **continue_preamble_cs;
   struct radeon_cmdbuf **postamble_cs;
   bool uses_shadow_regs;
};

/* Kernel effectively allows 0-31. This sets some priorities for fixed
 * functionality buffers */
enum {
   RADV_BO_PRIORITY_APPLICATION_MAX = 28,

   /* virtual buffers have 0 priority since the priority is not used. */
   RADV_BO_PRIORITY_VIRTUAL = 0,

   RADV_BO_PRIORITY_METADATA = 10,
   /* This should be considerably lower than most of the stuff below,
    * but how much lower is hard to say since we don't know application
    * assignments. Put it pretty high since it is GTT anyway. */
   RADV_BO_PRIORITY_QUERY_POOL = 29,

   RADV_BO_PRIORITY_DESCRIPTOR = 30,
   RADV_BO_PRIORITY_UPLOAD_BUFFER = 30,
   RADV_BO_PRIORITY_FENCE = 30,
   RADV_BO_PRIORITY_SHADER = 31,
   RADV_BO_PRIORITY_SCRATCH = 31,
   RADV_BO_PRIORITY_CS = 31,
};

struct radv_winsys_gpuvm_fault_info {
   uint64_t addr;
   uint32_t status;
   uint32_t vmhub;
};

struct radeon_winsys {
   void (*destroy)(struct radeon_winsys *ws);

   void (*query_info)(struct radeon_winsys *ws, struct radeon_info *info);

   uint64_t (*query_value)(struct radeon_winsys *ws, enum radeon_value_id value);

   bool (*read_registers)(struct radeon_winsys *ws, unsigned reg_offset, unsigned num_registers, uint32_t *out);

   const char *(*get_chip_name)(struct radeon_winsys *ws);

   bool (*query_gpuvm_fault)(struct radeon_winsys *ws, struct radv_winsys_gpuvm_fault_info *fault_info);

   VkResult (*buffer_create)(struct radeon_winsys *ws, uint64_t size, unsigned alignment, enum radeon_bo_domain domain,
                             enum radeon_bo_flag flags, unsigned priority, uint64_t address,
                             struct radeon_winsys_bo **out_bo);

   void (*buffer_destroy)(struct radeon_winsys *ws, struct radeon_winsys_bo *bo);
   void *(*buffer_map)(struct radeon_winsys_bo *bo);

   VkResult (*buffer_from_ptr)(struct radeon_winsys *ws, void *pointer, uint64_t size, unsigned priority,
                               struct radeon_winsys_bo **out_bo);

   VkResult (*buffer_from_fd)(struct radeon_winsys *ws, int fd, unsigned priority, struct radeon_winsys_bo **out_bo,
                              uint64_t *alloc_size);

   bool (*buffer_get_fd)(struct radeon_winsys *ws, struct radeon_winsys_bo *bo, int *fd);

   bool (*buffer_get_flags_from_fd)(struct radeon_winsys *ws, int fd, enum radeon_bo_domain *domains,
                                    enum radeon_bo_flag *flags);

   void (*buffer_unmap)(struct radeon_winsys_bo *bo);

   void (*buffer_set_metadata)(struct radeon_winsys *ws, struct radeon_winsys_bo *bo, struct radeon_bo_metadata *md);
   void (*buffer_get_metadata)(struct radeon_winsys *ws, struct radeon_winsys_bo *bo, struct radeon_bo_metadata *md);

   VkResult (*buffer_virtual_bind)(struct radeon_winsys *ws, struct radeon_winsys_bo *parent, uint64_t offset,
                                   uint64_t size, struct radeon_winsys_bo *bo, uint64_t bo_offset);

   VkResult (*buffer_make_resident)(struct radeon_winsys *ws, struct radeon_winsys_bo *bo, bool resident);

   VkResult (*ctx_create)(struct radeon_winsys *ws, enum radeon_ctx_priority priority, struct radeon_winsys_ctx **ctx);
   void (*ctx_destroy)(struct radeon_winsys_ctx *ctx);

   bool (*ctx_wait_idle)(struct radeon_winsys_ctx *ctx, enum amd_ip_type amd_ip_type, int ring_index);

   int (*ctx_set_pstate)(struct radeon_winsys_ctx *ctx, uint32_t pstate);

   enum radv_reset_status (*ctx_query_reset_status)(struct radeon_winsys_ctx *rwctx);

   enum radeon_bo_domain (*cs_domain)(const struct radeon_winsys *ws);

   struct radeon_cmdbuf *(*cs_create)(struct radeon_winsys *ws, enum amd_ip_type amd_ip_type, bool is_secondary);

   void (*cs_destroy)(struct radeon_cmdbuf *cs);

   void (*cs_reset)(struct radeon_cmdbuf *cs);

   bool (*cs_chain)(struct radeon_cmdbuf *cs, struct radeon_cmdbuf *next_cs, bool pre_en);

   void (*cs_unchain)(struct radeon_cmdbuf *cs);

   VkResult (*cs_finalize)(struct radeon_cmdbuf *cs);

   void (*cs_grow)(struct radeon_cmdbuf *cs, size_t min_size);

   VkResult (*cs_submit)(struct radeon_winsys_ctx *ctx, const struct radv_winsys_submit_info *submit,
                         uint32_t wait_count, const struct vk_sync_wait *waits, uint32_t signal_count,
                         const struct vk_sync_signal *signals);

   void (*cs_add_buffer)(struct radeon_cmdbuf *cs, struct radeon_winsys_bo *bo);

   void (*cs_execute_secondary)(struct radeon_cmdbuf *parent, struct radeon_cmdbuf *child, bool allow_ib2);

   void (*cs_execute_ib)(struct radeon_cmdbuf *cs, struct radeon_winsys_bo *bo, const uint64_t offset,
                         const uint32_t cdw, const bool predicate);

   void (*cs_dump)(struct radeon_cmdbuf *cs, FILE *file, const int *trace_ids, int trace_id_count);

   void (*dump_bo_ranges)(struct radeon_winsys *ws, FILE *file);

   void (*dump_bo_log)(struct radeon_winsys *ws, FILE *file);

   int (*surface_init)(struct radeon_winsys *ws, const struct ac_surf_info *surf_info, struct radeon_surf *surf);

   int (*get_fd)(struct radeon_winsys *ws);

   struct ac_addrlib *(*get_addrlib)(struct radeon_winsys *ws);

   const struct vk_sync_type *const *(*get_sync_types)(struct radeon_winsys *ws);
};

static inline void
radeon_emit(struct radeon_cmdbuf *cs, uint32_t value)
{
   assert(cs->cdw < cs->reserved_dw);
   cs->buf[cs->cdw++] = value;
}

static inline void
radeon_emit_array(struct radeon_cmdbuf *cs, const uint32_t *values, unsigned count)
{
   assert(cs->cdw + count <= cs->reserved_dw);
   memcpy(cs->buf + cs->cdw, values, count * 4);
   cs->cdw += count;
}

static inline uint64_t
radv_buffer_get_va(const struct radeon_winsys_bo *bo)
{
   return bo->va;
}

static inline bool
radv_buffer_is_resident(const struct radeon_winsys_bo *bo)
{
   return bo->use_global_list || bo->is_local;
}

static inline void
radv_cs_add_buffer(struct radeon_winsys *ws, struct radeon_cmdbuf *cs, struct radeon_winsys_bo *bo)
{
   if (radv_buffer_is_resident(bo))
      return;

   ws->cs_add_buffer(cs, bo);
}

#endif /* RADV_RADEON_WINSYS_H */
