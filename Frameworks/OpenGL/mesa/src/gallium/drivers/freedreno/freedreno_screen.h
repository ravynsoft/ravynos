/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FREEDRENO_SCREEN_H_
#define FREEDRENO_SCREEN_H_

#include "common/freedreno_dev_info.h"
#include "drm/freedreno_drmif.h"
#include "drm/freedreno_ringbuffer.h"
#include "perfcntrs/freedreno_perfcntr.h"

#include "pipe/p_screen.h"
#include "renderonly/renderonly.h"
#include "util/u_debug.h"
#include "util/simple_mtx.h"
#include "util/slab.h"
#include "util/u_idalloc.h"
#include "util/u_memory.h"
#include "util/u_queue.h"

#include "freedreno_batch_cache.h"
#include "freedreno_gmem.h"
#include "freedreno_util.h"

struct fd_bo;

/* Potential reasons for needing to skip bypass path and use GMEM, the
 * generation backend can override this with screen->gmem_reason_mask
 */
enum fd_gmem_reason {
   FD_GMEM_CLEARS_DEPTH_STENCIL = BIT(0),
   FD_GMEM_DEPTH_ENABLED = BIT(1),
   FD_GMEM_STENCIL_ENABLED = BIT(2),
   FD_GMEM_BLEND_ENABLED = BIT(3),
   FD_GMEM_LOGICOP_ENABLED = BIT(4),
   FD_GMEM_FB_READ = BIT(5),
};

struct fd_screen {
   struct pipe_screen base;

   struct list_head context_list;

   simple_mtx_t lock;

   struct slab_parent_pool transfer_pool;

   uint64_t gmem_base;
   uint32_t gmemsize_bytes;

   const struct fd_dev_id *dev_id;
   uint8_t gen;      /* GPU (major) generation */
   uint32_t gpu_id;  /* 220, 305, etc */
   uint64_t chip_id; /* coreid:8 majorrev:8 minorrev:8 patch:8 */
   uint32_t max_freq;
   uint32_t ram_size;
   uint32_t max_rts; /* max # of render targets */
   uint32_t priority_mask;
   unsigned prio_low, prio_norm, prio_high;  /* remap low/norm/high priority to kernel priority */
   bool has_timestamp;
   bool has_robustness;
   bool has_syncobj;

   struct {
      /* Conservative LRZ (default true) invalidates LRZ on draws with
       * blend and depth-write enabled, because this can lead to incorrect
       * rendering.  Driconf can be used to disable conservative LRZ for
       * games which do not have the problematic sequence of draws *and*
       * suffer a performance loss with conservative LRZ.
       */
      bool conservative_lrz;

      /* Enable EGL throttling (default true).
       */
      bool enable_throttling;
   } driconf;

   struct fd_dev_info dev_info;
   const struct fd_dev_info *info;
   uint32_t ccu_offset_gmem;
   uint32_t ccu_offset_bypass;

   /* Bitmask of gmem_reasons that do not force GMEM path over bypass
    * for current generation.
    */
   enum fd_gmem_reason gmem_reason_mask;

   unsigned num_perfcntr_groups;
   const struct fd_perfcntr_group *perfcntr_groups;

   /* generated at startup from the perfcntr groups: */
   unsigned num_perfcntr_queries;
   struct pipe_driver_query_info *perfcntr_queries;

   void *compiler;                  /* currently unused for a2xx */
   struct util_queue compile_queue; /* currently unused for a2xx */

   struct fd_device *dev;

   /* NOTE: we still need a pipe associated with the screen in a few
    * places, like screen->get_timestamp().  For anything context
    * related, use ctx->pipe instead.
    */
   struct fd_pipe *pipe;

   uint32_t (*setup_slices)(struct fd_resource *rsc);
   unsigned (*tile_mode)(const struct pipe_resource *prsc);
   int (*layout_resource_for_modifier)(struct fd_resource *rsc,
                                       uint64_t modifier);
   bool (*is_format_supported)(struct pipe_screen *pscreen,
                               enum pipe_format fmt, uint64_t modifier);

   /* indirect-branch emit: */
   void (*emit_ib)(struct fd_ringbuffer *ring, struct fd_ringbuffer *target);

