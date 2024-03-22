/*
 * Copyright © 2015 Intel Corporation
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

#ifndef ANV_PRIVATE_H
#define ANV_PRIVATE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include "drm-uapi/drm_fourcc.h"

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#define VG(x) x
#else
#define VG(x) ((void)0)
#endif

#include "common/intel_aux_map.h"
#include "common/intel_decoder.h"
#include "common/intel_engine.h"
#include "common/intel_gem.h"
#include "common/intel_l3_config.h"
#include "common/intel_measure.h"
#include "common/intel_mem.h"
#include "common/intel_sample_positions.h"
#include "dev/intel_device_info.h"
#include "blorp/blorp.h"
#include "compiler/brw_compiler.h"
#include "compiler/brw_kernel.h"
#include "compiler/brw_rt.h"
#include "ds/intel_driver_ds.h"
#include "util/bitset.h"
#include "util/bitscan.h"
#include "util/macros.h"
#include "util/hash_table.h"
#include "util/list.h"
#include "util/perf/u_trace.h"
#include "util/set.h"
#include "util/sparse_array.h"
#include "util/u_atomic.h"
#ifdef ANDROID
#include "util/u_gralloc/u_gralloc.h"
#endif
#include "util/u_vector.h"
#include "util/u_math.h"
#include "util/vma.h"
#include "util/xmlconfig.h"
#include "vk_acceleration_structure.h"
#include "vk_alloc.h"
#include "vk_buffer.h"
#include "vk_buffer_view.h"
#include "vk_command_buffer.h"
#include "vk_command_pool.h"
#include "vk_debug_report.h"
#include "vk_descriptor_update_template.h"
#include "vk_device.h"
#include "vk_device_memory.h"
#include "vk_drm_syncobj.h"
#include "vk_enum_defines.h"
#include "vk_format.h"
#include "vk_framebuffer.h"
#include "vk_graphics_state.h"
#include "vk_image.h"
#include "vk_instance.h"
#include "vk_pipeline_cache.h"
#include "vk_physical_device.h"
#include "vk_sampler.h"
#include "vk_shader_module.h"
#include "vk_sync.h"
#include "vk_sync_timeline.h"
#include "vk_texcompress_astc.h"
#include "vk_util.h"
#include "vk_query_pool.h"
#include "vk_queue.h"
#include "vk_log.h"
#include "vk_ycbcr_conversion.h"
#include "vk_video.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Pre-declarations needed for WSI entrypoints */
struct wl_surface;
struct wl_display;
typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_visualid_t;
typedef uint32_t xcb_window_t;

struct anv_batch;
struct anv_buffer;
struct anv_buffer_view;
struct anv_image_view;
struct anv_instance;

struct intel_aux_map_context;
struct intel_perf_config;
struct intel_perf_counter_pass;
struct intel_perf_query_result;

#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>

#include "anv_android.h"
#include "anv_entrypoints.h"
#include "anv_kmd_backend.h"
#include "isl/isl.h"

#include "dev/intel_debug.h"
#undef MESA_LOG_TAG
#define MESA_LOG_TAG "MESA-INTEL"
#include "util/log.h"
#include "wsi_common.h"

#define NSEC_PER_SEC 1000000000ull

#define BINDING_TABLE_POOL_BLOCK_SIZE (65536)

/* Allowing different clear colors requires us to perform a depth resolve at
 * the end of certain render passes. This is because while slow clears store
 * the clear color in the HiZ buffer, fast clears (without a resolve) don't.
 * See the PRMs for examples describing when additional resolves would be
 * necessary. To enable fast clears without requiring extra resolves, we set
 * the clear value to a globally-defined one. We could allow different values
 * if the user doesn't expect coherent data during or after a render passes
 * (VK_ATTACHMENT_STORE_OP_DONT_CARE), but such users (aside from the CTS)
 * don't seem to exist yet. In almost all Vulkan applications tested thus far,
 * 1.0f seems to be the only value used. The only application that doesn't set
 * this value does so through the usage of an seemingly uninitialized clear
 * value.
 */
#define ANV_HZ_FC_VAL 1.0f

/* 3DSTATE_VERTEX_BUFFER supports 33 VBs, we use 2 for base & drawid SGVs */
#define MAX_VBS         (33 - 2)

/* 3DSTATE_VERTEX_ELEMENTS supports up to 34 VEs, but our backend compiler
 * only supports the push model of VS inputs, and we only have 128 GRFs,
 * minus the g0 and g1 payload, which gives us a maximum of 31 VEs.  Plus,
 * we use two of them for SGVs.
 */
#define MAX_VES         (31 - 2)

#define MAX_XFB_BUFFERS  4
#define MAX_XFB_STREAMS  4
#define MAX_SETS         8
#define MAX_RTS          8
#define MAX_VIEWPORTS   16
#define MAX_SCISSORS    16
#define MAX_PUSH_CONSTANTS_SIZE 128
#define MAX_DYNAMIC_BUFFERS 16
#define MAX_PUSH_DESCRIPTORS 32 /* Minimum requirement */
#define MAX_INLINE_UNIFORM_BLOCK_SIZE 4096
#define MAX_INLINE_UNIFORM_BLOCK_DESCRIPTORS 32
/* We need 16 for UBO block reads to work and 32 for push UBOs. However, we
 * use 64 here to avoid cache issues. This could most likely bring it back to
 * 32 if we had different virtual addresses for the different views on a given
 * GEM object.
 */
#define ANV_UBO_ALIGNMENT 64
#define ANV_SSBO_ALIGNMENT 4
#define ANV_SSBO_BOUNDS_CHECK_ALIGNMENT 4
#define MAX_VIEWS_FOR_PRIMITIVE_REPLICATION 16
#define MAX_SAMPLE_LOCATIONS 16

/* RENDER_SURFACE_STATE is a bit smaller (48b) but since it is aligned to 64
 * and we can't put anything else there we use 64b.
 */
#define ANV_SURFACE_STATE_SIZE (64)

/* From the Skylake PRM Vol. 7 "Binding Table Surface State Model":
 *
 *    "The surface state model is used when a Binding Table Index (specified
 *    in the message descriptor) of less than 240 is specified. In this model,
 *    the Binding Table Index is used to index into the binding table, and the
 *    binding table entry contains a pointer to the SURFACE_STATE."
 *
 * Binding table values above 240 are used for various things in the hardware
 * such as stateless, stateless with incoherent cache, SLM, and bindless.
 */
#define MAX_BINDING_TABLE_SIZE 240

#define ANV_SVGS_VB_INDEX    MAX_VBS
#define ANV_DRAWID_VB_INDEX (MAX_VBS + 1)

/* We reserve this MI ALU register for the purpose of handling predication.
 * Other code which uses the MI ALU should leave it alone.
 */
#define ANV_PREDICATE_RESULT_REG 0x2678 /* MI_ALU_REG15 */

/* We reserve this MI ALU register to pass around an offset computed from
 * VkPerformanceQuerySubmitInfoKHR::counterPassIndex VK_KHR_performance_query.
 * Other code which uses the MI ALU should leave it alone.
 */
#define ANV_PERF_QUERY_OFFSET_REG 0x2670 /* MI_ALU_REG14 */

#define ANV_GRAPHICS_SHADER_STAGE_COUNT (MESA_SHADER_MESH + 1)

/* RENDER_SURFACE_STATE is a bit smaller (48b) but since it is aligned to 64
 * and we can't put anything else there we use 64b.
 */
#define ANV_SURFACE_STATE_SIZE (64)
#define ANV_SAMPLER_STATE_SIZE (32)

/* For gfx12 we set the streamout buffers using 4 separate commands
 * (3DSTATE_SO_BUFFER_INDEX_*) instead of 3DSTATE_SO_BUFFER. However the layout
 * of the 3DSTATE_SO_BUFFER_INDEX_* commands is identical to that of
 * 3DSTATE_SO_BUFFER apart from the SOBufferIndex field, so for now we use the
 * 3DSTATE_SO_BUFFER command, but change the 3DCommandSubOpcode.
 * SO_BUFFER_INDEX_0_CMD is actually the 3DCommandSubOpcode for
 * 3DSTATE_SO_BUFFER_INDEX_0.
 */
#define SO_BUFFER_INDEX_0_CMD 0x60
#define anv_printflike(a, b) __attribute__((__format__(__printf__, a, b)))

/* The TR-TT L1 page table entries may contain these values instead of actual
 * pointers to indicate the regions are either NULL or invalid. We program
 * these values to TR-TT registers, so we could change them, but it's super
 * convenient to have the NULL value be 0 because everything is
 * zero-initialized when allocated.
 *
 * Since we reserve these values for NULL/INVALID, then we can't use them as
 * destinations for TR-TT address translation. Both values are shifted by 16
 * bits, wich results in graphic addresses 0 and 64k. On Anv the first vma
 * starts at 2MB, so we already don't use 0 and 64k for anything, so there's
 * nothing really to reserve. We could instead just reserve random 64kb
 * ranges from any of the non-TR-TT vmas and use their addresses.
 */
#define ANV_TRTT_L1_NULL_TILE_VAL 0
#define ANV_TRTT_L1_INVALID_TILE_VAL 1

static inline uint32_t
align_down_npot_u32(uint32_t v, uint32_t a)
{
   return v - (v % a);
}

/** Alignment must be a power of 2. */
static inline bool
anv_is_aligned(uintmax_t n, uintmax_t a)
{
   assert(a == (a & -a));
   return (n & (a - 1)) == 0;
}

static inline union isl_color_value
vk_to_isl_color(VkClearColorValue color)
{
   return (union isl_color_value) {
      .u32 = {
         color.uint32[0],
         color.uint32[1],
         color.uint32[2],
         color.uint32[3],
      },
   };
}

static inline union isl_color_value
vk_to_isl_color_with_format(VkClearColorValue color, enum isl_format format)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(format);
   union isl_color_value isl_color = { .u32 = {0, } };

#define COPY_COLOR_CHANNEL(c, i) \
   if (fmtl->channels.c.bits) \
      isl_color.u32[i] = color.uint32[i]

   COPY_COLOR_CHANNEL(r, 0);
   COPY_COLOR_CHANNEL(g, 1);
   COPY_COLOR_CHANNEL(b, 2);
   COPY_COLOR_CHANNEL(a, 3);

#undef COPY_COLOR_CHANNEL

   return isl_color;
}

/**
 * Warn on ignored extension structs.
 *
 * The Vulkan spec requires us to ignore unsupported or unknown structs in
 * a pNext chain.  In debug mode, emitting warnings for ignored structs may
 * help us discover structs that we should not have ignored.
 *
 *
 * From the Vulkan 1.0.38 spec:
 *
 *    Any component of the implementation (the loader, any enabled layers,
 *    and drivers) must skip over, without processing (other than reading the
 *    sType and pNext members) any chained structures with sType values not
 *    defined by extensions supported by that component.
 */
#define anv_debug_ignored_stype(sType) \
   mesa_logd("%s: ignored VkStructureType %u\n", __func__, (sType))

void __anv_perf_warn(struct anv_device *device,
                     const struct vk_object_base *object,
                     const char *file, int line, const char *format, ...)
   anv_printflike(5, 6);

/**
 * Print a FINISHME message, including its source location.
 */
