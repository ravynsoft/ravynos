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
#include "drm-uapi/i915_drm.h"
#include "drm-uapi/drm_fourcc.h"

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#define VG(x) x
#else
#define VG(x) ((void)0)
#endif

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
#include "util/u_vector.h"
#include "util/u_math.h"
#include "util/vma.h"
#include "util/xmlconfig.h"
#include "vk_alloc.h"
#include "vk_buffer.h"
#include "vk_command_buffer.h"
#include "vk_command_pool.h"
#include "vk_debug_report.h"
#include "vk_descriptor_update_template.h"
#include "vk_device.h"
#include "vk_drm_syncobj.h"
#include "vk_enum_defines.h"
#include "vk_format.h"
#include "vk_framebuffer.h"
#include "vk_graphics_state.h"
#include "vk_image.h"
#include "vk_instance.h"
#include "vk_pipeline_cache.h"
#include "vk_physical_device.h"
#include "vk_shader_module.h"
#include "vk_sync.h"
#include "vk_sync_timeline.h"
#include "vk_util.h"
#include "vk_queue.h"
#include "vk_log.h"
#include "vk_ycbcr_conversion.h"

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

struct intel_perf_config;
struct intel_perf_counter_pass;
struct intel_perf_query_result;

#include <vulkan/vulkan.h>
#include <vulkan/vk_icd.h>

#include "anv_android.h"
#include "anv_entrypoints.h"
#include "isl/isl.h"

#include "dev/intel_debug.h"
#undef MESA_LOG_TAG
#define MESA_LOG_TAG "MESA-INTEL"
#include "util/log.h"
#include "wsi_common.h"

#define NSEC_PER_SEC 1000000000ull

/* anv Virtual Memory Layout
 * =========================
 *
 * When the anv driver is determining the virtual graphics addresses of memory
 * objects itself using the softpin mechanism, the following memory ranges
 * will be used.
 *
 * Three special considerations to notice:
 *
 * (1) the dynamic state pool is located within the same 4 GiB as the low
 * heap. This is to work around a VF cache issue described in a comment in
 * anv_physical_device_init_heaps.
 *
 * (2) the binding table pool is located at lower addresses than the surface
 * state pool, within a 4 GiB range. This allows surface state base addresses
 * to cover both binding tables (16 bit offsets) and surface states (32 bit
 * offsets).
 *
 * (3) the last 4 GiB of the address space is withheld from the high
 * heap. Various hardware units will read past the end of an object for
 * various reasons. This healthy margin prevents reads from wrapping around
 * 48-bit addresses.
 */
#define GENERAL_STATE_POOL_MIN_ADDRESS     0x000000200000ULL /* 2 MiB */
#define GENERAL_STATE_POOL_MAX_ADDRESS     0x00003fffffffULL
#define LOW_HEAP_MIN_ADDRESS               0x000040000000ULL /* 1 GiB */
#define LOW_HEAP_MAX_ADDRESS               0x00007fffffffULL
#define DYNAMIC_STATE_POOL_MIN_ADDRESS     0x0000c0000000ULL /* 3 GiB */
#define DYNAMIC_STATE_POOL_MAX_ADDRESS     0x0000ffffffffULL
#define BINDING_TABLE_POOL_MIN_ADDRESS     0x000100000000ULL /* 4 GiB */
#define BINDING_TABLE_POOL_MAX_ADDRESS     0x00013fffffffULL
#define SURFACE_STATE_POOL_MIN_ADDRESS     0x000140000000ULL /* 5 GiB */
#define SURFACE_STATE_POOL_MAX_ADDRESS     0x00017fffffffULL
#define INSTRUCTION_STATE_POOL_MIN_ADDRESS 0x000180000000ULL /* 6 GiB */
#define INSTRUCTION_STATE_POOL_MAX_ADDRESS 0x0001bfffffffULL
#define CLIENT_VISIBLE_HEAP_MIN_ADDRESS    0x0001c0000000ULL /* 7 GiB */
#define CLIENT_VISIBLE_HEAP_MAX_ADDRESS    0x0002bfffffffULL
#define HIGH_HEAP_MIN_ADDRESS              0x0002c0000000ULL /* 11 GiB */

#define GENERAL_STATE_POOL_SIZE     \
   (GENERAL_STATE_POOL_MAX_ADDRESS - GENERAL_STATE_POOL_MIN_ADDRESS + 1)
#define LOW_HEAP_SIZE               \
   (LOW_HEAP_MAX_ADDRESS - LOW_HEAP_MIN_ADDRESS + 1)
#define DYNAMIC_STATE_POOL_SIZE     \
   (DYNAMIC_STATE_POOL_MAX_ADDRESS - DYNAMIC_STATE_POOL_MIN_ADDRESS + 1)
#define BINDING_TABLE_POOL_SIZE     \
   (BINDING_TABLE_POOL_MAX_ADDRESS - BINDING_TABLE_POOL_MIN_ADDRESS + 1)
#define BINDING_TABLE_POOL_BLOCK_SIZE (65536)
#define SURFACE_STATE_POOL_SIZE     \
   (SURFACE_STATE_POOL_MAX_ADDRESS - SURFACE_STATE_POOL_MIN_ADDRESS + 1)
#define INSTRUCTION_STATE_POOL_SIZE \
   (INSTRUCTION_STATE_POOL_MAX_ADDRESS - INSTRUCTION_STATE_POOL_MIN_ADDRESS + 1)
#define CLIENT_VISIBLE_HEAP_SIZE               \
   (CLIENT_VISIBLE_HEAP_MAX_ADDRESS - CLIENT_VISIBLE_HEAP_MIN_ADDRESS + 1)

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
#define MAX_SETS        32
#define MAX_RTS          8
#define MAX_VIEWPORTS   16
#define MAX_SCISSORS    16
#define MAX_PUSH_CONSTANTS_SIZE 128
#define MAX_DYNAMIC_BUFFERS 16
#define MAX_IMAGES 64
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

/* The kernel relocation API has a limitation of a 32-bit delta value
 * applied to the address before it is written which, in spite of it being
 * unsigned, is treated as signed .  Because of the way that this maps to
 * the Vulkan API, we cannot handle an offset into a buffer that does not
 * fit into a signed 32 bits.  The only mechanism we have for dealing with
 * this at the moment is to limit all VkDeviceMemory objects to a maximum
 * of 2GB each.  The Vulkan spec allows us to do this:
 *
 *    "Some platforms may have a limit on the maximum size of a single
 *    allocation. For example, certain systems may fail to create
 *    allocations with a size greater than or equal to 4GB. Such a limit is
 *    implementation-dependent, and if such a failure occurs then the error
 *    VK_ERROR_OUT_OF_DEVICE_MEMORY should be returned."
 */
#define MAX_MEMORY_ALLOCATION_SIZE (1ull << 31)

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

struct anv_bo {
   const char *name;

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
    * relocations.
    */
   uint64_t offset;

   /** Size of the buffer not including implicit aux */
   uint64_t size;

   /* Map for internally mapped BOs.
    *
    * If ANV_BO_ALLOC_MAPPED is set in flags, this is the map for the whole
    * BO. If ANV_BO_WRAPPER is set in flags, map points to the wrapped BO.
    */
   void *map;

   /** Flags to pass to the kernel through drm_i915_exec_object2::flags */
   uint32_t flags;

   /** True if this BO may be shared with other processes */
   bool is_external:1;

   /** True if this BO is a wrapper
    *
    * When set to true, none of the fields in this BO are meaningful except
    * for anv_bo::is_wrapper and anv_bo::map which points to the actual BO.
    * See also anv_bo_unwrap().  Wrapper BOs are not allowed when use_softpin
    * is set in the physical device.
    */
   bool is_wrapper:1;

   /** See also ANV_BO_ALLOC_FIXED_ADDRESS */
   bool has_fixed_address:1;

   /** True if this BO wraps a host pointer */
   bool from_host_ptr:1;

   /** See also ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS */
   bool has_client_visible_address:1;
};

static inline struct anv_bo *
anv_bo_ref(struct anv_bo *bo)
{
   p_atomic_inc(&bo->refcount);
   return bo;
}

static inline struct anv_bo *
anv_bo_unwrap(struct anv_bo *bo)
{
   while (bo->is_wrapper)
      bo = bo->map;
   return bo;
}

static inline bool
anv_bo_is_pinned(struct anv_bo *bo)
{
#if defined(GFX_VERx10) && GFX_VERx10 >= 90
   /* Sky Lake and later always uses softpin */
   assert(bo->flags & EXEC_OBJECT_PINNED);
   return true;
#elif defined(GFX_VERx10) && GFX_VERx10 < 80
   /* Haswell and earlier never use softpin */
   assert(!(bo->flags & EXEC_OBJECT_PINNED));
   assert(!bo->has_fixed_address);
   return false;
#else
   /* If we don't have a GFX_VERx10 #define, we need to look at the BO.  Also,
    * for GFX version 8, we need to look at the BO because Broadwell softpins
    * but Cherryview doesn't.
    */
   assert((bo->flags & EXEC_OBJECT_PINNED) || !bo->has_fixed_address);
   return (bo->flags & EXEC_OBJECT_PINNED) != 0;
#endif
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
   if (addr.bo && anv_bo_is_pinned(addr.bo)) {
      return intel_canonical_address(addr.bo->offset + addr.offset);
   } else {
      return intel_canonical_address(addr.offset);
   }
}

