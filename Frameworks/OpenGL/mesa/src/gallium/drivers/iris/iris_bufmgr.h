/*
 * Copyright © 2017 Intel Corporation
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

#ifndef IRIS_BUFMGR_H
#define IRIS_BUFMGR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include "c11/threads.h"
#include "util/macros.h"
#include "util/u_atomic.h"
#include "util/u_dynarray.h"
#include "util/list.h"
#include "util/simple_mtx.h"
#include "pipe/p_defines.h"
#include "pipebuffer/pb_slab.h"
#include "intel/dev/intel_device_info.h"

struct intel_device_info;
struct util_debug_callback;
struct isl_surf;
struct iris_syncobj;

/**
 * Memory zones.  When allocating a buffer, you can request that it is
 * placed into a specific region of the virtual address space (PPGTT).
 *
 * Most buffers can go anywhere (IRIS_MEMZONE_OTHER).  Some buffers are
 * accessed via an offset from a base address.  STATE_BASE_ADDRESS has
 * a maximum 4GB size for each region, so we need to restrict those
 * buffers to be within 4GB of the base.  Each memory zone corresponds
 * to a particular base address.
 *
 * We lay out the virtual address space as follows:
 *
 * - [0,   4K): Nothing            (empty page for null address)
 * - [4K,  4G): Shaders            (Instruction Base Address)
 * - [4G,  8G): Surfaces & Binders (Surface State Base Address, Bindless ...)
 * - [8G, 12G): Dynamic            (Dynamic State Base Address)
 * - [12G, *):  Other              (everything else in the full 48-bit VMA)
 *
 * A special buffer for border color lives at the start of the dynamic state
 * memory zone.  This unfortunately has to be handled specially because the
 * SAMPLER_STATE "Indirect State Pointer" field is only a 24-bit pointer.
 *
 * Each GL context uses a separate GEM context, which technically gives them
 * each a separate VMA.  However, we assign address globally, so buffers will
 * have the same address in all GEM contexts.  This lets us have a single BO
 * field for the address, which is easy and cheap.
 */
enum iris_memory_zone {
   IRIS_MEMZONE_SHADER,
   IRIS_MEMZONE_BINDER,
   IRIS_MEMZONE_SCRATCH,
   IRIS_MEMZONE_SURFACE,
   IRIS_MEMZONE_DYNAMIC,
   IRIS_MEMZONE_OTHER,

   IRIS_MEMZONE_BORDER_COLOR_POOL,
};

/* Intentionally exclude single buffer "zones" */
#define IRIS_MEMZONE_COUNT (IRIS_MEMZONE_OTHER + 1)

#define IRIS_SCRATCH_ZONE_SIZE (8 * 1024 * 1024)
#define IRIS_BINDER_ZONE_SIZE ((1ull << 30) - IRIS_SCRATCH_ZONE_SIZE)

#define IRIS_MEMZONE_SHADER_START     (0ull * (1ull << 32))
#define IRIS_MEMZONE_BINDER_START     (1ull * (1ull << 32))
#define IRIS_MEMZONE_SCRATCH_START    IRIS_MEMZONE_BINDER_START
#define IRIS_MEMZONE_SURFACE_START    (IRIS_MEMZONE_BINDER_START + IRIS_BINDER_ZONE_SIZE)
#define IRIS_MEMZONE_DYNAMIC_START    (2ull * (1ull << 32))
#define IRIS_MEMZONE_OTHER_START      (3ull * (1ull << 32))

#define IRIS_BORDER_COLOR_POOL_ADDRESS IRIS_MEMZONE_DYNAMIC_START
#define IRIS_BORDER_COLOR_POOL_SIZE (64 * 4096)

/**
 * Classification of the various incoherent caches of the GPU into a number of
 * caching domains.
 */
enum iris_domain {
   /** Render color cache. */
   IRIS_DOMAIN_RENDER_WRITE = 0,
   /** (Hi)Z/stencil cache. */
   IRIS_DOMAIN_DEPTH_WRITE,
   /** Data port (HDC) cache. */
   IRIS_DOMAIN_DATA_WRITE,
   /** Any other read-write cache. */
   IRIS_DOMAIN_OTHER_WRITE,
   /** Vertex cache. */
   IRIS_DOMAIN_VF_READ,
   /** Texture cache. */
   IRIS_DOMAIN_SAMPLER_READ,
   /** Pull-style shader constant loads. */
   IRIS_DOMAIN_PULL_CONSTANT_READ,
   /** Any other read-only cache, including reads from non-L3 clients. */
   IRIS_DOMAIN_OTHER_READ,
   /** Number of caching domains. */
   NUM_IRIS_DOMAINS,
   /** Not a real cache, use to opt out of the cache tracking mechanism. */
   IRIS_DOMAIN_NONE = NUM_IRIS_DOMAINS
};

