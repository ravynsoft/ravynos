/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 * Copyright 2010 Marek Olšák <maraeo@gmail.com>
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef RADEON_WINSYS_H
#define RADEON_WINSYS_H

/* The public winsys interface header for the radeon driver. */

/* Skip command submission. Same as RADEON_NOOP=1. */
#define RADEON_FLUSH_NOOP                     (1u << 29)

/* Toggle the secure submission boolean after the flush */
#define RADEON_FLUSH_TOGGLE_SECURE_SUBMISSION (1u << 30)

/* Whether the next IB can start immediately and not wait for draws and
 * dispatches from the current IB to finish. */
#define RADEON_FLUSH_START_NEXT_GFX_IB_NOW    (1u << 31)

#define RADEON_FLUSH_ASYNC_START_NEXT_GFX_IB_NOW                                                   \
   (PIPE_FLUSH_ASYNC | RADEON_FLUSH_START_NEXT_GFX_IB_NOW)

#include "amd/common/ac_gpu_info.h"
#include "amd/common/ac_surface.h"
#include "pipebuffer/pb_buffer.h"

/* Tiling flags. */
enum radeon_bo_layout
{
   RADEON_LAYOUT_LINEAR = 0,
   RADEON_LAYOUT_TILED,
   RADEON_LAYOUT_SQUARETILED,

   RADEON_LAYOUT_UNKNOWN
};

enum radeon_bo_domain
{ /* bitfield */
  RADEON_DOMAIN_GTT = 2,
  RADEON_DOMAIN_VRAM = 4,
  RADEON_DOMAIN_VRAM_GTT = RADEON_DOMAIN_VRAM | RADEON_DOMAIN_GTT,
  RADEON_DOMAIN_GDS = 8,
  RADEON_DOMAIN_OA = 16,
};

enum radeon_bo_flag
{ /* bitfield */
  RADEON_FLAG_GTT_WC = (1 << 0),
  RADEON_FLAG_NO_CPU_ACCESS = (1 << 1),
  RADEON_FLAG_NO_SUBALLOC = (1 << 2),
  RADEON_FLAG_SPARSE = (1 << 3),
  RADEON_FLAG_NO_INTERPROCESS_SHARING = (1 << 4),
  RADEON_FLAG_READ_ONLY = (1 << 5),
  RADEON_FLAG_32BIT = (1 << 6),
  RADEON_FLAG_ENCRYPTED = (1 << 7),
  RADEON_FLAG_GL2_BYPASS = (1 << 8), /* only gfx9 and newer */
  RADEON_FLAG_DRIVER_INTERNAL = (1 << 9),
   /* Discard on eviction (instead of moving the buffer to GTT).
    * This guarantees that this buffer will never be moved to GTT.
    */
  RADEON_FLAG_DISCARDABLE = (1 << 10),
  RADEON_FLAG_WINSYS_SLAB_BACKING = (1 << 11), /* only used by the winsys */
};

static inline void
si_res_print_flags(enum radeon_bo_flag flags) {
   if (flags & RADEON_FLAG_GTT_WC)
      fprintf(stderr, "GTT_WC ");
   if (flags & RADEON_FLAG_NO_CPU_ACCESS)
      fprintf(stderr, "NO_CPU_ACCESS ");
   if (flags & RADEON_FLAG_NO_SUBALLOC)
      fprintf(stderr, "NO_SUBALLOC ");
   if (flags & RADEON_FLAG_SPARSE)
      fprintf(stderr, "SPARSE ");
   if (flags & RADEON_FLAG_NO_INTERPROCESS_SHARING)
      fprintf(stderr, "NO_INTERPROCESS_SHARING ");
   if (flags & RADEON_FLAG_READ_ONLY)
      fprintf(stderr, "READ_ONLY ");
   if (flags & RADEON_FLAG_32BIT)
      fprintf(stderr, "32BIT ");
   if (flags & RADEON_FLAG_ENCRYPTED)
      fprintf(stderr, "ENCRYPTED ");
   if (flags & RADEON_FLAG_GL2_BYPASS)
      fprintf(stderr, "GL2_BYPASS ");
   if (flags & RADEON_FLAG_DRIVER_INTERNAL)
      fprintf(stderr, "DRIVER_INTERNAL ");
   if (flags & RADEON_FLAG_DISCARDABLE)
      fprintf(stderr, "DISCARDABLE ");
}

enum radeon_map_flags
{
   /* Indicates that the caller will unmap the buffer.
    *
    * Not unmapping buffers is an important performance optimization for
    * OpenGL (avoids kernel overhead for frequently mapped buffers).
    */
   RADEON_MAP_TEMPORARY = (PIPE_MAP_DRV_PRV << 0),
};