static inline struct anv_address
anv_address_add(struct anv_address addr, uint64_t offset)
{
   addr.offset += offset;
   return addr;
}

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
   bool use_relocations;

   /* Wrapper BO for use in relocation lists.  This BO is simply a wrapper
    * around the actual BO so that we grow the pool after the wrapper BO has
    * been put in a relocation list.  This is only used in the non-softpin
    * case.
    */
   struct anv_bo wrapper_bo;

   struct anv_bo *bos[ANV_MAX_BLOCK_POOL_BOS];
   struct anv_bo *bo;
   uint32_t nbos;

   uint64_t size;

   /* The address where the start of the pool is pinned. The various bos that
    * are created as the pool grows will have addresses in the range
    * [start_address, start_address + BLOCK_POOL_MEMFD_SIZE).
    */
   uint64_t start_address;

   /* The offset from the start of the bo to the "center" of the block
    * pool.  Pointers to allocated blocks are given by
    * bo.map + center_bo_offset + offsets.
    */
   uint32_t center_bo_offset;

   /* Current memory map of the block pool.  This pointer may or may not
    * point to the actual beginning of the block pool memory.  If
    * anv_block_pool_alloc_back has ever been called, then this pointer
    * will point to the "center" position of the buffer and all offsets
    * (negative or positive) given out by the block pool alloc functions
    * will be valid relative to this pointer.
    *
    * In particular, map == bo.map + center_offset
    *
    * DO NOT access this pointer directly. Use anv_block_pool_map() instead,
    * since it will handle the softpin case as well, where this points to NULL.
    */
   void *map;
   int fd;

   /**
    * Array of mmaps and gem handles owned by the block pool, reclaimed when
    * the block pool is destroyed.
    */
   struct u_vector mmap_cleanups;

   struct anv_block_state state;

   struct anv_block_state back_state;
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
   return pool->state.end + pool->back_state.end;
}

struct anv_state {
   int32_t offset;
   uint32_t alloc_size;
   void *map;
   uint32_t idx;
};

#define ANV_STATE_NULL ((struct anv_state) { .alloc_size = 0 })

struct anv_fixed_size_state_pool {
   union anv_free_list free_list;
   struct anv_block_state block;
};

#define ANV_MIN_STATE_SIZE_LOG2 6
#define ANV_MAX_STATE_SIZE_LOG2 21

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
   struct anv_block_state state;
   struct u_vector cleanups;
};

struct anv_state_pool {
   struct anv_block_pool block_pool;

   /* Offset into the relevant state base address where the state pool starts
    * allocating memory.
    */
   int32_t start_offset;

   struct anv_state_table table;

   /* The size of blocks which will be allocated from the block pool */
   uint32_t block_size;

   /** Free list for "back" allocations */
   union anv_free_list back_alloc_free_list;

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

/* The block_pool functions exported for testing only.  The block pool should
 * only be used via a state pool (see below).
 */
VkResult anv_block_pool_init(struct anv_block_pool *pool,
                             struct anv_device *device,
                             const char *name,
                             uint64_t start_address,
                             uint32_t initial_size);
void anv_block_pool_finish(struct anv_block_pool *pool);
int32_t anv_block_pool_alloc(struct anv_block_pool *pool,
                             uint32_t block_size, uint32_t *padding);
int32_t anv_block_pool_alloc_back(struct anv_block_pool *pool,
                                  uint32_t block_size);
void* anv_block_pool_map(struct anv_block_pool *pool, int32_t offset, uint32_t
size);

VkResult anv_state_pool_init(struct anv_state_pool *pool,
                             struct anv_device *device,
                             const char *name,
                             uint64_t base_address,
                             int32_t start_offset,
                             uint32_t block_size);
void anv_state_pool_finish(struct anv_state_pool *pool);
struct anv_state anv_state_pool_alloc(struct anv_state_pool *pool,
                                      uint32_t state_size, uint32_t alignment);
struct anv_state anv_state_pool_alloc_back(struct anv_state_pool *pool);
void anv_state_pool_free(struct anv_state_pool *pool, struct anv_state state);
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

   struct util_sparse_array_free_list free_list[16];
};

void anv_bo_pool_init(struct anv_bo_pool *pool, struct anv_device *device,
                      const char *name);
void anv_bo_pool_finish(struct anv_bo_pool *pool);
VkResult anv_bo_pool_alloc(struct anv_bo_pool *pool, uint32_t size,
                           struct anv_bo **bo_out);
void anv_bo_pool_free(struct anv_bo_pool *pool, struct anv_bo *bo);

struct anv_scratch_pool {
   /* Indexed by Per-Thread Scratch Space number (the hardware value) and stage */
   struct anv_bo *bos[16][MESA_SHADER_STAGES];
};

void anv_scratch_pool_init(struct anv_device *device,
                           struct anv_scratch_pool *pool);
void anv_scratch_pool_finish(struct anv_device *device,
                             struct anv_scratch_pool *pool);
struct anv_bo *anv_scratch_pool_alloc(struct anv_device *device,
                                      struct anv_scratch_pool *pool,
                                      gl_shader_stage stage,
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

   /* Driver internal information */
   enum intel_engine_class engine_class;
};

#define ANV_MAX_QUEUE_FAMILIES 3

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
};

struct anv_memregion {
   uint64_t size;
   uint64_t available;
};

enum anv_timestamp_capture_type {
    ANV_TIMESTAMP_CAPTURE_TOP_OF_PIPE,
    ANV_TIMESTAMP_CAPTURE_END_OF_PIPE,
    ANV_TIMESTAMP_CAPTURE_AT_CS_STALL,
};

struct anv_physical_device {
    struct vk_physical_device                   vk;

    /* Link in anv_instance::physical_devices */
    struct list_head                            link;

    struct anv_instance *                       instance;
    char                                        path[20];
    struct intel_device_info                      info;
    bool                                        supports_48bit_addresses;
    struct brw_compiler *                       compiler;
    struct isl_device                           isl_dev;
    struct intel_perf_config *                    perf;
   /* True if hardware support is incomplete/alpha */
    bool                                        is_alpha;
    /*
     * Number of commands required to implement a performance query begin +
     * end.
     */
    uint32_t                                    n_perf_query_commands;
    int                                         cmd_parser_version;
    bool                                        has_exec_async;
    bool                                        has_exec_capture;
    int                                         max_context_priority;
    uint64_t                                    gtt_size;

    bool                                        use_relocations;
    bool                                        use_softpin;
    bool                                        always_use_bindless;
    bool                                        use_call_secondary;

    /** True if we can access buffers using A64 messages */
    bool                                        has_a64_buffer_access;
    /** True if we can use bindless access for samplers */
    bool                                        has_bindless_samplers;
    /** True if we can use timeline semaphores through execbuf */
    bool                                        has_exec_timeline;

    /** True if we can read the GPU timestamp register
     *
     * When running in a virtual context, the timestamp register is unreadable
     * on Gfx12+.
     */
    bool                                        has_reg_timestamp;

    bool                                        always_flush_cache;

    struct {
      uint32_t                                  family_count;
      struct anv_queue_family                   families[ANV_MAX_QUEUE_FAMILIES];
    } queue;

    struct {
      uint32_t                                  type_count;
      struct anv_memory_type                    types[VK_MAX_MEMORY_TYPES];
      uint32_t                                  heap_count;
      struct anv_memory_heap                    heaps[VK_MAX_MEMORY_HEAPS];
      bool                                      need_flush;
    } memory;

    struct anv_memregion                        sys;
    uint8_t                                     driver_build_sha1[20];
    uint8_t                                     pipeline_cache_uuid[VK_UUID_SIZE];
    uint8_t                                     driver_uuid[VK_UUID_SIZE];
    uint8_t                                     device_uuid[VK_UUID_SIZE];

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

    void (*cmd_emit_timestamp)(struct anv_batch *, struct anv_device *, struct anv_address, enum anv_timestamp_capture_type);
    struct intel_measure_device                 measure_device;
};

struct anv_instance {
    struct vk_instance                          vk;

    struct driOptionCache                       dri_options;
    struct driOptionCache                       available_dri_options;

    /**
     * Workarounds for game bugs.
     */
    uint8_t                                     assume_full_subgroups;
    bool                                        limit_trig_input_range;
    bool                                        sample_mask_out_opengl_behaviour;
    float                                       lower_depth_range_rate;
    bool                                        report_vk_1_3;

    /* HW workarounds */
    bool                                        no_16bit;
};

