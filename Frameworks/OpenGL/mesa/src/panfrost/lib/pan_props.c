/*
 * Copyright (C) 2019 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include <xf86drm.h>

#include "drm-uapi/panfrost_drm.h"
#include "util/hash_table.h"
#include "util/macros.h"
#include "util/u_math.h"
#include "util/u_thread.h"
#include "pan_bo.h"
#include "pan_device.h"
#include "pan_encoder.h"
#include "pan_texture.h"
#include "pan_util.h"
#include "wrap.h"

/* Fixed "minimum revisions" */
#define NO_ANISO  (~0)
#define HAS_ANISO (0)

#define MODEL(gpu_id_, shortname, counters_, min_rev_anisotropic_, tib_size_,  \
              quirks_)                                                         \
   {                                                                           \
      .gpu_id = gpu_id_, .name = "Mali-" shortname " (Panfrost)",              \
      .performance_counters = counters_,                                       \
      .min_rev_anisotropic = min_rev_anisotropic_,                             \
      .tilebuffer_size = tib_size_, .quirks = quirks_,                         \
   }

/* Table of supported Mali GPUs */
/* clang-format off */
const struct panfrost_model panfrost_model_list[] = {
        MODEL(0x620, "T620",    "T62x", NO_ANISO,          8192, {}),
        MODEL(0x720, "T720",    "T72x", NO_ANISO,          8192, { .no_hierarchical_tiling = true }),
        MODEL(0x750, "T760",    "T76x", NO_ANISO,          8192, {}),
        MODEL(0x820, "T820",    "T82x", NO_ANISO,          8192, { .no_hierarchical_tiling = true }),
        MODEL(0x830, "T830",    "T83x", NO_ANISO,          8192, { .no_hierarchical_tiling = true }),
        MODEL(0x860, "T860",    "T86x", NO_ANISO,          8192, {}),
        MODEL(0x880, "T880",    "T88x", NO_ANISO,          8192, {}),

        MODEL(0x6000, "G71",    "TMIx", NO_ANISO,          8192, {}),
        MODEL(0x6221, "G72",    "THEx", 0x0030 /* r0p3 */, 16384, {}),
        MODEL(0x7090, "G51",    "TSIx", 0x1010 /* r1p1 */, 16384, {}),
        MODEL(0x7093, "G31",    "TDVx", HAS_ANISO,         16384, {}),
        MODEL(0x7211, "G76",    "TNOx", HAS_ANISO,         16384, {}),
        MODEL(0x7212, "G52",    "TGOx", HAS_ANISO,         16384, {}),
        MODEL(0x7402, "G52 r1", "TGOx", HAS_ANISO,         16384, {}),
        MODEL(0x9091, "G57",    "TNAx", HAS_ANISO,         16384, {}),
        MODEL(0x9093, "G57",    "TNAx", HAS_ANISO,         16384, {}),
};
/* clang-format on */

#undef NO_ANISO
#undef HAS_ANISO
#undef MODEL

/*
 * Look up a supported model by its GPU ID, or return NULL if the model is not
 * supported at this time.
 */
const struct panfrost_model *
panfrost_get_model(uint32_t gpu_id)
{
   for (unsigned i = 0; i < ARRAY_SIZE(panfrost_model_list); ++i) {
      if (panfrost_model_list[i].gpu_id == gpu_id)
         return &panfrost_model_list[i];
   }

   return NULL;
}

unsigned
panfrost_query_l2_slices(const struct panfrost_device *dev)
{
   /* L2_SLICES is MEM_FEATURES[11:8] minus(1) */
   return ((dev->kmod.props.mem_features >> 8) & 0xF) + 1;
}