/**
 * Whether a caching domain is guaranteed not to write any data to memory.
 */
static inline bool
iris_domain_is_read_only(enum iris_domain access)
{
   return access >= IRIS_DOMAIN_VF_READ &&
          access <= IRIS_DOMAIN_OTHER_READ;
}

static inline bool
iris_domain_is_l3_coherent(const struct intel_device_info *devinfo,
                           enum iris_domain access)
{
   /* VF reads are coherent with the L3 on Tigerlake+ because we set
    * the "L3 Bypass Disable" bit in the vertex/index buffer packets.
    */
   if (access == IRIS_DOMAIN_VF_READ)
      return devinfo->ver >= 12;

   return access != IRIS_DOMAIN_OTHER_WRITE &&
          access != IRIS_DOMAIN_OTHER_READ;
}

enum iris_mmap_mode {
   IRIS_MMAP_NONE, /**< Cannot be mapped */
   IRIS_MMAP_UC, /**< Fully uncached memory map */
   IRIS_MMAP_WC, /**< Write-combining map with no caching of reads */
   IRIS_MMAP_WB, /**< Write-back mapping with CPU caches enabled */
};

enum iris_heap {
   /**
    * System memory which is CPU-cached at (at least 1-way) coherent.
    *
    * This will use WB (write-back) CPU mappings.
    *
    * LLC systems and discrete cards (which enable snooping) will mostly use
    * this heap.  Non-LLC systems will only use it when explicit coherency is
    * required, as snooping is expensive there.
    */
   IRIS_HEAP_SYSTEM_MEMORY_CACHED_COHERENT,

   /**
    * System memory which is not CPU cached.
    *
    * This will use WC (write-combining) CPU mappings, which has uncached
    * performance for reads.  This can be used for scanout on integrated
    * GPUs (which is never coherent with CPU caches).  It will be used for
    * most buffers on non-LLC platforms, where cache coherency is expensive.
    */
   IRIS_HEAP_SYSTEM_MEMORY_UNCACHED,

   /** Device-local memory (VRAM).  Cannot be placed in system memory! */
   IRIS_HEAP_DEVICE_LOCAL,

   /** Device-local memory that may be evicted to system memory if needed. */
   IRIS_HEAP_DEVICE_LOCAL_PREFERRED,

   IRIS_HEAP_MAX,
};

extern const char *iris_heap_to_string[];

static inline bool
iris_heap_is_device_local(enum iris_heap heap)
{
   return heap == IRIS_HEAP_DEVICE_LOCAL ||
          heap == IRIS_HEAP_DEVICE_LOCAL_PREFERRED;
}

#define IRIS_BATCH_COUNT 3

struct iris_bo_screen_deps {
   struct iris_syncobj *write_syncobjs[IRIS_BATCH_COUNT];
   struct iris_syncobj *read_syncobjs[IRIS_BATCH_COUNT];
};

struct iris_bo {
   /**
    * Size in bytes of the buffer object.
    *
    * The size may be larger than the size originally requested for the
    * allocation, such as being aligned to page size.
    */
   uint64_t size;

   /** Buffer manager context associated with this buffer object */
   struct iris_bufmgr *bufmgr;

   /** Pre-computed hash using _mesa_hash_pointer for cache tracking sets */
   uint32_t hash;

   /** The GEM handle for this buffer object. */
   uint32_t gem_handle;

   /**
    * Canonical virtual address of the buffer inside the PPGTT (Per-Process Graphics
    * Translation Table).
    *
    * Although each hardware context has its own VMA, we assign BO's to the
    * same address in all contexts, for simplicity.
    */
   uint64_t address;

   /**
    * If non-zero, then this bo has an aux-map translation to this address.
    */
   uint64_t aux_map_address;