VkResult anv_init_wsi(struct anv_physical_device *physical_device);
void anv_finish_wsi(struct anv_physical_device *physical_device);

struct anv_queue {
   struct vk_queue                           vk;

   struct anv_device *                       device;

   const struct anv_queue_family *           family;

   uint32_t                                  exec_flags;

   /** Synchronization object for debug purposes (DEBUG_SYNC) */
   struct vk_sync                           *sync;

   struct intel_ds_queue                     ds;
};

struct nir_xfb_info;
struct anv_pipeline_bind_map;

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
                         const struct anv_pipeline_bind_map *bind_map);

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

struct anv_device {
    struct vk_device                            vk;

    struct anv_physical_device *                physical;
    const struct intel_device_info *            info;
    struct isl_device                           isl_dev;
    uint32_t                                    context_id;
    int                                         fd;
    bool                                        can_chain_batches;

    pthread_mutex_t                             vma_mutex;
    struct util_vma_heap                        vma_lo;
    struct util_vma_heap                        vma_cva;
    struct util_vma_heap                        vma_hi;

    /** List of all anv_device_memory objects */
    struct list_head                            memory_objects;

    struct anv_bo_pool                          batch_bo_pool;
    struct anv_bo_pool                          utrace_bo_pool;

    struct anv_bo_cache                         bo_cache;

    struct anv_state_pool                       general_state_pool;
    struct anv_state_pool                       dynamic_state_pool;
    struct anv_state_pool                       instruction_state_pool;
    struct anv_state_pool                       binding_table_pool;
    struct anv_state_pool                       surface_state_pool;

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

    uint32_t                                    queue_count;
    struct anv_queue  *                         queues;

    struct anv_scratch_pool                     scratch_pool;

    bool                                        robust_buffer_access;

    pthread_mutex_t                             mutex;
    pthread_cond_t                              queue_submit;

    struct intel_batch_decode_ctx               decoder_ctx;
    /*
     * When decoding a anv_cmd_buffer, we might need to search for BOs through
     * the cmd_buffer's list.
     */
    struct anv_cmd_buffer                      *cmd_buffer_being_decoded;

    int                                         perf_fd; /* -1 if no opened */
    uint64_t                                    perf_metric; /* 0 if unset */

    const struct intel_l3_config                *l3_config;

    struct intel_debug_block_frame              *debug_frame_desc;

    struct intel_ds_device                       ds;
};

#if defined(GFX_VERx10) && GFX_VERx10 >= 90
#define ANV_ALWAYS_SOFTPIN true
#else
#define ANV_ALWAYS_SOFTPIN false
#endif

static inline bool
anv_use_relocations(const struct anv_physical_device *pdevice)
{
#if defined(GFX_VERx10) && GFX_VERx10 >= 90
   /* Sky Lake and later always uses softpin */
   assert(!pdevice->use_relocations);
   return false;
#elif defined(GFX_VERx10) && GFX_VERx10 < 80
   /* Haswell and earlier never use softpin */
   assert(pdevice->use_relocations);
   return true;
#else
   /* If we don't have a GFX_VERx10 #define, we need to look at the physical
    * device.  Also, for GFX version 8, we need to look at the physical
    * device because Broadwell softpins but Cherryview doesn't.
    */
   return pdevice->use_relocations;
#endif
}

static inline struct anv_state_pool *
anv_binding_table_pool(struct anv_device *device)
{
   if (anv_use_relocations(device->physical))
      return &device->surface_state_pool;
   else
      return &device->binding_table_pool;
}

static inline struct anv_state
anv_binding_table_pool_alloc(struct anv_device *device)
{
   if (anv_use_relocations(device->physical))
      return anv_state_pool_alloc_back(&device->surface_state_pool);
   else
      return anv_state_pool_alloc(&device->binding_table_pool,
                                  device->binding_table_pool.block_size, 0);
}

static inline void
anv_binding_table_pool_free(struct anv_device *device, struct anv_state state) {
   anv_state_pool_free(anv_binding_table_pool(device), state);
}

static inline uint32_t
anv_mocs(const struct anv_device *device,
         const struct anv_bo *bo,
         isl_surf_usage_flags_t usage)
{
   return isl_mocs(&device->isl_dev, usage, bo && bo->is_external);
}

void anv_device_init_blorp(struct anv_device *device);
void anv_device_finish_blorp(struct anv_device *device);

enum anv_bo_alloc_flags {
   /** Specifies that the BO must have a 32-bit address
    *
    * This is the opposite of EXEC_OBJECT_SUPPORTS_48B_ADDRESS.
    */
   ANV_BO_ALLOC_32BIT_ADDRESS =  (1 << 0),

   /** Specifies that the BO may be shared externally */
   ANV_BO_ALLOC_EXTERNAL =       (1 << 1),

   /** Specifies that the BO should be mapped */
   ANV_BO_ALLOC_MAPPED =         (1 << 2),

   /** Specifies that the BO should be snooped so we get coherency */
   ANV_BO_ALLOC_SNOOPED =        (1 << 3),

   /** Specifies that the BO should be captured in error states */
   ANV_BO_ALLOC_CAPTURE =        (1 << 4),

   /** Specifies that the BO will have an address assigned by the caller
    *
    * Such BOs do not exist in any VMA heap.
    */
   ANV_BO_ALLOC_FIXED_ADDRESS = (1 << 5),

   /** Enables implicit synchronization on the BO
    *
    * This is the opposite of EXEC_OBJECT_ASYNC.
    */
   ANV_BO_ALLOC_IMPLICIT_SYNC =  (1 << 6),

   /** Enables implicit synchronization on the BO
    *
    * This is equivalent to EXEC_OBJECT_WRITE.
    */
   ANV_BO_ALLOC_IMPLICIT_WRITE = (1 << 7),

   /** Has an address which is visible to the client */
   ANV_BO_ALLOC_CLIENT_VISIBLE_ADDRESS = (1 << 8),
};

VkResult anv_device_alloc_bo(struct anv_device *device,
                             const char *name, uint64_t size,
                             enum anv_bo_alloc_flags alloc_flags,
                             uint64_t explicit_address,
                             struct anv_bo **bo);
VkResult anv_device_map_bo(struct anv_device *device,
                           struct anv_bo *bo,
                           uint64_t offset,
                           size_t size,
                           uint32_t gem_flags,
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
                        uint32_t exec_flags,
                        const VkDeviceQueueCreateInfo *pCreateInfo,
                        uint32_t index_in_family);
void anv_queue_finish(struct anv_queue *queue);

VkResult anv_queue_submit(struct vk_queue *queue,
                          struct vk_queue_submit *submit);
VkResult anv_queue_submit_simple_batch(struct anv_queue *queue,
                                       struct anv_batch *batch);

void* anv_gem_mmap(struct anv_device *device,
                   uint32_t gem_handle, uint64_t offset, uint64_t size, uint32_t flags);
void anv_gem_munmap(struct anv_device *device, void *p, uint64_t size);
uint32_t anv_gem_create(struct anv_device *device, uint64_t size);
void anv_gem_close(struct anv_device *device, uint32_t gem_handle);
uint32_t anv_gem_userptr(struct anv_device *device, void *mem, size_t size);
int anv_gem_wait(struct anv_device *device, uint32_t gem_handle, int64_t *timeout_ns);
int anv_gem_execbuffer(struct anv_device *device,
                       struct drm_i915_gem_execbuffer2 *execbuf);
int anv_gem_set_tiling(struct anv_device *device, uint32_t gem_handle,
                       uint32_t stride, uint32_t tiling);
bool anv_gem_has_context_priority(int fd, int priority);
int anv_gem_set_context_param(int fd, uint32_t context, uint32_t param,
                              uint64_t value);
int anv_gem_get_tiling(struct anv_device *device, uint32_t gem_handle);
int anv_gem_context_get_reset_stats(int fd, int context,
                                    uint32_t *active, uint32_t *pending);
int anv_gem_handle_to_fd(struct anv_device *device, uint32_t gem_handle);
uint32_t anv_gem_fd_to_handle(struct anv_device *device, int fd);
int anv_gem_set_caching(struct anv_device *device, uint32_t gem_handle, uint32_t caching);

uint64_t anv_vma_alloc(struct anv_device *device,
                       uint64_t size, uint64_t align,
                       enum anv_bo_alloc_flags alloc_flags,
                       uint64_t client_address);
void anv_vma_free(struct anv_device *device,
                  uint64_t address, uint64_t size);

struct anv_reloc_list {
   uint32_t                                     num_relocs;
   uint32_t                                     array_length;
   struct drm_i915_gem_relocation_entry *       relocs;
   struct anv_bo **                             reloc_bos;
   uint32_t                                     dep_words;
   BITSET_WORD *                                deps;
};

VkResult anv_reloc_list_init(struct anv_reloc_list *list,
                             const VkAllocationCallbacks *alloc);
void anv_reloc_list_finish(struct anv_reloc_list *list,
                           const VkAllocationCallbacks *alloc);

