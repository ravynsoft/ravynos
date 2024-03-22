/*
 * Copyright © 2022 Valve Corporation
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
 * 
 * Authors:
 *    Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
 */

#ifndef ZINK_TYPES_H
#define ZINK_TYPES_H

#include <vulkan/vulkan_core.h>

#include "compiler/nir/nir.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_state.h"

#include "pipebuffer/pb_cache.h"
#include "pipebuffer/pb_slab.h"

#include "util/disk_cache.h"
#include "util/hash_table.h"
#include "util/list.h"
#include "util/log.h"
#include "util/rwlock.h"
#include "util/set.h"
#include "util/simple_mtx.h"
#include "util/slab.h"
#include "util/u_dynarray.h"
#include "util/u_idalloc.h"
#include "util/u_live_shader_cache.h"
#include "util/u_queue.h"
#include "util/u_range.h"
#include "util/u_threaded_context.h"
#include "util/u_transfer.h"
#include "util/u_vertex_state_cache.h"

#include "vulkan/util/vk_util.h"

#include "zink_device_info.h"
#include "zink_instance.h"
#include "zink_shader_keys.h"
#include "vk_dispatch_table.h"

#ifdef HAVE_RENDERDOC_APP_H
#include "renderdoc_app.h"
#endif

/* the descriptor binding id for fbfetch/input attachment */
#define ZINK_FBFETCH_BINDING 5
#define ZINK_GFX_SHADER_COUNT 5

/* number of descriptors to allocate in a pool */
#define MAX_LAZY_DESCRIPTORS 500
/* explicit clamping because descriptor caching used to exist */
#define ZINK_MAX_SHADER_IMAGES 32
/* total number of bindless ids that can be allocated */
#define ZINK_MAX_BINDLESS_HANDLES 1024

/* enum zink_descriptor_type */
#define ZINK_MAX_DESCRIPTOR_SETS 6
#define ZINK_MAX_DESCRIPTORS_PER_TYPE (32 * ZINK_GFX_SHADER_COUNT)
/* Descriptor size reported by lavapipe. */
#define ZINK_FBFETCH_DESCRIPTOR_SIZE 280

/* suballocator defines */
#define NUM_SLAB_ALLOCATORS 3
#define MIN_SLAB_ORDER 8


/* this is the spec minimum */
#define ZINK_SPARSE_BUFFER_PAGE_SIZE (64 * 1024)

/* flag to create screen->copy_context */
#define ZINK_CONTEXT_COPY_ONLY (1<<30)

/* convenience macros for accessing dispatch table functions */
#define VKCTX(fn) zink_screen(ctx->base.screen)->vk.fn
#define VKSCR(fn) screen->vk.fn

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t zink_debug;
extern bool zink_tracing;

#ifdef __cplusplus
}
#endif


/** enums */

/* features for draw/program templates */
typedef enum {
   ZINK_NO_MULTIDRAW,
   ZINK_MULTIDRAW,
} zink_multidraw;

typedef enum {
   ZINK_NO_DYNAMIC_STATE,
   ZINK_DYNAMIC_STATE,
   ZINK_DYNAMIC_STATE2,
   ZINK_DYNAMIC_VERTEX_INPUT2,
   ZINK_DYNAMIC_STATE3,
   ZINK_DYNAMIC_VERTEX_INPUT,
} zink_dynamic_state;

typedef enum {
   ZINK_PIPELINE_NO_DYNAMIC_STATE,
   ZINK_PIPELINE_DYNAMIC_STATE,
   ZINK_PIPELINE_DYNAMIC_STATE2,
   ZINK_PIPELINE_DYNAMIC_STATE2_PCP,
   ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT2,
   ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT2_PCP,
   ZINK_PIPELINE_DYNAMIC_STATE3,
   ZINK_PIPELINE_DYNAMIC_STATE3_PCP,
   ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT,
   ZINK_PIPELINE_DYNAMIC_VERTEX_INPUT_PCP,
} zink_pipeline_dynamic_state;

enum zink_blit_flags {
   ZINK_BLIT_NORMAL = 1 << 0,
   ZINK_BLIT_SAVE_FS = 1 << 1,
   ZINK_BLIT_SAVE_FB = 1 << 2,
   ZINK_BLIT_SAVE_TEXTURES = 1 << 3,
   ZINK_BLIT_NO_COND_RENDER = 1 << 4,
   ZINK_BLIT_SAVE_FS_CONST_BUF = 1 << 5,
};

/* descriptor types; also the ordering of the sets
 * ...except that ZINK_DESCRIPTOR_BASE_TYPES is actually ZINK_DESCRIPTOR_TYPE_UNIFORMS,
 * and all base type values are thus +1 to get the set id (using screen->desc_set_id[idx])
 */
enum zink_descriptor_type {
   ZINK_DESCRIPTOR_TYPE_UBO,
   ZINK_DESCRIPTOR_TYPE_SAMPLER_VIEW,
   ZINK_DESCRIPTOR_TYPE_SSBO,
   ZINK_DESCRIPTOR_TYPE_IMAGE,
   ZINK_DESCRIPTOR_BASE_TYPES, /**< the count/iterator for basic descriptor types */
   ZINK_DESCRIPTOR_BINDLESS,
   ZINK_DESCRIPTOR_ALL_TYPES,
   ZINK_DESCRIPTOR_TYPE_UNIFORMS = ZINK_DESCRIPTOR_BASE_TYPES, /**< this is aliased for convenience */
   ZINK_DESCRIPTOR_NON_BINDLESS_TYPES = ZINK_DESCRIPTOR_BASE_TYPES + 1, /**< for struct sizing */
};

enum zink_descriptor_mode {
   ZINK_DESCRIPTOR_MODE_AUTO,
   ZINK_DESCRIPTOR_MODE_LAZY,
   ZINK_DESCRIPTOR_MODE_DB,
};

/* the current mode */
extern enum zink_descriptor_mode zink_descriptor_mode;

/* indexing for descriptor template management */
enum zink_descriptor_size_index {
   ZDS_INDEX_UBO,
   ZDS_INDEX_COMBINED_SAMPLER,
   ZDS_INDEX_UNIFORM_TEXELS,
   ZDS_INDEX_SAMPLER,
   ZDS_INDEX_STORAGE_BUFFER,
   ZDS_INDEX_STORAGE_IMAGE,
   ZDS_INDEX_STORAGE_TEXELS,
   ZDS_INDEX_MAX,
};

/* indexing for descriptor template management in COMPACT mode */
enum zink_descriptor_size_index_compact {
   ZDS_INDEX_COMP_UBO,
   ZDS_INDEX_COMP_STORAGE_BUFFER,
   ZDS_INDEX_COMP_COMBINED_SAMPLER,
   ZDS_INDEX_COMP_UNIFORM_TEXELS,
   ZDS_INDEX_COMP_SAMPLER,
   ZDS_INDEX_COMP_STORAGE_IMAGE,
   ZDS_INDEX_COMP_STORAGE_TEXELS,
};

enum zink_resource_access {
   ZINK_RESOURCE_ACCESS_READ = 1,
   ZINK_RESOURCE_ACCESS_WRITE = 32,
   ZINK_RESOURCE_ACCESS_RW = ZINK_RESOURCE_ACCESS_READ | ZINK_RESOURCE_ACCESS_WRITE,
};


/* zink heaps are based off of vulkan memory types, but are not a 1-to-1 mapping to vulkan memory type indices and have no direct relation to vulkan memory heaps*/
enum zink_heap {
   ZINK_HEAP_DEVICE_LOCAL,
   ZINK_HEAP_DEVICE_LOCAL_SPARSE,
   ZINK_HEAP_DEVICE_LOCAL_LAZY,
   ZINK_HEAP_DEVICE_LOCAL_VISIBLE,
   ZINK_HEAP_HOST_VISIBLE_COHERENT,
   ZINK_HEAP_HOST_VISIBLE_COHERENT_CACHED,
   ZINK_HEAP_MAX,
};

enum zink_alloc_flag {
   ZINK_ALLOC_SPARSE = 1<<0,
   ZINK_ALLOC_NO_SUBALLOC = 1<<1,
};

enum zink_debug {
   ZINK_DEBUG_NIR = (1<<0),
   ZINK_DEBUG_SPIRV = (1<<1),
   ZINK_DEBUG_TGSI = (1<<2),
   ZINK_DEBUG_VALIDATION = (1<<3),
   ZINK_DEBUG_SYNC = (1<<4),
   ZINK_DEBUG_COMPACT = (1<<5),
   ZINK_DEBUG_NOREORDER = (1<<6),
   ZINK_DEBUG_GPL = (1<<7),
   ZINK_DEBUG_SHADERDB = (1<<8),
   ZINK_DEBUG_RP = (1<<9),
   ZINK_DEBUG_NORP = (1<<10),
   ZINK_DEBUG_MAP = (1<<11),
   ZINK_DEBUG_FLUSHSYNC = (1<<12),
   ZINK_DEBUG_NOSHOBJ = (1<<13),
   ZINK_DEBUG_OPTIMAL_KEYS = (1<<14),
   ZINK_DEBUG_NOOPT = (1<<15),
   ZINK_DEBUG_NOBGC = (1<<16),
   ZINK_DEBUG_DGC = (1<<17),
   ZINK_DEBUG_MEM = (1<<18),
   ZINK_DEBUG_QUIET = (1<<19),
};