#define anv_finishme(format, ...) \
   do { \
      static bool reported = false; \
      if (!reported) { \
         mesa_logw("%s:%d: FINISHME: " format, __FILE__, __LINE__, \
                    ##__VA_ARGS__); \
         reported = true; \
      } \
   } while (0)

/**
 * Print a perf warning message.  Set INTEL_DEBUG=perf to see these.
 */
#define anv_perf_warn(objects_macro, format, ...)   \
   do { \
      static bool reported = false; \
      if (!reported && INTEL_DEBUG(DEBUG_PERF)) { \
         __vk_log(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,      \
                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,      \
                  objects_macro, __FILE__, __LINE__,                    \
                  format, ## __VA_ARGS__);                              \
         reported = true; \
      } \
   } while (0)

/* A non-fatal assert.  Useful for debugging. */
#ifdef DEBUG
#define anv_assert(x) ({ \
   if (unlikely(!(x))) \
      mesa_loge("%s:%d ASSERT: %s", __FILE__, __LINE__, #x); \
})
#else
#define anv_assert(x)
#endif

enum anv_bo_alloc_flags {
   /** Specifies that the BO must have a 32-bit address
    *
    * This is the opposite of EXEC_OBJECT_SUPPORTS_48B_ADDRESS.
    */
   ANV_BO_ALLOC_32BIT_ADDRESS =           (1 << 0),

   /** Specifies that the BO may be shared externally */
   ANV_BO_ALLOC_EXTERNAL =                (1 << 1),

   /** Specifies that the BO should be mapped */
   ANV_BO_ALLOC_MAPPED =                  (1 << 2),

   /** Specifies that the BO should be coherent.
    *
    * Note: In platforms with LLC where HOST_CACHED + HOST_COHERENT is free,
    * bo can get upgraded to HOST_CACHED_COHERENT
    */
   ANV_BO_ALLOC_HOST_COHERENT =           (1 << 3),

   /** Specifies that the BO should be captured in error states */
   ANV_BO_ALLOC_CAPTURE =                 (1 << 4),

   /** Specifies that the BO will have an address assigned by the caller
    *
    * Such BOs do not exist in any VMA heap.
    */
   ANV_BO_ALLOC_FIXED_ADDRESS =           (1 << 5),

   /** Enables implicit synchronization on the BO
    *
    * This is the opposite of EXEC_OBJECT_ASYNC.
    */
   ANV_BO_ALLOC_IMPLICIT_SYNC =           (1 << 6),

   /** Enables implicit synchronization on the BO
    *
    * This is equivalent to EXEC_OBJECT_WRITE.
    */
   ANV_BO_ALLOC_IMPLICIT_WRITE =          (1 << 7),

   /** Has an address which is visible to the client */
   ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS =  (1 << 8),

   /** Align the BO's virtual address to match AUX-TT requirements */
   ANV_BO_ALLOC_AUX_TT_ALIGNED =          (1 << 9),

   /** This buffer is allocated from local memory and should be cpu visible */
   ANV_BO_ALLOC_LOCAL_MEM_CPU_VISIBLE =   (1 << 10),

   /** For non device local allocations */
   ANV_BO_ALLOC_NO_LOCAL_MEM =            (1 << 11),

   /** This buffer will be scanout to display */
   ANV_BO_ALLOC_SCANOUT =                 (1 << 12),

   /** For descriptor pools */
   ANV_BO_ALLOC_DESCRIPTOR_POOL =         (1 << 13),

   /** For buffers that will be bound using TR-TT.
    *
    * Not for buffers used as the TR-TT page tables.
    */
   ANV_BO_ALLOC_TRTT =                    (1 << 14),

   /** Protected buffer */
   ANV_BO_ALLOC_PROTECTED =               (1 << 15),

   /** Specifies that the BO should be cached and incoherent. */
   ANV_BO_ALLOC_HOST_CACHED =             (1 << 16),

   /** For sampler pools */
   ANV_BO_ALLOC_SAMPLER_POOL =            (1 << 17),

   /** Specifies that the BO is imported.
    *
    * Imported BOs must also be marked as ANV_BO_ALLOC_EXTERNAL
    */
   ANV_BO_ALLOC_IMPORTED =                (1 << 18),

   /** Specifies that the BO should be cached and coherent. */
   ANV_BO_ALLOC_HOST_CACHED_COHERENT =    (ANV_BO_ALLOC_HOST_COHERENT | ANV_BO_ALLOC_HOST_CACHED),
};

struct anv_bo {
   const char *name;

   /* The VMA heap in anv_device from which this BO takes its offset.
    *
    * This can only be NULL when has_fixed_address is true.
    */
   struct util_vma_heap *vma_heap;

   /* All userptr bos in Xe KMD has gem_handle set to workaround_bo->gem_handle */
   uint32_t gem_handle;

   uint32_t refcount;

   /* Index into the current validation list.  This is used by the
    * validation list building algorithm to track which buffers are already
    * in the validation list so that we can ensure uniqueness.
    */
   uint32_t exec_obj_index;

   /* Index for use with util_sparse_array_free_list */
   uint32_t free_index;

   /* Last known offset.  This value is provided by the kernel when we
    * execbuf and is used as the presumed offset for the next bunch of
    * relocations, in canonical address format.
    */
   uint64_t offset;

   /** Size of the buffer */
   uint64_t size;

   /* Map for internally mapped BOs.
    *
    * If ANV_BO_ALLOC_MAPPED is set in flags, this is the map for the whole
    * BO.
    */
   void *map;

   /* The actual size of bo allocated by kmd, basically:
    * align(size, mem_alignment)
    */
   uint64_t actual_size;

   /** Flags to pass to the kernel through drm_i915_exec_object2::flags */
   uint32_t flags;

   enum anv_bo_alloc_flags alloc_flags;

   /** True if this BO wraps a host pointer */
   bool from_host_ptr:1;
};

static inline bool
anv_bo_is_external(const struct anv_bo *bo)
{
   return bo->alloc_flags & ANV_BO_ALLOC_EXTERNAL;
}

static inline bool
anv_bo_is_vram_only(const struct anv_bo *bo)
{
   return !(bo->alloc_flags & (ANV_BO_ALLOC_NO_LOCAL_MEM |
                               ANV_BO_ALLOC_MAPPED |
                               ANV_BO_ALLOC_LOCAL_MEM_CPU_VISIBLE |
                               ANV_BO_ALLOC_IMPORTED));
}

static inline struct anv_bo *
anv_bo_ref(struct anv_bo *bo)
{
   p_atomic_inc(&bo->refcount);
   return bo;
}

enum intel_device_info_mmap_mode
anv_bo_get_mmap_mode(struct anv_device *device, struct anv_bo *bo);

static inline bool
anv_bo_needs_host_cache_flush(enum anv_bo_alloc_flags alloc_flags)
{
   return (alloc_flags & (ANV_BO_ALLOC_HOST_CACHED | ANV_BO_ALLOC_HOST_COHERENT)) ==
          ANV_BO_ALLOC_HOST_CACHED;
}

struct anv_address {
   struct anv_bo *bo;
   int64_t offset;
};

#define ANV_NULL_ADDRESS ((struct anv_address) { NULL, 0 })

static inline struct anv_address
anv_address_from_u64(uint64_t addr_u64)
{
   assert(addr_u64 == intel_canonical_address(addr_u64));
   return (struct anv_address) {
      .bo = NULL,
      .offset = addr_u64,
   };
}

static inline bool
anv_address_is_null(struct anv_address addr)
{
   return addr.bo == NULL && addr.offset == 0;
}

static inline uint64_t
anv_address_physical(struct anv_address addr)
{
   uint64_t address = (addr.bo ? addr.bo->offset : 0ull) + addr.offset;
   return intel_canonical_address(address);
}

static inline struct anv_address
anv_address_add(struct anv_address addr, uint64_t offset)
{
   addr.offset += offset;
   return addr;
}

static inline void *
anv_address_map(struct anv_address addr)
{
   if (addr.bo == NULL)
      return NULL;

   if (addr.bo->map == NULL)
      return NULL;

   return addr.bo->map + addr.offset;
}

/* Represent a virtual address range */
struct anv_va_range {
   uint64_t addr;
   uint64_t size;
};

/* Represents a lock-free linked list of "free" things.  This is used by
 * both the block pool and the state pools.  Unfortunately, in order to
 * solve the ABA problem, we can't use a single uint32_t head.
 */
union anv_free_list {
   struct {
      uint32_t offset;

      /* A simple count that is incremented every time the head changes. */
      uint32_t count;
   };
   /* Make sure it's aligned to 64 bits. This will make atomic operations
    * faster on 32 bit platforms.
    */
   alignas(8) uint64_t u64;
};

#define ANV_FREE_LIST_EMPTY ((union anv_free_list) { { UINT32_MAX, 0 } })

struct anv_block_state {
   union {
      struct {
         uint32_t next;
         uint32_t end;
      };
      /* Make sure it's aligned to 64 bits. This will make atomic operations
       * faster on 32 bit platforms.
       */
      alignas(8) uint64_t u64;
   };
};

#define anv_block_pool_foreach_bo(bo, pool)  \
   for (struct anv_bo **_pp_bo = (pool)->bos, *bo; \
        _pp_bo != &(pool)->bos[(pool)->nbos] && (bo = *_pp_bo, true); \
        _pp_bo++)

#define ANV_MAX_BLOCK_POOL_BOS 20

struct anv_block_pool {
   const char *name;

   struct anv_device *device;

   struct anv_bo *bos[ANV_MAX_BLOCK_POOL_BOS];
   struct anv_bo *bo;
   uint32_t nbos;

   /* Maximum size of the pool */
   uint64_t max_size;

   /* Current size of the pool */
   uint64_t size;

   /* The canonical address where the start of the pool is pinned. The various bos that
    * are created as the pool grows will have addresses in the range
    * [start_address, start_address + BLOCK_POOL_MEMFD_SIZE).
    */
   uint64_t start_address;

   /* The offset from the start of the bo to the "center" of the block
    * pool.  Pointers to allocated blocks are given by
    * bo.map + center_bo_offset + offsets.
    */
   uint32_t center_bo_offset;

   struct anv_block_state state;

   enum anv_bo_alloc_flags bo_alloc_flags;
};

/* Block pools are backed by a fixed-size 1GB memfd */
#define BLOCK_POOL_MEMFD_SIZE (1ul << 30)

/* The center of the block pool is also the middle of the memfd.  This may
 * change in the future if we decide differently for some reason.
 */
#define BLOCK_POOL_MEMFD_CENTER (BLOCK_POOL_MEMFD_SIZE / 2)

static inline uint32_t
anv_block_pool_size(struct anv_block_pool *pool)
{
   return pool->state.end;
}

struct anv_state {
   int64_t offset;
   uint32_t alloc_size;
   uint32_t idx;
   void *map;
};

#define ANV_STATE_NULL ((struct anv_state) { .alloc_size = 0 })

struct anv_fixed_size_state_pool {
   union anv_free_list free_list;
   struct anv_block_state block;
};

#define ANV_MIN_STATE_SIZE_LOG2 6
#define ANV_MAX_STATE_SIZE_LOG2 22

#define ANV_STATE_BUCKETS (ANV_MAX_STATE_SIZE_LOG2 - ANV_MIN_STATE_SIZE_LOG2 + 1)

struct anv_free_entry {
   uint32_t next;
   struct anv_state state;
};

struct anv_state_table {
   struct anv_device *device;
   int fd;
   struct anv_free_entry *map;
   uint32_t size;
   uint64_t max_size;
   struct anv_block_state state;
   struct u_vector cleanups;
};

struct anv_state_pool {
   struct anv_block_pool block_pool;

   /* Offset into the relevant state base address where the state pool starts
    * allocating memory.
    */
   int64_t start_offset;

   struct anv_state_table table;

   /* The size of blocks which will be allocated from the block pool */
   uint32_t block_size;

   struct anv_fixed_size_state_pool buckets[ANV_STATE_BUCKETS];
};

struct anv_state_reserved_pool {
   struct anv_state_pool *pool;
   union anv_free_list reserved_blocks;
   uint32_t count;
};

struct anv_state_stream {
   struct anv_state_pool *state_pool;

   /* The size of blocks to allocate from the state pool */
   uint32_t block_size;

   /* Current block we're allocating from */
   struct anv_state block;

   /* Offset into the current block at which to allocate the next state */
   uint32_t next;

   /* List of all blocks allocated from this pool */
   struct util_dynarray all_blocks;
};

struct anv_sparse_submission {
   struct anv_queue *queue;

   struct anv_vm_bind *binds;
   int binds_len;
   int binds_capacity;

   uint32_t wait_count;
   uint32_t signal_count;

   struct vk_sync_wait *waits;
   struct vk_sync_signal *signals;
};

struct anv_trtt_bind {
   uint64_t pte_addr;
   uint64_t entry_addr;
};

struct anv_trtt_submission {
   struct anv_sparse_submission *sparse;

   struct anv_trtt_bind *l3l2_binds;
   struct anv_trtt_bind *l1_binds;

   int l3l2_binds_len;
   int l1_binds_len;
};

/* The block_pool functions exported for testing only.  The block pool should
 * only be used via a state pool (see below).
 */
VkResult anv_block_pool_init(struct anv_block_pool *pool,
                             struct anv_device *device,
                             const char *name,
                             uint64_t start_address,
                             uint32_t initial_size,
                             uint32_t max_size);
void anv_block_pool_finish(struct anv_block_pool *pool);
VkResult anv_block_pool_alloc(struct anv_block_pool *pool,
                              uint32_t block_size,
                              int64_t *offset,
                              uint32_t *padding);
void* anv_block_pool_map(struct anv_block_pool *pool, int32_t offset, uint32_t
size);

struct anv_state_pool_params {
   const char *name;
   uint64_t    base_address;
   int64_t     start_offset;
   uint32_t    block_size;
   uint32_t    max_size;
};

VkResult anv_state_pool_init(struct anv_state_pool *pool,
                             struct anv_device *device,
                             const struct anv_state_pool_params *params);
void anv_state_pool_finish(struct anv_state_pool *pool);
struct anv_state anv_state_pool_alloc(struct anv_state_pool *pool,
                                      uint32_t state_size, uint32_t alignment);
void anv_state_pool_free(struct anv_state_pool *pool, struct anv_state state);

static inline struct anv_address
anv_state_pool_state_address(struct anv_state_pool *pool, struct anv_state state)
{
   return (struct anv_address) {
      .bo = pool->block_pool.bo,
      .offset = state.offset - pool->start_offset,
   };
}

void anv_state_stream_init(struct anv_state_stream *stream,
                           struct anv_state_pool *state_pool,
                           uint32_t block_size);
void anv_state_stream_finish(struct anv_state_stream *stream);
struct anv_state anv_state_stream_alloc(struct anv_state_stream *stream,
                                        uint32_t size, uint32_t alignment);

void anv_state_reserved_pool_init(struct anv_state_reserved_pool *pool,
                                      struct anv_state_pool *parent,
                                      uint32_t count, uint32_t size,
                                      uint32_t alignment);
void anv_state_reserved_pool_finish(struct anv_state_reserved_pool *pool);
struct anv_state anv_state_reserved_pool_alloc(struct anv_state_reserved_pool *pool);
void anv_state_reserved_pool_free(struct anv_state_reserved_pool *pool,
                                  struct anv_state state);

VkResult anv_state_table_init(struct anv_state_table *table,
                             struct anv_device *device,
                             uint32_t initial_entries);
void anv_state_table_finish(struct anv_state_table *table);
VkResult anv_state_table_add(struct anv_state_table *table, uint32_t *idx,
                             uint32_t count);
void anv_free_list_push(union anv_free_list *list,
                        struct anv_state_table *table,
                        uint32_t idx, uint32_t count);
struct anv_state* anv_free_list_pop(union anv_free_list *list,
                                    struct anv_state_table *table);


static inline struct anv_state *
anv_state_table_get(struct anv_state_table *table, uint32_t idx)
{
   return &table->map[idx].state;
}
/**
 * Implements a pool of re-usable BOs.  The interface is identical to that
 * of block_pool except that each block is its own BO.
 */
struct anv_bo_pool {
   const char *name;

   struct anv_device *device;

   enum anv_bo_alloc_flags bo_alloc_flags;

   struct util_sparse_array_free_list free_list[16];
};

void anv_bo_pool_init(struct anv_bo_pool *pool, struct anv_device *device,
                      const char *name, enum anv_bo_alloc_flags alloc_flags);
void anv_bo_pool_finish(struct anv_bo_pool *pool);
VkResult anv_bo_pool_alloc(struct anv_bo_pool *pool, uint32_t size,
                           struct anv_bo **bo_out);
void anv_bo_pool_free(struct anv_bo_pool *pool, struct anv_bo *bo);

struct anv_scratch_pool {
   /* Indexed by Per-Thread Scratch Space number (the hardware value) and stage */
   struct anv_bo *bos[16][MESA_SHADER_STAGES];
   uint32_t surfs[16];
   struct anv_state surf_states[16];
};

void anv_scratch_pool_init(struct anv_device *device,
                           struct anv_scratch_pool *pool);
void anv_scratch_pool_finish(struct anv_device *device,
                             struct anv_scratch_pool *pool);
struct anv_bo *anv_scratch_pool_alloc(struct anv_device *device,
                                      struct anv_scratch_pool *pool,
                                      gl_shader_stage stage,
                                      unsigned per_thread_scratch);
uint32_t anv_scratch_pool_get_surf(struct anv_device *device,
                                   struct anv_scratch_pool *pool,
                                   unsigned per_thread_scratch);

/** Implements a BO cache that ensures a 1-1 mapping of GEM BOs to anv_bos */
struct anv_bo_cache {
   struct util_sparse_array bo_map;
   pthread_mutex_t mutex;
};

VkResult anv_bo_cache_init(struct anv_bo_cache *cache,
                           struct anv_device *device);
void anv_bo_cache_finish(struct anv_bo_cache *cache);

struct anv_queue_family {
   /* Standard bits passed on to the client */
   VkQueueFlags   queueFlags;
   uint32_t       queueCount;

   enum intel_engine_class engine_class;
};

#define ANV_MAX_QUEUE_FAMILIES 5

struct anv_memory_type {
   /* Standard bits passed on to the client */
   VkMemoryPropertyFlags   propertyFlags;
   uint32_t                heapIndex;
};

struct anv_memory_heap {
   /* Standard bits passed on to the client */
   VkDeviceSize      size;
   VkMemoryHeapFlags flags;

   /** Driver-internal book-keeping.
    *
    * Align it to 64 bits to make atomic operations faster on 32 bit platforms.
    */
   alignas(8) VkDeviceSize used;

   bool              is_local_mem;
};

struct anv_memregion {
   const struct intel_memory_class_instance *region;
   uint64_t size;
   uint64_t available;
};

enum anv_timestamp_capture_type {
    ANV_TIMESTAMP_CAPTURE_TOP_OF_PIPE,
    ANV_TIMESTAMP_CAPTURE_END_OF_PIPE,
    ANV_TIMESTAMP_CAPTURE_AT_CS_STALL,
    ANV_TIMESTAMP_REWRITE_COMPUTE_WALKER,
    ANV_TIMESTAMP_REWRITE_INDIRECT_DISPATCH,
};

struct anv_physical_device {
    struct vk_physical_device                   vk;

    /* Link in anv_instance::physical_devices */
    struct list_head                            link;

    struct anv_instance *                       instance;
    char                                        path[20];
    struct intel_device_info                      info;

    bool                                        video_decode_enabled;

    struct brw_compiler *                       compiler;
    struct isl_device                           isl_dev;
    struct intel_perf_config *                    perf;
    /*
     * Number of commands required to implement a performance query begin +
     * end.
     */
    uint32_t                                    n_perf_query_commands;
    bool                                        has_exec_async;
    bool                                        has_exec_capture;
    VkQueueGlobalPriorityKHR                    max_context_priority;
    uint64_t                                    gtt_size;

    bool                                        always_use_bindless;
    bool                                        use_call_secondary;

    /** True if we can use timeline semaphores through execbuf */
    bool                                        has_exec_timeline;

    /** True if we can read the GPU timestamp register
     *
     * When running in a virtual context, the timestamp register is unreadable
     * on Gfx12+.
     */
    bool                                        has_reg_timestamp;

    /** True if we can create protected contexts. */
    bool                                        has_protected_contexts;

    /** Whether the i915 driver has the ability to create VM objects */
    bool                                        has_vm_control;

    /** True if we have the means to do sparse binding (e.g., a Kernel driver
     * a vm_bind ioctl).
     */
    bool                                        has_sparse;
    bool                                        sparse_uses_trtt;

    /** True if HW supports ASTC LDR */
    bool                                        has_astc_ldr;
    /** True if denorms in void extents should be flushed to zero */
    bool                                        flush_astc_ldr_void_extent_denorms;
    /** True if ASTC LDR is supported via emulation */
    bool                                        emu_astc_ldr;
    /* true if FCV optimization should be disabled. */
    bool                                        disable_fcv;
    /**/
    bool                                        uses_ex_bso;

    bool                                        always_flush_cache;
    /**
     * True if the descriptors buffers are holding one of the following :
     *    - anv_sampled_image_descriptor
     *    - anv_storage_image_descriptor
     *    - anv_address_range_descriptor
     *
     * Accessing the descriptors in a bindless fashion from the shader
     * requires an indirection in the shader, first fetch one of the structure
     * listed above from the descriptor buffer, then emit the send message to
     * the fixed function (sampler, dataport, etc...) with the handle fetched
     * above.
     *
     * We need to do things this way prior to DG2 because the bindless surface
     * state space is limited to 64Mb and some application will allocate more
     * than what HW can support. On DG2+ we get 4Gb of bindless surface state
     * and so we can reference directly RENDER_SURFACE_STATE/SAMPLER_STATE
     * structures instead.
     */
    bool                                        indirect_descriptors;

    bool                                        uses_relocs;

    /** Can the platform support cooperative matrices and is it enabled? */
    bool                                        has_cooperative_matrix;

    struct {
      uint32_t                                  family_count;
      struct anv_queue_family                   families[ANV_MAX_QUEUE_FAMILIES];
    } queue;

    struct {
      uint32_t                                  type_count;
      struct anv_memory_type                    types[VK_MAX_MEMORY_TYPES];
      uint32_t                                  heap_count;
      struct anv_memory_heap                    heaps[VK_MAX_MEMORY_HEAPS];
#ifdef SUPPORT_INTEL_INTEGRATED_GPUS
      bool                                      need_flush;
#endif
    } memory;

    struct {
       /**
        * General state pool
        */
       struct anv_va_range                      general_state_pool;
       /**
        * Low 32bit heap
        */
       struct anv_va_range                      low_heap;
       /**
        * Binding table pool
        */
       struct anv_va_range                      binding_table_pool;
       /**
        * Internal surface states for blorp & push descriptors.
        */
       struct anv_va_range                      internal_surface_state_pool;
       /**
        * Scratch surfaces (overlaps with internal_surface_state_pool).
        */
       struct anv_va_range                      scratch_surface_state_pool;
       /**
        * Bindless surface states (indirectly referred to by indirect
        * descriptors or for direct descriptors)
        */
       struct anv_va_range                      bindless_surface_state_pool;
       /**
        * Dynamic state pool
        */
       struct anv_va_range                      dynamic_state_pool;
       /**
        * Sampler state pool
        */
       struct anv_va_range                      sampler_state_pool;
       /**
        * Indirect descriptor pool
        */
       struct anv_va_range                      indirect_descriptor_pool;
       /**
        * Indirect push descriptor pool
        */
       struct anv_va_range                      indirect_push_descriptor_pool;
       /**
        * Instruction state pool
        */
       struct anv_va_range                      instruction_state_pool;
       /**
        * Client heap
        */
       struct anv_va_range                      high_heap;
       struct anv_va_range                      trtt;
    } va;

    /* Either we have a single vram region and it's all mappable, or we have
     * both mappable & non-mappable parts. System memory is always available.
     */
    struct anv_memregion                        vram_mappable;
    struct anv_memregion                        vram_non_mappable;
    struct anv_memregion                        sys;
    uint8_t                                     driver_build_sha1[20];
    uint8_t                                     pipeline_cache_uuid[VK_UUID_SIZE];
    uint8_t                                     driver_uuid[VK_UUID_SIZE];
    uint8_t                                     device_uuid[VK_UUID_SIZE];
    uint8_t                                     rt_uuid[VK_UUID_SIZE];

    /* Maximum amount of scratch space used by all the GRL kernels */
    uint32_t                                    max_grl_scratch_size;

    struct vk_sync_type                         sync_syncobj_type;
    struct vk_sync_timeline_type                sync_timeline_type;
    const struct vk_sync_type *                 sync_types[4];

    struct wsi_device                       wsi_device;
    int                                         local_fd;
    bool                                        has_local;
    int64_t                                     local_major;
    int64_t                                     local_minor;
    int                                         master_fd;
    bool                                        has_master;
    int64_t                                     master_major;
    int64_t                                     master_minor;
    struct intel_query_engine_info *            engine_info;

    void (*cmd_emit_timestamp)(struct anv_batch *, struct anv_device *, struct anv_address,
                               enum anv_timestamp_capture_type, void *);
    struct intel_measure_device                 measure_device;
};

static inline uint32_t
anv_physical_device_bindless_heap_size(const struct anv_physical_device *device)
{
   return device->uses_ex_bso ?
      128 * 1024 * 1024 /* 128 MiB */ :
      64 * 1024 * 1024 /* 64 MiB */;
}

static inline bool
anv_physical_device_has_vram(const struct anv_physical_device *device)
{
   return device->vram_mappable.size > 0;
}

struct anv_instance {
    struct vk_instance                          vk;

    struct driOptionCache                       dri_options;
    struct driOptionCache                       available_dri_options;

    int                                         mesh_conv_prim_attrs_to_vert_attrs;
    bool                                        enable_tbimr;

    /**
     * Workarounds for game bugs.
     */
    uint8_t                                     assume_full_subgroups;
    bool                                        limit_trig_input_range;
    bool                                        sample_mask_out_opengl_behaviour;
    bool                                        force_filter_addr_rounding;
    bool                                        fp64_workaround_enabled;
    float                                       lower_depth_range_rate;
    unsigned                                    generated_indirect_threshold;
    unsigned                                    generated_indirect_ring_threshold;
    unsigned                                    query_clear_with_blorp_threshold;
    unsigned                                    query_copy_with_shader_threshold;
    unsigned                                    force_vk_vendor;
    bool                                        has_fake_sparse;
    bool                                        disable_fcv;

    /* HW workarounds */
    bool                                        no_16bit;
    bool                                        intel_enable_wa_14018912822;
};

VkResult anv_init_wsi(struct anv_physical_device *physical_device);
void anv_finish_wsi(struct anv_physical_device *physical_device);

struct anv_queue {
   struct vk_queue                           vk;

   struct anv_device *                       device;

   const struct anv_queue_family *           family;

   struct intel_batch_decode_ctx *           decoder;

   union {
      uint32_t                               exec_flags; /* i915 */
      uint32_t                               context_id; /* i915 */
      uint32_t                               exec_queue_id; /* Xe */
   };

   /** Context/Engine id which executes companion RCS command buffer */
   uint32_t                                  companion_rcs_id;

   /** Synchronization object for debug purposes (DEBUG_SYNC) */
   struct vk_sync                           *sync;

   /** Companion synchronization object
    *
    * Vulkan command buffers can be destroyed as soon as their lifecycle moved
    * from the Pending state to the Invalid/Executable state. This transition
    * happens when the VkFence/VkSemaphore associated with the completion of
    * the command buffer work is signaled.
    *
    * When we're using a companion command buffer to execute part of another
    * command buffer, we need to tie the 2 work submissions together to ensure
    * when the associated VkFence/VkSemaphore is signaled, both command
    * buffers are actually unused by the HW. To do this, we run an empty batch
    * buffer that we use to signal after both submissions :
    *
    *   CCS -->    main   ---> empty_batch (with wait on companion) --> signal
    *   RCS --> companion -|
    *
    * When companion batch completes, it signals companion_sync and allow
    * empty_batch to execute. Since empty_batch is running on the main engine,
    * we're guaranteed that upon completion both main & companion command
    * buffers are not used by HW anymore.
    */
   struct vk_sync                           *companion_sync;

   struct intel_ds_queue                     ds;
};

struct nir_xfb_info;
struct anv_pipeline_bind_map;
struct anv_push_descriptor_info;
enum anv_dynamic_push_bits;

extern const struct vk_pipeline_cache_object_ops *const anv_cache_import_ops[2];

struct anv_shader_bin *
anv_device_search_for_kernel(struct anv_device *device,
                             struct vk_pipeline_cache *cache,
                             const void *key_data, uint32_t key_size,
                             bool *user_cache_bit);

struct anv_shader_bin *
anv_device_upload_kernel(struct anv_device *device,
                         struct vk_pipeline_cache *cache,
                         gl_shader_stage stage,
                         const void *key_data, uint32_t key_size,
                         const void *kernel_data, uint32_t kernel_size,
                         const struct brw_stage_prog_data *prog_data,
                         uint32_t prog_data_size,
                         const struct brw_compile_stats *stats,
                         uint32_t num_stats,
                         const struct nir_xfb_info *xfb_info,
                         const struct anv_pipeline_bind_map *bind_map,
                         const struct anv_push_descriptor_info *push_desc_info,
                         enum anv_dynamic_push_bits dynamic_push_values);

struct nir_shader;
struct nir_shader_compiler_options;

struct nir_shader *
anv_device_search_for_nir(struct anv_device *device,
                          struct vk_pipeline_cache *cache,
                          const struct nir_shader_compiler_options *nir_options,
                          unsigned char sha1_key[20],
                          void *mem_ctx);

void
anv_device_upload_nir(struct anv_device *device,
                      struct vk_pipeline_cache *cache,
                      const struct nir_shader *nir,
                      unsigned char sha1_key[20]);

void
anv_load_fp64_shader(struct anv_device *device);

/**
 * This enum tracks the various HW instructions that hold graphics state
 * needing to be reprogrammed. Some instructions are grouped together as they
 * pretty much need to be emitted together (like 3DSTATE_URB_*).
 *
 * Not all bits apply to all platforms. We build a dirty state based on
 * enabled extensions & generation on anv_device.
 */
enum anv_gfx_state_bits {
   /* Pipeline states */
   ANV_GFX_STATE_URB, /* All legacy stages, including mesh */
   ANV_GFX_STATE_VF_STATISTICS,
   ANV_GFX_STATE_VF_SGVS,
   ANV_GFX_STATE_VF_SGVS_2,
   ANV_GFX_STATE_VF_SGVS_VI, /* 3DSTATE_VERTEX_ELEMENTS for sgvs elements */
   ANV_GFX_STATE_VF_SGVS_INSTANCING, /* 3DSTATE_VF_INSTANCING for sgvs elements */
   ANV_GFX_STATE_PRIMITIVE_REPLICATION,
   ANV_GFX_STATE_MULTISAMPLE,
   ANV_GFX_STATE_SBE,
   ANV_GFX_STATE_SBE_SWIZ,
   ANV_GFX_STATE_SO_DECL_LIST,
   ANV_GFX_STATE_VS,
   ANV_GFX_STATE_HS,
   ANV_GFX_STATE_DS,
   ANV_GFX_STATE_GS,
   ANV_GFX_STATE_PS,
   ANV_GFX_STATE_PS_EXTRA,
   ANV_GFX_STATE_SBE_MESH,
   ANV_GFX_STATE_CLIP_MESH,
   ANV_GFX_STATE_MESH_CONTROL,
   ANV_GFX_STATE_MESH_SHADER,
   ANV_GFX_STATE_MESH_DISTRIB,
   ANV_GFX_STATE_TASK_CONTROL,
   ANV_GFX_STATE_TASK_SHADER,
   ANV_GFX_STATE_TASK_REDISTRIB,
   /* Dynamic states */
   ANV_GFX_STATE_BLEND_STATE, /* Just the dynamic state structure */
   ANV_GFX_STATE_BLEND_STATE_POINTERS, /* The pointer to the dynamic state */
   ANV_GFX_STATE_CLIP,
   ANV_GFX_STATE_CC_STATE,
   ANV_GFX_STATE_CPS,
   ANV_GFX_STATE_DEPTH_BOUNDS,
   ANV_GFX_STATE_INDEX_BUFFER,
   ANV_GFX_STATE_LINE_STIPPLE,
   ANV_GFX_STATE_PS_BLEND,
   ANV_GFX_STATE_RASTER,
   ANV_GFX_STATE_SAMPLE_MASK,
   ANV_GFX_STATE_SAMPLE_PATTERN,
   ANV_GFX_STATE_SCISSOR,
   ANV_GFX_STATE_SF,
   ANV_GFX_STATE_STREAMOUT,
   ANV_GFX_STATE_TE,
   ANV_GFX_STATE_VERTEX_INPUT,
   ANV_GFX_STATE_VF,
   ANV_GFX_STATE_VF_TOPOLOGY,
   ANV_GFX_STATE_VFG,
   ANV_GFX_STATE_VIEWPORT_CC,
   ANV_GFX_STATE_VIEWPORT_SF_CLIP,
   ANV_GFX_STATE_WM,
   ANV_GFX_STATE_WM_DEPTH_STENCIL,
   ANV_GFX_STATE_PMA_FIX, /* Fake state to implement workaround */
   ANV_GFX_STATE_WA_18019816803, /* Fake state to implement workaround */
   ANV_GFX_STATE_TBIMR_TILE_PASS_INFO,

   ANV_GFX_STATE_MAX,
};

const char *anv_gfx_state_bit_to_str(enum anv_gfx_state_bits state);

/* This structure tracks the values to program in HW instructions for
 * corresponding to dynamic states of the Vulkan API. Only fields that need to
 * be reemitted outside of the VkPipeline object are tracked here.
 */
struct anv_gfx_dynamic_state {
   /* 3DSTATE_BLEND_STATE_POINTERS */
   struct {
      bool AlphaToCoverageEnable;
      bool AlphaToOneEnable;
      bool IndependentAlphaBlendEnable;
      struct {
         bool     WriteDisableAlpha;
         bool     WriteDisableRed;
         bool     WriteDisableGreen;
         bool     WriteDisableBlue;

         uint32_t LogicOpFunction;
         bool     LogicOpEnable;

         bool     ColorBufferBlendEnable;
         uint32_t ColorClampRange;
         bool     PreBlendColorClampEnable;
         bool     PostBlendColorClampEnable;
         uint32_t SourceBlendFactor;
         uint32_t DestinationBlendFactor;
         uint32_t ColorBlendFunction;
         uint32_t SourceAlphaBlendFactor;
         uint32_t DestinationAlphaBlendFactor;
         uint32_t AlphaBlendFunction;
      } rts[MAX_RTS];
  } blend;

   /* 3DSTATE_CC_STATE_POINTERS */
   struct {
      float BlendConstantColorRed;
      float BlendConstantColorGreen;
      float BlendConstantColorBlue;
      float BlendConstantColorAlpha;
   } cc;

   /* 3DSTATE_CLIP */
   struct {
      uint32_t APIMode;
      uint32_t ViewportXYClipTestEnable;
      uint32_t MaximumVPIndex;
      uint32_t TriangleStripListProvokingVertexSelect;
      uint32_t LineStripListProvokingVertexSelect;
      uint32_t TriangleFanProvokingVertexSelect;
   } clip;

   /* 3DSTATE_CPS/3DSTATE_CPS_POINTERS */
   struct {
      /* Gfx11 */
      uint32_t CoarsePixelShadingMode;
      float    MinCPSizeX;
      float    MinCPSizeY;
      /* Gfx12+ */
      uint32_t CoarsePixelShadingStateArrayPointer;
   } cps;

   /* 3DSTATE_DEPTH_BOUNDS */
   struct {
      bool     DepthBoundsTestEnable;
      float    DepthBoundsTestMinValue;
      float    DepthBoundsTestMaxValue;
   } db;

   /* 3DSTATE_GS */
   struct {
      uint32_t ReorderMode;
   } gs;

   /* 3DSTATE_LINE_STIPPLE */
   struct {
      uint32_t LineStipplePattern;
      float    LineStippleInverseRepeatCount;
      uint32_t LineStippleRepeatCount;
   } ls;

   /* 3DSTATE_PS_BLEND */
   struct {
      bool     HasWriteableRT;
      bool     ColorBufferBlendEnable;
      uint32_t SourceAlphaBlendFactor;
      uint32_t DestinationAlphaBlendFactor;
      uint32_t SourceBlendFactor;
      uint32_t DestinationBlendFactor;
      bool     AlphaTestEnable;
      bool     IndependentAlphaBlendEnable;
      bool     AlphaToCoverageEnable;
   } ps_blend;

   /* 3DSTATE_RASTER */
   struct {
      uint32_t APIMode;
      bool     DXMultisampleRasterizationEnable;
      bool     AntialiasingEnable;
      uint32_t CullMode;
      uint32_t FrontWinding;
      bool     GlobalDepthOffsetEnableSolid;
      bool     GlobalDepthOffsetEnableWireframe;
      bool     GlobalDepthOffsetEnablePoint;
      float    GlobalDepthOffsetConstant;
      float    GlobalDepthOffsetScale;
      float    GlobalDepthOffsetClamp;
      uint32_t FrontFaceFillMode;
      uint32_t BackFaceFillMode;
      bool     ViewportZFarClipTestEnable;
      bool     ViewportZNearClipTestEnable;
      bool     ConservativeRasterizationEnable;
   } raster;

   /* 3DSTATE_SCISSOR_STATE_POINTERS */
   struct {
      uint32_t count;
      struct {
         uint32_t ScissorRectangleYMin;
         uint32_t ScissorRectangleXMin;
         uint32_t ScissorRectangleYMax;
         uint32_t ScissorRectangleXMax;
      } elem[MAX_SCISSORS];
   } scissor;

   /* 3DSTATE_SF */
   struct {
      float    LineWidth;
      uint32_t TriangleStripListProvokingVertexSelect;
      uint32_t LineStripListProvokingVertexSelect;
      uint32_t TriangleFanProvokingVertexSelect;
      bool     LegacyGlobalDepthBiasEnable;
   } sf;

   /* 3DSTATE_STREAMOUT */
   struct {
      bool     RenderingDisable;
      uint32_t RenderStreamSelect;
      uint32_t ReorderMode;
      uint32_t ForceRendering;
   } so;

   /* 3DSTATE_SAMPLE_MASK */
   struct {
      uint32_t SampleMask;
   } sm;

   /* 3DSTATE_TE */
   struct {
      uint32_t OutputTopology;
   } te;

   /* 3DSTATE_VF */
   struct {
      bool     IndexedDrawCutIndexEnable;
      uint32_t CutIndex;
   } vf;

   /* 3DSTATE_VFG */
   struct {
      uint32_t DistributionMode;
      bool     ListCutIndexEnable;
   } vfg;

   /* 3DSTATE_VF_TOPOLOGY */
   struct {
      uint32_t PrimitiveTopologyType;
   } vft;

   /* 3DSTATE_VIEWPORT_STATE_POINTERS_CC */
   struct {
      uint32_t count;
      struct {
         float MinimumDepth;
         float MaximumDepth;
      } elem[MAX_VIEWPORTS];
   } vp_cc;

   /* 3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP */
   struct {
      uint32_t count;
      struct {
         float ViewportMatrixElementm00;
         float ViewportMatrixElementm11;
         float ViewportMatrixElementm22;
         float ViewportMatrixElementm30;
         float ViewportMatrixElementm31;
         float ViewportMatrixElementm32;
         float XMinClipGuardband;
         float XMaxClipGuardband;
         float YMinClipGuardband;
         float YMaxClipGuardband;
         float XMinViewPort;
         float XMaxViewPort;
         float YMinViewPort;
         float YMaxViewPort;
      } elem[MAX_VIEWPORTS];
   } vp_sf_clip;

   /* 3DSTATE_WM */
   struct {
      uint32_t ForceThreadDispatchEnable;
      bool     LineStippleEnable;
   } wm;

   /* 3DSTATE_WM_DEPTH_STENCIL */
   struct {
      bool     DoubleSidedStencilEnable;
      uint32_t StencilTestMask;
      uint32_t StencilWriteMask;
      uint32_t BackfaceStencilTestMask;
      uint32_t BackfaceStencilWriteMask;
      uint32_t StencilReferenceValue;
      uint32_t BackfaceStencilReferenceValue;
      bool     DepthTestEnable;
      bool     DepthBufferWriteEnable;
      uint32_t DepthTestFunction;
      bool     StencilTestEnable;
      bool     StencilBufferWriteEnable;
      uint32_t StencilFailOp;
      uint32_t StencilPassDepthPassOp;
      uint32_t StencilPassDepthFailOp;
      uint32_t StencilTestFunction;
      uint32_t BackfaceStencilFailOp;
      uint32_t BackfaceStencilPassDepthPassOp;
      uint32_t BackfaceStencilPassDepthFailOp;
      uint32_t BackfaceStencilTestFunction;
   } ds;

   /* 3DSTATE_TBIMR_TILE_PASS_INFO */
   struct {
      unsigned TileRectangleHeight;
      unsigned TileRectangleWidth;
      unsigned VerticalTileCount;
      unsigned HorizontalTileCount;
      unsigned TBIMRBatchSize;
      unsigned TileBoxCheck;
   } tbimr;
   bool use_tbimr;

   bool pma_fix;

   BITSET_DECLARE(dirty, ANV_GFX_STATE_MAX);
};

enum anv_internal_kernel_name {
   ANV_INTERNAL_KERNEL_GENERATED_DRAWS,
   ANV_INTERNAL_KERNEL_COPY_QUERY_RESULTS_COMPUTE,
   ANV_INTERNAL_KERNEL_COPY_QUERY_RESULTS_FRAGMENT,
   ANV_INTERNAL_KERNEL_MEMCPY_COMPUTE,

   ANV_INTERNAL_KERNEL_COUNT,
};

struct anv_internal_kernel_bind_map {
   uint32_t num_bindings;
   struct {
      /* Whether this binding is provided through push constants */
      bool     push_constant;

      /* When not provided by push constants, this is offset at which the
       * 64bit address of the binding is located in the push constant data.
       */
      uint32_t address_offset;
   } bindings[5];
   uint32_t push_data_size;
};

enum anv_rt_bvh_build_method {
   ANV_BVH_BUILD_METHOD_TRIVIAL,
   ANV_BVH_BUILD_METHOD_NEW_SAH,
};

struct anv_device_astc_emu {
    struct vk_texcompress_astc_state           *texcompress;

    /* for flush_astc_ldr_void_extent_denorms */
    simple_mtx_t mutex;
    VkDescriptorSetLayout ds_layout;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
};

struct anv_trtt_batch_bo {
   struct anv_bo *bo;
   uint32_t size;

   /* Once device->trtt.timeline_handle signals timeline_val as complete we
    * can free this struct and its members.
    */
   uint64_t timeline_val;

   /* Part of device->trtt.in_flight_batches. */
   struct list_head link;
};

struct anv_device {
    struct vk_device                            vk;

    struct anv_physical_device *                physical;
    const struct intel_device_info *            info;
    const struct anv_kmd_backend *              kmd_backend;
    struct isl_device                           isl_dev;
    union {
       uint32_t                                 context_id; /* i915 */
       uint32_t                                 vm_id; /* Xe */
    };
    int                                         fd;

    pthread_mutex_t                             vma_mutex;
    struct util_vma_heap                        vma_lo;
    struct util_vma_heap                        vma_hi;
    struct util_vma_heap                        vma_desc;
    struct util_vma_heap                        vma_samplers;
    struct util_vma_heap                        vma_trtt;

    /** List of all anv_device_memory objects */
    struct list_head                            memory_objects;

    /** List of anv_image objects with a private binding for implicit CCS */
    struct list_head                            image_private_objects;

    /** Memory pool for batch buffers */
    struct anv_bo_pool                          batch_bo_pool;
    /** Memory pool for utrace timestamp buffers */
    struct anv_bo_pool                          utrace_bo_pool;
    /** Memory pool for BVH build buffers */
    struct anv_bo_pool                          bvh_bo_pool;

    struct anv_bo_cache                         bo_cache;

    struct anv_state_pool                       general_state_pool;
    struct anv_state_pool                       dynamic_state_pool;
    struct anv_state_pool                       instruction_state_pool;
    struct anv_state_pool                       binding_table_pool;
    struct anv_state_pool                       scratch_surface_state_pool;
    struct anv_state_pool                       internal_surface_state_pool;
    struct anv_state_pool                       bindless_surface_state_pool;
    struct anv_state_pool                       indirect_push_descriptor_pool;

    struct anv_state_reserved_pool              custom_border_colors;

    /** BO used for various workarounds
     *
     * There are a number of workarounds on our hardware which require writing
     * data somewhere and it doesn't really matter where.  For that, we use
     * this BO and just write to the first dword or so.
     *
     * We also need to be able to handle NULL buffers bound as pushed UBOs.
     * For that, we use the high bytes (>= 1024) of the workaround BO.
     */
    struct anv_bo *                             workaround_bo;
    struct anv_address                          workaround_address;

    /**
     * Workarounds for game bugs.
     */
    struct {
       struct set *                             doom64_images;
    } workarounds;

    struct anv_bo *                             trivial_batch_bo;
    struct anv_state                            null_surface_state;

    struct vk_pipeline_cache *                  default_pipeline_cache;
    struct vk_pipeline_cache *                  internal_cache;
    struct blorp_context                        blorp;

    struct anv_state                            border_colors;

    struct anv_state                            slice_hash;

    /** An array of CPS_STATE structures grouped by MAX_VIEWPORTS elements
     *
     * We need to emit CPS_STATE structures for each viewport accessible by a
     * pipeline. So rather than write many identical CPS_STATE structures
     * dynamically, we can enumerate all possible combinaisons and then just
     * emit a 3DSTATE_CPS_POINTERS instruction with the right offset into this
     * array.
     */
    struct anv_state                            cps_states;

    uint32_t                                    queue_count;
    struct anv_queue  *                         queues;

    struct anv_scratch_pool                     scratch_pool;
    struct anv_bo                              *rt_scratch_bos[16];
    struct anv_bo                              *btd_fifo_bo;
    struct anv_address                          rt_uuid_addr;

    /** A pre packed VERTEX_ELEMENT_STATE feeding 0s to the VS stage
     *
     * For use when a pipeline has no VS input
     */
    uint32_t                                    empty_vs_input[2];

    bool                                        robust_buffer_access;

    uint32_t                                    protected_session_id;

    /** Shadow ray query BO
     *
     * The ray_query_bo only holds the current ray being traced. When using
     * more than 1 ray query per thread, we cannot fit all the queries in
     * there, so we need a another buffer to hold query data that is not
     * currently being used by the HW for tracing, similar to a scratch space.
     *
     * The size of the shadow buffer depends on the number of queries per
     * shader.
     */
    struct anv_bo                              *ray_query_shadow_bos[16];
    /** Ray query buffer used to communicated with HW unit.
     */
    struct anv_bo                              *ray_query_bo;

    struct anv_shader_bin                      *rt_trampoline;
    struct anv_shader_bin                      *rt_trivial_return;

    enum anv_rt_bvh_build_method                bvh_build_method;

    /** Draw generation shader
     *
     * Generates direct draw calls out of indirect parameters. Used to
     * workaround slowness with indirect draw calls.
     */
    struct anv_shader_bin                      *internal_kernels[ANV_INTERNAL_KERNEL_COUNT];
    const struct intel_l3_config               *internal_kernels_l3_config;

    pthread_mutex_t                             mutex;
    pthread_cond_t                              queue_submit;

    struct intel_batch_decode_ctx               decoder[ANV_MAX_QUEUE_FAMILIES];
    /*
     * When decoding a anv_cmd_buffer, we might need to search for BOs through
     * the cmd_buffer's list.
     */
    struct anv_cmd_buffer                      *cmd_buffer_being_decoded;

    int                                         perf_fd; /* -1 if no opened */
    uint64_t                                    perf_metric; /* 0 if unset */

    struct intel_aux_map_context                *aux_map_ctx;

    const struct intel_l3_config                *l3_config;

    struct intel_debug_block_frame              *debug_frame_desc;

    struct intel_ds_device                       ds;

    nir_shader                                  *fp64_nir;

    uint32_t                                    draw_call_count;
    struct anv_state                            breakpoint;
#ifdef ANDROID
    struct u_gralloc                            *u_gralloc;
#endif

    /** Precompute all dirty graphics bits
     *
     * Depending on platforms, some of the dirty bits don't apply (for example
     * 3DSTATE_PRIMITIVE_REPLICATION is only Gfx12.0+). Disabling some
     * extensions like Mesh shaders also allow us to avoid emitting any
     * mesh/task related instructions (we only initialize them once at device
     * initialization).
     */
    BITSET_DECLARE(gfx_dirty_state, ANV_GFX_STATE_MAX);

    /*
     * Command pool for companion RCS command buffer.
     */
    VkCommandPool                               companion_rcs_cmd_pool;

    struct anv_trtt {
       pthread_mutex_t mutex;

       /* Sometimes we need to run batches from places where we don't have a
        * queue coming from the API, so we use this.
        */
       struct anv_queue *queue;

       /* There's only one L3 table, so if l3_addr is zero that means we
        * didn't initialize the TR-TT context yet (i.e., we're not using TR-TT
        * yet in this context).
        */
       uint64_t l3_addr;

       /* We don't want to access the page tables from the CPU, so just
        * maintain a mirror that we can use.
        */
       uint64_t *l3_mirror;
       uint64_t *l2_mirror;

       /* We keep a dynamic list of page table bos, and each bo can store
        * multiple page tables.
        */
       struct anv_bo **page_table_bos;
       int num_page_table_bos;
       int page_table_bos_capacity;

       /* These are used to keep track of space available for more page tables
        * within a bo.
        */
       struct anv_bo *cur_page_table_bo;
       uint64_t next_page_table_bo_offset;

       /* Timeline syncobj used to track completion of the TR-TT batch BOs. */
       uint32_t timeline_handle;
       uint64_t timeline_val;

       /* List of struct anv_trtt_batch_bo batches that are in flight and can
        * be freed once their timeline gets signaled.
        */
       struct list_head in_flight_batches;
    } trtt;

    /* This is true if the user ever bound a sparse resource to memory. This
     * is used for a workaround that makes every memoryBarrier flush more
     * things than it should. Many applications request for the sparse
     * featuers to be enabled but don't use them, and some create sparse
     * resources but never use them.
     */
    bool                                         using_sparse;

    struct anv_device_astc_emu                   astc_emu;
};

static inline uint32_t
anv_get_first_render_queue_index(struct anv_physical_device *pdevice)
{
   assert(pdevice != NULL);

   for (uint32_t i = 0; i < pdevice->queue.family_count; i++) {
      if (pdevice->queue.families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
         return i;
      }
   }

   unreachable("Graphics capable queue family not found");
}

static inline struct anv_state
anv_binding_table_pool_alloc(struct anv_device *device)
{
   return anv_state_pool_alloc(&device->binding_table_pool,
                               device->binding_table_pool.block_size, 0);
}

static inline void
anv_binding_table_pool_free(struct anv_device *device, struct anv_state state)
{
   anv_state_pool_free(&device->binding_table_pool, state);
}

static inline struct anv_state
anv_null_surface_state_for_binding_table(struct anv_device *device)
{
   struct anv_state state = device->null_surface_state;
   if (device->physical->indirect_descriptors) {
      state.offset += device->physical->va.bindless_surface_state_pool.addr -
                      device->physical->va.internal_surface_state_pool.addr;
   }
   return state;
}

static inline struct anv_state
anv_bindless_state_for_binding_table(struct anv_device *device,
                                     struct anv_state state)
{
   state.offset += device->physical->va.bindless_surface_state_pool.addr -
                   device->physical->va.internal_surface_state_pool.addr;
   return state;
}

static inline uint32_t
anv_mocs(const struct anv_device *device,
         const struct anv_bo *bo,
         isl_surf_usage_flags_t usage)
{
   return isl_mocs(&device->isl_dev, usage, bo && anv_bo_is_external(bo));
}

static inline uint32_t
anv_mocs_for_address(const struct anv_device *device,
                     struct anv_address *addr)
{
   return anv_mocs(device, addr->bo, 0);
}

void anv_device_init_blorp(struct anv_device *device);
void anv_device_finish_blorp(struct anv_device *device);

VkResult anv_device_alloc_bo(struct anv_device *device,
                             const char *name, uint64_t size,
                             enum anv_bo_alloc_flags alloc_flags,
                             uint64_t explicit_address,
                             struct anv_bo **bo);
VkResult anv_device_map_bo(struct anv_device *device,
                           struct anv_bo *bo,
                           uint64_t offset,
                           size_t size,
                           void **map_out);
void anv_device_unmap_bo(struct anv_device *device,
                         struct anv_bo *bo,
                         void *map, size_t map_size);
VkResult anv_device_import_bo_from_host_ptr(struct anv_device *device,
                                            void *host_ptr, uint32_t size,
                                            enum anv_bo_alloc_flags alloc_flags,
                                            uint64_t client_address,
                                            struct anv_bo **bo_out);
VkResult anv_device_import_bo(struct anv_device *device, int fd,
                              enum anv_bo_alloc_flags alloc_flags,
                              uint64_t client_address,
                              struct anv_bo **bo);
VkResult anv_device_export_bo(struct anv_device *device,
                              struct anv_bo *bo, int *fd_out);
VkResult anv_device_get_bo_tiling(struct anv_device *device,
                                  struct anv_bo *bo,
                                  enum isl_tiling *tiling_out);
VkResult anv_device_set_bo_tiling(struct anv_device *device,
                                  struct anv_bo *bo,
                                  uint32_t row_pitch_B,
                                  enum isl_tiling tiling);
void anv_device_release_bo(struct anv_device *device,
                           struct anv_bo *bo);

static inline void anv_device_set_physical(struct anv_device *device,
                                           struct anv_physical_device *physical_device)
{
   device->physical = physical_device;
   device->info = &physical_device->info;
   device->isl_dev = physical_device->isl_dev;
}

static inline struct anv_bo *
anv_device_lookup_bo(struct anv_device *device, uint32_t gem_handle)
{
   return util_sparse_array_get(&device->bo_cache.bo_map, gem_handle);
}

VkResult anv_device_wait(struct anv_device *device, struct anv_bo *bo,
                         int64_t timeout);

VkResult anv_queue_init(struct anv_device *device, struct anv_queue *queue,
                        const VkDeviceQueueCreateInfo *pCreateInfo,
                        uint32_t index_in_family);
void anv_queue_finish(struct anv_queue *queue);

VkResult anv_queue_submit(struct vk_queue *queue,
                          struct vk_queue_submit *submit);
VkResult anv_queue_submit_simple_batch(struct anv_queue *queue,
                                       struct anv_batch *batch,
                                       bool is_companion_rcs_batch);
VkResult anv_queue_submit_trtt_batch(struct anv_sparse_submission *submit,
                                     struct anv_batch *batch);

static inline void
anv_trtt_batch_bo_free(struct anv_device *device,
                       struct anv_trtt_batch_bo *trtt_bbo)
{
   anv_bo_pool_free(&device->batch_bo_pool, trtt_bbo->bo);
   list_del(&trtt_bbo->link);
   vk_free(&device->vk.alloc, trtt_bbo);
}

void anv_queue_trace(struct anv_queue *queue, const char *label,
                     bool frame, bool begin);

void *
anv_gem_mmap(struct anv_device *device, struct anv_bo *bo, uint64_t offset,
             uint64_t size);
void anv_gem_munmap(struct anv_device *device, void *p, uint64_t size);
int anv_gem_wait(struct anv_device *device, uint32_t gem_handle, int64_t *timeout_ns);
int anv_gem_set_tiling(struct anv_device *device, uint32_t gem_handle,
                       uint32_t stride, uint32_t tiling);
int anv_gem_get_tiling(struct anv_device *device, uint32_t gem_handle);
int anv_gem_handle_to_fd(struct anv_device *device, uint32_t gem_handle);
uint32_t anv_gem_fd_to_handle(struct anv_device *device, int fd);
int anv_gem_set_context_param(int fd, uint32_t context, uint32_t param,
                              uint64_t value);
VkResult
anv_gem_import_bo_alloc_flags_to_bo_flags(struct anv_device *device,
                                          struct anv_bo *bo,
                                          enum anv_bo_alloc_flags alloc_flags,
                                          uint32_t *bo_flags);
const struct intel_device_info_pat_entry *
anv_device_get_pat_entry(struct anv_device *device,
                         enum anv_bo_alloc_flags alloc_flags);

uint64_t anv_vma_alloc(struct anv_device *device,
                       uint64_t size, uint64_t align,
                       enum anv_bo_alloc_flags alloc_flags,
                       uint64_t client_address,
                       struct util_vma_heap **out_vma_heap);
void anv_vma_free(struct anv_device *device,
                  struct util_vma_heap *vma_heap,
                  uint64_t address, uint64_t size);

struct anv_reloc_list {
   bool                                         uses_relocs;
   uint32_t                                     dep_words;
   BITSET_WORD *                                deps;
   const VkAllocationCallbacks                  *alloc;
};

VkResult anv_reloc_list_init(struct anv_reloc_list *list,
                             const VkAllocationCallbacks *alloc,
                             bool uses_relocs);
void anv_reloc_list_finish(struct anv_reloc_list *list);

VkResult
anv_reloc_list_add_bo_impl(struct anv_reloc_list *list, struct anv_bo *target_bo);

static inline VkResult
anv_reloc_list_add_bo(struct anv_reloc_list *list, struct anv_bo *target_bo)
{
   return list->uses_relocs ? anv_reloc_list_add_bo_impl(list, target_bo) : VK_SUCCESS;
}

VkResult anv_reloc_list_append(struct anv_reloc_list *list,
                               struct anv_reloc_list *other);

struct anv_batch_bo {
   /* Link in the anv_cmd_buffer.owned_batch_bos list */
   struct list_head                             link;

   struct anv_bo *                              bo;

   /* Bytes actually consumed in this batch BO */
   uint32_t                                     length;

   /* When this batch BO is used as part of a primary batch buffer, this
    * tracked whether it is chained to another primary batch buffer.
    *
    * If this is the case, the relocation list's last entry points the
    * location of the MI_BATCH_BUFFER_START chaining to the next batch.
    */
   bool                                         chained;

   struct anv_reloc_list                        relocs;
};

struct anv_batch {
   const VkAllocationCallbacks *                alloc;

   /**
    * Sum of all the anv_batch_bo sizes allocated for this command buffer.
    * Used to increase allocation size for long command buffers.
    */
   size_t                                       allocated_batch_size;

   struct anv_address                           start_addr;

   void *                                       start;
   void *                                       end;
   void *                                       next;

   struct anv_reloc_list *                      relocs;

   /* This callback is called (with the associated user data) in the event
    * that the batch runs out of space.
    */
   VkResult (*extend_cb)(struct anv_batch *, uint32_t, void *);
   void *                                       user_data;

   /**
    * Current error status of the command buffer. Used to track inconsistent
    * or incomplete command buffer states that are the consequence of run-time
    * errors such as out of memory scenarios. We want to track this in the
    * batch because the command buffer object is not visible to some parts
    * of the driver.
    */
   VkResult                                     status;

   enum intel_engine_class                      engine_class;

   /**
    * Number of 3DPRIMITIVE's emitted for WA 16014538804
    */
   uint8_t num_3d_primitives_emitted;
};

void *anv_batch_emit_dwords(struct anv_batch *batch, int num_dwords);
VkResult anv_batch_emit_ensure_space(struct anv_batch *batch, uint32_t size);
void anv_batch_advance(struct anv_batch *batch, uint32_t size);
void anv_batch_emit_batch(struct anv_batch *batch, struct anv_batch *other);
struct anv_address anv_batch_address(struct anv_batch *batch, void *batch_location);

static inline struct anv_address
anv_batch_current_address(struct anv_batch *batch)
{
   return anv_batch_address(batch, batch->next);
}

static inline void
anv_batch_set_storage(struct anv_batch *batch, struct anv_address addr,
                      void *map, size_t size)
{
   batch->start_addr = addr;
   batch->next = batch->start = map;
   batch->end = map + size;
}

static inline VkResult
anv_batch_set_error(struct anv_batch *batch, VkResult error)
{
   assert(error != VK_SUCCESS);
   if (batch->status == VK_SUCCESS)
      batch->status = error;
   return batch->status;
}

static inline bool
anv_batch_has_error(struct anv_batch *batch)
{
   return batch->status != VK_SUCCESS;
}

static inline uint64_t
_anv_combine_address(struct anv_batch *batch, void *location,
                     const struct anv_address address, uint32_t delta)
{
   if (address.bo == NULL)
      return address.offset + delta;

   if (batch)
      anv_reloc_list_add_bo(batch->relocs, address.bo);

   return anv_address_physical(anv_address_add(address, delta));
}

#define __gen_address_type struct anv_address
#define __gen_user_data struct anv_batch
#define __gen_combine_address _anv_combine_address

/* Wrapper macros needed to work around preprocessor argument issues.  In
 * particular, arguments don't get pre-evaluated if they are concatenated.
 * This means that, if you pass GENX(3DSTATE_PS) into the emit macro, the
 * GENX macro won't get evaluated if the emit macro contains "cmd ## foo".
 * We can work around this easily enough with these helpers.
 */
#define __anv_cmd_length(cmd) cmd ## _length
#define __anv_cmd_length_bias(cmd) cmd ## _length_bias
#define __anv_cmd_header(cmd) cmd ## _header
#define __anv_cmd_pack(cmd) cmd ## _pack
#define __anv_reg_num(reg) reg ## _num

#define anv_pack_struct(dst, struc, ...) do {                              \
      struct struc __template = {                                          \
         __VA_ARGS__                                                       \
      };                                                                   \
      __anv_cmd_pack(struc)(NULL, dst, &__template);                       \
      VG(VALGRIND_CHECK_MEM_IS_DEFINED(dst, __anv_cmd_length(struc) * 4)); \
   } while (0)