   /**
    * If this BO is referenced by a batch, this _may_ be the index into the
    * batch->exec_bos[] list.
    *
    * Note that a single buffer may be used by multiple batches/contexts,
    * and thus appear in multiple lists, but we only track one index here.
    * In the common case one can guess that batch->exec_bos[bo->index] == bo
    * and double check if that's true to avoid a linear list walk.
    *
    * XXX: this is not ideal now that we have more than one batch per context,
    * XXX: as the index will flop back and forth between the render index and
    * XXX: compute index...
    */
   unsigned index;

   int refcount;
   const char *name;

   /** BO cache list */
   struct list_head head;

   /**
    * Synchronization sequence number of most recent access of this BO from
    * each caching domain.
    *
    * Although this is a global field, use in multiple contexts should be
    * safe, see iris_emit_buffer_barrier_for() for details.
    *
    * Also align it to 64 bits. This will make atomic operations faster on 32
    * bit platforms.
    */
   alignas(8) uint64_t last_seqnos[NUM_IRIS_DOMAINS];

   /** Up to one per screen, may need realloc. */
   struct iris_bo_screen_deps *deps;
   int deps_size;

   /**
    * Boolean of whether the GPU is definitely not accessing the buffer.
    *
    * This is only valid when reusable, since non-reusable
    * buffers are those that have been shared with other
    * processes, so we don't know their state.
    */
   bool idle;

   /** Was this buffer zeroed at allocation time? */
   bool zeroed;

   union {
      struct {
         uint64_t kflags;

         time_t free_time;

         /** Mapped address for the buffer, saved across map/unmap cycles */
         void *map;

         /** List of GEM handle exports of this buffer (bo_export) */
         struct list_head exports;

         /**
          * Kernel-assigned global name for this object
          *
          * List contains both flink named and prime fd'd objects
          */
         unsigned global_name;

         /** Prime fd used for shared buffers, -1 otherwise. */
         int prime_fd;

         /** The mmap coherency mode selected at BO allocation time */
         enum iris_mmap_mode mmap_mode;

         /** The heap selected at BO allocation time */
         enum iris_heap heap;

         /** Was this buffer imported from an external client? */
         bool imported;

         /** Has this buffer been exported to external clients? */
         bool exported;

         /** Boolean of whether this buffer can be re-used */
         bool reusable;

         /** Boolean of whether this buffer points into user memory */
         bool userptr;

         /** Boolean of whether this buffer is protected (HW encryption) */
         bool protected;
      } real;
      struct {
         struct pb_slab_entry entry;
         struct iris_bo *real;
      } slab;
   };
};

#define BO_ALLOC_PLAIN       0
#define BO_ALLOC_ZEROED      (1<<0)
#define BO_ALLOC_COHERENT    (1<<1)
#define BO_ALLOC_SMEM        (1<<2)
#define BO_ALLOC_SCANOUT     (1<<3)
#define BO_ALLOC_NO_SUBALLOC (1<<4)
#define BO_ALLOC_LMEM        (1<<5)
#define BO_ALLOC_PROTECTED   (1<<6)
#define BO_ALLOC_SHARED      (1<<7)

/**
 * Allocate a buffer object.
 *
 * Buffer objects are not necessarily initially mapped into CPU virtual
 * address space or graphics device aperture.  They must be mapped
 * using iris_bo_map() to be used by the CPU.
 */
struct iris_bo *iris_bo_alloc(struct iris_bufmgr *bufmgr,
                              const char *name,
                              uint64_t size,
                              uint32_t alignment,
                              enum iris_memory_zone memzone,
                              unsigned flags);

struct iris_bo *
iris_bo_create_userptr(struct iris_bufmgr *bufmgr, const char *name,
                       void *ptr, size_t size,
                       enum iris_memory_zone memzone);

/** Takes a reference on a buffer object */
static inline void
iris_bo_reference(struct iris_bo *bo)
{
   p_atomic_inc(&bo->refcount);
}

/**
 * Releases a reference on a buffer object, freeing the data if
 * no references remain.
 */
void iris_bo_unreference(struct iris_bo *bo);

#define MAP_READ          PIPE_MAP_READ
#define MAP_WRITE         PIPE_MAP_WRITE
#define MAP_ASYNC         PIPE_MAP_UNSYNCHRONIZED
#define MAP_PERSISTENT    PIPE_MAP_PERSISTENT
#define MAP_COHERENT      PIPE_MAP_COHERENT
/* internal */
#define MAP_RAW           (PIPE_MAP_DRV_PRV << 0)
#define MAP_INTERNAL_MASK (MAP_RAW)