enum zink_pv_emulation_primitive {
   ZINK_PVE_PRIMITIVE_NONE = 0,
   ZINK_PVE_PRIMITIVE_SIMPLE = 1,
   /* when triangle or quad strips are used and the gs outputs triangles */
   ZINK_PVE_PRIMITIVE_TRISTRIP = 2,
   ZINK_PVE_PRIMITIVE_FAN = 3,
};

enum zink_dgc_buffer {
   ZINK_DGC_VBO,
   ZINK_DGC_IB,
   ZINK_DGC_PSO,
   ZINK_DGC_PUSH,
   ZINK_DGC_DRAW,
   ZINK_DGC_MAX,
};

/** fence types */
struct tc_unflushed_batch_token;

/* an async fence created for tc */
struct zink_tc_fence {
   struct pipe_reference reference;
   /* enables distinction between tc fence submission and vk queue submission */
   uint32_t submit_count;
   /* when the tc fence is signaled for use */
   struct util_queue_fence ready;
   struct tc_unflushed_batch_token *tc_token;
   /* for deferred flushes */
   struct pipe_context *deferred_ctx;
   /* multiple tc fences may point to a real fence */
   struct zink_fence *fence;
   /* for use with semaphore/imported fences */
   VkSemaphore sem;
};

/* a fence is actually a zink_batch_state, but these are split out for logical consistency */
struct zink_fence {
   uint64_t batch_id;
   bool submitted;
   bool completed;
   struct util_dynarray mfences;
};


/** state types */

struct zink_vertex_elements_hw_state {
   uint32_t hash;
   uint32_t num_bindings, num_attribs;
   /* VK_EXT_vertex_input_dynamic_state uses different types */
   union {
      VkVertexInputAttributeDescription attribs[PIPE_MAX_ATTRIBS];
      VkVertexInputAttributeDescription2EXT dynattribs[PIPE_MAX_ATTRIBS];
   };
   union {
      struct {
         VkVertexInputBindingDivisorDescriptionEXT divisors[PIPE_MAX_ATTRIBS];
         VkVertexInputBindingDescription bindings[PIPE_MAX_ATTRIBS]; // combination of element_state and stride
         VkDeviceSize strides[PIPE_MAX_ATTRIBS];
         uint8_t divisors_present;
      } b;
      VkVertexInputBindingDescription2EXT dynbindings[PIPE_MAX_ATTRIBS];
   };
   uint8_t binding_map[PIPE_MAX_ATTRIBS];
};

struct zink_vertex_elements_state {
   /* decomposed attributes read only a single component for format compatibility */
   bool has_decomposed_attrs;
   struct {
      uint32_t binding;
      VkVertexInputRate inputRate;
   } bindings[PIPE_MAX_ATTRIBS];
   uint32_t divisor[PIPE_MAX_ATTRIBS];
   uint32_t min_stride[PIPE_MAX_ATTRIBS]; //for dynamic_state1
   uint32_t decomposed_attrs;
   unsigned decomposed_attrs_size;
   uint32_t decomposed_attrs_without_w;
   unsigned decomposed_attrs_without_w_size;
   struct zink_vertex_elements_hw_state hw_state;
};

/* for vertex state draws */
struct zink_vertex_state {
   struct pipe_vertex_state b;
   struct zink_vertex_elements_state velems;
};

struct zink_rasterizer_hw_state {
   unsigned polygon_mode : 2; //VkPolygonMode
   unsigned line_mode : 2; //VkLineRasterizationModeEXT
   unsigned depth_clip:1;
   unsigned depth_clamp:1;
   unsigned pv_last:1;
   unsigned line_stipple_enable:1;
   unsigned clip_halfz:1;
};

struct zink_rasterizer_state {
   struct pipe_rasterizer_state base;
   bool offset_fill;
   float offset_units, offset_clamp, offset_scale;
   float line_width;
   VkFrontFace front_face;
   VkCullModeFlags cull_mode;
   VkLineRasterizationModeEXT dynamic_line_mode;
   struct zink_rasterizer_hw_state hw_state;
};

struct zink_blend_state {
   uint32_t hash;
   unsigned num_rts;
   VkPipelineColorBlendAttachmentState attachments[PIPE_MAX_COLOR_BUFS];

   struct {
      VkBool32 enables[PIPE_MAX_COLOR_BUFS];
      VkColorBlendEquationEXT eq[PIPE_MAX_COLOR_BUFS];
      VkColorComponentFlags wrmask[PIPE_MAX_COLOR_BUFS];
   } ds3;

   VkBool32 logicop_enable;
   VkLogicOp logicop_func;

   VkBool32 alpha_to_coverage;
   VkBool32 alpha_to_one;

   uint32_t wrmask;
   uint8_t enables;

   bool dual_src_blend;
};

struct zink_depth_stencil_alpha_hw_state {
   VkBool32 depth_test;
   VkCompareOp depth_compare_op;

   VkBool32 depth_bounds_test;
   float min_depth_bounds, max_depth_bounds;

   VkBool32 stencil_test;
   VkStencilOpState stencil_front;
   VkStencilOpState stencil_back;

   VkBool32 depth_write;
};

struct zink_depth_stencil_alpha_state {
   struct pipe_depth_stencil_alpha_state base;
   struct zink_depth_stencil_alpha_hw_state hw_state;
};


/** descriptor types */

/* zink_descriptor_layout objects are cached: this is the key for one */
struct zink_descriptor_layout_key {
   unsigned num_bindings;
   VkDescriptorSetLayoutBinding *bindings;
};

struct zink_descriptor_layout {
   VkDescriptorSetLayout layout;
};

/* descriptor pools are cached: zink_descriptor_pool_key::id is the id for a type of pool */
struct zink_descriptor_pool_key {
   unsigned use_count;
   unsigned num_type_sizes;
   unsigned id;
   VkDescriptorPoolSize sizes[4];
   struct zink_descriptor_layout_key *layout;
};

/* a template used for updating descriptor buffers */
struct zink_descriptor_template {
   uint16_t stride; //the stride between mem pointers
   uint16_t db_size; //the size of the entry in the buffer
   unsigned count; //the number of descriptors
   size_t offset; //the offset of the base host pointer to update from
};

/* ctx->dd; created at context creation */
struct zink_descriptor_data {
   bool bindless_bound;
   bool bindless_init;
   bool has_fbfetch;
   bool push_state_changed[2]; //gfx, compute
   uint8_t state_changed[2]; //gfx, compute
   struct zink_descriptor_layout_key *push_layout_keys[2]; //gfx, compute
   struct zink_descriptor_layout *push_dsl[2]; //gfx, compute
   VkDescriptorUpdateTemplate push_template[2]; //gfx, compute

   struct zink_descriptor_layout *dummy_dsl;

   union {
      struct {
         VkDescriptorPool bindless_pool;
         VkDescriptorSet bindless_set;
      } t;
      struct {
         struct zink_resource *bindless_db;
         uint8_t *bindless_db_map;
         struct pipe_transfer *bindless_db_xfer;
         uint32_t bindless_db_offsets[4];
         unsigned max_db_size;
      } db;
   };

   struct zink_program *pg[2]; //gfx, compute

   VkDescriptorUpdateTemplateEntry push_entries[MESA_SHADER_STAGES]; //gfx+fbfetch
   VkDescriptorUpdateTemplateEntry compute_push_entry;

   /* push descriptor layout size and binding offsets */
   uint32_t db_size[2]; //gfx, compute
   uint32_t db_offset[ZINK_GFX_SHADER_COUNT + 1]; //gfx + fbfetch
   /* compute offset is always 0 */
};

/* pg->dd; created at program creation */
struct zink_program_descriptor_data {
   bool bindless;
   bool fbfetch;
   /* bitmask of ubo0 usage for stages */
   uint8_t push_usage;
   /* bitmask of which sets are used by the program */
   uint8_t binding_usage;
   /* all the pool keys for the program */
   struct zink_descriptor_pool_key *pool_key[ZINK_DESCRIPTOR_BASE_TYPES]; //push set doesn't need one
   /* all the layouts for the program */
   struct zink_descriptor_layout *layouts[ZINK_DESCRIPTOR_NON_BINDLESS_TYPES];
   /* all the templates for the program */
   union {
      VkDescriptorUpdateTemplate templates[ZINK_DESCRIPTOR_NON_BINDLESS_TYPES];
      struct zink_descriptor_template *db_template[ZINK_DESCRIPTOR_NON_BINDLESS_TYPES];
   };
   uint32_t db_size[ZINK_DESCRIPTOR_NON_BINDLESS_TYPES]; //the total size of the layout
   uint32_t *db_offset[ZINK_DESCRIPTOR_NON_BINDLESS_TYPES]; //the offset of each binding in the layout
};