VkResult anv_reloc_list_add(struct anv_reloc_list *list,
                            const VkAllocationCallbacks *alloc,
                            uint32_t offset, struct anv_bo *target_bo,
                            uint32_t delta, uint64_t *address_u64_out);

VkResult anv_reloc_list_add_bo(struct anv_reloc_list *list,
                               const VkAllocationCallbacks *alloc,
                               struct anv_bo *target_bo);

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

   struct anv_address                           start_addr;

   void *                                       start;
   void *                                       end;
   void *                                       next;

   struct anv_reloc_list *                      relocs;

   /* This callback is called (with the associated user data) in the event
    * that the batch runs out of space.
    */
   VkResult (*extend_cb)(struct anv_batch *, void *);
   void *                                       user_data;

   /**
    * Current error status of the command buffer. Used to track inconsistent
    * or incomplete command buffer states that are the consequence of run-time
    * errors such as out of memory scenarios. We want to track this in the
    * batch because the command buffer object is not visible to some parts
    * of the driver.
    */
   VkResult                                     status;
};

void *anv_batch_emit_dwords(struct anv_batch *batch, int num_dwords);
void anv_batch_emit_batch(struct anv_batch *batch, struct anv_batch *other);
struct anv_address anv_batch_address(struct anv_batch *batch, void *batch_location);

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
anv_batch_emit_reloc(struct anv_batch *batch,
                     void *location, struct anv_bo *bo, uint32_t delta)
{
   uint64_t address_u64 = 0;
   VkResult result;

   if (ANV_ALWAYS_SOFTPIN) {
      address_u64 = bo->offset + delta;
      result = anv_reloc_list_add_bo(batch->relocs, batch->alloc, bo);
   } else {
      result = anv_reloc_list_add(batch->relocs, batch->alloc,
                                  location - batch->start, bo, delta,
                                  &address_u64);
   }
   if (unlikely(result != VK_SUCCESS)) {
      anv_batch_set_error(batch, result);
      return 0;
   }

   return address_u64;
}

static inline void
write_reloc(const struct anv_device *device, void *p, uint64_t v, bool flush)
{
   UNUSED unsigned reloc_size = 0;
   if (device->info->ver >= 8) {
      reloc_size = sizeof(uint64_t);
      *(uint64_t *)p = intel_canonical_address(v);
   } else {
      reloc_size = sizeof(uint32_t);
      *(uint32_t *)p = v;
   }

#ifdef SUPPORT_INTEL_INTEGRATED_GPUS
   if (flush && device->physical->memory.need_flush)
      intel_flush_range(p, reloc_size);
#endif
}

static inline uint64_t
_anv_combine_address(struct anv_batch *batch, void *location,
                     const struct anv_address address, uint32_t delta)
{
   if (address.bo == NULL) {
      return address.offset + delta;
   } else if (batch == NULL) {
      assert(anv_bo_is_pinned(address.bo));
      return anv_address_physical(anv_address_add(address, delta));
   } else {
      assert(batch->start <= location && location < batch->end);
      /* i915 relocations are signed. */
      assert(INT32_MIN <= address.offset && address.offset <= INT32_MAX);
      return anv_batch_emit_reloc(batch, location, address.bo, address.offset + delta);
   }
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

#define anv_batch_emit_merge(batch, dwords0, dwords1)                   \
   do {                                                                 \
      uint32_t *dw;                                                     \
                                                                        \
      STATIC_ASSERT(ARRAY_SIZE(dwords0) == ARRAY_SIZE(dwords1));        \
      dw = anv_batch_emit_dwords((batch), ARRAY_SIZE(dwords0));         \
      if (!dw)                                                          \
         break;                                                         \
      for (uint32_t i = 0; i < ARRAY_SIZE(dwords0); i++)                \
         dw[i] = (dwords0)[i] | (dwords1)[i];                           \
      VG(VALGRIND_CHECK_MEM_IS_DEFINED(dw, ARRAY_SIZE(dwords0) * 4));\
   } while (0)

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
   struct vk_object_base                        base;

   struct list_head                             link;

   struct anv_bo *                              bo;
   const struct anv_memory_type *               type;

   void *                                       map;
   size_t                                       map_size;

   /* The map, from the user PoV is map + map_delta */
   uint64_t                                     map_delta;

   /* If set, we are holding reference to AHardwareBuffer
    * which we must release when memory is freed.
    */
   struct AHardwareBuffer *                     ahw;

   /* If set, this memory comes from a host pointer. */
   void *                                       host_ptr;
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

struct anv_texture_swizzle_descriptor {
   /** Texture swizzle
    *
    * See also nir_intrinsic_channel_select_intel
    */
   uint8_t swizzle[4];

   /** Unused padding to ensure the struct is a multiple of 64 bits */
   uint32_t _pad;
};

/** Struct representing a storage image descriptor */
struct anv_storage_image_descriptor {
   /** Bindless image handles
    *
    * These are expected to already be shifted such that the 20-bit
    * SURFACE_STATE table index is in the top 20 bits.
    */
   uint32_t vanilla;
   uint32_t lowered;
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
   ANV_DESCRIPTOR_SURFACE_STATE  = (1 << 0),
   /** The descriptor contains a BTI reference to a sampler state */
   ANV_DESCRIPTOR_SAMPLER_STATE  = (1 << 1),
   /** The descriptor contains an actual buffer view */
   ANV_DESCRIPTOR_BUFFER_VIEW    = (1 << 2),
   /** The descriptor contains auxiliary image layout data */
   ANV_DESCRIPTOR_IMAGE_PARAM    = (1 << 3),
   /** The descriptor contains auxiliary image layout data */
   ANV_DESCRIPTOR_INLINE_UNIFORM = (1 << 4),
   /** anv_address_range_descriptor with a buffer address and range */
   ANV_DESCRIPTOR_ADDRESS_RANGE  = (1 << 5),
   /** Bindless surface handle */
   ANV_DESCRIPTOR_SAMPLED_IMAGE  = (1 << 6),
   /** Storage image handles */
   ANV_DESCRIPTOR_STORAGE_IMAGE  = (1 << 7),
   /** Storage image handles */
   ANV_DESCRIPTOR_TEXTURE_SWIZZLE  = (1 << 8),
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

   /* Index into the dynamic state array for a dynamic buffer */
   int16_t dynamic_offset_index;

   /* Index into the descriptor set buffer views */
   int32_t buffer_view_index;

   /* Offset into the descriptor buffer where this descriptor lives */
   uint32_t descriptor_offset;

   /* Pre computed stride */
   unsigned descriptor_stride;

   /* Immutable samplers (or NULL if no immutable samplers) */
   struct anv_sampler **immutable_samplers;
};

bool anv_descriptor_supports_bindless(const struct anv_physical_device *pdevice,
                                      const struct anv_descriptor_set_binding_layout *binding,
                                      bool sampler);

bool anv_descriptor_requires_bindless(const struct anv_physical_device *pdevice,
                                      const struct anv_descriptor_set_binding_layout *binding,
                                      bool sampler);

struct anv_descriptor_set_layout {
   struct vk_object_base base;

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

   /* Size of the descriptor buffer for this descriptor set */
   uint32_t descriptor_buffer_size;

   /* Bindings in this descriptor set */
   struct anv_descriptor_set_binding_layout binding[0];
};

void anv_descriptor_set_layout_destroy(struct anv_device *device,
                                       struct anv_descriptor_set_layout *layout);

static inline void
anv_descriptor_set_layout_ref(struct anv_descriptor_set_layout *layout)
{
   assert(layout && layout->ref_cnt >= 1);
   p_atomic_inc(&layout->ref_cnt);
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
      };

      struct anv_buffer_view *buffer_view;
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

   /* State relative to anv_descriptor_pool::bo */
   struct anv_state desc_mem;
   /* Surface state for the descriptor buffer */
   struct anv_state desc_surface_state;

   /* Descriptor set address. */
   struct anv_address desc_addr;

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

struct anv_buffer_view {
   struct vk_object_base base;

   uint64_t range; /**< VkBufferViewCreateInfo::range */

   struct anv_address address;

   struct anv_state surface_state;
   struct anv_state storage_surface_state;
   struct anv_state lowered_storage_surface_state;

   struct brw_image_param lowered_storage_image_param;
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

   return set->desc_addr;
}

struct anv_descriptor_pool {
   struct vk_object_base base;

   uint32_t size;
   uint32_t next;
   uint32_t free_list;

   struct anv_bo *bo;
   struct util_vma_heap bo_heap;

   struct anv_state_stream surface_state_stream;
   void *surface_state_free_list;

   struct list_head desc_sets;

   bool host_only;

   char data[0];
};

size_t
anv_descriptor_set_layout_size(const struct anv_descriptor_set_layout *layout,
                               uint32_t var_desc_count);

uint32_t
anv_descriptor_set_layout_descriptor_buffer_size(const struct anv_descriptor_set_layout *set_layout,
                                                 uint32_t var_desc_count);

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
                                struct anv_state_stream *alloc_stream,
                                VkDescriptorType type,
                                struct anv_buffer *buffer,
                                uint32_t binding,
                                uint32_t element,
                                VkDeviceSize offset,
                                VkDeviceSize range);

