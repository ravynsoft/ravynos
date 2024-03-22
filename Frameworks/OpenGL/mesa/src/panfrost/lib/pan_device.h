/**************************************************************************
 *
 * Copyright 2018-2019 Alyssa Rosenzweig
 * Copyright 2018-2019 Collabora, Ltd.
 * Copyright © 2015 Intel Corporation
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef PAN_DEVICE_H
#define PAN_DEVICE_H

#include <xf86drm.h>
#include "renderonly/renderonly.h"
#include "util/bitset.h"
#include "util/list.h"
#include "util/sparse_array.h"
#include "util/u_dynarray.h"

#include "panfrost/util/pan_ir.h"
#include "pan_pool.h"
#include "pan_util.h"

#include "kmod/pan_kmod.h"

#include <genxml/gen_macros.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* Driver limits */
#define PAN_MAX_CONST_BUFFERS 16

/* How many power-of-two levels in the BO cache do we want? 2^12
 * minimum chosen as it is the page size that all allocations are
 * rounded to */

#define MIN_BO_CACHE_BUCKET (12) /* 2^12 = 4KB */
#define MAX_BO_CACHE_BUCKET (22) /* 2^22 = 4MB */

/* Fencepost problem, hence the off-by-one */
#define NR_BO_CACHE_BUCKETS (MAX_BO_CACHE_BUCKET - MIN_BO_CACHE_BUCKET + 1)

struct pan_blitter {
   struct {
      struct pan_pool *pool;
      struct hash_table *blit;
      struct hash_table *blend;
      pthread_mutex_t lock;
   } shaders;
   struct {
      struct pan_pool *pool;
      struct hash_table *rsds;
      pthread_mutex_t lock;
   } rsds;
};

struct pan_blend_shaders {
   struct hash_table *shaders;
   pthread_mutex_t lock;
};

struct pan_indirect_dispatch {
   struct panfrost_ubo_push push;
   struct panfrost_bo *bin;
   struct panfrost_bo *descs;
};

/** Implementation-defined tiler features */
struct panfrost_tiler_features {
   /** Number of bytes per tiler bin */
   unsigned bin_size;

   /** Maximum number of levels that may be simultaneously enabled.
    * Invariant: bitcount(hierarchy_mask) <= max_levels */
   unsigned max_levels;
};

struct panfrost_model {
   /* GPU ID */
   uint32_t gpu_id;

   /* Marketing name for the GPU, used as the GL_RENDERER */
   const char *name;

   /* Set of associated performance counters */
   const char *performance_counters;

   /* Minimum GPU revision required for anisotropic filtering. ~0 and 0
    * means "no revisions support anisotropy" and "all revisions support
    * anistropy" respectively -- so checking for anisotropy is simply
    * comparing the reivsion.
    */
   uint32_t min_rev_anisotropic;

   /* Default tilebuffer size in bytes for the model. */
   unsigned tilebuffer_size;

   struct {
      /* The GPU lacks the capability for hierarchical tiling, without
       * an "Advanced Tiling Unit", instead requiring a single bin
       * size for the entire framebuffer be selected by the driver
       */
      bool no_hierarchical_tiling;
   } quirks;
};

struct panfrost_device {
   /* For ralloc */
   void *memctx;

   /* Kmod objects. */
   struct {
      /* The pan_kmod_dev object backing this device. */
      struct pan_kmod_dev *dev;

      /* Cached pan_kmod_dev_props properties queried at device create time. */
      struct pan_kmod_dev_props props;

      /* VM attached to this device. */
      struct pan_kmod_vm *vm;
   } kmod;

   /* For pandecode */
   struct pandecode_context *decode_ctx;

   /* Properties of the GPU in use */
   unsigned arch;

   /* Number of shader cores */
   unsigned core_count;

   /* Range of core IDs, equal to the maximum core ID + 1. Satisfies
    * core_id_range >= core_count.
    */
   unsigned core_id_range;