struct zink_descriptor_pool {
   /* the current index of 'sets' */
   unsigned set_idx;
   /* number of sets allocated */
   unsigned sets_alloc;
   VkDescriptorPool pool;
   /* sets are lazily allocated */
   VkDescriptorSet sets[MAX_LAZY_DESCRIPTORS];
};

/* a zink_descriptor_pool_key matches up to this struct */
struct zink_descriptor_pool_multi {
   /* for flagging when overflowed pools must be destroyed instead of reused */
   bool reinit_overflow;
   /* this flips to split usable overflow from in-use overflow */
   unsigned overflow_idx;
   /* zink_descriptor_pool objects that have exceeded MAX_LAZY_DESCRIPTORS sets */
   struct util_dynarray overflowed_pools[2];
   /* the current pool; may be null */
   struct zink_descriptor_pool *pool;
   /* pool key for convenience */
   const struct zink_descriptor_pool_key *pool_key;
};

/* bs->dd; created on batch state creation */
struct zink_batch_descriptor_data {
   /* pools have fbfetch initialized */
   bool has_fbfetch;
   /* are descriptor buffers bound */
   bool db_bound;
   /* real size of 'pools' */
   unsigned pool_size[ZINK_DESCRIPTOR_BASE_TYPES];
   /* this array is sized based on the max zink_descriptor_pool_key::id used by the batch; members may be NULL */
   struct util_dynarray pools[ZINK_DESCRIPTOR_BASE_TYPES];
   struct zink_descriptor_pool_multi push_pool[2]; //gfx, compute
   /* the current program (for descriptor updating) */
   struct zink_program *pg[2]; //gfx, compute
   /* the current pipeline compatibility id (for pipeline compatibility rules) */
   uint32_t compat_id[2]; //gfx, compute
   /* the current set layout */
   VkDescriptorSetLayout dsl[2][ZINK_DESCRIPTOR_BASE_TYPES]; //gfx, compute
   union {
      /* the current set for a given type; used for rebinding if pipeline compat id changes and current set must be rebound */
      VkDescriptorSet sets[2][ZINK_DESCRIPTOR_NON_BINDLESS_TYPES]; //gfx, compute
      uint64_t cur_db_offset[ZINK_DESCRIPTOR_NON_BINDLESS_TYPES]; //gfx, compute; the current offset of a descriptor buffer for rebinds
   };
   /* mask of push descriptor usage */
   unsigned push_usage[2]; //gfx, compute

   struct zink_resource *db; //the descriptor buffer for a given type
   uint8_t *db_map; //the host map for the buffer
   struct pipe_transfer *db_xfer; //the transfer map for the buffer
   uint64_t db_offset; //the "next" offset that will be used when the buffer is updated
};

/** batch types */
/* zink_batch_usage concepts:
 * - batch "usage" is an indicator of when and how a BO was accessed
 * - batch "tracking" is the batch state(s) containing an extra ref for a BO
 *
 * - usage prevents a BO from being mapped while it has pending+conflicting access
 * - usage affects pipeline barrier generation for synchronizing reads and writes
 * - usage MUST be removed before context destruction to avoid crashing during BO
 *   reclaiming in suballocator
 *
 * - tracking prevents a BO from being destroyed early
 * - tracking enables usage to be pruned
 *
 *
 * tracking is added:
 * - any time a BO is used in a "one-off" operation (e.g., blit, index buffer, indirect buffer)
 * - any time a descriptor is unbound
 * - when a buffer is replaced (IFF: resource is bound as a descriptor or usage previously existed)
 *
 * tracking is removed:
 * - in zink_reset_batch_state()
 *
 * usage is added:
 * - any time a BO is used in a "one-off" operation (e.g., blit, index buffer, indirect buffer)
 * - any time a descriptor is bound
 * - any time a descriptor is unbound (IFF: usage previously existed)
 * - for all bound descriptors on the first draw/dispatch after a flush (zink_update_descriptor_refs)
 *
 * usage is removed:
 * - when tracking is removed (IFF: BO usage == tracking, i.e., this is the last batch that a BO was active on)
 */
struct zink_batch_usage {
   uint32_t usage;
    /* this is a monotonic int used to disambiguate internal fences from their tc fence references */
   uint32_t submit_count;
   cnd_t flush;
   mtx_t mtx;
   bool unflushed;
};

struct zink_bo_usage {
   uint32_t submit_count;
   struct zink_batch_usage *u;
};

struct zink_batch_obj_list {
   unsigned max_buffers;
   unsigned num_buffers;
   struct zink_resource_object **objs;
};

struct zink_batch_state {
   struct zink_fence fence;
   struct zink_batch_state *next;

   struct zink_batch_usage usage;
   struct zink_context *ctx;
   VkCommandPool cmdpool;
   VkCommandBuffer cmdbuf;
   VkCommandBuffer reordered_cmdbuf;
   VkCommandPool unsynchronized_cmdpool;
   VkCommandBuffer unsynchronized_cmdbuf;
   VkSemaphore signal_semaphore; //external signal semaphore
   struct util_dynarray signal_semaphores; //external signal semaphores
   struct util_dynarray wait_semaphores; //external wait semaphores
   struct util_dynarray wait_semaphore_stages; //external wait semaphores
   struct util_dynarray fd_wait_semaphores; //dmabuf wait semaphores
   struct util_dynarray fd_wait_semaphore_stages; //dmabuf wait semaphores
   struct util_dynarray fences; //zink_tc_fence refs

   VkSemaphore present;
   struct zink_resource *swapchain;
   struct util_dynarray acquires;
   struct util_dynarray acquire_flags;

   struct {
      struct util_dynarray pipelines;
      struct util_dynarray layouts;
   } dgc;

   VkAccessFlags unordered_write_access;
   VkPipelineStageFlags unordered_write_stages;

   simple_mtx_t exportable_lock;

   struct util_queue_fence flush_completed;

   struct set programs;
   struct set dmabuf_exports;

#define BUFFER_HASHLIST_SIZE 32768
   /* buffer_indices_hashlist[hash(bo)] returns -1 if the bo
    * isn't part of any buffer lists or the index where the bo could be found.
    * Since 1) hash collisions of 2 different bo can happen and 2) we use a
    * single hashlist for the 3 buffer list, this is only a hint.
    * batch_find_resource uses this hint to speed up buffers look up.
    */
   int16_t buffer_indices_hashlist[BUFFER_HASHLIST_SIZE];
   struct zink_batch_obj_list real_objs;
   struct zink_batch_obj_list slab_objs;
   struct zink_batch_obj_list sparse_objs;
   struct zink_resource_object *last_added_obj;
   struct util_dynarray swapchain_obj; //this doesn't have a zink_bo and must be handled differently

   struct util_dynarray unref_resources;
   struct util_dynarray bindless_releases[2];

   struct util_dynarray zombie_samplers;

   struct set active_queries; /* zink_query objects which were active at some point in this batch */
   struct util_dynarray dead_querypools;

   struct util_dynarray freed_sparse_backing_bos;

   struct zink_batch_descriptor_data dd;

   VkDeviceSize resource_size;

   bool is_device_lost;
   bool has_barriers;
   bool has_unsync;
};

static inline struct zink_batch_state *
zink_batch_state(struct zink_fence *fence)
{
   return (struct zink_batch_state *)fence;
}

struct zink_batch {
   struct zink_batch_state *state;

   struct zink_batch_usage *last_batch_usage;
   struct zink_resource *swapchain;

   unsigned work_count;

   simple_mtx_t ref_lock;

   bool has_work;
   bool last_was_compute;
   bool in_rp; //renderpass is currently active
};


/** bo types */
struct bo_export {
   /** File descriptor associated with a handle export. */
   int drm_fd;

   /** GEM handle in drm_fd */
   uint32_t gem_handle;

   struct list_head link;
};

struct zink_bo {
   struct pb_buffer base;

   union {
      struct {
         void *cpu_ptr; /* for user_ptr and permanent maps */
         int map_count;
         struct list_head exports;
         simple_mtx_t export_lock;

         bool is_user_ptr;
         bool use_reusable_pool;

         /* Whether buffer_get_handle or buffer_from_handle has been called,
          * it can only transition from false to true. Protected by lock.
          */
         bool is_shared;
      } real;
      struct {
         struct pb_slab_entry entry;
         struct zink_bo *real;
      } slab;
      struct {
         uint32_t num_va_pages;
         uint32_t num_backing_pages;

         struct list_head backing;

         /* Commitment information for each page of the virtual memory area. */
         struct zink_sparse_commitment *commitments;
      } sparse;
   } u;

   VkDeviceMemory mem;
   uint64_t offset;

   uint32_t unique_id;
   const char *name;

   simple_mtx_t lock;

   struct zink_bo_usage reads;
   struct zink_bo_usage writes;

   struct pb_cache_entry cache_entry[];
};

static inline struct zink_bo *
zink_bo(struct pb_buffer *pbuf)
{
   return (struct zink_bo*)pbuf;
}