static struct panfrost_tiler_features
panfrost_query_tiler_features(const struct panfrost_device *dev)
{
   /* Default value (2^9 bytes and 8 levels) to match old behaviour */
   uint32_t raw = dev->kmod.props.tiler_features;

   /* Bin size is log2 in the first byte, max levels in the second byte */
   return (struct panfrost_tiler_features){
      .bin_size = (1 << (raw & BITFIELD_MASK(5))),
      .max_levels = (raw >> 8) & BITFIELD_MASK(4),
   };
}

static unsigned
panfrost_query_core_count(const struct panfrost_device *dev,
                          unsigned *core_id_range)
{
   /* On older kernels, worst-case to 16 cores */

   unsigned mask = dev->kmod.props.shader_present;

   /* Some cores might be absent. In some cases, we care
    * about the range of core IDs (that is, the greatest core ID + 1). If
    * the core mask is contiguous, this equals the core count.
    */
   *core_id_range = util_last_bit(mask);

   /* The actual core count skips overs the gaps */
   return util_bitcount(mask);
}

static unsigned
panfrost_query_thread_tls_alloc(const struct panfrost_device *dev,
                                unsigned major)
{
   unsigned tls = dev->kmod.props.thread_tls_alloc;

   return (tls > 0) ? tls : panfrost_max_thread_count(major, 0);
}

static uint32_t
panfrost_query_compressed_formats(const struct panfrost_device *dev)
{
   return dev->kmod.props.texture_features[0];
}

/* DRM_PANFROST_PARAM_TEXTURE_FEATURES0 will return a bitmask of supported
 * compressed formats, so we offer a helper to test if a format is supported */

bool
panfrost_supports_compressed_format(struct panfrost_device *dev, unsigned fmt)
{
   if (MALI_EXTRACT_TYPE(fmt) != MALI_FORMAT_COMPRESSED)
      return true;

   unsigned idx = fmt & ~MALI_FORMAT_COMPRESSED;
   assert(idx < 32);

   return dev->compressed_formats & (1 << idx);
}

/* Check for AFBC hardware support. AFBC is introduced in v5. Implementations
 * may omit it, signaled as a nonzero value in the AFBC_FEATURES property. */

static bool
panfrost_query_afbc(struct panfrost_device *dev, unsigned arch)
{
   unsigned reg = dev->kmod.props.afbc_features;

   return (arch >= 5) && (reg == 0);
}

/*
 * To pipeline multiple tiles, a given tile may use at most half of the tile
 * buffer. This function returns the optimal size (assuming pipelining).
 *
 * For Mali-G510 and Mali-G310, we will need extra logic to query the tilebuffer
 * size for the particular variant. The CORE_FEATURES register might help.
 */
static unsigned
panfrost_query_optimal_tib_size(const struct panfrost_device *dev)
{
   /* Preconditions ensure the returned value is a multiple of 1 KiB, the
    * granularity of the colour buffer allocation field.
    */
   assert(dev->model->tilebuffer_size >= 2048);
   assert(util_is_power_of_two_nonzero(dev->model->tilebuffer_size));

   return dev->model->tilebuffer_size / 2;
}

static uint64_t
panfrost_clamp_to_usable_va_range(const struct panfrost_device *dev,
                                  uint64_t va)
{
   struct pan_kmod_va_range user_va_range =
      pan_kmod_dev_query_user_va_range(dev->kmod.dev);

   if (va < user_va_range.start)
      return user_va_range.start;
   else if (va > user_va_range.start + user_va_range.size)
      return user_va_range.start + user_va_range.size;

   return va;
}

/* Always reserve the lower 32MB. */
#define PANFROST_VA_RESERVE_BOTTOM 0x2000000ull