#define MAP_FLAGS         (MAP_READ | MAP_WRITE | MAP_ASYNC | \
                           MAP_PERSISTENT | MAP_COHERENT | MAP_INTERNAL_MASK)

/**
 * Maps the buffer into userspace.
 *
 * This function will block waiting for any existing execution on the
 * buffer to complete, first.  The resulting mapping is returned.
 */
MUST_CHECK void *iris_bo_map(struct util_debug_callback *dbg,
                             struct iris_bo *bo, unsigned flags);

/**
 * Reduces the refcount on the userspace mapping of the buffer
 * object.
 */
static inline int iris_bo_unmap(struct iris_bo *bo) { return 0; }

/**
 * Waits for rendering to an object by the GPU to have completed.
 *
 * This is not required for any access to the BO by bo_map,
 * bo_subdata, etc.  It is merely a way for the driver to implement
 * glFinish.
 */
void iris_bo_wait_rendering(struct iris_bo *bo);


/**
 * Unref a buffer manager instance.
 */
void iris_bufmgr_unref(struct iris_bufmgr *bufmgr);

/**
 * Create a visible name for a buffer which can be used by other apps
 *
 * \param buf Buffer to create a name for
 * \param name Returned name
 */
int iris_bo_flink(struct iris_bo *bo, uint32_t *name);

/**
 * Returns true if the BO is backed by a real GEM object, false if it's
 * a wrapper that's suballocated from a larger BO.
 */
static inline bool
iris_bo_is_real(struct iris_bo *bo)
{
   return bo->gem_handle != 0;
}

/**
 * Unwrap any slab-allocated wrapper BOs to get the BO for the underlying
 * backing storage, which is a real BO associated with a GEM object.
 */
static inline struct iris_bo *
iris_get_backing_bo(struct iris_bo *bo)
{
   if (!iris_bo_is_real(bo))
      bo = bo->slab.real;

   /* We only allow one level of wrapping. */
   assert(iris_bo_is_real(bo));

   return bo;
}

/**
 * Is this buffer shared with external clients (imported or exported)?
 */
static inline bool
iris_bo_is_external(const struct iris_bo *bo)
{
   bo = iris_get_backing_bo((struct iris_bo *) bo);
   return bo->real.exported || bo->real.imported;
}

static inline bool
iris_bo_is_imported(const struct iris_bo *bo)
{
   bo = iris_get_backing_bo((struct iris_bo *) bo);
   return bo->real.imported;
}

static inline bool
iris_bo_is_exported(const struct iris_bo *bo)
{
   bo = iris_get_backing_bo((struct iris_bo *) bo);
   return bo->real.exported;
}

/**
 * True if the BO prefers to reside in device-local memory.
 *
 * We don't consider eviction here; this is meant to be a performance hint.
 * It will return true for BOs allocated from the LMEM or LMEM+SMEM heaps,
 * even if the buffer has been temporarily evicted to system memory.
 */
static inline bool
iris_bo_likely_local(const struct iris_bo *bo)
{
   if (!bo)
      return false;

   bo = iris_get_backing_bo((struct iris_bo *) bo);
   return iris_heap_is_device_local(bo->real.heap);
}

static inline enum iris_mmap_mode
iris_bo_mmap_mode(const struct iris_bo *bo)
{
   bo = iris_get_backing_bo((struct iris_bo *) bo);
   return bo->real.mmap_mode;
}

/**
 * Mark a buffer as being shared with other external clients.
 */
void iris_bo_mark_exported(struct iris_bo *bo);

/**
 * Returns true  if mapping the buffer for write could cause the process
 * to block, due to the object being active in the GPU.
 */
bool iris_bo_busy(struct iris_bo *bo);

struct iris_bufmgr *iris_bufmgr_get_for_fd(int fd, bool bo_reuse);
int iris_bufmgr_get_fd(struct iris_bufmgr *bufmgr);

struct iris_bo *iris_bo_gem_create_from_name(struct iris_bufmgr *bufmgr,
                                             const char *name,
                                             unsigned handle);

void* iris_bufmgr_get_aux_map_context(struct iris_bufmgr *bufmgr);

int iris_gem_get_tiling(struct iris_bo *bo, uint32_t *tiling);
int iris_gem_set_tiling(struct iris_bo *bo, const struct isl_surf *surf);