/** clear types */
struct zink_framebuffer_clear_data {
   union {
      union pipe_color_union color;
      struct {
         float depth;
         unsigned stencil;
         uint8_t bits : 2; // PIPE_CLEAR_DEPTH, PIPE_CLEAR_STENCIL
      } zs;
   };
   struct pipe_scissor_state scissor;
   bool has_scissor;
   bool conditional;
};

struct zink_framebuffer_clear {
   struct util_dynarray clears;
};


/** compiler types */
struct zink_shader_info {
   uint16_t stride[PIPE_MAX_SO_BUFFERS];
   uint32_t sampler_mask;
   bool have_sparse;
   bool have_vulkan_memory_model;
   bool have_workgroup_memory_explicit_layout;
   struct {
      uint8_t flush_denorms:3; // 16, 32, 64
      uint8_t preserve_denorms:3; // 16, 32, 64
      bool denorms_32_bit_independence:1;
      bool denorms_all_independence:1;
   } float_controls;
   unsigned bindless_set_idx;
};

enum zink_rast_prim {
   ZINK_PRIM_POINTS,
   ZINK_PRIM_LINES,
   ZINK_PRIM_TRIANGLES,
   ZINK_PRIM_MAX,
};

struct zink_shader_object {
   union {
      VkShaderEXT obj;
      VkShaderModule mod;
   };
   struct spirv_shader *spirv;
};

struct zink_shader {
   struct util_live_shader base;
   uint32_t hash;
   struct blob blob;
   struct shader_info info;

   struct zink_shader_info sinfo;

   struct {
      int index;
      int binding;
      VkDescriptorType type;
      unsigned char size;
   } bindings[ZINK_DESCRIPTOR_BASE_TYPES][ZINK_MAX_DESCRIPTORS_PER_TYPE];
   size_t num_bindings[ZINK_DESCRIPTOR_BASE_TYPES];
   unsigned num_texel_buffers;
   uint32_t ubos_used; // bitfield of which ubo indices are used
   uint32_t ssbos_used; // bitfield of which ssbo indices are used
   uint64_t flat_flags;
   bool bindless;
   bool can_inline;
   bool has_uniforms;
   bool has_edgeflags;
   bool needs_inlining;
   struct spirv_shader *spirv;

   struct {
      struct util_queue_fence fence;
      struct zink_shader_object obj;
      VkDescriptorSetLayout dsl;
      VkPipelineLayout layout;
      VkPipeline gpl;
      VkDescriptorSetLayoutBinding *bindings;
      unsigned num_bindings;
      struct zink_descriptor_template *db_template;
      unsigned db_size;
      unsigned *db_offset;
   } precompile;

   simple_mtx_t lock;
   struct set *programs;
   struct util_dynarray pipeline_libs;

   union {
      struct {
         struct zink_shader *generated_tcs; // a generated shader that this shader "owns"; only valid in the tes stage
         struct zink_shader *generated_gs[MESA_PRIM_COUNT][ZINK_PRIM_MAX]; // generated shaders that this shader "owns"
         struct zink_shader *parent; // for a generated gs this points to the shader that "owns" it

         bool is_generated; // if this is a driver-created shader (e.g., tcs)
      } non_fs;

      struct {
         /* Bitmask of textures that have shadow sampling result components
          * other than RED accessed. This is a subset of !is_new_style_shadow
          * (GLSL <1.30, ARB_fp) shadow sampling usage.
          */
         uint32_t legacy_shadow_mask;
         nir_variable *fbfetch; //for fs output
      } fs;
   };
};


/** pipeline types */
struct zink_pipeline_dynamic_state1 {
   uint8_t front_face; //VkFrontFace:1
   uint8_t cull_mode; //VkCullModeFlags:2
   uint16_t num_viewports;
   struct zink_depth_stencil_alpha_hw_state *depth_stencil_alpha_state; //must be last
};

struct zink_pipeline_dynamic_state2 {
   bool primitive_restart;
   bool rasterizer_discard;
   uint16_t vertices_per_patch; //5 bits
};

#define zink_pipeline_dynamic_state3 zink_rasterizer_hw_state

struct zink_gfx_pipeline_state {
   /* order matches zink_gfx_output_key */
   unsigned force_persample_interp:1;
   uint32_t rast_samples:6;
   uint32_t min_samples:6;
   uint32_t feedback_loop : 1;
   uint32_t feedback_loop_zs : 1;
   uint32_t rast_attachment_order : 1;
   uint32_t rp_state : 16;
   VkSampleMask sample_mask;
   uint32_t blend_id;

   /* Pre-hashed value for table lookup, invalid when zero.
    * Members after this point are not included in pipeline state hash key */
   uint32_t hash;
   bool dirty;

   struct zink_pipeline_dynamic_state1 dyn_state1;

   struct zink_pipeline_dynamic_state2 dyn_state2;
   struct zink_pipeline_dynamic_state3 dyn_state3;

   union {
      VkShaderModule modules[MESA_SHADER_STAGES - 1];
      uint32_t optimal_key;
   };
   bool modules_changed;

   uint32_t vertex_hash;

   uint32_t final_hash;

   uint32_t _pad2;
   /* order matches zink_gfx_input_key */
   union {
      struct {
         unsigned idx:8;
         bool uses_dynamic_stride;
      };
      uint32_t input;
   };
   uint32_t vertex_buffers_enabled_mask;
   uint32_t vertex_strides[PIPE_MAX_ATTRIBS];
   struct zink_vertex_elements_hw_state *element_state;
   struct zink_zs_swizzle_key *shadow;
   bool sample_locations_enabled;
   enum mesa_prim shader_rast_prim, rast_prim; /* reduced type or max for unknown */
   union {
      struct {
         struct zink_shader_key key[5];
         struct zink_shader_key last_vertex;
      } shader_keys;
      struct {
         union zink_shader_key_optimal key;
      } shader_keys_optimal;
   };
   struct zink_blend_state *blend_state;
   struct zink_render_pass *render_pass;
   struct zink_render_pass *next_render_pass; //will be used next time rp is begun
   VkFormat rendering_formats[PIPE_MAX_COLOR_BUFS];
   VkPipelineRenderingCreateInfo rendering_info;
   VkPipeline pipeline;
   enum mesa_prim gfx_prim_mode; //pending mode
};

struct zink_compute_pipeline_state {
   /* Pre-hashed value for table lookup, invalid when zero.
    * Members after this point are not included in pipeline state hash key */
   uint32_t hash;
   uint32_t final_hash;
   bool dirty;
   uint32_t local_size[3];
   uint32_t variable_shared_mem;

   uint32_t module_hash;
   VkShaderModule module;
   bool module_changed;

   struct zink_shader_key key;

   VkPipeline pipeline;
};


/** program types */

/* create_gfx_pushconst must be kept in sync with this struct */
struct zink_gfx_push_constant {
   unsigned draw_mode_is_indexed;
   unsigned draw_id;
   unsigned framebuffer_is_layered;
   float default_inner_level[2];
   float default_outer_level[4];
   uint32_t line_stipple_pattern;
   float viewport_scale[2];
   float line_width;
};

/* The order of the enums MUST match the order of the zink_gfx_push_constant
 * members.
 */
enum zink_gfx_push_constant_member {
   ZINK_GFX_PUSHCONST_DRAW_MODE_IS_INDEXED,
   ZINK_GFX_PUSHCONST_DRAW_ID,
   ZINK_GFX_PUSHCONST_FRAMEBUFFER_IS_LAYERED,
   ZINK_GFX_PUSHCONST_DEFAULT_INNER_LEVEL,
   ZINK_GFX_PUSHCONST_DEFAULT_OUTER_LEVEL,
   ZINK_GFX_PUSHCONST_LINE_STIPPLE_PATTERN,
   ZINK_GFX_PUSHCONST_VIEWPORT_SCALE,
   ZINK_GFX_PUSHCONST_LINE_WIDTH,
   ZINK_GFX_PUSHCONST_MAX
};

/* a shader module is used for directly reusing a shader module between programs,
 * e.g., in the case where we're swapping out only one shader,
 * allowing us to skip going through shader keys
 */
struct zink_shader_module {
   struct zink_shader_object obj;
   uint32_t hash;
   bool shobj;
   bool default_variant;
   bool has_nonseamless;
   bool needs_zs_shader_swizzle;
   uint8_t num_uniforms;
   uint8_t key_size;
   uint8_t key[0]; /* | key | uniforms | zs shader swizzle | */
};

struct zink_program {
   struct pipe_reference reference;
   struct zink_context *ctx;
   unsigned char sha1[20];
   struct util_queue_fence cache_fence;
   struct u_rwlock pipeline_cache_lock;
   VkPipelineCache pipeline_cache;
   size_t pipeline_cache_size;
   struct zink_batch_usage *batch_uses;
   bool is_compute;
   bool can_precompile;
   bool uses_shobj; //whether shader objects are used; programs CANNOT mix shader objects and shader modules