#define RADEON_SPARSE_PAGE_SIZE (64 * 1024)

enum radeon_value_id
{
   RADEON_REQUESTED_VRAM_MEMORY,
   RADEON_REQUESTED_GTT_MEMORY,
   RADEON_MAPPED_VRAM,
   RADEON_MAPPED_GTT,
   RADEON_SLAB_WASTED_VRAM,
   RADEON_SLAB_WASTED_GTT,
   RADEON_BUFFER_WAIT_TIME_NS,
   RADEON_NUM_MAPPED_BUFFERS,
   RADEON_TIMESTAMP,
   RADEON_NUM_GFX_IBS,
   RADEON_NUM_SDMA_IBS,
   RADEON_GFX_BO_LIST_COUNTER, /* number of BOs submitted in gfx IBs */
   RADEON_GFX_IB_SIZE_COUNTER,
   RADEON_NUM_BYTES_MOVED,
   RADEON_NUM_EVICTIONS,
   RADEON_NUM_VRAM_CPU_PAGE_FAULTS,
   RADEON_VRAM_USAGE,
   RADEON_VRAM_VIS_USAGE,
   RADEON_GTT_USAGE,
   RADEON_GPU_TEMPERATURE,
   RADEON_CURRENT_SCLK,
   RADEON_CURRENT_MCLK,
   RADEON_CS_THREAD_TIME,
};

enum radeon_ctx_priority
{
   RADEON_CTX_PRIORITY_LOW = 0,
   RADEON_CTX_PRIORITY_MEDIUM,
   RADEON_CTX_PRIORITY_HIGH,
   RADEON_CTX_PRIORITY_REALTIME,
};

enum radeon_ctx_pstate
{
   RADEON_CTX_PSTATE_NONE = 0,
   RADEON_CTX_PSTATE_STANDARD,
   RADEON_CTX_PSTATE_MIN_SCLK,
   RADEON_CTX_PSTATE_MIN_MCLK,
   RADEON_CTX_PSTATE_PEAK,
};


/* Each group of two has the same priority. */
#define RADEON_PRIO_FENCE_TRACE (1 << 0)
#define RADEON_PRIO_SO_FILLED_SIZE (1 << 1)

#define RADEON_PRIO_QUERY (1 << 2)
#define RADEON_PRIO_IB (1 << 3)

#define RADEON_PRIO_DRAW_INDIRECT (1 << 4)
#define RADEON_PRIO_INDEX_BUFFER (1 << 5)

#define RADEON_PRIO_CP_DMA (1 << 6)
#define RADEON_PRIO_BORDER_COLORS (1 << 7)

#define RADEON_PRIO_CONST_BUFFER (1 << 8)
#define RADEON_PRIO_DESCRIPTORS (1 << 9)

#define RADEON_PRIO_SAMPLER_BUFFER (1 << 10)
#define RADEON_PRIO_VERTEX_BUFFER (1 << 11)

#define RADEON_PRIO_SHADER_RW_BUFFER (1 << 12)
#define RADEON_PRIO_SAMPLER_TEXTURE (1 << 13)

#define RADEON_PRIO_SHADER_RW_IMAGE (1 << 14)
#define RADEON_PRIO_SAMPLER_TEXTURE_MSAA (1 << 15)

#define RADEON_PRIO_COLOR_BUFFER (1 << 16)
#define RADEON_PRIO_DEPTH_BUFFER (1 << 17)

#define RADEON_PRIO_COLOR_BUFFER_MSAA (1 << 18)
#define RADEON_PRIO_DEPTH_BUFFER_MSAA (1 << 19)

#define RADEON_PRIO_SEPARATE_META (1 << 20)
#define RADEON_PRIO_SHADER_BINARY (1 << 21) /* the hw can't hide instruction cache misses */

#define RADEON_PRIO_SHADER_RINGS (1 << 22)
#define RADEON_PRIO_SCRATCH_BUFFER (1 << 23)

#define RADEON_ALL_PRIORITIES (RADEON_USAGE_READ - 1)

/* Upper bits of priorities are used by usage flags. */
#define RADEON_USAGE_READ (1 << 28)
#define RADEON_USAGE_WRITE (1 << 29)
#define RADEON_USAGE_READWRITE (RADEON_USAGE_READ | RADEON_USAGE_WRITE)

/* The winsys ensures that the CS submission will be scheduled after
 * previously flushed CSs referencing this BO in a conflicting way.
 */