void
anv_descriptor_set_write_inline_uniform_data(struct anv_device *device,
                                             struct anv_descriptor_set *set,
                                             uint32_t binding,
                                             const void *data,
                                             size_t offset,
                                             size_t size);

void
anv_descriptor_set_write_template(struct anv_device *device,
                                  struct anv_descriptor_set *set,
                                  struct anv_state_stream *alloc_stream,
                                  const struct vk_descriptor_update_template *template,
                                  const void *data);

#define ANV_DESCRIPTOR_SET_NULL             (UINT8_MAX - 5)
#define ANV_DESCRIPTOR_SET_PUSH_CONSTANTS   (UINT8_MAX - 4)
#define ANV_DESCRIPTOR_SET_DESCRIPTORS      (UINT8_MAX - 3)
#define ANV_DESCRIPTOR_SET_NUM_WORK_GROUPS  (UINT8_MAX - 2)
#define ANV_DESCRIPTOR_SET_SHADER_CONSTANTS (UINT8_MAX - 1)
#define ANV_DESCRIPTOR_SET_COLOR_ATTACHMENTS UINT8_MAX

struct anv_pipeline_binding {
   /** Index in the descriptor set
    *
    * This is a flattened index; the descriptor set layout is already taken
    * into account.
    */
   uint32_t index;

   /** The descriptor set this surface corresponds to.
    *
    * The special ANV_DESCRIPTOR_SET_* values above indicates that this
    * binding is not a normal descriptor set but something else.
    */
   uint8_t set;

   union {
      /** Plane in the binding index for images */
      uint8_t plane;

      /** Dynamic offset index (for dynamic UBOs and SSBOs) */
      uint8_t dynamic_offset_index;
   };

   /** For a storage image, whether it requires a lowered surface */
   uint8_t lowered_storage_surface;

   /** Pad to 64 bits so that there are no holes and we can safely memcmp
    * assuming POD zero-initialization.
    */
   uint8_t pad;
};

struct anv_push_range {
   /** Index in the descriptor set */
   uint32_t index;

   /** Descriptor set index */
   uint8_t set;

   /** Dynamic offset index (for dynamic UBOs) */
   uint8_t dynamic_offset_index;

   /** Start offset in units of 32B */
   uint8_t start;

   /** Range in units of 32B */
   uint8_t length;
};

struct anv_pipeline_layout {
   struct vk_object_base base;

   struct {
      struct anv_descriptor_set_layout *layout;
      uint32_t dynamic_offset_start;
   } set[MAX_SETS];

   uint32_t num_sets;

   unsigned char sha1[20];
};

struct anv_buffer {
   struct vk_buffer vk;

   /* Set when bound */
   struct anv_address address;
};

enum anv_cmd_dirty_bits {
   ANV_CMD_DIRTY_PIPELINE                            = 1 << 0,
   ANV_CMD_DIRTY_INDEX_BUFFER                        = 1 << 1,
   ANV_CMD_DIRTY_RENDER_TARGETS                      = 1 << 2,
   ANV_CMD_DIRTY_XFB_ENABLE                          = 1 << 3,
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

   ANV_PIPE_CS_STALL_BIT                     = (1 << 20),
   ANV_PIPE_END_OF_PIPE_SYNC_BIT             = (1 << 21),

   /* This bit does not exist directly in PIPE_CONTROL.  Instead it means that
    * a flush has happened but not a CS stall.  The next time we do any sort
    * of invalidation we need to insert a CS stall at that time.  Otherwise,
    * we would have to CS stall on every flush which could be bad.
    */
   ANV_PIPE_NEEDS_END_OF_PIPE_SYNC_BIT       = (1 << 22),

   /* This bit does not exist directly in PIPE_CONTROL. It means that render
    * target operations related to transfer commands with VkBuffer as
    * destination are ongoing. Some operations like copies on the command
    * streamer might need to be aware of this to trigger the appropriate stall
    * before they can proceed with the copy.
    */
   ANV_PIPE_RENDER_TARGET_BUFFER_WRITES      = (1 << 23),

   /* This bit does not exist directly in PIPE_CONTROL. It means that Gfx12
    * AUX-TT data has changed and we need to invalidate AUX-TT data.  This is
    * done by writing the AUX-TT register.
    */
   ANV_PIPE_AUX_TABLE_INVALIDATE_BIT         = (1 << 24),

   /* This bit does not exist directly in PIPE_CONTROL. It means that a
    * PIPE_CONTROL with a post-sync operation will follow. This is used to
    * implement a workaround for Gfx9.
    */
   ANV_PIPE_POST_SYNC_BIT                    = (1 << 25),
};

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
   ANV_PIPE_CS_STALL_BIT)

#define ANV_PIPE_INVALIDATE_BITS ( \
   ANV_PIPE_STATE_CACHE_INVALIDATE_BIT | \
   ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT | \
   ANV_PIPE_VF_CACHE_INVALIDATE_BIT | \
   ANV_PIPE_HDC_PIPELINE_FLUSH_BIT | \
   ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT | \
   ANV_PIPE_INSTRUCTION_CACHE_INVALIDATE_BIT | \
   ANV_PIPE_AUX_TABLE_INVALIDATE_BIT)

enum intel_ds_stall_flag
anv_pipe_flush_bit_to_ds_stall_flag(enum anv_pipe_bits bits);

static inline enum anv_pipe_bits
anv_pipe_flush_bits_for_access_flags(struct anv_device *device,
                                     VkAccessFlags2 flags)
{
   enum anv_pipe_bits pipe_bits = 0;

   u_foreach_bit64(b, flags) {
      switch ((VkAccessFlags2)BITFIELD64_BIT(b)) {
      case VK_ACCESS_2_SHADER_WRITE_BIT:
      case VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT:
         /* We're transitioning a buffer that was previously used as write
          * destination through the data port. To make its content available
          * to future operations, flush the hdc pipeline.
          */
         pipe_bits |= ANV_PIPE_HDC_PIPELINE_FLUSH_BIT;
         pipe_bits |= ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT;
         break;
      case VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT:
         /* We're transitioning a buffer that was previously used as render
          * target. To make its content available to future operations, flush
          * the render target cache.
          */
         pipe_bits |= ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT;
         break;
      case VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
         /* We're transitioning a buffer that was previously used as depth
          * buffer. To make its content available to future operations, flush
          * the depth cache.
          */
         pipe_bits |= ANV_PIPE_DEPTH_CACHE_FLUSH_BIT;
         break;
      case VK_ACCESS_2_TRANSFER_WRITE_BIT:
         /* We're transitioning a buffer that was previously used as a
          * transfer write destination. Generic write operations include color
          * & depth operations as well as buffer operations like :
          *     - vkCmdClearColorImage()
          *     - vkCmdClearDepthStencilImage()
          *     - vkCmdBlitImage()
          *     - vkCmdCopy*(), vkCmdUpdate*(), vkCmdFill*()
          *
          * Most of these operations are implemented using Blorp which writes
          * through the render target, so flush that cache to make it visible
          * to future operations. And for depth related operations we also
          * need to flush the depth cache.
          */
         pipe_bits |= ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT;
         pipe_bits |= ANV_PIPE_DEPTH_CACHE_FLUSH_BIT;
         break;
      case VK_ACCESS_2_MEMORY_WRITE_BIT:
         /* We're transitioning a buffer for generic write operations. Flush
          * all the caches.
          */
         pipe_bits |= ANV_PIPE_FLUSH_BITS;
         break;
      case VK_ACCESS_2_HOST_WRITE_BIT:
         /* We're transitioning a buffer for access by CPU. Invalidate
          * all the caches. Since data and tile caches don't have invalidate,
          * we are forced to flush those as well.
          */
         pipe_bits |= ANV_PIPE_FLUSH_BITS;
         pipe_bits |= ANV_PIPE_INVALIDATE_BITS;
         break;
      case VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT:
      case VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT:
         /* We're transitioning a buffer written either from VS stage or from
          * the command streamer (see CmdEndTransformFeedbackEXT), we just
          * need to stall the CS.
          */
         pipe_bits |= ANV_PIPE_CS_STALL_BIT;
         break;
      default:
         break; /* Nothing to do */
      }
   }

   return pipe_bits;
}