#define anv_batch_emitn(batch, n, cmd, ...) ({             \
      void *__dst = anv_batch_emit_dwords(batch, n);       \
      if (__dst) {                                         \
         struct cmd __template = {                         \
            __anv_cmd_header(cmd),                         \
           .DWordLength = n - __anv_cmd_length_bias(cmd),  \
            __VA_ARGS__                                    \
         };                                                \
         __anv_cmd_pack(cmd)(batch, __dst, &__template);   \
      }                                                    \
      __dst;                                               \
   })

#define anv_batch_emit_merge(batch, cmd, pipeline, state, name)         \
   for (struct cmd name = { 0 },                                        \
        *_dst = anv_batch_emit_dwords(batch, __anv_cmd_length(cmd));    \
        __builtin_expect(_dst != NULL, 1);                              \
        ({ uint32_t _partial[__anv_cmd_length(cmd)];                    \
           assert((pipeline)->state.len == __anv_cmd_length(cmd));      \
           __anv_cmd_pack(cmd)(batch, _partial, &name);                 \
           for (uint32_t i = 0; i < __anv_cmd_length(cmd); i++) {       \
              ((uint32_t *)_dst)[i] = _partial[i] |                     \
                 (pipeline)->batch_data[(pipeline)->state.offset + i];  \
           }                                                            \
           VG(VALGRIND_CHECK_MEM_IS_DEFINED(_dst, __anv_cmd_length(cmd) * 4)); \
           _dst = NULL;                                                 \
         }))