#define RADEON_USAGE_SYNCHRONIZED (1 << 30)

/* When used, an implicit sync is done to make sure a compute shader
 * will read the written values from a previous draw.
 */
#define RADEON_USAGE_NEEDS_IMPLICIT_SYNC (1u << 31)

struct winsys_handle;
struct radeon_winsys_ctx;

struct radeon_cmdbuf_chunk {
   unsigned cdw;    /* Number of used dwords. */
   unsigned max_dw; /* Maximum number of dwords. */
   uint32_t *buf;   /* The base pointer of the chunk. */
};

struct radeon_cmdbuf {
   struct radeon_cmdbuf_chunk current;
   struct radeon_cmdbuf_chunk *prev;
   uint16_t num_prev; /* Number of previous chunks. */
   uint16_t max_prev; /* Space in array pointed to by prev. */
   unsigned prev_dw;  /* Total number of dwords in previous chunks. */

   /* Memory usage of the buffer list. These are always 0 for preamble IBs. */
   uint32_t used_vram_kb;
   uint32_t used_gart_kb;

   /* Private winsys data. */
   void *priv;
   void *csc; /* amdgpu_cs_context */
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
   } u;

   enum radeon_surf_mode mode;   /* Output from buffer_get_metadata */

   /* Additional metadata associated with the buffer, in bytes.
    * The maximum size is 64 * 4. This is opaque for the winsys & kernel.
    * Supported by amdgpu only.
    */
   uint32_t size_metadata;
   uint32_t metadata[64];
};

enum radeon_feature_id
{
   RADEON_FID_R300_HYPERZ_ACCESS, /* ZMask + HiZ */
   RADEON_FID_R300_CMASK_ACCESS,
};

struct radeon_bo_list_item {
   uint64_t bo_size;
   uint64_t vm_address;
   uint32_t priority_usage; /* mask of (1 << RADEON_PRIO_*) */
};

struct radeon_winsys {
   /**
    * The screen object this winsys was created for
    */
   struct pipe_screen *screen;
   /**
    * Has the application created at least one TMZ buffer.
    */
   const bool uses_secure_bos;

   /**
    * Decrement the winsys reference count.
    *
    * \param ws  The winsys this function is called for.
    * \return    True if the winsys and screen should be destroyed.
    */
   bool (*unref)(struct radeon_winsys *ws);

   /**
    * Destroy this winsys.
    *
    * \param ws        The winsys this function is called from.
    */
   void (*destroy)(struct radeon_winsys *ws);

   /**
    * Get FD for winsys if winsys provides one
    */
   int (*get_fd)(struct radeon_winsys *ws);

   /**
    * Query an info structure from winsys.
    *
    * \param ws        The winsys this function is called from.
    * \param info      Return structure
    */
   void (*query_info)(struct radeon_winsys *ws, struct radeon_info *info);

   /**
    * A hint for the winsys that it should pin its execution threads to
    * a group of cores sharing a specific L3 cache if the CPU has multiple
    * L3 caches. This is needed for good multithreading performance on
    * AMD Zen CPUs.
    */
   void (*pin_threads_to_L3_cache)(struct radeon_winsys *ws, unsigned cache);

   /**************************************************************************
    * Buffer management. Buffer attributes are mostly fixed over its lifetime.
    *
    * Remember that gallium gets to choose the interface it needs, and the
    * window systems must then implement that interface (rather than the
    * other way around...).
    *************************************************************************/

   /**
    * Create a buffer object.
    *
    * \param ws        The winsys this function is called from.
    * \param size      The size to allocate.
    * \param alignment An alignment of the buffer in memory.
    * \param use_reusable_pool Whether the cache buffer manager should be used.
    * \param domain    A bitmask of the RADEON_DOMAIN_* flags.
    * \return          The created buffer object.
    */
   struct pb_buffer_lean *(*buffer_create)(struct radeon_winsys *ws, uint64_t size,
                                           unsigned alignment, enum radeon_bo_domain domain,
                                           enum radeon_bo_flag flags);

   /**
    * Don't use directly. Use radeon_bo_reference.
    */
   void (*buffer_destroy)(struct radeon_winsys *ws, struct pb_buffer_lean *buf);

   /**
    * Map the entire data store of a buffer object into the client's address
    * space.
    *
    * Callers are expected to unmap buffers again if and only if the
    * RADEON_MAP_TEMPORARY flag is set in \p usage.
    *
    * \param buf       A winsys buffer object to map.
    * \param cs        A command stream to flush if the buffer is referenced by it.
    * \param usage     A bitmask of the PIPE_MAP_* and RADEON_MAP_* flags.
    * \return          The pointer at the beginning of the buffer.
    */
   void *(*buffer_map)(struct radeon_winsys *ws, struct pb_buffer_lean *buf,
                       struct radeon_cmdbuf *cs, enum pipe_map_flags usage);