   struct zink_program_descriptor_data dd;

   uint32_t compat_id;
   VkPipelineLayout layout;
   VkDescriptorSetLayout dsl[ZINK_DESCRIPTOR_ALL_TYPES]; // one for each type + push + bindless
   unsigned num_dsl;

   bool removed;
};

#define STAGE_MASK_OPTIMAL (1<<16)
#define STAGE_MASK_OPTIMAL_SHADOW (1<<17)
typedef bool (*equals_gfx_pipeline_state_func)(const void *a, const void *b);

struct zink_gfx_library_key {
   uint32_t optimal_key; //equals_pipeline_lib_optimal
   VkShaderModule modules[ZINK_GFX_SHADER_COUNT];
   VkPipeline pipeline;
};

struct zink_gfx_input_key {
   union {
      struct {
         unsigned idx:8;
         bool uses_dynamic_stride;
      };
      uint32_t input;
   };
   uint32_t vertex_buffers_enabled_mask;
   uint32_t vertex_strides[PIPE_MAX_ATTRIBS];
   struct zink_vertex_elements_hw_state *element_state;
   VkPipeline pipeline;
};

struct zink_gfx_output_key {
   /* order matches zink_gfx_output_key */
   union {
      struct {
         unsigned force_persample_interp:1;
         uint32_t rast_samples:6;
         uint32_t min_samples:6;
         uint32_t feedback_loop : 1;
         uint32_t feedback_loop_zs : 1;
         uint32_t rast_attachment_order : 1;
         uint32_t rp_state : 16;
      };
      uint32_t key;
   };

   /* TODO: compress these */
   VkSampleMask sample_mask;
   uint32_t blend_id;
   VkPipeline pipeline;
};

struct zink_gfx_pipeline_cache_entry {
   struct zink_gfx_pipeline_state state;
   VkPipeline pipeline;
   struct zink_gfx_program *prog;
   /* GPL only */
   struct util_queue_fence fence;
   union {
      struct {
         struct zink_gfx_input_key *ikey;
         struct zink_gfx_library_key *gkey;
         struct zink_gfx_output_key *okey;
         VkPipeline unoptimized_pipeline;
      } gpl;
      struct zink_shader_object shobjs[ZINK_GFX_SHADER_COUNT];
   };
};

struct zink_gfx_lib_cache {
   /* for hashing */
   struct zink_shader *shaders[ZINK_GFX_SHADER_COUNT];
   unsigned refcount;
   bool removed; //once removed from cache
   uint8_t stages_present;

   simple_mtx_t lock;
   struct set libs; //zink_gfx_library_key -> VkPipeline
};

struct zink_gfx_program {
   struct zink_program base;

   bool is_separable; //not a full program
   struct zink_context *ctx; //the owner context

   uint32_t stages_present; //mask of stages present in this program
   uint32_t stages_remaining; //mask of zink_shader remaining in this program
   uint32_t gfx_hash; //from ctx->gfx_hash

   struct zink_shader *shaders[ZINK_GFX_SHADER_COUNT];
   struct zink_shader *last_vertex_stage;
   struct zink_shader_object objs[ZINK_GFX_SHADER_COUNT];

   /* full */
   VkShaderEXT objects[ZINK_GFX_SHADER_COUNT];
   uint32_t module_hash[ZINK_GFX_SHADER_COUNT];
   struct blob blobs[ZINK_GFX_SHADER_COUNT];
   struct util_dynarray shader_cache[ZINK_GFX_SHADER_COUNT][2][2]; //normal, nonseamless cubes, inline uniforms
   unsigned inlined_variant_count[ZINK_GFX_SHADER_COUNT];
   uint32_t default_variant_hash;
   uint8_t inline_variants; //which stages are using inlined uniforms
   bool needs_inlining; // whether this program requires some uniforms to be inlined
   bool has_edgeflags;
   bool optimal_keys;

   /* separable */
   struct zink_gfx_program *full_prog;

   struct hash_table pipelines[2][11]; // [dynamic, renderpass][number of draw modes we support]
   uint32_t last_variant_hash;

   uint32_t last_finalized_hash[2][4]; //[dynamic, renderpass][primtype idx]
   VkPipeline last_pipeline[2][4]; //[dynamic, renderpass][primtype idx]

   struct zink_gfx_lib_cache *libs;
};

struct zink_compute_program {
   struct zink_program base;

   bool use_local_size;
   bool has_variable_shared_mem;

   unsigned scratch_size;

   unsigned num_inlinable_uniforms;
   nir_shader *nir; //only until precompile finishes

   struct zink_shader_module *curr;

   struct zink_shader_module *module; //base
   struct util_dynarray shader_cache[2]; //nonseamless cubes, inline uniforms
   unsigned inlined_variant_count;

   struct zink_shader *shader;
   struct hash_table pipelines;

   simple_mtx_t cache_lock; //extra lock because threads are insane and sand was not meant to think

   VkPipeline base_pipeline;
};


/** renderpass types */

struct zink_rt_attrib {
  VkFormat format;
  VkSampleCountFlagBits samples;
  bool clear_color;
  union {
     bool clear_stencil;
     bool fbfetch;
  };
  bool invalid;
  bool needs_write;
  bool resolve;
  bool feedback_loop;
};

struct zink_render_pass_state {
   union {
      struct {
         uint8_t num_cbufs : 5; /* PIPE_MAX_COLOR_BUFS = 8 */
         uint8_t have_zsbuf : 1;
         uint8_t samples:1; //for fs samplemask
         uint32_t num_zsresolves : 1;
         uint32_t num_cresolves : 24; /* PIPE_MAX_COLOR_BUFS, but this is a struct hole */
      };
      uint32_t val; //for comparison
   };
   struct zink_rt_attrib rts[PIPE_MAX_COLOR_BUFS + 1];
   unsigned num_rts;
   uint32_t clears; //for extra verification and update flagging
   uint16_t msaa_expand_mask;
   uint16_t msaa_samples; //used with VK_EXT_multisampled_render_to_single_sampled
};

struct zink_pipeline_rt {
   VkFormat format;
   VkSampleCountFlagBits samples;
};

struct zink_render_pass_pipeline_state {
   uint32_t num_attachments:14;
   uint32_t msaa_samples : 8;
   uint32_t fbfetch:1;
   uint32_t color_read:1;
   uint32_t depth_read:1;
   uint32_t depth_write:1;
   uint32_t num_cresolves:4;
   uint32_t num_zsresolves:1;
   bool samples:1; //for fs samplemask
   struct zink_pipeline_rt attachments[PIPE_MAX_COLOR_BUFS + 1];
   unsigned id;
};

struct zink_render_pass {
   VkRenderPass render_pass;
   struct zink_render_pass_state state;
   unsigned pipeline_state;
};


/** resource types */
struct zink_resource_object {
   struct pipe_reference reference;

   VkPipelineStageFlags access_stage;
   VkAccessFlags access;
   VkPipelineStageFlags unordered_access_stage;
   VkAccessFlags unordered_access;
   VkAccessFlags last_write;

   /* 'access' is propagated from unordered_access to handle ops occurring
    * in the ordered cmdbuf which can promote barriers to unordered
    */
   bool ordered_access_is_copied;
   bool unordered_read;
   bool unordered_write;
   bool unsync_access;
   bool copies_valid;
   bool copies_need_reset; //for use with batch state resets

   struct u_rwlock copy_lock;
   struct util_dynarray copies[16]; //regions being copied to; for barrier omission

   VkBuffer storage_buffer;
   simple_mtx_t view_lock;
   uint32_t view_prune_count; //how many views to prune
   uint32_t view_prune_timeline; //when to prune
   struct util_dynarray views;

   union {
      VkBuffer buffer;
      VkImage image;
   };
   VkDeviceAddress bda;

   VkSampleLocationsInfoEXT zs_evaluate;
   bool needs_zs_evaluate;

   bool storage_init; //layout was set for image
   bool transfer_dst;
   bool render_target;
   bool is_buffer;
   bool exportable;

   /* TODO: this should be a union */
   int handle;
   struct zink_bo *bo;
   // struct {
   struct kopper_displaytarget *dt;
   uint32_t dt_idx;
   uint32_t last_dt_idx;
   VkSemaphore present;
   bool new_dt;
   bool indefinite_acquire;
   // }


   VkDeviceSize offset, size, alignment;
   uint64_t vkflags;
   uint64_t vkusage;
   VkFormatFeatureFlags vkfeats;
   uint64_t modifier;
   VkImageAspectFlags modifier_aspect;
   VkSamplerYcbcrConversion sampler_conversion;
   unsigned plane_offsets[3];
   unsigned plane_strides[3];
   unsigned plane_count;

   bool host_visible;
   bool coherent;
   bool is_aux;
};

struct zink_resource {
   struct threaded_resource base;

   enum pipe_format internal_format:16;