static inline enum anv_pipe_bits
anv_pipe_invalidate_bits_for_access_flags(struct anv_device *device,
                                          VkAccessFlags2 flags)
{
   enum anv_pipe_bits pipe_bits = 0;

   u_foreach_bit64(b, flags) {
      switch ((VkAccessFlags2)BITFIELD64_BIT(b)) {
      case VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT:
         /* Indirect draw commands take a buffer as input that we're going to
          * read from the command streamer to load some of the HW registers
          * (see genX_cmd_buffer.c:load_indirect_parameters). This requires a
          * command streamer stall so that all the cache flushes have
          * completed before the command streamer loads from memory.
          */
         pipe_bits |=  ANV_PIPE_CS_STALL_BIT;
         /* Indirect draw commands also set gl_BaseVertex & gl_BaseIndex
          * through a vertex buffer, so invalidate that cache.
          */
         pipe_bits |= ANV_PIPE_VF_CACHE_INVALIDATE_BIT;
         /* For CmdDipatchIndirect, we also load gl_NumWorkGroups through a
          * UBO from the buffer, so we need to invalidate constant cache.
          */
         pipe_bits |= ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT;
         pipe_bits |= ANV_PIPE_DATA_CACHE_FLUSH_BIT;
         /* Tile cache flush needed For CmdDipatchIndirect since command
          * streamer and vertex fetch aren't L3 coherent.
          */
         pipe_bits |= ANV_PIPE_TILE_CACHE_FLUSH_BIT;
         break;
      case VK_ACCESS_2_INDEX_READ_BIT:
      case VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT:
         /* We transitioning a buffer to be used for as input for vkCmdDraw*
          * commands, so we invalidate the VF cache to make sure there is no
          * stale data when we start rendering.
          */
         pipe_bits |= ANV_PIPE_VF_CACHE_INVALIDATE_BIT;
         break;
      case VK_ACCESS_2_UNIFORM_READ_BIT:
         /* We transitioning a buffer to be used as uniform data. Because
          * uniform is accessed through the data port & sampler, we need to
          * invalidate the texture cache (sampler) & constant cache (data
          * port) to avoid stale data.
          */
         pipe_bits |= ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT;
         if (device->physical->compiler->indirect_ubos_use_sampler) {
            pipe_bits |= ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT;
         } else {
            pipe_bits |= ANV_PIPE_HDC_PIPELINE_FLUSH_BIT;
            pipe_bits |= ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT;
         }
         break;
      case VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT:
      case VK_ACCESS_2_TRANSFER_READ_BIT:
      case VK_ACCESS_2_SHADER_SAMPLED_READ_BIT:
         /* Transitioning a buffer to be read through the sampler, so
          * invalidate the texture cache, we don't want any stale data.
          */
         pipe_bits |= ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT;
         break;
      case VK_ACCESS_2_SHADER_READ_BIT:
         /* Same as VK_ACCESS_2_UNIFORM_READ_BIT and
          * VK_ACCESS_2_SHADER_SAMPLED_READ_BIT cases above
          */
         pipe_bits |= ANV_PIPE_CONSTANT_CACHE_INVALIDATE_BIT |
                      ANV_PIPE_TEXTURE_CACHE_INVALIDATE_BIT;
         if (!device->physical->compiler->indirect_ubos_use_sampler) {
            pipe_bits |= ANV_PIPE_HDC_PIPELINE_FLUSH_BIT;
            pipe_bits |= ANV_PIPE_UNTYPED_DATAPORT_CACHE_FLUSH_BIT;
         }
         break;
      case VK_ACCESS_2_MEMORY_READ_BIT:
         /* Transitioning a buffer for generic read, invalidate all the
          * caches.
          */
         pipe_bits |= ANV_PIPE_INVALIDATE_BITS;
         break;
      case VK_ACCESS_2_MEMORY_WRITE_BIT:
         /* Generic write, make sure all previously written things land in
          * memory.
          */
         pipe_bits |= ANV_PIPE_FLUSH_BITS;
         break;
      case VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT:
      case VK_ACCESS_2_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT:
         /* Transitioning a buffer for conditional rendering or transform
          * feedback. We'll load the content of this buffer into HW registers
          * using the command streamer, so we need to stall the command
          * streamer , so we need to stall the command streamer to make sure
          * any in-flight flush operations have completed.
          */
         pipe_bits |= ANV_PIPE_CS_STALL_BIT;
         pipe_bits |= ANV_PIPE_TILE_CACHE_FLUSH_BIT;
         pipe_bits |= ANV_PIPE_DATA_CACHE_FLUSH_BIT;
         break;
      case VK_ACCESS_2_HOST_READ_BIT:
         /* We're transitioning a buffer that was written by CPU.  Flush
          * all the caches.
          */
         pipe_bits |= ANV_PIPE_FLUSH_BITS;
         break;
      case VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT:
         /* We're transitioning a buffer to be written by the streamout fixed
          * function. This one is apparently not L3 coherent, so we need a
          * tile cache flush to make sure any previous write is not going to
          * create WaW hazards.
          */
         pipe_bits |= ANV_PIPE_TILE_CACHE_FLUSH_BIT;
         break;
      case VK_ACCESS_2_SHADER_STORAGE_READ_BIT:
      default:
         break; /* Nothing to do */
      }
   }

   return pipe_bits;
}

#define VK_IMAGE_ASPECT_ANY_COLOR_BIT_ANV (         \
   VK_IMAGE_ASPECT_COLOR_BIT | \
   VK_IMAGE_ASPECT_PLANE_0_BIT | \
   VK_IMAGE_ASPECT_PLANE_1_BIT | \
   VK_IMAGE_ASPECT_PLANE_2_BIT)
#define VK_IMAGE_ASPECT_PLANES_BITS_ANV ( \
   VK_IMAGE_ASPECT_PLANE_0_BIT | \
   VK_IMAGE_ASPECT_PLANE_1_BIT | \
   VK_IMAGE_ASPECT_PLANE_2_BIT)

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

   /** Dynamic offsets for dynamic UBOs and SSBOs */
   uint32_t dynamic_offsets[MAX_DYNAMIC_BUFFERS];

   /* Robust access pushed registers. */
   uint64_t push_reg_mask[MESA_SHADER_STAGES];

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

struct anv_surface_state {
   struct anv_state state;
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

   assert(vb_address.bo && anv_bo_is_pinned(vb_address.bo));
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
   struct anv_push_descriptor_set *push_descriptors[MAX_SETS];

   struct anv_push_constants push_constants;

   /* Push constant state allocated when flushing push constants. */
   struct anv_state          push_constants_state;
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

   struct anv_graphics_pipeline *pipeline;

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

   struct anv_buffer *index_buffer;
   uint32_t index_type; /**< 3DSTATE_INDEX_BUFFER.IndexFormat */
   uint32_t index_offset;

   struct vk_sample_locations_state sample_locations;

   bool has_uint_rt;
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

   struct anv_compute_pipeline *pipeline;

   bool pipeline_dirty;

   struct anv_state push_data;

   struct anv_address num_workgroups;
};

/** State required while building cmd buffer */
struct anv_cmd_state {
   /* PIPELINE_SELECT.PipelineSelection */
   uint32_t                                     current_pipeline;
   const struct intel_l3_config *               current_l3_config;

   struct anv_cmd_graphics_state                gfx;
   struct anv_cmd_compute_state                 compute;

   enum anv_pipe_bits                           pending_pipe_bits;
   VkShaderStageFlags                           descriptors_dirty;
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

   bool                                         conditional_render_enabled;

   /**
    * Last rendering scale argument provided to
    * genX(cmd_buffer_emit_hashing_mode)().
    */
   unsigned                                     current_hash_scale;
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
   /** Last seen surface state block pool center bo offset */
   uint32_t                                     last_ss_pool_center;

   /* Serial for tracking buffer completion */
   uint32_t                                     serial;

   /* Stream objects for storing temporary data */
   struct anv_state_stream                      surface_state_stream;
   struct anv_state_stream                      dynamic_state_stream;
   struct anv_state_stream                      general_state_stream;

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
    * Sum of all the anv_batch_bo sizes allocated for this command buffer.
    * Used to increase allocation size for long command buffers.
    */
   uint32_t                                     total_batch_size;

   /**
    *
    */
   struct u_trace                               trace;
};

extern const struct vk_command_buffer_ops anv_cmd_buffer_ops;

/* Determine whether we can chain a given cmd_buffer to another one. We need
 * softpin and we also need to make sure that we can edit the end of the batch
 * to point to next one, which requires the command buffer to not be used
 * simultaneously.
 */