#define anv_batch_emit(batch, cmd, name)                            \
   for (struct cmd name = { __anv_cmd_header(cmd) },                    \
        *_dst = anv_batch_emit_dwords(batch, __anv_cmd_length(cmd));    \
        __builtin_expect(_dst != NULL, 1);                              \
        ({ __anv_cmd_pack(cmd)(batch, _dst, &name);                     \
           VG(VALGRIND_CHECK_MEM_IS_DEFINED(_dst, __anv_cmd_length(cmd) * 4)); \
           _dst = NULL;                                                 \
         }))

#define anv_batch_write_reg(batch, reg, name)                           \
   for (struct reg name = {}, *_cont = (struct reg *)1; _cont != NULL;  \
        ({                                                              \
            uint32_t _dw[__anv_cmd_length(reg)];                        \
            __anv_cmd_pack(reg)(NULL, _dw, &name);                      \
            for (unsigned i = 0; i < __anv_cmd_length(reg); i++) {      \
               anv_batch_emit(batch, GENX(MI_LOAD_REGISTER_IMM), lri) { \
                  lri.RegisterOffset   = __anv_reg_num(reg);            \
                  lri.DataDWord        = _dw[i];                        \
               }                                                        \
            }                                                           \
           _cont = NULL;                                                \
         }))

/* #define __gen_get_batch_dwords anv_batch_emit_dwords */
/* #define __gen_get_batch_address anv_batch_address */
/* #define __gen_address_value anv_address_physical */
/* #define __gen_address_offset anv_address_add */

struct anv_device_memory {
   struct vk_device_memory                      vk;

   struct list_head                             link;

   struct anv_bo *                              bo;
   const struct anv_memory_type *               type;

   void *                                       map;
   size_t                                       map_size;

   /* The map, from the user PoV is map + map_delta */
   uint64_t                                     map_delta;
};

/**
 * Header for Vertex URB Entry (VUE)
 */
struct anv_vue_header {
   uint32_t Reserved;
   uint32_t RTAIndex; /* RenderTargetArrayIndex */
   uint32_t ViewportIndex;
   float PointWidth;
};

/** Struct representing a sampled image descriptor
 *
 * This descriptor layout is used for sampled images, bare sampler, and
 * combined image/sampler descriptors.
 */
struct anv_sampled_image_descriptor {
   /** Bindless image handle
    *
    * This is expected to already be shifted such that the 20-bit
    * SURFACE_STATE table index is in the top 20 bits.
    */
   uint32_t image;

   /** Bindless sampler handle
    *
    * This is assumed to be a 32B-aligned SAMPLER_STATE pointer relative
    * to the dynamic state base address.
    */
   uint32_t sampler;
};

/** Struct representing a storage image descriptor */
struct anv_storage_image_descriptor {
   /** Bindless image handles
    *
    * These are expected to already be shifted such that the 20-bit
    * SURFACE_STATE table index is in the top 20 bits.
    */
   uint32_t vanilla;

   /** Image depth
    *
    * By default the HW RESINFO message allows us to query the depth of an image :
    *
    * From the Kaby Lake docs for the RESINFO message:
    *
    *    "Surface Type | ... | Blue
    *    --------------+-----+----------------
    *    SURFTYPE_3D  | ... | (Depth+1)»LOD"
    *
    * With VK_EXT_sliced_view_of_3d, we have to support a slice of a 3D image,
    * meaning at a depth offset with a new depth value potentially reduced
    * from the original image. Unfortunately if we change the Depth value of
    * the image, we then run into issues with Yf/Ys tilings where the HW fetch
    * data at incorrect locations.
    *
    * To solve this, we put the slice depth in the descriptor and recompose
    * the vec3 (width, height, depth) using this field for z and xy using the
    * RESINFO result.
    */
   uint32_t image_depth;
};

/** Struct representing a address/range descriptor
 *
 * The fields of this struct correspond directly to the data layout of
 * nir_address_format_64bit_bounded_global addresses.  The last field is the
 * offset in the NIR address so it must be zero so that when you load the
 * descriptor you get a pointer to the start of the range.
 */
struct anv_address_range_descriptor {
   uint64_t address;
   uint32_t range;
   uint32_t zero;
};

enum anv_descriptor_data {
   /** The descriptor contains a BTI reference to a surface state */
   ANV_DESCRIPTOR_BTI_SURFACE_STATE       = BITFIELD_BIT(0),
   /** The descriptor contains a BTI reference to a sampler state */
   ANV_DESCRIPTOR_BTI_SAMPLER_STATE       = BITFIELD_BIT(1),
   /** The descriptor contains an actual buffer view */
   ANV_DESCRIPTOR_BUFFER_VIEW             = BITFIELD_BIT(2),
   /** The descriptor contains inline uniform data */
   ANV_DESCRIPTOR_INLINE_UNIFORM          = BITFIELD_BIT(3),
   /** anv_address_range_descriptor with a buffer address and range */
   ANV_DESCRIPTOR_INDIRECT_ADDRESS_RANGE  = BITFIELD_BIT(4),
   /** Bindless surface handle (through anv_sampled_image_descriptor) */
   ANV_DESCRIPTOR_INDIRECT_SAMPLED_IMAGE  = BITFIELD_BIT(5),
   /** Storage image handles (through anv_storage_image_descriptor) */
   ANV_DESCRIPTOR_INDIRECT_STORAGE_IMAGE  = BITFIELD_BIT(6),
   /** The descriptor contains a single RENDER_SURFACE_STATE */
   ANV_DESCRIPTOR_SURFACE                 = BITFIELD_BIT(7),
   /** The descriptor contains a SAMPLER_STATE */
   ANV_DESCRIPTOR_SAMPLER                 = BITFIELD_BIT(8),
   /** A tuple of RENDER_SURFACE_STATE & SAMPLER_STATE */
   ANV_DESCRIPTOR_SURFACE_SAMPLER         = BITFIELD_BIT(9),
};

struct anv_descriptor_set_binding_layout {
   /* The type of the descriptors in this binding */
   VkDescriptorType type;

   /* Flags provided when this binding was created */
   VkDescriptorBindingFlags flags;

   /* Bitfield representing the type of data this descriptor contains */
   enum anv_descriptor_data data;

   /* Maximum number of YCbCr texture/sampler planes */
   uint8_t max_plane_count;

   /* Number of array elements in this binding (or size in bytes for inline
    * uniform data)
    */
   uint32_t array_size;

   /* Index into the flattened descriptor set */
   uint32_t descriptor_index;

   /* Index into the dynamic state array for a dynamic buffer, relative to the
    * set.
    */
   int16_t dynamic_offset_index;

   /* Computed surface size from data (for one plane) */
   uint16_t descriptor_data_surface_size;

   /* Computed sampler size from data (for one plane) */
   uint16_t descriptor_data_sampler_size;

   /* Index into the descriptor set buffer views */
   int32_t buffer_view_index;

   /* Offset into the descriptor buffer where the surface descriptor lives */
   uint32_t descriptor_surface_offset;

   /* Offset into the descriptor buffer where the sampler descriptor lives */
   uint16_t descriptor_sampler_offset;

   /* Pre computed surface stride (with multiplane descriptor, the descriptor
    * includes all the planes)
    */
   uint16_t descriptor_surface_stride;

   /* Pre computed sampler stride (with multiplane descriptor, the descriptor
    * includes all the planes)
    */
   uint16_t descriptor_sampler_stride;

   /* Immutable samplers (or NULL if no immutable samplers) */
   struct anv_sampler **immutable_samplers;
};

enum anv_descriptor_set_layout_type {
   ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_UNKNOWN,
   ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_INDIRECT,
   ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT,
   ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_BUFFER,
};

bool anv_descriptor_supports_bindless(const struct anv_physical_device *pdevice,
                                      const struct anv_descriptor_set_binding_layout *binding,
                                      bool sampler);

bool anv_descriptor_requires_bindless(const struct anv_physical_device *pdevice,
                                      const struct anv_descriptor_set_binding_layout *binding,
                                      bool sampler);

struct anv_descriptor_set_layout {
   struct vk_object_base base;

   VkDescriptorSetLayoutCreateFlags flags;

   /* Type of descriptor set layout */
   enum anv_descriptor_set_layout_type type;

   /* Descriptor set layouts can be destroyed at almost any time */
   uint32_t ref_cnt;

   /* Number of bindings in this descriptor set */
   uint32_t binding_count;

   /* Total number of descriptors */
   uint32_t descriptor_count;

   /* Shader stages affected by this descriptor set */
   uint16_t shader_stages;

   /* Number of buffer views in this descriptor set */
   uint32_t buffer_view_count;

   /* Number of dynamic offsets used by this descriptor set */
   uint16_t dynamic_offset_count;

   /* For each dynamic buffer, which VkShaderStageFlagBits stages are using
    * this buffer
    */
   VkShaderStageFlags dynamic_offset_stages[MAX_DYNAMIC_BUFFERS];

   /* Size of the descriptor buffer dedicated to surface states for this
    * descriptor set
    */
   uint32_t descriptor_buffer_surface_size;

   /* Size of the descriptor buffer dedicated to sampler states for this
    * descriptor set
    */
   uint32_t descriptor_buffer_sampler_size;

   /* Bindings in this descriptor set */
   struct anv_descriptor_set_binding_layout binding[0];
};

void anv_descriptor_set_layout_destroy(struct anv_device *device,
                                       struct anv_descriptor_set_layout *layout);

void anv_descriptor_set_layout_print(const struct anv_descriptor_set_layout *layout);

static inline struct anv_descriptor_set_layout *
anv_descriptor_set_layout_ref(struct anv_descriptor_set_layout *layout)
{
   assert(layout && layout->ref_cnt >= 1);
   p_atomic_inc(&layout->ref_cnt);

   return layout;
}

static inline void
anv_descriptor_set_layout_unref(struct anv_device *device,
                                struct anv_descriptor_set_layout *layout)
{
   assert(layout && layout->ref_cnt >= 1);
   if (p_atomic_dec_zero(&layout->ref_cnt))
      anv_descriptor_set_layout_destroy(device, layout);
}

struct anv_descriptor {
   VkDescriptorType type;

   union {
      struct {
         VkImageLayout layout;
         struct anv_image_view *image_view;
         struct anv_sampler *sampler;
      };

      struct {
         struct anv_buffer_view *set_buffer_view;
         struct anv_buffer *buffer;
         uint64_t offset;
         uint64_t range;
         uint64_t bind_range;
      };

      struct anv_buffer_view *buffer_view;

      struct vk_acceleration_structure *accel_struct;
   };
};

struct anv_descriptor_set {
   struct vk_object_base base;

   struct anv_descriptor_pool *pool;
   struct anv_descriptor_set_layout *layout;

   /* Amount of space occupied in the the pool by this descriptor set. It can
    * be larger than the size of the descriptor set.
    */
   uint32_t size;

   /* Is this descriptor set a push descriptor */
   bool is_push;

   /* Bitfield of descriptors for which we need to generate surface states.
    * Only valid for push descriptors
    */
   uint32_t generate_surface_states;

   /* State relative to anv_descriptor_pool::surface_bo */
   struct anv_state desc_surface_mem;
   /* State relative to anv_descriptor_pool::sampler_bo */
   struct anv_state desc_sampler_mem;
   /* Surface state for the descriptor buffer */
   struct anv_state desc_surface_state;

   /* Descriptor set address pointing to desc_surface_mem (we don't need one
    * for sampler because they're never accessed other than by the HW through
    * the shader sampler handle).
    */
   struct anv_address desc_surface_addr;

   struct anv_address desc_sampler_addr;

   /* Descriptor offset from the
    * device->va.internal_surface_state_pool.addr
    *
    * It just needs to be added to the binding table offset to be put into the
    * HW BTI entry.
    */
   uint32_t desc_offset;

   uint32_t buffer_view_count;
   struct anv_buffer_view *buffer_views;

   /* Link to descriptor pool's desc_sets list . */
   struct list_head pool_link;

   uint32_t descriptor_count;
   struct anv_descriptor descriptors[0];
};

static inline bool
anv_descriptor_set_is_push(struct anv_descriptor_set *set)
{
   return set->pool == NULL;
}

struct anv_surface_state_data {
   uint8_t data[ANV_SURFACE_STATE_SIZE];
};

struct anv_buffer_state {
   /** Surface state allocated from the bindless heap
    *
    * Only valid if anv_physical_device::indirect_descriptors is true
    */
   struct anv_state state;

   /** Surface state after genxml packing
    *
    * Only valid if anv_physical_device::indirect_descriptors is false
    */
   struct anv_surface_state_data state_data;
};

struct anv_buffer_view {
   struct vk_buffer_view vk;

   struct anv_address address;

   struct anv_buffer_state general;
   struct anv_buffer_state storage;
};

struct anv_push_descriptor_set {
   struct anv_descriptor_set set;

   /* Put this field right behind anv_descriptor_set so it fills up the
    * descriptors[0] field. */
   struct anv_descriptor descriptors[MAX_PUSH_DESCRIPTORS];

   /** True if the descriptor set buffer has been referenced by a draw or
    * dispatch command.
    */
   bool set_used_on_gpu;

   struct anv_buffer_view buffer_views[MAX_PUSH_DESCRIPTORS];
};

static inline struct anv_address
anv_descriptor_set_address(struct anv_descriptor_set *set)
{
   if (anv_descriptor_set_is_push(set)) {
      /* We have to flag push descriptor set as used on the GPU
       * so that the next time we push descriptors, we grab a new memory.
       */
      struct anv_push_descriptor_set *push_set =
         (struct anv_push_descriptor_set *)set;
      push_set->set_used_on_gpu = true;
   }

   return set->desc_surface_addr;
}

struct anv_descriptor_pool_heap {
   /* BO allocated to back the pool (unused for host pools) */
   struct anv_bo        *bo;

   /* Host memory allocated to back a host pool */
   void                 *host_mem;

   /* Heap tracking allocations in bo/host_mem */
   struct util_vma_heap  heap;

   /* Size of the heap */
   uint32_t              size;
};

struct anv_descriptor_pool {
   struct vk_object_base base;

   struct anv_descriptor_pool_heap surfaces;
   struct anv_descriptor_pool_heap samplers;

   struct anv_state_stream surface_state_stream;
   void *surface_state_free_list;

   /** List of anv_descriptor_set. */
   struct list_head desc_sets;

   /** Heap over host_mem */
   struct util_vma_heap host_heap;

   /** Allocated size of host_mem */
   uint32_t host_mem_size;

   /**
    * VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT. If set, then
    * surface_state_stream is unused.
    */
   bool host_only;

   char host_mem[0];
};

bool
anv_push_descriptor_set_init(struct anv_cmd_buffer *cmd_buffer,
                             struct anv_push_descriptor_set *push_set,
                             struct anv_descriptor_set_layout *layout);

void
anv_push_descriptor_set_finish(struct anv_push_descriptor_set *push_set);

void
anv_descriptor_set_write_image_view(struct anv_device *device,
                                    struct anv_descriptor_set *set,
                                    const VkDescriptorImageInfo * const info,
                                    VkDescriptorType type,
                                    uint32_t binding,
                                    uint32_t element);

void
anv_descriptor_set_write_buffer_view(struct anv_device *device,
                                     struct anv_descriptor_set *set,
                                     VkDescriptorType type,
                                     struct anv_buffer_view *buffer_view,
                                     uint32_t binding,
                                     uint32_t element);

void
anv_descriptor_set_write_buffer(struct anv_device *device,
                                struct anv_descriptor_set *set,
                                VkDescriptorType type,
                                struct anv_buffer *buffer,
                                uint32_t binding,
                                uint32_t element,
                                VkDeviceSize offset,
                                VkDeviceSize range);

void
anv_descriptor_write_surface_state(struct anv_device *device,
                                   struct anv_descriptor *desc,
                                   struct anv_state surface_state);

void
anv_descriptor_set_write_acceleration_structure(struct anv_device *device,
                                                struct anv_descriptor_set *set,
                                                struct vk_acceleration_structure *accel,
                                                uint32_t binding,
                                                uint32_t element);

void
anv_descriptor_set_write_inline_uniform_data(struct anv_device *device,
                                             struct anv_descriptor_set *set,
                                             uint32_t binding,
                                             const void *data,
                                             size_t offset,
                                             size_t size);

void
anv_descriptor_set_write(struct anv_device *device,
                         struct anv_descriptor_set *set_override,
                         uint32_t write_count,
                         const VkWriteDescriptorSet *writes);

void
anv_descriptor_set_write_template(struct anv_device *device,
                                  struct anv_descriptor_set *set,
                                  const struct vk_descriptor_update_template *template,
                                  const void *data);

#define ANV_DESCRIPTOR_SET_NULL             (UINT8_MAX - 4)
#define ANV_DESCRIPTOR_SET_PUSH_CONSTANTS   (UINT8_MAX - 3)
#define ANV_DESCRIPTOR_SET_DESCRIPTORS      (UINT8_MAX - 2)
#define ANV_DESCRIPTOR_SET_NUM_WORK_GROUPS  (UINT8_MAX - 1)
#define ANV_DESCRIPTOR_SET_COLOR_ATTACHMENTS UINT8_MAX

struct anv_pipeline_binding {
   /** Index in the descriptor set
    *
    * This is a flattened index; the descriptor set layout is already taken
    * into account.
    */
   uint32_t index;

   /** Binding in the descriptor set. Not valid for any of the
    * ANV_DESCRIPTOR_SET_*
    */
   uint32_t binding;

   /** Offset in the descriptor buffer
    *
    * Relative to anv_descriptor_set::desc_addr. This is useful for
    * ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT, to generate the binding
    * table entry.
    */
   uint32_t set_offset;

   /** The descriptor set this surface corresponds to.
    *
    * The special ANV_DESCRIPTOR_SET_* values above indicates that this
    * binding is not a normal descriptor set but something else.
    */
   uint8_t set;

   union {
      /** Plane in the binding index for images */
      uint8_t plane;

      /** Input attachment index (relative to the subpass) */
      uint8_t input_attachment_index;

      /** Dynamic offset index
       *
       * For dynamic UBOs and SSBOs, relative to set.
       */
      uint8_t dynamic_offset_index;
   };
};

struct anv_push_range {
   /** Index in the descriptor set */
   uint32_t index;

   /** Descriptor set index */
   uint8_t set;

   /** Dynamic offset index (for dynamic UBOs), relative to set. */
   uint8_t dynamic_offset_index;

   /** Start offset in units of 32B */
   uint8_t start;

   /** Range in units of 32B */
   uint8_t length;
};

struct anv_pipeline_sets_layout {
   struct anv_device *device;

   struct {
      struct anv_descriptor_set_layout *layout;
      uint32_t dynamic_offset_start;
   } set[MAX_SETS];

   enum anv_descriptor_set_layout_type type;

   uint32_t num_sets;
   uint32_t num_dynamic_buffers;
   int push_descriptor_set_index;

   bool independent_sets;

   unsigned char sha1[20];
};

void anv_pipeline_sets_layout_init(struct anv_pipeline_sets_layout *layout,
                                   struct anv_device *device,
                                   bool independent_sets);

void anv_pipeline_sets_layout_fini(struct anv_pipeline_sets_layout *layout);

void anv_pipeline_sets_layout_add(struct anv_pipeline_sets_layout *layout,
                                  uint32_t set_idx,
                                  struct anv_descriptor_set_layout *set_layout);

void anv_pipeline_sets_layout_hash(struct anv_pipeline_sets_layout *layout);

void anv_pipeline_sets_layout_print(const struct anv_pipeline_sets_layout *layout);

struct anv_pipeline_layout {
   struct vk_object_base base;

   struct anv_pipeline_sets_layout sets_layout;
};

const struct anv_descriptor_set_layout *
anv_pipeline_layout_get_push_set(const struct anv_pipeline_sets_layout *layout,
                                 uint8_t *desc_idx);

struct anv_sparse_binding_data {
   uint64_t address;
   uint64_t size;

   /* This is kept only because it's given to us by vma_alloc() and need to be
    * passed back to vma_free(), we have no other particular use for it
    */
   struct util_vma_heap *vma_heap;
};

#define ANV_SPARSE_BLOCK_SIZE (64 * 1024)

static inline bool
anv_sparse_binding_is_enabled(struct anv_device *device)
{
   return device->vk.enabled_features.sparseBinding;
}

static inline bool
anv_sparse_residency_is_enabled(struct anv_device *device)
{
   return device->vk.enabled_features.sparseResidencyBuffer ||
          device->vk.enabled_features.sparseResidencyImage2D ||
          device->vk.enabled_features.sparseResidencyImage3D ||
          device->vk.enabled_features.sparseResidency2Samples ||
          device->vk.enabled_features.sparseResidency4Samples ||
          device->vk.enabled_features.sparseResidency8Samples ||
          device->vk.enabled_features.sparseResidency16Samples ||
          device->vk.enabled_features.sparseResidencyAliased;
}

VkResult anv_init_sparse_bindings(struct anv_device *device,
                                  uint64_t size,
                                  struct anv_sparse_binding_data *sparse,
                                  enum anv_bo_alloc_flags alloc_flags,
                                  uint64_t client_address,
                                  struct anv_address *out_address);
VkResult anv_free_sparse_bindings(struct anv_device *device,
                                  struct anv_sparse_binding_data *sparse);
VkResult anv_sparse_bind_resource_memory(struct anv_device *device,
                                         struct anv_sparse_binding_data *data,
                                         const VkSparseMemoryBind *bind_,
                                         struct anv_sparse_submission *submit);
VkResult anv_sparse_bind_image_memory(struct anv_queue *queue,
                                      struct anv_image *image,
                                      const VkSparseImageMemoryBind *bind,
                                      struct anv_sparse_submission *submit);
VkResult anv_sparse_bind(struct anv_device *device,
                         struct anv_sparse_submission *sparse_submit);

VkSparseImageFormatProperties
anv_sparse_calc_image_format_properties(struct anv_physical_device *pdevice,
                                        VkImageAspectFlags aspect,
                                        VkImageType vk_image_type,
                                        struct isl_surf *surf);
void anv_sparse_calc_miptail_properties(struct anv_device *device,
                                        struct anv_image *image,
                                        VkImageAspectFlags vk_aspect,
                                        uint32_t *imageMipTailFirstLod,
                                        VkDeviceSize *imageMipTailSize,
                                        VkDeviceSize *imageMipTailOffset,
                                        VkDeviceSize *imageMipTailStride);
VkResult anv_sparse_image_check_support(struct anv_physical_device *pdevice,
                                        VkImageCreateFlags flags,
                                        VkImageTiling tiling,
                                        VkSampleCountFlagBits samples,
                                        VkImageType type,
                                        VkFormat format);
VkResult anv_trtt_batch_bo_new(struct anv_device *device, uint32_t batch_size,
                               struct anv_trtt_batch_bo **out_trtt_bbo);

struct anv_buffer {
   struct vk_buffer vk;

   /* Set when bound */
   struct anv_address address;

   struct anv_sparse_binding_data sparse_data;
};

static inline bool
anv_buffer_is_sparse(struct anv_buffer *buffer)
{
   return buffer->vk.create_flags & VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
}

enum anv_cmd_dirty_bits {
   ANV_CMD_DIRTY_PIPELINE                            = 1 << 0,
   ANV_CMD_DIRTY_INDEX_BUFFER                        = 1 << 1,
   ANV_CMD_DIRTY_RENDER_AREA                         = 1 << 2,
   ANV_CMD_DIRTY_RENDER_TARGETS                      = 1 << 3,
   ANV_CMD_DIRTY_XFB_ENABLE                          = 1 << 4,
   ANV_CMD_DIRTY_RESTART_INDEX                       = 1 << 5,
   ANV_CMD_DIRTY_OCCLUSION_QUERY_ACTIVE              = 1 << 6,
};
typedef enum anv_cmd_dirty_bits anv_cmd_dirty_mask_t;