   struct zink_resource_object *obj;
   uint32_t queue;
   union {
      struct {
         struct util_range valid_buffer_range;
         uint32_t vbo_bind_mask : PIPE_MAX_ATTRIBS;
         uint8_t ubo_bind_count[2];
         uint8_t ssbo_bind_count[2];
         uint8_t vbo_bind_count;
         uint8_t so_bind_count; //not counted in all_binds
         bool so_valid;
         uint32_t ubo_bind_mask[MESA_SHADER_STAGES];
         uint32_t ssbo_bind_mask[MESA_SHADER_STAGES];
      };
      struct {
         bool linear;
         bool need_2D;
         bool valid;
         uint8_t fb_bind_count; //not counted in all_binds
         uint16_t fb_binds; /* mask of attachment idx; zs is PIPE_MAX_COLOR_BUFS */
         VkSparseImageMemoryRequirements sparse;
         VkFormat format;
         VkImageLayout layout;
         VkImageAspectFlags aspect;
      };
   };
   uint32_t sampler_binds[MESA_SHADER_STAGES];
   uint32_t image_binds[MESA_SHADER_STAGES];
   uint16_t sampler_bind_count[2]; //gfx, compute
   uint16_t image_bind_count[2]; //gfx, compute
   uint16_t write_bind_count[2]; //gfx, compute
   union {
      uint16_t bindless[2]; //tex, img
      uint32_t all_bindless;
   };
   union {
      uint16_t bind_count[2]; //gfx, compute
      uint32_t all_binds;
   };

   VkPipelineStageFlagBits gfx_barrier;
   VkAccessFlagBits barrier_access[2]; //gfx, compute

   union {
      struct {
         struct hash_table bufferview_cache;
         simple_mtx_t bufferview_mtx;
      };
      struct {
         struct hash_table surface_cache;
         simple_mtx_t surface_mtx;
      };
   };

   bool copies_warned;
   bool swapchain;
   bool dmabuf;
   unsigned dt_stride;

   uint8_t modifiers_count;
   uint64_t *modifiers;
};

static inline struct zink_resource *
zink_resource(struct pipe_resource *r)
{
   return (struct zink_resource *)r;
}


struct zink_transfer {
   struct threaded_transfer base;
   struct pipe_resource *staging_res;
   unsigned offset;
   unsigned depthPitch;
};


/** screen types */
struct zink_modifier_prop {
    uint32_t                             drmFormatModifierCount;
    VkDrmFormatModifierPropertiesEXT*    pDrmFormatModifierProperties;
};

struct zink_format_props {
   VkFormatFeatureFlags2 linearTilingFeatures;
   VkFormatFeatureFlags2 optimalTilingFeatures;
   VkFormatFeatureFlags2 bufferFeatures;
};

struct zink_screen {
   struct pipe_screen base;

   struct util_dl_library *loader_lib;
   PFN_vkGetInstanceProcAddr vk_GetInstanceProcAddr;
   PFN_vkGetDeviceProcAddr vk_GetDeviceProcAddr;

   bool threaded;
   bool threaded_submit;
   bool is_cpu;
   bool abort_on_hang;
   bool frame_marker_emitted;
   uint64_t curr_batch; //the current batch id
   uint32_t last_finished;
   VkSemaphore sem;
   VkFence fence;
   struct util_queue flush_queue;
   simple_mtx_t copy_context_lock;
   struct zink_context *copy_context;

   struct zink_batch_state *free_batch_states; //unused batch states
   struct zink_batch_state *last_free_batch_state; //for appending
   simple_mtx_t free_batch_states_lock;

   simple_mtx_t semaphores_lock;
   struct util_dynarray semaphores;
   struct util_dynarray fd_semaphores;

   unsigned buffer_rebind_counter;
   unsigned image_rebind_counter;
   unsigned robust_ctx_count;

   struct hash_table dts;
   simple_mtx_t dt_lock;

   bool device_lost;
   int drm_fd;

   struct slab_parent_pool transfer_pool;
   struct disk_cache *disk_cache;
   struct util_queue cache_put_thread;
   struct util_queue cache_get_thread;

   /* there are 5 gfx stages, but VS and FS are assumed to be always present,
    * thus only 3 stages need to be considered, giving 2^3 = 8 program caches.
    */
   struct set pipeline_libs[8];
   simple_mtx_t pipeline_libs_lock[8];

   simple_mtx_t desc_set_layouts_lock;
   struct hash_table desc_set_layouts[ZINK_DESCRIPTOR_BASE_TYPES];
   simple_mtx_t desc_pool_keys_lock;
   struct set desc_pool_keys[ZINK_DESCRIPTOR_BASE_TYPES];
   struct util_live_shader_cache shaders;

   uint64_t db_size[ZINK_DESCRIPTOR_ALL_TYPES];
   unsigned base_descriptor_size;
   VkDescriptorSetLayout bindless_layout;

   struct {
      struct pb_cache bo_cache;
      struct pb_slabs bo_slabs[NUM_SLAB_ALLOCATORS];
      unsigned min_alloc_size;
      uint32_t next_bo_unique_id;
   } pb;
   uint8_t heap_map[ZINK_HEAP_MAX][VK_MAX_MEMORY_TYPES];  // mapping from zink heaps to memory type indices
   uint8_t heap_count[ZINK_HEAP_MAX];  // number of memory types per zink heap
   bool resizable_bar;

   uint64_t total_video_mem;
   uint64_t clamp_video_mem;
   uint64_t total_mem;
   uint64_t mapped_vram;

   VkInstance instance;
   struct zink_instance_info instance_info;

   struct hash_table *debug_mem_sizes;
   simple_mtx_t debug_mem_lock;

   VkPhysicalDevice pdev;
   uint32_t vk_version, spirv_version;
   struct util_idalloc_mt buffer_ids;
   struct util_vertex_state_cache vertex_state_cache;

   struct zink_device_info info;
   struct nir_shader_compiler_options nir_options;

   bool optimal_keys;
   bool have_full_ds3;
   bool have_X8_D24_UNORM_PACK32;
   bool have_D24_UNORM_S8_UINT;
   bool have_D32_SFLOAT_S8_UINT;
   bool have_triangle_fans;
   bool need_decompose_attrs;
   bool need_2D_zs;
   bool need_2D_sparse;
   bool faked_e5sparse; //drivers may not expose R9G9B9E5 but cts requires it
   bool can_hic_shader_read;

   uint32_t gfx_queue;
   uint32_t sparse_queue;
   uint32_t max_queues;
   uint32_t timestamp_valid_bits;
   VkDevice dev;
   VkQueue queue; //gfx+compute
   VkQueue queue_sparse;
   simple_mtx_t queue_lock;
   VkDebugUtilsMessengerEXT debugUtilsCallbackHandle;

   uint32_t cur_custom_border_color_samplers;

   unsigned screen_id;

#ifdef HAVE_RENDERDOC_APP_H
   RENDERDOC_API_1_0_0 *renderdoc_api;
   unsigned renderdoc_capture_start;
   unsigned renderdoc_capture_end;
   unsigned renderdoc_frame;
   bool renderdoc_capturing;
   bool renderdoc_capture_all;
#endif

   struct vk_uncompacted_dispatch_table vk;

   void (*buffer_barrier)(struct zink_context *ctx, struct zink_resource *res, VkAccessFlags flags, VkPipelineStageFlags pipeline);
   void (*image_barrier)(struct zink_context *ctx, struct zink_resource *res, VkImageLayout new_layout, VkAccessFlags flags, VkPipelineStageFlags pipeline);
   void (*image_barrier_unsync)(struct zink_context *ctx, struct zink_resource *res, VkImageLayout new_layout, VkAccessFlags flags, VkPipelineStageFlags pipeline);

   bool compact_descriptors; /**< toggled if descriptor set ids are compacted */
   uint8_t desc_set_id[ZINK_MAX_DESCRIPTOR_SETS]; /**< converts enum zink_descriptor_type -> the actual set id */

   struct {
      bool dual_color_blend_by_location;
      bool glsl_correct_derivatives_after_discard;
      bool inline_uniforms;
      bool emulate_point_smooth;
      bool zink_shader_object_enable;
   } driconf;

   struct zink_format_props format_props[PIPE_FORMAT_COUNT];
   struct zink_modifier_prop modifier_props[PIPE_FORMAT_COUNT];

   VkExtent2D maxSampleLocationGridSize[5];
   VkPipelineLayout gfx_push_constant_layout;

   struct {
      bool broken_l4a4;
      /* https://gitlab.khronos.org/vulkan/vulkan/-/issues/3306
       * HI TURNIP
       */
      bool broken_cache_semantics;
      bool missing_a8_unorm;
      bool implicit_sync;
      bool disable_optimized_compile;
      bool always_feedback_loop;
      bool always_feedback_loop_zs;
      bool needs_sanitised_layer;
      bool track_renderpasses;
      bool no_linestipple;
      bool no_linesmooth;
      bool no_hw_gl_point;
      bool lower_robustImageAccess2;
      bool needs_zs_shader_swizzle;
      bool can_do_invalid_linear_modifier;
      unsigned z16_unscaled_bias;
      unsigned z24_unscaled_bias;
   } driver_workarounds;
};