static inline bool
anv_cmd_buffer_is_chainable(struct anv_cmd_buffer *cmd_buffer)
{
   return !anv_use_relocations(cmd_buffer->device->physical) &&
      !(cmd_buffer->usage_flags & VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
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
anv_cmd_buffer_alloc_surface_state(struct anv_cmd_buffer *cmd_buffer);
struct anv_state
anv_cmd_buffer_alloc_dynamic_state(struct anv_cmd_buffer *cmd_buffer,
                                   uint32_t size, uint32_t alignment);

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

   struct anv_pipeline_binding *                surface_to_descriptor;
   struct anv_pipeline_binding *                sampler_to_descriptor;

   struct anv_push_range                        push_ranges[4];
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

   struct anv_pipeline_bind_map bind_map;
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
                      const struct anv_pipeline_bind_map *bind_map);

static inline void
anv_shader_bin_ref(struct anv_shader_bin *shader)
{
   vk_pipeline_cache_object_ref(&shader->base);
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
   ANV_PIPELINE_COMPUTE,
};

struct anv_pipeline {
   struct vk_object_base                        base;

   struct anv_device *                          device;

   struct anv_batch                             batch;
   struct anv_reloc_list                        batch_relocs;

   void *                                       mem_ctx;

   enum anv_pipeline_type                       type;
   VkPipelineCreateFlags                        flags;

   struct util_dynarray                         executables;

   const struct intel_l3_config *               l3_config;
};

struct anv_graphics_pipeline {
   struct anv_pipeline                          base;

   /* Shaders */
   struct anv_shader_bin *                      shaders[ANV_GRAPHICS_SHADER_STAGE_COUNT];

   VkShaderStageFlags                           active_stages;

   struct vk_sample_locations_state             sample_locations;
   struct vk_dynamic_graphics_state             dynamic_state;

   /* These fields are required with dynamic primitive topology,
    * rasterization_samples used only with gen < 8.
    */
   VkLineRasterizationModeEXT                   line_mode;
   VkPolygonMode                                polygon_mode;
   uint32_t                                     patch_control_points;
   uint32_t                                     rasterization_samples;

   VkColorComponentFlags                        color_comp_writes[MAX_RTS];

   uint32_t                                     view_mask;
   uint32_t                                     instance_multiplier;

   bool                                         depth_clamp_enable;
   bool                                         depth_clip_enable;
   bool                                         kill_pixel;
   bool                                         force_fragment_thread_dispatch;
   bool                                         negative_one_to_one;

   uint32_t                                     vb_used;
   struct anv_pipeline_vertex_binding {
      uint32_t                                  stride;
      bool                                      instanced;
      uint32_t                                  instance_divisor;
   } vb[MAX_VBS];

   /* Pre computed CS instructions that can directly be copied into
    * anv_cmd_buffer.
    */
   uint32_t                                     batch_data[512];

   /* Pre packed CS instructions & structures that need to be merged later
    * with dynamic state.
    */
   struct {
      uint32_t                                  sf[7];
      uint32_t                                  clip[4];
      uint32_t                                  xfb_bo_pitch[4];
      uint32_t                                  wm[3];
      uint32_t                                  blend_state[MAX_RTS * 2];
      uint32_t                                  streamout_state[3];
   } gfx7;

   struct {
      uint32_t                                  sf[4];
      uint32_t                                  raster[5];
      uint32_t                                  wm[2];
      uint32_t                                  ps_blend[2];
      uint32_t                                  blend_state[1 + MAX_RTS * 2];
      uint32_t                                  streamout_state[5];
   } gfx8;
};

struct anv_compute_pipeline {
   struct anv_pipeline                          base;

   struct anv_shader_bin *                      cs;
   uint32_t                                     batch_data[9];
   uint32_t                                     interface_descriptor_data[8];
};

#define ANV_DECL_PIPELINE_DOWNCAST(pipe_type, pipe_enum)             \
   static inline struct anv_##pipe_type##_pipeline *                 \
   anv_pipeline_to_##pipe_type(struct anv_pipeline *pipeline)      \
   {                                                                 \
      assert(pipeline->type == pipe_enum);                           \
      return (struct anv_##pipe_type##_pipeline *) pipeline;         \
   }

ANV_DECL_PIPELINE_DOWNCAST(graphics, ANV_PIPELINE_GRAPHICS)
ANV_DECL_PIPELINE_DOWNCAST(compute, ANV_PIPELINE_COMPUTE)

static inline bool
anv_pipeline_has_stage(const struct anv_graphics_pipeline *pipeline,
                       gl_shader_stage stage)
{
   return (pipeline->active_stages & mesa_to_vk_shader_stage(stage)) != 0;
}

static inline bool
anv_pipeline_is_primitive(const struct anv_graphics_pipeline *pipeline)
{
   return anv_pipeline_has_stage(pipeline, MESA_SHADER_VERTEX);
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
      if (state->pipeline->color_comp_writes[i] != 0)
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
             pipeline->shaders[stage]->prog_data;                       \
   } else {                                                             \
      return NULL;                                                      \
   }                                                                    \
}

ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(vs, MESA_SHADER_VERTEX)
ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(tcs, MESA_SHADER_TESS_CTRL)
ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(tes, MESA_SHADER_TESS_EVAL)
ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(gs, MESA_SHADER_GEOMETRY)
ANV_DECL_GET_GRAPHICS_PROG_DATA_FUNC(wm, MESA_SHADER_FRAGMENT)

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
anv_pipeline_init(struct anv_pipeline *pipeline,
                  struct anv_device *device,
                  enum anv_pipeline_type type,
                  VkPipelineCreateFlags flags,
                  const VkAllocationCallbacks *pAllocator);

void
anv_pipeline_finish(struct anv_pipeline *pipeline,
                    struct anv_device *device,
                    const VkAllocationCallbacks *pAllocator);

struct anv_format_plane {
   enum isl_format isl_format:16;
   struct isl_swizzle swizzle;

   /* Whether this plane contains chroma channels */
   bool has_chroma;

   /* For downscaling of YUV planes */
   uint8_t denominator_scales[2];

   /* How to map sampled ycbcr planes to a single 4 component element. */
   struct isl_swizzle ycbcr_swizzle;

   /* What aspect is associated to this plane */
   VkImageAspectFlags aspect;
};


struct anv_format {
   struct anv_format_plane planes[3];
   VkFormat vk_format;
   uint8_t n_planes;
   bool can_ycbcr;
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

extern VkFormat
vk_format_from_android(unsigned android_format, unsigned android_usage);

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
       * A surface which shadows the main surface and may have different
       * tiling. This is used for sampling using a tiling that isn't supported
       * for other operations.
       */
      struct anv_surface shadow_surface;

      /**
       * The base aux usage for this image.  For color images, this can be
       * either CCS_E or CCS_D depending on whether or not we can reliably
       * leave CCS on all the time.
       */
      enum isl_aux_usage aux_usage;

      struct anv_surface aux_surface;

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
   } planes[3];
};

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

   const unsigned clear_color_state_size = device->info->ver >= 10 ?
      device->isl_dev.ss.clear_color_state_size :
      device->isl_dev.ss.clear_value_size;
   return anv_address_add(addr, clear_color_state_size);
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

   /* Allow this feature on BDW even though it is disabled in the BDW devinfo
    * struct. There's documentation which suggests that this feature actually
    * reduces performance on BDW, but it has only been observed to help so
    * far. Sampling fast-cleared blocks on BDW must also be handled with care
    * (see depth_stencil_attachment_compute_aux_usage() for more info).
    */
   if (devinfo->ver != 8 && !devinfo->has_sample_with_hiz)
      return false;

   return image->vk.samples == 1;
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
anv_image_msaa_resolve(struct anv_cmd_buffer *cmd_buffer,
                       const struct anv_image *src_image,
                       enum isl_aux_usage src_aux_usage,
                       uint32_t src_level, uint32_t src_base_layer,
                       const struct anv_image *dst_image,
                       enum isl_aux_usage dst_aux_usage,
                       uint32_t dst_level, uint32_t dst_base_layer,
                       VkImageAspectFlagBits aspect,
                       uint32_t src_x, uint32_t src_y,
                       uint32_t dst_x, uint32_t dst_y,
                       uint32_t width, uint32_t height,
                       uint32_t layer_count,
                       enum blorp_filter filter);
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

void
anv_image_copy_to_shadow(struct anv_cmd_buffer *cmd_buffer,
                         const struct anv_image *image,
                         VkImageAspectFlagBits aspect,
                         uint32_t base_level, uint32_t level_count,
                         uint32_t base_layer, uint32_t layer_count);

enum isl_aux_state ATTRIBUTE_PURE
anv_layout_to_aux_state(const struct intel_device_info * const devinfo,
                        const struct anv_image *image,
                        const VkImageAspectFlagBits aspect,
                        const VkImageLayout layout);

enum isl_aux_usage ATTRIBUTE_PURE
anv_layout_to_aux_usage(const struct intel_device_info * const devinfo,
                        const struct anv_image *image,
                        const VkImageAspectFlagBits aspect,
                        const VkImageUsageFlagBits usage,
                        const VkImageLayout layout);

enum anv_fast_clear_type ATTRIBUTE_PURE
anv_layout_to_fast_clear_type(const struct intel_device_info * const devinfo,
                              const struct anv_image * const image,
                              const VkImageAspectFlagBits aspect,
                              const VkImageLayout layout);

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
   struct {
      uint32_t image_plane;

      struct isl_view isl;

      /**
       * RENDER_SURFACE_STATE when using image as a sampler surface with an
       * image layout of SHADER_READ_ONLY_OPTIMAL or
       * DEPTH_STENCIL_READ_ONLY_OPTIMAL.
       */
      struct anv_surface_state optimal_sampler_surface_state;

      /**
       * RENDER_SURFACE_STATE when using image as a sampler surface with an
       * image layout of GENERAL.
       */
      struct anv_surface_state general_sampler_surface_state;

      /**
       * RENDER_SURFACE_STATE when using image as a storage image. Separate
       * states for vanilla (with the original format) and one which has been
       * lowered to a format suitable for reading.  This may be a raw surface
       * in extreme cases or simply a surface with a different format where we
       * expect some conversion to be done in the shader.
       */
      struct anv_surface_state storage_surface_state;
      struct anv_surface_state lowered_storage_surface_state;

      struct brw_image_param lowered_storage_image_param;
   } planes[3];
};