enum anv_pipe_bits {
   ANV_PIPE_DEPTH_CACHE_FLUSH_BIT            = (1 << 0),
   ANV_PIPE_STALL_AT_SCOREBOARD_BIT          = (1 << 1),
   ANV_PIPE_STATE_CACHE_INVALIDATE_BIT       = (1 << 2),
   ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT    = (1 << 3),
   ANV_PIPE_VF_CACHE_INVALIDATE_BIT          = (1 << 4),
   ANV_PIPE_DATA_CACHE_FLUSH_BIT             = (1 << 5),
   ANV_PIPE_TILE_CACHE_FLUSH_BIT             = (1 << 6),
   ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT     = (1 << 10),
   ANV_PIPE_INSTRUCTION_CACHE_INVALIDATE_BIT = (1 << 11),
   ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT    = (1 << 12),
   ANV_PIPE_DEPTH_STALL_BIT                  = (1 << 13),

   /* ANV_PIPE_HDC_PIPELINE_FLUSH_BIT is a precise way to ensure prior data
    * cache work has completed.  Available on Gfx12+.  For earlier Gfx we
    * must reinterpret this flush as ANV_PIPE_DATA_CACHE_FLUSH_BIT.
    */
   ANV_PIPE_HDC_PIPELINE_FLUSH_BIT           = (1 << 14),
   ANV_PIPE_PSS_STALL_SYNC_BIT               = (1 << 15),

   /*
    * This bit flush data-port's Untyped L1 data cache (LSC L1).
    */
   ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT = (1 << 16),

   /* This bit controls the flushing of the engine (Render, Compute) specific
    * entries from the compression cache.
    */
   ANV_PIPE_CCS_CACHE_FLUSH_BIT              = (1 << 17),

   ANV_PIPE_CS_STALL_BIT                     = (1 << 20),
   ANV_PIPE_END_OF_PIPE_SYNC_BIT             = (1 << 21),

   /* This bit does not exist directly in PIPE_CONTROL.  Instead it means that
    * a flush has happened but not a CS stall.  The next time we do any sort
    * of invalidation we need to insert a CS stall at that time.  Otherwise,
    * we would have to CS stall on every flush which could be bad.
    */
   ANV_PIPE_NEEDS_END_OF_PIPE_SYNC_BIT       = (1 << 22),

   /* This bit does not exist directly in PIPE_CONTROL. It means that Gfx12
    * AUX-TT data has changed and we need to invalidate AUX-TT data.  This is
    * done by writing the AUX-TT register.
    */
   ANV_PIPE_AUX_TABLE_INVALIDATE_BIT         = (1 << 23),

   /* This bit does not exist directly in PIPE_CONTROL. It means that a
    * PIPE_CONTROL with a post-sync operation will follow. This is used to
    * implement a workaround for Gfx9.
    */
   ANV_PIPE_POST_SYNC_BIT                    = (1 << 24),
};

/* These bits track the state of buffer writes for queries. They get cleared
 * based on PIPE_CONTROL emissions.
 */
enum anv_query_bits {
   ANV_QUERY_WRITES_RT_FLUSH      = (1 << 0),

   ANV_QUERY_WRITES_TILE_FLUSH    = (1 << 1),

   ANV_QUERY_WRITES_CS_STALL      = (1 << 2),

   ANV_QUERY_WRITES_DATA_FLUSH    = (1 << 3),
};

/* It's not clear why DG2 doesn't have issues with L3/CS coherency. But it's
 * likely related to performance workaround 14015868140.
 *
 * For now we enable this only on DG2 and platform prior to Gfx12 where there
 * is no tile cache.
 */
#define ANV_DEVINFO_HAS_COHERENT_L3_CS(devinfo) \
   (intel_device_info_is_dg2(devinfo))

/* Things we need to flush before accessing query data using the command
 * streamer.
 *
 * Prior to DG2 experiments show that the command streamer is not coherent
 * with the tile cache so we need to flush it to make any data visible to CS.
 *
 * Otherwise we want to flush the RT cache which is where blorp writes, either
 * for clearing the query buffer or for clearing the destination buffer in
 * vkCopyQueryPoolResults().
 */
#define ANV_QUERY_RENDER_TARGET_WRITES_PENDING_BITS(devinfo) \
   (((devinfo->verx10 >= 120 && \
      devinfo->verx10 < 125) ? ANV_QUERY_WRITES_TILE_FLUSH : 0) | \
   ANV_QUERY_WRITES_RT_FLUSH | \
   ANV_QUERY_WRITES_CS_STALL)
#define ANV_QUERY_COMPUTE_WRITES_PENDING_BITS \
   (ANV_QUERY_WRITES_DATA_FLUSH | \
    ANV_QUERY_WRITES_CS_STALL)

#define ANV_PIPE_QUERY_BITS(pending_query_bits) ( \
   ((pending_query_bits & ANV_QUERY_WRITES_RT_FLUSH) ?   \
    ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT : 0) | \
   ((pending_query_bits & ANV_QUERY_WRITES_TILE_FLUSH) ?   \
    ANV_PIPE_TILE_CACHE_FLUSH_BIT : 0) | \
   ((pending_query_bits & ANV_QUERY_WRITES_CS_STALL) ?   \
    ANV_PIPE_CS_STALL_BIT : 0) | \
   ((pending_query_bits & ANV_QUERY_WRITES_DATA_FLUSH) ?  \
    (ANV_PIPE_DATA_CACHE_FLUSH_BIT | \
     ANV_PIPE_HDC_PIPELINE_FLUSH_BIT | \
     ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT) : 0))

#define ANV_PIPE_FLUSH_BITS ( \
   ANV_PIPE_DEPTH_CACHE_FLUSH_BIT | \
   ANV_PIPE_DATA_CACHE_FLUSH_BIT | \
   ANV_PIPE_HDC_PIPELINE_FLUSH_BIT | \
   ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT | \
   ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT | \
   ANV_PIPE_TILE_CACHE_FLUSH_BIT)

#define ANV_PIPE_STALL_BITS ( \
   ANV_PIPE_STALL_AT_SCOREBOARD_BIT | \
   ANV_PIPE_DEPTH_STALL_BIT | \
   ANV_PIPE_CS_STALL_BIT | \
   ANV_PIPE_PSS_STALL_SYNC_BIT)

#define ANV_PIPE_INVALIDATE_BITS ( \
   ANV_PIPE_STATE_CACHE_INVALIDATE_BIT | \
   ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT | \
   ANV_PIPE_VF_CACHE_INVALIDATE_BIT | \
   ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT | \
   ANV_PIPE_INSTRUCTION_CACHE_INVALIDATE_BIT | \
   ANV_PIPE_AUX_TABLE_INVALIDATE_BIT)

/* PIPE_CONTROL bits that should be set only in 3D RCS mode.
 * For more details see genX(emit_apply_pipe_flushes).
 */
#define ANV_PIPE_GFX_BITS ( \
   ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT | \
   ANV_PIPE_DEPTH_CACHE_FLUSH_BIT | \
   ANV_PIPE_TILE_CACHE_FLUSH_BIT | \
   ANV_PIPE_DEPTH_STALL_BIT | \
   ANV_PIPE_STALL_AT_SCOREBOARD_BIT | \
   (GFX_VERx10 >= 125 ? ANV_PIPE_PSS_STALL_SYNC_BIT : 0) | \
   ANV_PIPE_VF_CACHE_INVALIDATE_BIT)

/* PIPE_CONTROL bits that should be set only in Media/GPGPU RCS mode.
 * For more details see genX(emit_apply_pipe_flushes).
 *
 * Documentation says that untyped L1 dataport cache flush is controlled by
 * HDC pipeline flush in 3D mode according to HDC_CHICKEN0 register:
 *
 * BSpec 47112: PIPE_CONTROL::HDC Pipeline Flush:
 *
 *    "When the "Pipeline Select" mode in PIPELINE_SELECT command is set to
 *     "3D", HDC Pipeline Flush can also flush/invalidate the LSC Untyped L1
 *     cache based on the programming of HDC_Chicken0 register bits 13:11."
 *
 *    "When the 'Pipeline Select' mode is set to 'GPGPU', the LSC Untyped L1
 *     cache flush is controlled by 'Untyped Data-Port Cache Flush' bit in the
 *     PIPE_CONTROL command."
 *
 *    As part of Wa_22010960976 & Wa_14013347512, i915 is programming
 *    HDC_CHICKEN0[11:13] = 0 ("Untyped L1 is flushed, for both 3D Pipecontrol
 *    Dataport flush, and UAV coherency barrier event"). So there is no need
 *    to set "Untyped Data-Port Cache" in 3D mode.
 *
 * On MTL the HDC_CHICKEN0 default values changed to match what was programmed
 * by Wa_22010960976 & Wa_14013347512 on DG2, but experiments show that the
 * change runs a bit deeper. Even manually writing to the HDC_CHICKEN0
 * register to force L1 untyped flush with HDC pipeline flush has no effect on
 * MTL.
 *
 * It seems like the HW change completely disconnected L1 untyped flush from
 * HDC pipeline flush with no way to bring that behavior back. So leave the L1
 * untyped flush active in 3D mode on all platforms since it doesn't seems to
 * cause issues there too.
 *
 * Maybe we'll have some GPGPU only bits here at some point.
 */
#define ANV_PIPE_GPGPU_BITS (0)

enum intel_ds_stall_flag
anv_pipe_flush_bit_to_ds_stall_flag(enum anv_pipe_bits bits);

#define VK_IMAGE_ASPECT_PLANES_BITS_ANV ( \
   VK_IMAGE_ASPECT_PLANE_0_BIT | \
   VK_IMAGE_ASPECT_PLANE_1_BIT | \
   VK_IMAGE_ASPECT_PLANE_2_BIT)

#define VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV (         \
   VK_IMAGE_ASPECT_COLOR_BIT | \
   VK_IMAGE_ASPECT_PLANES_BITS_ANV)

struct anv_vertex_binding {
   struct anv_buffer *                          buffer;
   VkDeviceSize                                 offset;
   VkDeviceSize                                 size;
};

struct anv_xfb_binding {
   struct anv_buffer *                          buffer;
   VkDeviceSize                                 offset;
   VkDeviceSize                                 size;
};

struct anv_push_constants {
   /** Push constant data provided by the client through vkPushConstants */
   uint8_t client_data[MAX_PUSH_CONSTANTS_SIZE];

#define ANV_DESCRIPTOR_SET_DYNAMIC_INDEX_MASK ((uint32_t)ANV_UBO_ALIGNMENT - 1)
#define ANV_DESCRIPTOR_SET_OFFSET_MASK        (~(uint32_t)(ANV_UBO_ALIGNMENT - 1))

   /**
    * Base offsets for descriptor sets from
    * INDIRECT_DESCRIPTOR_POOL_MIN_ADDRESS
    *
    * In bits [0:5] : dynamic offset index in dynamic_offsets[] for the set
    *
    * In bits [6:63] : descriptor set address
    */
   uint32_t desc_surface_offsets[MAX_SETS];

   /**
    * Base offsets for descriptor sets from
    */
   uint32_t desc_sampler_offsets[MAX_SETS];

   /** Dynamic offsets for dynamic UBOs and SSBOs */
   uint32_t dynamic_offsets[MAX_DYNAMIC_BUFFERS];

   union {
      struct {
         /** Dynamic MSAA value */
         uint32_t fs_msaa_flags;

         /** Dynamic TCS input vertices */
         uint32_t tcs_input_vertices;
      } gfx;

      struct {
         /** Base workgroup ID
          *
          * Used for vkCmdDispatchBase.
          */
         uint32_t base_work_group_id[3];

         /** Subgroup ID
          *
          * This is never set by software but is implicitly filled out when
          * uploading the push constants for compute shaders.
          */
         uint32_t subgroup_id;
      } cs;
   };

   /* Robust access pushed registers. */
   uint64_t push_reg_mask[MESA_SHADER_STAGES];

   /** Ray query globals (RT_DISPATCH_GLOBALS) */
   uint64_t ray_query_globals;
};

struct anv_surface_state {
   /** Surface state allocated from the bindless heap
    *
    * Can be NULL if unused.
    */
   struct anv_state state;

   /** Surface state after genxml packing
    *
    * Same data as in state.
    */
   struct anv_surface_state_data state_data;

   /** Address of the surface referred to by this state
    *
    * This address is relative to the start of the BO.
    */
   struct anv_address address;
   /* Address of the aux surface, if any
    *
    * This field is ANV_NULL_ADDRESS if and only if no aux surface exists.
    *
    * With the exception of gfx8, the bottom 12 bits of this address' offset
    * include extra aux information.
    */
   struct anv_address aux_address;
   /* Address of the clear color, if any
    *
    * This address is relative to the start of the BO.
    */
   struct anv_address clear_address;
};

struct anv_attachment {
   VkFormat vk_format;
   const struct anv_image_view *iview;
   VkImageLayout layout;
   enum isl_aux_usage aux_usage;
   struct anv_surface_state surface_state;

   VkResolveModeFlagBits resolve_mode;
   const struct anv_image_view *resolve_iview;
   VkImageLayout resolve_layout;
};

/** State tracking for vertex buffer flushes
 *
 * On Gfx8-9, the VF cache only considers the bottom 32 bits of memory
 * addresses.  If you happen to have two vertex buffers which get placed
 * exactly 4 GiB apart and use them in back-to-back draw calls, you can get
 * collisions.  In order to solve this problem, we track vertex address ranges
 * which are live in the cache and invalidate the cache if one ever exceeds 32
 * bits.
 */
struct anv_vb_cache_range {
   /* Virtual address at which the live vertex buffer cache range starts for
    * this vertex buffer index.
    */
   uint64_t start;

   /* Virtual address of the byte after where vertex buffer cache range ends.
    * This is exclusive such that end - start is the size of the range.
    */
   uint64_t end;
};

static inline void
anv_merge_vb_cache_range(struct anv_vb_cache_range *dirty,
                         const struct anv_vb_cache_range *bound)
{
   if (dirty->start == dirty->end) {
      *dirty = *bound;
   } else if (bound->start != bound->end) {
      dirty->start = MIN2(dirty->start, bound->start);
      dirty->end = MAX2(dirty->end, bound->end);
   }
}

/* Check whether we need to apply the Gfx8-9 vertex buffer workaround*/
static inline bool
anv_gfx8_9_vb_cache_range_needs_workaround(struct anv_vb_cache_range *bound,
                                           struct anv_vb_cache_range *dirty,
                                           struct anv_address vb_address,
                                           uint32_t vb_size)
{
   if (vb_size == 0) {
      bound->start = 0;
      bound->end = 0;
      return false;
   }

   bound->start = intel_48b_address(anv_address_physical(vb_address));
   bound->end = bound->start + vb_size;
   assert(bound->end > bound->start); /* No overflow */

   /* Align everything to a cache line */
   bound->start &= ~(64ull - 1ull);
   bound->end = align64(bound->end, 64);

   anv_merge_vb_cache_range(dirty, bound);

   /* If our range is larger than 32 bits, we have to flush */
   assert(bound->end - bound->start <= (1ull << 32));
   return (dirty->end - dirty->start) > (1ull << 32);
}

/**
 * State tracking for simple internal shaders
 */
struct anv_simple_shader {
   /* The device associated with this emission */
   struct anv_device *device;
   /* The command buffer associated with this emission (can be NULL) */
   struct anv_cmd_buffer *cmd_buffer;
   /* State stream used for various internal allocations */
   struct anv_state_stream *dynamic_state_stream;
   struct anv_state_stream *general_state_stream;
   /* Where to emit the commands (can be different from cmd_buffer->batch) */
   struct anv_batch *batch;
   /* Shader to use */
   struct anv_shader_bin *kernel;
   /* L3 config used by the shader */
   const struct intel_l3_config *l3_config;

   /* Managed by the simpler shader helper*/
   struct anv_state bt_state;
};

/** State tracking for particular pipeline bind point
 *
 * This struct is the base struct for anv_cmd_graphics_state and
 * anv_cmd_compute_state.  These are used to track state which is bound to a
 * particular type of pipeline.  Generic state that applies per-stage such as
 * binding table offsets and push constants is tracked generically with a
 * per-stage array in anv_cmd_state.
 */
struct anv_cmd_pipeline_state {
   struct anv_descriptor_set *descriptors[MAX_SETS];
   struct anv_push_descriptor_set push_descriptor;

   struct anv_push_constants push_constants;

   /* Push constant state allocated when flushing push constants. */
   struct anv_state          push_constants_state;

   /**
    * Dynamic buffer offsets.
    *
    * We have a maximum of MAX_DYNAMIC_BUFFERS per pipeline, but with
    * independent sets we cannot know which how much in total is going to be
    * used. As a result we need to store the maximum possible number per set.
    *
    * Those values are written into anv_push_constants::dynamic_offsets at
    * flush time when have the pipeline with the final
    * anv_pipeline_sets_layout.
    */
   struct {
      uint32_t                                  offsets[MAX_DYNAMIC_BUFFERS];
   }                                            dynamic_offsets[MAX_SETS];

   /**
    * The current bound pipeline.
    */
   struct anv_pipeline      *pipeline;
};

/** State tracking for graphics pipeline
 *
 * This has anv_cmd_pipeline_state as a base struct to track things which get
 * bound to a graphics pipeline.  Along with general pipeline bind point state
 * which is in the anv_cmd_pipeline_state base struct, it also contains other
 * state which is graphics-specific.
 */
struct anv_cmd_graphics_state {
   struct anv_cmd_pipeline_state base;

   VkRenderingFlags rendering_flags;
   VkRect2D render_area;
   uint32_t layer_count;
   uint32_t samples;
   uint32_t view_mask;
   uint32_t color_att_count;
   struct anv_state att_states;
   struct anv_attachment color_att[MAX_RTS];
   struct anv_attachment depth_att;
   struct anv_attachment stencil_att;
   struct anv_state null_surface_state;

   anv_cmd_dirty_mask_t dirty;
   uint32_t vb_dirty;

   struct anv_vb_cache_range ib_bound_range;
   struct anv_vb_cache_range ib_dirty_range;
   struct anv_vb_cache_range vb_bound_ranges[33];
   struct anv_vb_cache_range vb_dirty_ranges[33];

   uint32_t restart_index;

   VkShaderStageFlags push_constant_stages;

   uint32_t primitive_topology;
   bool used_task_shader;

   struct anv_buffer *index_buffer;
   uint32_t index_type; /**< 3DSTATE_INDEX_BUFFER.IndexFormat */
   uint32_t index_offset;
   uint32_t index_size;

   struct vk_vertex_input_state vertex_input;
   struct vk_sample_locations_state sample_locations;

   /**
    * The latest BLEND_STATE structure packed in dynamic state heap
    */
   struct anv_state blend_states;

   bool object_preemption;
   bool has_uint_rt;

   /* State tracking for Wa_14018912822. */
   bool color_blend_zero;
   bool alpha_blend_zero;

   /**
    * DEPTH and STENCIL attachment write state for Wa_18019816803.
    */
   bool ds_write_state;

   /**
    * State tracking for Wa_18020335297.
    */
   bool                                         viewport_set;

   uint32_t n_occlusion_queries;

   struct anv_gfx_dynamic_state dyn_state;
};

enum anv_depth_reg_mode {
   ANV_DEPTH_REG_MODE_UNKNOWN = 0,
   ANV_DEPTH_REG_MODE_HW_DEFAULT,
   ANV_DEPTH_REG_MODE_D16_1X_MSAA,
};

/** State tracking for compute pipeline
 *
 * This has anv_cmd_pipeline_state as a base struct to track things which get
 * bound to a compute pipeline.  Along with general pipeline bind point state
 * which is in the anv_cmd_pipeline_state base struct, it also contains other
 * state which is compute-specific.
 */
struct anv_cmd_compute_state {
   struct anv_cmd_pipeline_state base;

   bool pipeline_dirty;

   struct anv_state push_data;

   struct anv_address num_workgroups;

   uint32_t scratch_size;
};

struct anv_cmd_ray_tracing_state {
   struct anv_cmd_pipeline_state base;

   bool pipeline_dirty;

   struct {
      struct anv_bo *bo;
      struct brw_rt_scratch_layout layout;
   } scratch;

   struct anv_address build_priv_mem_addr;
   size_t             build_priv_mem_size;
};

/** State required while building cmd buffer */
struct anv_cmd_state {
   /* PIPELINE_SELECT.PipelineSelection */
   uint32_t                                     current_pipeline;
   const struct intel_l3_config *               current_l3_config;
   uint32_t                                     last_aux_map_state;

   struct anv_cmd_graphics_state                gfx;
   struct anv_cmd_compute_state                 compute;
   struct anv_cmd_ray_tracing_state             rt;

   enum anv_pipe_bits                           pending_pipe_bits;

   struct {
      /**
       * Tracks operations susceptible to interfere with queries in the
       * destination buffer of vkCmdCopyQueryResults, we need those operations to
       * have completed before we do the work of vkCmdCopyQueryResults.
       */
      enum anv_query_bits                          buffer_write_bits;

      /**
       * Tracks clear operations of query buffers that can interact with
       * vkCmdQueryBegin*, vkCmdWriteTimestamp*,
       * vkCmdWriteAccelerationStructuresPropertiesKHR, etc...
       *
       * We need the clearing of the buffer completed before with write data with
       * the command streamer or a shader.
       */
      enum anv_query_bits                          clear_bits;
   } queries;

   VkShaderStageFlags                           descriptors_dirty;
   VkShaderStageFlags                           push_descriptors_dirty;
   VkShaderStageFlags                           push_constants_dirty;

   struct anv_vertex_binding                    vertex_bindings[MAX_VBS];
   bool                                         xfb_enabled;
   struct anv_xfb_binding                       xfb_bindings[MAX_XFB_BUFFERS];
   struct anv_state                             binding_tables[MESA_VULKAN_SHADER_STAGES];
   struct anv_state                             samplers[MESA_VULKAN_SHADER_STAGES];

   unsigned char                                sampler_sha1s[MESA_VULKAN_SHADER_STAGES][20];
   unsigned char                                surface_sha1s[MESA_VULKAN_SHADER_STAGES][20];
   unsigned char                                push_sha1s[MESA_VULKAN_SHADER_STAGES][20];

   /**
    * Whether or not the gfx8 PMA fix is enabled.  We ensure that, at the top
    * of any command buffer it is disabled by disabling it in EndCommandBuffer
    * and before invoking the secondary in ExecuteCommands.
    */
   bool                                         pma_fix_enabled;

   /**
    * Whether or not we know for certain that HiZ is enabled for the current
    * subpass.  If, for whatever reason, we are unsure as to whether HiZ is
    * enabled or not, this will be false.
    */
   bool                                         hiz_enabled;

   /* We ensure the registers for the gfx12 D16 fix are initialized at the
    * first non-NULL depth stencil packet emission of every command buffer.
    * For secondary command buffer execution, we transfer the state from the
    * last command buffer to the primary (if known).
    */
   enum anv_depth_reg_mode                      depth_reg_mode;

   /**
    * Whether RHWO optimization is enabled (Wa_1508744258).
    */
   bool                                         rhwo_optimization_enabled;

   /**
    * Pending state of the RHWO optimization, to be applied at the next
    * genX(cmd_buffer_apply_pipe_flushes).
    */
   bool                                         pending_rhwo_optimization_enabled;

   bool                                         conditional_render_enabled;

   /**
    * Last rendering scale argument provided to
    * genX(cmd_buffer_emit_hashing_mode)().
    */
   unsigned                                     current_hash_scale;

   /**
    * A buffer used for spill/fill of ray queries.
    */
   struct anv_bo *                              ray_query_shadow_bo;
};

#define ANV_MIN_CMD_BUFFER_BATCH_SIZE 8192
#define ANV_MAX_CMD_BUFFER_BATCH_SIZE (16 * 1024 * 1024)

enum anv_cmd_buffer_exec_mode {
   ANV_CMD_BUFFER_EXEC_MODE_PRIMARY,
   ANV_CMD_BUFFER_EXEC_MODE_EMIT,
   ANV_CMD_BUFFER_EXEC_MODE_GROW_AND_EMIT,
   ANV_CMD_BUFFER_EXEC_MODE_CHAIN,
   ANV_CMD_BUFFER_EXEC_MODE_COPY_AND_CHAIN,
   ANV_CMD_BUFFER_EXEC_MODE_CALL_AND_RETURN,
};

struct anv_measure_batch;

struct anv_cmd_buffer {
   struct vk_command_buffer                     vk;

   struct anv_device *                          device;
   struct anv_queue_family *                    queue_family;

   /** Batch where the main commands live */
   struct anv_batch                             batch;

   /* Pointer to the location in the batch where MI_BATCH_BUFFER_END was
    * recorded upon calling vkEndCommandBuffer(). This is useful if we need to
    * rewrite the end to chain multiple batch together at vkQueueSubmit().
    */
   void *                                       batch_end;