   /**
    * Unmap a buffer object from the client's address space.
    *
    * \param buf       A winsys buffer object to unmap.
    */
   void (*buffer_unmap)(struct radeon_winsys *ws, struct pb_buffer_lean *buf);

   /**
    * Wait for the buffer and return true if the buffer is not used
    * by the device.
    *
    * The timeout of 0 will only return the status.
    * The timeout of OS_TIMEOUT_INFINITE will always wait until the buffer
    * is idle.
    */
   bool (*buffer_wait)(struct radeon_winsys *ws, struct pb_buffer_lean *buf,
                       uint64_t timeout, unsigned usage);

   /**
    * Return buffer metadata.
    * (tiling info for display code, DRI sharing, and other data)
    *
    * \param buf       A winsys buffer object to get the flags from.
    * \param md        Metadata
    */
   void (*buffer_get_metadata)(struct radeon_winsys *ws, struct pb_buffer_lean *buf,
                               struct radeon_bo_metadata *md, struct radeon_surf *surf);

   /**
    * Set buffer metadata.
    * (tiling info for display code, DRI sharing, and other data)
    *
    * \param buf       A winsys buffer object to set the flags for.
    * \param md        Metadata
    */
   void (*buffer_set_metadata)(struct radeon_winsys *ws, struct pb_buffer_lean *buf,
                               struct radeon_bo_metadata *md, struct radeon_surf *surf);

   /**
    * Get a winsys buffer from a winsys handle. The internal structure
    * of the handle is platform-specific and only a winsys should access it.
    *
    * \param ws        The winsys this function is called from.
    * \param whandle   A winsys handle pointer as was received from a state
    *                  tracker.
    */
   struct pb_buffer_lean *(*buffer_from_handle)(struct radeon_winsys *ws,
                                                struct winsys_handle *whandle,
                                                unsigned vm_alignment,
                                                bool is_prime_linear_buffer);

   /**
    * Get a winsys buffer from a user pointer. The resulting buffer can't
    * be exported. Both pointer and size must be page aligned.
    *
    * \param ws        The winsys this function is called from.
    * \param pointer   User pointer to turn into a buffer object.
    * \param Size      Size in bytes for the new buffer.
    */
   struct pb_buffer_lean *(*buffer_from_ptr)(struct radeon_winsys *ws, void *pointer,
                                             uint64_t size, enum radeon_bo_flag flags);

   /**
    * Whether the buffer was created from a user pointer.
    *
    * \param buf       A winsys buffer object
    * \return          whether \p buf was created via buffer_from_ptr
    */
   bool (*buffer_is_user_ptr)(struct pb_buffer_lean *buf);

   /** Whether the buffer was suballocated. */
   bool (*buffer_is_suballocated)(struct pb_buffer_lean *buf);

   /**
    * Get a winsys handle from a winsys buffer. The internal structure
    * of the handle is platform-specific and only a winsys should access it.
    *
    * \param ws        The winsys instance for which the handle is to be valid
    * \param buf       A winsys buffer object to get the handle from.
    * \param whandle   A winsys handle pointer.
    * \return          true on success.
    */
   bool (*buffer_get_handle)(struct radeon_winsys *ws, struct pb_buffer_lean *buf,
                             struct winsys_handle *whandle);

   /**
    * Change the commitment of a (64KB-page aligned) region of the given
    * sparse buffer.
    *
    * \warning There is no automatic synchronization with command submission.
    *
    * \note Only implemented by the amdgpu winsys.
    *
    * \return false on out of memory or other failure, true on success.
    */
   bool (*buffer_commit)(struct radeon_winsys *ws, struct pb_buffer_lean *buf,
                         uint64_t offset, uint64_t size, bool commit);

   /**
    * Calc size of the first committed part of the given sparse buffer.
    * \note Only implemented by the amdgpu winsys.
    * \return the skipped count if the range_offset fall into a hole.
    */
   unsigned (*buffer_find_next_committed_memory)(struct pb_buffer_lean *buf,
                        uint64_t range_offset, unsigned *range_size);
   /**
    * Return the virtual address of a buffer.
    *
    * When virtual memory is not in use, this is the offset relative to the
    * relocation base (non-zero for sub-allocated buffers).
    *
    * \param buf       A winsys buffer object
    * \return          virtual address
    */
   uint64_t (*buffer_get_virtual_address)(struct pb_buffer_lean *buf);