static inline struct zink_screen *
zink_screen(struct pipe_screen *pipe)
{
   return (struct zink_screen *)pipe;
}

/** surface types */

/* info for validating/creating imageless framebuffers */
struct zink_surface_info {
   VkImageCreateFlags flags;
   VkImageUsageFlags usage;
   uint32_t width;
   uint32_t height;
   uint32_t layerCount;
   VkFormat format[2]; //base format, srgb format (for srgb framebuffer)
};

/* an imageview for a zink_resource:
   - may be a fb attachment, samplerview, or shader image
   - cached on the parent zink_resource_object
   - also handles swapchains
 */
struct zink_surface {
   struct pipe_surface base;
   /* all the info for creating a new imageview */
   VkImageViewCreateInfo ivci;
   VkImageViewUsageCreateInfo usage_info;
   /* for framebuffer use */
   struct zink_surface_info info;
   bool is_swapchain;
   /* the current imageview */
   VkImageView image_view;
   /* array of imageviews for swapchains, one for each image */
   VkImageView *swapchain;
   unsigned swapchain_size;
   void *obj; //backing resource object; used to determine rebinds
   void *dt_swapchain; //current swapchain object; used to determine swapchain rebinds
   uint32_t hash; //for surface caching
};

/* wrapper object that preserves the gallium expectation of having
 * pipe_surface::context match the context used to create the surface
 */
struct zink_ctx_surface {
   struct pipe_surface base;
   struct zink_surface *surf; //the actual surface
   struct zink_ctx_surface *transient; //for use with EXT_multisample_render_to_texture
   bool transient_init; //whether the transient surface has data
   bool needs_mutable;
};

/* use this cast for framebuffer surfaces */
static inline struct zink_surface *
zink_csurface(struct pipe_surface *psurface)
{
   return psurface ? ((struct zink_ctx_surface *)psurface)->surf : NULL;
}

/* use this cast for checking transient framebuffer surfaces */
static inline struct zink_surface *
zink_transient_surface(struct pipe_surface *psurface)
{
   return psurface ? ((struct zink_ctx_surface *)psurface)->transient ? ((struct zink_ctx_surface *)psurface)->transient->surf : NULL : NULL;
}

/* use this cast for internal surfaces */
static inline struct zink_surface *
zink_surface(struct pipe_surface *psurface)
{
   return (struct zink_surface *)psurface;
}


/** framebuffer types */
struct zink_framebuffer_state {
   uint32_t width;
   uint16_t height;
   uint32_t layers:6;
   uint32_t samples:6;
   uint32_t num_attachments:4;
   struct zink_surface_info infos[PIPE_MAX_COLOR_BUFS + 1];
};

struct zink_framebuffer {
   struct pipe_reference reference;

   /* current objects */
   VkFramebuffer fb;
   struct zink_render_pass *rp;

   struct zink_framebuffer_state state;
   VkFramebufferAttachmentImageInfo infos[PIPE_MAX_COLOR_BUFS + 1];
   struct hash_table objects;
};


/** context types */
struct zink_sampler_state {
   VkSampler sampler;
   VkSampler sampler_clamped;
   bool custom_border_color;
   bool emulate_nonseamless;
};

struct zink_buffer_view {
   struct pipe_reference reference;
   struct pipe_resource *pres;
   VkBufferViewCreateInfo bvci;
   VkBufferView buffer_view;
   uint32_t hash;
};

struct zink_sampler_view {
   struct pipe_sampler_view base;
   union {
      struct zink_surface *image_view;
      struct zink_buffer_view *buffer_view;
      unsigned tbo_size;
   };
   struct zink_surface *cube_array;
   /* Optional sampler view returning red (depth) in all channels, for shader rewrites. */
   struct zink_surface *zs_view;
   struct zink_zs_swizzle swizzle;
};

struct zink_image_view {
   struct pipe_image_view base;
   union {
      struct zink_surface *surface;
      struct zink_buffer_view *buffer_view;
   };
};

static inline struct zink_sampler_view *
zink_sampler_view(struct pipe_sampler_view *pview)
{
   return (struct zink_sampler_view *)pview;
}

struct zink_so_target {
   struct pipe_stream_output_target base;
   struct pipe_resource *counter_buffer;
   VkDeviceSize counter_buffer_offset;
   uint32_t stride;
   bool counter_buffer_valid;
};

static inline struct zink_so_target *
zink_so_target(struct pipe_stream_output_target *so_target)
{
   return (struct zink_so_target *)so_target;
}

struct zink_viewport_state {
   struct pipe_viewport_state viewport_states[PIPE_MAX_VIEWPORTS];
   struct pipe_scissor_state scissor_states[PIPE_MAX_VIEWPORTS];
   uint8_t num_viewports;
};

struct zink_descriptor_db_info {
   unsigned offset;
   unsigned size;
   enum pipe_format format;
   struct pipe_resource *pres;
};

struct zink_descriptor_surface {
   union {
      struct zink_surface *surface;
      struct zink_buffer_view *bufferview;
      struct zink_descriptor_db_info db;
   };
   bool is_buffer;
};

struct zink_bindless_descriptor {
   struct zink_descriptor_surface ds;
   struct zink_sampler_state *sampler;
   uint32_t handle;
   uint32_t access; //PIPE_ACCESS_...
};

struct zink_rendering_info {
   VkPipelineRenderingCreateInfo info;
   unsigned id;
};


typedef void (*pipe_draw_vertex_state_func)(struct pipe_context *ctx,
                                            struct pipe_vertex_state *vstate,
                                            uint32_t partial_velem_mask,
                                            struct pipe_draw_vertex_state_info info,
                                            const struct pipe_draw_start_count_bias *draws,
                                            unsigned num_draws);
typedef void (*pipe_launch_grid_func)(struct pipe_context *pipe, const struct pipe_grid_info *info);


enum zink_ds3_state {
   ZINK_DS3_RAST_STIPPLE,
   ZINK_DS3_RAST_CLIP,
   ZINK_DS3_RAST_CLAMP,
   ZINK_DS3_RAST_POLYGON,
   ZINK_DS3_RAST_HALFZ,
   ZINK_DS3_RAST_PV,
   ZINK_DS3_RAST_LINE,
   ZINK_DS3_RAST_STIPPLE_ON,
   ZINK_DS3_BLEND_A2C,
   ZINK_DS3_BLEND_A21,
   ZINK_DS3_BLEND_ON,
   ZINK_DS3_BLEND_WRITE,
   ZINK_DS3_BLEND_EQ,
   ZINK_DS3_BLEND_LOGIC_ON,
   ZINK_DS3_BLEND_LOGIC,
};

struct zink_context {
   struct pipe_context base;
   struct threaded_context *tc;
   struct slab_child_pool transfer_pool;
   struct slab_child_pool transfer_pool_unsync;
   struct blitter_context *blitter;
   struct util_debug_callback dbg;

   unsigned flags;

   pipe_draw_func draw_vbo[2]; //batch changed
   pipe_draw_vertex_state_func draw_state[2]; //batch changed
   pipe_launch_grid_func launch_grid[2]; //batch changed

   struct pipe_device_reset_callback reset;

   struct util_queue_fence unsync_fence; //unsigned during unsync recording (blocks flush ops)
   struct util_queue_fence flush_fence; //unsigned during flush (blocks unsync ops)

   struct zink_fence *deferred_fence;
   struct zink_fence *last_fence; //the last command buffer submitted
   struct zink_batch_state *batch_states; //list of submitted batch states: ordered by increasing timeline id
   unsigned batch_states_count; //number of states in `batch_states`
   struct zink_batch_state *free_batch_states; //unused batch states
   struct zink_batch_state *last_free_batch_state; //for appending
   bool oom_flush;
   bool oom_stall;
   bool track_renderpasses;
   struct zink_batch batch;

   unsigned shader_has_inlinable_uniforms_mask;
   unsigned inlinable_uniforms_valid_mask;

   struct pipe_constant_buffer ubos[MESA_SHADER_STAGES][PIPE_MAX_CONSTANT_BUFFERS];
   struct pipe_shader_buffer ssbos[MESA_SHADER_STAGES][PIPE_MAX_SHADER_BUFFERS];
   uint32_t writable_ssbos[MESA_SHADER_STAGES];
   struct zink_image_view image_views[MESA_SHADER_STAGES][ZINK_MAX_SHADER_IMAGES];

   uint32_t transient_attachments;
   struct pipe_framebuffer_state fb_state;
   struct hash_table framebuffer_cache;

   struct zink_vertex_elements_state *element_state;
   struct zink_rasterizer_state *rast_state;
   struct zink_depth_stencil_alpha_state *dsa_state;

   bool pipeline_changed[2]; //gfx, compute