   /* Fields required for the actual chain of anv_batch_bo's.
    *
    * These fields are initialized by anv_cmd_buffer_init_batch_bo_chain().
    */
   struct list_head                             batch_bos;
   enum anv_cmd_buffer_exec_mode                exec_mode;

   /* A vector of anv_batch_bo pointers for every batch or surface buffer
    * referenced by this command buffer
    *
    * initialized by anv_cmd_buffer_init_batch_bo_chain()
    */
   struct u_vector                            seen_bbos;

   /* A vector of int32_t's for every block of binding tables.
    *
    * initialized by anv_cmd_buffer_init_batch_bo_chain()
    */
   struct u_vector                              bt_block_states;
   struct anv_state                             bt_next;

   struct anv_reloc_list                        surface_relocs;

   /* Serial for tracking buffer completion */
   uint32_t                                     serial;

   /* Stream objects for storing temporary data */
   struct anv_state_stream                      surface_state_stream;
   struct anv_state_stream                      dynamic_state_stream;
   struct anv_state_stream                      general_state_stream;
   struct anv_state_stream                      indirect_push_descriptor_stream;

   VkCommandBufferUsageFlags                    usage_flags;

   struct anv_query_pool                       *perf_query_pool;

   struct anv_cmd_state                         state;

   struct anv_address                           return_addr;

   /* Set by SetPerformanceMarkerINTEL, written into queries by CmdBeginQuery */
   uint64_t                                     intel_perf_marker;

   struct anv_measure_batch *measure;

   /**
    * KHR_performance_query requires self modifying command buffers and this
    * array has the location of modifying commands to the query begin and end
    * instructions storing performance counters. The array length is
    * anv_physical_device::n_perf_query_commands.
    */
   struct mi_address_token                  *self_mod_locations;

   /**
    * Index tracking which of the self_mod_locations items have already been
    * used.
    */
   uint32_t                                      perf_reloc_idx;

   /**
    * Sum of all the anv_batch_bo written sizes for this command buffer
    * including any executed secondary command buffer.
    */
   uint32_t                                     total_batch_size;

   struct {
      /** Batch generating part of the anv_cmd_buffer::batch */
      struct anv_batch                          batch;

      /**
       * Location in anv_cmd_buffer::batch at which we left some space to
       * insert a MI_BATCH_BUFFER_START into the
       * anv_cmd_buffer::generation::batch if needed.
       */
      struct anv_address                        jump_addr;

      /**
       * Location in anv_cmd_buffer::batch at which the generation batch
       * should jump back to.
       */
      struct anv_address                        return_addr;

      /** List of anv_batch_bo used for generation
       *
       * We have to keep this separated of the anv_cmd_buffer::batch_bos that
       * is used for a chaining optimization.
       */
      struct list_head                          batch_bos;

      /** Ring buffer of generated commands
       *
       * When generating draws in ring mode, this buffer will hold generated
       * 3DPRIMITIVE commands.
       */
      struct anv_bo                            *ring_bo;

      /**
       * State tracking of the generation shader (only used for the non-ring
       * mode).
       */
      struct anv_simple_shader                  shader_state;
   } generation;

   /**
    * A vector of anv_bo pointers for chunks of memory used by the command
    * buffer that are too large to be allocated through dynamic_state_stream.
    * This is the case for large enough acceleration structures.
    *
    * initialized by anv_cmd_buffer_init_batch_bo_chain()
    */
   struct u_vector                              dynamic_bos;

   /**
    * Structure holding tracepoints recorded in the command buffer.
    */
   struct u_trace                               trace;

   /** Pointer to the last emitted COMPUTE_WALKER.
    *
    * This is used to edit the instruction post emission to replace the "Post
    * Sync" field for utrace timestamp emission.
    */
   void                                        *last_compute_walker;

   /** Pointer to the last emitted EXECUTE_INDIRECT_DISPATCH.
    *
    * This is used to edit the instruction post emission to replace the "Post
    * Sync" field for utrace timestamp emission.
    */
   void                                        *last_indirect_dispatch;

   struct {
      struct anv_video_session *vid;
      struct anv_video_session_params *params;
   } video;

   /**
    * Companion RCS command buffer to support the MSAA operations on compute
    * queue.
    */
   struct anv_cmd_buffer                        *companion_rcs_cmd_buffer;

   /**
    * Whether this command buffer is a companion command buffer of compute one.
    */
   bool                                         is_companion_rcs_cmd_buffer;

};

extern const struct vk_command_buffer_ops anv_cmd_buffer_ops;

/* Determine whether we can chain a given cmd_buffer to another one. We need
 * to make sure that we can edit the end of the batch to point to next one,
 * which requires the command buffer to not be used simultaneously.
 *
 * We could in theory also implement chaining with companion command buffers,
 * but let's sparse ourselves some pain and misery. This optimization has no
 * benefit on the brand new Xe kernel driver.
 */
static inline bool
anv_cmd_buffer_is_chainable(struct anv_cmd_buffer *cmd_buffer)
{
   return !(cmd_buffer->usage_flags &
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT) &&
          !(cmd_buffer->is_companion_rcs_cmd_buffer);
}