int iris_bo_export_dmabuf(struct iris_bo *bo, int *prime_fd);
struct iris_bo *iris_bo_import_dmabuf(struct iris_bufmgr *bufmgr, int prime_fd,
                                      const uint64_t modifier);

/**
 * Exports a bo as a GEM handle into a given DRM file descriptor
 * \param bo Buffer to export
 * \param drm_fd File descriptor where the new handle is created
 * \param out_handle Pointer to store the new handle
 *
 * Returns 0 if the buffer was successfully exported, a non zero error code
 * otherwise.
 */
int iris_bo_export_gem_handle_for_device(struct iris_bo *bo, int drm_fd,
                                         uint32_t *out_handle);

/**
 * Returns the BO's address relative to the appropriate base address.
 *
 * All of our base addresses are programmed to the start of a 4GB region,
 * so simply returning the bottom 32 bits of the BO address will give us
 * the offset from whatever base address corresponds to that memory region.
 */
static inline uint32_t
iris_bo_offset_from_base_address(struct iris_bo *bo)
{
   /* This only works for buffers in the memory zones corresponding to a
    * base address - the top, unbounded memory zone doesn't have a base.
    */
   assert(bo->address < IRIS_MEMZONE_OTHER_START);
   return bo->address;
}

/**
 * Track access of a BO from the specified caching domain and sequence number.
 *
 * Can be used without locking.  Only the most recent access (i.e. highest
 * seqno) is tracked.
 */
static inline void
iris_bo_bump_seqno(struct iris_bo *bo, uint64_t seqno,
                   enum iris_domain type)
{
   uint64_t *const last_seqno = &bo->last_seqnos[type];
   uint64_t tmp, prev_seqno = p_atomic_read(last_seqno);

   while (prev_seqno < seqno &&
          prev_seqno != (tmp = p_atomic_cmpxchg(last_seqno, prev_seqno, seqno)))
      prev_seqno = tmp;
}

/**
 * Return the PAT entry based for the given heap.
 */
const struct intel_device_info_pat_entry *
iris_heap_to_pat_entry(const struct intel_device_info *devinfo,
                       enum iris_heap heap);

enum iris_memory_zone iris_memzone_for_address(uint64_t address);

int iris_bufmgr_create_screen_id(struct iris_bufmgr *bufmgr);

simple_mtx_t *iris_bufmgr_get_bo_deps_lock(struct iris_bufmgr *bufmgr);

/**
 * A pool containing SAMPLER_BORDER_COLOR_STATE entries.
 *
 * See iris_border_color.c for more information.
 */
struct iris_border_color_pool {
   struct iris_bo *bo;
   void *map;
   unsigned insert_point;

   /** Map from border colors to offsets in the buffer. */
   struct hash_table *ht;

   /** Protects insert_point and the hash table. */
   simple_mtx_t lock;
};

struct iris_border_color_pool *iris_bufmgr_get_border_color_pool(
      struct iris_bufmgr *bufmgr);

/* iris_border_color.c */
void iris_init_border_color_pool(struct iris_bufmgr *bufmgr,
                                 struct iris_border_color_pool *pool);
void iris_destroy_border_color_pool(struct iris_border_color_pool *pool);
uint32_t iris_upload_border_color(struct iris_border_color_pool *pool,
                                  union pipe_color_union *color);

uint64_t iris_bufmgr_vram_size(struct iris_bufmgr *bufmgr);
uint64_t iris_bufmgr_sram_size(struct iris_bufmgr *bufmgr);
const struct intel_device_info *iris_bufmgr_get_device_info(struct iris_bufmgr *bufmgr);
const struct iris_kmd_backend *
iris_bufmgr_get_kernel_driver_backend(struct iris_bufmgr *bufmgr);
uint32_t iris_bufmgr_get_global_vm_id(struct iris_bufmgr *bufmgr);
bool iris_bufmgr_use_global_vm_id(struct iris_bufmgr *bufmgr);

enum iris_madvice {
   IRIS_MADVICE_WILL_NEED = 0,
   IRIS_MADVICE_DONT_NEED = 1,
};

void iris_bo_import_sync_state(struct iris_bo *bo, int sync_file_fd);
struct iris_syncobj *iris_bo_export_sync_state(struct iris_bo *bo);

#endif /* IRIS_BUFMGR_H */
