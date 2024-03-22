/*
 * Copyright 2021 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#ifndef __AGX_DEVICE_H
#define __AGX_DEVICE_H

#include "util/simple_mtx.h"
#include "util/sparse_array.h"
#include "util/timespec.h"
#include "util/vma.h"
#include "agx_bo.h"
#include "agx_formats.h"

enum agx_dbg {
   AGX_DBG_TRACE = BITFIELD_BIT(0),
   AGX_DBG_NOCLIPCTRL = BITFIELD_BIT(1),
   AGX_DBG_NO16 = BITFIELD_BIT(2),
   AGX_DBG_DIRTY = BITFIELD_BIT(3),
   AGX_DBG_PRECOMPILE = BITFIELD_BIT(4),
   AGX_DBG_PERF = BITFIELD_BIT(5),
   AGX_DBG_NOCOMPRESS = BITFIELD_BIT(6),
   AGX_DBG_NOCLUSTER = BITFIELD_BIT(7),
   AGX_DBG_SYNC = BITFIELD_BIT(8),
   AGX_DBG_STATS = BITFIELD_BIT(9),
   AGX_DBG_RESOURCE = BITFIELD_BIT(10),
   AGX_DBG_BATCH = BITFIELD_BIT(11),
   AGX_DBG_NOWC = BITFIELD_BIT(12),
   AGX_DBG_SYNCTVB = BITFIELD_BIT(13),
   AGX_DBG_SMALLTILE = BITFIELD_BIT(14),
   AGX_DBG_NOMSAA = BITFIELD_BIT(15),
   AGX_DBG_NOSHADOW = BITFIELD_BIT(16),
   AGX_DBG_VARYINGS = BITFIELD_BIT(17),
};

/* Dummy partial declarations, pending real UAPI */
enum drm_asahi_cmd_type { DRM_ASAHI_CMD_TYPE_PLACEHOLDER_FOR_DOWNSTREAM_UAPI };
enum drm_asahi_sync_type { DRM_ASAHI_SYNC_SYNCOBJ };
struct drm_asahi_sync {
   uint32_t sync_type;
   uint32_t handle;
};
struct drm_asahi_params_global {
   uint64_t vm_page_size;
   uint64_t vm_user_start;
   uint64_t vm_user_end;
   uint64_t vm_shader_start;
   uint64_t vm_shader_end;
   uint32_t chip_id;
   uint32_t num_clusters_total;
   uint32_t gpu_generation;
   uint32_t gpu_variant;
   uint32_t num_dies;
   uint32_t timer_frequency_hz;
};

/* How many power-of-two levels in the BO cache do we want? 2^14 minimum chosen
 * as it is the page size that all allocations are rounded to
 */
#define MIN_BO_CACHE_BUCKET (14) /* 2^14 = 16KB */
#define MAX_BO_CACHE_BUCKET (22) /* 2^22 = 4MB */

/* Fencepost problem, hence the off-by-one */
#define NR_BO_CACHE_BUCKETS (MAX_BO_CACHE_BUCKET - MIN_BO_CACHE_BUCKET + 1)

/* Forward decl only, do not pull in all of NIR */
struct nir_shader;

struct agx_device {
   uint32_t debug;

   /* NIR library of AGX helpers/shaders. Immutable once created. */
   const struct nir_shader *libagx;

   char name[64];
   struct drm_asahi_params_global params;
   uint64_t next_global_id, last_global_id;

   /* Device handle */
   int fd;

   /* VM handle */
   uint32_t vm_id;

   /* VMA heaps */
   simple_mtx_t vma_lock;
   uint64_t shader_base;
   struct util_vma_heap main_heap;
   struct util_vma_heap usc_heap;
   uint64_t guard_size;

   struct renderonly *ro;

   pthread_mutex_t bo_map_lock;
   struct util_sparse_array bo_map;
   uint32_t max_handle;

   struct {
      simple_mtx_t lock;

      /* List containing all cached BOs sorted in LRU (Least Recently Used)
       * order so we can quickly evict BOs that are more than 1 second old.
       */
      struct list_head lru;

      /* The BO cache is a set of buckets with power-of-two sizes.  Each bucket
       * is a linked list of free panfrost_bo objects.
       */
      struct list_head buckets[NR_BO_CACHE_BUCKETS];

      /* Current size of the BO cache in bytes (sum of sizes of cached BOs) */
      size_t size;

      /* Number of hits/misses for the BO cache */
      uint64_t hits, misses;
   } bo_cache;
};

bool agx_open_device(void *memctx, struct agx_device *dev);

void agx_close_device(struct agx_device *dev);

static inline struct agx_bo *
agx_lookup_bo(struct agx_device *dev, uint32_t handle)
{
   return util_sparse_array_get(&dev->bo_map, handle);
}

void agx_bo_mmap(struct agx_bo *bo);

uint64_t agx_get_global_id(struct agx_device *dev);

uint32_t agx_create_command_queue(struct agx_device *dev, uint32_t caps);

int agx_import_sync_file(struct agx_device *dev, struct agx_bo *bo, int fd);
int agx_export_sync_file(struct agx_device *dev, struct agx_bo *bo);

void agx_debug_fault(struct agx_device *dev, uint64_t addr);

uint64_t agx_get_gpu_timestamp(struct agx_device *dev);

static inline uint64_t
agx_gpu_time_to_ns(struct agx_device *dev, uint64_t gpu_time)
{
   return (gpu_time * NSEC_PER_SEC) / dev->params.timer_frequency_hz;
}

#endif