enum anv_image_view_state_flags {
   ANV_IMAGE_VIEW_STATE_STORAGE_LOWERED      = (1 << 0),
   ANV_IMAGE_VIEW_STATE_TEXTURE_OPTIMAL      = (1 << 1),
};

void anv_image_fill_surface_state(struct anv_device *device,
                                  const struct anv_image *image,
                                  VkImageAspectFlagBits aspect,
                                  const struct isl_view *view,
                                  isl_surf_usage_flags_t view_usage,
                                  enum isl_aux_usage aux_usage,
                                  const union isl_color_value *clear_color,
                                  enum anv_image_view_state_flags flags,
                                  struct anv_surface_state *state_inout,
                                  struct brw_image_param *image_param_out);

struct anv_image_create_info {
   const VkImageCreateInfo *vk_info;

   /** An opt-in bitmask which filters an ISL-mapping of the Vulkan tiling. */
   isl_tiling_flags_t isl_tiling_flags;

   /** These flags will be added to any derived from VkImageCreateInfo. */
   isl_surf_usage_flags_t isl_extra_usage_flags;
};

VkResult anv_image_init(struct anv_device *device, struct anv_image *image,
                        struct anv_image_create_info *create_info);

void anv_image_finish(struct anv_image *image);

void anv_image_get_memory_requirements(struct anv_device *device,
                                       struct anv_image *image,
                                       VkImageAspectFlags aspects,
                                       VkMemoryRequirements2 *pMemoryRequirements);

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

VkFormatFeatureFlags2
anv_get_image_format_features2(const struct intel_device_info *devinfo,
                               VkFormat vk_format,
                               const struct anv_format *anv_format,
                               VkImageTiling vk_tiling,
                               const struct isl_drm_modifier_info *isl_mod_info);

void anv_fill_buffer_surface_state(struct anv_device *device,
                                   struct anv_state state,
                                   enum isl_format format,
                                   struct isl_swizzle swizzle,
                                   isl_surf_usage_flags_t usage,
                                   struct anv_address address,
                                   uint32_t range, uint32_t stride);


/* Haswell border color is a bit of a disaster.  Float and unorm formats use a
 * straightforward 32-bit float color in the first 64 bytes.  Instead of using
 * a nice float/integer union like Gfx8+, Haswell specifies the integer border
 * color as a separate entry /after/ the float color.  The layout of this entry
 * also depends on the format's bpp (with extra hacks for RG32), and overlaps.
 *
 * Since we don't know the format/bpp, we can't make any of the border colors
 * containing '1' work for all formats, as it would be in the wrong place for
 * some of them.  We opt to make 32-bit integers work as this seems like the
 * most common option.  Fortunately, transparent black works regardless, as
 * all zeroes is the same in every bit-size.
 */
struct hsw_border_color {
   float float32[4];
   uint32_t _pad0[12];
   uint32_t uint32[4];
   uint32_t _pad1[108];
};

struct gfx8_border_color {
   union {
      float float32[4];
      uint32_t uint32[4];
   };
   /* Pad out to 64 bytes */
   uint32_t _pad[12];
};

struct anv_sampler {
   struct vk_object_base        base;

   uint32_t                     state[3][4];
   uint32_t                     n_planes;
   struct vk_ycbcr_conversion  *conversion;

   /* Blob of sampler state data which is guaranteed to be 32-byte aligned
    * and with a 32-byte stride for use as bindless samplers.
    */
   struct anv_state             bindless_state;

   struct anv_state             custom_border_color;
};

#define ANV_PIPELINE_STATISTICS_MASK 0x000007ff

struct anv_query_pool {
   struct vk_object_base                        base;

   VkQueryType                                  type;
   VkQueryPipelineStatisticFlags                pipeline_statistics;
   /** Stride between slots, in bytes */
   uint32_t                                     stride;
   /** Number of slots in this query pool */
   uint32_t                                     slots;
   struct anv_bo *                              bo;

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
   return pool->pass_size * pass + 8;
}

void
anv_dump_pipe_bits(enum anv_pipe_bits bits);

static inline void
anv_add_pending_pipe_bits(struct anv_cmd_buffer* cmd_buffer,
                          enum anv_pipe_bits bits,
                          const char* reason)
{
   cmd_buffer->state.pending_pipe_bits |= bits;
   if (INTEL_DEBUG(DEBUG_PIPE_CONTROL) && bits)
   {
      fputs("pc: add ", stderr);
      anv_dump_pipe_bits(bits);
      fprintf(stderr, "reason: %s\n", reason);
   }
}

struct anv_performance_configuration_intel {
   struct vk_object_base      base;

   struct intel_perf_registers *register_config;

   uint64_t                   config_id;
};

void anv_physical_device_init_perf(struct anv_physical_device *device, int fd);
void anv_device_perf_init(struct anv_device *device);
void anv_perf_write_pass_results(struct intel_perf_config *perf,
                                 struct anv_query_pool *pool, uint32_t pass,
                                 const struct intel_perf_query_result *accumulated_results,
                                 union VkPerformanceCounterResultKHR *results);

/* Use to emit a series of memcpy operations */
struct anv_memcpy_state {
   struct anv_device *device;
   struct anv_batch *batch;

   struct anv_vb_cache_range vb_bound;
   struct anv_vb_cache_range vb_dirty;
};

struct anv_utrace_flush_copy {
   /* Needs to be the first field */
   struct intel_ds_flush_data ds;

   /* Batch stuff to implement of copy of timestamps recorded in another
    * buffer.
    */
   struct anv_reloc_list relocs;
   struct anv_batch batch;
   struct anv_bo *batch_bo;

   /* Buffer of 64bits timestamps */
   struct anv_bo *trace_bo;

   /* Syncobj to be signaled when the batch completes */
   struct vk_sync *sync;

   /* Queue on which all the recorded traces are submitted */
   struct anv_queue *queue;

   struct anv_memcpy_state memcpy_state;
};

void anv_device_utrace_init(struct anv_device *device);
void anv_device_utrace_finish(struct anv_device *device);
VkResult
anv_device_utrace_flush_cmd_buffers(struct anv_queue *queue,
                                    uint32_t cmd_buffer_count,
                                    struct anv_cmd_buffer **cmd_buffers,
                                    struct anv_utrace_flush_copy **out_flush_data);

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
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_buffer_view, base, VkBufferView,
                               VK_OBJECT_TYPE_BUFFER_VIEW)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_descriptor_pool, base, VkDescriptorPool,
                               VK_OBJECT_TYPE_DESCRIPTOR_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_descriptor_set, base, VkDescriptorSet,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_descriptor_set_layout, base,
                               VkDescriptorSetLayout,
                               VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_device_memory, base, VkDeviceMemory,
                               VK_OBJECT_TYPE_DEVICE_MEMORY)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_event, base, VkEvent, VK_OBJECT_TYPE_EVENT)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_image, vk.base, VkImage, VK_OBJECT_TYPE_IMAGE)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_image_view, vk.base, VkImageView,
                               VK_OBJECT_TYPE_IMAGE_VIEW);
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_pipeline, base, VkPipeline,
                               VK_OBJECT_TYPE_PIPELINE)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_pipeline_layout, base, VkPipelineLayout,
                               VK_OBJECT_TYPE_PIPELINE_LAYOUT)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_query_pool, base, VkQueryPool,
                               VK_OBJECT_TYPE_QUERY_POOL)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_sampler, base, VkSampler,
                               VK_OBJECT_TYPE_SAMPLER)
VK_DEFINE_NONDISP_HANDLE_CASTS(anv_performance_configuration_intel, base,
                               VkPerformanceConfigurationINTEL,
                               VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL)

#define anv_genX(devinfo, thing) ({             \
   __typeof(&gfx7_##thing) genX_thing;          \
   switch ((devinfo)->verx10) {                 \
   case 70:                                     \
      genX_thing = &gfx7_##thing;               \
      break;                                    \
   case 75:                                     \
      genX_thing = &gfx75_##thing;              \
      break;                                    \
   case 80:                                     \
      genX_thing = &gfx8_##thing;               \
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
#  define genX(x) gfx7_##x
#  include "anv_genX.h"
#  undef genX
#  define genX(x) gfx75_##x
#  include "anv_genX.h"
#  undef genX
#  define genX(x) gfx8_##x
#  include "anv_genX.h"
#  undef genX
#endif

#endif /* ANV_PRIVATE_H */