static inline bool
anv_cmd_buffer_is_render_queue(const struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_queue_family *queue_family = cmd_buffer->queue_family;
   return (queue_family->queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
}

static inline bool
anv_cmd_buffer_is_video_queue(const struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_queue_family *queue_family = cmd_buffer->queue_family;
   return (queue_family->queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) != 0;
}

static inline bool
anv_cmd_buffer_is_compute_queue(const struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_queue_family *queue_family = cmd_buffer->queue_family;
   return queue_family->engine_class == INTEL_ENGINE_CLASS_COMPUTE;
}

static inline bool
anv_cmd_buffer_is_blitter_queue(const struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_queue_family *queue_family = cmd_buffer->queue_family;
   return queue_family->engine_class == INTEL_ENGINE_CLASS_COPY;
}

VkResult anv_cmd_buffer_init_batch_bo_chain(struct anv_cmd_buffer *cmd_buffer);
void anv_cmd_buffer_fini_batch_bo_chain(struct anv_cmd_buffer *cmd_buffer);
void anv_cmd_buffer_reset_batch_bo_chain(struct anv_cmd_buffer *cmd_buffer);
void anv_cmd_buffer_end_batch_buffer(struct anv_cmd_buffer *cmd_buffer);
void anv_cmd_buffer_add_secondary(struct anv_cmd_buffer *primary,
                                  struct anv_cmd_buffer *secondary);
void anv_cmd_buffer_prepare_execbuf(struct anv_cmd_buffer *cmd_buffer);
VkResult anv_cmd_buffer_execbuf(struct anv_queue *queue,
                                struct anv_cmd_buffer *cmd_buffer,
                                const VkSemaphore *in_semaphores,
                                const uint64_t *in_wait_values,
                                uint32_t num_in_semaphores,
                                const VkSemaphore *out_semaphores,
                                const uint64_t *out_signal_values,
                                uint32_t num_out_semaphores,
                                VkFence fence,
                                int perf_query_pass);

void anv_cmd_buffer_reset(struct vk_command_buffer *vk_cmd_buffer,
                          UNUSED VkCommandBufferResetFlags flags);

struct anv_state anv_cmd_buffer_emit_dynamic(struct anv_cmd_buffer *cmd_buffer,
                                             const void *data, uint32_t size, uint32_t alignment);
struct anv_state anv_cmd_buffer_merge_dynamic(struct anv_cmd_buffer *cmd_buffer,
                                              uint32_t *a, uint32_t *b,
                                              uint32_t dwords, uint32_t alignment);

struct anv_address
anv_cmd_buffer_surface_base_address(struct anv_cmd_buffer *cmd_buffer);
struct anv_state
anv_cmd_buffer_alloc_binding_table(struct anv_cmd_buffer *cmd_buffer,
                                   uint32_t entries, uint32_t *state_offset);
struct anv_state
anv_cmd_buffer_alloc_surface_states(struct anv_cmd_buffer *cmd_buffer,
                                    uint32_t count);
struct anv_state
anv_cmd_buffer_alloc_dynamic_state(struct anv_cmd_buffer *cmd_buffer,
                                   uint32_t size, uint32_t alignment);
struct anv_state
anv_cmd_buffer_alloc_general_state(struct anv_cmd_buffer *cmd_buffer,
                                   uint32_t size, uint32_t alignment);

void
anv_cmd_buffer_chain_command_buffers(struct anv_cmd_buffer **cmd_buffers,
                                     uint32_t num_cmd_buffers);
void
anv_cmd_buffer_exec_batch_debug(struct anv_queue *queue,
                                uint32_t cmd_buffer_count,
                                struct anv_cmd_buffer **cmd_buffers,
                                struct anv_query_pool *perf_query_pool,
                                uint32_t perf_query_pass);
void
anv_cmd_buffer_clflush(struct anv_cmd_buffer **cmd_buffers,
                       uint32_t num_cmd_buffers);

void
anv_cmd_buffer_update_pending_query_bits(struct anv_cmd_buffer *cmd_buffer,
                                         enum anv_pipe_bits flushed_bits);

/**
 * A allocation tied to a command buffer.
 *
 * Don't use anv_cmd_alloc::address::map to write memory from userspace, use
 * anv_cmd_alloc::map instead.
 */
struct anv_cmd_alloc {
   struct anv_address  address;
   void               *map;
   size_t              size;
};

#define ANV_EMPTY_ALLOC ((struct anv_cmd_alloc) { .map = NULL, .size = 0 })

static inline bool
anv_cmd_alloc_is_empty(struct anv_cmd_alloc alloc)
{
   return alloc.size == 0;
}

struct anv_cmd_alloc
anv_cmd_buffer_alloc_space(struct anv_cmd_buffer *cmd_buffer,
                           size_t size, uint32_t alignment,
                           bool private);

VkResult
anv_cmd_buffer_new_binding_table_block(struct anv_cmd_buffer *cmd_buffer);

void anv_cmd_buffer_emit_state_base_address(struct anv_cmd_buffer *cmd_buffer);

struct anv_state
anv_cmd_buffer_gfx_push_constants(struct anv_cmd_buffer *cmd_buffer);
struct anv_state
anv_cmd_buffer_cs_push_constants(struct anv_cmd_buffer *cmd_buffer);

VkResult
anv_cmd_buffer_alloc_blorp_binding_table(struct anv_cmd_buffer *cmd_buffer,
                                         uint32_t num_entries,
                                         uint32_t *state_offset,
                                         struct anv_state *bt_state);

void anv_cmd_buffer_dump(struct anv_cmd_buffer *cmd_buffer);

void anv_cmd_emit_conditional_render_predicate(struct anv_cmd_buffer *cmd_buffer);

static inline unsigned
anv_cmd_buffer_get_view_count(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   return MAX2(1, util_bitcount(gfx->view_mask));
}

/* Save/restore cmd buffer states for meta operations */
enum anv_cmd_saved_state_flags {
   ANV_CMD_SAVED_STATE_COMPUTE_PIPELINE         = BITFIELD_BIT(0),
   ANV_CMD_SAVED_STATE_DESCRIPTOR_SET_0         = BITFIELD_BIT(1),
   ANV_CMD_SAVED_STATE_PUSH_CONSTANTS           = BITFIELD_BIT(2),
};

struct anv_cmd_saved_state {
   uint32_t flags;

   struct anv_pipeline *pipeline;
   struct anv_descriptor_set *descriptor_set;
   uint8_t push_constants[MAX_PUSH_CONSTANTS_SIZE];
};

void anv_cmd_buffer_save_state(struct anv_cmd_buffer *cmd_buffer,
                               uint32_t flags,
                               struct anv_cmd_saved_state *state);

void anv_cmd_buffer_restore_state(struct anv_cmd_buffer *cmd_buffer,
                                  struct anv_cmd_saved_state *state);

enum anv_bo_sync_state {
   /** Indicates that this is a new (or newly reset fence) */
   ANV_BO_SYNC_STATE_RESET,

   /** Indicates that this fence has been submitted to the GPU but is still
    * (as far as we know) in use by the GPU.
    */
   ANV_BO_SYNC_STATE_SUBMITTED,

   ANV_BO_SYNC_STATE_SIGNALED,
};

struct anv_bo_sync {
   struct vk_sync sync;

   enum anv_bo_sync_state state;
   struct anv_bo *bo;
};

extern const struct vk_sync_type anv_bo_sync_type;

static inline bool
vk_sync_is_anv_bo_sync(const struct vk_sync *sync)
{
   return sync->type == &anv_bo_sync_type;
}

VkResult anv_create_sync_for_memory(struct vk_device *device,
                                    VkDeviceMemory memory,
                                    bool signal_memory,
                                    struct vk_sync **sync_out);

struct anv_event {
   struct vk_object_base                        base;
   uint64_t                                     semaphore;
   struct anv_state                             state;
};

#define ANV_STAGE_MASK ((1 << MESA_VULKAN_SHADER_STAGES) - 1)

#define anv_foreach_stage(stage, stage_bits)                         \
   for (gl_shader_stage stage,                                       \
        __tmp = (gl_shader_stage)((stage_bits) & ANV_STAGE_MASK);    \
        stage = __builtin_ffs(__tmp) - 1, __tmp;                     \
        __tmp &= ~(1 << (stage)))

struct anv_pipeline_bind_map {
   unsigned char                                surface_sha1[20];
   unsigned char                                sampler_sha1[20];
   unsigned char                                push_sha1[20];

   uint32_t surface_count;
   uint32_t sampler_count;
   uint16_t kernel_args_size;
   uint16_t kernel_arg_count;

   struct anv_pipeline_binding *                surface_to_descriptor;
   struct anv_pipeline_binding *                sampler_to_descriptor;
   struct brw_kernel_arg_desc *                 kernel_args;

   struct anv_push_range                        push_ranges[4];
};

struct anv_push_descriptor_info {
   /* A bitfield of descriptors used. */
   uint32_t used_descriptors;

   /* A bitfield of UBOs bindings fully promoted to push constants. */
   uint32_t fully_promoted_ubo_descriptors;

   /* */
   uint8_t used_set_buffer;
};

/* A list of values we push to implement some of the dynamic states */
enum anv_dynamic_push_bits {
   ANV_DYNAMIC_PUSH_INPUT_VERTICES = BITFIELD_BIT(0),
};

struct anv_shader_bin {
   struct vk_pipeline_cache_object base;

   gl_shader_stage stage;

   struct anv_state kernel;
   uint32_t kernel_size;

   const struct brw_stage_prog_data *prog_data;
   uint32_t prog_data_size;

   struct brw_compile_stats stats[3];
   uint32_t num_stats;

   struct nir_xfb_info *xfb_info;

   struct anv_push_descriptor_info push_desc_info;

   struct anv_pipeline_bind_map bind_map;

   enum anv_dynamic_push_bits dynamic_push_values;
};

struct anv_shader_bin *
anv_shader_bin_create(struct anv_device *device,
                      gl_shader_stage stage,
                      const void *key, uint32_t key_size,
                      const void *kernel, uint32_t kernel_size,
                      const struct brw_stage_prog_data *prog_data,
                      uint32_t prog_data_size,
                      const struct brw_compile_stats *stats, uint32_t num_stats,
                      const struct nir_xfb_info *xfb_info,
                      const struct anv_pipeline_bind_map *bind_map,
                      const struct anv_push_descriptor_info *push_desc_info,
                      enum anv_dynamic_push_bits dynamic_push_values);


static inline struct anv_shader_bin *
anv_shader_bin_ref(struct anv_shader_bin *shader)
{
   vk_pipeline_cache_object_ref(&shader->base);

   return shader;
}

static inline void
anv_shader_bin_unref(struct anv_device *device, struct anv_shader_bin *shader)
{
   vk_pipeline_cache_object_unref(&device->vk, &shader->base);
}

struct anv_pipeline_executable {
   gl_shader_stage stage;

   struct brw_compile_stats stats;

   char *nir;
   char *disasm;
};

enum anv_pipeline_type {
   ANV_PIPELINE_GRAPHICS,
   ANV_PIPELINE_GRAPHICS_LIB,
   ANV_PIPELINE_COMPUTE,
   ANV_PIPELINE_RAY_TRACING,
};

struct anv_pipeline {
   struct vk_object_base                        base;

   struct anv_device *                          device;

   struct anv_batch                             batch;
   struct anv_reloc_list                        batch_relocs;

   void *                                       mem_ctx;

   enum anv_pipeline_type                       type;
   VkPipelineCreateFlags                        flags;

   VkPipelineCreateFlags2KHR                    active_stages;

   uint32_t                                     ray_queries;

   /**
    * Mask of stages that are accessing push descriptors.
    */
   VkShaderStageFlags                           use_push_descriptor;

   /**
    * Mask of stages that are accessing the push descriptors buffer.
    */
   VkShaderStageFlags                           use_push_descriptor_buffer;

   /**
    * Maximum scratch size for all shaders in this pipeline.
    */
   uint32_t                                     scratch_size;

   /* Layout of the sets used by the pipeline. */
   struct anv_pipeline_sets_layout              layout;

   struct util_dynarray                         executables;

   const struct intel_l3_config *               l3_config;
};

/* The base graphics pipeline object only hold shaders. */
struct anv_graphics_base_pipeline {
   struct anv_pipeline                          base;

   struct vk_sample_locations_state             sample_locations;

   /* Shaders */
   struct anv_shader_bin *                      shaders[ANV_GRAPHICS_SHADER_STAGE_COUNT];

   /* A small hash based of shader_info::source_sha1 for identifying
    * shaders in renderdoc/shader-db.
    */
   uint32_t                                     source_hashes[ANV_GRAPHICS_SHADER_STAGE_COUNT];

   /* Feedback index in
    * VkPipelineCreationFeedbackCreateInfo::pPipelineStageCreationFeedbacks
    *
    * For pipeline libraries, we need to remember the order at creation when
    * included into a linked pipeline.
    */
   uint32_t                                     feedback_index[ANV_GRAPHICS_SHADER_STAGE_COUNT];

   /* Robustness flags used shaders
    */
   enum brw_robustness_flags                    robust_flags[ANV_GRAPHICS_SHADER_STAGE_COUNT];

   /* True if at the time the fragment shader was compiled, it didn't have all
    * the information to avoid BRW_WM_MSAA_FLAG_ENABLE_DYNAMIC.
    */
   bool                                         fragment_dynamic;
};

/* The library graphics pipeline object has a partial graphic state and
 * possibly some shaders. If requested, shaders are also present in NIR early
 * form.
 */
struct anv_graphics_lib_pipeline {
   struct anv_graphics_base_pipeline            base;

   VkGraphicsPipelineLibraryFlagsEXT            lib_flags;

   struct vk_graphics_pipeline_all_state        all_state;
   struct vk_graphics_pipeline_state            state;

   /* Retained shaders for link optimization. */
   struct {
      /* This hash is the same as computed in
       * anv_graphics_pipeline_gather_shaders().
       */
      unsigned char                             shader_sha1[20];

      enum gl_subgroup_size                     subgroup_size_type;

      /* NIR captured in anv_pipeline_stage_get_nir(), includes specialization
       * constants.
       */
      nir_shader *                              nir;
   }                                            retained_shaders[ANV_GRAPHICS_SHADER_STAGE_COUNT];

   /* Whether the shaders have been retained */
   bool                                         retain_shaders;
};

struct anv_gfx_state_ptr {
   /* Both in dwords */
   uint16_t  offset;
   uint16_t  len;
};

/* The final graphics pipeline object has all the graphics state ready to be
 * programmed into HW packets (dynamic_state field) or fully baked in its
 * batch.
 */
struct anv_graphics_pipeline {
   struct anv_graphics_base_pipeline            base;

   struct vk_vertex_input_state                 vertex_input;
   struct vk_sample_locations_state             sample_locations;
   struct vk_dynamic_graphics_state             dynamic_state;

   /* If true, the patch control points are passed through push constants
    * (anv_push_constants::gfx::tcs_input_vertices)
    */
   bool                                         dynamic_patch_control_points;

   /* This field is required with dynamic primitive topology,
    * rasterization_samples used only with gen < 8.
    */
   uint32_t                                     rasterization_samples;

   uint32_t                                     view_mask;
   uint32_t                                     instance_multiplier;

   bool                                         kill_pixel;
   bool                                         force_fragment_thread_dispatch;
   bool                                         uses_xfb;
   bool                                         primitive_id_override;

   /* Number of VERTEX_ELEMENT_STATE input elements used by the shader */
   uint32_t                                     vs_input_elements;

   /* Number of VERTEX_ELEMENT_STATE elements we need to implement some of the
    * draw parameters
    */
   uint32_t                                     svgs_count;

   /* Pre computed VERTEX_ELEMENT_STATE structures for the vertex input that
    * can be copied into the anv_cmd_buffer behind a 3DSTATE_VERTEX_BUFFER.
    *
    * When MESA_VK_DYNAMIC_VI is not dynamic
    *
    *     vertex_input_elems = vs_input_elements + svgs_count
    *
    * All the VERTEX_ELEMENT_STATE can be directly copied behind a
    * 3DSTATE_VERTEX_ELEMENTS instruction in the command buffer. Otherwise
    * this array only holds the svgs_count elements.
    */
   uint32_t                                     vertex_input_elems;
   uint32_t                                     vertex_input_data[2 * 31 /* MAX_VES + 2 internal */];

   enum brw_wm_msaa_flags                       fs_msaa_flags;

   /* Pre computed CS instructions that can directly be copied into
    * anv_cmd_buffer.
    */
   uint32_t                                     batch_data[416];

   /* Fully backed instructions, ready to be emitted in the anv_cmd_buffer */
   struct {
      struct anv_gfx_state_ptr                  urb;
      struct anv_gfx_state_ptr                  vf_statistics;
      struct anv_gfx_state_ptr                  vf_sgvs;
      struct anv_gfx_state_ptr                  vf_sgvs_2;
      struct anv_gfx_state_ptr                  vf_sgvs_instancing;
      struct anv_gfx_state_ptr                  vf_instancing;
      struct anv_gfx_state_ptr                  primitive_replication;
      struct anv_gfx_state_ptr                  sbe;
      struct anv_gfx_state_ptr                  sbe_swiz;
      struct anv_gfx_state_ptr                  so_decl_list;
      struct anv_gfx_state_ptr                  ms;
      struct anv_gfx_state_ptr                  vs;
      struct anv_gfx_state_ptr                  hs;
      struct anv_gfx_state_ptr                  ds;
      struct anv_gfx_state_ptr                  ps;
      struct anv_gfx_state_ptr                  ps_extra;

      struct anv_gfx_state_ptr                  task_control;
      struct anv_gfx_state_ptr                  task_shader;
      struct anv_gfx_state_ptr                  task_redistrib;
      struct anv_gfx_state_ptr                  clip_mesh;
      struct anv_gfx_state_ptr                  mesh_control;
      struct anv_gfx_state_ptr                  mesh_shader;
      struct anv_gfx_state_ptr                  mesh_distrib;
      struct anv_gfx_state_ptr                  sbe_mesh;
   } final;

   /* Pre packed CS instructions & structures that need to be merged later
    * with dynamic state.
    */
   struct {
      struct anv_gfx_state_ptr                  clip;
      struct anv_gfx_state_ptr                  sf;
      struct anv_gfx_state_ptr                  raster;
      struct anv_gfx_state_ptr                  wm;
      struct anv_gfx_state_ptr                  so;
      struct anv_gfx_state_ptr                  gs;
      struct anv_gfx_state_ptr                  te;
      struct anv_gfx_state_ptr                  vfg;
   } partial;
};

#define anv_batch_merge_pipeline_state(batch, dwords0, pipeline, state) \
   do {                                                                 \
      uint32_t *dw;                                                     \
                                                                        \
      assert(ARRAY_SIZE(dwords0) == (pipeline)->state.len);             \
      dw = anv_batch_emit_dwords((batch), ARRAY_SIZE(dwords0));         \
      if (!dw)                                                          \
         break;                                                         \
      for (uint32_t i = 0; i < ARRAY_SIZE(dwords0); i++)                \
         dw[i] = (dwords0)[i] |                                         \
            (pipeline)->batch_data[(pipeline)->state.offset + i];       \
      VG(VALGRIND_CHECK_MEM_IS_DEFINED(dw, ARRAY_SIZE(dwords0) * 4));   \
   } while (0)

#define anv_batch_emit_pipeline_state(batch, pipeline, state)           \
   do {                                                                 \
      if ((pipeline)->state.len == 0)                                   \
         break;                                                         \
      uint32_t *dw;                                                     \
      dw = anv_batch_emit_dwords((batch), (pipeline)->state.len);       \
      if (!dw)                                                          \
         break;                                                         \
      memcpy(dw, &(pipeline)->batch_data[(pipeline)->state.offset],     \
             4 * (pipeline)->state.len);                                \
   } while (0)


struct anv_compute_pipeline {
   struct anv_pipeline                          base;

   struct anv_shader_bin *                      cs;
   uint32_t                                     batch_data[9];
   uint32_t                                     interface_descriptor_data[8];

   /* A small hash based of shader_info::source_sha1 for identifying shaders
    * in renderdoc/shader-db.
    */
   uint32_t                                     source_hash;
};

struct anv_rt_shader_group {
   VkRayTracingShaderGroupTypeKHR type;

   struct anv_shader_bin *general;
   struct anv_shader_bin *closest_hit;
   struct anv_shader_bin *any_hit;
   struct anv_shader_bin *intersection;

   /* VK_KHR_ray_tracing requires shaderGroupHandleSize == 32 */
   uint32_t handle[8];
};

struct anv_ray_tracing_pipeline {
   struct anv_pipeline                          base;

   /* All shaders in the pipeline */
   struct util_dynarray                         shaders;

   uint32_t                                     group_count;
   struct anv_rt_shader_group *                 groups;

   /* If non-zero, this is the default computed stack size as per the stack
    * size computation in the Vulkan spec.  If zero, that indicates that the
    * client has requested a dynamic stack size.
    */
   uint32_t                                     stack_size;
};

#define ANV_DECL_PIPELINE_DOWNCAST(pipe_type, pipe_enum)             \
   static inline struct anv_##pipe_type##_pipeline *                 \
   anv_pipeline_to_##pipe_type(struct anv_pipeline *pipeline)      \
   {                                                                 \
      assert(pipeline->type == pipe_enum);                           \
      return (struct anv_##pipe_type##_pipeline *) pipeline;         \
   }

ANV_DECL_PIPELINE_DOWNCAST(graphics, ANV_PIPELINE_GRAPHICS)
ANV_DECL_PIPELINE_DOWNCAST(graphics_base, ANV_PIPELINE_GRAPHICS)
ANV_DECL_PIPELINE_DOWNCAST(graphics_lib, ANV_PIPELINE_GRAPHICS_LIB)
ANV_DECL_PIPELINE_DOWNCAST(compute, ANV_PIPELINE_COMPUTE)
ANV_DECL_PIPELINE_DOWNCAST(ray_tracing, ANV_PIPELINE_RAY_TRACING)

static inline bool
anv_pipeline_has_stage(const struct anv_graphics_pipeline *pipeline,
                       gl_shader_stage stage)
{
   return (pipeline->base.base.active_stages & mesa_to_vk_shader_stage(stage)) != 0;
}

static inline bool
anv_pipeline_base_has_stage(const struct anv_graphics_base_pipeline *pipeline,
                            gl_shader_stage stage)
{
   return (pipeline->base.active_stages & mesa_to_vk_shader_stage(stage)) != 0;
}

static inline bool
anv_pipeline_is_primitive(const struct anv_graphics_pipeline *pipeline)
{
   return anv_pipeline_has_stage(pipeline, MESA_SHADER_VERTEX);
}

static inline bool
anv_pipeline_is_mesh(const struct anv_graphics_pipeline *pipeline)
{
   return anv_pipeline_has_stage(pipeline, MESA_SHADER_MESH);
}

static inline bool
anv_cmd_buffer_all_color_write_masked(const struct anv_cmd_buffer *cmd_buffer)
{
   const struct anv_cmd_graphics_state *state = &cmd_buffer->state.gfx;
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;
   uint8_t color_writes = dyn->cb.color_write_enables;

   /* All writes disabled through vkCmdSetColorWriteEnableEXT */
   if ((color_writes & ((1u << state->color_att_count) - 1)) == 0)
      return true;

   /* Or all write masks are empty */
   for (uint32_t i = 0; i < state->color_att_count; i++) {
      if (dyn->cb.attachments[i].write_mask != 0)
         return false;
   }

   return true;
}

static inline void
anv_cmd_graphic_state_update_has_uint_rt(struct anv_cmd_graphics_state *state)
{
   state->has_uint_rt = false;
   for (unsigned a = 0; a < state->color_att_count; a++) {
      if (vk_format_is_int(state->color_att[a].vk_format)) {
         state->has_uint_rt = true;
         break;
      }
   }
}

#define ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(prefix, stage)             \
static inline const struct brw_##prefix##_prog_data *                   \
get_##prefix##_prog_data(const struct anv_graphics_pipeline *pipeline)  \
{                                                                       \
   if (anv_pipeline_has_stage(pipeline, stage)) {                       \
      return (const struct brw_##prefix##_prog_data *)                  \
         pipeline->base.shaders[stage]->prog_data;                      \
   } else {                                                             \
      return NULL;                                                      \
   }                                                                    \
}

ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(vs, MESA_SHADER_VERTEX)
ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(tcs, MESA_SHADER_TESS_CTRL)
ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(tes, MESA_SHADER_TESS_EVAL)
ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(gs, MESA_SHADER_GEOMETRY)
ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(wm, MESA_SHADER_FRAGMENT)
ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(mesh, MESA_SHADER_MESH)
ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(task, MESA_SHADER_TASK)

static inline const struct brw_cs_prog_data *
get_cs_prog_data(const struct anv_compute_pipeline *pipeline)
{
   assert(pipeline->cs);
   return (const struct brw_cs_prog_data *) pipeline->cs->prog_data;
}

static inline const struct brw_vue_prog_data *
anv_pipeline_get_last_vue_prog_data(const struct anv_graphics_pipeline *pipeline)
{
   if (anv_pipeline_has_stage(pipeline, MESA_SHADER_GEOMETRY))
      return &get_gs_prog_data(pipeline)->base;
   else if (anv_pipeline_has_stage(pipeline, MESA_SHADER_TESS_EVAL))
      return &get_tes_prog_data(pipeline)->base;
   else
      return &get_vs_prog_data(pipeline)->base;
}

VkResult
anv_device_init_rt_shaders(struct anv_device *device);

void
anv_device_finish_rt_shaders(struct anv_device *device);

struct anv_kernel_arg {
   bool is_ptr;
   uint16_t size;

   union {
      uint64_t u64;
      void *ptr;
   };
};

struct anv_kernel {
#ifndef NDEBUG
   const char *name;
#endif
   struct anv_shader_bin *bin;
   const struct intel_l3_config *l3_config;
};

struct anv_format_plane {
   enum isl_format isl_format:16;
   struct isl_swizzle swizzle;

   /* What aspect is associated to this plane */
   VkImageAspectFlags aspect;
};

struct anv_format {
   struct anv_format_plane planes[3];
   VkFormat vk_format;
   uint8_t n_planes;
   bool can_ycbcr;
   bool can_video;
};

static inline void
anv_assert_valid_aspect_set(VkImageAspectFlags aspects)
{
   if (util_bitcount(aspects) == 1) {
      assert(aspects & (VK_IMAGE_ASPECT_COLOR_BIT |
                        VK_IMAGE_ASPECT_DEPTH_BIT |
                        VK_IMAGE_ASPECT_STENCIL_BIT |
                        VK_IMAGE_ASPECT_PLANE_0_BIT |
                        VK_IMAGE_ASPECT_PLANE_1_BIT |
                        VK_IMAGE_ASPECT_PLANE_2_BIT));
   } else if (aspects & VK_IMAGE_ASPECT_PLANES_BITS_ANV) {
      assert(aspects == VK_IMAGE_ASPECT_PLANE_0_BIT ||
             aspects == (VK_IMAGE_ASPECT_PLANE_0_BIT |
                         VK_IMAGE_ASPECT_PLANE_1_BIT) ||
             aspects == (VK_IMAGE_ASPECT_PLANE_0_BIT |
                         VK_IMAGE_ASPECT_PLANE_1_BIT |
                         VK_IMAGE_ASPECT_PLANE_2_BIT));
   } else {
      assert(aspects == (VK_IMAGE_ASPECT_DEPTH_BIT |
                         VK_IMAGE_ASPECT_STENCIL_BIT));
   }
}

/**
 * Return the aspect's plane relative to all_aspects.  For an image, for
 * instance, all_aspects would be the set of aspects in the image.  For
 * an image view, all_aspects would be the subset of aspects represented
 * by that particular view.
 */
static inline uint32_t
anv_aspect_to_plane(VkImageAspectFlags all_aspects,
                    VkImageAspectFlagBits aspect)
{
   anv_assert_valid_aspect_set(all_aspects);
   assert(util_bitcount(aspect) == 1);
   assert(!(aspect & ~all_aspects));

   /* Because we always put image and view planes in aspect-bit-order, the
    * plane index is the number of bits in all_aspects before aspect.
    */
   return util_bitcount(all_aspects & (aspect - 1));
}

#define anv_foreach_image_aspect_bit(b, image, aspects) \
   u_foreach_bit(b, vk_image_expand_aspect_mask(&(image)->vk, aspects))

const struct anv_format *
anv_get_format(VkFormat format);

static inline uint32_t
anv_get_format_planes(VkFormat vk_format)
{
   const struct anv_format *format = anv_get_format(vk_format);

   return format != NULL ? format->n_planes : 0;
}

struct anv_format_plane
anv_get_format_plane(const struct intel_device_info *devinfo,
                     VkFormat vk_format, uint32_t plane,
                     VkImageTiling tiling);

struct anv_format_plane
anv_get_format_aspect(const struct intel_device_info *devinfo,
                      VkFormat vk_format,
                      VkImageAspectFlagBits aspect, VkImageTiling tiling);

static inline enum isl_format
anv_get_isl_format(const struct intel_device_info *devinfo, VkFormat vk_format,
                   VkImageAspectFlags aspect, VkImageTiling tiling)
{
   return anv_get_format_aspect(devinfo, vk_format, aspect, tiling).isl_format;
}

bool anv_format_supports_ccs_e(const struct intel_device_info *devinfo,
                               const enum isl_format format);

bool anv_formats_ccs_e_compatible(const struct intel_device_info *devinfo,
                                  VkImageCreateFlags create_flags,
                                  VkFormat vk_format, VkImageTiling vk_tiling,
                                  VkImageUsageFlags vk_usage,
                                  const VkImageFormatListCreateInfo *fmt_list);

extern VkFormat
vk_format_from_android(unsigned android_format, unsigned android_usage);

static inline VkFormat
anv_get_emulation_format(const struct anv_physical_device *pdevice, VkFormat format)
{
   if (pdevice->flush_astc_ldr_void_extent_denorms) {
      const struct util_format_description *desc =
         vk_format_description(format);
      if (desc->layout == UTIL_FORMAT_LAYOUT_ASTC &&
          desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB)
         return format;
   }

   if (pdevice->emu_astc_ldr)
      return vk_texcompress_astc_emulation_format(format);

   return VK_FORMAT_UNDEFINED;
}

static inline bool
anv_is_format_emulated(const struct anv_physical_device *pdevice, VkFormat format)
{
   return anv_get_emulation_format(pdevice, format) != VK_FORMAT_UNDEFINED;
}

static inline struct isl_swizzle
anv_swizzle_for_render(struct isl_swizzle swizzle)
{
   /* Sometimes the swizzle will have alpha map to one.  We do this to fake
    * RGB as RGBA for texturing
    */
   assert(swizzle.a == ISL_CHANNEL_SELECT_ONE ||
          swizzle.a == ISL_CHANNEL_SELECT_ALPHA);

   /* But it doesn't matter what we render to that channel */
   swizzle.a = ISL_CHANNEL_SELECT_ALPHA;

   return swizzle;
}

void
anv_pipeline_setup_l3_config(struct anv_pipeline *pipeline, bool needs_slm);

/**
 * Describes how each part of anv_image will be bound to memory.
 */
struct anv_image_memory_range {
   /**
    * Disjoint bindings into which each portion of the image will be bound.
    *
    * Binding images to memory can be complicated and invold binding different
    * portions of the image to different memory objects or regions.  For most
    * images, everything lives in the MAIN binding and gets bound by
    * vkBindImageMemory.  For disjoint multi-planar images, each plane has
    * a unique, disjoint binding and gets bound by vkBindImageMemory2 with
    * VkBindImagePlaneMemoryInfo.  There may also exist bits of memory which are
    * implicit or driver-managed and live in special-case bindings.
    */
   enum anv_image_memory_binding {
      /**
       * Used if and only if image is not multi-planar disjoint. Bound by
       * vkBindImageMemory2 without VkBindImagePlaneMemoryInfo.
       */
      ANV_IMAGE_MEMORY_BINDING_MAIN,

      /**
       * Used if and only if image is multi-planar disjoint.  Bound by
       * vkBindImageMemory2 with VkBindImagePlaneMemoryInfo.
       */
      ANV_IMAGE_MEMORY_BINDING_PLANE_0,
      ANV_IMAGE_MEMORY_BINDING_PLANE_1,
      ANV_IMAGE_MEMORY_BINDING_PLANE_2,

      /**
       * Driver-private bo. In special cases we may store the aux surface and/or
       * aux state in this binding.
       */
      ANV_IMAGE_MEMORY_BINDING_PRIVATE,

      /** Sentinel */
      ANV_IMAGE_MEMORY_BINDING_END,
   } binding;

   /**
    * Offset is relative to the start of the binding created by
    * vkBindImageMemory, not to the start of the bo.
    */
   uint64_t offset;

   uint64_t size;
   uint32_t alignment;
};

/**
 * Subsurface of an anv_image.
 */
struct anv_surface {
   struct isl_surf isl;
   struct anv_image_memory_range memory_range;
};

static inline bool MUST_CHECK
anv_surface_is_valid(const struct anv_surface *surface)
{
   return surface->isl.size_B > 0 && surface->memory_range.size > 0;
}

struct anv_image {
   struct vk_image vk;

   uint32_t n_planes;

   /**
    * Image has multi-planar format and was created with
    * VK_IMAGE_CREATE_DISJOINT_BIT.
    */
   bool disjoint;

   /**
    * Image is a WSI image
    */
   bool from_wsi;

   /**
    * Image was imported from an struct AHardwareBuffer.  We have to delay
    * final image creation until bind time.
    */
   bool from_ahb;

   /**
    * Image was imported from gralloc with VkNativeBufferANDROID. The gralloc bo
    * must be released when the image is destroyed.
    */
   bool from_gralloc;

   /**
    * If not UNDEFINED, image has a hidden plane at planes[n_planes] for ASTC
    * LDR workaround or emulation.
    */
   VkFormat emu_plane_format;

   /**
    * The memory bindings created by vkCreateImage and vkBindImageMemory.
    *
    * For details on the image's memory layout, see check_memory_bindings().
    *
    * vkCreateImage constructs the `memory_range` for each
    * anv_image_memory_binding.  After vkCreateImage, each binding is valid if
    * and only if `memory_range::size > 0`.
    *
    * vkBindImageMemory binds each valid `memory_range` to an `address`.
    * Usually, the app will provide the address via the parameters of
    * vkBindImageMemory.  However, special-case bindings may be bound to
    * driver-private memory.
    */
   struct anv_image_binding {
      struct anv_image_memory_range memory_range;
      struct anv_address address;
      struct anv_sparse_binding_data sparse_data;
   } bindings[ANV_IMAGE_MEMORY_BINDING_END];

   /**
    * Image subsurfaces
    *
    * For each foo, anv_image::planes[x].surface is valid if and only if
    * anv_image::aspects has a x aspect. Refer to anv_image_aspect_to_plane()
    * to figure the number associated with a given aspect.
    *
    * The hardware requires that the depth buffer and stencil buffer be
    * separate surfaces.  From Vulkan's perspective, though, depth and stencil
    * reside in the same VkImage.  To satisfy both the hardware and Vulkan, we
    * allocate the depth and stencil buffers as separate surfaces in the same
    * bo.
    */
   struct anv_image_plane {
      struct anv_surface primary_surface;

      /**
       * The base aux usage for this image.  For color images, this can be
       * either CCS_E or CCS_D depending on whether or not we can reliably
       * leave CCS on all the time.
       */
      enum isl_aux_usage aux_usage;

      struct anv_surface aux_surface;

      /** Location of the compression control surface.  */
      struct anv_image_memory_range compr_ctrl_memory_range;

      /** Location of the fast clear state.  */
      struct anv_image_memory_range fast_clear_memory_range;

      /**
       * Whether this image can be fast cleared with non-zero clear colors.
       * This can happen with mutable images when formats of different bit
       * sizes per components are used.
       *
       * On Gfx9+, because the clear colors are stored as a 4 components 32bit
       * values, we can clear in R16G16_UNORM (store 2 16bit values in the
       * components 0 & 1 of the clear color) and then draw in R32_UINT which
       * would interpret the clear color as a single component value, using
       * only the first 16bit component of the previous written clear color.
       *
       * On Gfx7/7.5/8, only CC_ZERO/CC_ONE clear colors are supported, this
       * boolean will prevent the usage of CC_ONE.
       */
      bool can_non_zero_fast_clear;

      struct {
         /** Whether the image has CCS data mapped through AUX-TT. */
         bool mapped;

         /** Main address of the mapping. */
         uint64_t addr;

         /** Size of the mapping. */
         uint64_t size;
      } aux_tt;
   } planes[3];

   struct anv_image_memory_range vid_dmv_top_surface;

   /* Link in the anv_device.image_private_objects list */
   struct list_head link;
};

static inline bool
anv_image_is_sparse(struct anv_image *image)
{
   return image->vk.create_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
}

static inline bool
anv_image_is_externally_shared(const struct anv_image *image)
{
   return image->vk.drm_format_mod != DRM_FORMAT_MOD_INVALID ||
          image->vk.external_handle_types != 0;
}

static inline bool
anv_image_has_private_binding(const struct anv_image *image)
{
   const struct anv_image_binding private_binding =
      image->bindings[ANV_IMAGE_MEMORY_BINDING_PRIVATE];
   return private_binding.memory_range.size != 0;
}

static inline bool
anv_image_format_is_d16_or_s8(const struct anv_image *image)
{
   return image->vk.format == VK_FORMAT_D16_UNORM ||
      image->vk.format == VK_FORMAT_D16_UNORM_S8_UINT ||
      image->vk.format == VK_FORMAT_D24_UNORM_S8_UINT ||
      image->vk.format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
      image->vk.format == VK_FORMAT_S8_UINT;
}

/* The ordering of this enum is important */
enum anv_fast_clear_type {
   /** Image does not have/support any fast-clear blocks */
   ANV_FAST_CLEAR_NONE = 0,
   /** Image has/supports fast-clear but only to the default value */
   ANV_FAST_CLEAR_DEFAULT_VALUE = 1,
   /** Image has/supports fast-clear with an arbitrary fast-clear value */
   ANV_FAST_CLEAR_ANY = 2,
};

/**
 * Return the aspect's _format_ plane, not its _memory_ plane (using the
 * vocabulary of VK_EXT_image_drm_format_modifier). As a consequence, \a
 * aspect_mask may contain VK_IMAGE_ASPECT_PLANE_*, but must not contain
 * VK_IMAGE_ASPECT_MEMORY_PLANE_* .
 */
static inline uint32_t
anv_image_aspect_to_plane(const struct anv_image *image,
                          VkImageAspectFlagBits aspect)
{
   return anv_aspect_to_plane(image->vk.aspects, aspect);
}

/* Returns the number of auxiliary buffer levels attached to an image. */
static inline uint8_t
anv_image_aux_levels(const struct anv_image * const image,
                     VkImageAspectFlagBits aspect)
{
   uint32_t plane = anv_image_aspect_to_plane(image, aspect);
   if (image->planes[plane].aux_usage == ISL_AUX_USAGE_NONE)
      return 0;

   return image->vk.mip_levels;
}

/* Returns the number of auxiliary buffer layers attached to an image. */
static inline uint32_t
anv_image_aux_layers(const struct anv_image * const image,
                     VkImageAspectFlagBits aspect,
                     const uint8_t miplevel)
{
   assert(image);

   /* The miplevel must exist in the main buffer. */
   assert(miplevel < image->vk.mip_levels);

   if (miplevel >= anv_image_aux_levels(image, aspect)) {
      /* There are no layers with auxiliary data because the miplevel has no
       * auxiliary data.
       */
      return 0;
   }

   return MAX2(image->vk.array_layers, image->vk.extent.depth >> miplevel);
}

static inline struct anv_address MUST_CHECK
anv_image_address(const struct anv_image *image,
                  const struct anv_image_memory_range *mem_range)
{
   const struct anv_image_binding *binding = &image->bindings[mem_range->binding];
   assert(binding->memory_range.offset == 0);

   if (mem_range->size == 0)
      return ANV_NULL_ADDRESS;

   return anv_address_add(binding->address, mem_range->offset);
}

static inline struct anv_address
anv_image_get_clear_color_addr(UNUSED const struct anv_device *device,
                               const struct anv_image *image,
                               VkImageAspectFlagBits aspect)
{
   assert(image->vk.aspects & (VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV |
                               VK_IMAGE_ASPECT_DEPTH_BIT));

   uint32_t plane = anv_image_aspect_to_plane(image, aspect);
   const struct anv_image_memory_range *mem_range =
      &image->planes[plane].fast_clear_memory_range;

   return anv_image_address(image, mem_range);
}

static inline struct anv_address
anv_image_get_fast_clear_type_addr(const struct anv_device *device,
                                   const struct anv_image *image,
                                   VkImageAspectFlagBits aspect)
{
   struct anv_address addr =
      anv_image_get_clear_color_addr(device, image, aspect);

   unsigned clear_color_state_size;
   if (device->info->ver >= 11) {
      /* The fast clear type and the first compression state are stored in the
       * last 2 dwords of the clear color struct. Refer to the comment in
       * add_aux_state_tracking_buffer().
       */
      assert(device->isl_dev.ss.clear_color_state_size >= 32);
      clear_color_state_size = device->isl_dev.ss.clear_color_state_size - 8;
   } else
      clear_color_state_size = device->isl_dev.ss.clear_value_size;
   return anv_address_add(addr, clear_color_state_size);
}

static inline struct anv_address
anv_image_get_compression_state_addr(const struct anv_device *device,
                                     const struct anv_image *image,
                                     VkImageAspectFlagBits aspect,
                                     uint32_t level, uint32_t array_layer)
{
   assert(level < anv_image_aux_levels(image, aspect));
   assert(array_layer < anv_image_aux_layers(image, aspect, level));
   UNUSED uint32_t plane = anv_image_aspect_to_plane(image, aspect);
   assert(isl_aux_usage_has_ccs_e(image->planes[plane].aux_usage));

   /* Relative to start of the plane's fast clear type */
   uint32_t offset;

   offset = 4; /* Go past the fast clear type */

   if (image->vk.image_type == VK_IMAGE_TYPE_3D) {
      for (uint32_t l = 0; l < level; l++)
         offset += u_minify(image->vk.extent.depth, l) * 4;
   } else {
      offset += level * image->vk.array_layers * 4;
   }

   offset += array_layer * 4;

   assert(offset < image->planes[plane].fast_clear_memory_range.size);

   return anv_address_add(
      anv_image_get_fast_clear_type_addr(device, image, aspect),
      offset);
}

static inline const struct anv_image_memory_range *
anv_image_get_aux_memory_range(const struct anv_image *image,
                               uint32_t plane)
{
   if (image->planes[plane].aux_surface.memory_range.size > 0)
     return &image->planes[plane].aux_surface.memory_range;
   else
     return &image->planes[plane].compr_ctrl_memory_range;
}

/* Returns true if a HiZ-enabled depth buffer can be sampled from. */
static inline bool
anv_can_sample_with_hiz(const struct intel_device_info * const devinfo,
                        const struct anv_image *image)
{
   if (!(image->vk.aspects & VK_IMAGE_ASPECT_DEPTH_BIT))
      return false;

   /* For Gfx8-11, there are some restrictions around sampling from HiZ.
    * The Skylake PRM docs for RENDER_SURFACE_STATE::AuxiliarySurfaceMode
    * say:
    *
    *    "If this field is set to AUX_HIZ, Number of Multisamples must
    *    be MULTISAMPLECOUNT_1, and Surface Type cannot be SURFTYPE_3D."
    */
   if (image->vk.image_type == VK_IMAGE_TYPE_3D)
      return false;

   if (!devinfo->has_sample_with_hiz)
      return false;

   return image->vk.samples == 1;
}

/* Returns true if an MCS-enabled buffer can be sampled from. */
static inline bool
anv_can_sample_mcs_with_clear(const struct intel_device_info * const devinfo,
                              const struct anv_image *image)
{
   assert(image->vk.aspects == VK_IMAGE_ASPECT_COLOR_BIT);
   const uint32_t plane =
      anv_image_aspect_to_plane(image, VK_IMAGE_ASPECT_COLOR_BIT);

   assert(isl_aux_usage_has_mcs(image->planes[plane].aux_usage));

   const struct anv_surface *anv_surf = &image->planes[plane].primary_surface;

   /* On TGL, the sampler has an issue with some 8 and 16bpp MSAA fast clears.
    * See HSD 1707282275, wa_14013111325. Due to the use of
    * format-reinterpretation, a simplified workaround is implemented.
    */
   if (intel_needs_workaround(devinfo, 14013111325) &&
       isl_format_get_layout(anv_surf->isl.format)->bpb <= 16) {
      return false;
   }

   return true;
}

static inline bool
anv_image_plane_uses_aux_map(const struct anv_device *device,
                             const struct anv_image *image,
                             uint32_t plane)
{
   return device->info->has_aux_map &&
      isl_aux_usage_has_ccs(image->planes[plane].aux_usage);
}

static inline bool
anv_image_uses_aux_map(const struct anv_device *device,
                       const struct anv_image *image)
{
   for (uint32_t p = 0; p < image->n_planes; ++p) {
      if (anv_image_plane_uses_aux_map(device, image, p))
         return true;
   }

   return false;
}

static inline bool
anv_bo_allows_aux_map(const struct anv_device *device,
                      const struct anv_bo *bo)
{
   if (device->aux_map_ctx == NULL)
      return false;

   return (bo->alloc_flags & ANV_BO_ALLOC_AUX_TT_ALIGNED) != 0;
}

static inline bool
anv_address_allows_aux_map(const struct anv_device *device,
                           struct anv_address addr)
{
   if (device->aux_map_ctx == NULL)
      return false;

   /* Technically, we really only care about what offset the image is bound
    * into on the BO, but we don't have that information here. As a heuristic,
    * rely on the BO offset instead.
    */
   if (((addr.bo ? addr.bo->offset : 0) + addr.offset) %
       intel_aux_map_get_alignment(device->aux_map_ctx) != 0)
      return false;

   return true;
}

void
anv_cmd_buffer_mark_image_written(struct anv_cmd_buffer *cmd_buffer,
                                  const struct anv_image *image,
                                  VkImageAspectFlagBits aspect,
                                  enum isl_aux_usage aux_usage,
                                  uint32_t level,
                                  uint32_t base_layer,
                                  uint32_t layer_count);

void
anv_cmd_buffer_mark_image_fast_cleared(struct anv_cmd_buffer *cmd_buffer,
                                       const struct anv_image *image,
                                       const enum isl_format format,
                                       union isl_color_value clear_color);

void
anv_cmd_buffer_load_clear_color_from_image(struct anv_cmd_buffer *cmd_buffer,
                                           struct anv_state state,
                                           const struct anv_image *image);

struct anv_image_binding *
anv_image_aspect_to_binding(struct anv_image *image,
                            VkImageAspectFlags aspect);

void
anv_image_clear_color(struct anv_cmd_buffer *cmd_buffer,
                      const struct anv_image *image,
                      VkImageAspectFlagBits aspect,
                      enum isl_aux_usage aux_usage,
                      enum isl_format format, struct isl_swizzle swizzle,
                      uint32_t level, uint32_t base_layer, uint32_t layer_count,
                      VkRect2D area, union isl_color_value clear_color);
void
anv_image_clear_depth_stencil(struct anv_cmd_buffer *cmd_buffer,
                              const struct anv_image *image,
                              VkImageAspectFlags aspects,
                              enum isl_aux_usage depth_aux_usage,
                              uint32_t level,
                              uint32_t base_layer, uint32_t layer_count,
                              VkRect2D area,
                              float depth_value, uint8_t stencil_value);
void
anv_attachment_msaa_resolve(struct anv_cmd_buffer *cmd_buffer,
                            const struct anv_attachment *att,
                            VkImageLayout layout,
                            VkImageAspectFlagBits aspect);
void
anv_image_hiz_op(struct anv_cmd_buffer *cmd_buffer,
                 const struct anv_image *image,
                 VkImageAspectFlagBits aspect, uint32_t level,
                 uint32_t base_layer, uint32_t layer_count,
                 enum isl_aux_op hiz_op);
void
anv_image_hiz_clear(struct anv_cmd_buffer *cmd_buffer,
                    const struct anv_image *image,
                    VkImageAspectFlags aspects,
                    uint32_t level,
                    uint32_t base_layer, uint32_t layer_count,
                    VkRect2D area, uint8_t stencil_value);
void
anv_image_mcs_op(struct anv_cmd_buffer *cmd_buffer,
                 const struct anv_image *image,
                 enum isl_format format, struct isl_swizzle swizzle,
                 VkImageAspectFlagBits aspect,
                 uint32_t base_layer, uint32_t layer_count,
                 enum isl_aux_op mcs_op, union isl_color_value *clear_value,
                 bool predicate);
void
anv_image_ccs_op(struct anv_cmd_buffer *cmd_buffer,
                 const struct anv_image *image,
                 enum isl_format format, struct isl_swizzle swizzle,
                 VkImageAspectFlagBits aspect, uint32_t level,
                 uint32_t base_layer, uint32_t layer_count,
                 enum isl_aux_op ccs_op, union isl_color_value *clear_value,
                 bool predicate);

isl_surf_usage_flags_t
anv_image_choose_isl_surf_usage(VkImageCreateFlags vk_create_flags,
                                VkImageUsageFlags vk_usage,
                                isl_surf_usage_flags_t isl_extra_usage,
                                VkImageAspectFlagBits aspect);

void
anv_cmd_buffer_fill_area(struct anv_cmd_buffer *cmd_buffer,
                         struct anv_address address,
                         VkDeviceSize size,
                         uint32_t data);

VkResult
anv_cmd_buffer_ensure_rcs_companion(struct anv_cmd_buffer *cmd_buffer);

bool
anv_can_hiz_clear_ds_view(struct anv_device *device,
                          const struct anv_image_view *iview,
                          VkImageLayout layout,
                          VkImageAspectFlags clear_aspects,
                          float depth_clear_value,
                          VkRect2D render_area,
                          const VkQueueFlagBits queue_flags);

bool
anv_can_fast_clear_color_view(struct anv_device *device,
                              struct anv_image_view *iview,
                              VkImageLayout layout,
                              union isl_color_value clear_color,
                              uint32_t num_layers,
                              VkRect2D render_area,
                              const VkQueueFlagBits queue_flags);

enum isl_aux_state ATTRIBUTE_PURE
anv_layout_to_aux_state(const struct intel_device_info * const devinfo,
                        const struct anv_image *image,
                        const VkImageAspectFlagBits aspect,
                        const VkImageLayout layout,
                        const VkQueueFlagBits queue_flags);

enum isl_aux_usage ATTRIBUTE_PURE
anv_layout_to_aux_usage(const struct intel_device_info * const devinfo,
                        const struct anv_image *image,
                        const VkImageAspectFlagBits aspect,
                        const VkImageUsageFlagBits usage,
                        const VkImageLayout layout,
                        const VkQueueFlagBits queue_flags);

enum anv_fast_clear_type ATTRIBUTE_PURE
anv_layout_to_fast_clear_type(const struct intel_device_info * const devinfo,
                              const struct anv_image * const image,
                              const VkImageAspectFlagBits aspect,
                              const VkImageLayout layout,
                              const VkQueueFlagBits queue_flags);

bool ATTRIBUTE_PURE
anv_layout_has_untracked_aux_writes(const struct intel_device_info * const devinfo,
                                    const struct anv_image * const image,
                                    const VkImageAspectFlagBits aspect,
                                    const VkImageLayout layout,
                                    const VkQueueFlagBits queue_flags);

static inline bool
anv_image_aspects_compatible(VkImageAspectFlags aspects1,
                             VkImageAspectFlags aspects2)
{
   if (aspects1 == aspects2)
      return true;

   /* Only 1 color aspects are compatibles. */
   if ((aspects1 & VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV) != 0 &&
       (aspects2 & VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV) != 0 &&
       util_bitcount(aspects1) == util_bitcount(aspects2))
      return true;

   return false;
}

struct anv_image_view {
   struct vk_image_view vk;

   const struct anv_image *image; /**< VkImageViewCreateInfo::image */

   unsigned n_planes;

   /**
    * True if the surface states (if any) are owned by some anv_state_stream
    * from internal_surface_state_pool.
    */
   bool use_surface_state_stream;

   struct {
      struct isl_view isl;

      /**
       * A version of the image view for storage usage (can apply 3D image
       * slicing).
       */
      struct isl_view isl_storage;

      /**
       * RENDER_SURFACE_STATE when using image as a sampler surface with an
       * image layout of SHADER_READ_ONLY_OPTIMAL or
       * DEPTH_STENCIL_READ_ONLY_OPTIMAL.
       */
      struct anv_surface_state optimal_sampler;

      /**
       * RENDER_SURFACE_STATE when using image as a sampler surface with an
       * image layout of GENERAL.
       */
      struct anv_surface_state general_sampler;

      /**
       * RENDER_SURFACE_STATE when using image as a storage image.
       */
      struct anv_surface_state storage;
   } planes[3];
};

enum anv_image_view_state_flags {
   ANV_IMAGE_VIEW_STATE_TEXTURE_OPTIMAL      = (1 << 0),
};

void anv_image_fill_surface_state(struct anv_device *device,
                                  const struct anv_image *image,
                                  VkImageAspectFlagBits aspect,
                                  const struct isl_view *view,
                                  isl_surf_usage_flags_t view_usage,
                                  enum isl_aux_usage aux_usage,
                                  const union isl_color_value *clear_color,
                                  enum anv_image_view_state_flags flags,
                                  struct anv_surface_state *state_inout);

struct anv_image_create_info {
   const VkImageCreateInfo *vk_info;

   /** An opt-in bitmask which filters an ISL-mapping of the Vulkan tiling. */
   isl_tiling_flags_t isl_tiling_flags;

   /** These flags will be added to any derived from VkImageCreateInfo. */
   isl_surf_usage_flags_t isl_extra_usage_flags;

   /** An opt-in stride in pixels, should be 0 for implicit layouts */
   uint32_t stride;

   /** Whether to allocate private binding */
   bool no_private_binding_alloc;
};

VkResult anv_image_init(struct anv_device *device, struct anv_image *image,
                        const struct anv_image_create_info *create_info);

void anv_image_finish(struct anv_image *image);

void anv_image_get_memory_requirements(struct anv_device *device,
                                       struct anv_image *image,
                                       VkImageAspectFlags aspects,
                                       VkMemoryRequirements2 *pMemoryRequirements);

void anv_image_view_init(struct anv_device *device,
                         struct anv_image_view *iview,
                         const VkImageViewCreateInfo *pCreateInfo,
                         struct anv_state_stream *state_stream);

void anv_image_view_finish(struct anv_image_view *iview);

enum isl_format
anv_isl_format_for_descriptor_type(const struct anv_device *device,
                                   VkDescriptorType type);

static inline uint32_t
anv_rasterization_aa_mode(VkPolygonMode raster_mode,
                          VkLineRasterizationModeEXT line_mode)
{
   if (raster_mode == VK_POLYGON_MODE_LINE &&
       line_mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT)
      return true;
   return false;
}

static inline VkLineRasterizationModeEXT
anv_line_rasterization_mode(VkLineRasterizationModeEXT line_mode,
                            unsigned rasterization_samples)
{
   if (line_mode == VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT) {
      if (rasterization_samples > 1) {
         return VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT;
      } else {
         return VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
      }
   }
   return line_mode;
}

static inline bool
anv_is_dual_src_blend_factor(VkBlendFactor factor)
{
   return factor == VK_BLEND_FACTOR_SRC1_COLOR ||
          factor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR ||
          factor == VK_BLEND_FACTOR_SRC1_ALPHA ||
          factor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
}

static inline bool
anv_is_dual_src_blend_equation(const struct vk_color_blend_attachment_state *cb)
{
   return anv_is_dual_src_blend_factor(cb->src_color_blend_factor) &&
          anv_is_dual_src_blend_factor(cb->dst_color_blend_factor) &&
          anv_is_dual_src_blend_factor(cb->src_alpha_blend_factor) &&
          anv_is_dual_src_blend_factor(cb->dst_alpha_blend_factor);
}

VkFormatFeatureFlags2
anv_get_image_format_features2(const struct anv_physical_device *physical_device,
                               VkFormat vk_format,
                               const struct anv_format *anv_format,
                               VkImageTiling vk_tiling,
                               const struct isl_drm_modifier_info *isl_mod_info);

void anv_fill_buffer_surface_state(struct anv_device *device,
                                   void *surface_state_ptr,
                                   enum isl_format format,
                                   struct isl_swizzle swizzle,
                                   isl_surf_usage_flags_t usage,
                                   struct anv_address address,
                                   uint32_t range, uint32_t stride);


struct gfx8_border_color {
   union {
      float float32[4];
      uint32_t uint32[4];
   };
   /* Pad out to 64 bytes */
   uint32_t _pad[12];
};

struct anv_sampler {
   struct vk_sampler            vk;

   uint32_t                     state[3][4];
   uint32_t                     n_planes;

   /* Blob of sampler state data which is guaranteed to be 32-byte aligned
    * and with a 32-byte stride for use as bindless samplers.
    */
   struct anv_state             bindless_state;

   struct anv_state             custom_border_color;
};

#define ANV_PIPELINE_STATISTICS_MASK 0x000007ff

struct anv_query_pool {
   struct vk_query_pool                         vk;

   /** Stride between slots, in bytes */
   uint32_t                                     stride;
   /** Number of slots in this query pool */
   struct anv_bo *                              bo;

   /** Location for the KHR_performance_query small batch updating
    *  ANV_PERF_QUERY_OFFSET_REG
    */
   uint32_t                                     khr_perf_preambles_offset;

   /** Size of each small batch */
   uint32_t                                     khr_perf_preamble_stride;

   /* KHR perf queries : */
   uint32_t                                     pass_size;
   uint32_t                                     data_offset;
   uint32_t                                     snapshot_size;
   uint32_t                                     n_counters;
   struct intel_perf_counter_pass                *counter_pass;
   uint32_t                                     n_passes;
   struct intel_perf_query_info                 **pass_query;
};

static inline uint32_t khr_perf_query_preamble_offset(const struct anv_query_pool *pool,
                                                      uint32_t pass)
{
   return pool->khr_perf_preambles_offset +
          pool->khr_perf_preamble_stride * pass;
}

struct anv_vid_mem {
   struct anv_device_memory *mem;
   VkDeviceSize       offset;
   VkDeviceSize       size;
};

#define ANV_VIDEO_MEM_REQS_H264 4
#define ANV_VIDEO_MEM_REQS_H265 9
#define ANV_MB_WIDTH 16
#define ANV_MB_HEIGHT 16
#define ANV_VIDEO_H264_MAX_NUM_REF_FRAME 16
#define ANV_VIDEO_H265_MAX_NUM_REF_FRAME 16
#define ANV_VIDEO_H265_HCP_NUM_REF_FRAME 8
#define ANV_MAX_H265_CTB_SIZE 64

enum anv_vid_mem_h264_types {
   ANV_VID_MEM_H264_INTRA_ROW_STORE,
   ANV_VID_MEM_H264_DEBLOCK_FILTER_ROW_STORE,
   ANV_VID_MEM_H264_BSD_MPC_ROW_SCRATCH,
   ANV_VID_MEM_H264_MPR_ROW_SCRATCH,
   ANV_VID_MEM_H264_MAX,
};

enum anv_vid_mem_h265_types {
   ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_LINE,
   ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_TILE_LINE,
   ANV_VID_MEM_H265_DEBLOCK_FILTER_ROW_STORE_TILE_COLUMN,
   ANV_VID_MEM_H265_METADATA_LINE,
   ANV_VID_MEM_H265_METADATA_TILE_LINE,
   ANV_VID_MEM_H265_METADATA_TILE_COLUMN,
   ANV_VID_MEM_H265_SAO_LINE,
   ANV_VID_MEM_H265_SAO_TILE_LINE,
   ANV_VID_MEM_H265_SAO_TILE_COLUMN,
   ANV_VID_MEM_H265_MAX,
};

struct anv_video_session {
   struct vk_video_session vk;

   /* the decoder needs some private memory allocations */
   struct anv_vid_mem vid_mem[ANV_VID_MEM_H265_MAX];
};

struct anv_video_session_params {
   struct vk_video_session_parameters vk;
};

void
anv_dump_pipe_bits(enum anv_pipe_bits bits, FILE *f);

static inline void
anv_add_pending_pipe_bits(struct anv_cmd_buffer* cmd_buffer,
                          enum anv_pipe_bits bits,
                          const char* reason)
{
   cmd_buffer->state.pending_pipe_bits |= bits;
   if (INTEL_DEBUG(DEBUG_PIPE_CONTROL) && bits) {
      fputs("pc: add ", stdout);
      anv_dump_pipe_bits(bits, stdout);
      fprintf(stdout, "reason: %s\n", reason);
   }
}

struct anv_performance_configuration_intel {
   struct vk_object_base      base;

   struct intel_perf_registers *register_config;

   uint64_t                   config_id;
};

void anv_physical_device_init_va_ranges(struct anv_physical_device *device);
void anv_physical_device_init_perf(struct anv_physical_device *device, int fd);
void anv_device_perf_init(struct anv_device *device);
void anv_perf_write_pass_results(struct intel_perf_config *perf,
                                 struct anv_query_pool *pool, uint32_t pass,
                                 const struct intel_perf_query_result *accumulated_results,
                                 union VkPerformanceCounterResultKHR *results);

void anv_apply_per_prim_attr_wa(struct nir_shader *ms_nir,
                                struct nir_shader *fs_nir,
                                struct anv_device *device,
                                const VkGraphicsPipelineCreateInfo *info);

/* Use to emit a series of memcpy operations */
struct anv_memcpy_state {
   struct anv_device *device;
   struct anv_batch *batch;

   struct anv_vb_cache_range vb_bound;
   struct anv_vb_cache_range vb_dirty;
};

VkResult anv_device_init_internal_kernels(struct anv_device *device);
void anv_device_finish_internal_kernels(struct anv_device *device);

VkResult anv_device_init_astc_emu(struct anv_device *device);
void anv_device_finish_astc_emu(struct anv_device *device);
void anv_astc_emu_process(struct anv_cmd_buffer *cmd_buffer,
                          struct anv_image *image,
                          VkImageLayout layout,
                          const VkImageSubresourceLayers *subresource,
                          VkOffset3D block_offset,
                          VkExtent3D block_extent);

/* This structure is used in 2 scenarios :
 *
 *    - copy utrace timestamps from command buffer so that command buffer can
 *      be resubmitted multiple times without the recorded timestamps being
 *      overwritten before they're read back
 *
 *    - emit trace points for queue debug tagging
 *      (vkQueueBeginDebugUtilsLabelEXT/vkQueueEndDebugUtilsLabelEXT)
 */
struct anv_utrace_submit {
   /* Needs to be the first field */
   struct intel_ds_flush_data ds;

   /* Batch stuff to implement of copy of timestamps recorded in another
    * buffer.
    */
   struct anv_reloc_list relocs;
   struct anv_batch batch;
   struct util_dynarray batch_bos;

   /* Stream for temporary allocations */
   struct anv_state_stream dynamic_state_stream;
   struct anv_state_stream general_state_stream;

   /* Syncobj to be signaled when the batch completes */
   struct vk_sync *sync;

   /* Queue on which all the recorded traces are submitted */
   struct anv_queue *queue;

   /* Buffer of 64bits timestamps (only used for timestamp copies) */
   struct anv_bo *trace_bo;

   /* Last fully read 64bit timestamp (used to rebuild the upper bits of 32bit
    * timestamps)
    */
   uint64_t last_full_timestamp;

   /* Memcpy state tracking (only used for timestamp copies on render engine) */
   struct anv_memcpy_state memcpy_state;

   /* Memcpy state tracking (only used for timestamp copies on compute engine) */
   struct anv_simple_shader simple_state;
};

void anv_device_utrace_init(struct anv_device *device);
void anv_device_utrace_finish(struct anv_device *device);
VkResult
anv_device_utrace_flush_cmd_buffers(struct anv_queue *queue,
                                    uint32_t cmd_buffer_count,
                                    struct anv_cmd_buffer **cmd_buffers,
                                    struct anv_utrace_submit **out_submit);

#ifdef HAVE_PERFETTO
void anv_perfetto_init(void);
uint64_t anv_perfetto_begin_submit(struct anv_queue *queue);
void anv_perfetto_end_submit(struct anv_queue *queue, uint32_t submission_id,
                             uint64_t start_ts);
#else
static inline void anv_perfetto_init(void)
{
}
static inline uint64_t anv_perfetto_begin_submit(struct anv_queue *queue)
{
   return 0;
}
static inline void anv_perfetto_end_submit(struct anv_queue *queue,
                                           uint32_t submission_id,
                                           uint64_t start_ts)
{}
#endif

static bool
anv_has_cooperative_matrix(const struct anv_physical_device *device)
{
   return device->has_cooperative_matrix;
}

#define ANV_FROM_HANDLE(__anv_type, __name, __handle) \
   VK_FROM_HANDLE(__anv_type, __name, __handle)

VK_DEFINE_HANDLE_CASTS(anv_cmd_buffer, vk.base, VkCommandBuffer,
                       VK_OBJECT_TYPE_COMMAND_BUFFER)
VK_DEFINE_HANDLE_CASTS(anv_device, vk.base, VkDevice, VK_OBJECT_TYPE_DEVICE)
VK_DEFINE_HANDLE_CASTS(anv_instance, vk.base, VkInstance, VK_OBJECT_TYPE_INSTANCE)
VK_DEFINE_HANDLE_CASTS(anv_physical_device, vk.base, VkPhysicalDevice,
                       VK_OBJECT_TYPE_PHYSICAL_DEVICE)
VK_DEFINE_HANDLE_CASTS(anv_queue, vk.base, VkQueue, VK_OBJECT_TYPE_QUEUE)

VK_DEFINE_NONDISP_HANDLE_CASTS(anv_buffer, vk.base, VkBuffer,
                               VK_OBJECT_TYPE_BUFFER)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_buffer_view, vk.base, VkBufferView,
                               VK_OBJECT_TYPE_BUFFER_VIEW)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_descriptor_pool, base, VkDescriptorPool,
                               VK_OBJECT_TYPE_DESCRIPTOR_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_descriptor_set, base, VkDescriptorSet,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_descriptor_set_layout, base,
                               VkDescriptorSetLayout,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_device_memory, vk.base, VkDeviceMemory,
                               VK_OBJECT_TYPE_DEVICE_MEMORY)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_event, base, VkEvent, VK_OBJECT_TYPE_EVENT)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_image, vk.base, VkImage, VK_OBJECT_TYPE_IMAGE)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_image_view, vk.base, VkImageView,
                               VK_OBJECT_TYPE_IMAGE_VIEW);
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_pipeline, base, VkPipeline,
                               VK_OBJECT_TYPE_PIPELINE)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_pipeline_layout, base, VkPipelineLayout,
                               VK_OBJECT_TYPE_PIPELINE_LAYOUT)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_query_pool, vk.base, VkQueryPool,
                               VK_OBJECT_TYPE_QUERY_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_sampler, vk.base, VkSampler,
                               VK_OBJECT_TYPE_SAMPLER)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_performance_configuration_intel, base,
                               VkPerformanceConfigurationINTEL,
                               VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_video_session, vk.base,
                               VkVideoSessionKHR,
                               VK_OBJECT_TYPE_VIDEO_SESSION_KHR)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_video_session_params, vk.base,
                               VkVideoSessionParametersKHR,
                               VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR)