   struct zink_shader *gfx_stages[ZINK_GFX_SHADER_COUNT];
   struct zink_shader *last_vertex_stage;
   bool shader_reads_drawid;
   bool shader_reads_basevertex;
   struct zink_gfx_pipeline_state gfx_pipeline_state;
   /* there are 5 gfx stages, but VS and FS are assumed to be always present,
    * thus only 3 stages need to be considered, giving 2^3 = 8 program caches.
    */
   struct hash_table program_cache[8];
   simple_mtx_t program_lock[8];
   uint32_t gfx_hash;
   struct zink_gfx_program *curr_program;
   struct set gfx_inputs;
   struct set gfx_outputs;

   struct zink_descriptor_data dd;

   struct zink_compute_pipeline_state compute_pipeline_state;
   struct zink_compute_program *curr_compute;

   unsigned shader_stages : ZINK_GFX_SHADER_COUNT; /* mask of bound gfx shader stages */
   uint8_t dirty_gfx_stages; /* mask of changed gfx shader stages */
   bool last_vertex_stage_dirty;
   bool compute_dirty;
   bool is_generated_gs_bound;

   struct {
      VkRenderingAttachmentInfo attachments[PIPE_MAX_COLOR_BUFS + 2]; //+depth, +stencil
      VkRenderingInfo info;
      struct tc_renderpass_info tc_info;
   } dynamic_fb;
   uint32_t fb_layer_mismatch; //bitmask
   unsigned depth_bias_scale_factor;
   struct set rendering_state_cache[6]; //[util_logbase2_ceil(msrtss samplecount)]
   struct set render_pass_state_cache;
   struct hash_table *render_pass_cache;
   VkExtent2D swapchain_size;
   bool fb_changed;
   bool rp_changed; //force renderpass restart
   bool rp_layout_changed; //renderpass changed, maybe restart
   bool rp_loadop_changed; //renderpass changed, don't restart
   bool zsbuf_unused;
   bool zsbuf_readonly;

   struct zink_framebuffer *framebuffer;
   struct zink_framebuffer_clear fb_clears[PIPE_MAX_COLOR_BUFS + 1];
   uint16_t clears_enabled;
   uint16_t rp_clears_enabled;
   uint16_t void_clears;
   uint16_t fbfetch_outputs;
   uint16_t feedback_loops;
   struct zink_resource *needs_present;

   struct pipe_vertex_buffer vertex_buffers[PIPE_MAX_ATTRIBS];
   bool vertex_buffers_dirty;

   struct zink_sampler_state *sampler_states[MESA_SHADER_STAGES][PIPE_MAX_SAMPLERS];
   struct pipe_sampler_view *sampler_views[MESA_SHADER_STAGES][PIPE_MAX_SAMPLERS];

   struct zink_viewport_state vp_state;
   bool vp_state_changed;
   bool scissor_changed;

   float blend_constants[4];

   bool sample_locations_changed;
   VkSampleLocationEXT vk_sample_locations[PIPE_MAX_SAMPLE_LOCATION_GRID_SIZE * PIPE_MAX_SAMPLE_LOCATION_GRID_SIZE];
   uint8_t sample_locations[2 * 4 * 8 * 16];

   struct pipe_stencil_ref stencil_ref;

   union {
      struct {
         float default_inner_level[2];
         float default_outer_level[4];
      };
      float tess_levels[6];
   };

   struct zink_vk_query *curr_xfb_queries[PIPE_MAX_VERTEX_STREAMS];
   struct zink_shader *null_fs;
   struct zink_shader *saved_fs;

   struct list_head query_pools;
   struct list_head suspended_queries;
   struct list_head primitives_generated_queries;
   struct zink_query *vertices_query;
   bool disable_fs;
   bool disable_color_writes;
   bool was_line_loop;
   bool fs_query_active;
   bool occlusion_query_active;
   bool primitives_generated_active;
   bool primitives_generated_suspended;
   bool queries_disabled, render_condition_active;
   bool queries_in_rp;
   struct {
      struct zink_query *query;
      bool inverted;
      bool active; //this is the internal vk state
   } render_condition;

   struct {
      bool valid;
      struct u_upload_mgr *upload[ZINK_DGC_MAX];
      struct zink_resource *buffers[ZINK_DGC_MAX];
      struct zink_gfx_program *last_prog;
      uint8_t *maps[ZINK_DGC_MAX];
      size_t bind_offsets[ZINK_DGC_MAX];
      size_t cur_offsets[ZINK_DGC_MAX];
      size_t max_size[ZINK_DGC_MAX];
      struct util_dynarray pipelines;
      struct util_dynarray tokens;
   } dgc;

   struct pipe_resource *dummy_vertex_buffer;
   struct pipe_resource *dummy_xfb_buffer;
   struct pipe_surface *dummy_surface[7];
   struct zink_buffer_view *dummy_bufferview;

   unsigned buffer_rebind_counter;
   unsigned image_rebind_counter;

   struct {
      /* descriptor info */
      uint32_t push_valid;
      uint8_t num_ubos[MESA_SHADER_STAGES];

      uint8_t num_ssbos[MESA_SHADER_STAGES];
      struct util_dynarray global_bindings;

      VkDescriptorImageInfo textures[MESA_SHADER_STAGES][PIPE_MAX_SAMPLERS];
      uint32_t emulate_nonseamless[MESA_SHADER_STAGES];
      uint32_t cubes[MESA_SHADER_STAGES];
      uint8_t num_samplers[MESA_SHADER_STAGES];
      uint8_t num_sampler_views[MESA_SHADER_STAGES];

      VkDescriptorImageInfo images[MESA_SHADER_STAGES][ZINK_MAX_SHADER_IMAGES];
      uint8_t num_images[MESA_SHADER_STAGES];

      union {
         struct {
            VkDescriptorBufferInfo ubos[MESA_SHADER_STAGES][PIPE_MAX_CONSTANT_BUFFERS];
            VkDescriptorBufferInfo ssbos[MESA_SHADER_STAGES][PIPE_MAX_SHADER_BUFFERS];
            VkBufferView tbos[MESA_SHADER_STAGES][PIPE_MAX_SAMPLERS];
            VkBufferView texel_images[MESA_SHADER_STAGES][ZINK_MAX_SHADER_IMAGES];
         } t;
         struct {
            VkDescriptorAddressInfoEXT ubos[MESA_SHADER_STAGES][PIPE_MAX_CONSTANT_BUFFERS];
            VkDescriptorAddressInfoEXT ssbos[MESA_SHADER_STAGES][PIPE_MAX_SHADER_BUFFERS];
            VkDescriptorAddressInfoEXT tbos[MESA_SHADER_STAGES][PIPE_MAX_SAMPLERS];
            VkDescriptorAddressInfoEXT texel_images[MESA_SHADER_STAGES][ZINK_MAX_SHADER_IMAGES];
         } db;
      };

      VkDescriptorImageInfo fbfetch;
      uint8_t fbfetch_db[ZINK_FBFETCH_DESCRIPTOR_SIZE];

      /* the current state of the zs swizzle data */
      struct zink_zs_swizzle_key zs_swizzle[MESA_SHADER_STAGES];

      struct zink_resource *descriptor_res[ZINK_DESCRIPTOR_BASE_TYPES][MESA_SHADER_STAGES][PIPE_MAX_SAMPLERS];

      struct {
         struct util_idalloc tex_slots; //img, buffer
         struct util_idalloc img_slots; //img, buffer
         struct hash_table tex_handles; //img, buffer
         struct hash_table img_handles; //img, buffer
         union {
            struct {
               VkBufferView *buffer_infos; //tex, img
            } t;
            struct {
               VkDescriptorAddressInfoEXT *buffer_infos;
            } db;
         };
         VkDescriptorImageInfo *img_infos; //tex, img
         struct util_dynarray updates; //texture, img
         struct util_dynarray resident; //texture, img
      } bindless[2];
      union {
         bool bindless_dirty[2]; //tex, img
         uint16_t any_bindless_dirty;
      };
      bool bindless_refs_dirty;
   } di;
   void (*invalidate_descriptor_state)(struct zink_context *ctx, gl_shader_stage shader, enum zink_descriptor_type type, unsigned, unsigned);
   struct set *need_barriers[2]; //gfx, compute
   struct set update_barriers[2][2]; //[gfx, compute][current, next]
   uint8_t barrier_set_idx[2];
   unsigned memory_barrier;

   uint32_t ds3_states;

   uint32_t num_so_targets;
   struct pipe_stream_output_target *so_targets[PIPE_MAX_SO_BUFFERS];
   bool dirty_so_targets;

   bool gfx_dirty;

   bool shobj_draw : 1; //using shader objects for draw
   bool is_device_lost;
   bool primitive_restart;
   bool blitting : 1;
   bool unordered_blitting : 1;
   bool vertex_state_changed : 1;
   bool blend_state_changed : 1;
   bool blend_color_changed : 1;
   bool sample_mask_changed : 1;
   bool rast_state_changed : 1;
   bool line_width_changed : 1;
   bool dsa_state_changed : 1;
   bool stencil_ref_changed : 1;
   bool rasterizer_discard_changed : 1;
   bool rp_tc_info_updated : 1;
};

static inline struct zink_context *
zink_context(struct pipe_context *context)
{
   return (struct zink_context *)context;
}

#endif