   /**
    * Return the offset of this buffer relative to the relocation base.
    * This is only non-zero for sub-allocated buffers.
    *
    * This is only supported in the radeon winsys, since amdgpu uses virtual
    * addresses in submissions even for the video engines.
    *
    * \param buf      A winsys buffer object
    * \return         the offset for relocations
    */
   unsigned (*buffer_get_reloc_offset)(struct pb_buffer_lean *buf);

   /**
    * Query the initial placement of the buffer from the kernel driver.
    */
   enum radeon_bo_domain (*buffer_get_initial_domain)(struct pb_buffer_lean *buf);

   /**
    * Query the flags used for creation of this buffer.
    *
    * Note that for imported buffer this may be lossy since not all flags
    * are passed 1:1.
    */
   enum radeon_bo_flag (*buffer_get_flags)(struct pb_buffer_lean *buf);

   /**************************************************************************
    * Command submission.
    *
    * Each pipe context should create its own command stream and submit
    * commands independently of other contexts.
    *************************************************************************/

   /**
    * Create a command submission context.
    * Various command streams can be submitted to the same context.
    *
    * \param allow_context_lost  If true, lost contexts skip command submission and report
    *                            the reset status.
    *                            If false, losing the context results in undefined behavior.
    */
   struct radeon_winsys_ctx *(*ctx_create)(struct radeon_winsys *ws,
                                           enum radeon_ctx_priority priority,
                                           bool allow_context_lost);

   /**
    * Destroy a context.
    */
   void (*ctx_destroy)(struct radeon_winsys_ctx *ctx);

   /**
    * Set a reset status for the context due to a software failure, such as an allocation failure
    * or a skipped draw.
    */
   void (*ctx_set_sw_reset_status)(struct radeon_winsys_ctx *ctx, enum pipe_reset_status status,
                                   const char *format, ...);

   /**
    * Query a GPU reset status.
    */
   enum pipe_reset_status (*ctx_query_reset_status)(struct radeon_winsys_ctx *ctx,
                                                    bool full_reset_only,
                                                    bool *needs_reset, bool *reset_completed);

   /**
    * Create a command stream.
    *
    * \param cs        The returned structure that is initialized by cs_create.
    * \param ctx       The submission context
    * \param ip_type   The IP type (GFX, DMA, UVD)
    * \param flush     Flush callback function associated with the command stream.
    * \param user      User pointer that will be passed to the flush callback.
    *
    * \return true on success
    */
   bool (*cs_create)(struct radeon_cmdbuf *cs,
                     struct radeon_winsys_ctx *ctx, enum amd_ip_type amd_ip_type,
                     void (*flush)(void *ctx, unsigned flags,
                                   struct pipe_fence_handle **fence),
                     void *flush_ctx);

   /**
    * Set up and enable mid command buffer preemption for the command stream.
    *
    * \param cs               Command stream
    * \param preamble_ib      Non-preemptible preamble IB for the context.
    * \param preamble_num_dw  Number of dwords in the preamble IB.
    */
   bool (*cs_setup_preemption)(struct radeon_cmdbuf *cs, const uint32_t *preamble_ib,
                               unsigned preamble_num_dw);

   /**
    * Destroy a command stream.
    *
    * \param cs        A command stream to destroy.
    */
   void (*cs_destroy)(struct radeon_cmdbuf *cs);

   /**
    * Add a buffer. Each buffer used by a CS must be added using this function.
    *
    * \param cs      Command stream
    * \param buf     Buffer
    * \param usage   Usage
    * \param domain  Bitmask of the RADEON_DOMAIN_* flags.
    * \return Buffer index.
    */
   unsigned (*cs_add_buffer)(struct radeon_cmdbuf *cs, struct pb_buffer_lean *buf,
                             unsigned usage, enum radeon_bo_domain domain);

   /**
    * Return the index of an already-added buffer.
    *
    * Not supported on amdgpu. Drivers with GPUVM should not care about
    * buffer indices.
    *
    * \param cs        Command stream
    * \param buf       Buffer
    * \return          The buffer index, or -1 if the buffer has not been added.
    */
   int (*cs_lookup_buffer)(struct radeon_cmdbuf *cs, struct pb_buffer_lean *buf);

   /**
    * Return true if there is enough memory in VRAM and GTT for the buffers
    * added so far. If the validation fails, all buffers which have
    * been added since the last call of cs_validate will be removed and
    * the CS will be flushed (provided there are still any buffers).
    *
    * \param cs        A command stream to validate.
    */
   bool (*cs_validate)(struct radeon_cmdbuf *cs);