#define anv_genX(devinfo, thing) ({             \
   __typeof(&gfx9_##thing) genX_thing;          \
   switch ((devinfo)->verx10) {                 \
   case 90:                                     \
      genX_thing = &gfx9_##thing;               \
      break;                                    \
   case 110:                                    \
      genX_thing = &gfx11_##thing;              \
      break;                                    \
   case 120:                                    \
      genX_thing = &gfx12_##thing;              \
      break;                                    \
   case 125:                                    \
      genX_thing = &gfx125_##thing;             \
      break;                                    \
   case 200:                                    \
      genX_thing = &gfx20_##thing;              \
      break;                                    \
   default:                                     \
      unreachable("Unknown hardware generation"); \
   }                                            \
   genX_thing;                                  \
})

/* Gen-specific function declarations */
#ifdef genX
#  include "anv_genX.h"
#else
#  define genX(x) gfx9_##x
#  include "anv_genX.h"
#  undef genX
#  define genX(x) gfx11_##x
#  include "anv_genX.h"
#  undef genX
#  define genX(x) gfx12_##x
#  include "anv_genX.h"
#  undef genX
#  define genX(x) gfx125_##x
#  include "anv_genX.h"
#  undef genX
#  define genX(x) gfx20_##x
#  include "anv_genX.h"
#  undef genX
#endif

#ifdef __cplusplus
}
#endif

#endif /* ANV_PRIVATE_H */