   /* simple gpu "memcpy": */
   void (*mem_to_mem)(struct fd_ringbuffer *ring, struct pipe_resource *dst,
                      unsigned dst_off, struct pipe_resource *src,
                      unsigned src_off, unsigned sizedwords);

   int64_t cpu_gpu_time_delta;

   struct fd_batch_cache batch_cache;
   struct fd_gmem_cache gmem_cache;

   bool reorder;

   seqno_t rsc_seqno;
   seqno_t ctx_seqno;
   struct util_idalloc_mt buffer_ids;

   unsigned num_supported_modifiers;
   const uint64_t *supported_modifiers;

   struct renderonly *ro;

   /* the blob seems to always use 8K factor and 128K param sizes, copy them */
#define FD6_TESS_FACTOR_SIZE (8 * 1024)
#define FD6_TESS_PARAM_SIZE (128 * 1024)
#define FD6_TESS_BO_SIZE (FD6_TESS_FACTOR_SIZE + FD6_TESS_PARAM_SIZE)
   struct fd_bo *tess_bo;

   /* table with MESA_PRIM_COUNT+1 entries mapping MESA_PRIM_x to
    * DI_PT_x value to use for draw initiator.  There are some
    * slight differences between generation.
    *
    * Note that primtypes[PRIM_TYPE_MAX] is used to map to the
    * internal RECTLIST primtype, if available, used for blits/
    * clears.
    */
   const enum pc_di_primtype *primtypes;
   uint32_t primtypes_mask;

   simple_mtx_t aux_ctx_lock;
   struct pipe_context *aux_ctx;
};

static inline struct fd_screen *
fd_screen(struct pipe_screen *pscreen)
{
   return (struct fd_screen *)pscreen;
}

struct fd_context;
struct fd_context * fd_screen_aux_context_get(struct pipe_screen *pscreen);
void fd_screen_aux_context_put(struct pipe_screen *pscreen);

static inline void
fd_screen_lock(struct fd_screen *screen)
{
   simple_mtx_lock(&screen->lock);
}

static inline void
fd_screen_unlock(struct fd_screen *screen)
{
   simple_mtx_unlock(&screen->lock);
}

static inline void
fd_screen_assert_locked(struct fd_screen *screen)
{
   simple_mtx_assert_locked(&screen->lock);
}

bool fd_screen_bo_get_handle(struct pipe_screen *pscreen, struct fd_bo *bo,
                             struct renderonly_scanout *scanout,
                             unsigned stride, struct winsys_handle *whandle);
struct fd_bo *fd_screen_bo_from_handle(struct pipe_screen *pscreen,
                                       struct winsys_handle *whandle);

struct pipe_screen *fd_screen_create(int fd,
                                     const struct pipe_screen_config *config,
                                     struct renderonly *ro);

static inline bool
is_a20x(struct fd_screen *screen)
{
   return (screen->gpu_id >= 200) && (screen->gpu_id < 210);
}

static inline bool
is_a2xx(struct fd_screen *screen)
{
   return screen->gen == 2;
}

/* is a3xx patch revision 0? */
/* TODO a306.0 probably doesn't need this.. be more clever?? */
static inline bool
is_a3xx_p0(struct fd_screen *screen)
{
   return (screen->chip_id & 0xff0000ff) == 0x03000000;
}

static inline bool
is_a3xx(struct fd_screen *screen)
{
   return screen->gen == 3;
}

static inline bool
is_a4xx(struct fd_screen *screen)
{
   return screen->gen == 4;
}

static inline bool
is_a5xx(struct fd_screen *screen)
{
   return screen->gen == 5;
}

static inline bool
is_a6xx(struct fd_screen *screen)
{
   return screen->gen == 6;
}

/* is it using the ir3 compiler (shader isa introduced with a3xx)? */
static inline bool
is_ir3(struct fd_screen *screen)
{
   return is_a3xx(screen) || is_a4xx(screen) || is_a5xx(screen) ||
          is_a6xx(screen);
}

static inline bool
has_compute(struct fd_screen *screen)
{
   return is_a4xx(screen) || is_a5xx(screen) || is_a6xx(screen);
}

#endif /* FREEDRENO_SCREEN_H_ */