   /**
    * Check whether the given number of dwords is available in the IB.
    * Optionally chain a new chunk of the IB if necessary and supported.
    *
    * \param cs        A command stream.
    * \param dw        Number of CS dwords requested by the caller.
    * \return true if there is enough space
    */
   bool (*cs_check_space)(struct radeon_cmdbuf *cs, unsigned dw);

   /**
    * Return the buffer list.
    *
    * This is the buffer list as passed to the kernel, i.e. it only contains
    * the parent buffers of sub-allocated buffers.
    *
    * \param cs    Command stream
    * \param list  Returned buffer list. Set to NULL to query the count only.
    * \return      The buffer count.
    */
   unsigned (*cs_get_buffer_list)(struct radeon_cmdbuf *cs, struct radeon_bo_list_item *list);

   /**
    * Flush a command stream.
    *
    * \param cs          A command stream to flush.
    * \param flags,      PIPE_FLUSH_* flags.
    * \param fence       Pointer to a fence. If non-NULL, a fence is inserted
    *                    after the CS and is returned through this parameter.
    * \return Negative POSIX error code or 0 for success.
    *         Asynchronous submissions never return an error.
    */
   int (*cs_flush)(struct radeon_cmdbuf *cs, unsigned flags, struct pipe_fence_handle **fence);

   /**
    * Create a fence before the CS is flushed.
    * The user must flush manually to complete the initializaton of the fence.
    *
    * The fence must not be used for anything except \ref cs_add_fence_dependency
    * before the flush.
    */
   struct pipe_fence_handle *(*cs_get_next_fence)(struct radeon_cmdbuf *cs);

   /**
    * Return true if a buffer is referenced by a command stream.
    *
    * \param cs        A command stream.
    * \param buf       A winsys buffer.
    */
   bool (*cs_is_buffer_referenced)(struct radeon_cmdbuf *cs, struct pb_buffer_lean *buf,
                                   unsigned usage);

   /**
    * Request access to a feature for a command stream.
    *
    * \param cs        A command stream.
    * \param fid       Feature ID, one of RADEON_FID_*
    * \param enable    Whether to enable or disable the feature.
    */
   bool (*cs_request_feature)(struct radeon_cmdbuf *cs, enum radeon_feature_id fid, bool enable);
   /**
    * Make sure all asynchronous flush of the cs have completed
    *
    * \param cs        A command stream.
    */
   void (*cs_sync_flush)(struct radeon_cmdbuf *cs);

   /**
    * Add a fence dependency to the CS, so that the CS will wait for
    * the fence before execution.
    */
   void (*cs_add_fence_dependency)(struct radeon_cmdbuf *cs, struct pipe_fence_handle *fence);

   /**
    * Signal a syncobj when the CS finishes execution.
    */
   void (*cs_add_syncobj_signal)(struct radeon_cmdbuf *cs, struct pipe_fence_handle *fence);

   /**
    * Returns the amd_ip_type type of a CS.
    */
   enum amd_ip_type (*cs_get_ip_type)(struct radeon_cmdbuf *cs);

   /**
    * Wait for the fence and return true if the fence has been signalled.
    * The timeout of 0 will only return the status.
    * The timeout of OS_TIMEOUT_INFINITE will always wait until the fence
    * is signalled.
    */
   bool (*fence_wait)(struct radeon_winsys *ws, struct pipe_fence_handle *fence, uint64_t timeout);

   /**
    * Reference counting for fences.
    */
   void (*fence_reference)(struct radeon_winsys *ws, struct pipe_fence_handle **dst,
                           struct pipe_fence_handle *src);

   /**
    * Create a new fence object corresponding to the given syncobj fd.
    */
   struct pipe_fence_handle *(*fence_import_syncobj)(struct radeon_winsys *ws, int fd);

   /**
    * Create a new fence object corresponding to the given sync_file.
    */
   struct pipe_fence_handle *(*fence_import_sync_file)(struct radeon_winsys *ws, int fd);

   /**
    * Return a sync_file FD corresponding to the given fence object.
    */
   int (*fence_export_sync_file)(struct radeon_winsys *ws, struct pipe_fence_handle *fence);

   /**
    * Return a sync file FD that is already signalled.
    */
   int (*export_signalled_sync_file)(struct radeon_winsys *ws);