void
panfrost_open_device(void *memctx, int fd, struct panfrost_device *dev)
{
   dev->memctx = memctx;

   dev->kmod.dev = pan_kmod_dev_create(fd, PAN_KMOD_DEV_FLAG_OWNS_FD, NULL);
   if (!dev->kmod.dev) {
      close(fd);
      return;
   }

   pan_kmod_dev_query_props(dev->kmod.dev, &dev->kmod.props);

   dev->arch = pan_arch(dev->kmod.props.gpu_prod_id);
   dev->model = panfrost_get_model(dev->kmod.props.gpu_prod_id);

   /* If we don't recognize the model, bail early */
   if (!dev->model)
      goto err_free_kmod_dev;

   /* 32bit address space, with the lower 32MB reserved. We clamp
    * things so it matches kmod VA range limitations.
    */
   uint64_t user_va_start =
      panfrost_clamp_to_usable_va_range(dev, PANFROST_VA_RESERVE_BOTTOM);
   uint64_t user_va_end =
      panfrost_clamp_to_usable_va_range(dev, 1ull << 32);

   dev->kmod.vm =
      pan_kmod_vm_create(dev->kmod.dev, PAN_KMOD_VM_FLAG_AUTO_VA, user_va_start,
                         user_va_end - user_va_start);
   if (!dev->kmod.vm)
      goto err_free_kmod_dev;

   dev->core_count = panfrost_query_core_count(dev, &dev->core_id_range);
   dev->thread_tls_alloc = panfrost_query_thread_tls_alloc(dev, dev->arch);
   dev->optimal_tib_size = panfrost_query_optimal_tib_size(dev);
   dev->compressed_formats = panfrost_query_compressed_formats(dev);
   dev->tiler_features = panfrost_query_tiler_features(dev);
   dev->has_afbc = panfrost_query_afbc(dev, dev->arch);

   if (dev->arch <= 6) {
      dev->formats = panfrost_pipe_format_v6;
      dev->blendable_formats = panfrost_blendable_formats_v6;
   } else if (dev->arch <= 7) {
      dev->formats = panfrost_pipe_format_v7;
      dev->blendable_formats = panfrost_blendable_formats_v7;
   } else {
      dev->formats = panfrost_pipe_format_v9;
      dev->blendable_formats = panfrost_blendable_formats_v9;
   }

   util_sparse_array_init(&dev->bo_map, sizeof(struct panfrost_bo), 512);

   pthread_mutex_init(&dev->bo_cache.lock, NULL);
   list_inithead(&dev->bo_cache.lru);

   for (unsigned i = 0; i < ARRAY_SIZE(dev->bo_cache.buckets); ++i)
      list_inithead(&dev->bo_cache.buckets[i]);

   /* Initialize pandecode before we start allocating */
   if (dev->debug & (PAN_DBG_TRACE | PAN_DBG_SYNC))
      dev->decode_ctx = pandecode_create_context(!(dev->debug & PAN_DBG_TRACE));

   /* Tiler heap is internally required by the tiler, which can only be
    * active for a single job chain at once, so a single heap can be
    * shared across batches/contextes */

   dev->tiler_heap = panfrost_bo_create(
      dev, 128 * 1024 * 1024, PAN_BO_INVISIBLE | PAN_BO_GROWABLE, "Tiler heap");

   pthread_mutex_init(&dev->submit_lock, NULL);

   /* Done once on init */
   panfrost_upload_sample_positions(dev);
   return;

err_free_kmod_dev:
   pan_kmod_dev_destroy(dev->kmod.dev);
   dev->kmod.dev = NULL;
}

void
panfrost_close_device(struct panfrost_device *dev)
{
   /* If we don't recognize the model, the rest of the device won't exist,
    * we will have early-exited the device open.
    */
   if (dev->model) {
      pthread_mutex_destroy(&dev->submit_lock);
      panfrost_bo_unreference(dev->tiler_heap);
      panfrost_bo_unreference(dev->sample_positions);
      panfrost_bo_cache_evict_all(dev);
      pthread_mutex_destroy(&dev->bo_cache.lock);
      util_sparse_array_finish(&dev->bo_map);
   }

   if (dev->kmod.vm)
      pan_kmod_vm_destroy(dev->kmod.vm);

   if (dev->kmod.dev)
      pan_kmod_dev_destroy(dev->kmod.dev);
}