   /* Maximum tilebuffer size in bytes for optimal performance. */
   unsigned optimal_tib_size;

   unsigned thread_tls_alloc;
   struct panfrost_tiler_features tiler_features;
   const struct panfrost_model *model;
   bool has_afbc;

   /* Table of formats, indexed by a PIPE format */
   const struct panfrost_format *formats;
   const struct pan_blendable_format *blendable_formats;

   /* Bitmask of supported compressed texture formats */
   uint32_t compressed_formats;

   /* debug flags, see pan_util.h how to interpret */
   unsigned debug;

   struct renderonly *ro;

   pthread_mutex_t bo_map_lock;
   struct util_sparse_array bo_map;

   struct {
      pthread_mutex_t lock;

      /* List containing all cached BOs sorted in LRU (Least
       * Recently Used) order. This allows us to quickly evict BOs
       * that are more than 1 second old.
       */
      struct list_head lru;

      /* The BO cache is a set of buckets with power-of-two sizes
       * ranging from 2^12 (4096, the page size) to
       * 2^(12 + MAX_BO_CACHE_BUCKETS).
       * Each bucket is a linked list of free panfrost_bo objects. */

      struct list_head buckets[NR_BO_CACHE_BUCKETS];
   } bo_cache;

   struct pan_blitter blitter;
   struct pan_blend_shaders blend_shaders;
   struct pan_indirect_dispatch indirect_dispatch;

   /* Tiler heap shared across all tiler jobs, allocated against the
    * device since there's only a single tiler. Since this is invisible to
    * the CPU, it's okay for multiple contexts to reference it
    * simultaneously; by keeping on the device struct, we eliminate a
    * costly per-context allocation. */

   struct panfrost_bo *tiler_heap;

   /* The tiler heap is shared by all contexts, and is written by tiler
    * jobs and read by fragment job. We need to ensure that a
    * vertex/tiler job chain from one context is not inserted between
    * the vertex/tiler and fragment job of another context, otherwise
    * we end up with tiler heap corruption.
    */
   pthread_mutex_t submit_lock;

   /* Sample positions are preloaded into a write-once constant buffer,
    * such that they can be referenced fore free later. Needed
    * unconditionally on Bifrost, and useful for sharing with Midgard */

   struct panfrost_bo *sample_positions;
};

static inline int
panfrost_device_fd(const struct panfrost_device *dev)
{
   return dev->kmod.dev->fd;
}

static inline uint32_t
panfrost_device_gpu_id(const struct panfrost_device *dev)
{
   return dev->kmod.props.gpu_prod_id;
}

static inline uint32_t
panfrost_device_gpu_rev(const struct panfrost_device *dev)
{
   return dev->kmod.props.gpu_revision;
}

static inline int
panfrost_device_kmod_version_major(const struct panfrost_device *dev)
{
   return dev->kmod.dev->driver.version.major;
}

static inline int
panfrost_device_kmod_version_minor(const struct panfrost_device *dev)
{
   return dev->kmod.dev->driver.version.minor;
}

void panfrost_open_device(void *memctx, int fd, struct panfrost_device *dev);

void panfrost_close_device(struct panfrost_device *dev);

bool panfrost_supports_compressed_format(struct panfrost_device *dev,
                                         unsigned fmt);

void panfrost_upload_sample_positions(struct panfrost_device *dev);

mali_ptr panfrost_sample_positions(const struct panfrost_device *dev,
                                   enum mali_sample_pattern pattern);

unsigned panfrost_query_l2_slices(const struct panfrost_device *dev);

static inline struct panfrost_bo *
pan_lookup_bo(struct panfrost_device *dev, uint32_t gem_handle)
{
   return (struct panfrost_bo *)util_sparse_array_get(&dev->bo_map, gem_handle);
}

static inline bool
pan_is_bifrost(const struct panfrost_device *dev)
{
   return dev->arch >= 6 && dev->arch <= 7;
}

const struct panfrost_model *panfrost_get_model(uint32_t gpu_id);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