   /**
    * Initialize surface
    *
    * \param ws        The winsys this function is called from.
    * \param info      radeon_info from the driver
    * \param tex       Input texture description
    * \param flags     Bitmask of RADEON_SURF_* flags
    * \param bpe       Bytes per pixel, it can be different for Z buffers.
    * \param mode      Preferred tile mode. (linear, 1D, or 2D)
    * \param surf      Output structure
    */
   int (*surface_init)(struct radeon_winsys *ws, const struct radeon_info *info,
                       const struct pipe_resource *tex, uint64_t flags,
                       unsigned bpe, enum radeon_surf_mode mode, struct radeon_surf *surf);

   uint64_t (*query_value)(struct radeon_winsys *ws, enum radeon_value_id value);

   bool (*read_registers)(struct radeon_winsys *ws, unsigned reg_offset, unsigned num_registers,
                          uint32_t *out);

   /**
    * Secure context
    */
   bool (*cs_is_secure)(struct radeon_cmdbuf *cs);

   /**
    * Stable pstate
    */
   bool (*cs_set_pstate)(struct radeon_cmdbuf *cs, enum radeon_ctx_pstate state);

   /**
    * Pass the VAs to the buffers where various information is saved by the FW during mcbp.
    */
   void (*cs_set_mcbp_reg_shadowing_va)(struct radeon_cmdbuf *cs, uint64_t regs_va,
                                                                  uint64_t csa_va);
};

static inline bool radeon_emitted(struct radeon_cmdbuf *cs, unsigned num_dw)
{
   return cs && (cs->prev_dw + cs->current.cdw > num_dw);
}

static inline void radeon_emit(struct radeon_cmdbuf *cs, uint32_t value)
{
   cs->current.buf[cs->current.cdw++] = value;
}

static inline void radeon_emit_array(struct radeon_cmdbuf *cs, const uint32_t *values,
                                     unsigned count)
{
   memcpy(cs->current.buf + cs->current.cdw, values, count * 4);
   cs->current.cdw += count;
}

static inline bool radeon_uses_secure_bos(struct radeon_winsys* ws)
{
  return ws->uses_secure_bos;
}

static inline void
radeon_bo_reference(struct radeon_winsys *rws, struct pb_buffer_lean **dst,
                    struct pb_buffer_lean *src)
{
   struct pb_buffer_lean *old = *dst;

   if (pipe_reference(&(*dst)->reference, &src->reference))
      rws->buffer_destroy(rws, old);
   *dst = src;
}

/* The following bits describe the heaps managed by slab allocators (pb_slab) and
 * the allocation cache (pb_cache).
 */
#define RADEON_HEAP_BIT_VRAM           (1 << 0) /* if false, it's GTT */
#define RADEON_HEAP_BIT_READ_ONLY      (1 << 1) /* both VRAM and GTT */
#define RADEON_HEAP_BIT_32BIT          (1 << 2) /* both VRAM and GTT */
#define RADEON_HEAP_BIT_ENCRYPTED      (1 << 3) /* both VRAM and GTT */

#define RADEON_HEAP_BIT_NO_CPU_ACCESS  (1 << 4) /* VRAM only */

#define RADEON_HEAP_BIT_WC             (1 << 4) /* GTT only, VRAM implies this to be true */
#define RADEON_HEAP_BIT_GL2_BYPASS     (1 << 5) /* GTT only */

/* The number of all possible heap descriptions using the bits above. */
#define RADEON_NUM_HEAPS               (1 << 6)

static inline enum radeon_bo_domain radeon_domain_from_heap(int heap)
{
   assert(heap >= 0);

   if (heap & RADEON_HEAP_BIT_VRAM)
      return RADEON_DOMAIN_VRAM;
   else
      return RADEON_DOMAIN_GTT;
}

static inline unsigned radeon_flags_from_heap(int heap)
{
   assert(heap >= 0);

   unsigned flags = RADEON_FLAG_NO_INTERPROCESS_SHARING;

   if (heap & RADEON_HEAP_BIT_READ_ONLY)
      flags |= RADEON_FLAG_READ_ONLY;
   if (heap & RADEON_HEAP_BIT_32BIT)
      flags |= RADEON_FLAG_32BIT;
   if (heap & RADEON_HEAP_BIT_ENCRYPTED)
      flags |= RADEON_FLAG_ENCRYPTED;

   if (heap & RADEON_HEAP_BIT_VRAM) {
      flags |= RADEON_FLAG_GTT_WC;
      if (heap & RADEON_HEAP_BIT_NO_CPU_ACCESS)
         flags |= RADEON_FLAG_NO_CPU_ACCESS;
   } else {
      /* GTT only */
      if (heap & RADEON_HEAP_BIT_WC)
         flags |= RADEON_FLAG_GTT_WC;
      if (heap & RADEON_HEAP_BIT_GL2_BYPASS)
         flags |= RADEON_FLAG_GL2_BYPASS;
   }

   return flags;
}

/* This cleans up flags, so that we can comfortably assume that no invalid flag combinations
 * are set.
 */
static void radeon_canonicalize_bo_flags(enum radeon_bo_domain *_domain,
                                         enum radeon_bo_flag *_flags)
{
   unsigned domain = *_domain;
   unsigned flags = *_flags;

   /* Only set 1 domain, e.g. ignore GTT if VRAM is set. */
   if (domain)
      domain = BITFIELD_BIT(ffs(domain) - 1);
   else
      domain = RADEON_DOMAIN_VRAM;

   switch (domain) {
   case RADEON_DOMAIN_VRAM:
      flags |= RADEON_FLAG_GTT_WC;
      flags &= ~RADEON_FLAG_GL2_BYPASS;
      break;
   case RADEON_DOMAIN_GTT:
      flags &= ~RADEON_FLAG_NO_CPU_ACCESS;
      break;
   case RADEON_DOMAIN_GDS:
   case RADEON_DOMAIN_OA:
      flags |= RADEON_FLAG_NO_SUBALLOC | RADEON_FLAG_NO_CPU_ACCESS;
      flags &= ~RADEON_FLAG_SPARSE;
      break;
   }

   /* Sparse buffers must have NO_CPU_ACCESS set. */
   if (flags & RADEON_FLAG_SPARSE)
      flags |= RADEON_FLAG_NO_CPU_ACCESS;

   *_domain = (enum radeon_bo_domain)domain;
   *_flags = (enum radeon_bo_flag)flags;
}

/* Return the heap index for winsys allocators, or -1 on failure. */
static inline int radeon_get_heap_index(enum radeon_bo_domain domain, enum radeon_bo_flag flags)
{
   radeon_canonicalize_bo_flags(&domain, &flags);

   /* Resources with interprocess sharing don't use any winsys allocators. */
   if (!(flags & RADEON_FLAG_NO_INTERPROCESS_SHARING))
      return -1;

   /* These are unsupported flags. */
   /* RADEON_FLAG_DRIVER_INTERNAL is ignored. It doesn't affect allocators. */
   if (flags & (RADEON_FLAG_NO_SUBALLOC | RADEON_FLAG_SPARSE |
                RADEON_FLAG_DISCARDABLE))
      return -1;

   int heap = 0;

   if (flags & RADEON_FLAG_READ_ONLY)
      heap |= RADEON_HEAP_BIT_READ_ONLY;
   if (flags & RADEON_FLAG_32BIT)
      heap |= RADEON_HEAP_BIT_32BIT;
   if (flags & RADEON_FLAG_ENCRYPTED)
      heap |= RADEON_HEAP_BIT_ENCRYPTED;

   if (domain == RADEON_DOMAIN_VRAM) {
      /* VRAM | GTT shouldn't occur, but if it does, ignore GTT. */
      heap |= RADEON_HEAP_BIT_VRAM;
      if (flags & RADEON_FLAG_NO_CPU_ACCESS)
         heap |= RADEON_HEAP_BIT_NO_CPU_ACCESS;
      /* RADEON_FLAG_WC is ignored and implied to be true for VRAM */
      /* RADEON_FLAG_GL2_BYPASS is ignored and implied to be false for VRAM */
   } else if (domain == RADEON_DOMAIN_GTT) {
      /* GTT is implied by RADEON_HEAP_BIT_VRAM not being set. */
      if (flags & RADEON_FLAG_GTT_WC)
         heap |= RADEON_HEAP_BIT_WC;
      if (flags & RADEON_FLAG_GL2_BYPASS)
         heap |= RADEON_HEAP_BIT_GL2_BYPASS;
      /* RADEON_FLAG_NO_CPU_ACCESS is ignored and implied to be false for GTT */
      /* RADEON_FLAG_MALL_NOALLOC is ignored and implied to be false for GTT */
   } else {
      return -1; /*  */
   }

   assert(heap < RADEON_NUM_HEAPS);
   return heap;
}

typedef struct pipe_screen *(*radeon_screen_create_t)(struct radeon_winsys *,
                                                      const struct pipe_screen_config *);

/* These functions create the radeon_winsys instance for the corresponding kernel driver. */
struct radeon_winsys *
amdgpu_winsys_create(int fd, const struct pipe_screen_config *config,
		     radeon_screen_create_t screen_create);
struct radeon_winsys *
radeon_drm_winsys_create(int fd, const struct pipe_screen_config *config,
			 radeon_screen_create_t screen_create);

#endif
