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
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iris_state.c
 *
 * ============================= GENXML CODE =============================
 *              [This file is compiled once per generation.]
 * =======================================================================
 *
 * This is the main state upload code.
 *
 * Gallium uses Constant State Objects, or CSOs, for most state.  Large,
 * complex, or highly reusable state can be created once, and bound and
 * rebound multiple times.  This is modeled with the pipe->create_*_state()
 * and pipe->bind_*_state() hooks.  Highly dynamic or inexpensive state is
 * streamed out on the fly, via pipe->set_*_state() hooks.
 *
 * OpenGL involves frequently mutating context state, which is mirrored in
 * core Mesa by highly mutable data structures.  However, most applications
 * typically draw the same things over and over - from frame to frame, most
 * of the same objects are still visible and need to be redrawn.  So, rather
 * than inventing new state all the time, applications usually mutate to swap
 * between known states that we've seen before.
 *
 * Gallium isolates us from this mutation by tracking API state, and
 * distilling it into a set of Constant State Objects, or CSOs.  Large,
 * complex, or typically reusable state can be created once, then reused
 * multiple times.  Drivers can create and store their own associated data.
 * This create/bind model corresponds to the pipe->create_*_state() and
 * pipe->bind_*_state() driver hooks.
 *
 * Some state is cheap to create, or expected to be highly dynamic.  Rather
 * than creating and caching piles of CSOs for these, Gallium simply streams
 * them out, via the pipe->set_*_state() driver hooks.
 *
 * To reduce draw time overhead, we try to compute as much state at create
 * time as possible.  Wherever possible, we translate the Gallium pipe state
 * to 3DSTATE commands, and store those commands in the CSO.  At draw time,
 * we can simply memcpy them into a batch buffer.
 *
 * No hardware matches the abstraction perfectly, so some commands require
 * information from multiple CSOs.  In this case, we can store two copies
 * of the packet (one in each CSO), and simply | together their DWords at
 * draw time.  Sometimes the second set is trivial (one or two fields), so
 * we simply pack it at draw time.
 *
 * There are two main components in the file below.  First, the CSO hooks
 * create/bind/track state.  The second are the draw-time upload functions,
 * iris_upload_render_state() and iris_upload_compute_state(), which read
 * the context state and emit the commands into the actual batch.
 */

#include <stdio.h>
#include <errno.h>

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#define VG(x) x
#else
#define VG(x)
#endif

#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_dual_blend.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "util/u_framebuffer.h"
#include "util/u_transfer.h"
#include "util/u_upload_mgr.h"
#include "util/u_viewport.h"
#include "util/u_memory.h"
#include "util/u_trace_gallium.h"
#include "drm-uapi/i915_drm.h"
#include "nir.h"
#include "intel/compiler/brw_compiler.h"
#include "intel/common/intel_aux_map.h"
#include "intel/common/intel_l3_config.h"
#include "intel/common/intel_sample_positions.h"
#include "intel/ds/intel_tracepoints.h"
#include "iris_batch.h"
#include "iris_context.h"
#include "iris_defines.h"
#include "iris_pipe.h"
#include "iris_resource.h"
#include "iris_utrace.h"

#include "iris_genx_macros.h"
#include "intel/common/intel_genX_state.h"
#include "intel/common/intel_guardband.h"
#include "intel/common/intel_pixel_hash.h"
#include "intel/common/intel_tiled_render.h"

/**
 * Statically assert that PIPE_* enums match the hardware packets.
 * (As long as they match, we don't need to translate them.)
 */
UNUSED static void pipe_asserts()
{
#define PIPE_ASSERT(x) STATIC_ASSERT((int)x)

   /* pipe_logicop happens to match the hardware. */
   PIPE_ASSERT(PIPE_LOGICOP_CLEAR == LOGICOP_CLEAR);
   PIPE_ASSERT(PIPE_LOGICOP_NOR == LOGICOP_NOR);
   PIPE_ASSERT(PIPE_LOGICOP_AND_INVERTED == LOGICOP_AND_INVERTED);
   PIPE_ASSERT(PIPE_LOGICOP_COPY_INVERTED == LOGICOP_COPY_INVERTED);
   PIPE_ASSERT(PIPE_LOGICOP_AND_REVERSE == LOGICOP_AND_REVERSE);
   PIPE_ASSERT(PIPE_LOGICOP_INVERT == LOGICOP_INVERT);
   PIPE_ASSERT(PIPE_LOGICOP_XOR == LOGICOP_XOR);
   PIPE_ASSERT(PIPE_LOGICOP_NAND == LOGICOP_NAND);
   PIPE_ASSERT(PIPE_LOGICOP_AND == LOGICOP_AND);
   PIPE_ASSERT(PIPE_LOGICOP_EQUIV == LOGICOP_EQUIV);
   PIPE_ASSERT(PIPE_LOGICOP_NOOP == LOGICOP_NOOP);
   PIPE_ASSERT(PIPE_LOGICOP_OR_INVERTED == LOGICOP_OR_INVERTED);
   PIPE_ASSERT(PIPE_LOGICOP_COPY == LOGICOP_COPY);
   PIPE_ASSERT(PIPE_LOGICOP_OR_REVERSE == LOGICOP_OR_REVERSE);
   PIPE_ASSERT(PIPE_LOGICOP_OR == LOGICOP_OR);
   PIPE_ASSERT(PIPE_LOGICOP_SET == LOGICOP_SET);

   /* pipe_blend_func happens to match the hardware. */
   PIPE_ASSERT(PIPE_BLENDFACTOR_ONE == BLENDFACTOR_ONE);
   PIPE_ASSERT(PIPE_BLENDFACTOR_SRC_COLOR == BLENDFACTOR_SRC_COLOR);
   PIPE_ASSERT(PIPE_BLENDFACTOR_SRC_ALPHA == BLENDFACTOR_SRC_ALPHA);
   PIPE_ASSERT(PIPE_BLENDFACTOR_DST_ALPHA == BLENDFACTOR_DST_ALPHA);
   PIPE_ASSERT(PIPE_BLENDFACTOR_DST_COLOR == BLENDFACTOR_DST_COLOR);
   PIPE_ASSERT(PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE == BLENDFACTOR_SRC_ALPHA_SATURATE);
   PIPE_ASSERT(PIPE_BLENDFACTOR_CONST_COLOR == BLENDFACTOR_CONST_COLOR);
   PIPE_ASSERT(PIPE_BLENDFACTOR_CONST_ALPHA == BLENDFACTOR_CONST_ALPHA);
   PIPE_ASSERT(PIPE_BLENDFACTOR_SRC1_COLOR == BLENDFACTOR_SRC1_COLOR);
   PIPE_ASSERT(PIPE_BLENDFACTOR_SRC1_ALPHA == BLENDFACTOR_SRC1_ALPHA);
   PIPE_ASSERT(PIPE_BLENDFACTOR_ZERO == BLENDFACTOR_ZERO);
   PIPE_ASSERT(PIPE_BLENDFACTOR_INV_SRC_COLOR == BLENDFACTOR_INV_SRC_COLOR);
   PIPE_ASSERT(PIPE_BLENDFACTOR_INV_SRC_ALPHA == BLENDFACTOR_INV_SRC_ALPHA);
   PIPE_ASSERT(PIPE_BLENDFACTOR_INV_DST_ALPHA == BLENDFACTOR_INV_DST_ALPHA);
   PIPE_ASSERT(PIPE_BLENDFACTOR_INV_DST_COLOR == BLENDFACTOR_INV_DST_COLOR);
   PIPE_ASSERT(PIPE_BLENDFACTOR_INV_CONST_COLOR == BLENDFACTOR_INV_CONST_COLOR);
   PIPE_ASSERT(PIPE_BLENDFACTOR_INV_CONST_ALPHA == BLENDFACTOR_INV_CONST_ALPHA);
   PIPE_ASSERT(PIPE_BLENDFACTOR_INV_SRC1_COLOR == BLENDFACTOR_INV_SRC1_COLOR);
   PIPE_ASSERT(PIPE_BLENDFACTOR_INV_SRC1_ALPHA == BLENDFACTOR_INV_SRC1_ALPHA);

   /* pipe_blend_func happens to match the hardware. */
   PIPE_ASSERT(PIPE_BLEND_ADD == BLENDFUNCTION_ADD);
   PIPE_ASSERT(PIPE_BLEND_SUBTRACT == BLENDFUNCTION_SUBTRACT);
   PIPE_ASSERT(PIPE_BLEND_REVERSE_SUBTRACT == BLENDFUNCTION_REVERSE_SUBTRACT);
   PIPE_ASSERT(PIPE_BLEND_MIN == BLENDFUNCTION_MIN);
   PIPE_ASSERT(PIPE_BLEND_MAX == BLENDFUNCTION_MAX);

   /* pipe_stencil_op happens to match the hardware. */
   PIPE_ASSERT(PIPE_STENCIL_OP_KEEP == STENCILOP_KEEP);
   PIPE_ASSERT(PIPE_STENCIL_OP_ZERO == STENCILOP_ZERO);
   PIPE_ASSERT(PIPE_STENCIL_OP_REPLACE == STENCILOP_REPLACE);
   PIPE_ASSERT(PIPE_STENCIL_OP_INCR == STENCILOP_INCRSAT);
   PIPE_ASSERT(PIPE_STENCIL_OP_DECR == STENCILOP_DECRSAT);
   PIPE_ASSERT(PIPE_STENCIL_OP_INCR_WRAP == STENCILOP_INCR);
   PIPE_ASSERT(PIPE_STENCIL_OP_DECR_WRAP == STENCILOP_DECR);
   PIPE_ASSERT(PIPE_STENCIL_OP_INVERT == STENCILOP_INVERT);

   /* pipe_sprite_coord_mode happens to match 3DSTATE_SBE */
   PIPE_ASSERT(PIPE_SPRITE_COORD_UPPER_LEFT == UPPERLEFT);
   PIPE_ASSERT(PIPE_SPRITE_COORD_LOWER_LEFT == LOWERLEFT);
#undef PIPE_ASSERT
}

static unsigned
translate_prim_type(enum mesa_prim prim, uint8_t verts_per_patch)
{
   static const unsigned map[] = {
      [MESA_PRIM_POINTS]                   = _3DPRIM_POINTLIST,
      [MESA_PRIM_LINES]                    = _3DPRIM_LINELIST,
      [MESA_PRIM_LINE_LOOP]                = _3DPRIM_LINELOOP,
      [MESA_PRIM_LINE_STRIP]               = _3DPRIM_LINESTRIP,
      [MESA_PRIM_TRIANGLES]                = _3DPRIM_TRILIST,
      [MESA_PRIM_TRIANGLE_STRIP]           = _3DPRIM_TRISTRIP,
      [MESA_PRIM_TRIANGLE_FAN]             = _3DPRIM_TRIFAN,
      [MESA_PRIM_QUADS]                    = _3DPRIM_QUADLIST,
      [MESA_PRIM_QUAD_STRIP]               = _3DPRIM_QUADSTRIP,
      [MESA_PRIM_POLYGON]                  = _3DPRIM_POLYGON,
      [MESA_PRIM_LINES_ADJACENCY]          = _3DPRIM_LINELIST_ADJ,
      [MESA_PRIM_LINE_STRIP_ADJACENCY]     = _3DPRIM_LINESTRIP_ADJ,
      [MESA_PRIM_TRIANGLES_ADJACENCY]      = _3DPRIM_TRILIST_ADJ,
      [MESA_PRIM_TRIANGLE_STRIP_ADJACENCY] = _3DPRIM_TRISTRIP_ADJ,
      [MESA_PRIM_PATCHES]                  = _3DPRIM_PATCHLIST_1 - 1,
   };

   return map[prim] + (prim == MESA_PRIM_PATCHES ? verts_per_patch : 0);
}

static unsigned
translate_compare_func(enum pipe_compare_func pipe_func)
{
   static const unsigned map[] = {
      [PIPE_FUNC_NEVER]    = COMPAREFUNCTION_NEVER,
      [PIPE_FUNC_LESS]     = COMPAREFUNCTION_LESS,
      [PIPE_FUNC_EQUAL]    = COMPAREFUNCTION_EQUAL,
      [PIPE_FUNC_LEQUAL]   = COMPAREFUNCTION_LEQUAL,
      [PIPE_FUNC_GREATER]  = COMPAREFUNCTION_GREATER,
      [PIPE_FUNC_NOTEQUAL] = COMPAREFUNCTION_NOTEQUAL,
      [PIPE_FUNC_GEQUAL]   = COMPAREFUNCTION_GEQUAL,
      [PIPE_FUNC_ALWAYS]   = COMPAREFUNCTION_ALWAYS,
   };
   return map[pipe_func];
}

static unsigned
translate_shadow_func(enum pipe_compare_func pipe_func)
{
   /* Gallium specifies the result of shadow comparisons as:
    *
    *    1 if ref <op> texel,
    *    0 otherwise.
    *
    * The hardware does:
    *
    *    0 if texel <op> ref,
    *    1 otherwise.
    *
    * So we need to flip the operator and also negate.
    */
   static const unsigned map[] = {
      [PIPE_FUNC_NEVER]    = PREFILTEROP_ALWAYS,
      [PIPE_FUNC_LESS]     = PREFILTEROP_LEQUAL,
      [PIPE_FUNC_EQUAL]    = PREFILTEROP_NOTEQUAL,
      [PIPE_FUNC_LEQUAL]   = PREFILTEROP_LESS,
      [PIPE_FUNC_GREATER]  = PREFILTEROP_GEQUAL,
      [PIPE_FUNC_NOTEQUAL] = PREFILTEROP_EQUAL,
      [PIPE_FUNC_GEQUAL]   = PREFILTEROP_GREATER,
      [PIPE_FUNC_ALWAYS]   = PREFILTEROP_NEVER,
   };
   return map[pipe_func];
}

static unsigned
translate_cull_mode(unsigned pipe_face)
{
   static const unsigned map[4] = {
      [PIPE_FACE_NONE]           = CULLMODE_NONE,
      [PIPE_FACE_FRONT]          = CULLMODE_FRONT,
      [PIPE_FACE_BACK]           = CULLMODE_BACK,
      [PIPE_FACE_FRONT_AND_BACK] = CULLMODE_BOTH,
   };
   return map[pipe_face];
}

static unsigned
translate_fill_mode(unsigned pipe_polymode)
{
   static const unsigned map[4] = {
      [PIPE_POLYGON_MODE_FILL]           = FILL_MODE_SOLID,
      [PIPE_POLYGON_MODE_LINE]           = FILL_MODE_WIREFRAME,
      [PIPE_POLYGON_MODE_POINT]          = FILL_MODE_POINT,
      [PIPE_POLYGON_MODE_FILL_RECTANGLE] = FILL_MODE_SOLID,
   };
   return map[pipe_polymode];
}

static unsigned
translate_mip_filter(enum pipe_tex_mipfilter pipe_mip)
{
   static const unsigned map[] = {
      [PIPE_TEX_MIPFILTER_NEAREST] = MIPFILTER_NEAREST,
      [PIPE_TEX_MIPFILTER_LINEAR]  = MIPFILTER_LINEAR,
      [PIPE_TEX_MIPFILTER_NONE]    = MIPFILTER_NONE,
   };
   return map[pipe_mip];
}

static uint32_t
translate_wrap(unsigned pipe_wrap)
{
   static const unsigned map[] = {
      [PIPE_TEX_WRAP_REPEAT]                 = TCM_WRAP,
      [PIPE_TEX_WRAP_CLAMP]                  = TCM_HALF_BORDER,
      [PIPE_TEX_WRAP_CLAMP_TO_EDGE]          = TCM_CLAMP,
      [PIPE_TEX_WRAP_CLAMP_TO_BORDER]        = TCM_CLAMP_BORDER,
      [PIPE_TEX_WRAP_MIRROR_REPEAT]          = TCM_MIRROR,
      [PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE]   = TCM_MIRROR_ONCE,

      /* These are unsupported. */
      [PIPE_TEX_WRAP_MIRROR_CLAMP]           = -1,
      [PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER] = -1,
   };
   return map[pipe_wrap];
}

/**
 * Allocate space for some indirect state.
 *
 * Return a pointer to the map (to fill it out) and a state ref (for
 * referring to the state in GPU commands).
 */
static void *
upload_state(struct u_upload_mgr *uploader,
             struct iris_state_ref *ref,
             unsigned size,
             unsigned alignment)
{
   void *p = NULL;
   u_upload_alloc(uploader, 0, size, alignment, &ref->offset, &ref->res, &p);
   return p;
}

/**
 * Stream out temporary/short-lived state.
 *
 * This allocates space, pins the BO, and includes the BO address in the
 * returned offset (which works because all state lives in 32-bit memory
 * zones).
 */
static uint32_t *
stream_state(struct iris_batch *batch,
             struct u_upload_mgr *uploader,
             struct pipe_resource **out_res,
             unsigned size,
             unsigned alignment,
             uint32_t *out_offset)
{
   void *ptr = NULL;

   u_upload_alloc(uploader, 0, size, alignment, out_offset, out_res, &ptr);

   struct iris_bo *bo = iris_resource_bo(*out_res);
   iris_use_pinned_bo(batch, bo, false, IRIS_DOMAIN_NONE);

   iris_record_state_size(batch->state_sizes,
                          bo->address + *out_offset, size);

   *out_offset += iris_bo_offset_from_base_address(bo);

   return ptr;
}

/**
 * stream_state() + memcpy.
 */
static uint32_t
emit_state(struct iris_batch *batch,
           struct u_upload_mgr *uploader,
           struct pipe_resource **out_res,
           const void *data,
           unsigned size,
           unsigned alignment)
{
   unsigned offset = 0;
   uint32_t *map =
      stream_state(batch, uploader, out_res, size, alignment, &offset);

   if (map)
      memcpy(map, data, size);

   return offset;
}

/**
 * Did field 'x' change between 'old_cso' and 'new_cso'?
 *
 * (If so, we may want to set some dirty flags.)
 */
#define cso_changed(x) (!old_cso || (old_cso->x != new_cso->x))
#define cso_changed_memcmp(x) \
   (!old_cso || memcmp(old_cso->x, new_cso->x, sizeof(old_cso->x)) != 0)
#define cso_changed_memcmp_elts(x, n) \
   (!old_cso || memcmp(old_cso->x, new_cso->x, n * sizeof(old_cso->x[0])) != 0)

static void
flush_before_state_base_change(struct iris_batch *batch)
{
   /* Wa_14014427904 - We need additional invalidate/flush when
    * emitting NP state commands with ATS-M in compute mode.
    */
   bool atsm_compute = intel_device_info_is_atsm(batch->screen->devinfo) &&
                       batch->name == IRIS_BATCH_COMPUTE;
   uint32_t np_state_wa_bits =
      PIPE_CONTROL_CS_STALL |
      PIPE_CONTROL_STATE_CACHE_INVALIDATE |
      PIPE_CONTROL_CONST_CACHE_INVALIDATE |
      PIPE_CONTROL_UNTYPED_DATAPORT_CACHE_FLUSH |
      PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
      PIPE_CONTROL_INSTRUCTION_INVALIDATE |
      PIPE_CONTROL_FLUSH_HDC;

   /* Flush before emitting STATE_BASE_ADDRESS.
    *
    * This isn't documented anywhere in the PRM.  However, it seems to be
    * necessary prior to changing the surface state base address.  We've
    * seen issues in Vulkan where we get GPU hangs when using multi-level
    * command buffers which clear depth, reset state base address, and then
    * go render stuff.
    *
    * Normally, in GL, we would trust the kernel to do sufficient stalls
    * and flushes prior to executing our batch.  However, it doesn't seem
    * as if the kernel's flushing is always sufficient and we don't want to
    * rely on it.
    *
    * We make this an end-of-pipe sync instead of a normal flush because we
    * do not know the current status of the GPU.  On Haswell at least,
    * having a fast-clear operation in flight at the same time as a normal
    * rendering operation can cause hangs.  Since the kernel's flushing is
    * insufficient, we need to ensure that any rendering operations from
    * other processes are definitely complete before we try to do our own
    * rendering.  It's a bit of a big hammer but it appears to work.
    */
   iris_emit_end_of_pipe_sync(batch,
                              "change STATE_BASE_ADDRESS (flushes)",
                              atsm_compute ? np_state_wa_bits : 0 |
                              PIPE_CONTROL_RENDER_TARGET_FLUSH |
                              PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                              PIPE_CONTROL_DATA_CACHE_FLUSH);
}

static void
flush_after_state_base_change(struct iris_batch *batch)
{
   const struct intel_device_info *devinfo = batch->screen->devinfo;
   /* After re-setting the surface state base address, we have to do some
    * cache flusing so that the sampler engine will pick up the new
    * SURFACE_STATE objects and binding tables. From the Broadwell PRM,
    * Shared Function > 3D Sampler > State > State Caching (page 96):
    *
    *    Coherency with system memory in the state cache, like the texture
    *    cache is handled partially by software. It is expected that the
    *    command stream or shader will issue Cache Flush operation or
    *    Cache_Flush sampler message to ensure that the L1 cache remains
    *    coherent with system memory.
    *
    *    [...]
    *
    *    Whenever the value of the Dynamic_State_Base_Addr,
    *    Surface_State_Base_Addr are altered, the L1 state cache must be
    *    invalidated to ensure the new surface or sampler state is fetched
    *    from system memory.
    *
    * The PIPE_CONTROL command has a "State Cache Invalidation Enable" bit
    * which, according the PIPE_CONTROL instruction documentation in the
    * Broadwell PRM:
    *
    *    Setting this bit is independent of any other bit in this packet.
    *    This bit controls the invalidation of the L1 and L2 state caches
    *    at the top of the pipe i.e. at the parsing time.
    *
    * Unfortunately, experimentation seems to indicate that state cache
    * invalidation through a PIPE_CONTROL does nothing whatsoever in
    * regards to surface state and binding tables.  In stead, it seems that
    * invalidating the texture cache is what is actually needed.
    *
    * XXX:  As far as we have been able to determine through
    * experimentation, shows that flush the texture cache appears to be
    * sufficient.  The theory here is that all of the sampling/rendering
    * units cache the binding table in the texture cache.  However, we have
    * yet to be able to actually confirm this.
    *
    * Wa_16013000631:
    *
    *  "DG2 128/256/512-A/B: S/W must program STATE_BASE_ADDRESS command twice
    *   or program pipe control with Instruction cache invalidate post
    *   STATE_BASE_ADDRESS command"
    */
   iris_emit_end_of_pipe_sync(batch,
                              "change STATE_BASE_ADDRESS (invalidates)",
                              PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
                              PIPE_CONTROL_CONST_CACHE_INVALIDATE |
                              PIPE_CONTROL_STATE_CACHE_INVALIDATE |
                              (intel_needs_workaround(devinfo, 16013000631) ?
                               PIPE_CONTROL_INSTRUCTION_INVALIDATE : 0));
}

static void
iris_load_register_reg32(struct iris_batch *batch, uint32_t dst,
                         uint32_t src)
{
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   mi_store(&b, mi_reg32(dst), mi_reg32(src));
}

static void
iris_load_register_reg64(struct iris_batch *batch, uint32_t dst,
                         uint32_t src)
{
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   mi_store(&b, mi_reg64(dst), mi_reg64(src));
}

static void
iris_load_register_imm32(struct iris_batch *batch, uint32_t reg,
                         uint32_t val)
{
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   mi_store(&b, mi_reg32(reg), mi_imm(val));
}

static void
iris_load_register_imm64(struct iris_batch *batch, uint32_t reg,
                         uint64_t val)
{
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   mi_store(&b, mi_reg64(reg), mi_imm(val));
}

/**
 * Emit MI_LOAD_REGISTER_MEM to load a 32-bit MMIO register from a buffer.
 */
static void
iris_load_register_mem32(struct iris_batch *batch, uint32_t reg,
                         struct iris_bo *bo, uint32_t offset)
{
   iris_batch_sync_region_start(batch);
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   struct mi_value src = mi_mem32(ro_bo(bo, offset));
   mi_store(&b, mi_reg32(reg), src);
   iris_batch_sync_region_end(batch);
}

/**
 * Load a 64-bit value from a buffer into a MMIO register via
 * two MI_LOAD_REGISTER_MEM commands.
 */
static void
iris_load_register_mem64(struct iris_batch *batch, uint32_t reg,
                         struct iris_bo *bo, uint32_t offset)
{
   iris_batch_sync_region_start(batch);
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   struct mi_value src = mi_mem64(ro_bo(bo, offset));
   mi_store(&b, mi_reg64(reg), src);
   iris_batch_sync_region_end(batch);
}

static void
iris_store_register_mem32(struct iris_batch *batch, uint32_t reg,
                          struct iris_bo *bo, uint32_t offset,
                          bool predicated)
{
   iris_batch_sync_region_start(batch);
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   struct mi_value dst = mi_mem32(rw_bo(bo, offset, IRIS_DOMAIN_OTHER_WRITE));
   struct mi_value src = mi_reg32(reg);
   if (predicated)
      mi_store_if(&b, dst, src);
   else
      mi_store(&b, dst, src);
   iris_batch_sync_region_end(batch);
}

static void
iris_store_register_mem64(struct iris_batch *batch, uint32_t reg,
                          struct iris_bo *bo, uint32_t offset,
                          bool predicated)
{
   iris_batch_sync_region_start(batch);
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   struct mi_value dst = mi_mem64(rw_bo(bo, offset, IRIS_DOMAIN_OTHER_WRITE));
   struct mi_value src = mi_reg64(reg);
   if (predicated)
      mi_store_if(&b, dst, src);
   else
      mi_store(&b, dst, src);
   iris_batch_sync_region_end(batch);
}

static void
iris_store_data_imm32(struct iris_batch *batch,
                      struct iris_bo *bo, uint32_t offset,
                      uint32_t imm)
{
   iris_batch_sync_region_start(batch);
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   struct mi_value dst = mi_mem32(rw_bo(bo, offset, IRIS_DOMAIN_OTHER_WRITE));
   struct mi_value src = mi_imm(imm);
   mi_store(&b, dst, src);
   iris_batch_sync_region_end(batch);
}

static void
iris_store_data_imm64(struct iris_batch *batch,
                      struct iris_bo *bo, uint32_t offset,
                      uint64_t imm)
{
   iris_batch_sync_region_start(batch);
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   struct mi_value dst = mi_mem64(rw_bo(bo, offset, IRIS_DOMAIN_OTHER_WRITE));
   struct mi_value src = mi_imm(imm);
   mi_store(&b, dst, src);
   iris_batch_sync_region_end(batch);
}

static void
iris_copy_mem_mem(struct iris_batch *batch,
                  struct iris_bo *dst_bo, uint32_t dst_offset,
                  struct iris_bo *src_bo, uint32_t src_offset,
                  unsigned bytes)
{
   /* MI_COPY_MEM_MEM operates on DWords. */
   assert(bytes % 4 == 0);
   assert(dst_offset % 4 == 0);
   assert(src_offset % 4 == 0);
   iris_batch_sync_region_start(batch);

   for (unsigned i = 0; i < bytes; i += 4) {
      iris_emit_cmd(batch, GENX(MI_COPY_MEM_MEM), cp) {
         cp.DestinationMemoryAddress = rw_bo(dst_bo, dst_offset + i,
                                             IRIS_DOMAIN_OTHER_WRITE);
         cp.SourceMemoryAddress = ro_bo(src_bo, src_offset + i);
      }
   }

   iris_batch_sync_region_end(batch);
}

static void
iris_rewrite_compute_walker_pc(struct iris_batch *batch,
                               uint32_t *walker,
                               struct iris_bo *bo,
                               uint32_t offset)
{
#if GFX_VERx10 >= 125
   struct iris_screen *screen = batch->screen;
   struct iris_address addr = rw_bo(bo, offset, IRIS_DOMAIN_OTHER_WRITE);

   uint32_t dwords[GENX(COMPUTE_WALKER_length)];

   _iris_pack_command(batch, GENX(COMPUTE_WALKER), dwords, cw) {
      cw.PostSync.Operation          = WriteTimestamp;
      cw.PostSync.DestinationAddress = addr;
      cw.PostSync.MOCS               = iris_mocs(NULL, &screen->isl_dev, 0);
   }

   for (uint32_t i = 0; i < GENX(COMPUTE_WALKER_length); i++)
      walker[i] |= dwords[i];
#else
   unreachable("Unsupported");
#endif
}

static void
emit_pipeline_select(struct iris_batch *batch, uint32_t pipeline)
{
   /* Bspec 55860: Xe2+ no longer requires PIPELINE_SELECT */
#if GFX_VER < 20

#if GFX_VER >= 8 && GFX_VER < 10
   /* From the Broadwell PRM, Volume 2a: Instructions, PIPELINE_SELECT:
    *
    *   Software must clear the COLOR_CALC_STATE Valid field in
    *   3DSTATE_CC_STATE_POINTERS command prior to send a PIPELINE_SELECT
    *   with Pipeline Select set to GPGPU.
    *
    * The internal hardware docs recommend the same workaround for Gfx9
    * hardware too.
    */
   if (pipeline == GPGPU)
      iris_emit_cmd(batch, GENX(3DSTATE_CC_STATE_POINTERS), t);
#endif

#if GFX_VER >= 12
   /* From Tigerlake PRM, Volume 2a, PIPELINE_SELECT:
    *
    *   "Software must ensure Render Cache, Depth Cache and HDC Pipeline flush
    *   are flushed through a stalling PIPE_CONTROL command prior to
    *   programming of PIPELINE_SELECT command transitioning Pipeline Select
    *   from 3D to GPGPU/Media.
    *   Software must ensure HDC Pipeline flush and Generic Media State Clear
    *   is issued through a stalling PIPE_CONTROL command prior to programming
    *   of PIPELINE_SELECT command transitioning Pipeline Select from
    *   GPGPU/Media to 3D."
    *
    * Note: Issuing PIPE_CONTROL_MEDIA_STATE_CLEAR causes GPU hangs, probably
    * because PIPE was not in MEDIA mode?!
    */
   enum pipe_control_flags flags = PIPE_CONTROL_CS_STALL |
                                   PIPE_CONTROL_FLUSH_HDC;

   if (pipeline == GPGPU && batch->name == IRIS_BATCH_RENDER) {
      flags |= PIPE_CONTROL_RENDER_TARGET_FLUSH |
               PIPE_CONTROL_DEPTH_CACHE_FLUSH;
   } else {
      flags |= PIPE_CONTROL_UNTYPED_DATAPORT_CACHE_FLUSH;
   }
   /* Wa_16013063087 -  State Cache Invalidate must be issued prior to
    * PIPELINE_SELECT when switching from 3D to Compute.
    *
    * SW must do this by programming of PIPECONTROL with “CS Stall” followed
    * by a PIPECONTROL with State Cache Invalidate bit set.
    */
   if (pipeline == GPGPU &&
       intel_needs_workaround(batch->screen->devinfo, 16013063087))
      flags |= PIPE_CONTROL_STATE_CACHE_INVALIDATE;

   iris_emit_pipe_control_flush(batch, "PIPELINE_SELECT flush", flags);
#else
   /* From "BXML » GT » MI » vol1a GPU Overview » [Instruction]
    * PIPELINE_SELECT [DevBWR+]":
    *
    *    "Project: DEVSNB+
    *
    *     Software must ensure all the write caches are flushed through a
    *     stalling PIPE_CONTROL command followed by another PIPE_CONTROL
    *     command to invalidate read only caches prior to programming
    *     MI_PIPELINE_SELECT command to change the Pipeline Select Mode."
    */
    iris_emit_pipe_control_flush(batch,
                                 "workaround: PIPELINE_SELECT flushes (1/2)",
                                 PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                 PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                 PIPE_CONTROL_DATA_CACHE_FLUSH |
                                 PIPE_CONTROL_UNTYPED_DATAPORT_CACHE_FLUSH |
                                 PIPE_CONTROL_CS_STALL);

    iris_emit_pipe_control_flush(batch,
                                 "workaround: PIPELINE_SELECT flushes (2/2)",
                                 PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
                                 PIPE_CONTROL_CONST_CACHE_INVALIDATE |
                                 PIPE_CONTROL_STATE_CACHE_INVALIDATE |
                                 PIPE_CONTROL_INSTRUCTION_INVALIDATE);
#endif

   iris_emit_cmd(batch, GENX(PIPELINE_SELECT), sel) {
#if GFX_VER >= 9
      sel.MaskBits = GFX_VER == 12 ? 0x13 : 0x3;
#if GFX_VER == 12
      sel.MediaSamplerDOPClockGateEnable = true;
#endif /* if GFX_VER == 12 */
#endif /* if GFX_VER >= 9 */
      sel.PipelineSelection = pipeline;
   }
#endif /* if GFX_VER < 20 */
}

UNUSED static void
init_glk_barrier_mode(struct iris_batch *batch, uint32_t value)
{
#if GFX_VER == 9
   /* Project: DevGLK
    *
    *    "This chicken bit works around a hardware issue with barrier
    *     logic encountered when switching between GPGPU and 3D pipelines.
    *     To workaround the issue, this mode bit should be set after a
    *     pipeline is selected."
    */
   iris_emit_reg(batch, GENX(SLICE_COMMON_ECO_CHICKEN1), reg) {
      reg.GLKBarrierMode = value;
      reg.GLKBarrierModeMask = 1;
   }
#endif
}

static void
init_state_base_address(struct iris_batch *batch)
{
   struct isl_device *isl_dev = &batch->screen->isl_dev;
   uint32_t mocs = isl_mocs(isl_dev, 0, false);
   flush_before_state_base_change(batch);

   /* We program most base addresses once at context initialization time.
    * Each base address points at a 4GB memory zone, and never needs to
    * change.  See iris_bufmgr.h for a description of the memory zones.
    *
    * The one exception is Surface State Base Address, which needs to be
    * updated occasionally.  See iris_binder.c for the details there.
    */
   iris_emit_cmd(batch, GENX(STATE_BASE_ADDRESS), sba) {
      sba.GeneralStateMOCS            = mocs;
      sba.StatelessDataPortAccessMOCS = mocs;
      sba.DynamicStateMOCS            = mocs;
      sba.IndirectObjectMOCS          = mocs;
      sba.InstructionMOCS             = mocs;
      sba.SurfaceStateMOCS            = mocs;
#if GFX_VER >= 9
      sba.BindlessSurfaceStateMOCS    = mocs;
#endif

      sba.GeneralStateBaseAddressModifyEnable   = true;
      sba.DynamicStateBaseAddressModifyEnable   = true;
      sba.IndirectObjectBaseAddressModifyEnable = true;
      sba.InstructionBaseAddressModifyEnable    = true;
      sba.GeneralStateBufferSizeModifyEnable    = true;
      sba.DynamicStateBufferSizeModifyEnable    = true;
      sba.SurfaceStateBaseAddressModifyEnable   = true;
#if GFX_VER >= 11
      sba.BindlessSamplerStateMOCS    = mocs;
#endif
      sba.IndirectObjectBufferSizeModifyEnable  = true;
      sba.InstructionBuffersizeModifyEnable     = true;

      sba.InstructionBaseAddress  = ro_bo(NULL, IRIS_MEMZONE_SHADER_START);
      sba.DynamicStateBaseAddress = ro_bo(NULL, IRIS_MEMZONE_DYNAMIC_START);
      sba.SurfaceStateBaseAddress = ro_bo(NULL, IRIS_MEMZONE_BINDER_START);

      sba.GeneralStateBufferSize   = 0xfffff;
      sba.IndirectObjectBufferSize = 0xfffff;
      sba.InstructionBufferSize    = 0xfffff;
      sba.DynamicStateBufferSize   = 0xfffff;
#if GFX_VERx10 >= 125
      sba.L1CacheControl = L1CC_WB;
#endif
   }

   flush_after_state_base_change(batch);
}

static void
iris_emit_l3_config(struct iris_batch *batch,
                    const struct intel_l3_config *cfg)
{
   assert(cfg || GFX_VER >= 12);

#if GFX_VER >= 12
#define L3_ALLOCATION_REG GENX(L3ALLOC)
#define L3_ALLOCATION_REG_num GENX(L3ALLOC_num)
#else
#define L3_ALLOCATION_REG GENX(L3CNTLREG)
#define L3_ALLOCATION_REG_num GENX(L3CNTLREG_num)
#endif

   iris_emit_reg(batch, L3_ALLOCATION_REG, reg) {
#if GFX_VER < 11
      reg.SLMEnable = cfg->n[INTEL_L3P_SLM] > 0;
#endif
#if GFX_VER == 11
      /* Wa_1406697149: Bit 9 "Error Detection Behavior Control" must be set
       * in L3CNTLREG register. The default setting of the bit is not the
       * desirable behavior.
       */
      reg.ErrorDetectionBehaviorControl = true;
      reg.UseFullWays = true;
#endif
      if (GFX_VER < 12 || (cfg && cfg->n[INTEL_L3P_ALL] <= 126)) {
         reg.URBAllocation = cfg->n[INTEL_L3P_URB];
         reg.ROAllocation = cfg->n[INTEL_L3P_RO];
         reg.DCAllocation = cfg->n[INTEL_L3P_DC];
         reg.AllAllocation = cfg->n[INTEL_L3P_ALL];
      } else {
         assert(!cfg || !(cfg->n[INTEL_L3P_SLM] || cfg->n[INTEL_L3P_URB] ||
                          cfg->n[INTEL_L3P_DC] || cfg->n[INTEL_L3P_RO] ||
                          cfg->n[INTEL_L3P_IS] || cfg->n[INTEL_L3P_C] ||
                          cfg->n[INTEL_L3P_T] || cfg->n[INTEL_L3P_TC]));
#if GFX_VER >= 12
         reg.L3FullWayAllocationEnable = true;
#endif
      }
   }
}

#if GFX_VER == 9
static void
iris_enable_obj_preemption(struct iris_batch *batch, bool enable)
{
   /* A fixed function pipe flush is required before modifying this field */
   iris_emit_end_of_pipe_sync(batch, enable ? "enable preemption"
                                            : "disable preemption",
                              PIPE_CONTROL_RENDER_TARGET_FLUSH);

   /* enable object level preemption */
   iris_emit_reg(batch, GENX(CS_CHICKEN1), reg) {
      reg.ReplayMode = enable;
      reg.ReplayModeMask = true;
   }
}
#endif

static void
upload_pixel_hashing_tables(struct iris_batch *batch)
{
   UNUSED const struct intel_device_info *devinfo = batch->screen->devinfo;
   UNUSED struct iris_context *ice = batch->ice;
   assert(&ice->batches[IRIS_BATCH_RENDER] == batch);

#if GFX_VER == 11
   /* Gfx11 hardware has two pixel pipes at most. */
   for (unsigned i = 2; i < ARRAY_SIZE(devinfo->ppipe_subslices); i++)
      assert(devinfo->ppipe_subslices[i] == 0);

   if (devinfo->ppipe_subslices[0] == devinfo->ppipe_subslices[1])
      return;

   unsigned size = GENX(SLICE_HASH_TABLE_length) * 4;
   uint32_t hash_address;
   struct pipe_resource *tmp = NULL;
   uint32_t *map =
      stream_state(batch, ice->state.dynamic_uploader, &tmp,
                   size, 64, &hash_address);
   pipe_resource_reference(&tmp, NULL);

   const bool flip = devinfo->ppipe_subslices[0] < devinfo->ppipe_subslices[1];
   struct GENX(SLICE_HASH_TABLE) table;
   intel_compute_pixel_hash_table_3way(16, 16, 3, 3, flip, table.Entry[0]);

   GENX(SLICE_HASH_TABLE_pack)(NULL, map, &table);

   iris_emit_cmd(batch, GENX(3DSTATE_SLICE_TABLE_STATE_POINTERS), ptr) {
      ptr.SliceHashStatePointerValid = true;
      ptr.SliceHashTableStatePointer = hash_address;
   }

   iris_emit_cmd(batch, GENX(3DSTATE_3D_MODE), mode) {
      mode.SliceHashingTableEnable = true;
   }

#elif GFX_VERx10 == 120
   /* For each n calculate ppipes_of[n], equal to the number of pixel pipes
    * present with n active dual subslices.
    */
   unsigned ppipes_of[3] = {};

   for (unsigned n = 0; n < ARRAY_SIZE(ppipes_of); n++) {
      for (unsigned p = 0; p < 3; p++)
         ppipes_of[n] += (devinfo->ppipe_subslices[p] == n);
   }

   /* Gfx12 has three pixel pipes. */
   for (unsigned p = 3; p < ARRAY_SIZE(devinfo->ppipe_subslices); p++)
      assert(devinfo->ppipe_subslices[p] == 0);

   if (ppipes_of[2] == 3 || ppipes_of[0] == 2) {
      /* All three pixel pipes have the maximum number of active dual
       * subslices, or there is only one active pixel pipe: Nothing to do.
       */
      return;
   }

   iris_emit_cmd(batch, GENX(3DSTATE_SUBSLICE_HASH_TABLE), p) {
      p.SliceHashControl[0] = TABLE_0;

      if (ppipes_of[2] == 2 && ppipes_of[0] == 1)
         intel_compute_pixel_hash_table_3way(8, 16, 2, 2, 0, p.TwoWayTableEntry[0]);
      else if (ppipes_of[2] == 1 && ppipes_of[1] == 1 && ppipes_of[0] == 1)
         intel_compute_pixel_hash_table_3way(8, 16, 3, 3, 0, p.TwoWayTableEntry[0]);

      if (ppipes_of[2] == 2 && ppipes_of[1] == 1)
         intel_compute_pixel_hash_table_3way(8, 16, 5, 4, 0, p.ThreeWayTableEntry[0]);
      else if (ppipes_of[2] == 2 && ppipes_of[0] == 1)
         intel_compute_pixel_hash_table_3way(8, 16, 2, 2, 0, p.ThreeWayTableEntry[0]);
      else if (ppipes_of[2] == 1 && ppipes_of[1] == 1 && ppipes_of[0] == 1)
         intel_compute_pixel_hash_table_3way(8, 16, 3, 3, 0, p.ThreeWayTableEntry[0]);
      else
         unreachable("Illegal fusing.");
   }

   iris_emit_cmd(batch, GENX(3DSTATE_3D_MODE), p) {
      p.SubsliceHashingTableEnable = true;
      p.SubsliceHashingTableEnableMask = true;
   }

#elif GFX_VERx10 == 125
   struct pipe_screen *pscreen = &batch->screen->base;
   const unsigned size = GENX(SLICE_HASH_TABLE_length) * 4;
   const struct pipe_resource tmpl = {
     .target = PIPE_BUFFER,
     .format = PIPE_FORMAT_R8_UNORM,
     .bind = PIPE_BIND_CUSTOM,
     .usage = PIPE_USAGE_IMMUTABLE,
     .flags = IRIS_RESOURCE_FLAG_DYNAMIC_MEMZONE,
     .width0 = size,
     .height0 = 1,
     .depth0 = 1,
     .array_size = 1
   };

   pipe_resource_reference(&ice->state.pixel_hashing_tables, NULL);
   ice->state.pixel_hashing_tables = pscreen->resource_create(pscreen, &tmpl);

   struct iris_resource *res = (struct iris_resource *)ice->state.pixel_hashing_tables;
   struct pipe_transfer *transfer = NULL;
   uint32_t *map = pipe_buffer_map_range(&ice->ctx, ice->state.pixel_hashing_tables,
                                         0, size, PIPE_MAP_WRITE,
                                         &transfer);

   /* Calculate the set of present pixel pipes, and another set of
    * present pixel pipes with 2 dual subslices enabled, the latter
    * will appear on the hashing table with twice the frequency of
    * pixel pipes with a single dual subslice present.
    */
   uint32_t ppipe_mask1 = 0, ppipe_mask2 = 0;
   for (unsigned p = 0; p < ARRAY_SIZE(devinfo->ppipe_subslices); p++) {
      if (devinfo->ppipe_subslices[p])
         ppipe_mask1 |= (1u << p);
      if (devinfo->ppipe_subslices[p] > 1)
         ppipe_mask2 |= (1u << p);
   }
   assert(ppipe_mask1);

   struct GENX(SLICE_HASH_TABLE) table;

   /* Note that the hardware expects an array with 7 tables, each
    * table is intended to specify the pixel pipe hashing behavior for
    * every possible slice count between 2 and 8, however that doesn't
    * actually work, among other reasons due to hardware bugs that
    * will cause the GPU to erroneously access the table at the wrong
    * index in some cases, so in practice all 7 tables need to be
    * initialized to the same value.
    */
   for (unsigned i = 0; i < 7; i++)
      intel_compute_pixel_hash_table_nway(16, 16, ppipe_mask1, ppipe_mask2,
                                          table.Entry[i][0]);

   GENX(SLICE_HASH_TABLE_pack)(NULL, map, &table);

   pipe_buffer_unmap(&ice->ctx, transfer);

   iris_use_pinned_bo(batch, res->bo, false, IRIS_DOMAIN_NONE);
   iris_record_state_size(batch->state_sizes, res->bo->address + res->offset, size);

   iris_emit_cmd(batch, GENX(3DSTATE_SLICE_TABLE_STATE_POINTERS), ptr) {
      ptr.SliceHashStatePointerValid = true;
      ptr.SliceHashTableStatePointer = iris_bo_offset_from_base_address(res->bo) +
                                       res->offset;
   }

   iris_emit_cmd(batch, GENX(3DSTATE_3D_MODE), mode) {
      mode.SliceHashingTableEnable = true;
      mode.SliceHashingTableEnableMask = true;
      mode.CrossSliceHashingMode = (util_bitcount(ppipe_mask1) > 1 ?
                                    hashing32x32 : NormalMode);
      mode.CrossSliceHashingModeMask = -1;
   }
#endif
}

static void
iris_alloc_push_constants(struct iris_batch *batch)
{
   const struct intel_device_info *devinfo = batch->screen->devinfo;

   /* For now, we set a static partitioning of the push constant area,
    * assuming that all stages could be in use.
    *
    * TODO: Try lazily allocating the HS/DS/GS sections as needed, and
    *       see if that improves performance by offering more space to
    *       the VS/FS when those aren't in use.  Also, try dynamically
    *       enabling/disabling it like i965 does.  This would be more
    *       stalls and may not actually help; we don't know yet.
    */

   /* Divide as equally as possible with any remainder given to FRAGMENT. */
   const unsigned push_constant_kb = devinfo->max_constant_urb_size_kb;
   const unsigned stage_size = push_constant_kb / 5;
   const unsigned frag_size = push_constant_kb - 4 * stage_size;

   for (int i = 0; i <= MESA_SHADER_FRAGMENT; i++) {
      iris_emit_cmd(batch, GENX(3DSTATE_PUSH_CONSTANT_ALLOC_VS), alloc) {
         alloc._3DCommandSubOpcode = 18 + i;
         alloc.ConstantBufferOffset = stage_size * i;
         alloc.ConstantBufferSize = i == MESA_SHADER_FRAGMENT ? frag_size : stage_size;
      }
   }

#if GFX_VERx10 == 125
   /* DG2: Wa_22011440098
    * MTL: Wa_18022330953
    *
    * In 3D mode, after programming push constant alloc command immediately
    * program push constant command(ZERO length) without any commit between
    * them.
    */
   iris_emit_cmd(batch, GENX(3DSTATE_CONSTANT_ALL), c) {
      /* Update empty push constants for all stages (bitmask = 11111b) */
      c.ShaderUpdateEnable = 0x1f;
      c.MOCS = iris_mocs(NULL, &batch->screen->isl_dev, 0);
   }
#endif
}

#if GFX_VER >= 12
static void
init_aux_map_state(struct iris_batch *batch);
#endif

/* This updates a register. Caller should stall the pipeline as needed. */
static void
iris_disable_rhwo_optimization(struct iris_batch *batch, bool disable)
{
   assert(batch->screen->devinfo->verx10 == 120);
#if GFX_VERx10 == 120
   iris_emit_reg(batch, GENX(COMMON_SLICE_CHICKEN1), c1) {
      c1.RCCRHWOOptimizationDisable = disable;
      c1.RCCRHWOOptimizationDisableMask = true;
   };
#endif
}

/**
 * Upload initial GPU state for any kind of context.
 *
 * These need to happen for both render and compute.
 */
static void
iris_init_common_context(struct iris_batch *batch)
{
#if GFX_VER == 11
   iris_emit_reg(batch, GENX(SAMPLER_MODE), reg) {
      reg.HeaderlessMessageforPreemptableContexts = 1;
      reg.HeaderlessMessageforPreemptableContextsMask = 1;
   }

   /* Bit 1 must be set in HALF_SLICE_CHICKEN7. */
   iris_emit_reg(batch, GENX(HALF_SLICE_CHICKEN7), reg) {
      reg.EnabledTexelOffsetPrecisionFix = 1;
      reg.EnabledTexelOffsetPrecisionFixMask = 1;
   }
#endif

   /* Select 256B-aligned binding table mode on Icelake through Tigerlake,
    * which gives us larger binding table pointers, at the cost of higher
    * alignment requirements (bits 18:8 are valid instead of 15:5).  When
    * using this mode, we have to shift binding table pointers by 3 bits,
    * as they're still stored in the same bit-location in the field.
    */
#if GFX_VER >= 11 && GFX_VERx10 < 125
   iris_emit_reg(batch, GENX(GT_MODE), reg) {
      reg.BindingTableAlignment = BTP_18_8;
      reg.BindingTableAlignmentMask = true;
   }
#define IRIS_BT_OFFSET_SHIFT 3
#else
#define IRIS_BT_OFFSET_SHIFT 0
#endif

#if GFX_VERx10 == 125
   /* Even though L3 partial write merging is supposed to be enabled
    * by default on Gfx12.5 according to the hardware spec, i915
    * appears to accidentally clear the enables during context
    * initialization, so make sure to enable them here since partial
    * write merging has a large impact on rendering performance.
    */
   iris_emit_reg(batch, GENX(L3SQCREG5), reg) {
      reg.L3CachePartialWriteMergeTimerInitialValue = 0x7f;
      reg.CompressiblePartialWriteMergeEnable = true;
      reg.CoherentPartialWriteMergeEnable = true;
      reg.CrossTilePartialWriteMergeEnable = true;
   }
#endif
}

static void
toggle_protected(struct iris_batch *batch)
{
   struct iris_context *ice;

   if (batch->name == IRIS_BATCH_RENDER)
      ice =container_of(batch, struct iris_context, batches[IRIS_BATCH_RENDER]);
   else if (batch->name == IRIS_BATCH_COMPUTE)
      ice = container_of(batch, struct iris_context, batches[IRIS_BATCH_COMPUTE]);
   else
      unreachable("unhandled batch");

   if (!ice->protected)
      return;

#if GFX_VER >= 12
   iris_emit_cmd(batch, GENX(PIPE_CONTROL), pc) {
      pc.CommandStreamerStallEnable = true;
      pc.RenderTargetCacheFlushEnable = true;
      pc.ProtectedMemoryDisable = true;
   }
   iris_emit_cmd(batch, GENX(MI_SET_APPID), appid) {
      /* Default value for single session. */
      appid.ProtectedMemoryApplicationID = 0xf;
      appid.ProtectedMemoryApplicationIDType = DISPLAY_APP;
   }
   iris_emit_cmd(batch, GENX(PIPE_CONTROL), pc) {
      pc.CommandStreamerStallEnable = true;
      pc.RenderTargetCacheFlushEnable = true;
      pc.ProtectedMemoryEnable = true;
   }
#else
   unreachable("Not supported");
#endif
}

/**
 * Upload the initial GPU state for a render context.
 *
 * This sets some invariant state that needs to be programmed a particular
 * way, but we never actually change.
 */
static void
iris_init_render_context(struct iris_batch *batch)
{
   UNUSED const struct intel_device_info *devinfo = batch->screen->devinfo;

   iris_batch_sync_region_start(batch);

   emit_pipeline_select(batch, _3D);

   toggle_protected(batch);

   iris_emit_l3_config(batch, batch->screen->l3_config_3d);

   init_state_base_address(batch);

   iris_init_common_context(batch);

#if GFX_VER >= 9
   iris_emit_reg(batch, GENX(CS_DEBUG_MODE2), reg) {
      reg.CONSTANT_BUFFERAddressOffsetDisable = true;
      reg.CONSTANT_BUFFERAddressOffsetDisableMask = true;
   }
#else
   iris_emit_reg(batch, GENX(INSTPM), reg) {
      reg.CONSTANT_BUFFERAddressOffsetDisable = true;
      reg.CONSTANT_BUFFERAddressOffsetDisableMask = true;
   }
#endif

#if GFX_VER == 9
   iris_emit_reg(batch, GENX(CACHE_MODE_1), reg) {
      reg.FloatBlendOptimizationEnable = true;
      reg.FloatBlendOptimizationEnableMask = true;
      reg.MSCRAWHazardAvoidanceBit = true;
      reg.MSCRAWHazardAvoidanceBitMask = true;
      reg.PartialResolveDisableInVC = true;
      reg.PartialResolveDisableInVCMask = true;
   }

   if (devinfo->platform == INTEL_PLATFORM_GLK)
      init_glk_barrier_mode(batch, GLK_BARRIER_MODE_3D_HULL);
#endif

#if GFX_VER == 11
   iris_emit_reg(batch, GENX(TCCNTLREG), reg) {
      reg.L3DataPartialWriteMergingEnable = true;
      reg.ColorZPartialWriteMergingEnable = true;
      reg.URBPartialWriteMergingEnable = true;
      reg.TCDisable = true;
   }

   /* Hardware specification recommends disabling repacking for the
    * compatibility with decompression mechanism in display controller.
    */
   if (devinfo->disable_ccs_repack) {
      iris_emit_reg(batch, GENX(CACHE_MODE_0), reg) {
         reg.DisableRepackingforCompression = true;
         reg.DisableRepackingforCompressionMask = true;
      }
   }
#endif

#if GFX_VER == 12
   iris_emit_reg(batch, GENX(FF_MODE2), reg) {
      /* On Alchemist, the FF_MODE2 docs for the GS timer say:
       *
       *    "The timer value must be set to 224."
       *
       * and Wa_16011163337 indicates this is the case for all Gfx12 parts,
       * and that this is necessary to avoid hanging the HS/DS units.  It
       * also clarifies that 224 is literally 0xE0 in the bits, not 7*32=224.
       *
       * The HS timer docs also have the same quote for Alchemist.  I am
       * unaware of a reason it needs to be set to 224 on Tigerlake, but
       * we do so for consistency if nothing else.
       *
       * For the TDS timer value, the docs say:
       *
       *    "For best performance, a value of 4 should be programmed."
       *
       * i915 also sets it this way on Tigerlake due to workarounds.
       *
       * The default VS timer appears to be 0, so we leave it at that.
       */
      reg.GSTimerValue  = 224;
      reg.HSTimerValue  = 224;
      reg.TDSTimerValue = 4;
      reg.VSTimerValue  = 0;
   }
#endif

#if INTEL_NEEDS_WA_1508744258
   /* The suggested workaround is:
    *
    *    Disable RHWO by setting 0x7010[14] by default except during resolve
    *    pass.
    *
    * We implement global disabling of the optimization here and we toggle it
    * in iris_resolve_color.
    *
    * iris_init_compute_context is unmodified because we don't expect to
    * access the RCC in the compute context. iris_mcs_partial_resolve is
    * unmodified because that pass doesn't use a HW bit to perform the
    * resolve (related HSDs specifically call out the RenderTargetResolveType
    * field in the 3DSTATE_PS instruction).
    */
   iris_disable_rhwo_optimization(batch, true);
#endif

#if GFX_VERx10 == 120
   /* Wa_1806527549 says to disable the following HiZ optimization when the
    * depth buffer is D16_UNORM. We've found the WA to help with more depth
    * buffer configurations however, so we always disable it just to be safe.
    */
   iris_emit_reg(batch, GENX(HIZ_CHICKEN), reg) {
      reg.HZDepthTestLEGEOptimizationDisable = true;
      reg.HZDepthTestLEGEOptimizationDisableMask = true;
   }
#endif

#if GFX_VERx10 == 125
   iris_emit_reg(batch, GENX(CHICKEN_RASTER_2), reg) {
      reg.TBIMRBatchSizeOverride = true;
      reg.TBIMROpenBatchEnable = true;
      reg.TBIMRFastClip = true;
      reg.TBIMRBatchSizeOverrideMask = true;
      reg.TBIMROpenBatchEnableMask = true;
      reg.TBIMRFastClipMask = true;
   };
#endif

   upload_pixel_hashing_tables(batch);

   /* 3DSTATE_DRAWING_RECTANGLE is non-pipelined, so we want to avoid
    * changing it dynamically.  We set it to the maximum size here, and
    * instead include the render target dimensions in the viewport, so
    * viewport extents clipping takes care of pruning stray geometry.
    */
   iris_emit_cmd(batch, GENX(3DSTATE_DRAWING_RECTANGLE), rect) {
      rect.ClippedDrawingRectangleXMax = UINT16_MAX;
      rect.ClippedDrawingRectangleYMax = UINT16_MAX;
   }

   /* Set the initial MSAA sample positions. */
   iris_emit_cmd(batch, GENX(3DSTATE_SAMPLE_PATTERN), pat) {
      INTEL_SAMPLE_POS_1X(pat._1xSample);
      INTEL_SAMPLE_POS_2X(pat._2xSample);
      INTEL_SAMPLE_POS_4X(pat._4xSample);
      INTEL_SAMPLE_POS_8X(pat._8xSample);
#if GFX_VER >= 9
      INTEL_SAMPLE_POS_16X(pat._16xSample);
#endif
   }

   /* Use the legacy AA line coverage computation. */
   iris_emit_cmd(batch, GENX(3DSTATE_AA_LINE_PARAMETERS), foo);

   /* Disable chromakeying (it's for media) */
   iris_emit_cmd(batch, GENX(3DSTATE_WM_CHROMAKEY), foo);

   /* We want regular rendering, not special HiZ operations. */
   iris_emit_cmd(batch, GENX(3DSTATE_WM_HZ_OP), foo);

   /* No polygon stippling offsets are necessary. */
   /* TODO: may need to set an offset for origin-UL framebuffers */
   iris_emit_cmd(batch, GENX(3DSTATE_POLY_STIPPLE_OFFSET), foo);

#if GFX_VERx10 >= 125
   iris_emit_cmd(batch, GENX(3DSTATE_MESH_CONTROL), foo);
   iris_emit_cmd(batch, GENX(3DSTATE_TASK_CONTROL), foo);
#endif

   iris_alloc_push_constants(batch);


#if GFX_VER >= 12
   init_aux_map_state(batch);
#endif

   iris_batch_sync_region_end(batch);
}

static void
iris_init_compute_context(struct iris_batch *batch)
{
   UNUSED const struct intel_device_info *devinfo = batch->screen->devinfo;

   iris_batch_sync_region_start(batch);

   /* Wa_1607854226:
    *
    *  Start with pipeline in 3D mode to set the STATE_BASE_ADDRESS.
    */
#if GFX_VERx10 == 120
   emit_pipeline_select(batch, _3D);
#else
   emit_pipeline_select(batch, GPGPU);
#endif

   toggle_protected(batch);

   iris_emit_l3_config(batch, batch->screen->l3_config_cs);

   init_state_base_address(batch);

   iris_init_common_context(batch);

#if GFX_VERx10 == 120
   emit_pipeline_select(batch, GPGPU);
#endif

#if GFX_VER == 9
   if (devinfo->platform == INTEL_PLATFORM_GLK)
      init_glk_barrier_mode(batch, GLK_BARRIER_MODE_GPGPU);
#endif

#if GFX_VER >= 12
   init_aux_map_state(batch);
#endif

#if GFX_VERx10 >= 125
   iris_emit_cmd(batch, GENX(CFE_STATE), cfe) {
      cfe.MaximumNumberofThreads =
         devinfo->max_cs_threads * devinfo->subslice_total;
   }
#endif

   iris_batch_sync_region_end(batch);
}

static void
iris_init_copy_context(struct iris_batch *batch)
{
   iris_batch_sync_region_start(batch);

#if GFX_VER >= 12
   init_aux_map_state(batch);
#endif

   iris_batch_sync_region_end(batch);
}

struct iris_vertex_buffer_state {
   /** The VERTEX_BUFFER_STATE hardware structure. */
   uint32_t state[GENX(VERTEX_BUFFER_STATE_length)];

   /** The resource to source vertex data from. */
   struct pipe_resource *resource;

   int offset;
};

struct iris_depth_buffer_state {
   /* Depth/HiZ/Stencil related hardware packets. */
#if GFX_VER < 20
   uint32_t packets[GENX(3DSTATE_DEPTH_BUFFER_length) +
                    GENX(3DSTATE_STENCIL_BUFFER_length) +
                    GENX(3DSTATE_HIER_DEPTH_BUFFER_length) +
                    GENX(3DSTATE_CLEAR_PARAMS_length)];
#else
   uint32_t packets[GENX(3DSTATE_DEPTH_BUFFER_length) +
                    GENX(3DSTATE_STENCIL_BUFFER_length) +
                    GENX(3DSTATE_HIER_DEPTH_BUFFER_length)];
#endif
};

#if INTEL_NEEDS_WA_1808121037
enum iris_depth_reg_mode {
   IRIS_DEPTH_REG_MODE_HW_DEFAULT = 0,
   IRIS_DEPTH_REG_MODE_D16_1X_MSAA,
   IRIS_DEPTH_REG_MODE_UNKNOWN,
};
#endif

/**
 * Generation-specific context state (ice->state.genx->...).
 *
 * Most state can go in iris_context directly, but these encode hardware
 * packets which vary by generation.
 */
struct iris_genx_state {
   struct iris_vertex_buffer_state vertex_buffers[33];
   uint32_t last_index_buffer[GENX(3DSTATE_INDEX_BUFFER_length)];

   struct iris_depth_buffer_state depth_buffer;

   uint32_t so_buffers[4 * GENX(3DSTATE_SO_BUFFER_length)];

#if GFX_VER == 8
   bool pma_fix_enabled;
#endif

   /* Is object level preemption enabled? */
   bool object_preemption;

#if INTEL_NEEDS_WA_1808121037
   enum iris_depth_reg_mode depth_reg_mode;
#endif

   struct {
#if GFX_VER == 8
      struct brw_image_param image_param[PIPE_MAX_SHADER_IMAGES];
#endif
   } shaders[MESA_SHADER_STAGES];
};

/**
 * The pipe->set_blend_color() driver hook.
 *
 * This corresponds to our COLOR_CALC_STATE.
 */
static void
iris_set_blend_color(struct pipe_context *ctx,
                     const struct pipe_blend_color *state)
{
   struct iris_context *ice = (struct iris_context *) ctx;

   /* Our COLOR_CALC_STATE is exactly pipe_blend_color, so just memcpy */
   memcpy(&ice->state.blend_color, state, sizeof(struct pipe_blend_color));
   ice->state.dirty |= IRIS_DIRTY_COLOR_CALC_STATE;
}

/**
 * Gallium CSO for blend state (see pipe_blend_state).
 */
struct iris_blend_state {
   /** Partial 3DSTATE_PS_BLEND */
   uint32_t ps_blend[GENX(3DSTATE_PS_BLEND_length)];

   /** Partial BLEND_STATE */
   uint32_t blend_state[GENX(BLEND_STATE_length) +
                        BRW_MAX_DRAW_BUFFERS * GENX(BLEND_STATE_ENTRY_length)];

   bool alpha_to_coverage; /* for shader key */

   /** Bitfield of whether blending is enabled for RT[i] - for aux resolves */
   uint8_t blend_enables;

   /** Bitfield of whether color writes are enabled for RT[i] */
   uint8_t color_write_enables;

   /** Does RT[0] use dual color blending? */
   bool dual_color_blending;

   int ps_dst_blend_factor[BRW_MAX_DRAW_BUFFERS];
   int ps_dst_alpha_blend_factor[BRW_MAX_DRAW_BUFFERS];
};

static enum pipe_blendfactor
fix_blendfactor(enum pipe_blendfactor f, bool alpha_to_one)
{
   if (alpha_to_one) {
      if (f == PIPE_BLENDFACTOR_SRC1_ALPHA)
         return PIPE_BLENDFACTOR_ONE;

      if (f == PIPE_BLENDFACTOR_INV_SRC1_ALPHA)
         return PIPE_BLENDFACTOR_ZERO;
   }

   return f;
}

/**
 * The pipe->create_blend_state() driver hook.
 *
 * Translates a pipe_blend_state into iris_blend_state.
 */
static void *
iris_create_blend_state(struct pipe_context *ctx,
                        const struct pipe_blend_state *state)
{
   struct iris_blend_state *cso = malloc(sizeof(struct iris_blend_state));
   uint32_t *blend_entry = cso->blend_state + GENX(BLEND_STATE_length);

   cso->blend_enables = 0;
   cso->color_write_enables = 0;
   STATIC_ASSERT(BRW_MAX_DRAW_BUFFERS <= 8);

   cso->alpha_to_coverage = state->alpha_to_coverage;

   bool indep_alpha_blend = false;

   for (int i = 0; i < BRW_MAX_DRAW_BUFFERS; i++) {
      const struct pipe_rt_blend_state *rt =
         &state->rt[state->independent_blend_enable ? i : 0];

      enum pipe_blendfactor src_rgb =
         fix_blendfactor(rt->rgb_src_factor, state->alpha_to_one);
      enum pipe_blendfactor src_alpha =
         fix_blendfactor(rt->alpha_src_factor, state->alpha_to_one);
      enum pipe_blendfactor dst_rgb =
         fix_blendfactor(rt->rgb_dst_factor, state->alpha_to_one);
      enum pipe_blendfactor dst_alpha =
         fix_blendfactor(rt->alpha_dst_factor, state->alpha_to_one);

      /* Stored separately in cso for dynamic emission. */
      cso->ps_dst_blend_factor[i] = (int) dst_rgb;
      cso->ps_dst_alpha_blend_factor[i] = (int) dst_alpha;

      if (rt->rgb_func != rt->alpha_func ||
          src_rgb != src_alpha || dst_rgb != dst_alpha)
         indep_alpha_blend = true;

      if (rt->blend_enable)
         cso->blend_enables |= 1u << i;

      if (rt->colormask)
         cso->color_write_enables |= 1u << i;

      iris_pack_state(GENX(BLEND_STATE_ENTRY), blend_entry, be) {
         be.LogicOpEnable = state->logicop_enable;
         be.LogicOpFunction = state->logicop_func;

         be.PreBlendSourceOnlyClampEnable = false;
         be.ColorClampRange = COLORCLAMP_RTFORMAT;
         be.PreBlendColorClampEnable = true;
         be.PostBlendColorClampEnable = true;

         be.ColorBufferBlendEnable = rt->blend_enable;

         be.ColorBlendFunction          = rt->rgb_func;
         be.AlphaBlendFunction          = rt->alpha_func;

         /* The casts prevent warnings about implicit enum type conversions. */
         be.SourceBlendFactor           = (int) src_rgb;
         be.SourceAlphaBlendFactor      = (int) src_alpha;

         be.WriteDisableRed   = !(rt->colormask & PIPE_MASK_R);
         be.WriteDisableGreen = !(rt->colormask & PIPE_MASK_G);
         be.WriteDisableBlue  = !(rt->colormask & PIPE_MASK_B);
         be.WriteDisableAlpha = !(rt->colormask & PIPE_MASK_A);
      }
      blend_entry += GENX(BLEND_STATE_ENTRY_length);
   }

   iris_pack_command(GENX(3DSTATE_PS_BLEND), cso->ps_blend, pb) {
      /* pb.HasWriteableRT is filled in at draw time.
       * pb.AlphaTestEnable is filled in at draw time.
       *
       * pb.ColorBufferBlendEnable is filled in at draw time so we can avoid
       * setting it when dual color blending without an appropriate shader.
       */

      pb.AlphaToCoverageEnable = state->alpha_to_coverage;
      pb.IndependentAlphaBlendEnable = indep_alpha_blend;

      /* The casts prevent warnings about implicit enum type conversions. */
      pb.SourceBlendFactor =
         (int) fix_blendfactor(state->rt[0].rgb_src_factor, state->alpha_to_one);
      pb.SourceAlphaBlendFactor =
         (int) fix_blendfactor(state->rt[0].alpha_src_factor, state->alpha_to_one);
   }

   iris_pack_state(GENX(BLEND_STATE), cso->blend_state, bs) {
      bs.AlphaToCoverageEnable = state->alpha_to_coverage;
      bs.IndependentAlphaBlendEnable = indep_alpha_blend;
      bs.AlphaToOneEnable = state->alpha_to_one;
      bs.AlphaToCoverageDitherEnable = state->alpha_to_coverage_dither;
      bs.ColorDitherEnable = state->dither;
      /* bl.AlphaTestEnable and bs.AlphaTestFunction are filled in later. */
   }

   cso->dual_color_blending = util_blend_state_is_dual(state, 0);

   return cso;
}

/**
 * The pipe->bind_blend_state() driver hook.
 *
 * Bind a blending CSO and flag related dirty bits.
 */
static void
iris_bind_blend_state(struct pipe_context *ctx, void *state)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_blend_state *cso = state;

   ice->state.cso_blend = cso;

   ice->state.dirty |= IRIS_DIRTY_PS_BLEND;
   ice->state.dirty |= IRIS_DIRTY_BLEND_STATE;
   ice->state.stage_dirty |= ice->state.stage_dirty_for_nos[IRIS_NOS_BLEND];

   if (GFX_VER == 8)
      ice->state.dirty |= IRIS_DIRTY_PMA_FIX;
}

/**
 * Return true if the FS writes to any color outputs which are not disabled
 * via color masking.
 */
static bool
has_writeable_rt(const struct iris_blend_state *cso_blend,
                 const struct shader_info *fs_info)
{
   if (!fs_info)
      return false;

   unsigned rt_outputs = fs_info->outputs_written >> FRAG_RESULT_DATA0;

   if (fs_info->outputs_written & BITFIELD64_BIT(FRAG_RESULT_COLOR))
      rt_outputs = (1 << BRW_MAX_DRAW_BUFFERS) - 1;

   return cso_blend->color_write_enables & rt_outputs;
}

/**
 * Gallium CSO for depth, stencil, and alpha testing state.
 */
struct iris_depth_stencil_alpha_state {
   /** Partial 3DSTATE_WM_DEPTH_STENCIL. */
   uint32_t wmds[GENX(3DSTATE_WM_DEPTH_STENCIL_length)];

#if GFX_VER >= 12
   uint32_t depth_bounds[GENX(3DSTATE_DEPTH_BOUNDS_length)];
#endif

   /** Outbound to BLEND_STATE, 3DSTATE_PS_BLEND, COLOR_CALC_STATE. */
   unsigned alpha_enabled:1;
   unsigned alpha_func:3;     /**< PIPE_FUNC_x */
   float alpha_ref_value;     /**< reference value */

   /** Outbound to resolve and cache set tracking. */
   bool depth_writes_enabled;
   bool stencil_writes_enabled;

   /** Outbound to Gfx8-9 PMA stall equations */
   bool depth_test_enabled;

   /** Tracking state of DS writes for Wa_18019816803. */
   bool ds_write_state;
};

/**
 * The pipe->create_depth_stencil_alpha_state() driver hook.
 *
 * We encode most of 3DSTATE_WM_DEPTH_STENCIL, and just save off the alpha
 * testing state since we need pieces of it in a variety of places.
 */
static void *
iris_create_zsa_state(struct pipe_context *ctx,
                      const struct pipe_depth_stencil_alpha_state *state)
{
   struct iris_depth_stencil_alpha_state *cso =
      malloc(sizeof(struct iris_depth_stencil_alpha_state));

   bool two_sided_stencil = state->stencil[1].enabled;

   bool depth_write_enabled = false;
   bool stencil_write_enabled = false;

   /* Depth writes enabled? */
   if (state->depth_writemask &&
      ((!state->depth_enabled) ||
      ((state->depth_func != PIPE_FUNC_NEVER) &&
        (state->depth_func != PIPE_FUNC_EQUAL))))
      depth_write_enabled = true;

   bool stencil_all_keep =
      state->stencil[0].fail_op == PIPE_STENCIL_OP_KEEP &&
      state->stencil[0].zfail_op == PIPE_STENCIL_OP_KEEP &&
      state->stencil[0].zpass_op == PIPE_STENCIL_OP_KEEP &&
      (!two_sided_stencil ||
       (state->stencil[1].fail_op == PIPE_STENCIL_OP_KEEP &&
        state->stencil[1].zfail_op == PIPE_STENCIL_OP_KEEP &&
        state->stencil[1].zpass_op == PIPE_STENCIL_OP_KEEP));

   bool stencil_mask_zero =
      state->stencil[0].writemask == 0 ||
      (!two_sided_stencil || state->stencil[1].writemask  == 0);

   bool stencil_func_never =
      state->stencil[0].func == PIPE_FUNC_NEVER &&
      state->stencil[0].fail_op == PIPE_STENCIL_OP_KEEP &&
      (!two_sided_stencil ||
       (state->stencil[1].func == PIPE_FUNC_NEVER &&
        state->stencil[1].fail_op == PIPE_STENCIL_OP_KEEP));

   /* Stencil writes enabled? */
   if (state->stencil[0].writemask != 0 ||
      ((two_sided_stencil && state->stencil[1].writemask != 0) &&
       (!stencil_all_keep &&
        !stencil_mask_zero &&
        !stencil_func_never)))
      stencil_write_enabled = true;

   cso->ds_write_state = depth_write_enabled || stencil_write_enabled;

   cso->alpha_enabled = state->alpha_enabled;
   cso->alpha_func = state->alpha_func;
   cso->alpha_ref_value = state->alpha_ref_value;
   cso->depth_writes_enabled = state->depth_writemask;
   cso->depth_test_enabled = state->depth_enabled;
   cso->stencil_writes_enabled =
      state->stencil[0].writemask != 0 ||
      (two_sided_stencil && state->stencil[1].writemask != 0);

   /* gallium frontends need to optimize away EQUAL writes for us. */
   assert(!(state->depth_func == PIPE_FUNC_EQUAL && state->depth_writemask));

   iris_pack_command(GENX(3DSTATE_WM_DEPTH_STENCIL), cso->wmds, wmds) {
      wmds.StencilFailOp = state->stencil[0].fail_op;
      wmds.StencilPassDepthFailOp = state->stencil[0].zfail_op;
      wmds.StencilPassDepthPassOp = state->stencil[0].zpass_op;
      wmds.StencilTestFunction =
         translate_compare_func(state->stencil[0].func);
      wmds.BackfaceStencilFailOp = state->stencil[1].fail_op;
      wmds.BackfaceStencilPassDepthFailOp = state->stencil[1].zfail_op;
      wmds.BackfaceStencilPassDepthPassOp = state->stencil[1].zpass_op;
      wmds.BackfaceStencilTestFunction =
         translate_compare_func(state->stencil[1].func);
      wmds.DepthTestFunction = translate_compare_func(state->depth_func);
      wmds.DoubleSidedStencilEnable = two_sided_stencil;
      wmds.StencilTestEnable = state->stencil[0].enabled;
      wmds.StencilBufferWriteEnable =
         state->stencil[0].writemask != 0 ||
         (two_sided_stencil && state->stencil[1].writemask != 0);
      wmds.DepthTestEnable = state->depth_enabled;
      wmds.DepthBufferWriteEnable = state->depth_writemask;
      wmds.StencilTestMask = state->stencil[0].valuemask;
      wmds.StencilWriteMask = state->stencil[0].writemask;
      wmds.BackfaceStencilTestMask = state->stencil[1].valuemask;
      wmds.BackfaceStencilWriteMask = state->stencil[1].writemask;
      /* wmds.[Backface]StencilReferenceValue are merged later */
#if GFX_VER >= 12
      wmds.StencilReferenceValueModifyDisable = true;
#endif
   }

#if GFX_VER >= 12
   iris_pack_command(GENX(3DSTATE_DEPTH_BOUNDS), cso->depth_bounds, depth_bounds) {
      depth_bounds.DepthBoundsTestValueModifyDisable = false;
      depth_bounds.DepthBoundsTestEnableModifyDisable = false;
      depth_bounds.DepthBoundsTestEnable = state->depth_bounds_test;
      depth_bounds.DepthBoundsTestMinValue = state->depth_bounds_min;
      depth_bounds.DepthBoundsTestMaxValue = state->depth_bounds_max;
   }
#endif

   return cso;
}

/**
 * The pipe->bind_depth_stencil_alpha_state() driver hook.
 *
 * Bind a depth/stencil/alpha CSO and flag related dirty bits.
 */
static void
iris_bind_zsa_state(struct pipe_context *ctx, void *state)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_depth_stencil_alpha_state *old_cso = ice->state.cso_zsa;
   struct iris_depth_stencil_alpha_state *new_cso = state;

   if (new_cso) {
      if (cso_changed(alpha_ref_value))
         ice->state.dirty |= IRIS_DIRTY_COLOR_CALC_STATE;

      if (cso_changed(alpha_enabled))
         ice->state.dirty |= IRIS_DIRTY_PS_BLEND | IRIS_DIRTY_BLEND_STATE;

      if (cso_changed(alpha_func))
         ice->state.dirty |= IRIS_DIRTY_BLEND_STATE;

      if (cso_changed(depth_writes_enabled) || cso_changed(stencil_writes_enabled))
         ice->state.dirty |= IRIS_DIRTY_RENDER_RESOLVES_AND_FLUSHES;

      ice->state.depth_writes_enabled = new_cso->depth_writes_enabled;
      ice->state.stencil_writes_enabled = new_cso->stencil_writes_enabled;

      /* State ds_write_enable changed, need to flag dirty DS. */
      if (!old_cso || (ice->state.ds_write_state != new_cso->ds_write_state)) {
         ice->state.dirty |= IRIS_DIRTY_DS_WRITE_ENABLE;
         ice->state.ds_write_state = new_cso->ds_write_state;
      }

#if GFX_VER >= 12
      if (cso_changed(depth_bounds))
         ice->state.dirty |= IRIS_DIRTY_DEPTH_BOUNDS;
#endif
   }

   ice->state.cso_zsa = new_cso;
   ice->state.dirty |= IRIS_DIRTY_CC_VIEWPORT;
   ice->state.dirty |= IRIS_DIRTY_WM_DEPTH_STENCIL;
   ice->state.stage_dirty |=
      ice->state.stage_dirty_for_nos[IRIS_NOS_DEPTH_STENCIL_ALPHA];

   if (GFX_VER == 8)
      ice->state.dirty |= IRIS_DIRTY_PMA_FIX;
}

#if GFX_VER == 8
static bool
want_pma_fix(struct iris_context *ice)
{
   UNUSED struct iris_screen *screen = (void *) ice->ctx.screen;
   UNUSED const struct intel_device_info *devinfo = screen->devinfo;
   const struct brw_wm_prog_data *wm_prog_data = (void *)
      ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data;
   const struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
   const struct iris_depth_stencil_alpha_state *cso_zsa = ice->state.cso_zsa;
   const struct iris_blend_state *cso_blend = ice->state.cso_blend;

   /* In very specific combinations of state, we can instruct Gfx8-9 hardware
    * to avoid stalling at the pixel mask array.  The state equations are
    * documented in these places:
    *
    * - Gfx8 Depth PMA Fix:   CACHE_MODE_1::NP_PMA_FIX_ENABLE
    * - Gfx9 Stencil PMA Fix: CACHE_MODE_0::STC PMA Optimization Enable
    *
    * Both equations share some common elements:
    *
    *    no_hiz_op =
    *       !(3DSTATE_WM_HZ_OP::DepthBufferClear ||
    *         3DSTATE_WM_HZ_OP::DepthBufferResolve ||
    *         3DSTATE_WM_HZ_OP::Hierarchical Depth Buffer Resolve Enable ||
    *         3DSTATE_WM_HZ_OP::StencilBufferClear) &&
    *
    *    killpixels =
    *       3DSTATE_WM::ForceKillPix != ForceOff &&
    *       (3DSTATE_PS_EXTRA::PixelShaderKillsPixels ||
    *        3DSTATE_PS_EXTRA::oMask Present to RenderTarget ||
    *        3DSTATE_PS_BLEND::AlphaToCoverageEnable ||
    *        3DSTATE_PS_BLEND::AlphaTestEnable ||
    *        3DSTATE_WM_CHROMAKEY::ChromaKeyKillEnable)
    *
    *    (Technically the stencil PMA treats ForceKillPix differently,
    *     but I think this is a documentation oversight, and we don't
    *     ever use it in this way, so it doesn't matter).
    *
    *    common_pma_fix =
    *       3DSTATE_WM::ForceThreadDispatch != 1 &&
    *       3DSTATE_RASTER::ForceSampleCount == NUMRASTSAMPLES_0 &&
    *       3DSTATE_DEPTH_BUFFER::SURFACE_TYPE != NULL &&
    *       3DSTATE_DEPTH_BUFFER::HIZ Enable &&
    *       3DSTATE_WM::EDSC_Mode != EDSC_PREPS &&
    *       3DSTATE_PS_EXTRA::PixelShaderValid &&
    *       no_hiz_op
    *
    * These are always true:
    *
    *    3DSTATE_RASTER::ForceSampleCount == NUMRASTSAMPLES_0
    *    3DSTATE_PS_EXTRA::PixelShaderValid
    *
    * Also, we never use the normal drawing path for HiZ ops; these are true:
    *
    *    !(3DSTATE_WM_HZ_OP::DepthBufferClear ||
    *      3DSTATE_WM_HZ_OP::DepthBufferResolve ||
    *      3DSTATE_WM_HZ_OP::Hierarchical Depth Buffer Resolve Enable ||
    *      3DSTATE_WM_HZ_OP::StencilBufferClear)
    *
    * This happens sometimes:
    *
    *    3DSTATE_WM::ForceThreadDispatch != 1
    *
    * However, we choose to ignore it as it either agrees with the signal
    * (dispatch was already enabled, so nothing out of the ordinary), or
    * there are no framebuffer attachments (so no depth or HiZ anyway,
    * meaning the PMA signal will already be disabled).
    */

   if (!cso_fb->zsbuf)
      return false;

   struct iris_resource *zres, *sres;
   iris_get_depth_stencil_resources(cso_fb->zsbuf->texture, &zres, &sres);

   /* 3DSTATE_DEPTH_BUFFER::SURFACE_TYPE != NULL &&
    * 3DSTATE_DEPTH_BUFFER::HIZ Enable &&
    */
   if (!zres ||
       !iris_resource_level_has_hiz(devinfo, zres, cso_fb->zsbuf->u.tex.level))
      return false;

   /* 3DSTATE_WM::EDSC_Mode != EDSC_PREPS */
   if (wm_prog_data->early_fragment_tests)
      return false;

   /* 3DSTATE_WM::ForceKillPix != ForceOff &&
    * (3DSTATE_PS_EXTRA::PixelShaderKillsPixels ||
    *  3DSTATE_PS_EXTRA::oMask Present to RenderTarget ||
    *  3DSTATE_PS_BLEND::AlphaToCoverageEnable ||
    *  3DSTATE_PS_BLEND::AlphaTestEnable ||
    *  3DSTATE_WM_CHROMAKEY::ChromaKeyKillEnable)
    */
   bool killpixels = wm_prog_data->uses_kill || wm_prog_data->uses_omask ||
                     cso_blend->alpha_to_coverage || cso_zsa->alpha_enabled;

   /* The Gfx8 depth PMA equation becomes:
    *
    *    depth_writes =
    *       3DSTATE_WM_DEPTH_STENCIL::DepthWriteEnable &&
    *       3DSTATE_DEPTH_BUFFER::DEPTH_WRITE_ENABLE
    *
    *    stencil_writes =
    *       3DSTATE_WM_DEPTH_STENCIL::Stencil Buffer Write Enable &&
    *       3DSTATE_DEPTH_BUFFER::STENCIL_WRITE_ENABLE &&
    *       3DSTATE_STENCIL_BUFFER::STENCIL_BUFFER_ENABLE
    *
    *    Z_PMA_OPT =
    *       common_pma_fix &&
    *       3DSTATE_WM_DEPTH_STENCIL::DepthTestEnable &&
    *       ((killpixels && (depth_writes || stencil_writes)) ||
    *        3DSTATE_PS_EXTRA::PixelShaderComputedDepthMode != PSCDEPTH_OFF)
    *
    */
   if (!cso_zsa->depth_test_enabled)
      return false;

   return wm_prog_data->computed_depth_mode != PSCDEPTH_OFF ||
          (killpixels && (cso_zsa->depth_writes_enabled ||
                          (sres && cso_zsa->stencil_writes_enabled)));
}
#endif

void
genX(update_pma_fix)(struct iris_context *ice,
                     struct iris_batch *batch,
                     bool enable)
{
#if GFX_VER == 8
   struct iris_genx_state *genx = ice->state.genx;

   if (genx->pma_fix_enabled == enable)
      return;

   genx->pma_fix_enabled = enable;

   /* According to the Broadwell PIPE_CONTROL documentation, software should
    * emit a PIPE_CONTROL with the CS Stall and Depth Cache Flush bits set
    * prior to the LRI.  If stencil buffer writes are enabled, then a Render        * Cache Flush is also necessary.
    *
    * The Gfx9 docs say to use a depth stall rather than a command streamer
    * stall.  However, the hardware seems to violently disagree.  A full
    * command streamer stall seems to be needed in both cases.
    */
   iris_emit_pipe_control_flush(batch, "PMA fix change (1/2)",
                                PIPE_CONTROL_CS_STALL |
                                PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                PIPE_CONTROL_RENDER_TARGET_FLUSH);

   iris_emit_reg(batch, GENX(CACHE_MODE_1), reg) {
      reg.NPPMAFixEnable = enable;
      reg.NPEarlyZFailsDisable = enable;
      reg.NPPMAFixEnableMask = true;
      reg.NPEarlyZFailsDisableMask = true;
   }

   /* After the LRI, a PIPE_CONTROL with both the Depth Stall and Depth Cache
    * Flush bits is often necessary.  We do it regardless because it's easier.
    * The render cache flush is also necessary if stencil writes are enabled.
    *
    * Again, the Gfx9 docs give a different set of flushes but the Broadwell
    * flushes seem to work just as well.
    */
   iris_emit_pipe_control_flush(batch, "PMA fix change (1/2)",
                                PIPE_CONTROL_DEPTH_STALL |
                                PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                PIPE_CONTROL_RENDER_TARGET_FLUSH);
#endif
}

/**
 * Gallium CSO for rasterizer state.
 */
struct iris_rasterizer_state {
   uint32_t sf[GENX(3DSTATE_SF_length)];
   uint32_t clip[GENX(3DSTATE_CLIP_length)];
   uint32_t raster[GENX(3DSTATE_RASTER_length)];
   uint32_t wm[GENX(3DSTATE_WM_length)];
   uint32_t line_stipple[GENX(3DSTATE_LINE_STIPPLE_length)];

   uint8_t num_clip_plane_consts;
   bool clip_halfz; /* for CC_VIEWPORT */
   bool depth_clip_near; /* for CC_VIEWPORT */
   bool depth_clip_far; /* for CC_VIEWPORT */
   bool flatshade; /* for shader state */
   bool flatshade_first; /* for stream output */
   bool clamp_fragment_color; /* for shader state */
   bool light_twoside; /* for shader state */
   bool rasterizer_discard; /* for 3DSTATE_STREAMOUT and 3DSTATE_CLIP */
   bool half_pixel_center; /* for 3DSTATE_MULTISAMPLE */
   bool line_smooth;
   bool line_stipple_enable;
   bool poly_stipple_enable;
   bool multisample;
   bool force_persample_interp;
   bool conservative_rasterization;
   bool fill_mode_point;
   bool fill_mode_line;
   bool fill_mode_point_or_line;
   enum pipe_sprite_coord_mode sprite_coord_mode; /* PIPE_SPRITE_* */
   uint16_t sprite_coord_enable;
};

static float
get_line_width(const struct pipe_rasterizer_state *state)
{
   float line_width = state->line_width;

   /* From the OpenGL 4.4 spec:
    *
    * "The actual width of non-antialiased lines is determined by rounding
    *  the supplied width to the nearest integer, then clamping it to the
    *  implementation-dependent maximum non-antialiased line width."
    */
   if (!state->multisample && !state->line_smooth)
      line_width = roundf(state->line_width);

   if (!state->multisample && state->line_smooth && line_width < 1.5f) {
      /* For 1 pixel line thickness or less, the general anti-aliasing
       * algorithm gives up, and a garbage line is generated.  Setting a
       * Line Width of 0.0 specifies the rasterization of the "thinnest"
       * (one-pixel-wide), non-antialiased lines.
       *
       * Lines rendered with zero Line Width are rasterized using the
       * "Grid Intersection Quantization" rules as specified by the
       * "Zero-Width (Cosmetic) Line Rasterization" section of the docs.
       */
      line_width = 0.0f;
   }

   return line_width;
}

/**
 * The pipe->create_rasterizer_state() driver hook.
 */
static void *
iris_create_rasterizer_state(struct pipe_context *ctx,
                             const struct pipe_rasterizer_state *state)
{
   struct iris_rasterizer_state *cso =
      malloc(sizeof(struct iris_rasterizer_state));

   cso->multisample = state->multisample;
   cso->force_persample_interp = state->force_persample_interp;
   cso->clip_halfz = state->clip_halfz;
   cso->depth_clip_near = state->depth_clip_near;
   cso->depth_clip_far = state->depth_clip_far;
   cso->flatshade = state->flatshade;
   cso->flatshade_first = state->flatshade_first;
   cso->clamp_fragment_color = state->clamp_fragment_color;
   cso->light_twoside = state->light_twoside;
   cso->rasterizer_discard = state->rasterizer_discard;
   cso->half_pixel_center = state->half_pixel_center;
   cso->sprite_coord_mode = state->sprite_coord_mode;
   cso->sprite_coord_enable = state->sprite_coord_enable;
   cso->line_smooth = state->line_smooth;
   cso->line_stipple_enable = state->line_stipple_enable;
   cso->poly_stipple_enable = state->poly_stipple_enable;
   cso->conservative_rasterization =
      state->conservative_raster_mode == PIPE_CONSERVATIVE_RASTER_POST_SNAP;

   cso->fill_mode_point =
      state->fill_front == PIPE_POLYGON_MODE_POINT ||
      state->fill_back == PIPE_POLYGON_MODE_POINT;
   cso->fill_mode_line =
      state->fill_front == PIPE_POLYGON_MODE_LINE ||
      state->fill_back == PIPE_POLYGON_MODE_LINE;
   cso->fill_mode_point_or_line =
      cso->fill_mode_point ||
      cso->fill_mode_line;

   if (state->clip_plane_enable != 0)
      cso->num_clip_plane_consts = util_logbase2(state->clip_plane_enable) + 1;
   else
      cso->num_clip_plane_consts = 0;

   float line_width = get_line_width(state);

   iris_pack_command(GENX(3DSTATE_SF), cso->sf, sf) {
      sf.StatisticsEnable = true;
      sf.AALineDistanceMode = AALINEDISTANCE_TRUE;
      sf.LineEndCapAntialiasingRegionWidth =
         state->line_smooth ? _10pixels : _05pixels;
      sf.LastPixelEnable = state->line_last_pixel;
      sf.LineWidth = line_width;
      sf.SmoothPointEnable = (state->point_smooth || state->multisample) &&
                             !state->point_quad_rasterization;
      sf.PointWidthSource = state->point_size_per_vertex ? Vertex : State;
      sf.PointWidth = CLAMP(state->point_size, 0.125f, 255.875f);

      if (state->flatshade_first) {
         sf.TriangleFanProvokingVertexSelect = 1;
      } else {
         sf.TriangleStripListProvokingVertexSelect = 2;
         sf.TriangleFanProvokingVertexSelect = 2;
         sf.LineStripListProvokingVertexSelect = 1;
      }
   }

   iris_pack_command(GENX(3DSTATE_RASTER), cso->raster, rr) {
      rr.FrontWinding = state->front_ccw ? CounterClockwise : Clockwise;
      rr.CullMode = translate_cull_mode(state->cull_face);
      rr.FrontFaceFillMode = translate_fill_mode(state->fill_front);
      rr.BackFaceFillMode = translate_fill_mode(state->fill_back);
      rr.DXMultisampleRasterizationEnable = state->multisample;
      rr.GlobalDepthOffsetEnableSolid = state->offset_tri;
      rr.GlobalDepthOffsetEnableWireframe = state->offset_line;
      rr.GlobalDepthOffsetEnablePoint = state->offset_point;
      rr.GlobalDepthOffsetConstant = state->offset_units * 2;
      rr.GlobalDepthOffsetScale = state->offset_scale;
      rr.GlobalDepthOffsetClamp = state->offset_clamp;
      rr.SmoothPointEnable = state->point_smooth;
      rr.ScissorRectangleEnable = state->scissor;
#if GFX_VER >= 9
      rr.ViewportZNearClipTestEnable = state->depth_clip_near;
      rr.ViewportZFarClipTestEnable = state->depth_clip_far;
      rr.ConservativeRasterizationEnable =
         cso->conservative_rasterization;
#else
      rr.ViewportZClipTestEnable = (state->depth_clip_near || state->depth_clip_far);
#endif
   }

   iris_pack_command(GENX(3DSTATE_CLIP), cso->clip, cl) {
      /* cl.NonPerspectiveBarycentricEnable is filled in at draw time from
       * the FS program; cl.ForceZeroRTAIndexEnable is filled in from the FB.
       */
      cl.EarlyCullEnable = true;
      cl.UserClipDistanceClipTestEnableBitmask = state->clip_plane_enable;
      cl.ForceUserClipDistanceClipTestEnableBitmask = true;
      cl.APIMode = state->clip_halfz ? APIMODE_D3D : APIMODE_OGL;
      cl.GuardbandClipTestEnable = true;
      cl.ClipEnable = true;
      cl.MinimumPointWidth = 0.125;
      cl.MaximumPointWidth = 255.875;

      if (state->flatshade_first) {
         cl.TriangleFanProvokingVertexSelect = 1;
      } else {
         cl.TriangleStripListProvokingVertexSelect = 2;
         cl.TriangleFanProvokingVertexSelect = 2;
         cl.LineStripListProvokingVertexSelect = 1;
      }
   }

   iris_pack_command(GENX(3DSTATE_WM), cso->wm, wm) {
      /* wm.BarycentricInterpolationMode and wm.EarlyDepthStencilControl are
       * filled in at draw time from the FS program.
       */
      wm.LineAntialiasingRegionWidth = _10pixels;
      wm.LineEndCapAntialiasingRegionWidth = _05pixels;
      wm.PointRasterizationRule = RASTRULE_UPPER_RIGHT;
      wm.LineStippleEnable = state->line_stipple_enable;
      wm.PolygonStippleEnable = state->poly_stipple_enable;
   }

   /* Remap from 0..255 back to 1..256 */
   const unsigned line_stipple_factor = state->line_stipple_factor + 1;

   iris_pack_command(GENX(3DSTATE_LINE_STIPPLE), cso->line_stipple, line) {
      if (state->line_stipple_enable) {
         line.LineStipplePattern = state->line_stipple_pattern;
         line.LineStippleInverseRepeatCount = 1.0f / line_stipple_factor;
         line.LineStippleRepeatCount = line_stipple_factor;
      }
   }

   return cso;
}

/**
 * The pipe->bind_rasterizer_state() driver hook.
 *
 * Bind a rasterizer CSO and flag related dirty bits.
 */
static void
iris_bind_rasterizer_state(struct pipe_context *ctx, void *state)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_rasterizer_state *old_cso = ice->state.cso_rast;
   struct iris_rasterizer_state *new_cso = state;

   if (new_cso) {
      /* Try to avoid re-emitting 3DSTATE_LINE_STIPPLE, it's non-pipelined */
      if (cso_changed_memcmp(line_stipple))
         ice->state.dirty |= IRIS_DIRTY_LINE_STIPPLE;

      if (cso_changed(half_pixel_center))
         ice->state.dirty |= IRIS_DIRTY_MULTISAMPLE;

      if (cso_changed(line_stipple_enable) || cso_changed(poly_stipple_enable))
         ice->state.dirty |= IRIS_DIRTY_WM;

      if (cso_changed(rasterizer_discard))
         ice->state.dirty |= IRIS_DIRTY_STREAMOUT | IRIS_DIRTY_CLIP;

      if (cso_changed(flatshade_first))
         ice->state.dirty |= IRIS_DIRTY_STREAMOUT;

      if (cso_changed(depth_clip_near) || cso_changed(depth_clip_far) ||
          cso_changed(clip_halfz))
         ice->state.dirty |= IRIS_DIRTY_CC_VIEWPORT;

      if (cso_changed(sprite_coord_enable) ||
          cso_changed(sprite_coord_mode) ||
          cso_changed(light_twoside))
         ice->state.dirty |= IRIS_DIRTY_SBE;

      if (cso_changed(conservative_rasterization))
         ice->state.stage_dirty |= IRIS_STAGE_DIRTY_FS;
   }

   ice->state.cso_rast = new_cso;
   ice->state.dirty |= IRIS_DIRTY_RASTER;
   ice->state.dirty |= IRIS_DIRTY_CLIP;
   ice->state.stage_dirty |=
      ice->state.stage_dirty_for_nos[IRIS_NOS_RASTERIZER];
}

/**
 * Return true if the given wrap mode requires the border color to exist.
 *
 * (We can skip uploading it if the sampler isn't going to use it.)
 */
static bool
wrap_mode_needs_border_color(unsigned wrap_mode)
{
   return wrap_mode == TCM_CLAMP_BORDER || wrap_mode == TCM_HALF_BORDER;
}

/**
 * Gallium CSO for sampler state.
 */
struct iris_sampler_state {
   union pipe_color_union border_color;
   bool needs_border_color;

   uint32_t sampler_state[GENX(SAMPLER_STATE_length)];

#if GFX_VERx10 == 125
   /* Sampler state structure to use for 3D textures in order to
    * implement Wa_14014414195.
    */
   uint32_t sampler_state_3d[GENX(SAMPLER_STATE_length)];
#endif
};

static void
fill_sampler_state(uint32_t *sampler_state,
                   const struct pipe_sampler_state *state,
                   unsigned max_anisotropy)
{
   float min_lod = state->min_lod;
   unsigned mag_img_filter = state->mag_img_filter;

   // XXX: explain this code ported from ilo...I don't get it at all...
   if (state->min_mip_filter == PIPE_TEX_MIPFILTER_NONE &&
       state->min_lod > 0.0f) {
      min_lod = 0.0f;
      mag_img_filter = state->min_img_filter;
   }

   iris_pack_state(GENX(SAMPLER_STATE), sampler_state, samp) {
      samp.TCXAddressControlMode = translate_wrap(state->wrap_s);
      samp.TCYAddressControlMode = translate_wrap(state->wrap_t);
      samp.TCZAddressControlMode = translate_wrap(state->wrap_r);
      samp.CubeSurfaceControlMode = state->seamless_cube_map;
      samp.NonnormalizedCoordinateEnable = state->unnormalized_coords;
      samp.MinModeFilter = state->min_img_filter;
      samp.MagModeFilter = mag_img_filter;
      samp.MipModeFilter = translate_mip_filter(state->min_mip_filter);
      samp.MaximumAnisotropy = RATIO21;

      if (max_anisotropy >= 2) {
         if (state->min_img_filter == PIPE_TEX_FILTER_LINEAR) {
            samp.MinModeFilter = MAPFILTER_ANISOTROPIC;
            samp.AnisotropicAlgorithm = EWAApproximation;
         }

         if (state->mag_img_filter == PIPE_TEX_FILTER_LINEAR)
            samp.MagModeFilter = MAPFILTER_ANISOTROPIC;

         samp.MaximumAnisotropy =
            MIN2((max_anisotropy - 2) / 2, RATIO161);
      }

      /* Set address rounding bits if not using nearest filtering. */
      if (state->min_img_filter != PIPE_TEX_FILTER_NEAREST) {
         samp.UAddressMinFilterRoundingEnable = true;
         samp.VAddressMinFilterRoundingEnable = true;
         samp.RAddressMinFilterRoundingEnable = true;
      }

      if (state->mag_img_filter != PIPE_TEX_FILTER_NEAREST) {
         samp.UAddressMagFilterRoundingEnable = true;
         samp.VAddressMagFilterRoundingEnable = true;
         samp.RAddressMagFilterRoundingEnable = true;
      }

      if (state->compare_mode == PIPE_TEX_COMPARE_R_TO_TEXTURE)
         samp.ShadowFunction = translate_shadow_func(state->compare_func);

      const float hw_max_lod = GFX_VER >= 7 ? 14 : 13;

      samp.LODPreClampMode = CLAMP_MODE_OGL;
      samp.MinLOD = CLAMP(min_lod, 0, hw_max_lod);
      samp.MaxLOD = CLAMP(state->max_lod, 0, hw_max_lod);
      samp.TextureLODBias = CLAMP(state->lod_bias, -16, 15);

      /* .BorderColorPointer is filled in by iris_bind_sampler_states. */
   }
}

/**
 * The pipe->create_sampler_state() driver hook.
 *
 * We fill out SAMPLER_STATE (except for the border color pointer), and
 * store that on the CPU.  It doesn't make sense to upload it to a GPU
 * buffer object yet, because 3DSTATE_SAMPLER_STATE_POINTERS requires
 * all bound sampler states to be in contiguous memor.
 */
static void *
iris_create_sampler_state(struct pipe_context *ctx,
                          const struct pipe_sampler_state *state)
{
   UNUSED struct iris_screen *screen = (void *)ctx->screen;
   UNUSED const struct intel_device_info *devinfo = screen->devinfo;
   struct iris_sampler_state *cso = CALLOC_STRUCT(iris_sampler_state);

   if (!cso)
      return NULL;

   STATIC_ASSERT(PIPE_TEX_FILTER_NEAREST == MAPFILTER_NEAREST);
   STATIC_ASSERT(PIPE_TEX_FILTER_LINEAR == MAPFILTER_LINEAR);

   unsigned wrap_s = translate_wrap(state->wrap_s);
   unsigned wrap_t = translate_wrap(state->wrap_t);
   unsigned wrap_r = translate_wrap(state->wrap_r);

   memcpy(&cso->border_color, &state->border_color, sizeof(cso->border_color));

   cso->needs_border_color = wrap_mode_needs_border_color(wrap_s) ||
                             wrap_mode_needs_border_color(wrap_t) ||
                             wrap_mode_needs_border_color(wrap_r);

   fill_sampler_state(cso->sampler_state, state, state->max_anisotropy);

#if GFX_VERx10 == 125
   /* Fill an extra sampler state structure with anisotropic filtering
    * disabled used to implement Wa_14014414195.
    */
   if (intel_needs_workaround(screen->devinfo, 14014414195))
      fill_sampler_state(cso->sampler_state_3d, state, 0);
#endif

   return cso;
}

/**
 * The pipe->bind_sampler_states() driver hook.
 */
static void
iris_bind_sampler_states(struct pipe_context *ctx,
                         enum pipe_shader_type p_stage,
                         unsigned start, unsigned count,
                         void **states)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct iris_shader_state *shs = &ice->state.shaders[stage];

   assert(start + count <= IRIS_MAX_SAMPLERS);

   bool dirty = false;

   for (int i = 0; i < count; i++) {
      struct iris_sampler_state *state = states ? states[i] : NULL;
      if (shs->samplers[start + i] != state) {
         shs->samplers[start + i] = state;
         dirty = true;
      }
   }

   if (dirty)
      ice->state.stage_dirty |= IRIS_STAGE_DIRTY_SAMPLER_STATES_VS << stage;
}

/**
 * Upload the sampler states into a contiguous area of GPU memory, for
 * for 3DSTATE_SAMPLER_STATE_POINTERS_*.
 *
 * Also fill out the border color state pointers.
 */
static void
iris_upload_sampler_states(struct iris_context *ice, gl_shader_stage stage)
{
   struct iris_screen *screen = (struct iris_screen *) ice->ctx.screen;
   struct iris_compiled_shader *shader = ice->shaders.prog[stage];
   struct iris_shader_state *shs = &ice->state.shaders[stage];
   struct iris_border_color_pool *border_color_pool =
      iris_bufmgr_get_border_color_pool(screen->bufmgr);

   /* We assume gallium frontends will call pipe->bind_sampler_states()
    * if the program's number of textures changes.
    */
   unsigned count = util_last_bit64(shader->bt.samplers_used_mask);

   if (!count)
      return;

   /* Assemble the SAMPLER_STATEs into a contiguous table that lives
    * in the dynamic state memory zone, so we can point to it via the
    * 3DSTATE_SAMPLER_STATE_POINTERS_* commands.
    */
   unsigned size = count * 4 * GENX(SAMPLER_STATE_length);
   uint32_t *map =
      upload_state(ice->state.dynamic_uploader, &shs->sampler_table, size, 32);
   if (unlikely(!map))
      return;

   struct pipe_resource *res = shs->sampler_table.res;
   struct iris_bo *bo = iris_resource_bo(res);

   iris_record_state_size(ice->state.sizes,
                          bo->address + shs->sampler_table.offset, size);

   shs->sampler_table.offset += iris_bo_offset_from_base_address(bo);

   ice->state.need_border_colors &= ~(1 << stage);

   for (int i = 0; i < count; i++) {
      struct iris_sampler_state *state = shs->samplers[i];
      struct iris_sampler_view *tex = shs->textures[i];

      if (!state) {
         memset(map, 0, 4 * GENX(SAMPLER_STATE_length));
      } else {
         const uint32_t *sampler_state = state->sampler_state;

#if GFX_VERx10 == 125
         if (intel_needs_workaround(screen->devinfo, 14014414195) &&
             tex && tex->res->base.b.target == PIPE_TEXTURE_3D) {
               sampler_state = state->sampler_state_3d;
         }
#endif

         if (!state->needs_border_color) {
            memcpy(map, sampler_state, 4 * GENX(SAMPLER_STATE_length));
         } else {
            ice->state.need_border_colors |= 1 << stage;

            /* We may need to swizzle the border color for format faking.
             * A/LA formats are faked as R/RG with 000R or R00G swizzles.
             * This means we need to move the border color's A channel into
             * the R or G channels so that those read swizzles will move it
             * back into A.
             */
            union pipe_color_union *color = &state->border_color;
            union pipe_color_union tmp;
            if (tex) {
               enum pipe_format internal_format = tex->res->internal_format;

               if (util_format_is_alpha(internal_format)) {
                  unsigned char swz[4] = {
                     PIPE_SWIZZLE_W, PIPE_SWIZZLE_0,
                     PIPE_SWIZZLE_0, PIPE_SWIZZLE_0
                  };
                  util_format_apply_color_swizzle(&tmp, color, swz, true);
                  color = &tmp;
               } else if (util_format_is_luminance_alpha(internal_format) &&
                          internal_format != PIPE_FORMAT_L8A8_SRGB) {
                  unsigned char swz[4] = {
                     PIPE_SWIZZLE_X, PIPE_SWIZZLE_W,
                     PIPE_SWIZZLE_0, PIPE_SWIZZLE_0
                  };
                  util_format_apply_color_swizzle(&tmp, color, swz, true);
                  color = &tmp;
               }
            }

            /* Stream out the border color and merge the pointer. */
            uint32_t offset = iris_upload_border_color(border_color_pool,
                                                       color);

            uint32_t dynamic[GENX(SAMPLER_STATE_length)];
            iris_pack_state(GENX(SAMPLER_STATE), dynamic, dyns) {
               dyns.BorderColorPointer = offset;
            }

            for (uint32_t j = 0; j < GENX(SAMPLER_STATE_length); j++)
               map[j] = sampler_state[j] | dynamic[j];
         }
      }

      map += GENX(SAMPLER_STATE_length);
   }
}

static enum isl_channel_select
fmt_swizzle(const struct iris_format_info *fmt, enum pipe_swizzle swz)
{
   switch (swz) {
   case PIPE_SWIZZLE_X: return fmt->swizzle.r;
   case PIPE_SWIZZLE_Y: return fmt->swizzle.g;
   case PIPE_SWIZZLE_Z: return fmt->swizzle.b;
   case PIPE_SWIZZLE_W: return fmt->swizzle.a;
   case PIPE_SWIZZLE_1: return ISL_CHANNEL_SELECT_ONE;
   case PIPE_SWIZZLE_0: return ISL_CHANNEL_SELECT_ZERO;
   default: unreachable("invalid swizzle");
   }
}

static void
fill_buffer_surface_state(struct isl_device *isl_dev,
                          struct iris_resource *res,
                          void *map,
                          enum isl_format format,
                          struct isl_swizzle swizzle,
                          unsigned offset,
                          unsigned size,
                          isl_surf_usage_flags_t usage)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(format);
   const unsigned cpp = format == ISL_FORMAT_RAW ? 1 : fmtl->bpb / 8;

   /* The ARB_texture_buffer_specification says:
    *
    *    "The number of texels in the buffer texture's texel array is given by
    *
    *       floor(<buffer_size> / (<components> * sizeof(<base_type>)),
    *
    *     where <buffer_size> is the size of the buffer object, in basic
    *     machine units and <components> and <base_type> are the element count
    *     and base data type for elements, as specified in Table X.1.  The
    *     number of texels in the texel array is then clamped to the
    *     implementation-dependent limit MAX_TEXTURE_BUFFER_SIZE_ARB."
    *
    * We need to clamp the size in bytes to MAX_TEXTURE_BUFFER_SIZE * stride,
    * so that when ISL divides by stride to obtain the number of texels, that
    * texel count is clamped to MAX_TEXTURE_BUFFER_SIZE.
    */
   unsigned final_size =
      MIN3(size, res->bo->size - res->offset - offset,
           IRIS_MAX_TEXTURE_BUFFER_SIZE * cpp);

   isl_buffer_fill_state(isl_dev, map,
                         .address = res->bo->address + res->offset + offset,
                         .size_B = final_size,
                         .format = format,
                         .swizzle = swizzle,
                         .stride_B = cpp,
                         .mocs = iris_mocs(res->bo, isl_dev, usage));
}

#define SURFACE_STATE_ALIGNMENT 64

/**
 * Allocate several contiguous SURFACE_STATE structures, one for each
 * supported auxiliary surface mode.  This only allocates the CPU-side
 * copy, they will need to be uploaded later after they're filled in.
 */
static void
alloc_surface_states(struct iris_surface_state *surf_state,
                     unsigned aux_usages)
{
   enum { surf_size = 4 * GENX(RENDER_SURFACE_STATE_length) };

   /* If this changes, update this to explicitly align pointers */
   STATIC_ASSERT(surf_size == SURFACE_STATE_ALIGNMENT);

   assert(aux_usages != 0);

   /* In case we're re-allocating them... */
   free(surf_state->cpu);

   surf_state->aux_usages = aux_usages;
   surf_state->num_states = util_bitcount(aux_usages);
   surf_state->cpu = calloc(surf_state->num_states, surf_size);
   surf_state->ref.offset = 0;
   pipe_resource_reference(&surf_state->ref.res, NULL);

   assert(surf_state->cpu);
}

/**
 * Upload the CPU side SURFACE_STATEs into a GPU buffer.
 */
static void
upload_surface_states(struct u_upload_mgr *mgr,
                      struct iris_surface_state *surf_state)
{
   const unsigned surf_size = 4 * GENX(RENDER_SURFACE_STATE_length);
   const unsigned bytes = surf_state->num_states * surf_size;

   void *map =
      upload_state(mgr, &surf_state->ref, bytes, SURFACE_STATE_ALIGNMENT);

   surf_state->ref.offset +=
      iris_bo_offset_from_base_address(iris_resource_bo(surf_state->ref.res));

   if (map)
      memcpy(map, surf_state->cpu, bytes);
}

/**
 * Update resource addresses in a set of SURFACE_STATE descriptors,
 * and re-upload them if necessary.
 */
static bool
update_surface_state_addrs(struct u_upload_mgr *mgr,
                           struct iris_surface_state *surf_state,
                           struct iris_bo *bo)
{
   if (surf_state->bo_address == bo->address)
      return false;

   STATIC_ASSERT(GENX(RENDER_SURFACE_STATE_SurfaceBaseAddress_start) % 64 == 0);
   STATIC_ASSERT(GENX(RENDER_SURFACE_STATE_SurfaceBaseAddress_bits) == 64);

   uint64_t *ss_addr = (uint64_t *) &surf_state->cpu[GENX(RENDER_SURFACE_STATE_SurfaceBaseAddress_start) / 32];

   /* First, update the CPU copies.  We assume no other fields exist in
    * the QWord containing Surface Base Address.
    */
   for (unsigned i = 0; i < surf_state->num_states; i++) {
      *ss_addr = *ss_addr - surf_state->bo_address + bo->address;
      ss_addr = ((void *) ss_addr) + SURFACE_STATE_ALIGNMENT;
   }

   /* Next, upload the updated copies to a GPU buffer. */
   upload_surface_states(mgr, surf_state);

   surf_state->bo_address = bo->address;

   return true;
}

/* We should only use this function when it's needed to fill out
 * surf with information provided by the pipe_(image|sampler)_view.
 * This is only necessary for CL extension cl_khr_image2d_from_buffer.
 * This is the reason why ISL_SURF_DIM_2D is hardcoded on dim field.
 */
static void
fill_surf_for_tex2d_from_buffer(struct isl_device *isl_dev,
                                enum isl_format format,
                                unsigned width,
                                unsigned height,
                                unsigned row_stride,
                                isl_surf_usage_flags_t usage,
                                struct isl_surf *surf)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(format);
   const unsigned cpp = format == ISL_FORMAT_RAW ? 1 : fmtl->bpb / 8;

   const struct isl_surf_init_info init_info = {
      .dim = ISL_SURF_DIM_2D,
      .format = format,
      .width = width,
      .height = height,
      .depth = 1,
      .levels = 1,
      .array_len = 1,
      .samples = 1,
      .min_alignment_B = 4,
      .row_pitch_B = row_stride * cpp,
      .usage = usage,
      .tiling_flags = ISL_TILING_LINEAR_BIT,
   };

   const bool isl_surf_created_successfully =
      isl_surf_init_s(isl_dev, surf, &init_info);

   assert(isl_surf_created_successfully);
}

static void
fill_surface_state(struct isl_device *isl_dev,
                   void *map,
                   struct iris_resource *res,
                   struct isl_surf *surf,
                   struct isl_view *view,
                   unsigned aux_usage,
                   uint32_t extra_main_offset,
                   uint32_t tile_x_sa,
                   uint32_t tile_y_sa)
{
   struct isl_surf_fill_state_info f = {
      .surf = surf,
      .view = view,
      .mocs = iris_mocs(res->bo, isl_dev, view->usage),
      .address = res->bo->address + res->offset + extra_main_offset,
      .x_offset_sa = tile_x_sa,
      .y_offset_sa = tile_y_sa,
   };

   if (aux_usage != ISL_AUX_USAGE_NONE) {
      f.aux_surf = &res->aux.surf;
      f.aux_usage = aux_usage;
      f.clear_color = res->aux.clear_color;

      if (aux_usage == ISL_AUX_USAGE_MC)
         f.mc_format = iris_format_for_usage(isl_dev->info,
                                             res->external_format,
                                             surf->usage).fmt;

      if (res->aux.bo)
         f.aux_address = res->aux.bo->address + res->aux.offset;

      if (res->aux.clear_color_bo) {
         f.clear_address = res->aux.clear_color_bo->address +
                           res->aux.clear_color_offset;
         f.use_clear_address = isl_dev->info->ver > 9;
      }
   }

   isl_surf_fill_state_s(isl_dev, map, &f);
}

static void
fill_surface_states(struct isl_device *isl_dev,
                    struct iris_surface_state *surf_state,
                    struct iris_resource *res,
                    struct isl_surf *surf,
                    struct isl_view *view,
                    uint64_t extra_main_offset,
                    uint32_t tile_x_sa,
                    uint32_t tile_y_sa)
{
   void *map = surf_state->cpu;
   unsigned aux_modes = surf_state->aux_usages;

   while (aux_modes) {
      enum isl_aux_usage aux_usage = u_bit_scan(&aux_modes);

      fill_surface_state(isl_dev, map, res, surf, view, aux_usage,
                         extra_main_offset, tile_x_sa, tile_y_sa);

      map += SURFACE_STATE_ALIGNMENT;
   }
}

/**
 * The pipe->create_sampler_view() driver hook.
 */
static struct pipe_sampler_view *
iris_create_sampler_view(struct pipe_context *ctx,
                         struct pipe_resource *tex,
                         const struct pipe_sampler_view *tmpl)
{
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;
   const struct intel_device_info *devinfo = screen->devinfo;
   struct iris_sampler_view *isv = calloc(1, sizeof(struct iris_sampler_view));

   if (!isv)
      return NULL;

   /* initialize base object */
   isv->base = *tmpl;
   isv->base.context = ctx;
   isv->base.texture = NULL;
   pipe_reference_init(&isv->base.reference, 1);
   pipe_resource_reference(&isv->base.texture, tex);

   if (util_format_is_depth_or_stencil(tmpl->format)) {
      struct iris_resource *zres, *sres;
      const struct util_format_description *desc =
         util_format_description(tmpl->format);

      iris_get_depth_stencil_resources(tex, &zres, &sres);

      tex = util_format_has_depth(desc) ? &zres->base.b : &sres->base.b;
   }

   isv->res = (struct iris_resource *) tex;

   isl_surf_usage_flags_t usage = ISL_SURF_USAGE_TEXTURE_BIT;

   if (isv->base.target == PIPE_TEXTURE_CUBE ||
       isv->base.target == PIPE_TEXTURE_CUBE_ARRAY)
      usage |= ISL_SURF_USAGE_CUBE_BIT;

   const struct iris_format_info fmt =
      iris_format_for_usage(devinfo, tmpl->format, usage);

   isv->clear_color = isv->res->aux.clear_color;

   isv->view = (struct isl_view) {
      .format = fmt.fmt,
      .swizzle = (struct isl_swizzle) {
         .r = fmt_swizzle(&fmt, tmpl->swizzle_r),
         .g = fmt_swizzle(&fmt, tmpl->swizzle_g),
         .b = fmt_swizzle(&fmt, tmpl->swizzle_b),
         .a = fmt_swizzle(&fmt, tmpl->swizzle_a),
      },
      .usage = usage,
   };

   unsigned aux_usages = 0;

   if ((isv->res->aux.usage == ISL_AUX_USAGE_CCS_D ||
        isv->res->aux.usage == ISL_AUX_USAGE_CCS_E ||
        isv->res->aux.usage == ISL_AUX_USAGE_FCV_CCS_E) &&
       !isl_format_supports_ccs_e(devinfo, isv->view.format)) {
      aux_usages = 1 << ISL_AUX_USAGE_NONE;
   } else if (isl_aux_usage_has_hiz(isv->res->aux.usage) &&
              !iris_sample_with_depth_aux(devinfo, isv->res)) {
      aux_usages = 1 << ISL_AUX_USAGE_NONE;
   } else {
      aux_usages = 1 << ISL_AUX_USAGE_NONE |
                   1 << isv->res->aux.usage;
   }

   alloc_surface_states(&isv->surface_state, aux_usages);
   isv->surface_state.bo_address = isv->res->bo->address;

   /* Fill out SURFACE_STATE for this view. */
   if (tmpl->target != PIPE_BUFFER) {
      isv->view.base_level = tmpl->u.tex.first_level;
      isv->view.levels = tmpl->u.tex.last_level - tmpl->u.tex.first_level + 1;

      if (tmpl->target == PIPE_TEXTURE_3D) {
         isv->view.base_array_layer = 0;
         isv->view.array_len = 1;
      } else {
#if GFX_VER < 9
         /* Hardware older than skylake ignores this value */
         assert(tex->target != PIPE_TEXTURE_3D || !tmpl->u.tex.first_layer);
#endif
         isv->view.base_array_layer = tmpl->u.tex.first_layer;
         isv->view.array_len =
            tmpl->u.tex.last_layer - tmpl->u.tex.first_layer + 1;
      }

      fill_surface_states(&screen->isl_dev, &isv->surface_state, isv->res,
                          &isv->res->surf, &isv->view, 0, 0, 0);
   } else if (isv->base.is_tex2d_from_buf) {
      /* In case it's a 2d image created from a buffer, we should
       * use fill_surface_states function with image parameters provided
       * by the CL application
       */
      isv->view.base_array_layer = 0;
      isv->view.array_len = 1;

      /* Create temp_surf and fill with values provided by CL application */
      struct isl_surf temp_surf;
      fill_surf_for_tex2d_from_buffer(&screen->isl_dev, fmt.fmt,
                                      isv->base.u.tex2d_from_buf.width,
                                      isv->base.u.tex2d_from_buf.height,
                                      isv->base.u.tex2d_from_buf.row_stride,
                                      usage,
                                      &temp_surf);

      fill_surface_states(&screen->isl_dev, &isv->surface_state, isv->res,
                          &temp_surf, &isv->view, 0, 0, 0);
   } else {
      fill_buffer_surface_state(&screen->isl_dev, isv->res,
                                isv->surface_state.cpu,
                                isv->view.format, isv->view.swizzle,
                                tmpl->u.buf.offset, tmpl->u.buf.size,
                                ISL_SURF_USAGE_TEXTURE_BIT);
   }

   return &isv->base;
}

static void
iris_sampler_view_destroy(struct pipe_context *ctx,
                          struct pipe_sampler_view *state)
{
   struct iris_sampler_view *isv = (void *) state;
   pipe_resource_reference(&state->texture, NULL);
   pipe_resource_reference(&isv->surface_state.ref.res, NULL);
   free(isv->surface_state.cpu);
   free(isv);
}

/**
 * The pipe->create_surface() driver hook.
 *
 * In Gallium nomenclature, "surfaces" are a view of a resource that
 * can be bound as a render target or depth/stencil buffer.
 */
static struct pipe_surface *
iris_create_surface(struct pipe_context *ctx,
                    struct pipe_resource *tex,
                    const struct pipe_surface *tmpl)
{
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;
   const struct intel_device_info *devinfo = screen->devinfo;

   isl_surf_usage_flags_t usage = 0;
   if (tmpl->writable)
      usage = ISL_SURF_USAGE_STORAGE_BIT;
   else if (util_format_is_depth_or_stencil(tmpl->format))
      usage = ISL_SURF_USAGE_DEPTH_BIT;
   else
      usage = ISL_SURF_USAGE_RENDER_TARGET_BIT;

   const struct iris_format_info fmt =
      iris_format_for_usage(devinfo, tmpl->format, usage);

   if ((usage & ISL_SURF_USAGE_RENDER_TARGET_BIT) &&
       !isl_format_supports_rendering(devinfo, fmt.fmt)) {
      /* Framebuffer validation will reject this invalid case, but it
       * hasn't had the opportunity yet.  In the meantime, we need to
       * avoid hitting ISL asserts about unsupported formats below.
       */
      return NULL;
   }

   struct iris_surface *surf = calloc(1, sizeof(struct iris_surface));
   struct iris_resource *res = (struct iris_resource *) tex;

   if (!surf)
      return NULL;

   uint32_t array_len = tmpl->u.tex.last_layer - tmpl->u.tex.first_layer + 1;

   struct isl_view *view = &surf->view;
   *view = (struct isl_view) {
      .format = fmt.fmt,
      .base_level = tmpl->u.tex.level,
      .levels = 1,
      .base_array_layer = tmpl->u.tex.first_layer,
      .array_len = array_len,
      .swizzle = ISL_SWIZZLE_IDENTITY,
      .usage = usage,
   };

#if GFX_VER == 8
   struct isl_view *read_view = &surf->read_view;
   *read_view = (struct isl_view) {
      .format = fmt.fmt,
      .base_level = tmpl->u.tex.level,
      .levels = 1,
      .base_array_layer = tmpl->u.tex.first_layer,
      .array_len = array_len,
      .swizzle = ISL_SWIZZLE_IDENTITY,
      .usage = ISL_SURF_USAGE_TEXTURE_BIT,
   };

   struct isl_surf read_surf = res->surf;
   uint64_t read_surf_offset_B = 0;
   uint32_t read_surf_tile_x_sa = 0, read_surf_tile_y_sa = 0;
   if (tex->target == PIPE_TEXTURE_3D && array_len == 1) {
      /* The minimum array element field of the surface state structure is
       * ignored by the sampler unit for 3D textures on some hardware.  If the
       * render buffer is a single slice of a 3D texture, create a 2D texture
       * covering that slice.
       *
       * TODO: This only handles the case where we're rendering to a single
       * slice of an array texture.  If we have layered rendering combined
       * with non-coherent FB fetch and a non-zero base_array_layer, then
       * we're going to run into problems.
       *
       * See https://gitlab.freedesktop.org/mesa/mesa/-/issues/4904
       */
      isl_surf_get_image_surf(&screen->isl_dev, &res->surf,
                              read_view->base_level,
                              0, read_view->base_array_layer,
                              &read_surf, &read_surf_offset_B,
                              &read_surf_tile_x_sa, &read_surf_tile_y_sa);
      read_view->base_level = 0;
      read_view->base_array_layer = 0;
      assert(read_view->array_len == 1);
   } else if (tex->target == PIPE_TEXTURE_1D_ARRAY) {
      /* Convert 1D array textures to 2D arrays because shaders always provide
       * the array index coordinate at the Z component to avoid recompiles
       * when changing the texture target of the framebuffer.
       */
      assert(read_surf.dim_layout == ISL_DIM_LAYOUT_GFX4_2D);
      read_surf.dim = ISL_SURF_DIM_2D;
   }
#endif

   struct isl_surf isl_surf = res->surf;
   uint64_t offset_B = 0;
   uint32_t tile_x_el = 0, tile_y_el = 0;
   if (isl_format_is_compressed(res->surf.format)) {
      /* The resource has a compressed format, which is not renderable, but we
       * have a renderable view format.  We must be attempting to upload
       * blocks of compressed data via an uncompressed view.
       *
       * In this case, we can assume there are no auxiliary buffers, a single
       * miplevel, and that the resource is single-sampled.  Gallium may try
       * and create an uncompressed view with multiple layers, however.
       */
      assert(res->aux.usage == ISL_AUX_USAGE_NONE);
      assert(res->surf.samples == 1);
      assert(view->levels == 1);

      bool ok = isl_surf_get_uncompressed_surf(&screen->isl_dev,
                                               &res->surf, view,
                                               &isl_surf, view, &offset_B,
                                               &tile_x_el, &tile_y_el);

      /* On Broadwell, HALIGN and VALIGN are specified in pixels and are
       * hard-coded to align to exactly the block size of the compressed
       * texture. This means that, when reinterpreted as a non-compressed
       * texture, the tile offsets may be anything.
       *
       * We need them to be multiples of 4 to be usable in RENDER_SURFACE_STATE,
       * so force the state tracker to take fallback paths if they're not.
       */
#if GFX_VER == 8
      if (tile_x_el % 4 != 0 || tile_y_el % 4 != 0) {
         ok = false;
      }
#endif

      if (!ok) {
         free(surf);
         return NULL;
      }
   }

   surf->clear_color = res->aux.clear_color;

   struct pipe_surface *psurf = &surf->base;
   pipe_reference_init(&psurf->reference, 1);
   pipe_resource_reference(&psurf->texture, tex);
   psurf->context = ctx;
   psurf->format = tmpl->format;
   psurf->width = isl_surf.logical_level0_px.width;
   psurf->height = isl_surf.logical_level0_px.height;
   psurf->texture = tex;
   psurf->u.tex.first_layer = tmpl->u.tex.first_layer;
   psurf->u.tex.last_layer = tmpl->u.tex.last_layer;
   psurf->u.tex.level = tmpl->u.tex.level;

   /* Bail early for depth/stencil - we don't want SURFACE_STATE for them. */
   if (res->surf.usage & (ISL_SURF_USAGE_DEPTH_BIT |
                          ISL_SURF_USAGE_STENCIL_BIT))
      return psurf;

   /* Fill out a SURFACE_STATE for each possible auxiliary surface mode and
    * return the pipe_surface.
    */
   unsigned aux_usages = 0;

   if ((res->aux.usage == ISL_AUX_USAGE_CCS_E ||
        res->aux.usage == ISL_AUX_USAGE_FCV_CCS_E) &&
       !isl_format_supports_ccs_e(devinfo, view->format)) {
      aux_usages = 1 << ISL_AUX_USAGE_NONE;
   } else {
      aux_usages = 1 << ISL_AUX_USAGE_NONE |
                   1 << res->aux.usage;
   }

   alloc_surface_states(&surf->surface_state, aux_usages);
   surf->surface_state.bo_address = res->bo->address;
   fill_surface_states(&screen->isl_dev, &surf->surface_state, res,
                       &isl_surf, view, offset_B, tile_x_el, tile_y_el);

#if GFX_VER == 8
   alloc_surface_states(&surf->surface_state_read, aux_usages);
   surf->surface_state_read.bo_address = res->bo->address;
   fill_surface_states(&screen->isl_dev, &surf->surface_state_read, res,
                       &read_surf, read_view, read_surf_offset_B,
                       read_surf_tile_x_sa, read_surf_tile_y_sa);
#endif

   return psurf;
}

#if GFX_VER < 9
static void
fill_default_image_param(struct brw_image_param *param)
{
   memset(param, 0, sizeof(*param));
   /* Set the swizzling shifts to all-ones to effectively disable swizzling --
    * See emit_address_calculation() in brw_fs_surface_builder.cpp for a more
    * detailed explanation of these parameters.
    */
   param->swizzling[0] = 0xff;
   param->swizzling[1] = 0xff;
}

static void
fill_buffer_image_param(struct brw_image_param *param,
                        enum pipe_format pfmt,
                        unsigned size)
{
   const unsigned cpp = util_format_get_blocksize(pfmt);

   fill_default_image_param(param);
   param->size[0] = size / cpp;
   param->stride[0] = cpp;
}
#else
#define isl_surf_fill_image_param(x, ...)
#define fill_default_image_param(x, ...)
#define fill_buffer_image_param(x, ...)
#endif

/**
 * The pipe->set_shader_images() driver hook.
 */
static void
iris_set_shader_images(struct pipe_context *ctx,
                       enum pipe_shader_type p_stage,
                       unsigned start_slot, unsigned count,
                       unsigned unbind_num_trailing_slots,
                       const struct pipe_image_view *p_images)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct iris_shader_state *shs = &ice->state.shaders[stage];
#if GFX_VER == 8
   struct iris_genx_state *genx = ice->state.genx;
   struct brw_image_param *image_params = genx->shaders[stage].image_param;
#endif

   shs->bound_image_views &=
      ~u_bit_consecutive64(start_slot, count + unbind_num_trailing_slots);

   for (unsigned i = 0; i < count; i++) {
      struct iris_image_view *iv = &shs->image[start_slot + i];

      if (p_images && p_images[i].resource) {
         const struct pipe_image_view *img = &p_images[i];
         struct iris_resource *res = (void *) img->resource;

         util_copy_image_view(&iv->base, img);

         shs->bound_image_views |= BITFIELD64_BIT(start_slot + i);

         res->bind_history |= PIPE_BIND_SHADER_IMAGE;
         res->bind_stages |= 1 << stage;

         enum isl_format isl_fmt = iris_image_view_get_format(ice, img);

         unsigned aux_usages = 1 << ISL_AUX_USAGE_NONE;

         /* Gfx12+ supports render compression for images */
         if (GFX_VER >= 12 && isl_aux_usage_has_ccs_e(res->aux.usage))
            aux_usages |= 1 << ISL_AUX_USAGE_CCS_E;

         alloc_surface_states(&iv->surface_state, aux_usages);
         iv->surface_state.bo_address = res->bo->address;

         if (res->base.b.target != PIPE_BUFFER) {
            struct isl_view view = {
               .format = isl_fmt,
               .base_level = img->u.tex.level,
               .levels = 1,
               .base_array_layer = img->u.tex.first_layer,
               .array_len = img->u.tex.last_layer - img->u.tex.first_layer + 1,
               .swizzle = ISL_SWIZZLE_IDENTITY,
               .usage = ISL_SURF_USAGE_STORAGE_BIT,
            };

            /* If using untyped fallback. */
            if (isl_fmt == ISL_FORMAT_RAW) {
               fill_buffer_surface_state(&screen->isl_dev, res,
                                         iv->surface_state.cpu,
                                         isl_fmt, ISL_SWIZZLE_IDENTITY,
                                         0, res->bo->size,
                                         ISL_SURF_USAGE_STORAGE_BIT);
            } else {
               fill_surface_states(&screen->isl_dev, &iv->surface_state, res,
                                   &res->surf, &view, 0, 0, 0);
            }

            isl_surf_fill_image_param(&screen->isl_dev,
                                      &image_params[start_slot + i],
                                      &res->surf, &view);
         } else if (img->access & PIPE_IMAGE_ACCESS_TEX2D_FROM_BUFFER) {
            /* In case it's a 2d image created from a buffer, we should
             * use fill_surface_states function with image parameters provided
             * by the CL application
             */
            isl_surf_usage_flags_t usage =  ISL_SURF_USAGE_STORAGE_BIT;
            struct isl_view view = {
               .format = isl_fmt,
               .base_level = 0,
               .levels = 1,
               .base_array_layer = 0,
               .array_len = 1,
               .swizzle = ISL_SWIZZLE_IDENTITY,
               .usage = usage,
            };

            /* Create temp_surf and fill with values provided by CL application */
            struct isl_surf temp_surf;
            enum isl_format fmt = iris_image_view_get_format(ice, img);
            fill_surf_for_tex2d_from_buffer(&screen->isl_dev, fmt,
                                            img->u.tex2d_from_buf.width,
                                            img->u.tex2d_from_buf.height,
                                            img->u.tex2d_from_buf.row_stride,
                                            usage,
                                            &temp_surf);

            fill_surface_states(&screen->isl_dev, &iv->surface_state, res,
                                &temp_surf, &view, 0, 0, 0);
            isl_surf_fill_image_param(&screen->isl_dev,
                                      &image_params[start_slot + i],
                                      &temp_surf, &view);
         } else {
            util_range_add(&res->base.b, &res->valid_buffer_range, img->u.buf.offset,
                           img->u.buf.offset + img->u.buf.size);

            fill_buffer_surface_state(&screen->isl_dev, res,
                                      iv->surface_state.cpu,
                                      isl_fmt, ISL_SWIZZLE_IDENTITY,
                                      img->u.buf.offset, img->u.buf.size,
                                      ISL_SURF_USAGE_STORAGE_BIT);
            fill_buffer_image_param(&image_params[start_slot + i],
                                    img->format, img->u.buf.size);
         }

         upload_surface_states(ice->state.surface_uploader, &iv->surface_state);
      } else {
         pipe_resource_reference(&iv->base.resource, NULL);
         pipe_resource_reference(&iv->surface_state.ref.res, NULL);
         fill_default_image_param(&image_params[start_slot + i]);
      }
   }

   ice->state.stage_dirty |= IRIS_STAGE_DIRTY_BINDINGS_VS << stage;
   ice->state.dirty |=
      stage == MESA_SHADER_COMPUTE ? IRIS_DIRTY_COMPUTE_RESOLVES_AND_FLUSHES
                                   : IRIS_DIRTY_RENDER_RESOLVES_AND_FLUSHES;

   /* Broadwell also needs brw_image_params re-uploaded */
   if (GFX_VER < 9) {
      ice->state.stage_dirty |= IRIS_STAGE_DIRTY_CONSTANTS_VS << stage;
      shs->sysvals_need_upload = true;
   }

   if (unbind_num_trailing_slots) {
      iris_set_shader_images(ctx, p_stage, start_slot + count,
                             unbind_num_trailing_slots, 0, NULL);
   }
}

UNUSED static bool
is_sampler_view_3d(const struct iris_sampler_view *view)
{
   return view && view->res->base.b.target == PIPE_TEXTURE_3D;
}

/**
 * The pipe->set_sampler_views() driver hook.
 */
static void
iris_set_sampler_views(struct pipe_context *ctx,
                       enum pipe_shader_type p_stage,
                       unsigned start, unsigned count,
                       unsigned unbind_num_trailing_slots,
                       bool take_ownership,
                       struct pipe_sampler_view **views)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   UNUSED struct iris_screen *screen = (void *) ctx->screen;
   UNUSED const struct intel_device_info *devinfo = screen->devinfo;
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct iris_shader_state *shs = &ice->state.shaders[stage];
   unsigned i;

   if (count == 0 && unbind_num_trailing_slots == 0)
      return;

   BITSET_CLEAR_RANGE(shs->bound_sampler_views, start,
                      start + count + unbind_num_trailing_slots - 1);

   for (i = 0; i < count; i++) {
      struct pipe_sampler_view *pview = views ? views[i] : NULL;
      struct iris_sampler_view *view = (void *) pview;

#if GFX_VERx10 == 125
      if (intel_needs_workaround(screen->devinfo, 14014414195)) {
         if (is_sampler_view_3d(shs->textures[start + i]) !=
             is_sampler_view_3d(view))
            ice->state.stage_dirty |= IRIS_STAGE_DIRTY_SAMPLER_STATES_VS << stage;
      }
#endif

      if (take_ownership) {
         pipe_sampler_view_reference((struct pipe_sampler_view **)
                                     &shs->textures[start + i], NULL);
         shs->textures[start + i] = (struct iris_sampler_view *)pview;
      } else {
         pipe_sampler_view_reference((struct pipe_sampler_view **)
                                     &shs->textures[start + i], pview);
      }
      if (view) {
         view->res->bind_history |= PIPE_BIND_SAMPLER_VIEW;
         view->res->bind_stages |= 1 << stage;

         BITSET_SET(shs->bound_sampler_views, start + i);

         update_surface_state_addrs(ice->state.surface_uploader,
                                    &view->surface_state, view->res->bo);
      }
   }
   for (; i < count + unbind_num_trailing_slots; i++) {
      pipe_sampler_view_reference((struct pipe_sampler_view **)
                                  &shs->textures[start + i], NULL);
   }

   ice->state.stage_dirty |= (IRIS_STAGE_DIRTY_BINDINGS_VS << stage);
   ice->state.dirty |=
      stage == MESA_SHADER_COMPUTE ? IRIS_DIRTY_COMPUTE_RESOLVES_AND_FLUSHES
                                   : IRIS_DIRTY_RENDER_RESOLVES_AND_FLUSHES;
}

static void
iris_set_compute_resources(struct pipe_context *ctx,
                           unsigned start, unsigned count,
                           struct pipe_surface **resources)
{
   assert(count == 0);
}

static void
iris_set_global_binding(struct pipe_context *ctx,
                        unsigned start_slot, unsigned count,
                        struct pipe_resource **resources,
                        uint32_t **handles)
{
   struct iris_context *ice = (struct iris_context *) ctx;

   assert(start_slot + count <= IRIS_MAX_GLOBAL_BINDINGS);
   for (unsigned i = 0; i < count; i++) {
      if (resources && resources[i]) {
         pipe_resource_reference(&ice->state.global_bindings[start_slot + i],
                                 resources[i]);

         struct iris_resource *res = (void *) resources[i];
         assert(res->base.b.target == PIPE_BUFFER);
         util_range_add(&res->base.b, &res->valid_buffer_range,
                        0, res->base.b.width0);

         uint64_t addr = 0;
         memcpy(&addr, handles[i], sizeof(addr));
         addr += res->bo->address + res->offset;
         memcpy(handles[i], &addr, sizeof(addr));
      } else {
         pipe_resource_reference(&ice->state.global_bindings[start_slot + i],
                                 NULL);
      }
   }

   ice->state.stage_dirty |= IRIS_STAGE_DIRTY_BINDINGS_CS;
}

/**
 * The pipe->set_tess_state() driver hook.
 */
static void
iris_set_tess_state(struct pipe_context *ctx,
                    const float default_outer_level[4],
                    const float default_inner_level[2])
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_shader_state *shs = &ice->state.shaders[MESA_SHADER_TESS_CTRL];

   memcpy(&ice->state.default_outer_level[0], &default_outer_level[0], 4 * sizeof(float));
   memcpy(&ice->state.default_inner_level[0], &default_inner_level[0], 2 * sizeof(float));

   ice->state.stage_dirty |= IRIS_STAGE_DIRTY_CONSTANTS_TCS;
   shs->sysvals_need_upload = true;
}

static void
iris_set_patch_vertices(struct pipe_context *ctx, uint8_t patch_vertices)
{
   struct iris_context *ice = (struct iris_context *) ctx;

   ice->state.patch_vertices = patch_vertices;
}

static void
iris_surface_destroy(struct pipe_context *ctx, struct pipe_surface *p_surf)
{
   struct iris_surface *surf = (void *) p_surf;
   pipe_resource_reference(&p_surf->texture, NULL);
   pipe_resource_reference(&surf->surface_state.ref.res, NULL);
   pipe_resource_reference(&surf->surface_state_read.ref.res, NULL);
   free(surf->surface_state.cpu);
   free(surf->surface_state_read.cpu);
   free(surf);
}

static void
iris_set_clip_state(struct pipe_context *ctx,
                    const struct pipe_clip_state *state)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_shader_state *shs = &ice->state.shaders[MESA_SHADER_VERTEX];
   struct iris_shader_state *gshs = &ice->state.shaders[MESA_SHADER_GEOMETRY];
   struct iris_shader_state *tshs = &ice->state.shaders[MESA_SHADER_TESS_EVAL];

   memcpy(&ice->state.clip_planes, state, sizeof(*state));

   ice->state.stage_dirty |= IRIS_STAGE_DIRTY_CONSTANTS_VS |
                             IRIS_STAGE_DIRTY_CONSTANTS_GS |
                             IRIS_STAGE_DIRTY_CONSTANTS_TES;
   shs->sysvals_need_upload = true;
   gshs->sysvals_need_upload = true;
   tshs->sysvals_need_upload = true;
}

/**
 * The pipe->set_polygon_stipple() driver hook.
 */
static void
iris_set_polygon_stipple(struct pipe_context *ctx,
                         const struct pipe_poly_stipple *state)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   memcpy(&ice->state.poly_stipple, state, sizeof(*state));
   ice->state.dirty |= IRIS_DIRTY_POLYGON_STIPPLE;
}

/**
 * The pipe->set_sample_mask() driver hook.
 */
static void
iris_set_sample_mask(struct pipe_context *ctx, unsigned sample_mask)
{
   struct iris_context *ice = (struct iris_context *) ctx;

   /* We only support 16x MSAA, so we have 16 bits of sample maks.
    * st/mesa may pass us 0xffffffff though, meaning "enable all samples".
    */
   ice->state.sample_mask = sample_mask & 0xffff;
   ice->state.dirty |= IRIS_DIRTY_SAMPLE_MASK;
}

/**
 * The pipe->set_scissor_states() driver hook.
 *
 * This corresponds to our SCISSOR_RECT state structures.  It's an
 * exact match, so we just store them, and memcpy them out later.
 */
static void
iris_set_scissor_states(struct pipe_context *ctx,
                        unsigned start_slot,
                        unsigned num_scissors,
                        const struct pipe_scissor_state *rects)
{
   struct iris_context *ice = (struct iris_context *) ctx;

   for (unsigned i = 0; i < num_scissors; i++) {
      if (rects[i].minx == rects[i].maxx || rects[i].miny == rects[i].maxy) {
         /* If the scissor was out of bounds and got clamped to 0 width/height
          * at the bounds, the subtraction of 1 from maximums could produce a
          * negative number and thus not clip anything.  Instead, just provide
          * a min > max scissor inside the bounds, which produces the expected
          * no rendering.
          */
         ice->state.scissors[start_slot + i] = (struct pipe_scissor_state) {
            .minx = 1, .maxx = 0, .miny = 1, .maxy = 0,
         };
      } else {
         ice->state.scissors[start_slot + i] = (struct pipe_scissor_state) {
            .minx = rects[i].minx,     .miny = rects[i].miny,
            .maxx = rects[i].maxx - 1, .maxy = rects[i].maxy - 1,
         };
      }
   }

   ice->state.dirty |= IRIS_DIRTY_SCISSOR_RECT;
}

/**
 * The pipe->set_stencil_ref() driver hook.
 *
 * This is added to 3DSTATE_WM_DEPTH_STENCIL dynamically at draw time.
 */
static void
iris_set_stencil_ref(struct pipe_context *ctx,
                     const struct pipe_stencil_ref state)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   memcpy(&ice->state.stencil_ref, &state, sizeof(state));
   if (GFX_VER >= 12)
      ice->state.dirty |= IRIS_DIRTY_STENCIL_REF;
   else if (GFX_VER >= 9)
      ice->state.dirty |= IRIS_DIRTY_WM_DEPTH_STENCIL;
   else
      ice->state.dirty |= IRIS_DIRTY_COLOR_CALC_STATE;
}

static float
viewport_extent(const struct pipe_viewport_state *state, int axis, float sign)
{
   return copysignf(state->scale[axis], sign) + state->translate[axis];
}

/**
 * The pipe->set_viewport_states() driver hook.
 *
 * This corresponds to our SF_CLIP_VIEWPORT states.  We can't calculate
 * the guardband yet, as we need the framebuffer dimensions, but we can
 * at least fill out the rest.
 */
static void
iris_set_viewport_states(struct pipe_context *ctx,
                         unsigned start_slot,
                         unsigned count,
                         const struct pipe_viewport_state *states)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;

   memcpy(&ice->state.viewports[start_slot], states, sizeof(*states) * count);

   /* Fix depth test misrenderings by lowering translated depth range */
   if (screen->driconf.lower_depth_range_rate != 1.0f)
      ice->state.viewports[start_slot].translate[2] *=
         screen->driconf.lower_depth_range_rate;

   ice->state.dirty |= IRIS_DIRTY_SF_CL_VIEWPORT;

   if (ice->state.cso_rast && (!ice->state.cso_rast->depth_clip_near ||
                               !ice->state.cso_rast->depth_clip_far))
      ice->state.dirty |= IRIS_DIRTY_CC_VIEWPORT;
}

/**
 * The pipe->set_framebuffer_state() driver hook.
 *
 * Sets the current draw FBO, including color render targets, depth,
 * and stencil buffers.
 */
static void
iris_set_framebuffer_state(struct pipe_context *ctx,
                           const struct pipe_framebuffer_state *state)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;
   const struct intel_device_info *devinfo = screen->devinfo;
   struct isl_device *isl_dev = &screen->isl_dev;
   struct pipe_framebuffer_state *cso = &ice->state.framebuffer;
   struct iris_resource *zres;
   struct iris_resource *stencil_res;

   unsigned samples = util_framebuffer_get_num_samples(state);
   unsigned layers = util_framebuffer_get_num_layers(state);

   if (cso->samples != samples) {
      ice->state.dirty |= IRIS_DIRTY_MULTISAMPLE;

      /* We need to toggle 3DSTATE_PS::32 Pixel Dispatch Enable */
      if (GFX_VER >= 9 && (cso->samples == 16 || samples == 16))
         ice->state.stage_dirty |= IRIS_STAGE_DIRTY_FS;

      /* We may need to emit blend state for Wa_14018912822. */
      if ((cso->samples > 1) != (samples > 1) &&
          intel_needs_workaround(devinfo, 14018912822)) {
         ice->state.dirty |= IRIS_DIRTY_BLEND_STATE;
         ice->state.dirty |= IRIS_DIRTY_PS_BLEND;
      }
   }

   if (cso->nr_cbufs != state->nr_cbufs) {
      ice->state.dirty |= IRIS_DIRTY_BLEND_STATE;
   }

   if ((cso->layers == 0) != (layers == 0)) {
      ice->state.dirty |= IRIS_DIRTY_CLIP;
   }

   if (cso->width != state->width || cso->height != state->height) {
      ice->state.dirty |= IRIS_DIRTY_SF_CL_VIEWPORT;
   }

   if (cso->zsbuf || state->zsbuf) {
      ice->state.dirty |= IRIS_DIRTY_DEPTH_BUFFER;
   }

   bool has_integer_rt = false;
   for (unsigned i = 0; i < state->nr_cbufs; i++) {
      if (state->cbufs[i]) {
         enum isl_format ifmt =
            isl_format_for_pipe_format(state->cbufs[i]->format);
         has_integer_rt |= isl_format_has_int_channel(ifmt);
      }
   }

   /* 3DSTATE_RASTER::AntialiasingEnable */
   if (has_integer_rt != ice->state.has_integer_rt ||
       cso->samples != samples) {
      ice->state.dirty |= IRIS_DIRTY_RASTER;
   }

   util_copy_framebuffer_state(cso, state);
   cso->samples = samples;
   cso->layers = layers;

   ice->state.has_integer_rt = has_integer_rt;

   struct iris_depth_buffer_state *cso_z = &ice->state.genx->depth_buffer;

   struct isl_view view = {
      .base_level = 0,
      .levels = 1,
      .base_array_layer = 0,
      .array_len = 1,
      .swizzle = ISL_SWIZZLE_IDENTITY,
   };

   struct isl_depth_stencil_hiz_emit_info info = {
      .view = &view,
      .mocs = iris_mocs(NULL, isl_dev, ISL_SURF_USAGE_DEPTH_BIT),
   };

   if (cso->zsbuf) {
      iris_get_depth_stencil_resources(cso->zsbuf->texture, &zres,
                                       &stencil_res);

      view.base_level = cso->zsbuf->u.tex.level;
      view.base_array_layer = cso->zsbuf->u.tex.first_layer;
      view.array_len =
         cso->zsbuf->u.tex.last_layer - cso->zsbuf->u.tex.first_layer + 1;

      if (zres) {
         view.usage |= ISL_SURF_USAGE_DEPTH_BIT;

         info.depth_surf = &zres->surf;
         info.depth_address = zres->bo->address + zres->offset;
         info.mocs = iris_mocs(zres->bo, isl_dev, view.usage);

         view.format = zres->surf.format;

         if (iris_resource_level_has_hiz(devinfo, zres, view.base_level)) {
            info.hiz_usage = zres->aux.usage;
            info.hiz_surf = &zres->aux.surf;
            info.hiz_address = zres->aux.bo->address + zres->aux.offset;
         }

         ice->state.hiz_usage = info.hiz_usage;
      }

      if (stencil_res) {
         view.usage |= ISL_SURF_USAGE_STENCIL_BIT;
         info.stencil_aux_usage = stencil_res->aux.usage;
         info.stencil_surf = &stencil_res->surf;
         info.stencil_address = stencil_res->bo->address + stencil_res->offset;
         if (!zres) {
            view.format = stencil_res->surf.format;
            info.mocs = iris_mocs(stencil_res->bo, isl_dev, view.usage);
         }
      }
   }

   isl_emit_depth_stencil_hiz_s(isl_dev, cso_z->packets, &info);

   /* Make a null surface for unbound buffers */
   void *null_surf_map =
      upload_state(ice->state.surface_uploader, &ice->state.null_fb,
                   4 * GENX(RENDER_SURFACE_STATE_length), 64);
   isl_null_fill_state(&screen->isl_dev, null_surf_map,
                       .size = isl_extent3d(MAX2(cso->width, 1),
                                            MAX2(cso->height, 1),
                                            cso->layers ? cso->layers : 1));
   ice->state.null_fb.offset +=
      iris_bo_offset_from_base_address(iris_resource_bo(ice->state.null_fb.res));

   /* Render target change */
   ice->state.stage_dirty |= IRIS_STAGE_DIRTY_BINDINGS_FS;

   ice->state.dirty |= IRIS_DIRTY_RENDER_BUFFER;

   ice->state.dirty |= IRIS_DIRTY_RENDER_RESOLVES_AND_FLUSHES;

   ice->state.stage_dirty |=
      ice->state.stage_dirty_for_nos[IRIS_NOS_FRAMEBUFFER];

   if (GFX_VER == 8)
      ice->state.dirty |= IRIS_DIRTY_PMA_FIX;
}

/**
 * The pipe->set_constant_buffer() driver hook.
 *
 * This uploads any constant data in user buffers, and references
 * any UBO resources containing constant data.
 */
static void
iris_set_constant_buffer(struct pipe_context *ctx,
                         enum pipe_shader_type p_stage, unsigned index,
                         bool take_ownership,
                         const struct pipe_constant_buffer *input)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct iris_shader_state *shs = &ice->state.shaders[stage];
   struct pipe_shader_buffer *cbuf = &shs->constbuf[index];

   /* TODO: Only do this if the buffer changes? */
   pipe_resource_reference(&shs->constbuf_surf_state[index].res, NULL);

   if (input && input->buffer_size && (input->buffer || input->user_buffer)) {
      shs->bound_cbufs |= 1u << index;

      if (input->user_buffer) {
         void *map = NULL;
         pipe_resource_reference(&cbuf->buffer, NULL);
         u_upload_alloc(ice->ctx.const_uploader, 0, input->buffer_size, 64,
                        &cbuf->buffer_offset, &cbuf->buffer, (void **) &map);

         if (!cbuf->buffer) {
            /* Allocation was unsuccessful - just unbind */
            iris_set_constant_buffer(ctx, p_stage, index, false, NULL);
            return;
         }

         assert(map);
         memcpy(map, input->user_buffer, input->buffer_size);
      } else if (input->buffer) {
         if (cbuf->buffer != input->buffer) {
            ice->state.dirty |= (IRIS_DIRTY_RENDER_MISC_BUFFER_FLUSHES |
                                 IRIS_DIRTY_COMPUTE_MISC_BUFFER_FLUSHES);
            shs->dirty_cbufs |= 1u << index;
         }

         if (take_ownership) {
            pipe_resource_reference(&cbuf->buffer, NULL);
            cbuf->buffer = input->buffer;
         } else {
            pipe_resource_reference(&cbuf->buffer, input->buffer);
         }

         cbuf->buffer_offset = input->buffer_offset;
      }

      cbuf->buffer_size =
         MIN2(input->buffer_size,
              iris_resource_bo(cbuf->buffer)->size - cbuf->buffer_offset);

      struct iris_resource *res = (void *) cbuf->buffer;
      res->bind_history |= PIPE_BIND_CONSTANT_BUFFER;
      res->bind_stages |= 1 << stage;
   } else {
      shs->bound_cbufs &= ~(1u << index);
      pipe_resource_reference(&cbuf->buffer, NULL);
   }

   ice->state.stage_dirty |= IRIS_STAGE_DIRTY_CONSTANTS_VS << stage;
}

static void
upload_sysvals(struct iris_context *ice,
               gl_shader_stage stage,
               const struct pipe_grid_info *grid)
{
   UNUSED struct iris_genx_state *genx = ice->state.genx;
   struct iris_shader_state *shs = &ice->state.shaders[stage];

   struct iris_compiled_shader *shader = ice->shaders.prog[stage];
   if (!shader || (shader->num_system_values == 0 &&
                   shader->kernel_input_size == 0))
      return;

   assert(shader->num_cbufs > 0);

   unsigned sysval_cbuf_index = shader->num_cbufs - 1;
   struct pipe_shader_buffer *cbuf = &shs->constbuf[sysval_cbuf_index];
   unsigned system_values_start =
      ALIGN(shader->kernel_input_size, sizeof(uint32_t));
   unsigned upload_size = system_values_start +
                          shader->num_system_values * sizeof(uint32_t);
   void *map = NULL;

   assert(sysval_cbuf_index < PIPE_MAX_CONSTANT_BUFFERS);
   u_upload_alloc(ice->ctx.const_uploader, 0, upload_size, 64,
                  &cbuf->buffer_offset, &cbuf->buffer, &map);

   if (shader->kernel_input_size > 0)
      memcpy(map, grid->input, shader->kernel_input_size);

   uint32_t *sysval_map = map + system_values_start;
   for (int i = 0; i < shader->num_system_values; i++) {
      uint32_t sysval = shader->system_values[i];
      uint32_t value = 0;

      if (BRW_PARAM_DOMAIN(sysval) == BRW_PARAM_DOMAIN_IMAGE) {
#if GFX_VER == 8
         unsigned img = BRW_PARAM_IMAGE_IDX(sysval);
         unsigned offset = BRW_PARAM_IMAGE_OFFSET(sysval);
         struct brw_image_param *param =
            &genx->shaders[stage].image_param[img];

         assert(offset < sizeof(struct brw_image_param));
         value = ((uint32_t *) param)[offset];
#endif
      } else if (sysval == BRW_PARAM_BUILTIN_ZERO) {
         value = 0;
      } else if (BRW_PARAM_BUILTIN_IS_CLIP_PLANE(sysval)) {
         int plane = BRW_PARAM_BUILTIN_CLIP_PLANE_IDX(sysval);
         int comp  = BRW_PARAM_BUILTIN_CLIP_PLANE_COMP(sysval);
         value = fui(ice->state.clip_planes.ucp[plane][comp]);
      } else if (sysval == BRW_PARAM_BUILTIN_PATCH_VERTICES_IN) {
         if (stage == MESA_SHADER_TESS_CTRL) {
            value = ice->state.vertices_per_patch;
         } else {
            assert(stage == MESA_SHADER_TESS_EVAL);
            const struct shader_info *tcs_info =
               iris_get_shader_info(ice, MESA_SHADER_TESS_CTRL);
            if (tcs_info)
               value = tcs_info->tess.tcs_vertices_out;
            else
               value = ice->state.vertices_per_patch;
         }
      } else if (sysval >= BRW_PARAM_BUILTIN_TESS_LEVEL_OUTER_X &&
                 sysval <= BRW_PARAM_BUILTIN_TESS_LEVEL_OUTER_W) {
         unsigned i = sysval - BRW_PARAM_BUILTIN_TESS_LEVEL_OUTER_X;
         value = fui(ice->state.default_outer_level[i]);
      } else if (sysval == BRW_PARAM_BUILTIN_TESS_LEVEL_INNER_X) {
         value = fui(ice->state.default_inner_level[0]);
      } else if (sysval == BRW_PARAM_BUILTIN_TESS_LEVEL_INNER_Y) {
         value = fui(ice->state.default_inner_level[1]);
      } else if (sysval >= BRW_PARAM_BUILTIN_WORK_GROUP_SIZE_X &&
                 sysval <= BRW_PARAM_BUILTIN_WORK_GROUP_SIZE_Z) {
         unsigned i = sysval - BRW_PARAM_BUILTIN_WORK_GROUP_SIZE_X;
         value = ice->state.last_block[i];
      } else if (sysval == BRW_PARAM_BUILTIN_WORK_DIM) {
         value = grid->work_dim;
      } else {
         assert(!"unhandled system value");
      }

      *sysval_map++ = value;
   }

   cbuf->buffer_size = upload_size;
   iris_upload_ubo_ssbo_surf_state(ice, cbuf,
                                   &shs->constbuf_surf_state[sysval_cbuf_index],
                                   ISL_SURF_USAGE_CONSTANT_BUFFER_BIT);

   shs->sysvals_need_upload = false;
}

/**
 * The pipe->set_shader_buffers() driver hook.
 *
 * This binds SSBOs and ABOs.  Unfortunately, we need to stream out
 * SURFACE_STATE here, as the buffer offset may change each time.
 */
static void
iris_set_shader_buffers(struct pipe_context *ctx,
                        enum pipe_shader_type p_stage,
                        unsigned start_slot, unsigned count,
                        const struct pipe_shader_buffer *buffers,
                        unsigned writable_bitmask)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct iris_shader_state *shs = &ice->state.shaders[stage];

   unsigned modified_bits = u_bit_consecutive(start_slot, count);

   shs->bound_ssbos &= ~modified_bits;
   shs->writable_ssbos &= ~modified_bits;
   shs->writable_ssbos |= writable_bitmask << start_slot;

   for (unsigned i = 0; i < count; i++) {
      if (buffers && buffers[i].buffer) {
         struct iris_resource *res = (void *) buffers[i].buffer;
         struct pipe_shader_buffer *ssbo = &shs->ssbo[start_slot + i];
         struct iris_state_ref *surf_state =
            &shs->ssbo_surf_state[start_slot + i];
         pipe_resource_reference(&ssbo->buffer, &res->base.b);
         ssbo->buffer_offset = buffers[i].buffer_offset;
         ssbo->buffer_size =
            MIN2(buffers[i].buffer_size, res->bo->size - ssbo->buffer_offset);

         shs->bound_ssbos |= 1 << (start_slot + i);

         isl_surf_usage_flags_t usage = ISL_SURF_USAGE_STORAGE_BIT;

         iris_upload_ubo_ssbo_surf_state(ice, ssbo, surf_state, usage);

         res->bind_history |= PIPE_BIND_SHADER_BUFFER;
         res->bind_stages |= 1 << stage;

         util_range_add(&res->base.b, &res->valid_buffer_range, ssbo->buffer_offset,
                        ssbo->buffer_offset + ssbo->buffer_size);
      } else {
         pipe_resource_reference(&shs->ssbo[start_slot + i].buffer, NULL);
         pipe_resource_reference(&shs->ssbo_surf_state[start_slot + i].res,
                                 NULL);
      }
   }

   ice->state.dirty |= (IRIS_DIRTY_RENDER_MISC_BUFFER_FLUSHES |
                        IRIS_DIRTY_COMPUTE_MISC_BUFFER_FLUSHES);
   ice->state.stage_dirty |= IRIS_STAGE_DIRTY_BINDINGS_VS << stage;
}

static void
iris_delete_state(struct pipe_context *ctx, void *state)
{
   free(state);
}

/**
 * The pipe->set_vertex_buffers() driver hook.
 *
 * This translates pipe_vertex_buffer to our 3DSTATE_VERTEX_BUFFERS packet.
 */
static void
iris_set_vertex_buffers(struct pipe_context *ctx,
                        unsigned count,
                        unsigned unbind_num_trailing_slots,
                        bool take_ownership,
                        const struct pipe_vertex_buffer *buffers)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;
   struct iris_genx_state *genx = ice->state.genx;

   ice->state.bound_vertex_buffers &=
      ~u_bit_consecutive64(0, count + unbind_num_trailing_slots);

   for (unsigned i = 0; i < count; i++) {
      const struct pipe_vertex_buffer *buffer = buffers ? &buffers[i] : NULL;
      struct iris_vertex_buffer_state *state =
         &genx->vertex_buffers[i];

      if (!buffer) {
         pipe_resource_reference(&state->resource, NULL);
         continue;
      }

      /* We may see user buffers that are NULL bindings. */
      assert(!(buffer->is_user_buffer && buffer->buffer.user != NULL));

      if (buffer->buffer.resource &&
          state->resource != buffer->buffer.resource)
         ice->state.dirty |= IRIS_DIRTY_VERTEX_BUFFER_FLUSHES;

      if (take_ownership) {
         pipe_resource_reference(&state->resource, NULL);
         state->resource = buffer->buffer.resource;
      } else {
         pipe_resource_reference(&state->resource, buffer->buffer.resource);
      }
      struct iris_resource *res = (void *) state->resource;

      state->offset = (int) buffer->buffer_offset;

      if (res) {
         ice->state.bound_vertex_buffers |= 1ull << i;
         res->bind_history |= PIPE_BIND_VERTEX_BUFFER;
      }

      iris_pack_state(GENX(VERTEX_BUFFER_STATE), state->state, vb) {
         vb.VertexBufferIndex = i;
         vb.AddressModifyEnable = true;
         /* vb.BufferPitch is merged in dynamically from VE state later */
         if (res) {
            vb.BufferSize = res->base.b.width0 - (int) buffer->buffer_offset;
            vb.BufferStartingAddress =
               ro_bo(NULL, res->bo->address + (int) buffer->buffer_offset);
            vb.MOCS = iris_mocs(res->bo, &screen->isl_dev,
                                ISL_SURF_USAGE_VERTEX_BUFFER_BIT);
#if GFX_VER >= 12
            vb.L3BypassDisable       = true;
#endif
         } else {
            vb.NullVertexBuffer = true;
            vb.MOCS = iris_mocs(NULL, &screen->isl_dev,
                                ISL_SURF_USAGE_VERTEX_BUFFER_BIT);
         }
      }
   }

   for (unsigned i = 0; i < unbind_num_trailing_slots; i++) {
      struct iris_vertex_buffer_state *state =
         &genx->vertex_buffers[count + i];

      pipe_resource_reference(&state->resource, NULL);
   }

   ice->state.dirty |= IRIS_DIRTY_VERTEX_BUFFERS;
}

/**
 * Gallium CSO for vertex elements.
 */
struct iris_vertex_element_state {
   uint32_t vertex_elements[1 + 33 * GENX(VERTEX_ELEMENT_STATE_length)];
   uint32_t vf_instancing[33 * GENX(3DSTATE_VF_INSTANCING_length)];
   uint32_t edgeflag_ve[GENX(VERTEX_ELEMENT_STATE_length)];
   uint32_t edgeflag_vfi[GENX(3DSTATE_VF_INSTANCING_length)];
   uint32_t stride[PIPE_MAX_ATTRIBS];
   unsigned vb_count;
   unsigned count;
};

/**
 * The pipe->create_vertex_elements_state() driver hook.
 *
 * This translates pipe_vertex_element to our 3DSTATE_VERTEX_ELEMENTS
 * and 3DSTATE_VF_INSTANCING commands. The vertex_elements and vf_instancing
 * arrays are ready to be emitted at draw time if no EdgeFlag or SGVs are
 * needed. In these cases we will need information available at draw time.
 * We setup edgeflag_ve and edgeflag_vfi as alternatives last
 * 3DSTATE_VERTEX_ELEMENT and 3DSTATE_VF_INSTANCING that can be used at
 * draw time if we detect that EdgeFlag is needed by the Vertex Shader.
 */
static void *
iris_create_vertex_elements(struct pipe_context *ctx,
                            unsigned count,
                            const struct pipe_vertex_element *state)
{
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;
   const struct intel_device_info *devinfo = screen->devinfo;
   struct iris_vertex_element_state *cso =
      calloc(1, sizeof(struct iris_vertex_element_state));

   cso->count = count;
   cso->vb_count = 0;

   iris_pack_command(GENX(3DSTATE_VERTEX_ELEMENTS), cso->vertex_elements, ve) {
      ve.DWordLength =
         1 + GENX(VERTEX_ELEMENT_STATE_length) * MAX2(count, 1) - 2;
   }

   uint32_t *ve_pack_dest = &cso->vertex_elements[1];
   uint32_t *vfi_pack_dest = cso->vf_instancing;

   if (count == 0) {
      iris_pack_state(GENX(VERTEX_ELEMENT_STATE), ve_pack_dest, ve) {
         ve.Valid = true;
         ve.SourceElementFormat = ISL_FORMAT_R32G32B32A32_FLOAT;
         ve.Component0Control = VFCOMP_STORE_0;
         ve.Component1Control = VFCOMP_STORE_0;
         ve.Component2Control = VFCOMP_STORE_0;
         ve.Component3Control = VFCOMP_STORE_1_FP;
      }

      iris_pack_command(GENX(3DSTATE_VF_INSTANCING), vfi_pack_dest, vi) {
      }
   }

   for (int i = 0; i < count; i++) {
      const struct iris_format_info fmt =
         iris_format_for_usage(devinfo, state[i].src_format, 0);
      unsigned comp[4] = { VFCOMP_STORE_SRC, VFCOMP_STORE_SRC,
                           VFCOMP_STORE_SRC, VFCOMP_STORE_SRC };

      switch (isl_format_get_num_channels(fmt.fmt)) {
      case 0: comp[0] = VFCOMP_STORE_0; FALLTHROUGH;
      case 1: comp[1] = VFCOMP_STORE_0; FALLTHROUGH;
      case 2: comp[2] = VFCOMP_STORE_0; FALLTHROUGH;
      case 3:
         comp[3] = isl_format_has_int_channel(fmt.fmt) ? VFCOMP_STORE_1_INT
                                                       : VFCOMP_STORE_1_FP;
         break;
      }
      iris_pack_state(GENX(VERTEX_ELEMENT_STATE), ve_pack_dest, ve) {
         ve.EdgeFlagEnable = false;
         ve.VertexBufferIndex = state[i].vertex_buffer_index;
         ve.Valid = true;
         ve.SourceElementOffset = state[i].src_offset;
         ve.SourceElementFormat = fmt.fmt;
         ve.Component0Control = comp[0];
         ve.Component1Control = comp[1];
         ve.Component2Control = comp[2];
         ve.Component3Control = comp[3];
      }

      iris_pack_command(GENX(3DSTATE_VF_INSTANCING), vfi_pack_dest, vi) {
         vi.VertexElementIndex = i;
         vi.InstancingEnable = state[i].instance_divisor > 0;
         vi.InstanceDataStepRate = state[i].instance_divisor;
      }

      ve_pack_dest += GENX(VERTEX_ELEMENT_STATE_length);
      vfi_pack_dest += GENX(3DSTATE_VF_INSTANCING_length);
      cso->stride[state[i].vertex_buffer_index] = state[i].src_stride;
      cso->vb_count = MAX2(state[i].vertex_buffer_index + 1, cso->vb_count);
   }

   /* An alternative version of the last VE and VFI is stored so it
    * can be used at draw time in case Vertex Shader uses EdgeFlag
    */
   if (count) {
      const unsigned edgeflag_index = count - 1;
      const struct iris_format_info fmt =
         iris_format_for_usage(devinfo, state[edgeflag_index].src_format, 0);
      iris_pack_state(GENX(VERTEX_ELEMENT_STATE), cso->edgeflag_ve, ve) {
         ve.EdgeFlagEnable = true ;
         ve.VertexBufferIndex = state[edgeflag_index].vertex_buffer_index;
         ve.Valid = true;
         ve.SourceElementOffset = state[edgeflag_index].src_offset;
         ve.SourceElementFormat = fmt.fmt;
         ve.Component0Control = VFCOMP_STORE_SRC;
         ve.Component1Control = VFCOMP_STORE_0;
         ve.Component2Control = VFCOMP_STORE_0;
         ve.Component3Control = VFCOMP_STORE_0;
      }
      iris_pack_command(GENX(3DSTATE_VF_INSTANCING), cso->edgeflag_vfi, vi) {
         /* The vi.VertexElementIndex of the EdgeFlag Vertex Element is filled
          * at draw time, as it should change if SGVs are emitted.
          */
         vi.InstancingEnable = state[edgeflag_index].instance_divisor > 0;
         vi.InstanceDataStepRate = state[edgeflag_index].instance_divisor;
      }
   }

   return cso;
}

/**
 * The pipe->bind_vertex_elements_state() driver hook.
 */
static void
iris_bind_vertex_elements_state(struct pipe_context *ctx, void *state)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_vertex_element_state *old_cso = ice->state.cso_vertex_elements;
   struct iris_vertex_element_state *new_cso = state;

   /* 3DSTATE_VF_SGVs overrides the last VE, so if the count is changing,
    * we need to re-emit it to ensure we're overriding the right one.
    */
   if (new_cso && cso_changed(count))
      ice->state.dirty |= IRIS_DIRTY_VF_SGVS;

   ice->state.cso_vertex_elements = state;
   ice->state.dirty |= IRIS_DIRTY_VERTEX_ELEMENTS;
   if (new_cso) {
      /* re-emit vertex buffer state if stride changes */
      if (cso_changed(vb_count) ||
          cso_changed_memcmp_elts(stride, new_cso->vb_count))
         ice->state.dirty |= IRIS_DIRTY_VERTEX_BUFFERS;
   }
}

/**
 * The pipe->create_stream_output_target() driver hook.
 *
 * "Target" here refers to a destination buffer.  We translate this into
 * a 3DSTATE_SO_BUFFER packet.  We can handle most fields, but don't yet
 * know which buffer this represents, or whether we ought to zero the
 * write-offsets, or append.  Those are handled in the set() hook.
 */
static struct pipe_stream_output_target *
iris_create_stream_output_target(struct pipe_context *ctx,
                                 struct pipe_resource *p_res,
                                 unsigned buffer_offset,
                                 unsigned buffer_size)
{
   struct iris_resource *res = (void *) p_res;
   struct iris_stream_output_target *cso = calloc(1, sizeof(*cso));
   if (!cso)
      return NULL;

   res->bind_history |= PIPE_BIND_STREAM_OUTPUT;

   pipe_reference_init(&cso->base.reference, 1);
   pipe_resource_reference(&cso->base.buffer, p_res);
   cso->base.buffer_offset = buffer_offset;
   cso->base.buffer_size = buffer_size;
   cso->base.context = ctx;

   util_range_add(&res->base.b, &res->valid_buffer_range, buffer_offset,
                  buffer_offset + buffer_size);

   return &cso->base;
}

static void
iris_stream_output_target_destroy(struct pipe_context *ctx,
                                  struct pipe_stream_output_target *state)
{
   struct iris_stream_output_target *cso = (void *) state;

   pipe_resource_reference(&cso->base.buffer, NULL);
   pipe_resource_reference(&cso->offset.res, NULL);

   free(cso);
}

/**
 * The pipe->set_stream_output_targets() driver hook.
 *
 * At this point, we know which targets are bound to a particular index,
 * and also whether we want to append or start over.  We can finish the
 * 3DSTATE_SO_BUFFER packets we started earlier.
 */
static void
iris_set_stream_output_targets(struct pipe_context *ctx,
                               unsigned num_targets,
                               struct pipe_stream_output_target **targets,
                               const unsigned *offsets)
{
   struct iris_context *ice = (struct iris_context *) ctx;
   struct iris_genx_state *genx = ice->state.genx;
   uint32_t *so_buffers = genx->so_buffers;
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;

   const bool active = num_targets > 0;
   if (ice->state.streamout_active != active) {
      ice->state.streamout_active = active;
      ice->state.dirty |= IRIS_DIRTY_STREAMOUT;

      /* We only emit 3DSTATE_SO_DECL_LIST when streamout is active, because
       * it's a non-pipelined command.  If we're switching streamout on, we
       * may have missed emitting it earlier, so do so now.  (We're already
       * taking a stall to update 3DSTATE_SO_BUFFERS anyway...)
       */
      if (active) {
         ice->state.dirty |= IRIS_DIRTY_SO_DECL_LIST;
      } else {
         for (int i = 0; i < PIPE_MAX_SO_BUFFERS; i++) {
            struct iris_stream_output_target *tgt =
               (void *) ice->state.so_target[i];

            if (tgt)
               iris_dirty_for_history(ice, (void *)tgt->base.buffer);
         }
      }
   }

   for (int i = 0; i < 4; i++) {
      pipe_so_target_reference(&ice->state.so_target[i],
                               i < num_targets ? targets[i] : NULL);
   }

   /* No need to update 3DSTATE_SO_BUFFER unless SOL is active. */
   if (!active)
      return;

   for (unsigned i = 0; i < 4; i++,
        so_buffers += GENX(3DSTATE_SO_BUFFER_length)) {

      struct iris_stream_output_target *tgt = (void *) ice->state.so_target[i];
      unsigned offset = offsets[i];

      if (!tgt) {
         iris_pack_command(GENX(3DSTATE_SO_BUFFER), so_buffers, sob) {
#if GFX_VER < 12
            sob.SOBufferIndex = i;
#else
            sob._3DCommandOpcode = 0;
            sob._3DCommandSubOpcode = SO_BUFFER_INDEX_0_CMD + i;
#endif
            sob.MOCS = iris_mocs(NULL, &screen->isl_dev, 0);
         }
         continue;
      }

      if (!tgt->offset.res)
         upload_state(ctx->const_uploader, &tgt->offset, sizeof(uint32_t), 4);

      struct iris_resource *res = (void *) tgt->base.buffer;

      /* Note that offsets[i] will either be 0, causing us to zero
       * the value in the buffer, or 0xFFFFFFFF, which happens to mean
       * "continue appending at the existing offset."
       */
      assert(offset == 0 || offset == 0xFFFFFFFF);

      /* When we're first called with an offset of 0, we want the next
       * 3DSTATE_SO_BUFFER packets to reset the offset to the beginning.
       * Any further times we emit those packets, we want to use 0xFFFFFFFF
       * to continue appending from the current offset.
       *
       * Note that we might be called by Begin (offset = 0), Pause, then
       * Resume (offset = 0xFFFFFFFF) before ever drawing (where these
       * commands will actually be sent to the GPU).  In this case, we
       * don't want to append - we still want to do our initial zeroing.
       */
      if (offset == 0)
         tgt->zero_offset = true;

      iris_pack_command(GENX(3DSTATE_SO_BUFFER), so_buffers, sob) {
#if GFX_VER < 12
         sob.SOBufferIndex = i;
#else
         sob._3DCommandOpcode = 0;
         sob._3DCommandSubOpcode = SO_BUFFER_INDEX_0_CMD + i;
#endif
         sob.SurfaceBaseAddress =
            rw_bo(NULL, res->bo->address + tgt->base.buffer_offset,
                  IRIS_DOMAIN_OTHER_WRITE);
         sob.SOBufferEnable = true;
         sob.StreamOffsetWriteEnable = true;
         sob.StreamOutputBufferOffsetAddressEnable = true;
         sob.MOCS = iris_mocs(res->bo, &screen->isl_dev,
                              ISL_SURF_USAGE_STREAM_OUT_BIT);

         sob.SurfaceSize = MAX2(tgt->base.buffer_size / 4, 1) - 1;
         sob.StreamOutputBufferOffsetAddress =
            rw_bo(NULL, iris_resource_bo(tgt->offset.res)->address +
                        tgt->offset.offset, IRIS_DOMAIN_OTHER_WRITE);
         sob.StreamOffset = 0xFFFFFFFF; /* not offset, see above */
      }
   }

   ice->state.dirty |= IRIS_DIRTY_SO_BUFFERS;
}

/**
 * An iris-vtable helper for encoding the 3DSTATE_SO_DECL_LIST and
 * 3DSTATE_STREAMOUT packets.
 *
 * 3DSTATE_SO_DECL_LIST is a list of shader outputs we want the streamout
 * hardware to record.  We can create it entirely based on the shader, with
 * no dynamic state dependencies.
 *
 * 3DSTATE_STREAMOUT is an annoying mix of shader-based information and
 * state-based settings.  We capture the shader-related ones here, and merge
 * the rest in at draw time.
 */
static uint32_t *
iris_create_so_decl_list(const struct pipe_stream_output_info *info,
                         const struct brw_vue_map *vue_map)
{
   struct GENX(SO_DECL) so_decl[PIPE_MAX_VERTEX_STREAMS][128];
   int buffer_mask[PIPE_MAX_VERTEX_STREAMS] = {0, 0, 0, 0};
   int next_offset[PIPE_MAX_VERTEX_STREAMS] = {0, 0, 0, 0};
   int decls[PIPE_MAX_VERTEX_STREAMS] = {0, 0, 0, 0};
   int max_decls = 0;
   STATIC_ASSERT(ARRAY_SIZE(so_decl[0]) >= PIPE_MAX_SO_OUTPUTS);

   memset(so_decl, 0, sizeof(so_decl));

   /* Construct the list of SO_DECLs to be emitted.  The formatting of the
    * command feels strange -- each dword pair contains a SO_DECL per stream.
    */
   for (unsigned i = 0; i < info->num_outputs; i++) {
      const struct pipe_stream_output *output = &info->output[i];
      const int buffer = output->output_buffer;
      const int varying = output->register_index;
      const unsigned stream_id = output->stream;
      assert(stream_id < PIPE_MAX_VERTEX_STREAMS);

      buffer_mask[stream_id] |= 1 << buffer;

      assert(vue_map->varying_to_slot[varying] >= 0);

      /* Mesa doesn't store entries for gl_SkipComponents in the Outputs[]
       * array.  Instead, it simply increments DstOffset for the following
       * input by the number of components that should be skipped.
       *
       * Our hardware is unusual in that it requires us to program SO_DECLs
       * for fake "hole" components, rather than simply taking the offset
       * for each real varying.  Each hole can have size 1, 2, 3, or 4; we
       * program as many size = 4 holes as we can, then a final hole to
       * accommodate the final 1, 2, or 3 remaining.
       */
      int skip_components = output->dst_offset - next_offset[buffer];

      while (skip_components > 0) {
         so_decl[stream_id][decls[stream_id]++] = (struct GENX(SO_DECL)) {
            .HoleFlag = 1,
            .OutputBufferSlot = output->output_buffer,
            .ComponentMask = (1 << MIN2(skip_components, 4)) - 1,
         };
         skip_components -= 4;
      }

      next_offset[buffer] = output->dst_offset + output->num_components;

      so_decl[stream_id][decls[stream_id]++] = (struct GENX(SO_DECL)) {
         .OutputBufferSlot = output->output_buffer,
         .RegisterIndex = vue_map->varying_to_slot[varying],
         .ComponentMask =
            ((1 << output->num_components) - 1) << output->start_component,
      };

      if (decls[stream_id] > max_decls)
         max_decls = decls[stream_id];
   }

   unsigned dwords = GENX(3DSTATE_STREAMOUT_length) + (3 + 2 * max_decls);
   uint32_t *map = ralloc_size(NULL, sizeof(uint32_t) * dwords);
   uint32_t *so_decl_map = map + GENX(3DSTATE_STREAMOUT_length);

   iris_pack_command(GENX(3DSTATE_STREAMOUT), map, sol) {
      int urb_entry_read_offset = 0;
      int urb_entry_read_length = (vue_map->num_slots + 1) / 2 -
         urb_entry_read_offset;

      /* We always read the whole vertex.  This could be reduced at some
       * point by reading less and offsetting the register index in the
       * SO_DECLs.
       */
      sol.Stream0VertexReadOffset = urb_entry_read_offset;
      sol.Stream0VertexReadLength = urb_entry_read_length - 1;
      sol.Stream1VertexReadOffset = urb_entry_read_offset;
      sol.Stream1VertexReadLength = urb_entry_read_length - 1;
      sol.Stream2VertexReadOffset = urb_entry_read_offset;
      sol.Stream2VertexReadLength = urb_entry_read_length - 1;
      sol.Stream3VertexReadOffset = urb_entry_read_offset;
      sol.Stream3VertexReadLength = urb_entry_read_length - 1;

      /* Set buffer pitches; 0 means unbound. */
      sol.Buffer0SurfacePitch = 4 * info->stride[0];
      sol.Buffer1SurfacePitch = 4 * info->stride[1];
      sol.Buffer2SurfacePitch = 4 * info->stride[2];
      sol.Buffer3SurfacePitch = 4 * info->stride[3];
   }

   iris_pack_command(GENX(3DSTATE_SO_DECL_LIST), so_decl_map, list) {
      list.DWordLength = 3 + 2 * max_decls - 2;
      list.StreamtoBufferSelects0 = buffer_mask[0];
      list.StreamtoBufferSelects1 = buffer_mask[1];
      list.StreamtoBufferSelects2 = buffer_mask[2];
      list.StreamtoBufferSelects3 = buffer_mask[3];
      list.NumEntries0 = decls[0];
      list.NumEntries1 = decls[1];
      list.NumEntries2 = decls[2];
      list.NumEntries3 = decls[3];
   }

   for (int i = 0; i < max_decls; i++) {
      iris_pack_state(GENX(SO_DECL_ENTRY), so_decl_map + 3 + i * 2, entry) {
         entry.Stream0Decl = so_decl[0][i];
         entry.Stream1Decl = so_decl[1][i];
         entry.Stream2Decl = so_decl[2][i];
         entry.Stream3Decl = so_decl[3][i];
      }
   }

   return map;
}

static void
iris_compute_sbe_urb_read_interval(uint64_t fs_input_slots,
                                   const struct brw_vue_map *last_vue_map,
                                   bool two_sided_color,
                                   unsigned *out_offset,
                                   unsigned *out_length)
{
   /* The compiler computes the first URB slot without considering COL/BFC
    * swizzling (because it doesn't know whether it's enabled), so we need
    * to do that here too.  This may result in a smaller offset, which
    * should be safe.
    */
   const unsigned first_slot =
      brw_compute_first_urb_slot_required(fs_input_slots, last_vue_map);

   /* This becomes the URB read offset (counted in pairs of slots). */
   assert(first_slot % 2 == 0);
   *out_offset = first_slot / 2;

   /* We need to adjust the inputs read to account for front/back color
    * swizzling, as it can make the URB length longer.
    */
   for (int c = 0; c <= 1; c++) {
      if (fs_input_slots & (VARYING_BIT_COL0 << c)) {
         /* If two sided color is enabled, the fragment shader's gl_Color
          * (COL0) input comes from either the gl_FrontColor (COL0) or
          * gl_BackColor (BFC0) input varyings.  Mark BFC as used, too.
          */
         if (two_sided_color)
            fs_input_slots |= (VARYING_BIT_BFC0 << c);

         /* If front color isn't written, we opt to give them back color
          * instead of an undefined value.  Switch from COL to BFC.
          */
         if (last_vue_map->varying_to_slot[VARYING_SLOT_COL0 + c] == -1) {
            fs_input_slots &= ~(VARYING_BIT_COL0 << c);
            fs_input_slots |= (VARYING_BIT_BFC0 << c);
         }
      }
   }

   /* Compute the minimum URB Read Length necessary for the FS inputs.
    *
    * From the Sandy Bridge PRM, Volume 2, Part 1, documentation for
    * 3DSTATE_SF DWord 1 bits 15:11, "Vertex URB Entry Read Length":
    *
    * "This field should be set to the minimum length required to read the
    *  maximum source attribute.  The maximum source attribute is indicated
    *  by the maximum value of the enabled Attribute # Source Attribute if
    *  Attribute Swizzle Enable is set, Number of Output Attributes-1 if
    *  enable is not set.
    *  read_length = ceiling((max_source_attr + 1) / 2)
    *
    *  [errata] Corruption/Hang possible if length programmed larger than
    *  recommended"
    *
    * Similar text exists for Ivy Bridge.
    *
    * We find the last URB slot that's actually read by the FS.
    */
   unsigned last_read_slot = last_vue_map->num_slots - 1;
   while (last_read_slot > first_slot && !(fs_input_slots &
          (1ull << last_vue_map->slot_to_varying[last_read_slot])))
      --last_read_slot;

   /* The URB read length is the difference of the two, counted in pairs. */
   *out_length = DIV_ROUND_UP(last_read_slot - first_slot + 1, 2);
}

static void
iris_emit_sbe_swiz(struct iris_batch *batch,
                   const struct iris_context *ice,
                   const struct brw_vue_map *vue_map,
                   unsigned urb_read_offset,
                   unsigned sprite_coord_enables)
{
   struct GENX(SF_OUTPUT_ATTRIBUTE_DETAIL) attr_overrides[16] = {};
   const struct brw_wm_prog_data *wm_prog_data = (void *)
      ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data;
   const struct iris_rasterizer_state *cso_rast = ice->state.cso_rast;

   /* XXX: this should be generated when putting programs in place */

   for (uint8_t idx = 0; idx < wm_prog_data->urb_setup_attribs_count; idx++) {
      const uint8_t fs_attr = wm_prog_data->urb_setup_attribs[idx];
      const int input_index = wm_prog_data->urb_setup[fs_attr];
      if (input_index < 0 || input_index >= 16)
         continue;

      struct GENX(SF_OUTPUT_ATTRIBUTE_DETAIL) *attr =
         &attr_overrides[input_index];
      int slot = vue_map->varying_to_slot[fs_attr];

      /* Viewport and Layer are stored in the VUE header.  We need to override
       * them to zero if earlier stages didn't write them, as GL requires that
       * they read back as zero when not explicitly set.
       */
      switch (fs_attr) {
      case VARYING_SLOT_VIEWPORT:
      case VARYING_SLOT_LAYER:
         attr->ComponentOverrideX = true;
         attr->ComponentOverrideW = true;
         attr->ConstantSource = CONST_0000;

         if (!(vue_map->slots_valid & VARYING_BIT_LAYER))
            attr->ComponentOverrideY = true;
         if (!(vue_map->slots_valid & VARYING_BIT_VIEWPORT))
            attr->ComponentOverrideZ = true;
         continue;

      default:
         break;
      }

      if (sprite_coord_enables & (1 << input_index))
         continue;

      /* If there was only a back color written but not front, use back
       * as the color instead of undefined.
       */
      if (slot == -1 && fs_attr == VARYING_SLOT_COL0)
         slot = vue_map->varying_to_slot[VARYING_SLOT_BFC0];
      if (slot == -1 && fs_attr == VARYING_SLOT_COL1)
         slot = vue_map->varying_to_slot[VARYING_SLOT_BFC1];

      /* Not written by the previous stage - undefined. */
      if (slot == -1) {
         attr->ComponentOverrideX = true;
         attr->ComponentOverrideY = true;
         attr->ComponentOverrideZ = true;
         attr->ComponentOverrideW = true;
         attr->ConstantSource = CONST_0001_FLOAT;
         continue;
      }

      /* Compute the location of the attribute relative to the read offset,
       * which is counted in 256-bit increments (two 128-bit VUE slots).
       */
      const int source_attr = slot - 2 * urb_read_offset;
      assert(source_attr >= 0 && source_attr <= 32);
      attr->SourceAttribute = source_attr;

      /* If we are doing two-sided color, and the VUE slot following this one
       * represents a back-facing color, then we need to instruct the SF unit
       * to do back-facing swizzling.
       */
      if (cso_rast->light_twoside &&
          ((vue_map->slot_to_varying[slot] == VARYING_SLOT_COL0 &&
            vue_map->slot_to_varying[slot+1] == VARYING_SLOT_BFC0) ||
           (vue_map->slot_to_varying[slot] == VARYING_SLOT_COL1 &&
            vue_map->slot_to_varying[slot+1] == VARYING_SLOT_BFC1)))
         attr->SwizzleSelect = INPUTATTR_FACING;
   }

   iris_emit_cmd(batch, GENX(3DSTATE_SBE_SWIZ), sbes) {
      for (int i = 0; i < 16; i++)
         sbes.Attribute[i] = attr_overrides[i];
   }
}

static bool
iris_is_drawing_points(const struct iris_context *ice)
{
   const struct iris_rasterizer_state *cso_rast = ice->state.cso_rast;

   if (cso_rast->fill_mode_point) {
      return true;
   }

   if (ice->shaders.prog[MESA_SHADER_GEOMETRY]) {
      const struct brw_gs_prog_data *gs_prog_data =
         (void *) ice->shaders.prog[MESA_SHADER_GEOMETRY]->prog_data;
      return gs_prog_data->output_topology == _3DPRIM_POINTLIST;
   } else if (ice->shaders.prog[MESA_SHADER_TESS_EVAL]) {
      const struct brw_tes_prog_data *tes_data =
         (void *) ice->shaders.prog[MESA_SHADER_TESS_EVAL]->prog_data;
      return tes_data->output_topology == BRW_TESS_OUTPUT_TOPOLOGY_POINT;
   } else {
      return ice->state.prim_mode == MESA_PRIM_POINTS;
   }
}

static unsigned
iris_calculate_point_sprite_overrides(const struct brw_wm_prog_data *prog_data,
                                      const struct iris_rasterizer_state *cso)
{
   unsigned overrides = 0;

   if (prog_data->urb_setup[VARYING_SLOT_PNTC] != -1)
      overrides |= 1 << prog_data->urb_setup[VARYING_SLOT_PNTC];

   for (int i = 0; i < 8; i++) {
      if ((cso->sprite_coord_enable & (1 << i)) &&
          prog_data->urb_setup[VARYING_SLOT_TEX0 + i] != -1)
         overrides |= 1 << prog_data->urb_setup[VARYING_SLOT_TEX0 + i];
   }

   return overrides;
}

static void
iris_emit_sbe(struct iris_batch *batch, const struct iris_context *ice)
{
   const struct iris_rasterizer_state *cso_rast = ice->state.cso_rast;
   const struct brw_wm_prog_data *wm_prog_data = (void *)
      ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data;
   const struct brw_vue_map *last_vue_map =
      &brw_vue_prog_data(ice->shaders.last_vue_shader->prog_data)->vue_map;

   unsigned urb_read_offset, urb_read_length;
   iris_compute_sbe_urb_read_interval(wm_prog_data->inputs,
                                      last_vue_map,
                                      cso_rast->light_twoside,
                                      &urb_read_offset, &urb_read_length);

   unsigned sprite_coord_overrides =
      iris_is_drawing_points(ice) ?
      iris_calculate_point_sprite_overrides(wm_prog_data, cso_rast) : 0;

   iris_emit_cmd(batch, GENX(3DSTATE_SBE), sbe) {
      sbe.AttributeSwizzleEnable = true;
      sbe.NumberofSFOutputAttributes = wm_prog_data->num_varying_inputs;
      sbe.PointSpriteTextureCoordinateOrigin = cso_rast->sprite_coord_mode;
      sbe.VertexURBEntryReadOffset = urb_read_offset;
      sbe.VertexURBEntryReadLength = urb_read_length;
      sbe.ForceVertexURBEntryReadOffset = true;
      sbe.ForceVertexURBEntryReadLength = true;
      sbe.ConstantInterpolationEnable = wm_prog_data->flat_inputs;
      sbe.PointSpriteTextureCoordinateEnable = sprite_coord_overrides;
#if GFX_VER >= 9
      for (int i = 0; i < 32; i++) {
         sbe.AttributeActiveComponentFormat[i] = ACTIVE_COMPONENT_XYZW;
      }
#endif

      /* Ask the hardware to supply PrimitiveID if the fragment shader
       * reads it but a previous stage didn't write one.
       */
      if ((wm_prog_data->inputs & VARYING_BIT_PRIMITIVE_ID) &&
          last_vue_map->varying_to_slot[VARYING_SLOT_PRIMITIVE_ID] == -1) {
         sbe.PrimitiveIDOverrideAttributeSelect =
            wm_prog_data->urb_setup[VARYING_SLOT_PRIMITIVE_ID];
         sbe.PrimitiveIDOverrideComponentX = true;
         sbe.PrimitiveIDOverrideComponentY = true;
         sbe.PrimitiveIDOverrideComponentZ = true;
         sbe.PrimitiveIDOverrideComponentW = true;
      }
   }

   iris_emit_sbe_swiz(batch, ice, last_vue_map, urb_read_offset,
                      sprite_coord_overrides);
}

/* ------------------------------------------------------------------- */

/**
 * Populate VS program key fields based on the current state.
 */
static void
iris_populate_vs_key(const struct iris_context *ice,
                     const struct shader_info *info,
                     gl_shader_stage last_stage,
                     struct iris_vs_prog_key *key)
{
   const struct iris_rasterizer_state *cso_rast = ice->state.cso_rast;

   if (info->clip_distance_array_size == 0 &&
       (info->outputs_written & (VARYING_BIT_POS | VARYING_BIT_CLIP_VERTEX)) &&
       last_stage == MESA_SHADER_VERTEX)
      key->vue.nr_userclip_plane_consts = cso_rast->num_clip_plane_consts;
}

/**
 * Populate TCS program key fields based on the current state.
 */
static void
iris_populate_tcs_key(const struct iris_context *ice,
                      struct iris_tcs_prog_key *key)
{
}

/**
 * Populate TES program key fields based on the current state.
 */
static void
iris_populate_tes_key(const struct iris_context *ice,
                      const struct shader_info *info,
                      gl_shader_stage last_stage,
                      struct iris_tes_prog_key *key)
{
   const struct iris_rasterizer_state *cso_rast = ice->state.cso_rast;

   if (info->clip_distance_array_size == 0 &&
       (info->outputs_written & (VARYING_BIT_POS | VARYING_BIT_CLIP_VERTEX)) &&
       last_stage == MESA_SHADER_TESS_EVAL)
      key->vue.nr_userclip_plane_consts = cso_rast->num_clip_plane_consts;
}

/**
 * Populate GS program key fields based on the current state.
 */
static void
iris_populate_gs_key(const struct iris_context *ice,
                     const struct shader_info *info,
                     gl_shader_stage last_stage,
                     struct iris_gs_prog_key *key)
{
   const struct iris_rasterizer_state *cso_rast = ice->state.cso_rast;

   if (info->clip_distance_array_size == 0 &&
       (info->outputs_written & (VARYING_BIT_POS | VARYING_BIT_CLIP_VERTEX)) &&
       last_stage == MESA_SHADER_GEOMETRY)
      key->vue.nr_userclip_plane_consts = cso_rast->num_clip_plane_consts;
}

/**
 * Populate FS program key fields based on the current state.
 */
static void
iris_populate_fs_key(const struct iris_context *ice,
                     const struct shader_info *info,
                     struct iris_fs_prog_key *key)
{
   struct iris_screen *screen = (void *) ice->ctx.screen;
   const struct pipe_framebuffer_state *fb = &ice->state.framebuffer;
   const struct iris_depth_stencil_alpha_state *zsa = ice->state.cso_zsa;
   const struct iris_rasterizer_state *rast = ice->state.cso_rast;
   const struct iris_blend_state *blend = ice->state.cso_blend;

   key->nr_color_regions = fb->nr_cbufs;

   key->clamp_fragment_color = rast->clamp_fragment_color;

   key->alpha_to_coverage = blend->alpha_to_coverage;

   key->alpha_test_replicate_alpha = fb->nr_cbufs > 1 && zsa->alpha_enabled;

   key->flat_shade = rast->flatshade &&
      (info->inputs_read & (VARYING_BIT_COL0 | VARYING_BIT_COL1));

   key->persample_interp = rast->force_persample_interp;
   key->multisample_fbo = rast->multisample && fb->samples > 1;

   key->coherent_fb_fetch = GFX_VER >= 9;

   key->force_dual_color_blend =
      screen->driconf.dual_color_blend_by_location &&
      (blend->blend_enables & 1) && blend->dual_color_blending;
}

static void
iris_populate_cs_key(const struct iris_context *ice,
                     struct iris_cs_prog_key *key)
{
}

static uint64_t
KSP(const struct iris_compiled_shader *shader)
{
   struct iris_resource *res = (void *) shader->assembly.res;
   return iris_bo_offset_from_base_address(res->bo) + shader->assembly.offset;
}

static uint32_t
encode_sampler_count(const struct iris_compiled_shader *shader)
{
   uint32_t count = util_last_bit64(shader->bt.samplers_used_mask);
   uint32_t count_by_4 = DIV_ROUND_UP(count, 4);

   /* We can potentially have way more than 32 samplers and that's ok.
    * However, the 3DSTATE_XS packets only have 3 bits to specify how
    * many to pre-fetch and all values above 4 are marked reserved.
    */
   return MIN2(count_by_4, 4);
}

#define INIT_THREAD_DISPATCH_FIELDS(pkt, prefix, stage)                   \
   pkt.KernelStartPointer = KSP(shader);                                  \
   pkt.BindingTableEntryCount = shader->bt.size_bytes / 4;                \
   pkt.SamplerCount = encode_sampler_count(shader);                       \
   pkt.FloatingPointMode = prog_data->use_alt_mode;                       \
                                                                          \
   pkt.DispatchGRFStartRegisterForURBData =                               \
      prog_data->dispatch_grf_start_reg;                                  \
   pkt.prefix##URBEntryReadLength = vue_prog_data->urb_read_length;       \
   pkt.prefix##URBEntryReadOffset = 0;                                    \
                                                                          \
   pkt.StatisticsEnable = true;                                           \
   pkt.Enable           = true;                                           \
                                                                          \
   if (prog_data->total_scratch) {                                        \
      INIT_THREAD_SCRATCH_SIZE(pkt)                                       \
   }

#if GFX_VERx10 >= 125
#define INIT_THREAD_SCRATCH_SIZE(pkt)
#define MERGE_SCRATCH_ADDR(name)                                          \
{                                                                         \
   uint32_t pkt2[GENX(name##_length)] = {0};                              \
   _iris_pack_command(batch, GENX(name), pkt2, p) {                       \
      p.ScratchSpaceBuffer = scratch_addr >> 4;                           \
   }                                                                      \
   iris_emit_merge(batch, pkt, pkt2, GENX(name##_length));                \
}
#else
#define INIT_THREAD_SCRATCH_SIZE(pkt)                                     \
   pkt.PerThreadScratchSpace = ffs(prog_data->total_scratch) - 11;
#define MERGE_SCRATCH_ADDR(name)                                          \
{                                                                         \
   uint32_t pkt2[GENX(name##_length)] = {0};                              \
   _iris_pack_command(batch, GENX(name), pkt2, p) {                       \
      p.ScratchSpaceBasePointer =                                         \
         rw_bo(NULL, scratch_addr, IRIS_DOMAIN_NONE);                     \
   }                                                                      \
   iris_emit_merge(batch, pkt, pkt2, GENX(name##_length));                \
}
#endif


/**
 * Encode most of 3DSTATE_VS based on the compiled shader.
 */
static void
iris_store_vs_state(const struct intel_device_info *devinfo,
                    struct iris_compiled_shader *shader)
{
   struct brw_stage_prog_data *prog_data = shader->prog_data;
   struct brw_vue_prog_data *vue_prog_data = (void *) prog_data;

   iris_pack_command(GENX(3DSTATE_VS), shader->derived_data, vs) {
      INIT_THREAD_DISPATCH_FIELDS(vs, Vertex, MESA_SHADER_VERTEX);
      vs.MaximumNumberofThreads = devinfo->max_vs_threads - 1;
#if GFX_VER < 20
      vs.SIMD8DispatchEnable = true;
#endif
      vs.UserClipDistanceCullTestEnableBitmask =
         vue_prog_data->cull_distance_mask;
   }
}

/**
 * Encode most of 3DSTATE_HS based on the compiled shader.
 */
static void
iris_store_tcs_state(const struct intel_device_info *devinfo,
                     struct iris_compiled_shader *shader)
{
   struct brw_stage_prog_data *prog_data = shader->prog_data;
   struct brw_vue_prog_data *vue_prog_data = (void *) prog_data;
   struct brw_tcs_prog_data *tcs_prog_data = (void *) prog_data;

   iris_pack_command(GENX(3DSTATE_HS), shader->derived_data, hs) {
      INIT_THREAD_DISPATCH_FIELDS(hs, Vertex, MESA_SHADER_TESS_CTRL);

#if GFX_VER >= 12
      /* Wa_1604578095:
       *
       *    Hang occurs when the number of max threads is less than 2 times
       *    the number of instance count. The number of max threads must be
       *    more than 2 times the number of instance count.
       */
      assert((devinfo->max_tcs_threads / 2) > tcs_prog_data->instances);
      hs.DispatchGRFStartRegisterForURBData = prog_data->dispatch_grf_start_reg & 0x1f;
      hs.DispatchGRFStartRegisterForURBData5 = prog_data->dispatch_grf_start_reg >> 5;
#endif

      hs.InstanceCount = tcs_prog_data->instances - 1;
      hs.MaximumNumberofThreads = devinfo->max_tcs_threads - 1;
      hs.IncludeVertexHandles = true;

#if GFX_VER == 12
      /* Patch Count threshold specifies the maximum number of patches that
       * will be accumulated before a thread dispatch is forced.
       */
      hs.PatchCountThreshold = tcs_prog_data->patch_count_threshold;
#endif

#if GFX_VER >= 9
#if GFX_VER < 20
      hs.DispatchMode = vue_prog_data->dispatch_mode;
#endif
      hs.IncludePrimitiveID = tcs_prog_data->include_primitive_id;
#endif
   }
}

/**
 * Encode 3DSTATE_TE and most of 3DSTATE_DS based on the compiled shader.
 */
static void
iris_store_tes_state(const struct intel_device_info *devinfo,
                     struct iris_compiled_shader *shader)
{
   struct brw_stage_prog_data *prog_data = shader->prog_data;
   struct brw_vue_prog_data *vue_prog_data = (void *) prog_data;
   struct brw_tes_prog_data *tes_prog_data = (void *) prog_data;

   uint32_t *ds_state = (void *) shader->derived_data;
   uint32_t *te_state = ds_state + GENX(3DSTATE_DS_length);

   iris_pack_command(GENX(3DSTATE_DS), ds_state, ds) {
      INIT_THREAD_DISPATCH_FIELDS(ds, Patch, MESA_SHADER_TESS_EVAL);

      ds.DispatchMode = DISPATCH_MODE_SIMD8_SINGLE_PATCH;
      ds.MaximumNumberofThreads = devinfo->max_tes_threads - 1;
      ds.ComputeWCoordinateEnable =
         tes_prog_data->domain == BRW_TESS_DOMAIN_TRI;

#if GFX_VER >= 12
      ds.PrimitiveIDNotRequired = !tes_prog_data->include_primitive_id;
#endif
      ds.UserClipDistanceCullTestEnableBitmask =
         vue_prog_data->cull_distance_mask;
   }

   iris_pack_command(GENX(3DSTATE_TE), te_state, te) {
      te.Partitioning = tes_prog_data->partitioning;
      te.OutputTopology = tes_prog_data->output_topology;
      te.TEDomain = tes_prog_data->domain;
      te.TEEnable = true;
      te.MaximumTessellationFactorOdd = 63.0;
      te.MaximumTessellationFactorNotOdd = 64.0;
#if GFX_VERx10 >= 125
      STATIC_ASSERT(TEDMODE_OFF == 0);
      if (intel_needs_workaround(devinfo, 14015055625)) {
         te.TessellationDistributionMode = TEDMODE_OFF;
      } else if (intel_needs_workaround(devinfo, 22012699309)) {
         te.TessellationDistributionMode = TEDMODE_RR_STRICT;
      } else {
         te.TessellationDistributionMode = TEDMODE_RR_FREE;
      }

   #if GFX_VER >= 20
      te.TessellationDistributionLevel = TEDLEVEL_REGION;
   #else
      te.TessellationDistributionLevel = TEDLEVEL_PATCH;
   #endif
      /* 64_TRIANGLES */
      te.SmallPatchThreshold = 3;
      /* 1K_TRIANGLES */
      te.TargetBlockSize = 8;
      /* 1K_TRIANGLES */
      te.LocalBOPAccumulatorThreshold = 1;
#endif
   }
}

/**
 * Encode most of 3DSTATE_GS based on the compiled shader.
 */
static void
iris_store_gs_state(const struct intel_device_info *devinfo,
                    struct iris_compiled_shader *shader)
{
   struct brw_stage_prog_data *prog_data = shader->prog_data;
   struct brw_vue_prog_data *vue_prog_data = (void *) prog_data;
   struct brw_gs_prog_data *gs_prog_data = (void *) prog_data;

   iris_pack_command(GENX(3DSTATE_GS), shader->derived_data, gs) {
      INIT_THREAD_DISPATCH_FIELDS(gs, Vertex, MESA_SHADER_GEOMETRY);

      gs.OutputVertexSize = gs_prog_data->output_vertex_size_hwords * 2 - 1;
      gs.OutputTopology = gs_prog_data->output_topology;
      gs.ControlDataHeaderSize =
         gs_prog_data->control_data_header_size_hwords;
      gs.InstanceControl = gs_prog_data->invocations - 1;
#if GFX_VER < 20
      gs.DispatchMode = DISPATCH_MODE_SIMD8;
#endif
      gs.IncludePrimitiveID = gs_prog_data->include_primitive_id;
      gs.ControlDataFormat = gs_prog_data->control_data_format;
      gs.ReorderMode = TRAILING;
      gs.ExpectedVertexCount = gs_prog_data->vertices_in;
      gs.MaximumNumberofThreads =
         GFX_VER == 8 ? (devinfo->max_gs_threads / 2 - 1)
                      : (devinfo->max_gs_threads - 1);

      if (gs_prog_data->static_vertex_count != -1) {
         gs.StaticOutput = true;
         gs.StaticOutputVertexCount = gs_prog_data->static_vertex_count;
      }
      gs.IncludeVertexHandles = vue_prog_data->include_vue_handles;

      gs.UserClipDistanceCullTestEnableBitmask =
         vue_prog_data->cull_distance_mask;

      const int urb_entry_write_offset = 1;
      const uint32_t urb_entry_output_length =
         DIV_ROUND_UP(vue_prog_data->vue_map.num_slots, 2) -
         urb_entry_write_offset;

      gs.VertexURBEntryOutputReadOffset = urb_entry_write_offset;
      gs.VertexURBEntryOutputLength = MAX2(urb_entry_output_length, 1);
   }
}

/**
 * Encode most of 3DSTATE_PS and 3DSTATE_PS_EXTRA based on the shader.
 */
static void
iris_store_fs_state(const struct intel_device_info *devinfo,
                    struct iris_compiled_shader *shader)
{
   struct brw_stage_prog_data *prog_data = shader->prog_data;
   struct brw_wm_prog_data *wm_prog_data = (void *) shader->prog_data;

   uint32_t *ps_state = (void *) shader->derived_data;
   uint32_t *psx_state = ps_state + GENX(3DSTATE_PS_length);

   iris_pack_command(GENX(3DSTATE_PS), ps_state, ps) {
      ps.VectorMaskEnable = wm_prog_data->uses_vmask;
      ps.BindingTableEntryCount = shader->bt.size_bytes / 4;
      ps.SamplerCount = encode_sampler_count(shader);
      ps.FloatingPointMode = prog_data->use_alt_mode;
      ps.MaximumNumberofThreadsPerPSD =
         devinfo->max_threads_per_psd - (GFX_VER == 8 ? 2 : 1);

#if GFX_VER < 20
      ps.PushConstantEnable = prog_data->ubo_ranges[0].length > 0;
#endif

      /* From the documentation for this packet:
       * "If the PS kernel does not need the Position XY Offsets to
       *  compute a Position Value, then this field should be programmed
       *  to POSOFFSET_NONE."
       *
       * "SW Recommendation: If the PS kernel needs the Position Offsets
       *  to compute a Position XY value, this field should match Position
       *  ZW Interpolation Mode to ensure a consistent position.xyzw
       *  computation."
       *
       * We only require XY sample offsets. So, this recommendation doesn't
       * look useful at the moment.  We might need this in future.
       */
      ps.PositionXYOffsetSelect =
         wm_prog_data->uses_pos_offset ? POSOFFSET_SAMPLE : POSOFFSET_NONE;

      if (prog_data->total_scratch) {
         INIT_THREAD_SCRATCH_SIZE(ps);
      }
   }

   iris_pack_command(GENX(3DSTATE_PS_EXTRA), psx_state, psx) {
      psx.PixelShaderValid = true;
      psx.PixelShaderComputedDepthMode = wm_prog_data->computed_depth_mode;
      psx.PixelShaderKillsPixel = wm_prog_data->uses_kill;
#if GFX_VER < 20
      psx.AttributeEnable = wm_prog_data->num_varying_inputs != 0;
#endif
      psx.PixelShaderUsesSourceDepth = wm_prog_data->uses_src_depth;
      psx.PixelShaderUsesSourceW = wm_prog_data->uses_src_w;
      psx.PixelShaderIsPerSample =
         brw_wm_prog_data_is_persample(wm_prog_data, 0);
      psx.oMaskPresenttoRenderTarget = wm_prog_data->uses_omask;

#if GFX_VER >= 9
#if GFX_VER >= 20
      assert(!wm_prog_data->pulls_bary);
#else
      psx.PixelShaderPullsBary = wm_prog_data->pulls_bary;
#endif
      psx.PixelShaderComputesStencil = wm_prog_data->computed_stencil;
#endif
   }
}

/**
 * Compute the size of the derived data (shader command packets).
 *
 * This must match the data written by the iris_store_xs_state() functions.
 */
static void
iris_store_cs_state(const struct intel_device_info *devinfo,
                    struct iris_compiled_shader *shader)
{
   struct brw_cs_prog_data *cs_prog_data = (void *) shader->prog_data;
   void *map = shader->derived_data;

   iris_pack_state(GENX(INTERFACE_DESCRIPTOR_DATA), map, desc) {
#if GFX_VERx10 < 125
      desc.ConstantURBEntryReadLength = cs_prog_data->push.per_thread.regs;
      desc.CrossThreadConstantDataReadLength =
         cs_prog_data->push.cross_thread.regs;
#else
      assert(cs_prog_data->push.per_thread.regs == 0);
      assert(cs_prog_data->push.cross_thread.regs == 0);
#endif
#if GFX_VERx10 <= 125
      desc.BarrierEnable = cs_prog_data->uses_barrier;
#endif
      /* Typically set to 0 to avoid prefetching on every thread dispatch. */
      desc.BindingTableEntryCount = devinfo->verx10 == 125 ?
         0 : MIN2(shader->bt.size_bytes / 4, 31);
      desc.SamplerCount = encode_sampler_count(shader);
      /* TODO: Check if we are missing workarounds and enable mid-thread
       * preemption.
       *
       * We still have issues with mid-thread preemption (it was already
       * disabled by the kernel on gfx11, due to missing workarounds). It's
       * possible that we are just missing some workarounds, and could enable
       * it later, but for now let's disable it to fix a GPU in compute in Car
       * Chase (and possibly more).
       */
#if GFX_VER >= 20
      desc.ThreadPreemption = false;
#elif GFX_VER >= 12
      desc.ThreadPreemptionDisable = true;
#endif
   }
}

static unsigned
iris_derived_program_state_size(enum iris_program_cache_id cache_id)
{
   assert(cache_id <= IRIS_CACHE_BLORP);

   static const unsigned dwords[] = {
      [IRIS_CACHE_VS] = GENX(3DSTATE_VS_length),
      [IRIS_CACHE_TCS] = GENX(3DSTATE_HS_length),
      [IRIS_CACHE_TES] = GENX(3DSTATE_TE_length) + GENX(3DSTATE_DS_length),
      [IRIS_CACHE_GS] = GENX(3DSTATE_GS_length),
      [IRIS_CACHE_FS] =
         GENX(3DSTATE_PS_length) + GENX(3DSTATE_PS_EXTRA_length),
      [IRIS_CACHE_CS] = GENX(INTERFACE_DESCRIPTOR_DATA_length),
      [IRIS_CACHE_BLORP] = 0,
   };

   return sizeof(uint32_t) * dwords[cache_id];
}

/**
 * Create any state packets corresponding to the given shader stage
 * (i.e. 3DSTATE_VS) and save them as "derived data" in the shader variant.
 * This means that we can look up a program in the in-memory cache and
 * get most of the state packet without having to reconstruct it.
 */
static void
iris_store_derived_program_state(const struct intel_device_info *devinfo,
                                 enum iris_program_cache_id cache_id,
                                 struct iris_compiled_shader *shader)
{
   switch (cache_id) {
   case IRIS_CACHE_VS:
      iris_store_vs_state(devinfo, shader);
      break;
   case IRIS_CACHE_TCS:
      iris_store_tcs_state(devinfo, shader);
      break;
   case IRIS_CACHE_TES:
      iris_store_tes_state(devinfo, shader);
      break;
   case IRIS_CACHE_GS:
      iris_store_gs_state(devinfo, shader);
      break;
   case IRIS_CACHE_FS:
      iris_store_fs_state(devinfo, shader);
      break;
   case IRIS_CACHE_CS:
      iris_store_cs_state(devinfo, shader);
      break;
   case IRIS_CACHE_BLORP:
      break;
   }
}

/* ------------------------------------------------------------------- */

static const uint32_t push_constant_opcodes[] = {
   [MESA_SHADER_VERTEX]    = 21,
   [MESA_SHADER_TESS_CTRL] = 25, /* HS */
   [MESA_SHADER_TESS_EVAL] = 26, /* DS */
   [MESA_SHADER_GEOMETRY]  = 22,
   [MESA_SHADER_FRAGMENT]  = 23,
   [MESA_SHADER_COMPUTE]   = 0,
};

static uint32_t
use_null_surface(struct iris_batch *batch, struct iris_context *ice)
{
   struct iris_bo *state_bo = iris_resource_bo(ice->state.unbound_tex.res);

   iris_use_pinned_bo(batch, state_bo, false, IRIS_DOMAIN_NONE);

   return ice->state.unbound_tex.offset;
}

static uint32_t
use_null_fb_surface(struct iris_batch *batch, struct iris_context *ice)
{
   /* If set_framebuffer_state() was never called, fall back to 1x1x1 */
   if (!ice->state.null_fb.res)
      return use_null_surface(batch, ice);

   struct iris_bo *state_bo = iris_resource_bo(ice->state.null_fb.res);

   iris_use_pinned_bo(batch, state_bo, false, IRIS_DOMAIN_NONE);

   return ice->state.null_fb.offset;
}

static uint32_t
surf_state_offset_for_aux(unsigned aux_modes,
                          enum isl_aux_usage aux_usage)
{
   assert(aux_modes & (1 << aux_usage));
   return SURFACE_STATE_ALIGNMENT *
          util_bitcount(aux_modes & ((1 << aux_usage) - 1));
}

#if GFX_VER == 9
static void
surf_state_update_clear_value(struct iris_batch *batch,
                              struct iris_resource *res,
                              struct iris_surface_state *surf_state,
                              enum isl_aux_usage aux_usage)
{
   struct isl_device *isl_dev = &batch->screen->isl_dev;
   struct iris_bo *state_bo = iris_resource_bo(surf_state->ref.res);
   uint64_t real_offset = surf_state->ref.offset + IRIS_MEMZONE_BINDER_START;
   uint32_t offset_into_bo = real_offset - state_bo->address;
   uint32_t clear_offset = offset_into_bo +
      isl_dev->ss.clear_value_offset +
      surf_state_offset_for_aux(surf_state->aux_usages, aux_usage);
   uint32_t *color = res->aux.clear_color.u32;

   assert(isl_dev->ss.clear_value_size == 16);

   if (aux_usage == ISL_AUX_USAGE_HIZ) {
      iris_emit_pipe_control_write(batch, "update fast clear value (Z)",
                                   PIPE_CONTROL_WRITE_IMMEDIATE,
                                   state_bo, clear_offset, color[0]);
   } else {
      iris_emit_pipe_control_write(batch, "update fast clear color (RG__)",
                                   PIPE_CONTROL_WRITE_IMMEDIATE,
                                   state_bo, clear_offset,
                                   (uint64_t) color[0] |
                                   (uint64_t) color[1] << 32);
      iris_emit_pipe_control_write(batch, "update fast clear color (__BA)",
                                   PIPE_CONTROL_WRITE_IMMEDIATE,
                                   state_bo, clear_offset + 8,
                                   (uint64_t) color[2] |
                                   (uint64_t) color[3] << 32);
   }

   iris_emit_pipe_control_flush(batch,
                                "update fast clear: state cache invalidate",
                                PIPE_CONTROL_FLUSH_ENABLE |
                                PIPE_CONTROL_STATE_CACHE_INVALIDATE);
}
#endif

static void
update_clear_value(struct iris_context *ice,
                   struct iris_batch *batch,
                   struct iris_resource *res,
                   struct iris_surface_state *surf_state,
                   struct isl_view *view)
{
   UNUSED struct isl_device *isl_dev = &batch->screen->isl_dev;
   UNUSED unsigned aux_modes = surf_state->aux_usages;

   /* We only need to update the clear color in the surface state for gfx8 and
    * gfx9. Newer gens can read it directly from the clear color state buffer.
    */
#if GFX_VER == 9
   /* Skip updating the ISL_AUX_USAGE_NONE surface state */
   aux_modes &= ~(1 << ISL_AUX_USAGE_NONE);

   while (aux_modes) {
      enum isl_aux_usage aux_usage = u_bit_scan(&aux_modes);

      surf_state_update_clear_value(batch, res, surf_state, aux_usage);
   }
#elif GFX_VER == 8
   /* TODO: Could update rather than re-filling */
   alloc_surface_states(surf_state, surf_state->aux_usages);

   fill_surface_states(isl_dev, surf_state, res, &res->surf, view, 0, 0, 0);

   upload_surface_states(ice->state.surface_uploader, surf_state);
#endif
}

static uint32_t
use_surface_state(struct iris_batch *batch,
                  struct iris_surface_state *surf_state,
                  enum isl_aux_usage aux_usage)
{
   iris_use_pinned_bo(batch, iris_resource_bo(surf_state->ref.res), false,
                      IRIS_DOMAIN_NONE);

   return surf_state->ref.offset +
          surf_state_offset_for_aux(surf_state->aux_usages, aux_usage);
}

/**
 * Add a surface to the validation list, as well as the buffer containing
 * the corresponding SURFACE_STATE.
 *
 * Returns the binding table entry (offset to SURFACE_STATE).
 */
static uint32_t
use_surface(struct iris_context *ice,
            struct iris_batch *batch,
            struct pipe_surface *p_surf,
            bool writeable,
            enum isl_aux_usage aux_usage,
            bool is_read_surface,
            enum iris_domain access)
{
   struct iris_surface *surf = (void *) p_surf;
   struct iris_resource *res = (void *) p_surf->texture;

   if (GFX_VER == 8 && is_read_surface && !surf->surface_state_read.ref.res) {
      upload_surface_states(ice->state.surface_uploader,
                            &surf->surface_state_read);
   }

   if (!surf->surface_state.ref.res) {
      upload_surface_states(ice->state.surface_uploader,
                            &surf->surface_state);
   }

   if (memcmp(&res->aux.clear_color, &surf->clear_color,
              sizeof(surf->clear_color)) != 0) {
      update_clear_value(ice, batch, res, &surf->surface_state, &surf->view);
      if (GFX_VER == 8) {
         update_clear_value(ice, batch, res, &surf->surface_state_read,
                            &surf->read_view);
      }
      surf->clear_color = res->aux.clear_color;
   }

   if (res->aux.clear_color_bo)
      iris_use_pinned_bo(batch, res->aux.clear_color_bo, false, access);

   if (res->aux.bo)
      iris_use_pinned_bo(batch, res->aux.bo, writeable, access);

   iris_use_pinned_bo(batch, res->bo, writeable, access);

   if (GFX_VER == 8 && is_read_surface) {
      return use_surface_state(batch, &surf->surface_state_read, aux_usage);
   } else {
      return use_surface_state(batch, &surf->surface_state, aux_usage);
   }
}

static uint32_t
use_sampler_view(struct iris_context *ice,
                 struct iris_batch *batch,
                 struct iris_sampler_view *isv)
{
   enum isl_aux_usage aux_usage =
      iris_resource_texture_aux_usage(ice, isv->res, isv->view.format,
                                      isv->view.base_level, isv->view.levels);

   if (!isv->surface_state.ref.res)
      upload_surface_states(ice->state.surface_uploader, &isv->surface_state);

   if (memcmp(&isv->res->aux.clear_color, &isv->clear_color,
              sizeof(isv->clear_color)) != 0) {
      update_clear_value(ice, batch, isv->res, &isv->surface_state,
                         &isv->view);
      isv->clear_color = isv->res->aux.clear_color;
   }

   if (isv->res->aux.clear_color_bo) {
      iris_use_pinned_bo(batch, isv->res->aux.clear_color_bo,
                         false, IRIS_DOMAIN_SAMPLER_READ);
   }

   if (isv->res->aux.bo) {
      iris_use_pinned_bo(batch, isv->res->aux.bo,
                         false, IRIS_DOMAIN_SAMPLER_READ);
   }

   iris_use_pinned_bo(batch, isv->res->bo, false, IRIS_DOMAIN_SAMPLER_READ);

   return use_surface_state(batch, &isv->surface_state, aux_usage);
}

static uint32_t
use_ubo_ssbo(struct iris_batch *batch,
             struct iris_context *ice,
             struct pipe_shader_buffer *buf,
             struct iris_state_ref *surf_state,
             bool writable, enum iris_domain access)
{
   if (!buf->buffer || !surf_state->res)
      return use_null_surface(batch, ice);

   iris_use_pinned_bo(batch, iris_resource_bo(buf->buffer), writable, access);
   iris_use_pinned_bo(batch, iris_resource_bo(surf_state->res), false,
                      IRIS_DOMAIN_NONE);

   return surf_state->offset;
}

static uint32_t
use_image(struct iris_batch *batch, struct iris_context *ice,
          struct iris_shader_state *shs, const struct shader_info *info,
          int i)
{
   struct iris_image_view *iv = &shs->image[i];
   struct iris_resource *res = (void *) iv->base.resource;

   if (!res)
      return use_null_surface(batch, ice);

   bool write = iv->base.shader_access & PIPE_IMAGE_ACCESS_WRITE;

   iris_use_pinned_bo(batch, res->bo, write, IRIS_DOMAIN_NONE);

   if (res->aux.bo)
      iris_use_pinned_bo(batch, res->aux.bo, write, IRIS_DOMAIN_NONE);

   if (res->aux.clear_color_bo) {
      iris_use_pinned_bo(batch, res->aux.clear_color_bo, false,
                         IRIS_DOMAIN_NONE);
   }

   enum isl_aux_usage aux_usage = shs->image_aux_usage[i];

   return use_surface_state(batch, &iv->surface_state, aux_usage);
}

#define push_bt_entry(addr) \
   assert(addr >= surf_base_offset); \
   assert(s < shader->bt.size_bytes / sizeof(uint32_t)); \
   if (!pin_only) bt_map[s++] = (addr) - surf_base_offset;

#define bt_assert(section) \
   if (!pin_only && shader->bt.used_mask[section] != 0) \
      assert(shader->bt.offsets[section] == s);

/**
 * Populate the binding table for a given shader stage.
 *
 * This fills out the table of pointers to surfaces required by the shader,
 * and also adds those buffers to the validation list so the kernel can make
 * resident before running our batch.
 */
static void
iris_populate_binding_table(struct iris_context *ice,
                            struct iris_batch *batch,
                            gl_shader_stage stage,
                            bool pin_only)
{
   const struct iris_binder *binder = &ice->state.binder;
   struct iris_compiled_shader *shader = ice->shaders.prog[stage];
   if (!shader)
      return;

   struct iris_binding_table *bt = &shader->bt;
   UNUSED struct brw_stage_prog_data *prog_data = shader->prog_data;
   struct iris_shader_state *shs = &ice->state.shaders[stage];
   uint32_t surf_base_offset = GFX_VER < 11 ? binder->bo->address : 0;

   uint32_t *bt_map = binder->map + binder->bt_offset[stage];
   int s = 0;

   const struct shader_info *info = iris_get_shader_info(ice, stage);
   if (!info) {
      /* TCS passthrough doesn't need a binding table. */
      assert(stage == MESA_SHADER_TESS_CTRL);
      return;
   }

   if (stage == MESA_SHADER_COMPUTE &&
       shader->bt.used_mask[IRIS_SURFACE_GROUP_CS_WORK_GROUPS]) {
      /* surface for gl_NumWorkGroups */
      struct iris_state_ref *grid_data = &ice->state.grid_size;
      struct iris_state_ref *grid_state = &ice->state.grid_surf_state;
      iris_use_pinned_bo(batch, iris_resource_bo(grid_data->res), false,
                         IRIS_DOMAIN_PULL_CONSTANT_READ);
      iris_use_pinned_bo(batch, iris_resource_bo(grid_state->res), false,
                         IRIS_DOMAIN_NONE);
      push_bt_entry(grid_state->offset);
   }

   if (stage == MESA_SHADER_FRAGMENT) {
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      /* Note that cso_fb->nr_cbufs == fs_key->nr_color_regions. */
      if (cso_fb->nr_cbufs) {
         for (unsigned i = 0; i < cso_fb->nr_cbufs; i++) {
            uint32_t addr;
            if (cso_fb->cbufs[i]) {
               addr = use_surface(ice, batch, cso_fb->cbufs[i], true,
                                  ice->state.draw_aux_usage[i], false,
                                  IRIS_DOMAIN_RENDER_WRITE);
            } else {
               addr = use_null_fb_surface(batch, ice);
            }
            push_bt_entry(addr);
         }
      } else if (GFX_VER < 11) {
         uint32_t addr = use_null_fb_surface(batch, ice);
         push_bt_entry(addr);
      }
   }

#define foreach_surface_used(index, group) \
   bt_assert(group); \
   for (int index = 0; index < bt->sizes[group]; index++) \
      if (iris_group_index_to_bti(bt, group, index) != \
          IRIS_SURFACE_NOT_USED)

   foreach_surface_used(i, IRIS_SURFACE_GROUP_RENDER_TARGET_READ) {
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      uint32_t addr;
      if (cso_fb->cbufs[i]) {
         addr = use_surface(ice, batch, cso_fb->cbufs[i],
                            false, ice->state.draw_aux_usage[i], true,
                            IRIS_DOMAIN_SAMPLER_READ);
         push_bt_entry(addr);
      }
   }

   foreach_surface_used(i, IRIS_SURFACE_GROUP_TEXTURE_LOW64) {
      struct iris_sampler_view *view = shs->textures[i];
      uint32_t addr = view ? use_sampler_view(ice, batch, view)
                           : use_null_surface(batch, ice);
      push_bt_entry(addr);
   }

   foreach_surface_used(i, IRIS_SURFACE_GROUP_TEXTURE_HIGH64) {
      struct iris_sampler_view *view = shs->textures[64 + i];
      uint32_t addr = view ? use_sampler_view(ice, batch, view)
                           : use_null_surface(batch, ice);
      push_bt_entry(addr);
   }

   foreach_surface_used(i, IRIS_SURFACE_GROUP_IMAGE) {
      uint32_t addr = use_image(batch, ice, shs, info, i);
      push_bt_entry(addr);
   }

   foreach_surface_used(i, IRIS_SURFACE_GROUP_UBO) {
      uint32_t addr = use_ubo_ssbo(batch, ice, &shs->constbuf[i],
                                   &shs->constbuf_surf_state[i], false,
                                   IRIS_DOMAIN_PULL_CONSTANT_READ);
      push_bt_entry(addr);
   }

   foreach_surface_used(i, IRIS_SURFACE_GROUP_SSBO) {
      uint32_t addr =
         use_ubo_ssbo(batch, ice, &shs->ssbo[i], &shs->ssbo_surf_state[i],
                      shs->writable_ssbos & (1u << i), IRIS_DOMAIN_NONE);
      push_bt_entry(addr);
   }

#if 0
      /* XXX: YUV surfaces not implemented yet */
      bt_assert(plane_start[1], ...);
      bt_assert(plane_start[2], ...);
#endif
}

static void
iris_use_optional_res(struct iris_batch *batch,
                      struct pipe_resource *res,
                      bool writeable,
                      enum iris_domain access)
{
   if (res) {
      struct iris_bo *bo = iris_resource_bo(res);
      iris_use_pinned_bo(batch, bo, writeable, access);
   }
}

static void
pin_depth_and_stencil_buffers(struct iris_batch *batch,
                              struct pipe_surface *zsbuf,
                              struct iris_depth_stencil_alpha_state *cso_zsa)
{
   if (!zsbuf)
      return;

   struct iris_resource *zres, *sres;
   iris_get_depth_stencil_resources(zsbuf->texture, &zres, &sres);

   if (zres) {
      iris_use_pinned_bo(batch, zres->bo, cso_zsa->depth_writes_enabled,
                         IRIS_DOMAIN_DEPTH_WRITE);
      if (zres->aux.bo) {
         iris_use_pinned_bo(batch, zres->aux.bo,
                            cso_zsa->depth_writes_enabled,
                            IRIS_DOMAIN_DEPTH_WRITE);
      }
   }

   if (sres) {
      iris_use_pinned_bo(batch, sres->bo, cso_zsa->stencil_writes_enabled,
                         IRIS_DOMAIN_DEPTH_WRITE);
   }
}

static uint32_t
pin_scratch_space(struct iris_context *ice,
                  struct iris_batch *batch,
                  const struct brw_stage_prog_data *prog_data,
                  gl_shader_stage stage)
{
   uint32_t scratch_addr = 0;

   if (prog_data->total_scratch > 0) {
      struct iris_bo *scratch_bo =
         iris_get_scratch_space(ice, prog_data->total_scratch, stage);
      iris_use_pinned_bo(batch, scratch_bo, true, IRIS_DOMAIN_NONE);

#if GFX_VERx10 >= 125
      const struct iris_state_ref *ref =
         iris_get_scratch_surf(ice, prog_data->total_scratch);
      iris_use_pinned_bo(batch, iris_resource_bo(ref->res),
                         false, IRIS_DOMAIN_NONE);
      scratch_addr = ref->offset +
                     iris_resource_bo(ref->res)->address -
                     IRIS_MEMZONE_SCRATCH_START;
      assert((scratch_addr & 0x3f) == 0 && scratch_addr < (1 << 26));
#else
      scratch_addr = scratch_bo->address;
#endif
   }

   return scratch_addr;
}

/* ------------------------------------------------------------------- */

/**
 * Pin any BOs which were installed by a previous batch, and restored
 * via the hardware logical context mechanism.
 *
 * We don't need to re-emit all state every batch - the hardware context
 * mechanism will save and restore it for us.  This includes pointers to
 * various BOs...which won't exist unless we ask the kernel to pin them
 * by adding them to the validation list.
 *
 * We can skip buffers if we've re-emitted those packets, as we're
 * overwriting those stale pointers with new ones, and don't actually
 * refer to the old BOs.
 */
static void
iris_restore_render_saved_bos(struct iris_context *ice,
                              struct iris_batch *batch,
                              const struct pipe_draw_info *draw)
{
   struct iris_genx_state *genx = ice->state.genx;

   const uint64_t clean = ~ice->state.dirty;
   const uint64_t stage_clean = ~ice->state.stage_dirty;

   if (clean & IRIS_DIRTY_CC_VIEWPORT) {
      iris_use_optional_res(batch, ice->state.last_res.cc_vp, false,
                            IRIS_DOMAIN_NONE);
   }

   if (clean & IRIS_DIRTY_SF_CL_VIEWPORT) {
      iris_use_optional_res(batch, ice->state.last_res.sf_cl_vp, false,
                            IRIS_DOMAIN_NONE);
   }

   if (clean & IRIS_DIRTY_BLEND_STATE) {
      iris_use_optional_res(batch, ice->state.last_res.blend, false,
                            IRIS_DOMAIN_NONE);
   }

   if (clean & IRIS_DIRTY_COLOR_CALC_STATE) {
      iris_use_optional_res(batch, ice->state.last_res.color_calc, false,
                            IRIS_DOMAIN_NONE);
   }

   if (clean & IRIS_DIRTY_SCISSOR_RECT) {
      iris_use_optional_res(batch, ice->state.last_res.scissor, false,
                            IRIS_DOMAIN_NONE);
   }

   if (ice->state.streamout_active && (clean & IRIS_DIRTY_SO_BUFFERS)) {
      for (int i = 0; i < 4; i++) {
         struct iris_stream_output_target *tgt =
            (void *) ice->state.so_target[i];
         if (tgt) {
            iris_use_pinned_bo(batch, iris_resource_bo(tgt->base.buffer),
                               true, IRIS_DOMAIN_OTHER_WRITE);
            iris_use_pinned_bo(batch, iris_resource_bo(tgt->offset.res),
                               true, IRIS_DOMAIN_OTHER_WRITE);
         }
      }
   }

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (!(stage_clean & (IRIS_STAGE_DIRTY_CONSTANTS_VS << stage)))
         continue;

      struct iris_shader_state *shs = &ice->state.shaders[stage];
      struct iris_compiled_shader *shader = ice->shaders.prog[stage];

      if (!shader)
         continue;

      struct brw_stage_prog_data *prog_data = (void *) shader->prog_data;

      for (int i = 0; i < 4; i++) {
         const struct brw_ubo_range *range = &prog_data->ubo_ranges[i];

         if (range->length == 0)
            continue;

         /* Range block is a binding table index, map back to UBO index. */
         unsigned block_index = iris_bti_to_group_index(
            &shader->bt, IRIS_SURFACE_GROUP_UBO, range->block);
         assert(block_index != IRIS_SURFACE_NOT_USED);

         struct pipe_shader_buffer *cbuf = &shs->constbuf[block_index];
         struct iris_resource *res = (void *) cbuf->buffer;

         if (res)
            iris_use_pinned_bo(batch, res->bo, false, IRIS_DOMAIN_OTHER_READ);
         else
            iris_use_pinned_bo(batch, batch->screen->workaround_bo, false,
                               IRIS_DOMAIN_OTHER_READ);
      }
   }

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (stage_clean & (IRIS_STAGE_DIRTY_BINDINGS_VS << stage)) {
         /* Re-pin any buffers referred to by the binding table. */
         iris_populate_binding_table(ice, batch, stage, true);
      }
   }

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      struct iris_shader_state *shs = &ice->state.shaders[stage];
      struct pipe_resource *res = shs->sampler_table.res;
      if (res)
         iris_use_pinned_bo(batch, iris_resource_bo(res), false,
                            IRIS_DOMAIN_NONE);
   }

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (stage_clean & (IRIS_STAGE_DIRTY_VS << stage)) {
         struct iris_compiled_shader *shader = ice->shaders.prog[stage];

         if (shader) {
            struct iris_bo *bo = iris_resource_bo(shader->assembly.res);
            iris_use_pinned_bo(batch, bo, false, IRIS_DOMAIN_NONE);

            pin_scratch_space(ice, batch, shader->prog_data, stage);
         }
      }
   }

   if ((clean & IRIS_DIRTY_DEPTH_BUFFER) &&
       (clean & IRIS_DIRTY_WM_DEPTH_STENCIL)) {
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      pin_depth_and_stencil_buffers(batch, cso_fb->zsbuf, ice->state.cso_zsa);
   }

   iris_use_optional_res(batch, ice->state.last_res.index_buffer, false,
                         IRIS_DOMAIN_VF_READ);

   if (clean & IRIS_DIRTY_VERTEX_BUFFERS) {
      uint64_t bound = ice->state.bound_vertex_buffers;
      while (bound) {
         const int i = u_bit_scan64(&bound);
         struct pipe_resource *res = genx->vertex_buffers[i].resource;
         iris_use_pinned_bo(batch, iris_resource_bo(res), false,
                            IRIS_DOMAIN_VF_READ);
      }
   }

#if GFX_VERx10 == 125
   iris_use_pinned_bo(batch, iris_resource_bo(ice->state.pixel_hashing_tables),
                      false, IRIS_DOMAIN_NONE);
#else
   assert(!ice->state.pixel_hashing_tables);
#endif
}

static void
iris_restore_compute_saved_bos(struct iris_context *ice,
                               struct iris_batch *batch,
                               const struct pipe_grid_info *grid)
{
   const uint64_t stage_clean = ~ice->state.stage_dirty;

   const int stage = MESA_SHADER_COMPUTE;
   struct iris_shader_state *shs = &ice->state.shaders[stage];

   if (stage_clean & IRIS_STAGE_DIRTY_BINDINGS_CS) {
      /* Re-pin any buffers referred to by the binding table. */
      iris_populate_binding_table(ice, batch, stage, true);
   }

   struct pipe_resource *sampler_res = shs->sampler_table.res;
   if (sampler_res)
      iris_use_pinned_bo(batch, iris_resource_bo(sampler_res), false,
                         IRIS_DOMAIN_NONE);

   if ((stage_clean & IRIS_STAGE_DIRTY_SAMPLER_STATES_CS) &&
       (stage_clean & IRIS_STAGE_DIRTY_BINDINGS_CS) &&
       (stage_clean & IRIS_STAGE_DIRTY_CONSTANTS_CS) &&
       (stage_clean & IRIS_STAGE_DIRTY_CS)) {
      iris_use_optional_res(batch, ice->state.last_res.cs_desc, false,
                            IRIS_DOMAIN_NONE);
   }

   if (stage_clean & IRIS_STAGE_DIRTY_CS) {
      struct iris_compiled_shader *shader = ice->shaders.prog[stage];

      if (shader) {
         struct iris_bo *bo = iris_resource_bo(shader->assembly.res);
         iris_use_pinned_bo(batch, bo, false, IRIS_DOMAIN_NONE);

         if (GFX_VERx10 < 125) {
            struct iris_bo *curbe_bo =
               iris_resource_bo(ice->state.last_res.cs_thread_ids);
            iris_use_pinned_bo(batch, curbe_bo, false, IRIS_DOMAIN_NONE);
         }

         pin_scratch_space(ice, batch, shader->prog_data, stage);
      }
   }
}

/**
 * Possibly emit STATE_BASE_ADDRESS to update Surface State Base Address.
 */
static void
iris_update_binder_address(struct iris_batch *batch,
                           struct iris_binder *binder)
{
   if (batch->last_binder_address == binder->bo->address)
      return;

   struct isl_device *isl_dev = &batch->screen->isl_dev;
   uint32_t mocs = isl_mocs(isl_dev, 0, false);

   iris_batch_sync_region_start(batch);

#if GFX_VER >= 11
   /* Use 3DSTATE_BINDING_TABLE_POOL_ALLOC on Icelake and later */

#if GFX_VERx10 == 120
   /* Wa_1607854226:
    *
    *  Workaround the non pipelined state not applying in MEDIA/GPGPU pipeline
    *  mode by putting the pipeline temporarily in 3D mode..
    */
   if (batch->name == IRIS_BATCH_COMPUTE)
      emit_pipeline_select(batch, _3D);
#endif

   iris_emit_pipe_control_flush(batch, "Stall for binder realloc",
                                PIPE_CONTROL_CS_STALL);

   iris_emit_cmd(batch, GENX(3DSTATE_BINDING_TABLE_POOL_ALLOC), btpa) {
      btpa.BindingTablePoolBaseAddress = ro_bo(binder->bo, 0);
      btpa.BindingTablePoolBufferSize = binder->size / 4096;
#if GFX_VERx10 < 125
      btpa.BindingTablePoolEnable = true;
#endif
      btpa.MOCS = mocs;
   }

#if GFX_VERx10 == 120
   /* Wa_1607854226:
    *
    *  Put the pipeline back into compute mode.
    */
   if (batch->name == IRIS_BATCH_COMPUTE)
      emit_pipeline_select(batch, GPGPU);
#endif
#else
   /* Use STATE_BASE_ADDRESS on older platforms */
   flush_before_state_base_change(batch);

   iris_emit_cmd(batch, GENX(STATE_BASE_ADDRESS), sba) {
      sba.SurfaceStateBaseAddressModifyEnable = true;
      sba.SurfaceStateBaseAddress = ro_bo(binder->bo, 0);

      /* The hardware appears to pay attention to the MOCS fields even
       * if you don't set the "Address Modify Enable" bit for the base.
       */
      sba.GeneralStateMOCS            = mocs;
      sba.StatelessDataPortAccessMOCS = mocs;
      sba.DynamicStateMOCS            = mocs;
      sba.IndirectObjectMOCS          = mocs;
      sba.InstructionMOCS             = mocs;
      sba.SurfaceStateMOCS            = mocs;
#if GFX_VER >= 9
      sba.BindlessSurfaceStateMOCS    = mocs;
#endif
#if GFX_VERx10 >= 125
      sba.L1CacheControl = L1CC_WB;
#endif
   }
#endif

   flush_after_state_base_change(batch);
   iris_batch_sync_region_end(batch);

   batch->last_binder_address = binder->bo->address;
}

static inline void
iris_viewport_zmin_zmax(const struct pipe_viewport_state *vp, bool halfz,
                        bool window_space_position, float *zmin, float *zmax)
{
   if (window_space_position) {
      *zmin = 0.f;
      *zmax = 1.f;
      return;
   }
   util_viewport_zmin_zmax(vp, halfz, zmin, zmax);
}

/* Wa_16018063123 */
static inline void
batch_emit_fast_color_dummy_blit(struct iris_batch *batch)
{
#if GFX_VERx10 >= 125
   iris_emit_cmd(batch, GENX(XY_FAST_COLOR_BLT), blt) {
      blt.DestinationBaseAddress = batch->screen->workaround_address;
      blt.DestinationMOCS = iris_mocs(batch->screen->workaround_address.bo,
                                      &batch->screen->isl_dev,
                                      ISL_SURF_USAGE_BLITTER_DST_BIT);
      blt.DestinationPitch = 63;
      blt.DestinationX2 = 1;
      blt.DestinationY2 = 4;
      blt.DestinationSurfaceWidth = 1;
      blt.DestinationSurfaceHeight = 4;
      blt.DestinationSurfaceType = XY_SURFTYPE_2D;
      blt.DestinationSurfaceQPitch = 4;
      blt.DestinationTiling = XY_TILE_LINEAR;
   }
#endif
}

#if GFX_VER >= 12
static void
invalidate_aux_map_state_per_engine(struct iris_batch *batch)
{
   uint64_t register_addr = 0;

   switch (batch->name) {
   case IRIS_BATCH_RENDER: {
      /* HSD 1209978178: docs say that before programming the aux table:
       *
       *    "Driver must ensure that the engine is IDLE but ensure it doesn't
       *    add extra flushes in the case it knows that the engine is already
       *    IDLE."
       *
       * An end of pipe sync is needed here, otherwise we see GPU hangs in
       * dEQP-GLES31.functional.copy_image.* tests.
       *
       * HSD 22012751911: SW Programming sequence when issuing aux invalidation:
       *
       *    "Render target Cache Flush + L3 Fabric Flush + State Invalidation + CS Stall"
       *
       * Notice we don't set the L3 Fabric Flush here, because we have
       * PIPE_CONTROL_CS_STALL. The PIPE_CONTROL::L3 Fabric Flush
       * documentation says :
       *
       *    "L3 Fabric Flush will ensure all the pending transactions in the
       *     L3 Fabric are flushed to global observation point. HW does
       *     implicit L3 Fabric Flush on all stalling flushes (both explicit
       *     and implicit) and on PIPECONTROL having Post Sync Operation
       *     enabled."
       *
       * Therefore setting L3 Fabric Flush here would be redundant.
       *
       * From Bspec 43904 (Register_CCSAuxiliaryTableInvalidate):
       * RCS engine idle sequence:
       *
       *    Gfx125+:
       *       PIPE_CONTROL:- DC Flush + L3 Fabric Flush + CS Stall + Render
       *                      Target Cache Flush + Depth Cache + CCS flush
       *
       */
      iris_emit_end_of_pipe_sync(batch, "Invalidate aux map table",
                                 PIPE_CONTROL_CS_STALL |
                                 PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                 PIPE_CONTROL_STATE_CACHE_INVALIDATE |
                                 (GFX_VERx10 == 125 ?
                                  PIPE_CONTROL_CCS_CACHE_FLUSH : 0));

      register_addr = GENX(GFX_CCS_AUX_INV_num);
      break;
   }
   case IRIS_BATCH_COMPUTE: {
      /*
       * Notice we don't set the L3 Fabric Flush here, because we have
       * PIPE_CONTROL_CS_STALL. The PIPE_CONTROL::L3 Fabric Flush
       * documentation says :
       *
       *    "L3 Fabric Flush will ensure all the pending transactions in the
       *     L3 Fabric are flushed to global observation point. HW does
       *     implicit L3 Fabric Flush on all stalling flushes (both explicit
       *     and implicit) and on PIPECONTROL having Post Sync Operation
       *     enabled."
       *
       * Therefore setting L3 Fabric Flush here would be redundant.
       *
       * From Bspec 43904 (Register_CCSAuxiliaryTableInvalidate):
       * Compute engine idle sequence:
       *
       *    Gfx125+:
       *       PIPE_CONTROL:- DC Flush + L3 Fabric Flush + CS Stall + CCS flush
       */
      iris_emit_end_of_pipe_sync(batch, "Invalidate aux map table",
                                 PIPE_CONTROL_DATA_CACHE_FLUSH |
                                 PIPE_CONTROL_CS_STALL |
                                 (GFX_VERx10 == 125 ?
                                  PIPE_CONTROL_CCS_CACHE_FLUSH : 0));

      register_addr = GENX(COMPCS0_CCS_AUX_INV_num);
      break;
   }
   case IRIS_BATCH_BLITTER: {
#if GFX_VERx10 >= 125
      /* Wa_16018063123 - emit fast color dummy blit before MI_FLUSH_DW. */
      if (intel_needs_workaround(batch->screen->devinfo, 16018063123))
         batch_emit_fast_color_dummy_blit(batch);

      /*
       * Notice we don't set the L3 Fabric Flush here, because we have
       * PIPE_CONTROL_CS_STALL. The PIPE_CONTROL::L3 Fabric Flush
       * documentation says :
       *
       *    "L3 Fabric Flush will ensure all the pending transactions in the
       *     L3 Fabric are flushed to global observation point. HW does
       *     implicit L3 Fabric Flush on all stalling flushes (both explicit
       *     and implicit) and on PIPECONTROL having Post Sync Operation
       *     enabled."
       *
       * Therefore setting L3 Fabric Flush here would be redundant.
       *
       * From Bspec 43904 (Register_CCSAuxiliaryTableInvalidate):
       * Blitter engine idle sequence:
       *
       *    Gfx125+:
       *       MI_FLUSH_DW (dw0;b16 – flush CCS)
       */
      iris_emit_cmd(batch, GENX(MI_FLUSH_DW), fd) {
         fd.FlushCCS = true;
      }
      register_addr = GENX(BCS_CCS_AUX_INV_num);
#endif
      break;
   }
   default:
      unreachable("Invalid batch for aux map invalidation");
      break;
   }

   if (register_addr != 0) {
      /* If the aux-map state number increased, then we need to rewrite the
       * register. Rewriting the register is used to both set the aux-map
       * translation table address, and also to invalidate any previously
       * cached translations.
       */
      iris_load_register_imm32(batch, register_addr, 1);

      /* HSD 22012751911: SW Programming sequence when issuing aux invalidation:
       *
       *    "Poll Aux Invalidation bit once the invalidation is set (Register
       *     4208 bit 0)"
       */
      iris_emit_cmd(batch, GENX(MI_SEMAPHORE_WAIT), sem) {
         sem.CompareOperation = COMPARE_SAD_EQUAL_SDD;
         sem.WaitMode = PollingMode;
         sem.RegisterPollMode = true;
         sem.SemaphoreDataDword = 0x0;
         sem.SemaphoreAddress = ro_bo(NULL, register_addr);
      }
   }
}

void
genX(invalidate_aux_map_state)(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   void *aux_map_ctx = iris_bufmgr_get_aux_map_context(screen->bufmgr);
   if (!aux_map_ctx)
      return;
   uint32_t aux_map_state_num = intel_aux_map_get_state_num(aux_map_ctx);
   if (batch->last_aux_map_state != aux_map_state_num) {
      invalidate_aux_map_state_per_engine(batch);
      batch->last_aux_map_state = aux_map_state_num;
   }
}

static void
init_aux_map_state(struct iris_batch *batch)
{
   struct iris_screen *screen = batch->screen;
   const struct intel_device_info *devinfo = screen->devinfo;
   void *aux_map_ctx = iris_bufmgr_get_aux_map_context(screen->bufmgr);
   if (!aux_map_ctx)
      return;

   uint64_t base_addr = intel_aux_map_get_base(aux_map_ctx);
   assert(base_addr != 0 && align64(base_addr, 32 * 1024) == base_addr);

   uint32_t reg = 0;
   switch (batch->name) {
   case IRIS_BATCH_COMPUTE:
      if (devinfo->has_compute_engine &&
          debug_get_bool_option("INTEL_COMPUTE_CLASS", false)) {
         reg = GENX(COMPCS0_AUX_TABLE_BASE_ADDR_num);
         break;
      }
      /* fallthrough */
      FALLTHROUGH;
   case IRIS_BATCH_RENDER:
      reg = GENX(GFX_AUX_TABLE_BASE_ADDR_num);
      break;
   case IRIS_BATCH_BLITTER:
#if GFX_VERx10 >= 125
      reg = GENX(BCS_AUX_TABLE_BASE_ADDR_num);
#endif
      break;
   default:
      unreachable("Invalid batch for aux map init.");
   }

   if (reg)
      iris_load_register_imm64(batch, reg, base_addr);
}
#endif

struct push_bos {
   struct {
      struct iris_address addr;
      uint32_t length;
   } buffers[4];
   int buffer_count;
   uint32_t max_length;
};

static void
setup_constant_buffers(struct iris_context *ice,
                       struct iris_batch *batch,
                       int stage,
                       struct push_bos *push_bos)
{
   struct iris_shader_state *shs = &ice->state.shaders[stage];
   struct iris_compiled_shader *shader = ice->shaders.prog[stage];
   struct brw_stage_prog_data *prog_data = (void *) shader->prog_data;

   uint32_t push_range_sum = 0;

   int n = 0;
   for (int i = 0; i < 4; i++) {
      const struct brw_ubo_range *range = &prog_data->ubo_ranges[i];

      if (range->length == 0)
         continue;

      push_range_sum += range->length;

      if (range->length > push_bos->max_length)
         push_bos->max_length = range->length;

      /* Range block is a binding table index, map back to UBO index. */
      unsigned block_index = iris_bti_to_group_index(
         &shader->bt, IRIS_SURFACE_GROUP_UBO, range->block);
      assert(block_index != IRIS_SURFACE_NOT_USED);

      struct pipe_shader_buffer *cbuf = &shs->constbuf[block_index];
      struct iris_resource *res = (void *) cbuf->buffer;

      assert(cbuf->buffer_offset % 32 == 0);

      if (res)
         iris_emit_buffer_barrier_for(batch, res->bo, IRIS_DOMAIN_OTHER_READ);

      push_bos->buffers[n].length = range->length;
      push_bos->buffers[n].addr =
         res ? ro_bo(res->bo, range->start * 32 + cbuf->buffer_offset)
         : batch->screen->workaround_address;
      n++;
   }

   /* From the 3DSTATE_CONSTANT_XS and 3DSTATE_CONSTANT_ALL programming notes:
    *
    *    "The sum of all four read length fields must be less than or
    *    equal to the size of 64."
    */
   assert(push_range_sum <= 64);

   push_bos->buffer_count = n;
}

static void
emit_push_constant_packets(struct iris_context *ice,
                           struct iris_batch *batch,
                           int stage,
                           const struct push_bos *push_bos)
{
   UNUSED struct isl_device *isl_dev = &batch->screen->isl_dev;
   struct iris_compiled_shader *shader = ice->shaders.prog[stage];
   struct brw_stage_prog_data *prog_data = (void *) shader->prog_data;

   iris_emit_cmd(batch, GENX(3DSTATE_CONSTANT_VS), pkt) {
      pkt._3DCommandSubOpcode = push_constant_opcodes[stage];

#if GFX_VER >= 9
      pkt.MOCS = isl_mocs(isl_dev, 0, false);
#endif

      if (prog_data) {
         /* The Skylake PRM contains the following restriction:
          *
          *    "The driver must ensure The following case does not occur
          *     without a flush to the 3D engine: 3DSTATE_CONSTANT_* with
          *     buffer 3 read length equal to zero committed followed by a
          *     3DSTATE_CONSTANT_* with buffer 0 read length not equal to
          *     zero committed."
          *
          * To avoid this, we program the buffers in the highest slots.
          * This way, slot 0 is only used if slot 3 is also used.
          */
         int n = push_bos->buffer_count;
         assert(n <= 4);
         const unsigned shift = 4 - n;
         for (int i = 0; i < n; i++) {
            pkt.ConstantBody.ReadLength[i + shift] =
               push_bos->buffers[i].length;
            pkt.ConstantBody.Buffer[i + shift] = push_bos->buffers[i].addr;
         }
      }
   }
}

#if GFX_VER >= 12
static void
emit_push_constant_packet_all(struct iris_context *ice,
                              struct iris_batch *batch,
                              uint32_t shader_mask,
                              const struct push_bos *push_bos)
{
   struct isl_device *isl_dev = &batch->screen->isl_dev;

   if (!push_bos) {
      iris_emit_cmd(batch, GENX(3DSTATE_CONSTANT_ALL), pc) {
         pc.ShaderUpdateEnable = shader_mask;
         pc.MOCS = iris_mocs(NULL, isl_dev, 0);
      }
      return;
   }

   const uint32_t n = push_bos->buffer_count;
   const uint32_t max_pointers = 4;
   const uint32_t num_dwords = 2 + 2 * n;
   uint32_t const_all[2 + 2 * max_pointers];
   uint32_t *dw = &const_all[0];

   assert(n <= max_pointers);
   iris_pack_command(GENX(3DSTATE_CONSTANT_ALL), dw, all) {
      all.DWordLength = num_dwords - 2;
      all.MOCS = isl_mocs(isl_dev, 0, false);
      all.ShaderUpdateEnable = shader_mask;
      all.PointerBufferMask = (1 << n) - 1;
   }
   dw += 2;

   for (int i = 0; i < n; i++) {
      _iris_pack_state(batch, GENX(3DSTATE_CONSTANT_ALL_DATA),
                       dw + i * 2, data) {
         data.PointerToConstantBuffer = push_bos->buffers[i].addr;
         data.ConstantBufferReadLength = push_bos->buffers[i].length;
      }
   }
   iris_batch_emit(batch, const_all, sizeof(uint32_t) * num_dwords);
}
#endif

void
genX(emit_depth_state_workarounds)(struct iris_context *ice,
                                   struct iris_batch *batch,
                                   const struct isl_surf *surf)
{
#if INTEL_NEEDS_WA_1808121037
   const bool is_d16_1x_msaa = surf->format == ISL_FORMAT_R16_UNORM &&
                               surf->samples == 1;

   switch (ice->state.genx->depth_reg_mode) {
   case IRIS_DEPTH_REG_MODE_HW_DEFAULT:
      if (!is_d16_1x_msaa)
         return;
      break;
   case IRIS_DEPTH_REG_MODE_D16_1X_MSAA:
      if (is_d16_1x_msaa)
         return;
      break;
   case IRIS_DEPTH_REG_MODE_UNKNOWN:
      break;
   }

   /* We'll change some CHICKEN registers depending on the depth surface
    * format. Do a depth flush and stall so the pipeline is not using these
    * settings while we change the registers.
    */
   iris_emit_end_of_pipe_sync(batch,
                              "Workaround: Stop pipeline for Wa_1808121037",
                              PIPE_CONTROL_DEPTH_STALL |
                              PIPE_CONTROL_DEPTH_CACHE_FLUSH);

   /* Wa_1808121037
    *
    * To avoid sporadic corruptions “Set 0x7010[9] when Depth Buffer
    * Surface Format is D16_UNORM , surface type is not NULL & 1X_MSAA”.
    */
   iris_emit_reg(batch, GENX(COMMON_SLICE_CHICKEN1), reg) {
      reg.HIZPlaneOptimizationdisablebit = is_d16_1x_msaa;
      reg.HIZPlaneOptimizationdisablebitMask = true;
   }

   ice->state.genx->depth_reg_mode =
      is_d16_1x_msaa ? IRIS_DEPTH_REG_MODE_D16_1X_MSAA :
                       IRIS_DEPTH_REG_MODE_HW_DEFAULT;
#endif
}

/* Calculate TBIMR tiling parameters adequate for the current pipeline
 * setup.  Return true if TBIMR should be enabled.
 */
UNUSED static bool
calculate_tile_dimensions(struct iris_context *ice,
                          unsigned *tile_width, unsigned *tile_height)
{
   struct iris_screen *screen = (void *)ice->ctx.screen;
   const struct intel_device_info *devinfo = screen->devinfo;

   /* Perform a rough calculation of the tile cache footprint of the
    * pixel pipeline, approximating it as the sum of the amount of
    * memory used per pixel by every render target, depth, stencil and
    * auxiliary surfaces bound to the pipeline.
    */
   unsigned pixel_size = 0;

   struct pipe_framebuffer_state *cso = &ice->state.framebuffer;

   if (cso->width == 0 || cso->height == 0)
      return false;

   for (unsigned i = 0; i < cso->nr_cbufs; i++) {
      const struct iris_surface *surf = (void *)cso->cbufs[i];

      if (surf) {
         const struct iris_resource *res = (void *)surf->base.texture;

         pixel_size += intel_calculate_surface_pixel_size(&res->surf);

         /* XXX - Pessimistic, in some cases it might be helpful to neglect
          *       aux surface traffic.
          */
         if (ice->state.draw_aux_usage[i]) {
            pixel_size += intel_calculate_surface_pixel_size(&res->aux.surf);
            pixel_size += intel_calculate_surface_pixel_size(&res->aux.extra_aux.surf);
         }
      }
   }

   if (cso->zsbuf) {
      struct iris_resource *zres;
      struct iris_resource *sres;
      iris_get_depth_stencil_resources(cso->zsbuf->texture, &zres, &sres);

      if (zres) {
         pixel_size += intel_calculate_surface_pixel_size(&zres->surf);

         /* XXX - Pessimistic, in some cases it might be helpful to neglect
          *       aux surface traffic.
          */
         if (iris_resource_level_has_hiz(devinfo, zres, cso->zsbuf->u.tex.level)) {
            pixel_size += intel_calculate_surface_pixel_size(&zres->aux.surf);
            pixel_size += intel_calculate_surface_pixel_size(&zres->aux.extra_aux.surf);
         }
      }

      if (sres) {
         pixel_size += intel_calculate_surface_pixel_size(&sres->surf);
      }
   }

   /* Compute a tile layout that allows reasonable utilization of the
    * tile cache based on the per-pixel cache footprint estimated
    * above.
    */
   intel_calculate_tile_dimensions(devinfo, screen->l3_config_3d,
                                   32, 32, cso->width, cso->height, pixel_size,
                                   tile_width, tile_height);

   /* Perform TBIMR tile passes only if the framebuffer covers more
    * than a single tile.
    */
   return *tile_width < cso->width || *tile_height < cso->height;
}

static void
iris_preemption_streamout_wa(struct iris_context *ice,
                             struct iris_batch *batch,
                             bool enable)
{
#if GFX_VERx10 >= 120
   if (!intel_needs_workaround(batch->screen->devinfo, 16013994831))
      return;

   iris_emit_reg(batch, GENX(CS_CHICKEN1), reg) {
      reg.DisablePreemptionandHighPriorityPausingdueto3DPRIMITIVECommand = !enable;
      reg.DisablePreemptionandHighPriorityPausingdueto3DPRIMITIVECommandMask = true;
   }

   /* Emit CS_STALL and 250 noops. */
   iris_emit_pipe_control_flush(batch, "workaround: Wa_16013994831",
                                PIPE_CONTROL_CS_STALL);
   for (unsigned i = 0; i < 250; i++)
      iris_emit_cmd(batch, GENX(MI_NOOP), noop);

   ice->state.genx->object_preemption = enable;
#endif
}

static void
shader_program_needs_wa_14015055625(struct iris_context *ice,
                                    struct iris_batch *batch,
                                    const struct brw_stage_prog_data *prog_data,
                                    gl_shader_stage stage,
                                    bool *program_needs_wa_14015055625)
{
   if (!intel_needs_workaround(batch->screen->devinfo, 14015055625))
      return;

   switch (stage) {
   case MESA_SHADER_TESS_CTRL: {
      struct brw_tcs_prog_data *tcs_prog_data = (void *) prog_data;
      *program_needs_wa_14015055625 |= tcs_prog_data->include_primitive_id;
      break;
   }
   case MESA_SHADER_TESS_EVAL: {
      struct brw_tes_prog_data *tes_prog_data = (void *) prog_data;
      *program_needs_wa_14015055625 |= tes_prog_data->include_primitive_id;
      break;
   }
   default:
      break;
   }

   struct iris_compiled_shader *gs_shader =
      ice->shaders.prog[MESA_SHADER_GEOMETRY];
   const struct brw_gs_prog_data *gs_prog_data =
      gs_shader ? (void *) gs_shader->prog_data : NULL;

   *program_needs_wa_14015055625 |=
      gs_prog_data && gs_prog_data->include_primitive_id;
}

static void
emit_wa_18020335297_dummy_draw(struct iris_batch *batch)
{
#if GFX_VERx10 >= 125
   iris_emit_cmd(batch, GENX(3DSTATE_VFG), vfg) {
      vfg.DistributionMode = RR_STRICT;
   }
   iris_emit_cmd(batch, GENX(3DSTATE_VF), vf) {
      vf.GeometryDistributionEnable = true;
   }
#endif

#if GFX_VER >= 12
   iris_emit_cmd(batch, GENX(3DSTATE_PRIMITIVE_REPLICATION), pr) {
      pr.ReplicaMask = 1;
   }
#endif

   iris_emit_cmd(batch, GENX(3DSTATE_RASTER), rr) {
      rr.CullMode = CULLMODE_NONE;
      rr.FrontFaceFillMode = FILL_MODE_SOLID;
      rr.BackFaceFillMode = FILL_MODE_SOLID;
   }

   iris_emit_cmd(batch, GENX(3DSTATE_VF_STATISTICS), vf) { }
   iris_emit_cmd(batch, GENX(3DSTATE_VF_SGVS), sgvs) { }

#if GFX_VER >= 11
   iris_emit_cmd(batch, GENX(3DSTATE_VF_SGVS_2), sgvs2) { }
#endif

   iris_emit_cmd(batch, GENX(3DSTATE_CLIP), clip) {
      clip.ClipEnable = true;
      clip.ClipMode = CLIPMODE_REJECT_ALL;
   }

   iris_emit_cmd(batch, GENX(3DSTATE_VS), vs) { }
   iris_emit_cmd(batch, GENX(3DSTATE_GS), gs) { }
   iris_emit_cmd(batch, GENX(3DSTATE_HS), hs) { }
   iris_emit_cmd(batch, GENX(3DSTATE_TE), te) { }
   iris_emit_cmd(batch, GENX(3DSTATE_DS), ds) { }
   iris_emit_cmd(batch, GENX(3DSTATE_STREAMOUT), so) { }

   uint32_t vertex_elements[1 + 2 * GENX(VERTEX_ELEMENT_STATE_length)];
   uint32_t *ve_pack_dest = &vertex_elements[1];

   iris_pack_command(GENX(3DSTATE_VERTEX_ELEMENTS), vertex_elements, ve) {
      ve.DWordLength = 1 + GENX(VERTEX_ELEMENT_STATE_length) * 2 -
                       GENX(3DSTATE_VERTEX_ELEMENTS_length_bias);
   }

   for (int i = 0; i < 2; i++) {
      iris_pack_state(GENX(VERTEX_ELEMENT_STATE), ve_pack_dest, ve) {
         ve.Valid = true;
         ve.SourceElementFormat = ISL_FORMAT_R32G32B32A32_FLOAT;
         ve.Component0Control = VFCOMP_STORE_0;
         ve.Component1Control = VFCOMP_STORE_0;
         ve.Component2Control = i == 0 ? VFCOMP_STORE_0 : VFCOMP_STORE_1_FP;
         ve.Component3Control = i == 0 ? VFCOMP_STORE_0 : VFCOMP_STORE_1_FP;
      }
      ve_pack_dest += GENX(VERTEX_ELEMENT_STATE_length);
   }

   iris_batch_emit(batch, vertex_elements, sizeof(uint32_t) *
                   (1 + 2 * GENX(VERTEX_ELEMENT_STATE_length)));

   iris_emit_cmd(batch, GENX(3DSTATE_VF_TOPOLOGY), topo) {
      topo.PrimitiveTopologyType = _3DPRIM_TRILIST;
   }

   /* Emit dummy draw per slice. */
   for (unsigned i = 0; i < batch->screen->devinfo->num_slices; i++) {
      iris_emit_cmd(batch, GENX(3DPRIMITIVE), prim) {
         prim.VertexCountPerInstance = 3;
         prim.PrimitiveTopologyType = _3DPRIM_TRILIST;
         prim.InstanceCount = 1;
         prim.VertexAccessType = SEQUENTIAL;
      }
   }
}

static void
iris_upload_dirty_render_state(struct iris_context *ice,
                               struct iris_batch *batch,
                               const struct pipe_draw_info *draw)
{
   struct iris_screen *screen = batch->screen;
   struct iris_border_color_pool *border_color_pool =
      iris_bufmgr_get_border_color_pool(screen->bufmgr);

   /* Re-emit 3DSTATE_DS before any 3DPRIMITIVE when tessellation is on */
   if (intel_needs_workaround(batch->screen->devinfo, 22018402687) &&
       ice->shaders.prog[MESA_SHADER_TESS_EVAL])
      ice->state.stage_dirty |= IRIS_STAGE_DIRTY_TES;

   uint64_t dirty = ice->state.dirty;
   uint64_t stage_dirty = ice->state.stage_dirty;

   if (!(dirty & IRIS_ALL_DIRTY_FOR_RENDER) &&
       !(stage_dirty & IRIS_ALL_STAGE_DIRTY_FOR_RENDER))
      return;

   struct iris_genx_state *genx = ice->state.genx;
   struct iris_binder *binder = &ice->state.binder;
   struct brw_wm_prog_data *wm_prog_data = (void *)
      ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data;

   /* When MSAA is enabled, instead of using BLENDFACTOR_ZERO use
    * CONST_COLOR, CONST_ALPHA and supply zero by using blend constants.
    */
   bool needs_wa_14018912822 =
      screen->driconf.intel_enable_wa_14018912822 &&
      intel_needs_workaround(batch->screen->devinfo, 14018912822) &&
      util_framebuffer_get_num_samples(&ice->state.framebuffer) > 1;

   if (dirty & IRIS_DIRTY_CC_VIEWPORT) {
      const struct iris_rasterizer_state *cso_rast = ice->state.cso_rast;
      uint32_t cc_vp_address;
      bool wa_18020335297_applied = false;

      /* Wa_18020335297 - Apply the WA when viewport ptr is reprogrammed. */
      if (intel_needs_workaround(screen->devinfo, 18020335297) &&
          batch->name == IRIS_BATCH_RENDER &&
          ice->state.viewport_ptr_set) {
         emit_wa_18020335297_dummy_draw(batch);
         wa_18020335297_applied = true;
      }

      /* XXX: could avoid streaming for depth_clip [0,1] case. */
      uint32_t *cc_vp_map =
         stream_state(batch, ice->state.dynamic_uploader,
                      &ice->state.last_res.cc_vp,
                      4 * ice->state.num_viewports *
                      GENX(CC_VIEWPORT_length), 32, &cc_vp_address);
      for (int i = 0; i < ice->state.num_viewports; i++) {
         float zmin, zmax;
         iris_viewport_zmin_zmax(&ice->state.viewports[i], cso_rast->clip_halfz,
                                 ice->state.window_space_position,
                                 &zmin, &zmax);
         if (cso_rast->depth_clip_near)
            zmin = 0.0;
         if (cso_rast->depth_clip_far)
            zmax = 1.0;

         iris_pack_state(GENX(CC_VIEWPORT), cc_vp_map, ccv) {
            ccv.MinimumDepth = zmin;
            ccv.MaximumDepth = zmax;
         }

         cc_vp_map += GENX(CC_VIEWPORT_length);
      }

      iris_emit_cmd(batch, GENX(3DSTATE_VIEWPORT_STATE_POINTERS_CC), ptr) {
         ptr.CCViewportPointer = cc_vp_address;
      }

      if (wa_18020335297_applied) {
#if GFX_VER >= 12
         iris_emit_cmd(batch, GENX(3DSTATE_PRIMITIVE_REPLICATION), pr) { }
#endif
         /* Dirty all emitted WA state to make sure that current real
          * state is restored.
          */
         dirty |= IRIS_DIRTY_VFG |
                  IRIS_DIRTY_VF |
                  IRIS_DIRTY_RASTER |
                  IRIS_DIRTY_VF_STATISTICS |
                  IRIS_DIRTY_VF_SGVS |
                  IRIS_DIRTY_CLIP |
                  IRIS_DIRTY_STREAMOUT |
                  IRIS_DIRTY_VERTEX_ELEMENTS |
                  IRIS_DIRTY_VF_TOPOLOGY;

         for (int stage = 0; stage < MESA_SHADER_FRAGMENT; stage++) {
            if (ice->shaders.prog[stage])
               stage_dirty |= (IRIS_STAGE_DIRTY_VS << stage);
         }
      }
      ice->state.viewport_ptr_set = true;
   }

   if (dirty & IRIS_DIRTY_SF_CL_VIEWPORT) {
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      uint32_t sf_cl_vp_address;
      uint32_t *vp_map =
         stream_state(batch, ice->state.dynamic_uploader,
                      &ice->state.last_res.sf_cl_vp,
                      4 * ice->state.num_viewports *
                      GENX(SF_CLIP_VIEWPORT_length), 64, &sf_cl_vp_address);

      for (unsigned i = 0; i < ice->state.num_viewports; i++) {
         const struct pipe_viewport_state *state = &ice->state.viewports[i];
         float gb_xmin, gb_xmax, gb_ymin, gb_ymax;

         float vp_xmin = viewport_extent(state, 0, -1.0f);
         float vp_xmax = viewport_extent(state, 0,  1.0f);
         float vp_ymin = viewport_extent(state, 1, -1.0f);
         float vp_ymax = viewport_extent(state, 1,  1.0f);

         intel_calculate_guardband_size(0, cso_fb->width, 0, cso_fb->height,
                                        state->scale[0], state->scale[1],
                                        state->translate[0], state->translate[1],
                                        &gb_xmin, &gb_xmax, &gb_ymin, &gb_ymax);

         iris_pack_state(GENX(SF_CLIP_VIEWPORT), vp_map, vp) {
            vp.ViewportMatrixElementm00 = state->scale[0];
            vp.ViewportMatrixElementm11 = state->scale[1];
            vp.ViewportMatrixElementm22 = state->scale[2];
            vp.ViewportMatrixElementm30 = state->translate[0];
            vp.ViewportMatrixElementm31 = state->translate[1];
            vp.ViewportMatrixElementm32 = state->translate[2];
            vp.XMinClipGuardband = gb_xmin;
            vp.XMaxClipGuardband = gb_xmax;
            vp.YMinClipGuardband = gb_ymin;
            vp.YMaxClipGuardband = gb_ymax;
            vp.XMinViewPort = MAX2(vp_xmin, 0);
            vp.XMaxViewPort = MIN2(vp_xmax, cso_fb->width) - 1;
            vp.YMinViewPort = MAX2(vp_ymin, 0);
            vp.YMaxViewPort = MIN2(vp_ymax, cso_fb->height) - 1;
         }

         vp_map += GENX(SF_CLIP_VIEWPORT_length);
      }

      iris_emit_cmd(batch, GENX(3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP), ptr) {
         ptr.SFClipViewportPointer = sf_cl_vp_address;
      }
   }

   if (dirty & IRIS_DIRTY_URB) {
      for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
         if (!ice->shaders.prog[i]) {
            ice->shaders.urb.size[i] = 1;
         } else {
            struct brw_vue_prog_data *vue_prog_data =
               (void *) ice->shaders.prog[i]->prog_data;
            ice->shaders.urb.size[i] = vue_prog_data->urb_entry_size;
         }
         assert(ice->shaders.urb.size[i] != 0);
      }

      intel_get_urb_config(screen->devinfo,
                           screen->l3_config_3d,
                           ice->shaders.prog[MESA_SHADER_TESS_EVAL] != NULL,
                           ice->shaders.prog[MESA_SHADER_GEOMETRY] != NULL,
                           ice->shaders.urb.size,
                           ice->shaders.urb.entries,
                           ice->shaders.urb.start,
                           &ice->state.urb_deref_block_size,
                           &ice->shaders.urb.constrained);

      for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
         iris_emit_cmd(batch, GENX(3DSTATE_URB_VS), urb) {
            urb._3DCommandSubOpcode += i;
            urb.VSURBStartingAddress     = ice->shaders.urb.start[i];
            urb.VSURBEntryAllocationSize = ice->shaders.urb.size[i] - 1;
            urb.VSNumberofURBEntries     = ice->shaders.urb.entries[i];
         }
      }
   }

   if (dirty & IRIS_DIRTY_BLEND_STATE) {
      struct iris_blend_state *cso_blend = ice->state.cso_blend;
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      struct iris_depth_stencil_alpha_state *cso_zsa = ice->state.cso_zsa;
      const int header_dwords = GENX(BLEND_STATE_length);

      bool color_blend_zero = false;
      bool alpha_blend_zero = false;

      /* Always write at least one BLEND_STATE - the final RT message will
       * reference BLEND_STATE[0] even if there aren't color writes.  There
       * may still be alpha testing, computed depth, and so on.
       */
      const int rt_dwords =
         MAX2(cso_fb->nr_cbufs, 1) * GENX(BLEND_STATE_ENTRY_length);

      uint32_t blend_offset;
      uint32_t *blend_map =
         stream_state(batch, ice->state.dynamic_uploader,
                      &ice->state.last_res.blend,
                      4 * (header_dwords + rt_dwords), 64, &blend_offset);

      /* Copy of blend entries for merging dynamic changes. */
      uint32_t blend_entries[4 * rt_dwords];
      memcpy(blend_entries, &cso_blend->blend_state[1], sizeof(blend_entries));

      unsigned cbufs = MAX2(cso_fb->nr_cbufs, 1);

      uint32_t *blend_entry = blend_entries;
      for (unsigned i = 0; i < cbufs; i++) {
         int dst_blend_factor = cso_blend->ps_dst_blend_factor[i];
         int dst_alpha_blend_factor = cso_blend->ps_dst_alpha_blend_factor[i];
         uint32_t entry[GENX(BLEND_STATE_ENTRY_length)];
         iris_pack_state(GENX(BLEND_STATE_ENTRY), entry, be) {
            if (needs_wa_14018912822) {
               if (dst_blend_factor == BLENDFACTOR_ZERO) {
                  dst_blend_factor = BLENDFACTOR_CONST_COLOR;
                  color_blend_zero = true;
               }
               if (dst_alpha_blend_factor == BLENDFACTOR_ZERO) {
                  dst_alpha_blend_factor = BLENDFACTOR_CONST_ALPHA;
                  alpha_blend_zero = true;
               }
            }
            be.DestinationBlendFactor = dst_blend_factor;
            be.DestinationAlphaBlendFactor = dst_alpha_blend_factor;
         }

         /* Merge entry. */
         uint32_t *dst = blend_entry;
         uint32_t *src = entry;
         for (unsigned j = 0; j < GENX(BLEND_STATE_ENTRY_length); j++)
            *dst |= *src;

         blend_entry += GENX(BLEND_STATE_ENTRY_length);
      }

      /* Blend constants modified for Wa_14018912822. */
      if (ice->state.color_blend_zero != color_blend_zero) {
         ice->state.color_blend_zero = color_blend_zero;
         ice->state.dirty |= IRIS_DIRTY_COLOR_CALC_STATE;
      }
      if (ice->state.alpha_blend_zero != alpha_blend_zero) {
         ice->state.alpha_blend_zero = alpha_blend_zero;
         ice->state.dirty |= IRIS_DIRTY_COLOR_CALC_STATE;
      }

      uint32_t blend_state_header;
      iris_pack_state(GENX(BLEND_STATE), &blend_state_header, bs) {
         bs.AlphaTestEnable = cso_zsa->alpha_enabled;
         bs.AlphaTestFunction = translate_compare_func(cso_zsa->alpha_func);
      }

      blend_map[0] = blend_state_header | cso_blend->blend_state[0];
      memcpy(&blend_map[1], blend_entries, 4 * rt_dwords);

      iris_emit_cmd(batch, GENX(3DSTATE_BLEND_STATE_POINTERS), ptr) {
         ptr.BlendStatePointer = blend_offset;
         ptr.BlendStatePointerValid = true;
      }
   }

   if (dirty & IRIS_DIRTY_COLOR_CALC_STATE) {
      struct iris_depth_stencil_alpha_state *cso = ice->state.cso_zsa;
#if GFX_VER == 8
      struct pipe_stencil_ref *p_stencil_refs = &ice->state.stencil_ref;
#endif
      uint32_t cc_offset;
      void *cc_map =
         stream_state(batch, ice->state.dynamic_uploader,
                      &ice->state.last_res.color_calc,
                      sizeof(uint32_t) * GENX(COLOR_CALC_STATE_length),
                      64, &cc_offset);
      iris_pack_state(GENX(COLOR_CALC_STATE), cc_map, cc) {
         cc.AlphaTestFormat = ALPHATEST_FLOAT32;
         cc.AlphaReferenceValueAsFLOAT32 = cso->alpha_ref_value;
         cc.BlendConstantColorRed   = ice->state.color_blend_zero ?
            0.0 : ice->state.blend_color.color[0];
         cc.BlendConstantColorGreen = ice->state.color_blend_zero ?
            0.0 : ice->state.blend_color.color[1];
         cc.BlendConstantColorBlue  = ice->state.color_blend_zero ?
            0.0 : ice->state.blend_color.color[2];
         cc.BlendConstantColorAlpha = ice->state.alpha_blend_zero ?
            0.0 : ice->state.blend_color.color[3];
#if GFX_VER == 8
	 cc.StencilReferenceValue = p_stencil_refs->ref_value[0];
	 cc.BackfaceStencilReferenceValue = p_stencil_refs->ref_value[1];
#endif
      }
      iris_emit_cmd(batch, GENX(3DSTATE_CC_STATE_POINTERS), ptr) {
         ptr.ColorCalcStatePointer = cc_offset;
         ptr.ColorCalcStatePointerValid = true;
      }
   }

#if GFX_VERx10 == 125
   if (dirty & (IRIS_DIRTY_RENDER_BUFFER | IRIS_DIRTY_DEPTH_BUFFER)) {
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      unsigned tile_width, tile_height;

      ice->state.use_tbimr = batch->screen->driconf.enable_tbimr &&
         calculate_tile_dimensions(ice, &tile_width, &tile_height);

      if (ice->state.use_tbimr) {
         /* Use a batch size of 128 polygons per slice as recommended
          * by BSpec 68436 "TBIMR Programming".
          */
         const unsigned num_slices = screen->devinfo->num_slices;
         const unsigned batch_size = DIV_ROUND_UP(num_slices, 2) * 256;

         iris_emit_cmd(batch, GENX(3DSTATE_TBIMR_TILE_PASS_INFO), tbimr) {
            tbimr.TileRectangleHeight = tile_height;
            tbimr.TileRectangleWidth = tile_width;
            tbimr.VerticalTileCount = DIV_ROUND_UP(cso_fb->height, tile_height);
            tbimr.HorizontalTileCount = DIV_ROUND_UP(cso_fb->width, tile_width);
            tbimr.TBIMRBatchSize = util_logbase2(batch_size) - 5;
            tbimr.TileBoxCheck = true;
         }
      }
   }
#endif

   /* Wa_1604061319
    *
    *    3DSTATE_CONSTANT_* needs to be programmed before BTP_*
    *
    * Testing shows that all the 3DSTATE_CONSTANT_XS need to be emitted if
    * any stage has a dirty binding table.
    */
   const bool emit_const_wa = GFX_VER >= 11 &&
      ((dirty & IRIS_DIRTY_RENDER_BUFFER) ||
       (stage_dirty & IRIS_ALL_STAGE_DIRTY_BINDINGS_FOR_RENDER));

#if GFX_VER >= 12
   uint32_t nobuffer_stages = 0;
#endif

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (!(stage_dirty & (IRIS_STAGE_DIRTY_CONSTANTS_VS << stage)) &&
          !emit_const_wa)
         continue;

      struct iris_shader_state *shs = &ice->state.shaders[stage];
      struct iris_compiled_shader *shader = ice->shaders.prog[stage];

      if (!shader)
         continue;

      if (shs->sysvals_need_upload)
         upload_sysvals(ice, stage, NULL);

      struct push_bos push_bos = {};
      setup_constant_buffers(ice, batch, stage, &push_bos);

#if GFX_VER >= 12
      /* If this stage doesn't have any push constants, emit it later in a
       * single CONSTANT_ALL packet with all the other stages.
       */
      if (push_bos.buffer_count == 0) {
         nobuffer_stages |= 1 << stage;
         continue;
      }

      /* The Constant Buffer Read Length field from 3DSTATE_CONSTANT_ALL
       * contains only 5 bits, so we can only use it for buffers smaller than
       * 32.
       *
       * According to Wa_16011448509, Gfx12.0 misinterprets some address bits
       * in 3DSTATE_CONSTANT_ALL.  It should still be safe to use the command
       * for disabling stages, where all address bits are zero.  However, we
       * can't safely use it for general buffers with arbitrary addresses.
       * Just fall back to the individual 3DSTATE_CONSTANT_XS commands in that
       * case.
       */
      if (push_bos.max_length < 32 && GFX_VERx10 > 120) {
         emit_push_constant_packet_all(ice, batch, 1 << stage, &push_bos);
         continue;
      }
#endif
      emit_push_constant_packets(ice, batch, stage, &push_bos);
   }

#if GFX_VER >= 12
   if (nobuffer_stages)
      /* Wa_16011448509: all address bits are zero */
      emit_push_constant_packet_all(ice, batch, nobuffer_stages, NULL);
#endif

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      /* Gfx9 requires 3DSTATE_BINDING_TABLE_POINTERS_XS to be re-emitted
       * in order to commit constants.  TODO: Investigate "Disable Gather
       * at Set Shader" to go back to legacy mode...
       */
      if (stage_dirty & ((IRIS_STAGE_DIRTY_BINDINGS_VS |
                          (GFX_VER == 9 ? IRIS_STAGE_DIRTY_CONSTANTS_VS : 0))
                            << stage)) {
         iris_emit_cmd(batch, GENX(3DSTATE_BINDING_TABLE_POINTERS_VS), ptr) {
            ptr._3DCommandSubOpcode = 38 + stage;
            ptr.PointertoVSBindingTable =
               binder->bt_offset[stage] >> IRIS_BT_OFFSET_SHIFT;
         }
      }
   }

   if (GFX_VER >= 11 && (dirty & IRIS_DIRTY_RENDER_BUFFER)) {
      // XXX: we may want to flag IRIS_DIRTY_MULTISAMPLE (or SAMPLE_MASK?)
      // XXX: see commit 979fc1bc9bcc64027ff2cfafd285676f31b930a6

      /* The PIPE_CONTROL command description says:
       *
       *   "Whenever a Binding Table Index (BTI) used by a Render Target
       *    Message points to a different RENDER_SURFACE_STATE, SW must issue a
       *    Render Target Cache Flush by enabling this bit. When render target
       *    flush is set due to new association of BTI, PS Scoreboard Stall bit
       *    must be set in this packet."
       */
      // XXX: does this need to happen at 3DSTATE_BTP_PS time?
      iris_emit_pipe_control_flush(batch, "workaround: RT BTI change [draw]",
                                   PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                   PIPE_CONTROL_STALL_AT_SCOREBOARD);
   }

   if (dirty & IRIS_DIRTY_RENDER_BUFFER)
      trace_framebuffer_state(&batch->trace, NULL, &ice->state.framebuffer);

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (stage_dirty & (IRIS_STAGE_DIRTY_BINDINGS_VS << stage)) {
         iris_populate_binding_table(ice, batch, stage, false);
      }
   }

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (!(stage_dirty & (IRIS_STAGE_DIRTY_SAMPLER_STATES_VS << stage)) ||
          !ice->shaders.prog[stage])
         continue;

      iris_upload_sampler_states(ice, stage);

      struct iris_shader_state *shs = &ice->state.shaders[stage];
      struct pipe_resource *res = shs->sampler_table.res;
      if (res)
         iris_use_pinned_bo(batch, iris_resource_bo(res), false,
                            IRIS_DOMAIN_NONE);

      iris_emit_cmd(batch, GENX(3DSTATE_SAMPLER_STATE_POINTERS_VS), ptr) {
         ptr._3DCommandSubOpcode = 43 + stage;
         ptr.PointertoVSSamplerState = shs->sampler_table.offset;
      }
   }

   if (ice->state.need_border_colors)
      iris_use_pinned_bo(batch, border_color_pool->bo, false, IRIS_DOMAIN_NONE);

   if (dirty & IRIS_DIRTY_MULTISAMPLE) {
      iris_emit_cmd(batch, GENX(3DSTATE_MULTISAMPLE), ms) {
         ms.PixelLocation =
            ice->state.cso_rast->half_pixel_center ? CENTER : UL_CORNER;
         if (ice->state.framebuffer.samples > 0)
            ms.NumberofMultisamples = ffs(ice->state.framebuffer.samples) - 1;
      }
   }

   if (dirty & IRIS_DIRTY_SAMPLE_MASK) {
      iris_emit_cmd(batch, GENX(3DSTATE_SAMPLE_MASK), ms) {
         ms.SampleMask = ice->state.sample_mask;
      }
   }

#if GFX_VERx10 >= 125
   /* This is only used on >= gfx125 for dynamic 3DSTATE_TE emission
    * related workarounds.
    */
   bool program_needs_wa_14015055625 = false;
#endif

#if INTEL_WA_14015055625_GFX_VER
   /* Check if FS stage will use primitive ID overrides for Wa_14015055625. */
   const struct brw_vue_map *last_vue_map =
      &brw_vue_prog_data(ice->shaders.last_vue_shader->prog_data)->vue_map;
   if ((wm_prog_data->inputs & VARYING_BIT_PRIMITIVE_ID) &&
       last_vue_map->varying_to_slot[VARYING_SLOT_PRIMITIVE_ID] == -1 &&
       intel_needs_workaround(batch->screen->devinfo, 14015055625)) {
      program_needs_wa_14015055625 = true;
   }
#endif

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (!(stage_dirty & (IRIS_STAGE_DIRTY_VS << stage)))
         continue;

      struct iris_compiled_shader *shader = ice->shaders.prog[stage];

      if (shader) {
         struct brw_stage_prog_data *prog_data = shader->prog_data;
         struct iris_resource *cache = (void *) shader->assembly.res;
         iris_use_pinned_bo(batch, cache->bo, false, IRIS_DOMAIN_NONE);

         uint32_t scratch_addr =
            pin_scratch_space(ice, batch, prog_data, stage);

#if INTEL_WA_14015055625_GFX_VER
         shader_program_needs_wa_14015055625(ice, batch, prog_data, stage,
                                             &program_needs_wa_14015055625);
#endif

         if (stage == MESA_SHADER_FRAGMENT) {
            UNUSED struct iris_rasterizer_state *cso = ice->state.cso_rast;
            struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;

            uint32_t ps_state[GENX(3DSTATE_PS_length)] = {0};
            _iris_pack_command(batch, GENX(3DSTATE_PS), ps_state, ps) {
               intel_set_ps_dispatch_state(&ps, batch->screen->devinfo,
                                           wm_prog_data, util_framebuffer_get_num_samples(cso_fb),
                                           0 /* msaa_flags */);

#if GFX_VER == 12
               assert(wm_prog_data->dispatch_multi == 0 ||
                      (wm_prog_data->dispatch_multi == 16 && wm_prog_data->max_polygons == 2));
               ps.DualSIMD8DispatchEnable = wm_prog_data->dispatch_multi;
               /* XXX - No major improvement observed from enabling
                *       overlapping subspans, but it could be helpful
                *       in theory when the requirements listed on the
                *       BSpec page for 3DSTATE_PS_BODY are met.
                */
               ps.OverlappingSubspansEnable = false;
#endif

               ps.DispatchGRFStartRegisterForConstantSetupData0 =
                  brw_wm_prog_data_dispatch_grf_start_reg(wm_prog_data, ps, 0);
               ps.DispatchGRFStartRegisterForConstantSetupData1 =
                  brw_wm_prog_data_dispatch_grf_start_reg(wm_prog_data, ps, 1);
#if GFX_VER < 20
               ps.DispatchGRFStartRegisterForConstantSetupData2 =
                  brw_wm_prog_data_dispatch_grf_start_reg(wm_prog_data, ps, 2);
#endif

               ps.KernelStartPointer0 = KSP(shader) +
                  brw_wm_prog_data_prog_offset(wm_prog_data, ps, 0);
               ps.KernelStartPointer1 = KSP(shader) +
                  brw_wm_prog_data_prog_offset(wm_prog_data, ps, 1);
#if GFX_VER < 20
               ps.KernelStartPointer2 = KSP(shader) +
                  brw_wm_prog_data_prog_offset(wm_prog_data, ps, 2);
#endif

#if GFX_VERx10 >= 125
               ps.ScratchSpaceBuffer = scratch_addr >> 4;
#else
               ps.ScratchSpaceBasePointer =
                  rw_bo(NULL, scratch_addr, IRIS_DOMAIN_NONE);
#endif
            }

            uint32_t psx_state[GENX(3DSTATE_PS_EXTRA_length)] = {0};
            iris_pack_command(GENX(3DSTATE_PS_EXTRA), psx_state, psx) {
#if GFX_VER >= 9
               if (!wm_prog_data->uses_sample_mask)
                  psx.InputCoverageMaskState  = ICMS_NONE;
               else if (wm_prog_data->post_depth_coverage)
                  psx.InputCoverageMaskState = ICMS_DEPTH_COVERAGE;
               else if (wm_prog_data->inner_coverage &&
                        cso->conservative_rasterization)
                  psx.InputCoverageMaskState = ICMS_INNER_CONSERVATIVE;
               else
                  psx.InputCoverageMaskState = ICMS_NORMAL;
#else
               psx.PixelShaderUsesInputCoverageMask =
                  wm_prog_data->uses_sample_mask;
#endif
            }

            uint32_t *shader_ps = (uint32_t *) shader->derived_data;
            uint32_t *shader_psx = shader_ps + GENX(3DSTATE_PS_length);
            iris_emit_merge(batch, shader_ps, ps_state,
                            GENX(3DSTATE_PS_length));
            iris_emit_merge(batch, shader_psx, psx_state,
                            GENX(3DSTATE_PS_EXTRA_length));
#if GFX_VERx10 >= 125
         } else if (stage == MESA_SHADER_TESS_EVAL) {
            uint32_t te_state[GENX(3DSTATE_TE_length)] = { 0 };
            iris_pack_command(GENX(3DSTATE_TE), te_state, te) {
               if (intel_needs_workaround(screen->devinfo, 14015055625) &&
                   program_needs_wa_14015055625)
                  te.TessellationDistributionMode = TEDMODE_OFF;
               else if (intel_needs_workaround(screen->devinfo, 22012699309))
                  te.TessellationDistributionMode = TEDMODE_RR_STRICT;
               else
                  te.TessellationDistributionMode = TEDMODE_RR_FREE;
            }

            uint32_t ds_state[GENX(3DSTATE_DS_length)] = { 0 };
            iris_pack_command(GENX(3DSTATE_DS), ds_state, ds) {
               if (scratch_addr)
                  ds.ScratchSpaceBuffer = scratch_addr >> 4;
            }

            uint32_t *shader_ds = (uint32_t *) shader->derived_data;
            uint32_t *shader_te = shader_ds + GENX(3DSTATE_DS_length);

            iris_emit_merge(batch, shader_ds, ds_state,
                            GENX(3DSTATE_DS_length));
            iris_emit_merge(batch, shader_te, te_state,
                            GENX(3DSTATE_TE_length));
#endif
         } else if (scratch_addr) {
            uint32_t *pkt = (uint32_t *) shader->derived_data;
            switch (stage) {
            case MESA_SHADER_VERTEX:    MERGE_SCRATCH_ADDR(3DSTATE_VS); break;
            case MESA_SHADER_TESS_CTRL: MERGE_SCRATCH_ADDR(3DSTATE_HS); break;
            case MESA_SHADER_TESS_EVAL: {
               uint32_t *shader_ds = (uint32_t *) shader->derived_data;
               uint32_t *shader_te = shader_ds + GENX(3DSTATE_DS_length);
               iris_batch_emit(batch, shader_te, 4 * GENX(3DSTATE_TE_length));
               MERGE_SCRATCH_ADDR(3DSTATE_DS);
               break;
            }
            case MESA_SHADER_GEOMETRY:  MERGE_SCRATCH_ADDR(3DSTATE_GS); break;
            }
         } else {
            iris_batch_emit(batch, shader->derived_data,
                            iris_derived_program_state_size(stage));
         }
      } else {
         if (stage == MESA_SHADER_TESS_EVAL) {
            iris_emit_cmd(batch, GENX(3DSTATE_HS), hs);
            iris_emit_cmd(batch, GENX(3DSTATE_TE), te);
            iris_emit_cmd(batch, GENX(3DSTATE_DS), ds);
         } else if (stage == MESA_SHADER_GEOMETRY) {
            iris_emit_cmd(batch, GENX(3DSTATE_GS), gs);
         }
      }
   }

   if (ice->state.streamout_active) {
      if (dirty & IRIS_DIRTY_SO_BUFFERS) {
         /* Wa_16011411144
          * SW must insert a PIPE_CONTROL cmd before and after the
          * 3dstate_so_buffer_index_0/1/2/3 states to ensure so_buffer_index_* state is
          * not combined with other state changes.
          */
         if (intel_device_info_is_dg2(batch->screen->devinfo)) {
            iris_emit_pipe_control_flush(batch,
                                         "SO pre change stall WA",
                                         PIPE_CONTROL_CS_STALL);
         }

         for (int i = 0; i < 4; i++) {
            struct iris_stream_output_target *tgt =
               (void *) ice->state.so_target[i];
            enum { dwords = GENX(3DSTATE_SO_BUFFER_length) };
            uint32_t *so_buffers = genx->so_buffers + i * dwords;
            bool zero_offset = false;

            if (tgt) {
               zero_offset = tgt->zero_offset;
               iris_use_pinned_bo(batch, iris_resource_bo(tgt->base.buffer),
                                  true, IRIS_DOMAIN_OTHER_WRITE);
               iris_use_pinned_bo(batch, iris_resource_bo(tgt->offset.res),
                                  true, IRIS_DOMAIN_OTHER_WRITE);
            }

            if (zero_offset) {
               /* Skip the last DWord which contains "Stream Offset" of
                * 0xFFFFFFFF and instead emit a dword of zero directly.
                */
               STATIC_ASSERT(GENX(3DSTATE_SO_BUFFER_StreamOffset_start) ==
                             32 * (dwords - 1));
               const uint32_t zero = 0;
               iris_batch_emit(batch, so_buffers, 4 * (dwords - 1));
               iris_batch_emit(batch, &zero, sizeof(zero));
               tgt->zero_offset = false;
            } else {
               iris_batch_emit(batch, so_buffers, 4 * dwords);
            }
         }

         /* Wa_16011411144 */
         if (intel_device_info_is_dg2(batch->screen->devinfo)) {
            iris_emit_pipe_control_flush(batch,
                                         "SO post change stall WA",
                                         PIPE_CONTROL_CS_STALL);
         }
      }

      if ((dirty & IRIS_DIRTY_SO_DECL_LIST) && ice->state.streamout) {
         /* Wa_16011773973:
          * If SOL is enabled and SO_DECL state has to be programmed,
          *    1. Send 3D State SOL state with SOL disabled
          *    2. Send SO_DECL NP state
          *    3. Send 3D State SOL with SOL Enabled
          */
         if (intel_device_info_is_dg2(batch->screen->devinfo))
            iris_emit_cmd(batch, GENX(3DSTATE_STREAMOUT), sol);

         uint32_t *decl_list =
            ice->state.streamout + GENX(3DSTATE_STREAMOUT_length);
         iris_batch_emit(batch, decl_list, 4 * ((decl_list[0] & 0xff) + 2));

#if GFX_VER >= 11
         /* ICL PRMs, Volume 2a - Command Reference: Instructions,
          * 3DSTATE_SO_DECL_LIST:
          *
          *    "Workaround: This command must be followed by a PIPE_CONTROL
          *     with CS Stall bit set."
          *
          * On DG2+ also known as Wa_1509820217.
          */
         iris_emit_pipe_control_flush(batch,
                                      "workaround: cs stall after so_decl",
                                      PIPE_CONTROL_CS_STALL);
#endif
      }

      if (dirty & IRIS_DIRTY_STREAMOUT) {
         const struct iris_rasterizer_state *cso_rast = ice->state.cso_rast;

#if GFX_VERx10 >= 120
         /* Wa_16013994831 - Disable preemption. */
         if (intel_needs_workaround(batch->screen->devinfo, 16013994831))
            iris_preemption_streamout_wa(ice, batch, false);
#endif

         uint32_t dynamic_sol[GENX(3DSTATE_STREAMOUT_length)];
         iris_pack_command(GENX(3DSTATE_STREAMOUT), dynamic_sol, sol) {
            sol.SOFunctionEnable = true;
            sol.SOStatisticsEnable = true;

            sol.RenderingDisable = cso_rast->rasterizer_discard &&
                                   !ice->state.prims_generated_query_active;
            sol.ReorderMode = cso_rast->flatshade_first ? LEADING : TRAILING;


#if INTEL_NEEDS_WA_18022508906
            /* Wa_14017076903 :
             *
             * SKL PRMs, Volume 7: 3D-Media-GPGPU, Stream Output Logic (SOL) Stage:
             *
             * SOL_INT::Render_Enable =
             *   (3DSTATE_STREAMOUT::Force_Rending == Force_On) ||
             *   (
             *     (3DSTATE_STREAMOUT::Force_Rending != Force_Off) &&
             *     !(3DSTATE_GS::Enable && 3DSTATE_GS::Output Vertex Size == 0) &&
             *     !3DSTATE_STREAMOUT::API_Render_Disable &&
             *     (
             *       3DSTATE_DEPTH_STENCIL_STATE::Stencil_TestEnable ||
             *       3DSTATE_DEPTH_STENCIL_STATE::Depth_TestEnable ||
             *       3DSTATE_DEPTH_STENCIL_STATE::Depth_WriteEnable ||
             *       3DSTATE_PS_EXTRA::PS_Valid ||
             *       3DSTATE_WM::Legacy Depth_Buffer_Clear ||
             *       3DSTATE_WM::Legacy Depth_Buffer_Resolve_Enable ||
             *       3DSTATE_WM::Legacy Hierarchical_Depth_Buffer_Resolve_Enable
             *     )
             *   )
             *
             * If SOL_INT::Render_Enable is false, the SO stage will not forward any
             * topologies down the pipeline. Which is not what we want for occlusion
             * queries.
             *
             * Here we force rendering to get SOL_INT::Render_Enable when occlusion
             * queries are active.
             */
            const struct iris_rasterizer_state *cso_rast = ice->state.cso_rast;
            if (!cso_rast->rasterizer_discard && ice->state.occlusion_query_active)
               sol.ForceRendering = Force_on;
#endif
         }

         assert(ice->state.streamout);

         iris_emit_merge(batch, ice->state.streamout, dynamic_sol,
                         GENX(3DSTATE_STREAMOUT_length));
      }
   } else {
      if (dirty & IRIS_DIRTY_STREAMOUT) {

#if GFX_VERx10 >= 120
         /* Wa_16013994831 - Enable preemption. */
         if (!ice->state.genx->object_preemption)
            iris_preemption_streamout_wa(ice, batch, true);
#endif

         iris_emit_cmd(batch, GENX(3DSTATE_STREAMOUT), sol);
      }
   }

   if (dirty & IRIS_DIRTY_CLIP) {
      struct iris_rasterizer_state *cso_rast = ice->state.cso_rast;
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;

      bool gs_or_tes = ice->shaders.prog[MESA_SHADER_GEOMETRY] ||
                       ice->shaders.prog[MESA_SHADER_TESS_EVAL];
      bool points_or_lines = cso_rast->fill_mode_point_or_line ||
         (gs_or_tes ? ice->shaders.output_topology_is_points_or_lines
                    : ice->state.prim_is_points_or_lines);

      uint32_t dynamic_clip[GENX(3DSTATE_CLIP_length)];
      iris_pack_command(GENX(3DSTATE_CLIP), &dynamic_clip, cl) {
         cl.StatisticsEnable = ice->state.statistics_counters_enabled;
         if (cso_rast->rasterizer_discard)
            cl.ClipMode = CLIPMODE_REJECT_ALL;
         else if (ice->state.window_space_position)
            cl.ClipMode = CLIPMODE_ACCEPT_ALL;
         else
            cl.ClipMode = CLIPMODE_NORMAL;

         cl.PerspectiveDivideDisable = ice->state.window_space_position;
         cl.ViewportXYClipTestEnable = !points_or_lines;

         cl.NonPerspectiveBarycentricEnable = wm_prog_data->uses_nonperspective_interp_modes;

         cl.ForceZeroRTAIndexEnable = cso_fb->layers <= 1;
         cl.MaximumVPIndex = ice->state.num_viewports - 1;
      }
      iris_emit_merge(batch, cso_rast->clip, dynamic_clip,
                      ARRAY_SIZE(cso_rast->clip));
   }

   if (dirty & (IRIS_DIRTY_RASTER | IRIS_DIRTY_URB)) {
      /* From the Browadwell PRM, Volume 2, documentation for
       * 3DSTATE_RASTER, "Antialiasing Enable":
       *
       * "This field must be disabled if any of the render targets
       * have integer (UINT or SINT) surface format."
       *
       * Additionally internal documentation for Gfx12+ states:
       *
       * "This bit MUST not be set when NUM_MULTISAMPLES > 1 OR
       *  FORCED_SAMPLE_COUNT > 1."
       */
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      unsigned samples = util_framebuffer_get_num_samples(cso_fb);
      struct iris_rasterizer_state *cso = ice->state.cso_rast;

      bool aa_enable = cso->line_smooth &&
                       !ice->state.has_integer_rt &&
                       !(batch->screen->devinfo->ver >= 12 && samples > 1);

      uint32_t dynamic_raster[GENX(3DSTATE_RASTER_length)];
      iris_pack_command(GENX(3DSTATE_RASTER), &dynamic_raster, raster) {
         raster.AntialiasingEnable = aa_enable;
      }
      iris_emit_merge(batch, cso->raster, dynamic_raster,
                      ARRAY_SIZE(cso->raster));

      uint32_t dynamic_sf[GENX(3DSTATE_SF_length)];
      iris_pack_command(GENX(3DSTATE_SF), &dynamic_sf, sf) {
         sf.ViewportTransformEnable = !ice->state.window_space_position;

#if GFX_VER >= 12
         sf.DerefBlockSize = ice->state.urb_deref_block_size;
#endif
      }
      iris_emit_merge(batch, cso->sf, dynamic_sf,
                      ARRAY_SIZE(dynamic_sf));
   }

   if (dirty & IRIS_DIRTY_WM) {
      struct iris_rasterizer_state *cso = ice->state.cso_rast;
      uint32_t dynamic_wm[GENX(3DSTATE_WM_length)];

      iris_pack_command(GENX(3DSTATE_WM), &dynamic_wm, wm) {
         wm.StatisticsEnable = ice->state.statistics_counters_enabled;

         wm.BarycentricInterpolationMode =
            wm_prog_data_barycentric_modes(wm_prog_data, 0);

         if (wm_prog_data->early_fragment_tests)
            wm.EarlyDepthStencilControl = EDSC_PREPS;
         else if (wm_prog_data->has_side_effects)
            wm.EarlyDepthStencilControl = EDSC_PSEXEC;
         else
            wm.EarlyDepthStencilControl = EDSC_NORMAL;

         /* We could skip this bit if color writes are enabled. */
         if (wm_prog_data->has_side_effects || wm_prog_data->uses_kill)
            wm.ForceThreadDispatchEnable = ForceON;
      }
      iris_emit_merge(batch, cso->wm, dynamic_wm, ARRAY_SIZE(cso->wm));
   }

   if (dirty & IRIS_DIRTY_SBE) {
      iris_emit_sbe(batch, ice);
   }

   if (dirty & IRIS_DIRTY_PS_BLEND) {
      struct iris_blend_state *cso_blend = ice->state.cso_blend;
      struct iris_depth_stencil_alpha_state *cso_zsa = ice->state.cso_zsa;
      const struct shader_info *fs_info =
         iris_get_shader_info(ice, MESA_SHADER_FRAGMENT);

      int dst_blend_factor = cso_blend->ps_dst_blend_factor[0];
      int dst_alpha_blend_factor = cso_blend->ps_dst_alpha_blend_factor[0];

      /* When MSAA is enabled, instead of using BLENDFACTOR_ZERO use
       * CONST_COLOR, CONST_ALPHA and supply zero by using blend constants.
       */
      if (needs_wa_14018912822) {
         if (ice->state.color_blend_zero)
            dst_blend_factor = BLENDFACTOR_CONST_COLOR;
         if (ice->state.alpha_blend_zero)
            dst_alpha_blend_factor = BLENDFACTOR_CONST_ALPHA;
      }

      uint32_t dynamic_pb[GENX(3DSTATE_PS_BLEND_length)];
      iris_pack_command(GENX(3DSTATE_PS_BLEND), &dynamic_pb, pb) {
         pb.HasWriteableRT = has_writeable_rt(cso_blend, fs_info);
         pb.AlphaTestEnable = cso_zsa->alpha_enabled;

         pb.DestinationBlendFactor = dst_blend_factor;
         pb.DestinationAlphaBlendFactor = dst_alpha_blend_factor;

         /* The dual source blending docs caution against using SRC1 factors
          * when the shader doesn't use a dual source render target write.
          * Empirically, this can lead to GPU hangs, and the results are
          * undefined anyway, so simply disable blending to avoid the hang.
          */
         pb.ColorBufferBlendEnable = (cso_blend->blend_enables & 1) &&
            (!cso_blend->dual_color_blending || wm_prog_data->dual_src_blend);
      }

      iris_emit_merge(batch, cso_blend->ps_blend, dynamic_pb,
                      ARRAY_SIZE(cso_blend->ps_blend));
   }

   if (dirty & IRIS_DIRTY_WM_DEPTH_STENCIL) {
      struct iris_depth_stencil_alpha_state *cso = ice->state.cso_zsa;
#if GFX_VER >= 9 && GFX_VER < 12
      struct pipe_stencil_ref *p_stencil_refs = &ice->state.stencil_ref;
      uint32_t stencil_refs[GENX(3DSTATE_WM_DEPTH_STENCIL_length)];
      iris_pack_command(GENX(3DSTATE_WM_DEPTH_STENCIL), &stencil_refs, wmds) {
         wmds.StencilReferenceValue = p_stencil_refs->ref_value[0];
         wmds.BackfaceStencilReferenceValue = p_stencil_refs->ref_value[1];
      }
      iris_emit_merge(batch, cso->wmds, stencil_refs, ARRAY_SIZE(cso->wmds));
#else
      /* Use modify disable fields which allow us to emit packets
       * directly instead of merging them later.
       */
      iris_batch_emit(batch, cso->wmds, sizeof(cso->wmds));
#endif

   /* Depth or stencil write changed in cso. */
   if (intel_needs_workaround(batch->screen->devinfo, 18019816803) &&
       (dirty & IRIS_DIRTY_DS_WRITE_ENABLE)) {
      iris_emit_pipe_control_flush(
         batch, "workaround: PSS stall after DS write enable change",
         PIPE_CONTROL_PSS_STALL_SYNC);
   }

#if GFX_VER >= 12
      iris_batch_emit(batch, cso->depth_bounds, sizeof(cso->depth_bounds));
#endif
   }

   if (dirty & IRIS_DIRTY_STENCIL_REF) {
#if GFX_VER >= 12
      /* Use modify disable fields which allow us to emit packets
       * directly instead of merging them later.
       */
      struct pipe_stencil_ref *p_stencil_refs = &ice->state.stencil_ref;
      uint32_t stencil_refs[GENX(3DSTATE_WM_DEPTH_STENCIL_length)];
      iris_pack_command(GENX(3DSTATE_WM_DEPTH_STENCIL), &stencil_refs, wmds) {
         wmds.StencilReferenceValue = p_stencil_refs->ref_value[0];
         wmds.BackfaceStencilReferenceValue = p_stencil_refs->ref_value[1];
         wmds.StencilTestMaskModifyDisable = true;
         wmds.StencilWriteMaskModifyDisable = true;
         wmds.StencilStateModifyDisable = true;
         wmds.DepthStateModifyDisable = true;
      }
      iris_batch_emit(batch, stencil_refs, sizeof(stencil_refs));
#endif
   }

   if (dirty & IRIS_DIRTY_SCISSOR_RECT) {
      /* Wa_1409725701:
       *    "The viewport-specific state used by the SF unit (SCISSOR_RECT) is
       *    stored as an array of up to 16 elements. The location of first
       *    element of the array, as specified by Pointer to SCISSOR_RECT,
       *    should be aligned to a 64-byte boundary.
       */
      uint32_t alignment = 64;
      uint32_t scissor_offset =
         emit_state(batch, ice->state.dynamic_uploader,
                    &ice->state.last_res.scissor,
                    ice->state.scissors,
                    sizeof(struct pipe_scissor_state) *
                    ice->state.num_viewports, alignment);

      iris_emit_cmd(batch, GENX(3DSTATE_SCISSOR_STATE_POINTERS), ptr) {
         ptr.ScissorRectPointer = scissor_offset;
      }
   }

   if (dirty & IRIS_DIRTY_DEPTH_BUFFER) {
      struct iris_depth_buffer_state *cso_z = &ice->state.genx->depth_buffer;

      /* Do not emit the cso yet. We may need to update clear params first. */
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      struct iris_resource *zres = NULL, *sres = NULL;
      if (cso_fb->zsbuf) {
         iris_get_depth_stencil_resources(cso_fb->zsbuf->texture,
                                          &zres, &sres);
      }

      if (zres && ice->state.hiz_usage != ISL_AUX_USAGE_NONE) {
#if GFX_VER < 20
         uint32_t *clear_params =
            cso_z->packets + ARRAY_SIZE(cso_z->packets) -
            GENX(3DSTATE_CLEAR_PARAMS_length);

         iris_pack_command(GENX(3DSTATE_CLEAR_PARAMS), clear_params, clear) {
            clear.DepthClearValueValid = true;
            clear.DepthClearValue = zres->aux.clear_color.f32[0];
         }
#endif
      }

      iris_batch_emit(batch, cso_z->packets, sizeof(cso_z->packets));

      /* Wa_14016712196:
       * Emit depth flush after state that sends implicit depth flush.
       */
      if (intel_needs_workaround(batch->screen->devinfo, 14016712196)) {
         iris_emit_pipe_control_flush(batch, "Wa_14016712196",
                                      PIPE_CONTROL_DEPTH_CACHE_FLUSH);
      }

      if (zres)
         genX(emit_depth_state_workarounds)(ice, batch, &zres->surf);

      if (intel_needs_workaround(batch->screen->devinfo, 1408224581) ||
          intel_needs_workaround(batch->screen->devinfo, 14014097488)) {
         /* Wa_1408224581
          *
          * Workaround: Gfx12LP Astep only An additional pipe control with
          * post-sync = store dword operation would be required.( w/a is to
          * have an additional pipe control after the stencil state whenever
          * the surface state bits of this state is changing).
          *
          * This also seems sufficient to handle Wa_14014097488.
          */
         iris_emit_pipe_control_write(batch, "WA for stencil state",
                                      PIPE_CONTROL_WRITE_IMMEDIATE,
                                      screen->workaround_address.bo,
                                      screen->workaround_address.offset, 0);
      }
   }

   if (dirty & (IRIS_DIRTY_DEPTH_BUFFER | IRIS_DIRTY_WM_DEPTH_STENCIL)) {
      /* Listen for buffer changes, and also write enable changes. */
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      pin_depth_and_stencil_buffers(batch, cso_fb->zsbuf, ice->state.cso_zsa);
   }

   if (dirty & IRIS_DIRTY_POLYGON_STIPPLE) {
      iris_emit_cmd(batch, GENX(3DSTATE_POLY_STIPPLE_PATTERN), poly) {
         for (int i = 0; i < 32; i++) {
            poly.PatternRow[i] = ice->state.poly_stipple.stipple[i];
         }
      }
   }

   if (dirty & IRIS_DIRTY_LINE_STIPPLE) {
      struct iris_rasterizer_state *cso = ice->state.cso_rast;
      iris_batch_emit(batch, cso->line_stipple, sizeof(cso->line_stipple));
#if GFX_VER >= 11
      /* ICL PRMs, Volume 2a - Command Reference: Instructions,
       * 3DSTATE_LINE_STIPPLE:
       *
       *    "Workaround: This command must be followed by a PIPE_CONTROL with
       *     CS Stall bit set."
       */
      iris_emit_pipe_control_flush(batch,
                                   "workaround: post 3DSTATE_LINE_STIPPLE",
                                   PIPE_CONTROL_CS_STALL);
#endif
   }

   if (dirty & IRIS_DIRTY_VF_TOPOLOGY) {
      iris_emit_cmd(batch, GENX(3DSTATE_VF_TOPOLOGY), topo) {
         topo.PrimitiveTopologyType =
            translate_prim_type(draw->mode, ice->state.vertices_per_patch);
      }
   }

   if (dirty & IRIS_DIRTY_VERTEX_BUFFERS) {
      int count = util_bitcount64(ice->state.bound_vertex_buffers);
      uint64_t dynamic_bound = ice->state.bound_vertex_buffers;

      if (ice->state.vs_uses_draw_params) {
         assert(ice->draw.draw_params.res);

         struct iris_vertex_buffer_state *state =
            &(ice->state.genx->vertex_buffers[count]);
         pipe_resource_reference(&state->resource, ice->draw.draw_params.res);
         struct iris_resource *res = (void *) state->resource;

         iris_pack_state(GENX(VERTEX_BUFFER_STATE), state->state, vb) {
            vb.VertexBufferIndex = count;
            vb.AddressModifyEnable = true;
            vb.BufferPitch = 0;
            vb.BufferSize = res->bo->size - ice->draw.draw_params.offset;
            vb.BufferStartingAddress =
               ro_bo(NULL, res->bo->address +
                           (int) ice->draw.draw_params.offset);
            vb.MOCS = iris_mocs(res->bo, &screen->isl_dev,
                                ISL_SURF_USAGE_VERTEX_BUFFER_BIT);
#if GFX_VER >= 12
            vb.L3BypassDisable       = true;
#endif
         }
         dynamic_bound |= 1ull << count;
         count++;
      }

      if (ice->state.vs_uses_derived_draw_params) {
         struct iris_vertex_buffer_state *state =
            &(ice->state.genx->vertex_buffers[count]);
         pipe_resource_reference(&state->resource,
                                 ice->draw.derived_draw_params.res);
         struct iris_resource *res = (void *) ice->draw.derived_draw_params.res;

         iris_pack_state(GENX(VERTEX_BUFFER_STATE), state->state, vb) {
             vb.VertexBufferIndex = count;
            vb.AddressModifyEnable = true;
            vb.BufferPitch = 0;
            vb.BufferSize =
               res->bo->size - ice->draw.derived_draw_params.offset;
            vb.BufferStartingAddress =
               ro_bo(NULL, res->bo->address +
                           (int) ice->draw.derived_draw_params.offset);
            vb.MOCS = iris_mocs(res->bo, &screen->isl_dev,
                                ISL_SURF_USAGE_VERTEX_BUFFER_BIT);
#if GFX_VER >= 12
            vb.L3BypassDisable       = true;
#endif
         }
         dynamic_bound |= 1ull << count;
         count++;
      }

      if (count) {
#if GFX_VER >= 11
         /* Gfx11+ doesn't need the cache workaround below */
         uint64_t bound = dynamic_bound;
         while (bound) {
            const int i = u_bit_scan64(&bound);
            iris_use_optional_res(batch, genx->vertex_buffers[i].resource,
                                  false, IRIS_DOMAIN_VF_READ);
         }
#else
         /* The VF cache designers cut corners, and made the cache key's
          * <VertexBufferIndex, Memory Address> tuple only consider the bottom
          * 32 bits of the address.  If you have two vertex buffers which get
          * placed exactly 4 GiB apart and use them in back-to-back draw calls,
          * you can get collisions (even within a single batch).
          *
          * So, we need to do a VF cache invalidate if the buffer for a VB
          * slot slot changes [48:32] address bits from the previous time.
          */
         unsigned flush_flags = 0;

         uint64_t bound = dynamic_bound;
         while (bound) {
            const int i = u_bit_scan64(&bound);
            uint16_t high_bits = 0;

            struct iris_resource *res =
               (void *) genx->vertex_buffers[i].resource;
            if (res) {
               iris_use_pinned_bo(batch, res->bo, false, IRIS_DOMAIN_VF_READ);

               high_bits = res->bo->address >> 32ull;
               if (high_bits != ice->state.last_vbo_high_bits[i]) {
                  flush_flags |= PIPE_CONTROL_VF_CACHE_INVALIDATE |
                                 PIPE_CONTROL_CS_STALL;
                  ice->state.last_vbo_high_bits[i] = high_bits;
               }
            }
         }

         if (flush_flags) {
            iris_emit_pipe_control_flush(batch,
                                         "workaround: VF cache 32-bit key [VB]",
                                         flush_flags);
         }
#endif

         const unsigned vb_dwords = GENX(VERTEX_BUFFER_STATE_length);

         uint32_t *map =
            iris_get_command_space(batch, 4 * (1 + vb_dwords * count));
         _iris_pack_command(batch, GENX(3DSTATE_VERTEX_BUFFERS), map, vb) {
            vb.DWordLength = (vb_dwords * count + 1) - 2;
         }
         map += 1;

         const struct iris_vertex_element_state *cso_ve =
            ice->state.cso_vertex_elements;

         bound = dynamic_bound;
         while (bound) {
            const int i = u_bit_scan64(&bound);

            uint32_t vb_stride[GENX(VERTEX_BUFFER_STATE_length)];
            struct iris_bo *bo =
               iris_resource_bo(genx->vertex_buffers[i].resource);
            iris_pack_state(GENX(VERTEX_BUFFER_STATE), &vb_stride, vbs) {
               vbs.BufferPitch = cso_ve->stride[i];
               /* Unnecessary except to defeat the genxml nonzero checker */
               vbs.MOCS = iris_mocs(bo, &screen->isl_dev,
                                    ISL_SURF_USAGE_VERTEX_BUFFER_BIT);
            }
            for (unsigned d = 0; d < vb_dwords; d++)
               map[d] = genx->vertex_buffers[i].state[d] | vb_stride[d];

            map += vb_dwords;
         }
      }
   }

   if (dirty & IRIS_DIRTY_VERTEX_ELEMENTS) {
      struct iris_vertex_element_state *cso = ice->state.cso_vertex_elements;
      const unsigned entries = MAX2(cso->count, 1);
      if (!(ice->state.vs_needs_sgvs_element ||
            ice->state.vs_uses_derived_draw_params ||
            ice->state.vs_needs_edge_flag)) {
         iris_batch_emit(batch, cso->vertex_elements, sizeof(uint32_t) *
                         (1 + entries * GENX(VERTEX_ELEMENT_STATE_length)));
      } else {
         uint32_t dynamic_ves[1 + 33 * GENX(VERTEX_ELEMENT_STATE_length)];
         const unsigned dyn_count = cso->count +
            ice->state.vs_needs_sgvs_element +
            ice->state.vs_uses_derived_draw_params;

         iris_pack_command(GENX(3DSTATE_VERTEX_ELEMENTS),
                           &dynamic_ves, ve) {
            ve.DWordLength =
               1 + GENX(VERTEX_ELEMENT_STATE_length) * dyn_count - 2;
         }
         memcpy(&dynamic_ves[1], &cso->vertex_elements[1],
                (cso->count - ice->state.vs_needs_edge_flag) *
                GENX(VERTEX_ELEMENT_STATE_length) * sizeof(uint32_t));
         uint32_t *ve_pack_dest =
            &dynamic_ves[1 + (cso->count - ice->state.vs_needs_edge_flag) *
                         GENX(VERTEX_ELEMENT_STATE_length)];

         if (ice->state.vs_needs_sgvs_element) {
            uint32_t base_ctrl = ice->state.vs_uses_draw_params ?
                                 VFCOMP_STORE_SRC : VFCOMP_STORE_0;
            iris_pack_state(GENX(VERTEX_ELEMENT_STATE), ve_pack_dest, ve) {
               ve.Valid = true;
               ve.VertexBufferIndex =
                  util_bitcount64(ice->state.bound_vertex_buffers);
               ve.SourceElementFormat = ISL_FORMAT_R32G32_UINT;
               ve.Component0Control = base_ctrl;
               ve.Component1Control = base_ctrl;
               ve.Component2Control = VFCOMP_STORE_0;
               ve.Component3Control = VFCOMP_STORE_0;
            }
            ve_pack_dest += GENX(VERTEX_ELEMENT_STATE_length);
         }
         if (ice->state.vs_uses_derived_draw_params) {
            iris_pack_state(GENX(VERTEX_ELEMENT_STATE), ve_pack_dest, ve) {
               ve.Valid = true;
               ve.VertexBufferIndex =
                  util_bitcount64(ice->state.bound_vertex_buffers) +
                  ice->state.vs_uses_draw_params;
               ve.SourceElementFormat = ISL_FORMAT_R32G32_UINT;
               ve.Component0Control = VFCOMP_STORE_SRC;
               ve.Component1Control = VFCOMP_STORE_SRC;
               ve.Component2Control = VFCOMP_STORE_0;
               ve.Component3Control = VFCOMP_STORE_0;
            }
            ve_pack_dest += GENX(VERTEX_ELEMENT_STATE_length);
         }
         if (ice->state.vs_needs_edge_flag) {
            for (int i = 0; i < GENX(VERTEX_ELEMENT_STATE_length);  i++)
               ve_pack_dest[i] = cso->edgeflag_ve[i];
         }

         iris_batch_emit(batch, &dynamic_ves, sizeof(uint32_t) *
                         (1 + dyn_count * GENX(VERTEX_ELEMENT_STATE_length)));
      }

      if (!ice->state.vs_needs_edge_flag) {
         iris_batch_emit(batch, cso->vf_instancing, sizeof(uint32_t) *
                         entries * GENX(3DSTATE_VF_INSTANCING_length));
      } else {
         assert(cso->count > 0);
         const unsigned edgeflag_index = cso->count - 1;
         uint32_t dynamic_vfi[33 * GENX(3DSTATE_VF_INSTANCING_length)];
         memcpy(&dynamic_vfi[0], cso->vf_instancing, edgeflag_index *
                GENX(3DSTATE_VF_INSTANCING_length) * sizeof(uint32_t));

         uint32_t *vfi_pack_dest = &dynamic_vfi[0] +
            edgeflag_index * GENX(3DSTATE_VF_INSTANCING_length);
         iris_pack_command(GENX(3DSTATE_VF_INSTANCING), vfi_pack_dest, vi) {
            vi.VertexElementIndex = edgeflag_index +
               ice->state.vs_needs_sgvs_element +
               ice->state.vs_uses_derived_draw_params;
         }
         for (int i = 0; i < GENX(3DSTATE_VF_INSTANCING_length);  i++)
            vfi_pack_dest[i] |= cso->edgeflag_vfi[i];

         iris_batch_emit(batch, &dynamic_vfi[0], sizeof(uint32_t) *
                         entries * GENX(3DSTATE_VF_INSTANCING_length));
      }
   }

   if (dirty & IRIS_DIRTY_VF_SGVS) {
      const struct brw_vs_prog_data *vs_prog_data = (void *)
         ice->shaders.prog[MESA_SHADER_VERTEX]->prog_data;
      struct iris_vertex_element_state *cso = ice->state.cso_vertex_elements;

      iris_emit_cmd(batch, GENX(3DSTATE_VF_SGVS), sgv) {
         if (vs_prog_data->uses_vertexid) {
            sgv.VertexIDEnable = true;
            sgv.VertexIDComponentNumber = 2;
            sgv.VertexIDElementOffset =
               cso->count - ice->state.vs_needs_edge_flag;
         }

         if (vs_prog_data->uses_instanceid) {
            sgv.InstanceIDEnable = true;
            sgv.InstanceIDComponentNumber = 3;
            sgv.InstanceIDElementOffset =
               cso->count - ice->state.vs_needs_edge_flag;
         }
      }
   }

   if (dirty & IRIS_DIRTY_VF) {
      iris_emit_cmd(batch, GENX(3DSTATE_VF), vf) {
#if GFX_VERx10 >= 125
         vf.GeometryDistributionEnable = true;
#endif
         if (draw->primitive_restart) {
            vf.IndexedDrawCutIndexEnable = true;
            vf.CutIndex = draw->restart_index;
         }
      }
   }

#if GFX_VERx10 >= 125
   if (dirty & IRIS_DIRTY_VFG) {
      iris_emit_cmd(batch, GENX(3DSTATE_VFG), vfg) {
         /* If 3DSTATE_TE: TE Enable == 1 then RR_STRICT else RR_FREE*/
         vfg.DistributionMode =
            ice->shaders.prog[MESA_SHADER_TESS_EVAL] != NULL ? RR_STRICT :
                                                               RR_FREE;
         vfg.DistributionGranularity = BatchLevelGranularity;
#if INTEL_WA_14014851047_GFX_VER
         vfg.GranularityThresholdDisable =
            intel_needs_workaround(batch->screen->devinfo, 14014851047);
#endif
         vfg.ListCutIndexEnable = draw->primitive_restart;
         /* 192 vertices for TRILIST_ADJ */
         vfg.ListNBatchSizeScale = 0;
         /* Batch size of 384 vertices */
         vfg.List3BatchSizeScale = 2;
         /* Batch size of 128 vertices */
         vfg.List2BatchSizeScale = 1;
         /* Batch size of 128 vertices */
         vfg.List1BatchSizeScale = 2;
         /* Batch size of 256 vertices for STRIP topologies */
         vfg.StripBatchSizeScale = 3;
         /* 192 control points for PATCHLIST_3 */
         vfg.PatchBatchSizeScale = 1;
         /* 192 control points for PATCHLIST_3 */
         vfg.PatchBatchSizeMultiplier = 31;
      }
   }
#endif

   if (dirty & IRIS_DIRTY_VF_STATISTICS) {
      iris_emit_cmd(batch, GENX(3DSTATE_VF_STATISTICS), vf) {
         vf.StatisticsEnable = true;
      }
   }

#if GFX_VER == 8
   if (dirty & IRIS_DIRTY_PMA_FIX) {
      bool enable = want_pma_fix(ice);
      genX(update_pma_fix)(ice, batch, enable);
   }
#endif

   if (ice->state.current_hash_scale != 1)
      genX(emit_hashing_mode)(ice, batch, UINT_MAX, UINT_MAX, 1);

#if GFX_VER >= 12
   genX(invalidate_aux_map_state)(batch);
#endif
}

static void
flush_vbos(struct iris_context *ice, struct iris_batch *batch)
{
   struct iris_genx_state *genx = ice->state.genx;
   uint64_t bound = ice->state.bound_vertex_buffers;
   while (bound) {
      const int i = u_bit_scan64(&bound);
      struct iris_bo *bo = iris_resource_bo(genx->vertex_buffers[i].resource);
      iris_emit_buffer_barrier_for(batch, bo, IRIS_DOMAIN_VF_READ);
   }
}

static bool
point_or_line_list(enum mesa_prim prim_type)
{
   switch (prim_type) {
   case MESA_PRIM_POINTS:
   case MESA_PRIM_LINES:
   case MESA_PRIM_LINE_STRIP:
   case MESA_PRIM_LINES_ADJACENCY:
   case MESA_PRIM_LINE_STRIP_ADJACENCY:
   case MESA_PRIM_LINE_LOOP:
      return true;
   default:
      return false;
   }
   return false;
}

void
genX(emit_breakpoint)(struct iris_batch *batch, bool emit_before_draw)
{
   struct iris_context *ice = batch->ice;
   uint32_t draw_count = emit_before_draw ?
                         p_atomic_inc_return(&ice->draw_call_count) :
                         p_atomic_read(&ice->draw_call_count);

   if (((draw_count == intel_debug_bkp_before_draw_count &&
         emit_before_draw) ||
        (draw_count == intel_debug_bkp_after_draw_count &&
         !emit_before_draw)))  {
      iris_emit_cmd(batch, GENX(MI_SEMAPHORE_WAIT), sem) {
         sem.WaitMode            = PollingMode;
         sem.CompareOperation    = COMPARE_SAD_EQUAL_SDD;
         sem.SemaphoreDataDword  = 0x1;
         sem.SemaphoreAddress    = rw_bo(batch->screen->breakpoint_bo, 0,
                                         IRIS_DOMAIN_OTHER_WRITE);
      };
   }
}

void
genX(emit_3dprimitive_was)(struct iris_batch *batch,
                           const struct pipe_draw_indirect_info *indirect,
                           uint32_t primitive_type,
                           uint32_t vertex_count)
{
   UNUSED const struct intel_device_info *devinfo = batch->screen->devinfo;
   UNUSED const struct iris_context *ice = batch->ice;

#if INTEL_WA_22014412737_GFX_VER || INTEL_WA_16014538804_GFX_VER
   if (intel_needs_workaround(devinfo, 22014412737) &&
       (point_or_line_list(primitive_type) || indirect ||
        (vertex_count == 1 || vertex_count == 2))) {
         iris_emit_pipe_control_write(batch, "Wa_22014412737",
                                      PIPE_CONTROL_WRITE_IMMEDIATE,
                                      batch->screen->workaround_bo,
                                      batch->screen->workaround_address.offset,
                                      0ull);
      batch->num_3d_primitives_emitted = 0;
   } else if (intel_needs_workaround(devinfo, 16014538804)) {
      batch->num_3d_primitives_emitted++;

      /* Wa_16014538804 - Send empty/dummy pipe control after 3 3DPRIMITIVE. */
      if (batch->num_3d_primitives_emitted == 3) {
         iris_emit_pipe_control_flush(batch, "Wa_16014538804", 0);
         batch->num_3d_primitives_emitted = 0;
      }
   }
#endif
}

static void
iris_upload_render_state(struct iris_context *ice,
                         struct iris_batch *batch,
                         const struct pipe_draw_info *draw,
                         unsigned drawid_offset,
                         const struct pipe_draw_indirect_info *indirect,
                         const struct pipe_draw_start_count_bias *sc)
{
   UNUSED const struct intel_device_info *devinfo = batch->screen->devinfo;
   bool use_predicate = ice->state.predicate == IRIS_PREDICATE_STATE_USE_BIT;

   trace_intel_begin_draw(&batch->trace);

   if (ice->state.dirty & IRIS_DIRTY_VERTEX_BUFFER_FLUSHES)
      flush_vbos(ice, batch);

   iris_batch_sync_region_start(batch);

   /* Always pin the binder.  If we're emitting new binding table pointers,
    * we need it.  If not, we're probably inheriting old tables via the
    * context, and need it anyway.  Since true zero-bindings cases are
    * practically non-existent, just pin it and avoid last_res tracking.
    */
   iris_use_pinned_bo(batch, ice->state.binder.bo, false,
                      IRIS_DOMAIN_NONE);

   if (!batch->contains_draw) {
      if (GFX_VER == 12) {
         /* Re-emit constants when starting a new batch buffer in order to
          * work around push constant corruption on context switch.
          *
          * XXX - Provide hardware spec quotation when available.
          */
         ice->state.stage_dirty |= (IRIS_STAGE_DIRTY_CONSTANTS_VS  |
                                    IRIS_STAGE_DIRTY_CONSTANTS_TCS |
                                    IRIS_STAGE_DIRTY_CONSTANTS_TES |
                                    IRIS_STAGE_DIRTY_CONSTANTS_GS  |
                                    IRIS_STAGE_DIRTY_CONSTANTS_FS);
      }
      batch->contains_draw = true;
   }

   if (!batch->contains_draw_with_next_seqno) {
      iris_restore_render_saved_bos(ice, batch, draw);
      batch->contains_draw_with_next_seqno = true;
   }

   /* Wa_1306463417 - Send HS state for every primitive on gfx11.
    * Wa_16011107343 (same for gfx12)
    * We implement this by setting TCS dirty on each draw.
    */
   if ((INTEL_NEEDS_WA_1306463417 || INTEL_NEEDS_WA_16011107343) &&
       ice->shaders.prog[MESA_SHADER_TESS_CTRL]) {
      ice->state.stage_dirty |= IRIS_STAGE_DIRTY_TCS;
   }

   iris_upload_dirty_render_state(ice, batch, draw);

   if (draw->index_size > 0) {
      unsigned offset;

      if (draw->has_user_indices) {
         unsigned start_offset = draw->index_size * sc->start;

         u_upload_data(ice->ctx.const_uploader, start_offset,
                       sc->count * draw->index_size, 4,
                       (char*)draw->index.user + start_offset,
                       &offset, &ice->state.last_res.index_buffer);
         offset -= start_offset;
      } else {
         struct iris_resource *res = (void *) draw->index.resource;
         res->bind_history |= PIPE_BIND_INDEX_BUFFER;

         pipe_resource_reference(&ice->state.last_res.index_buffer,
                                 draw->index.resource);
         offset = 0;

         iris_emit_buffer_barrier_for(batch, res->bo, IRIS_DOMAIN_VF_READ);
      }

      struct iris_genx_state *genx = ice->state.genx;
      struct iris_bo *bo = iris_resource_bo(ice->state.last_res.index_buffer);

      uint32_t ib_packet[GENX(3DSTATE_INDEX_BUFFER_length)];
      iris_pack_command(GENX(3DSTATE_INDEX_BUFFER), ib_packet, ib) {
         ib.IndexFormat = draw->index_size >> 1;
         ib.MOCS = iris_mocs(bo, &batch->screen->isl_dev,
                             ISL_SURF_USAGE_INDEX_BUFFER_BIT);
         ib.BufferSize = bo->size - offset;
         ib.BufferStartingAddress = ro_bo(NULL, bo->address + offset);
#if GFX_VER >= 12
         ib.L3BypassDisable       = true;
#endif
      }

      if (memcmp(genx->last_index_buffer, ib_packet, sizeof(ib_packet)) != 0) {
         memcpy(genx->last_index_buffer, ib_packet, sizeof(ib_packet));
         iris_batch_emit(batch, ib_packet, sizeof(ib_packet));
         iris_use_pinned_bo(batch, bo, false, IRIS_DOMAIN_VF_READ);
      }

#if GFX_VER < 11
      /* The VF cache key only uses 32-bits, see vertex buffer comment above */
      uint16_t high_bits = bo->address >> 32ull;
      if (high_bits != ice->state.last_index_bo_high_bits) {
         iris_emit_pipe_control_flush(batch,
                                      "workaround: VF cache 32-bit key [IB]",
                                      PIPE_CONTROL_VF_CACHE_INVALIDATE |
                                      PIPE_CONTROL_CS_STALL);
         ice->state.last_index_bo_high_bits = high_bits;
      }
#endif
   }

   if (indirect) {
      struct mi_builder b;
      uint32_t mocs;
      mi_builder_init(&b, batch->screen->devinfo, batch);

#define _3DPRIM_END_OFFSET          0x2420
#define _3DPRIM_START_VERTEX        0x2430
#define _3DPRIM_VERTEX_COUNT        0x2434
#define _3DPRIM_INSTANCE_COUNT      0x2438
#define _3DPRIM_START_INSTANCE      0x243C
#define _3DPRIM_BASE_VERTEX         0x2440

      if (!indirect->count_from_stream_output) {
         if (indirect->indirect_draw_count) {
            use_predicate = true;

            struct iris_bo *draw_count_bo =
               iris_resource_bo(indirect->indirect_draw_count);
            unsigned draw_count_offset =
               indirect->indirect_draw_count_offset;
            mocs = iris_mocs(draw_count_bo, &batch->screen->isl_dev, 0);
            mi_builder_set_mocs(&b, mocs);

            if (ice->state.predicate == IRIS_PREDICATE_STATE_USE_BIT) {
               /* comparison = draw id < draw count */
               struct mi_value comparison =
                  mi_ult(&b, mi_imm(drawid_offset),
                             mi_mem32(ro_bo(draw_count_bo, draw_count_offset)));

               /* predicate = comparison & conditional rendering predicate */
               mi_store(&b, mi_reg32(MI_PREDICATE_RESULT),
                            mi_iand(&b, comparison, mi_reg32(CS_GPR(15))));
            } else {
               uint32_t mi_predicate;

               /* Upload the id of the current primitive to MI_PREDICATE_SRC1. */
               mi_store(&b, mi_reg64(MI_PREDICATE_SRC1), mi_imm(drawid_offset));
               /* Upload the current draw count from the draw parameters buffer
                * to MI_PREDICATE_SRC0. Zero the top 32-bits of
                * MI_PREDICATE_SRC0.
                */
               mi_store(&b, mi_reg64(MI_PREDICATE_SRC0),
                        mi_mem32(ro_bo(draw_count_bo, draw_count_offset)));

               if (drawid_offset == 0) {
                  mi_predicate = MI_PREDICATE | MI_PREDICATE_LOADOP_LOADINV |
                                 MI_PREDICATE_COMBINEOP_SET |
                                 MI_PREDICATE_COMPAREOP_SRCS_EQUAL;
               } else {
                  /* While draw_index < draw_count the predicate's result will be
                   *  (draw_index == draw_count) ^ TRUE = TRUE
                   * When draw_index == draw_count the result is
                   *  (TRUE) ^ TRUE = FALSE
                   * After this all results will be:
                   *  (FALSE) ^ FALSE = FALSE
                   */
                  mi_predicate = MI_PREDICATE | MI_PREDICATE_LOADOP_LOAD |
                                 MI_PREDICATE_COMBINEOP_XOR |
                                 MI_PREDICATE_COMPAREOP_SRCS_EQUAL;
               }
               iris_batch_emit(batch, &mi_predicate, sizeof(uint32_t));
            }
         }
         struct iris_bo *bo = iris_resource_bo(indirect->buffer);
         assert(bo);

         mocs = iris_mocs(bo, &batch->screen->isl_dev, 0);
         mi_builder_set_mocs(&b, mocs);

         mi_store(&b, mi_reg32(_3DPRIM_VERTEX_COUNT),
                  mi_mem32(ro_bo(bo, indirect->offset + 0)));
         mi_store(&b, mi_reg32(_3DPRIM_INSTANCE_COUNT),
                  mi_mem32(ro_bo(bo, indirect->offset + 4)));
         mi_store(&b, mi_reg32(_3DPRIM_START_VERTEX),
                  mi_mem32(ro_bo(bo, indirect->offset + 8)));
         if (draw->index_size) {
            mi_store(&b, mi_reg32(_3DPRIM_BASE_VERTEX),
                     mi_mem32(ro_bo(bo, indirect->offset + 12)));
            mi_store(&b, mi_reg32(_3DPRIM_START_INSTANCE),
                     mi_mem32(ro_bo(bo, indirect->offset + 16)));
         } else {
            mi_store(&b, mi_reg32(_3DPRIM_START_INSTANCE),
                     mi_mem32(ro_bo(bo, indirect->offset + 12)));
            mi_store(&b, mi_reg32(_3DPRIM_BASE_VERTEX), mi_imm(0));
         }
      } else if (indirect->count_from_stream_output) {
         struct iris_stream_output_target *so =
            (void *) indirect->count_from_stream_output;
         struct iris_bo *so_bo = iris_resource_bo(so->offset.res);

         mocs = iris_mocs(so_bo, &batch->screen->isl_dev, 0);
         mi_builder_set_mocs(&b, mocs);

         iris_emit_buffer_barrier_for(batch, so_bo, IRIS_DOMAIN_OTHER_READ);

         struct iris_address addr = ro_bo(so_bo, so->offset.offset);
         struct mi_value offset =
            mi_iadd_imm(&b, mi_mem32(addr), -so->base.buffer_offset);
         mi_store(&b, mi_reg32(_3DPRIM_VERTEX_COUNT),
                      mi_udiv32_imm(&b, offset, so->stride));
         mi_store(&b, mi_reg32(_3DPRIM_START_VERTEX), mi_imm(0));
         mi_store(&b, mi_reg32(_3DPRIM_BASE_VERTEX), mi_imm(0));
         mi_store(&b, mi_reg32(_3DPRIM_START_INSTANCE), mi_imm(0));
         mi_store(&b, mi_reg32(_3DPRIM_INSTANCE_COUNT),
                  mi_imm(draw->instance_count));
      }
   }

   iris_measure_snapshot(ice, batch, INTEL_SNAPSHOT_DRAW, draw, indirect, sc);

   genX(maybe_emit_breakpoint)(batch, true);

   iris_emit_cmd(batch, GENX(3DPRIMITIVE), prim) {
      prim.VertexAccessType = draw->index_size > 0 ? RANDOM : SEQUENTIAL;
      prim.PredicateEnable = use_predicate;
#if GFX_VERx10 >= 125
      prim.TBIMREnable = ice->state.use_tbimr;
#endif
      if (indirect) {
         prim.IndirectParameterEnable = true;
      } else {
         prim.StartInstanceLocation = draw->start_instance;
         prim.InstanceCount = draw->instance_count;
         prim.VertexCountPerInstance = sc->count;

         prim.StartVertexLocation = sc->start;

         if (draw->index_size) {
            prim.BaseVertexLocation += sc->index_bias;
         }
      }
   }

   genX(emit_3dprimitive_was)(batch, indirect, ice->state.prim_mode, sc->count);
   genX(maybe_emit_breakpoint)(batch, false);

   iris_batch_sync_region_end(batch);

   uint32_t count = (sc) ? sc->count : 0;
   count *= draw->instance_count ? draw->instance_count : 1;
   trace_intel_end_draw(&batch->trace, count);
}

static void
iris_upload_indirect_render_state(struct iris_context *ice,
                                  const struct pipe_draw_info *draw,
                                  const struct pipe_draw_indirect_info *indirect,
                                  const struct pipe_draw_start_count_bias *sc)
{
#if GFX_VERx10 >= 125
   assert(indirect);

   struct iris_batch *batch = &ice->batches[IRIS_BATCH_RENDER];
   UNUSED struct iris_screen *screen = batch->screen;
   UNUSED const struct intel_device_info *devinfo = screen->devinfo;
   const bool use_predicate =
      ice->state.predicate == IRIS_PREDICATE_STATE_USE_BIT;

   trace_intel_begin_draw(&batch->trace);

   if (ice->state.dirty & IRIS_DIRTY_VERTEX_BUFFER_FLUSHES)
      flush_vbos(ice, batch);

   iris_batch_sync_region_start(batch);

   /* Always pin the binder.  If we're emitting new binding table pointers,
    * we need it.  If not, we're probably inheriting old tables via the
    * context, and need it anyway.  Since true zero-bindings cases are
    * practically non-existent, just pin it and avoid last_res tracking.
    */
   iris_use_pinned_bo(batch, ice->state.binder.bo, false,
                      IRIS_DOMAIN_NONE);

   if (!batch->contains_draw) {
      /* Re-emit constants when starting a new batch buffer in order to
       * work around push constant corruption on context switch.
       *
       * XXX - Provide hardware spec quotation when available.
       */
      ice->state.stage_dirty |= (IRIS_STAGE_DIRTY_CONSTANTS_VS  |
                                 IRIS_STAGE_DIRTY_CONSTANTS_TCS |
                                 IRIS_STAGE_DIRTY_CONSTANTS_TES |
                                 IRIS_STAGE_DIRTY_CONSTANTS_GS  |
                                 IRIS_STAGE_DIRTY_CONSTANTS_FS);
      batch->contains_draw = true;
   }

   if (!batch->contains_draw_with_next_seqno) {
      iris_restore_render_saved_bos(ice, batch, draw);
      batch->contains_draw_with_next_seqno = true;
   }

   /* Wa_1306463417 - Send HS state for every primitive on gfx11.
    * Wa_16011107343 (same for gfx12)
    * We implement this by setting TCS dirty on each draw.
    */
   if ((INTEL_NEEDS_WA_1306463417 || INTEL_NEEDS_WA_16011107343) &&
       ice->shaders.prog[MESA_SHADER_TESS_CTRL]) {
      ice->state.stage_dirty |= IRIS_STAGE_DIRTY_TCS;
   }

   iris_upload_dirty_render_state(ice, batch, draw);

   if (draw->index_size > 0) {
      unsigned offset;

      if (draw->has_user_indices) {
         unsigned start_offset = draw->index_size * sc->start;

         u_upload_data(ice->ctx.const_uploader, start_offset,
                       sc->count * draw->index_size, 4,
                       (char*)draw->index.user + start_offset,
                       &offset, &ice->state.last_res.index_buffer);
         offset -= start_offset;
      } else {
         struct iris_resource *res = (void *) draw->index.resource;
         res->bind_history |= PIPE_BIND_INDEX_BUFFER;

         pipe_resource_reference(&ice->state.last_res.index_buffer,
                                 draw->index.resource);
         offset = 0;

         iris_emit_buffer_barrier_for(batch, res->bo, IRIS_DOMAIN_VF_READ);
      }

      struct iris_genx_state *genx = ice->state.genx;
      struct iris_bo *bo = iris_resource_bo(ice->state.last_res.index_buffer);

      uint32_t ib_packet[GENX(3DSTATE_INDEX_BUFFER_length)];
      iris_pack_command(GENX(3DSTATE_INDEX_BUFFER), ib_packet, ib) {
         ib.IndexFormat = draw->index_size >> 1;
         ib.MOCS = iris_mocs(bo, &batch->screen->isl_dev,
                             ISL_SURF_USAGE_INDEX_BUFFER_BIT);
         ib.BufferSize = bo->size - offset;
         ib.BufferStartingAddress = ro_bo(NULL, bo->address + offset);
         ib.L3BypassDisable       = true;
      }

      if (memcmp(genx->last_index_buffer, ib_packet, sizeof(ib_packet)) != 0) {
          memcpy(genx->last_index_buffer, ib_packet, sizeof(ib_packet));
         iris_batch_emit(batch, ib_packet, sizeof(ib_packet));
         iris_use_pinned_bo(batch, bo, false, IRIS_DOMAIN_VF_READ);
      }

   }

   iris_measure_snapshot(ice, batch, INTEL_SNAPSHOT_DRAW, draw, indirect, sc);

   genX(maybe_emit_breakpoint)(batch, true);

   iris_emit_cmd(batch, GENX(EXECUTE_INDIRECT_DRAW), ind) {
      ind.ArgumentFormat             =
         draw->index_size > 0 ? DRAWINDEXED : DRAW;
      ind.PredicateEnable            = use_predicate;
      ind.TBIMREnabled               = ice->state.use_tbimr;
      ind.MaxCount                   = indirect->draw_count;

      if (indirect->buffer) {
         struct iris_bo *bo = iris_resource_bo(indirect->buffer);
         ind.ArgumentBufferStartAddress = ro_bo(bo, indirect->offset);
         ind.MOCS = iris_mocs(bo, &screen->isl_dev, 0);
         } else {
         ind.MOCS = iris_mocs(NULL, &screen->isl_dev, 0);
      }

      if (indirect->indirect_draw_count) {
         struct iris_bo *draw_count_bo      =
            iris_resource_bo(indirect->indirect_draw_count);
         ind.CountBufferIndirectEnable      = true;
         ind.CountBufferAddress             =
            ro_bo(draw_count_bo, indirect->indirect_draw_count_offset);
      }
   }

   genX(emit_3dprimitive_was)(batch, indirect, ice->state.prim_mode, sc->count);
   genX(maybe_emit_breakpoint)(batch, false);

   iris_batch_sync_region_end(batch);

   uint32_t count = (sc) ? sc->count : 0;
   count *= draw->instance_count ? draw->instance_count : 1;
   trace_intel_end_draw(&batch->trace, count);
#else
   unreachable("Unsupported path");
#endif /* GFX_VERx10 >= 125 */
}

static void
iris_load_indirect_location(struct iris_context *ice,
                            struct iris_batch *batch,
                            const struct pipe_grid_info *grid)
{
#define GPGPU_DISPATCHDIMX 0x2500
#define GPGPU_DISPATCHDIMY 0x2504
#define GPGPU_DISPATCHDIMZ 0x2508

   assert(grid->indirect);

   struct iris_state_ref *grid_size = &ice->state.grid_size;
   struct iris_bo *bo = iris_resource_bo(grid_size->res);
   struct mi_builder b;
   mi_builder_init(&b, batch->screen->devinfo, batch);
   struct mi_value size_x = mi_mem32(ro_bo(bo, grid_size->offset + 0));
   struct mi_value size_y = mi_mem32(ro_bo(bo, grid_size->offset + 4));
   struct mi_value size_z = mi_mem32(ro_bo(bo, grid_size->offset + 8));
   mi_store(&b, mi_reg32(GPGPU_DISPATCHDIMX), size_x);
   mi_store(&b, mi_reg32(GPGPU_DISPATCHDIMY), size_y);
   mi_store(&b, mi_reg32(GPGPU_DISPATCHDIMZ), size_z);
}

static bool iris_emit_indirect_dispatch_supported(const struct intel_device_info *devinfo)
{
   // TODO: Swizzling X and Y workgroup sizes is not supported in execute indirect dispatch
   return devinfo->has_indirect_unroll;
}

#if GFX_VERx10 >= 125

static void iris_emit_execute_indirect_dispatch(struct iris_context *ice,
                                                struct iris_batch *batch,
                                                const struct pipe_grid_info *grid,
                                                const struct GENX(INTERFACE_DESCRIPTOR_DATA) idd)
{
   const struct iris_screen *screen = batch->screen;
   const struct intel_device_info *devinfo = screen->devinfo;
   struct iris_compiled_shader *shader =
      ice->shaders.prog[MESA_SHADER_COMPUTE];
   struct brw_stage_prog_data *prog_data = shader->prog_data;
   struct brw_cs_prog_data *cs_prog_data = (void *) prog_data;
   const struct brw_cs_dispatch_info dispatch =
      brw_cs_get_dispatch_info(devinfo, cs_prog_data, grid->block);
   struct iris_bo *indirect = iris_resource_bo(grid->indirect);
   const int dispatch_size = dispatch.simd_size / 16;

   struct GENX(COMPUTE_WALKER_BODY) body = {};
   body.SIMDSize            = dispatch_size;
   body.MessageSIMD         = dispatch_size;
   body.LocalXMaximum       = grid->block[0] - 1;
   body.LocalYMaximum       = grid->block[1] - 1;
   body.LocalZMaximum       = grid->block[2] - 1;
   body.ExecutionMask       = dispatch.right_mask;
   body.PostSync.MOCS       = iris_mocs(NULL, &screen->isl_dev, 0);
   body.InterfaceDescriptor = idd;

   struct iris_address indirect_bo = ro_bo(indirect, grid->indirect_offset);
   iris_emit_cmd(batch, GENX(EXECUTE_INDIRECT_DISPATCH), ind) {
      ind.PredicateEnable            =
         ice->state.predicate == IRIS_PREDICATE_STATE_USE_BIT;
      ind.MaxCount                   = 1;
      ind.COMPUTE_WALKER_BODY        = body;
      ind.ArgumentBufferStartAddress = indirect_bo;
      ind.MOCS                       =
         iris_mocs(indirect_bo.bo, &screen->isl_dev, 0);
   }
}

static void
iris_upload_compute_walker(struct iris_context *ice,
                           struct iris_batch *batch,
                           const struct pipe_grid_info *grid)
{
   const uint64_t stage_dirty = ice->state.stage_dirty;
   struct iris_screen *screen = batch->screen;
   const struct intel_device_info *devinfo = screen->devinfo;
   struct iris_binder *binder = &ice->state.binder;
   struct iris_shader_state *shs = &ice->state.shaders[MESA_SHADER_COMPUTE];
   struct iris_compiled_shader *shader =
      ice->shaders.prog[MESA_SHADER_COMPUTE];
   struct brw_stage_prog_data *prog_data = shader->prog_data;
   struct brw_cs_prog_data *cs_prog_data = (void *) prog_data;
   const struct brw_cs_dispatch_info dispatch =
      brw_cs_get_dispatch_info(devinfo, cs_prog_data, grid->block);

   trace_intel_begin_compute(&batch->trace);

   if (stage_dirty & IRIS_STAGE_DIRTY_CS) {
      iris_emit_cmd(batch, GENX(CFE_STATE), cfe) {
         cfe.MaximumNumberofThreads =
            devinfo->max_cs_threads * devinfo->subslice_total;
         uint32_t scratch_addr = pin_scratch_space(ice, batch, prog_data,
                                                   MESA_SHADER_COMPUTE);
         cfe.ScratchSpaceBuffer = scratch_addr >> 4;
      }
   }

   struct GENX(INTERFACE_DESCRIPTOR_DATA) idd = {};
   idd.KernelStartPointer = KSP(shader);
   idd.NumberofThreadsinGPGPUThreadGroup = dispatch.threads;
   idd.SharedLocalMemorySize =
      encode_slm_size(GFX_VER, prog_data->total_shared);
   idd.SamplerStatePointer = shs->sampler_table.offset;
   idd.SamplerCount = encode_sampler_count(shader),
   idd.BindingTablePointer = binder->bt_offset[MESA_SHADER_COMPUTE];
   /* Typically set to 0 to avoid prefetching on every thread dispatch. */
   idd.BindingTableEntryCount = devinfo->verx10 == 125 ?
      0 : MIN2(shader->bt.size_bytes / 4, 31);
   idd.PreferredSLMAllocationSize = preferred_slm_allocation_size(devinfo);
   idd.NumberOfBarriers = cs_prog_data->uses_barrier;

   iris_measure_snapshot(ice, batch, INTEL_SNAPSHOT_COMPUTE, NULL, NULL, NULL);

   if (iris_emit_indirect_dispatch_supported(devinfo) && grid->indirect) {
      iris_emit_execute_indirect_dispatch(ice, batch, grid, idd);
   } else {
      if (grid->indirect)
         iris_load_indirect_location(ice, batch, grid);

      iris_measure_snapshot(ice, batch, INTEL_SNAPSHOT_COMPUTE, NULL, NULL, NULL);

      ice->utrace.last_compute_walker =
         iris_emit_dwords(batch, GENX(COMPUTE_WALKER_length));
      _iris_pack_command(batch, GENX(COMPUTE_WALKER),
                         ice->utrace.last_compute_walker, cw) {
         cw.IndirectParameterEnable        = grid->indirect;
         cw.SIMDSize                       = dispatch.simd_size / 16;
         cw.MessageSIMD                    = dispatch.simd_size / 16;
         cw.LocalXMaximum                  = grid->block[0] - 1;
         cw.LocalYMaximum                  = grid->block[1] - 1;
         cw.LocalZMaximum                  = grid->block[2] - 1;
         cw.ThreadGroupIDXDimension        = grid->grid[0];
         cw.ThreadGroupIDYDimension        = grid->grid[1];
         cw.ThreadGroupIDZDimension        = grid->grid[2];
         cw.ExecutionMask                  = dispatch.right_mask;
         cw.PostSync.MOCS                  = iris_mocs(NULL, &screen->isl_dev, 0);
         cw.InterfaceDescriptor            = idd;

         assert(brw_cs_push_const_total_size(cs_prog_data, dispatch.threads) == 0);
      }
   }

   trace_intel_end_compute(&batch->trace, grid->grid[0], grid->grid[1], grid->grid[2]);
}

#else /* #if GFX_VERx10 >= 125 */

static void
iris_upload_gpgpu_walker(struct iris_context *ice,
                         struct iris_batch *batch,
                         const struct pipe_grid_info *grid)
{
   const uint64_t stage_dirty = ice->state.stage_dirty;
   struct iris_screen *screen = batch->screen;
   const struct intel_device_info *devinfo = screen->devinfo;
   struct iris_binder *binder = &ice->state.binder;
   struct iris_shader_state *shs = &ice->state.shaders[MESA_SHADER_COMPUTE];
   struct iris_uncompiled_shader *ish =
      ice->shaders.uncompiled[MESA_SHADER_COMPUTE];
   struct iris_compiled_shader *shader =
      ice->shaders.prog[MESA_SHADER_COMPUTE];
   struct brw_stage_prog_data *prog_data = shader->prog_data;
   struct brw_cs_prog_data *cs_prog_data = (void *) prog_data;
   const struct brw_cs_dispatch_info dispatch =
      brw_cs_get_dispatch_info(devinfo, cs_prog_data, grid->block);

   trace_intel_begin_compute(&batch->trace);

   if ((stage_dirty & IRIS_STAGE_DIRTY_CS) ||
       cs_prog_data->local_size[0] == 0 /* Variable local group size */) {
      /* The MEDIA_VFE_STATE documentation for Gfx8+ says:
       *
       *   "A stalling PIPE_CONTROL is required before MEDIA_VFE_STATE unless
       *    the only bits that are changed are scoreboard related: Scoreboard
       *    Enable, Scoreboard Type, Scoreboard Mask, Scoreboard Delta.  For
       *    these scoreboard related states, a MEDIA_STATE_FLUSH is
       *    sufficient."
       */
      iris_emit_pipe_control_flush(batch,
                                   "workaround: stall before MEDIA_VFE_STATE",
                                   PIPE_CONTROL_CS_STALL);

      iris_emit_cmd(batch, GENX(MEDIA_VFE_STATE), vfe) {
         if (prog_data->total_scratch) {
            uint32_t scratch_addr =
               pin_scratch_space(ice, batch, prog_data, MESA_SHADER_COMPUTE);

            vfe.PerThreadScratchSpace = ffs(prog_data->total_scratch) - 11;
            vfe.ScratchSpaceBasePointer =
               rw_bo(NULL, scratch_addr, IRIS_DOMAIN_NONE);
         }

         vfe.MaximumNumberofThreads =
            devinfo->max_cs_threads * devinfo->subslice_total - 1;
#if GFX_VER < 11
         vfe.ResetGatewayTimer =
            Resettingrelativetimerandlatchingtheglobaltimestamp;
#endif
#if GFX_VER == 8
         vfe.BypassGatewayControl = true;
#endif
         vfe.NumberofURBEntries = 2;
         vfe.URBEntryAllocationSize = 2;

         vfe.CURBEAllocationSize =
            ALIGN(cs_prog_data->push.per_thread.regs * dispatch.threads +
                  cs_prog_data->push.cross_thread.regs, 2);
      }
   }

   /* TODO: Combine subgroup-id with cbuf0 so we can push regular uniforms */
   if ((stage_dirty & IRIS_STAGE_DIRTY_CS) ||
       cs_prog_data->local_size[0] == 0 /* Variable local group size */) {
      uint32_t curbe_data_offset = 0;
      assert(cs_prog_data->push.cross_thread.dwords == 0 &&
             cs_prog_data->push.per_thread.dwords == 1 &&
             cs_prog_data->base.param[0] == BRW_PARAM_BUILTIN_SUBGROUP_ID);
      const unsigned push_const_size =
         brw_cs_push_const_total_size(cs_prog_data, dispatch.threads);
      uint32_t *curbe_data_map =
         stream_state(batch, ice->state.dynamic_uploader,
                      &ice->state.last_res.cs_thread_ids,
                      ALIGN(push_const_size, 64), 64,
                      &curbe_data_offset);
      assert(curbe_data_map);
      memset(curbe_data_map, 0x5a, ALIGN(push_const_size, 64));
      iris_fill_cs_push_const_buffer(cs_prog_data, dispatch.threads,
                                     curbe_data_map);

      iris_emit_cmd(batch, GENX(MEDIA_CURBE_LOAD), curbe) {
         curbe.CURBETotalDataLength = ALIGN(push_const_size, 64);
         curbe.CURBEDataStartAddress = curbe_data_offset;
      }
   }

   for (unsigned i = 0; i < IRIS_MAX_GLOBAL_BINDINGS; i++) {
      struct pipe_resource *res = ice->state.global_bindings[i];
      if (!res)
         break;

      iris_use_pinned_bo(batch, iris_resource_bo(res),
                         true, IRIS_DOMAIN_NONE);
   }

   if (stage_dirty & (IRIS_STAGE_DIRTY_SAMPLER_STATES_CS |
                      IRIS_STAGE_DIRTY_BINDINGS_CS |
                      IRIS_STAGE_DIRTY_CONSTANTS_CS |
                      IRIS_STAGE_DIRTY_CS)) {
      uint32_t desc[GENX(INTERFACE_DESCRIPTOR_DATA_length)];

      iris_pack_state(GENX(INTERFACE_DESCRIPTOR_DATA), desc, idd) {
         idd.SharedLocalMemorySize =
            encode_slm_size(GFX_VER, ish->kernel_shared_size + grid->variable_shared_mem);
         idd.KernelStartPointer =
            KSP(shader) + brw_cs_prog_data_prog_offset(cs_prog_data,
                                                       dispatch.simd_size);
         idd.SamplerStatePointer = shs->sampler_table.offset;
         idd.BindingTablePointer =
            binder->bt_offset[MESA_SHADER_COMPUTE] >> IRIS_BT_OFFSET_SHIFT;
         idd.NumberofThreadsinGPGPUThreadGroup = dispatch.threads;
      }

      for (int i = 0; i < GENX(INTERFACE_DESCRIPTOR_DATA_length); i++)
         desc[i] |= ((uint32_t *) shader->derived_data)[i];

      iris_emit_cmd(batch, GENX(MEDIA_INTERFACE_DESCRIPTOR_LOAD), load) {
         load.InterfaceDescriptorTotalLength =
            GENX(INTERFACE_DESCRIPTOR_DATA_length) * sizeof(uint32_t);
         load.InterfaceDescriptorDataStartAddress =
            emit_state(batch, ice->state.dynamic_uploader,
                       &ice->state.last_res.cs_desc, desc, sizeof(desc), 64);
      }
   }

   if (grid->indirect)
      iris_load_indirect_location(ice, batch, grid);

   iris_measure_snapshot(ice, batch, INTEL_SNAPSHOT_COMPUTE, NULL, NULL, NULL);

   iris_emit_cmd(batch, GENX(GPGPU_WALKER), ggw) {
      ggw.IndirectParameterEnable    = grid->indirect != NULL;
      ggw.SIMDSize                   = dispatch.simd_size / 16;
      ggw.ThreadDepthCounterMaximum  = 0;
      ggw.ThreadHeightCounterMaximum = 0;
      ggw.ThreadWidthCounterMaximum  = dispatch.threads - 1;
      ggw.ThreadGroupIDXDimension    = grid->grid[0];
      ggw.ThreadGroupIDYDimension    = grid->grid[1];
      ggw.ThreadGroupIDZDimension    = grid->grid[2];
      ggw.RightExecutionMask         = dispatch.right_mask;
      ggw.BottomExecutionMask        = 0xffffffff;
   }

   iris_emit_cmd(batch, GENX(MEDIA_STATE_FLUSH), msf);

   trace_intel_end_compute(&batch->trace, grid->grid[0], grid->grid[1], grid->grid[2]);
}

#endif /* #if GFX_VERx10 >= 125 */

static void
iris_upload_compute_state(struct iris_context *ice,
                          struct iris_batch *batch,
                          const struct pipe_grid_info *grid)
{
   struct iris_screen *screen = batch->screen;
   const uint64_t stage_dirty = ice->state.stage_dirty;
   struct iris_shader_state *shs = &ice->state.shaders[MESA_SHADER_COMPUTE];
   struct iris_compiled_shader *shader =
      ice->shaders.prog[MESA_SHADER_COMPUTE];
   struct iris_border_color_pool *border_color_pool =
      iris_bufmgr_get_border_color_pool(screen->bufmgr);

   iris_batch_sync_region_start(batch);

   /* Always pin the binder.  If we're emitting new binding table pointers,
    * we need it.  If not, we're probably inheriting old tables via the
    * context, and need it anyway.  Since true zero-bindings cases are
    * practically non-existent, just pin it and avoid last_res tracking.
    */
   iris_use_pinned_bo(batch, ice->state.binder.bo, false, IRIS_DOMAIN_NONE);

   if (((stage_dirty & IRIS_STAGE_DIRTY_CONSTANTS_CS) &&
        shs->sysvals_need_upload) ||
       shader->kernel_input_size > 0)
      upload_sysvals(ice, MESA_SHADER_COMPUTE, grid);

   if (stage_dirty & IRIS_STAGE_DIRTY_BINDINGS_CS)
      iris_populate_binding_table(ice, batch, MESA_SHADER_COMPUTE, false);

   if (stage_dirty & IRIS_STAGE_DIRTY_SAMPLER_STATES_CS)
      iris_upload_sampler_states(ice, MESA_SHADER_COMPUTE);

   iris_use_optional_res(batch, shs->sampler_table.res, false,
                         IRIS_DOMAIN_NONE);
   iris_use_pinned_bo(batch, iris_resource_bo(shader->assembly.res), false,
                      IRIS_DOMAIN_NONE);

   if (ice->state.need_border_colors)
      iris_use_pinned_bo(batch, border_color_pool->bo, false,
                         IRIS_DOMAIN_NONE);

#if GFX_VER >= 12
   genX(invalidate_aux_map_state)(batch);
#endif

#if GFX_VERx10 >= 125
   iris_upload_compute_walker(ice, batch, grid);
#else
   iris_upload_gpgpu_walker(ice, batch, grid);
#endif

   if (!batch->contains_draw_with_next_seqno) {
      iris_restore_compute_saved_bos(ice, batch, grid);
      batch->contains_draw_with_next_seqno = batch->contains_draw = true;
   }

   iris_batch_sync_region_end(batch);
}

/**
 * State module teardown.
 */
static void
iris_destroy_state(struct iris_context *ice)
{
   struct iris_genx_state *genx = ice->state.genx;

   pipe_resource_reference(&ice->state.pixel_hashing_tables, NULL);

   pipe_resource_reference(&ice->draw.draw_params.res, NULL);
   pipe_resource_reference(&ice->draw.derived_draw_params.res, NULL);

   /* Loop over all VBOs, including ones for draw parameters */
   for (unsigned i = 0; i < ARRAY_SIZE(genx->vertex_buffers); i++) {
      pipe_resource_reference(&genx->vertex_buffers[i].resource, NULL);
   }

   free(ice->state.genx);

   for (int i = 0; i < 4; i++) {
      pipe_so_target_reference(&ice->state.so_target[i], NULL);
   }

   util_unreference_framebuffer_state(&ice->state.framebuffer);

   for (int stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      struct iris_shader_state *shs = &ice->state.shaders[stage];
      pipe_resource_reference(&shs->sampler_table.res, NULL);
      for (int i = 0; i < PIPE_MAX_CONSTANT_BUFFERS; i++) {
         pipe_resource_reference(&shs->constbuf[i].buffer, NULL);
         pipe_resource_reference(&shs->constbuf_surf_state[i].res, NULL);
      }
      for (int i = 0; i < PIPE_MAX_SHADER_IMAGES; i++) {
         pipe_resource_reference(&shs->image[i].base.resource, NULL);
         pipe_resource_reference(&shs->image[i].surface_state.ref.res, NULL);
         free(shs->image[i].surface_state.cpu);
      }
      for (int i = 0; i < PIPE_MAX_SHADER_BUFFERS; i++) {
         pipe_resource_reference(&shs->ssbo[i].buffer, NULL);
         pipe_resource_reference(&shs->ssbo_surf_state[i].res, NULL);
      }
      for (int i = 0; i < IRIS_MAX_TEXTURES; i++) {
         pipe_sampler_view_reference((struct pipe_sampler_view **)
                                     &shs->textures[i], NULL);
      }
   }

   pipe_resource_reference(&ice->state.grid_size.res, NULL);
   pipe_resource_reference(&ice->state.grid_surf_state.res, NULL);

   pipe_resource_reference(&ice->state.null_fb.res, NULL);
   pipe_resource_reference(&ice->state.unbound_tex.res, NULL);

   pipe_resource_reference(&ice->state.last_res.cc_vp, NULL);
   pipe_resource_reference(&ice->state.last_res.sf_cl_vp, NULL);
   pipe_resource_reference(&ice->state.last_res.color_calc, NULL);
   pipe_resource_reference(&ice->state.last_res.scissor, NULL);
   pipe_resource_reference(&ice->state.last_res.blend, NULL);
   pipe_resource_reference(&ice->state.last_res.index_buffer, NULL);
   pipe_resource_reference(&ice->state.last_res.cs_thread_ids, NULL);
   pipe_resource_reference(&ice->state.last_res.cs_desc, NULL);
}

/* ------------------------------------------------------------------- */

static void
iris_rebind_buffer(struct iris_context *ice,
                   struct iris_resource *res)
{
   struct pipe_context *ctx = &ice->ctx;
   struct iris_genx_state *genx = ice->state.genx;

   assert(res->base.b.target == PIPE_BUFFER);

   /* Buffers can't be framebuffer attachments, nor display related,
    * and we don't have upstream Clover support.
    */
   assert(!(res->bind_history & (PIPE_BIND_DEPTH_STENCIL |
                                 PIPE_BIND_RENDER_TARGET |
                                 PIPE_BIND_BLENDABLE |
                                 PIPE_BIND_DISPLAY_TARGET |
                                 PIPE_BIND_CURSOR |
                                 PIPE_BIND_COMPUTE_RESOURCE |
                                 PIPE_BIND_GLOBAL)));

   if (res->bind_history & PIPE_BIND_VERTEX_BUFFER) {
      uint64_t bound_vbs = ice->state.bound_vertex_buffers;
      while (bound_vbs) {
         const int i = u_bit_scan64(&bound_vbs);
         struct iris_vertex_buffer_state *state = &genx->vertex_buffers[i];

         /* Update the CPU struct */
         STATIC_ASSERT(GENX(VERTEX_BUFFER_STATE_BufferStartingAddress_start) == 32);
         STATIC_ASSERT(GENX(VERTEX_BUFFER_STATE_BufferStartingAddress_bits) == 64);
         uint64_t *addr = (uint64_t *) &state->state[1];
         struct iris_bo *bo = iris_resource_bo(state->resource);

         if (*addr != bo->address + state->offset) {
            *addr = bo->address + state->offset;
            ice->state.dirty |= IRIS_DIRTY_VERTEX_BUFFERS |
                                IRIS_DIRTY_VERTEX_BUFFER_FLUSHES;
         }
      }
   }

   /* We don't need to handle PIPE_BIND_INDEX_BUFFER here: we re-emit
    * the 3DSTATE_INDEX_BUFFER packet whenever the address changes.
    *
    * There is also no need to handle these:
    * - PIPE_BIND_COMMAND_ARGS_BUFFER (emitted for every indirect draw)
    * - PIPE_BIND_QUERY_BUFFER (no persistent state references)
    */

   if (res->bind_history & PIPE_BIND_STREAM_OUTPUT) {
      uint32_t *so_buffers = genx->so_buffers;
      for (unsigned i = 0; i < 4; i++,
           so_buffers += GENX(3DSTATE_SO_BUFFER_length)) {

         /* There are no other fields in bits 127:64 */
         uint64_t *addr = (uint64_t *) &so_buffers[2];
         STATIC_ASSERT(GENX(3DSTATE_SO_BUFFER_SurfaceBaseAddress_start) == 66);
         STATIC_ASSERT(GENX(3DSTATE_SO_BUFFER_SurfaceBaseAddress_bits) == 46);

         struct pipe_stream_output_target *tgt = ice->state.so_target[i];
         if (tgt) {
            struct iris_bo *bo = iris_resource_bo(tgt->buffer);
            if (*addr != bo->address + tgt->buffer_offset) {
               *addr = bo->address + tgt->buffer_offset;
               ice->state.dirty |= IRIS_DIRTY_SO_BUFFERS;
            }
         }
      }
   }

   for (int s = MESA_SHADER_VERTEX; s < MESA_SHADER_STAGES; s++) {
      struct iris_shader_state *shs = &ice->state.shaders[s];
      enum pipe_shader_type p_stage = stage_to_pipe(s);

      if (!(res->bind_stages & (1 << s)))
         continue;

      if (res->bind_history & PIPE_BIND_CONSTANT_BUFFER) {
         /* Skip constant buffer 0, it's for regular uniforms, not UBOs */
         uint32_t bound_cbufs = shs->bound_cbufs & ~1u;
         while (bound_cbufs) {
            const int i = u_bit_scan(&bound_cbufs);
            struct pipe_shader_buffer *cbuf = &shs->constbuf[i];
            struct iris_state_ref *surf_state = &shs->constbuf_surf_state[i];

            if (res->bo == iris_resource_bo(cbuf->buffer)) {
               pipe_resource_reference(&surf_state->res, NULL);
               shs->dirty_cbufs |= 1u << i;
               ice->state.dirty |= (IRIS_DIRTY_RENDER_MISC_BUFFER_FLUSHES |
                                    IRIS_DIRTY_COMPUTE_MISC_BUFFER_FLUSHES);
               ice->state.stage_dirty |= IRIS_STAGE_DIRTY_CONSTANTS_VS << s;
            }
         }
      }

      if (res->bind_history & PIPE_BIND_SHADER_BUFFER) {
         uint32_t bound_ssbos = shs->bound_ssbos;
         while (bound_ssbos) {
            const int i = u_bit_scan(&bound_ssbos);
            struct pipe_shader_buffer *ssbo = &shs->ssbo[i];

            if (res->bo == iris_resource_bo(ssbo->buffer)) {
               struct pipe_shader_buffer buf = {
                  .buffer = &res->base.b,
                  .buffer_offset = ssbo->buffer_offset,
                  .buffer_size = ssbo->buffer_size,
               };
               iris_set_shader_buffers(ctx, p_stage, i, 1, &buf,
                                       (shs->writable_ssbos >> i) & 1);
            }
         }
      }

      if (res->bind_history & PIPE_BIND_SAMPLER_VIEW) {
         int i;
         BITSET_FOREACH_SET(i, shs->bound_sampler_views, IRIS_MAX_TEXTURES) {
            struct iris_sampler_view *isv = shs->textures[i];
            struct iris_bo *bo = isv->res->bo;

            if (update_surface_state_addrs(ice->state.surface_uploader,
                                           &isv->surface_state, bo)) {
               ice->state.stage_dirty |= IRIS_STAGE_DIRTY_BINDINGS_VS << s;
            }
         }
      }

      if (res->bind_history & PIPE_BIND_SHADER_IMAGE) {
         uint64_t bound_image_views = shs->bound_image_views;
         while (bound_image_views) {
            const int i = u_bit_scan64(&bound_image_views);
            struct iris_image_view *iv = &shs->image[i];
            struct iris_bo *bo = iris_resource_bo(iv->base.resource);

            if (update_surface_state_addrs(ice->state.surface_uploader,
                                           &iv->surface_state, bo)) {
               ice->state.stage_dirty |= IRIS_STAGE_DIRTY_BINDINGS_VS << s;
            }
         }
      }
   }
}

/* ------------------------------------------------------------------- */

/**
 * Introduce a batch synchronization boundary, and update its cache coherency
 * status to reflect the execution of a PIPE_CONTROL command with the
 * specified flags.
 */
static void
batch_mark_sync_for_pipe_control(struct iris_batch *batch, uint32_t flags)
{
   const struct intel_device_info *devinfo = batch->screen->devinfo;

   iris_batch_sync_boundary(batch);

   if ((flags & PIPE_CONTROL_CS_STALL)) {
      if ((flags & PIPE_CONTROL_RENDER_TARGET_FLUSH))
         iris_batch_mark_flush_sync(batch, IRIS_DOMAIN_RENDER_WRITE);

      if ((flags & PIPE_CONTROL_DEPTH_CACHE_FLUSH))
         iris_batch_mark_flush_sync(batch, IRIS_DOMAIN_DEPTH_WRITE);

      if ((flags & PIPE_CONTROL_TILE_CACHE_FLUSH)) {
         /* A tile cache flush makes any C/Z data in L3 visible to memory. */
         const unsigned c = IRIS_DOMAIN_RENDER_WRITE;
         const unsigned z = IRIS_DOMAIN_DEPTH_WRITE;
         batch->coherent_seqnos[c][c] = batch->l3_coherent_seqnos[c];
         batch->coherent_seqnos[z][z] = batch->l3_coherent_seqnos[z];
      }

      if (flags & (PIPE_CONTROL_FLUSH_HDC | PIPE_CONTROL_DATA_CACHE_FLUSH)) {
         /* HDC and DC flushes both flush the data cache out to L3 */
         iris_batch_mark_flush_sync(batch, IRIS_DOMAIN_DATA_WRITE);
      }

      if ((flags & PIPE_CONTROL_DATA_CACHE_FLUSH)) {
         /* A DC flush also flushes L3 data cache lines out to memory. */
         const unsigned i = IRIS_DOMAIN_DATA_WRITE;
         batch->coherent_seqnos[i][i] = batch->l3_coherent_seqnos[i];
      }

      if ((flags & PIPE_CONTROL_FLUSH_ENABLE))
         iris_batch_mark_flush_sync(batch, IRIS_DOMAIN_OTHER_WRITE);

      if ((flags & (PIPE_CONTROL_CACHE_FLUSH_BITS |
                    PIPE_CONTROL_STALL_AT_SCOREBOARD))) {
         iris_batch_mark_flush_sync(batch, IRIS_DOMAIN_VF_READ);
         iris_batch_mark_flush_sync(batch, IRIS_DOMAIN_SAMPLER_READ);
         iris_batch_mark_flush_sync(batch, IRIS_DOMAIN_PULL_CONSTANT_READ);
         iris_batch_mark_flush_sync(batch, IRIS_DOMAIN_OTHER_READ);
      }
   }

   if ((flags & PIPE_CONTROL_RENDER_TARGET_FLUSH))
      iris_batch_mark_invalidate_sync(batch, IRIS_DOMAIN_RENDER_WRITE);

   if ((flags & PIPE_CONTROL_DEPTH_CACHE_FLUSH))
      iris_batch_mark_invalidate_sync(batch, IRIS_DOMAIN_DEPTH_WRITE);

   if (flags & (PIPE_CONTROL_FLUSH_HDC | PIPE_CONTROL_DATA_CACHE_FLUSH))
      iris_batch_mark_invalidate_sync(batch, IRIS_DOMAIN_DATA_WRITE);

   if ((flags & PIPE_CONTROL_FLUSH_ENABLE))
      iris_batch_mark_invalidate_sync(batch, IRIS_DOMAIN_OTHER_WRITE);

   if ((flags & PIPE_CONTROL_VF_CACHE_INVALIDATE))
      iris_batch_mark_invalidate_sync(batch, IRIS_DOMAIN_VF_READ);

   if ((flags & PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE))
      iris_batch_mark_invalidate_sync(batch, IRIS_DOMAIN_SAMPLER_READ);

   /* Technically, to invalidate IRIS_DOMAIN_PULL_CONSTANT_READ, we need
    * both "Constant Cache Invalidate" and either "Texture Cache Invalidate"
    * or "Data Cache Flush" set, depending on the setting of
    * compiler->indirect_ubos_use_sampler.
    *
    * However, "Data Cache Flush" and "Constant Cache Invalidate" will never
    * appear in the same PIPE_CONTROL command, because one is bottom-of-pipe
    * while the other is top-of-pipe.  Because we only look at one flush at
    * a time, we won't see both together.
    *
    * To deal with this, we mark it as invalidated when the constant cache
    * is invalidated, and trust the callers to also flush the other related
    * cache correctly at the same time.
    */
   if ((flags & PIPE_CONTROL_CONST_CACHE_INVALIDATE))
      iris_batch_mark_invalidate_sync(batch, IRIS_DOMAIN_PULL_CONSTANT_READ);

   /* IRIS_DOMAIN_OTHER_READ no longer uses any caches. */

   if ((flags & PIPE_CONTROL_L3_RO_INVALIDATE_BITS) == PIPE_CONTROL_L3_RO_INVALIDATE_BITS) {
      /* If we just invalidated the read-only lines of L3, then writes from non-L3-coherent
       * domains will now be visible to those L3 clients.
       */
      for (unsigned i = 0; i < NUM_IRIS_DOMAINS; i++) {
         if (!iris_domain_is_l3_coherent(devinfo, i))
            batch->l3_coherent_seqnos[i] = batch->coherent_seqnos[i][i];
      }
   }
}

static unsigned
flags_to_post_sync_op(uint32_t flags)
{
   if (flags & PIPE_CONTROL_WRITE_IMMEDIATE)
      return WriteImmediateData;

   if (flags & PIPE_CONTROL_WRITE_DEPTH_COUNT)
      return WritePSDepthCount;

   if (flags & PIPE_CONTROL_WRITE_TIMESTAMP)
      return WriteTimestamp;

   return 0;
}

/**
 * Do the given flags have a Post Sync or LRI Post Sync operation?
 */
static enum pipe_control_flags
get_post_sync_flags(enum pipe_control_flags flags)
{
   flags &= PIPE_CONTROL_WRITE_IMMEDIATE |
            PIPE_CONTROL_WRITE_DEPTH_COUNT |
            PIPE_CONTROL_WRITE_TIMESTAMP |
            PIPE_CONTROL_LRI_POST_SYNC_OP;

   /* Only one "Post Sync Op" is allowed, and it's mutually exclusive with
    * "LRI Post Sync Operation".  So more than one bit set would be illegal.
    */
   assert(util_bitcount(flags) <= 1);

   return flags;
}

#define IS_COMPUTE_PIPELINE(batch) (batch->name == IRIS_BATCH_COMPUTE)

/**
 * Emit a series of PIPE_CONTROL commands, taking into account any
 * workarounds necessary to actually accomplish the caller's request.
 *
 * Unless otherwise noted, spec quotations in this function come from:
 *
 * Synchronization of the 3D Pipeline > PIPE_CONTROL Command > Programming
 * Restrictions for PIPE_CONTROL.
 *
 * You should not use this function directly.  Use the helpers in
 * iris_pipe_control.c instead, which may split the pipe control further.
 */
static void
iris_emit_raw_pipe_control(struct iris_batch *batch,
                           const char *reason,
                           uint32_t flags,
                           struct iris_bo *bo,
                           uint32_t offset,
                           uint64_t imm)
{
   UNUSED const struct intel_device_info *devinfo = batch->screen->devinfo;
   enum pipe_control_flags post_sync_flags = get_post_sync_flags(flags);
   enum pipe_control_flags non_lri_post_sync_flags =
      post_sync_flags & ~PIPE_CONTROL_LRI_POST_SYNC_OP;

#if GFX_VER >= 12
   if (batch->name == IRIS_BATCH_BLITTER) {
      batch_mark_sync_for_pipe_control(batch, flags);
      iris_batch_sync_region_start(batch);

      assert(!(flags & PIPE_CONTROL_WRITE_DEPTH_COUNT));

      /* Wa_16018063123 - emit fast color dummy blit before MI_FLUSH_DW. */
      if (intel_needs_workaround(batch->screen->devinfo, 16018063123))
         batch_emit_fast_color_dummy_blit(batch);

      /* The blitter doesn't actually use PIPE_CONTROL; rather it uses the
       * MI_FLUSH_DW command.  However, all of our code is set up to flush
       * via emitting a pipe control, so we just translate it at this point,
       * even if it is a bit hacky.
       */
      iris_emit_cmd(batch, GENX(MI_FLUSH_DW), fd) {
         fd.Address = rw_bo(bo, offset, IRIS_DOMAIN_OTHER_WRITE);
         fd.ImmediateData = imm;
         fd.PostSyncOperation = flags_to_post_sync_op(flags);
#if GFX_VERx10 >= 125
         /* TODO: This may not always be necessary */
         fd.FlushCCS = true;
#endif
      }
      iris_batch_sync_region_end(batch);
      return;
   }
#endif

   /* The "L3 Read Only Cache Invalidation Bit" docs say it "controls the
    * invalidation of the Geometry streams cached in L3 cache at the top
    * of the pipe".  In other words, index & vertex data that gets cached
    * in L3 when VERTEX_BUFFER_STATE::L3BypassDisable is set.
    *
    * Normally, invalidating L1/L2 read-only caches also invalidate their
    * related L3 cachelines, but this isn't the case for the VF cache.
    * Emulate it by setting the L3 Read Only bit when doing a VF invalidate.
    */
   if (flags & PIPE_CONTROL_VF_CACHE_INVALIDATE)
      flags |= PIPE_CONTROL_L3_READ_ONLY_CACHE_INVALIDATE;

   /* Recursive PIPE_CONTROL workarounds --------------------------------
    * (http://knowyourmeme.com/memes/xzibit-yo-dawg)
    *
    * We do these first because we want to look at the original operation,
    * rather than any workarounds we set.
    */
   if (GFX_VER == 9 && (flags & PIPE_CONTROL_VF_CACHE_INVALIDATE)) {
      /* The PIPE_CONTROL "VF Cache Invalidation Enable" bit description
       * lists several workarounds:
       *
       *    "Project: SKL, KBL, BXT
       *
       *     If the VF Cache Invalidation Enable is set to a 1 in a
       *     PIPE_CONTROL, a separate Null PIPE_CONTROL, all bitfields
       *     sets to 0, with the VF Cache Invalidation Enable set to 0
       *     needs to be sent prior to the PIPE_CONTROL with VF Cache
       *     Invalidation Enable set to a 1."
       */
      iris_emit_raw_pipe_control(batch,
                                 "workaround: recursive VF cache invalidate",
                                 0, NULL, 0, 0);
   }

   if (GFX_VER == 9 && IS_COMPUTE_PIPELINE(batch) && post_sync_flags) {
      /* Project: SKL / Argument: LRI Post Sync Operation [23]
       *
       * "PIPECONTROL command with “Command Streamer Stall Enable” must be
       *  programmed prior to programming a PIPECONTROL command with "LRI
       *  Post Sync Operation" in GPGPU mode of operation (i.e when
       *  PIPELINE_SELECT command is set to GPGPU mode of operation)."
       *
       * The same text exists a few rows below for Post Sync Op.
       */
      iris_emit_raw_pipe_control(batch,
                                 "workaround: CS stall before gpgpu post-sync",
                                 PIPE_CONTROL_CS_STALL, bo, offset, imm);
   }

   /* "Flush Types" workarounds ---------------------------------------------
    * We do these now because they may add post-sync operations or CS stalls.
    */

   if (GFX_VER < 11 && flags & PIPE_CONTROL_VF_CACHE_INVALIDATE) {
      /* Project: BDW, SKL+ (stopping at CNL) / Argument: VF Invalidate
       *
       * "'Post Sync Operation' must be enabled to 'Write Immediate Data' or
       *  'Write PS Depth Count' or 'Write Timestamp'."
       */
      if (!bo) {
         flags |= PIPE_CONTROL_WRITE_IMMEDIATE;
         post_sync_flags |= PIPE_CONTROL_WRITE_IMMEDIATE;
         non_lri_post_sync_flags |= PIPE_CONTROL_WRITE_IMMEDIATE;
         bo = batch->screen->workaround_address.bo;
         offset = batch->screen->workaround_address.offset;
      }
   }

   if (flags & PIPE_CONTROL_DEPTH_STALL) {
      /* From the PIPE_CONTROL instruction table, bit 13 (Depth Stall Enable):
       *
       *    "This bit must be DISABLED for operations other than writing
       *     PS_DEPTH_COUNT."
       *
       * This seems like nonsense.  An Ivybridge workaround requires us to
       * emit a PIPE_CONTROL with a depth stall and write immediate post-sync
       * operation.  Gfx8+ requires us to emit depth stalls and depth cache
       * flushes together.  So, it's hard to imagine this means anything other
       * than "we originally intended this to be used for PS_DEPTH_COUNT".
       *
       * We ignore the supposed restriction and do nothing.
       */
   }

   if (flags & (PIPE_CONTROL_RENDER_TARGET_FLUSH |
                PIPE_CONTROL_STALL_AT_SCOREBOARD)) {
      /* From the PIPE_CONTROL instruction table, bit 12 and bit 1:
       *
       *    "This bit must be DISABLED for End-of-pipe (Read) fences,
       *     PS_DEPTH_COUNT or TIMESTAMP queries."
       *
       * TODO: Implement end-of-pipe checking.
       */
      assert(!(post_sync_flags & (PIPE_CONTROL_WRITE_DEPTH_COUNT |
                                  PIPE_CONTROL_WRITE_TIMESTAMP)));
   }

   if (GFX_VER < 11 && (flags & PIPE_CONTROL_STALL_AT_SCOREBOARD)) {
      /* From the PIPE_CONTROL instruction table, bit 1:
       *
       *    "This bit is ignored if Depth Stall Enable is set.
       *     Further, the render cache is not flushed even if Write Cache
       *     Flush Enable bit is set."
       *
       * We assert that the caller doesn't do this combination, to try and
       * prevent mistakes.  It shouldn't hurt the GPU, though.
       *
       * We skip this check on Gfx11+ as the "Stall at Pixel Scoreboard"
       * and "Render Target Flush" combo is explicitly required for BTI
       * update workarounds.
       */
      assert(!(flags & (PIPE_CONTROL_DEPTH_STALL |
                        PIPE_CONTROL_RENDER_TARGET_FLUSH)));
   }

   /* PIPE_CONTROL page workarounds ------------------------------------- */

   if (GFX_VER <= 8 && (flags & PIPE_CONTROL_STATE_CACHE_INVALIDATE)) {
      /* From the PIPE_CONTROL page itself:
       *
       *    "IVB, HSW, BDW
       *     Restriction: Pipe_control with CS-stall bit set must be issued
       *     before a pipe-control command that has the State Cache
       *     Invalidate bit set."
       */
      flags |= PIPE_CONTROL_CS_STALL;
   }

   if (flags & PIPE_CONTROL_FLUSH_LLC) {
      /* From the PIPE_CONTROL instruction table, bit 26 (Flush LLC):
       *
       *    "Project: ALL
       *     SW must always program Post-Sync Operation to "Write Immediate
       *     Data" when Flush LLC is set."
       *
       * For now, we just require the caller to do it.
       */
      assert(flags & PIPE_CONTROL_WRITE_IMMEDIATE);
   }

   /* Emulate a HDC flush with a full Data Cache Flush on older hardware which
    * doesn't support the new lightweight flush.
    */
#if GFX_VER < 12
      if (flags & PIPE_CONTROL_FLUSH_HDC)
         flags |= PIPE_CONTROL_DATA_CACHE_FLUSH;
#endif

   /* "Post-Sync Operation" workarounds -------------------------------- */

   /* Project: All / Argument: Global Snapshot Count Reset [19]
    *
    * "This bit must not be exercised on any product.
    *  Requires stall bit ([20] of DW1) set."
    *
    * We don't use this, so we just assert that it isn't used.  The
    * PIPE_CONTROL instruction page indicates that they intended this
    * as a debug feature and don't think it is useful in production,
    * but it may actually be usable, should we ever want to.
    */
   assert((flags & PIPE_CONTROL_GLOBAL_SNAPSHOT_COUNT_RESET) == 0);

   if (flags & (PIPE_CONTROL_MEDIA_STATE_CLEAR |
                PIPE_CONTROL_INDIRECT_STATE_POINTERS_DISABLE)) {
      /* Project: All / Arguments:
       *
       * - Generic Media State Clear [16]
       * - Indirect State Pointers Disable [16]
       *
       *    "Requires stall bit ([20] of DW1) set."
       *
       * Also, the PIPE_CONTROL instruction table, bit 16 (Generic Media
       * State Clear) says:
       *
       *    "PIPECONTROL command with “Command Streamer Stall Enable” must be
       *     programmed prior to programming a PIPECONTROL command with "Media
       *     State Clear" set in GPGPU mode of operation"
       *
       * This is a subset of the earlier rule, so there's nothing to do.
       */
      flags |= PIPE_CONTROL_CS_STALL;
   }

   if (flags & PIPE_CONTROL_STORE_DATA_INDEX) {
      /* Project: All / Argument: Store Data Index
       *
       * "Post-Sync Operation ([15:14] of DW1) must be set to something other
       *  than '0'."
       *
       * For now, we just assert that the caller does this.  We might want to
       * automatically add a write to the workaround BO...
       */
      assert(non_lri_post_sync_flags != 0);
   }

   if (flags & PIPE_CONTROL_SYNC_GFDT) {
      /* Project: All / Argument: Sync GFDT
       *
       * "Post-Sync Operation ([15:14] of DW1) must be set to something other
       *  than '0' or 0x2520[13] must be set."
       *
       * For now, we just assert that the caller does this.
       */
      assert(non_lri_post_sync_flags != 0);
   }

   if (flags & PIPE_CONTROL_TLB_INVALIDATE) {
      /* Project: IVB+ / Argument: TLB inv
       *
       *    "Requires stall bit ([20] of DW1) set."
       *
       * Also, from the PIPE_CONTROL instruction table:
       *
       *    "Project: SKL+
       *     Post Sync Operation or CS stall must be set to ensure a TLB
       *     invalidation occurs.  Otherwise no cycle will occur to the TLB
       *     cache to invalidate."
       *
       * This is not a subset of the earlier rule, so there's nothing to do.
       */
      flags |= PIPE_CONTROL_CS_STALL;
   }

   if (GFX_VER == 9 && devinfo->gt == 4) {
      /* TODO: The big Skylake GT4 post sync op workaround */
   }

   /* "GPGPU specific workarounds" (both post-sync and flush) ------------ */

   if (IS_COMPUTE_PIPELINE(batch)) {
      if ((GFX_VER == 9 || GFX_VER == 11) &&
          (flags & PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE)) {
         /* Project: SKL, ICL / Argument: Tex Invalidate
          * "Requires stall bit ([20] of DW) set for all GPGPU Workloads."
          */
         flags |= PIPE_CONTROL_CS_STALL;
      }

      if (GFX_VER == 8 && (post_sync_flags ||
                           (flags & (PIPE_CONTROL_NOTIFY_ENABLE |
                                     PIPE_CONTROL_DEPTH_STALL |
                                     PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                     PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                     PIPE_CONTROL_DATA_CACHE_FLUSH)))) {
         /* Project: BDW / Arguments:
          *
          * - LRI Post Sync Operation   [23]
          * - Post Sync Op              [15:14]
          * - Notify En                 [8]
          * - Depth Stall               [13]
          * - Render Target Cache Flush [12]
          * - Depth Cache Flush         [0]
          * - DC Flush Enable           [5]
          *
          *    "Requires stall bit ([20] of DW) set for all GPGPU and Media
          *     Workloads."
          */
         flags |= PIPE_CONTROL_CS_STALL;

         /* Also, from the PIPE_CONTROL instruction table, bit 20:
          *
          *    "Project: BDW
          *     This bit must be always set when PIPE_CONTROL command is
          *     programmed by GPGPU and MEDIA workloads, except for the cases
          *     when only Read Only Cache Invalidation bits are set (State
          *     Cache Invalidation Enable, Instruction cache Invalidation
          *     Enable, Texture Cache Invalidation Enable, Constant Cache
          *     Invalidation Enable). This is to WA FFDOP CG issue, this WA
          *     need not implemented when FF_DOP_CG is disable via "Fixed
          *     Function DOP Clock Gate Disable" bit in RC_PSMI_CTRL register."
          *
          * It sounds like we could avoid CS stalls in some cases, but we
          * don't currently bother.  This list isn't exactly the list above,
          * either...
          */
      }
   }

   /* "Stall" workarounds ----------------------------------------------
    * These have to come after the earlier ones because we may have added
    * some additional CS stalls above.
    */

   if (GFX_VER < 9 && (flags & PIPE_CONTROL_CS_STALL)) {
      /* Project: PRE-SKL, VLV, CHV
       *
       * "[All Stepping][All SKUs]:
       *
       *  One of the following must also be set:
       *
       *  - Render Target Cache Flush Enable ([12] of DW1)
       *  - Depth Cache Flush Enable ([0] of DW1)
       *  - Stall at Pixel Scoreboard ([1] of DW1)
       *  - Depth Stall ([13] of DW1)
       *  - Post-Sync Operation ([13] of DW1)
       *  - DC Flush Enable ([5] of DW1)"
       *
       * If we don't already have one of those bits set, we choose to add
       * "Stall at Pixel Scoreboard".  Some of the other bits require a
       * CS stall as a workaround (see above), which would send us into
       * an infinite recursion of PIPE_CONTROLs.  "Stall at Pixel Scoreboard"
       * appears to be safe, so we choose that.
       */
      const uint32_t wa_bits = PIPE_CONTROL_RENDER_TARGET_FLUSH |
                               PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                               PIPE_CONTROL_WRITE_IMMEDIATE |
                               PIPE_CONTROL_WRITE_DEPTH_COUNT |
                               PIPE_CONTROL_WRITE_TIMESTAMP |
                               PIPE_CONTROL_STALL_AT_SCOREBOARD |
                               PIPE_CONTROL_DEPTH_STALL |
                               PIPE_CONTROL_DATA_CACHE_FLUSH;
      if (!(flags & wa_bits))
         flags |= PIPE_CONTROL_STALL_AT_SCOREBOARD;
   }

   if (INTEL_NEEDS_WA_1409600907 && (flags & PIPE_CONTROL_DEPTH_CACHE_FLUSH)) {
      /* Wa_1409600907:
       *
       * "PIPE_CONTROL with Depth Stall Enable bit must be set
       * with any PIPE_CONTROL with Depth Flush Enable bit set.
       */
      flags |= PIPE_CONTROL_DEPTH_STALL;
   }

   /* Wa_14014966230: For COMPUTE Workload - Any PIPE_CONTROL command with
    * POST_SYNC Operation Enabled MUST be preceded by a PIPE_CONTROL
    * with CS_STALL Bit set (with No POST_SYNC ENABLED)
    */
   if (intel_device_info_is_adln(devinfo) &&
       IS_COMPUTE_PIPELINE(batch) &&
       flags_to_post_sync_op(flags) != NoWrite) {
      iris_emit_raw_pipe_control(batch, "Wa_14014966230",
                                 PIPE_CONTROL_CS_STALL, NULL, 0, 0);
   }

   batch_mark_sync_for_pipe_control(batch, flags);

#if INTEL_NEEDS_WA_14010840176
   /* "If the intention of “constant cache invalidate” is
    *  to invalidate the L1 cache (which can cache constants), use “HDC
    *  pipeline flush” instead of Constant Cache invalidate command."
    *
    * "If L3 invalidate is needed, the w/a should be to set state invalidate
    * in the pipe control command, in addition to the HDC pipeline flush."
    */
   if (flags & PIPE_CONTROL_CONST_CACHE_INVALIDATE) {
      flags &= ~PIPE_CONTROL_CONST_CACHE_INVALIDATE;
      flags |= PIPE_CONTROL_FLUSH_HDC | PIPE_CONTROL_STATE_CACHE_INVALIDATE;
   }
#endif

   /* Emit --------------------------------------------------------------- */

   if (INTEL_DEBUG(DEBUG_PIPE_CONTROL)) {
      fprintf(stderr,
              "  PC [%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%"PRIx64"]: %s\n",
              (flags & PIPE_CONTROL_FLUSH_ENABLE) ? "PipeCon " : "",
              (flags & PIPE_CONTROL_CS_STALL) ? "CS " : "",
              (flags & PIPE_CONTROL_STALL_AT_SCOREBOARD) ? "Scoreboard " : "",
              (flags & PIPE_CONTROL_VF_CACHE_INVALIDATE) ? "VF " : "",
              (flags & PIPE_CONTROL_RENDER_TARGET_FLUSH) ? "RT " : "",
              (flags & PIPE_CONTROL_CONST_CACHE_INVALIDATE) ? "Const " : "",
              (flags & PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE) ? "TC " : "",
              (flags & PIPE_CONTROL_DATA_CACHE_FLUSH) ? "DC " : "",
              (flags & PIPE_CONTROL_DEPTH_CACHE_FLUSH) ? "ZFlush " : "",
              (flags & PIPE_CONTROL_TILE_CACHE_FLUSH) ? "Tile " : "",
              (flags & PIPE_CONTROL_CCS_CACHE_FLUSH) ? "CCS " : "",
              (flags & PIPE_CONTROL_DEPTH_STALL) ? "ZStall " : "",
              (flags & PIPE_CONTROL_STATE_CACHE_INVALIDATE) ? "State " : "",
              (flags & PIPE_CONTROL_TLB_INVALIDATE) ? "TLB " : "",
              (flags & PIPE_CONTROL_INSTRUCTION_INVALIDATE) ? "Inst " : "",
              (flags & PIPE_CONTROL_MEDIA_STATE_CLEAR) ? "MediaClear " : "",
              (flags & PIPE_CONTROL_NOTIFY_ENABLE) ? "Notify " : "",
              (flags & PIPE_CONTROL_GLOBAL_SNAPSHOT_COUNT_RESET) ?
                 "SnapRes" : "",
              (flags & PIPE_CONTROL_INDIRECT_STATE_POINTERS_DISABLE) ?
                  "ISPDis" : "",
              (flags & PIPE_CONTROL_WRITE_IMMEDIATE) ? "WriteImm " : "",
              (flags & PIPE_CONTROL_WRITE_DEPTH_COUNT) ? "WriteZCount " : "",
              (flags & PIPE_CONTROL_WRITE_TIMESTAMP) ? "WriteTimestamp " : "",
              (flags & PIPE_CONTROL_FLUSH_HDC) ? "HDC " : "",
              (flags & PIPE_CONTROL_PSS_STALL_SYNC) ? "PSS " : "",
              (flags & PIPE_CONTROL_UNTYPED_DATAPORT_CACHE_FLUSH) ? "UntypedDataPortCache " : "",
              imm, reason);
   }

   iris_batch_sync_region_start(batch);

   const bool trace_pc =
      (flags & (PIPE_CONTROL_CACHE_FLUSH_BITS | PIPE_CONTROL_CACHE_INVALIDATE_BITS)) != 0;

   if (trace_pc)
      trace_intel_begin_stall(&batch->trace);

   iris_emit_cmd(batch, GENX(PIPE_CONTROL), pc) {
#if GFX_VERx10 >= 125
      pc.PSSStallSyncEnable = flags & PIPE_CONTROL_PSS_STALL_SYNC;
#endif
#if GFX_VER == 12
      pc.TileCacheFlushEnable = flags & PIPE_CONTROL_TILE_CACHE_FLUSH;
#endif
#if GFX_VER > 11
      pc.HDCPipelineFlushEnable = flags & PIPE_CONTROL_FLUSH_HDC;
#endif
#if GFX_VERx10 >= 125
      pc.UntypedDataPortCacheFlushEnable =
         (flags & (PIPE_CONTROL_UNTYPED_DATAPORT_CACHE_FLUSH |
                   PIPE_CONTROL_FLUSH_HDC |
                   PIPE_CONTROL_DATA_CACHE_FLUSH)) &&
         IS_COMPUTE_PIPELINE(batch);
      pc.HDCPipelineFlushEnable |= pc.UntypedDataPortCacheFlushEnable;
      pc.CCSFlushEnable |= flags & PIPE_CONTROL_CCS_CACHE_FLUSH;
#endif
      pc.LRIPostSyncOperation = NoLRIOperation;
      pc.PipeControlFlushEnable = flags & PIPE_CONTROL_FLUSH_ENABLE;
      pc.DCFlushEnable = flags & PIPE_CONTROL_DATA_CACHE_FLUSH;
      pc.StoreDataIndex = 0;
      pc.CommandStreamerStallEnable = flags & PIPE_CONTROL_CS_STALL;
#if GFX_VERx10 < 125
      pc.GlobalSnapshotCountReset =
         flags & PIPE_CONTROL_GLOBAL_SNAPSHOT_COUNT_RESET;
#endif
      pc.TLBInvalidate = flags & PIPE_CONTROL_TLB_INVALIDATE;
#if GFX_VERx10 < 200
      pc.GenericMediaStateClear = flags & PIPE_CONTROL_MEDIA_STATE_CLEAR;
#endif
      pc.StallAtPixelScoreboard = flags & PIPE_CONTROL_STALL_AT_SCOREBOARD;
      pc.RenderTargetCacheFlushEnable =
         flags & PIPE_CONTROL_RENDER_TARGET_FLUSH;
      pc.DepthCacheFlushEnable = flags & PIPE_CONTROL_DEPTH_CACHE_FLUSH;
      pc.StateCacheInvalidationEnable =
         flags & PIPE_CONTROL_STATE_CACHE_INVALIDATE;
#if GFX_VER >= 12
      pc.L3ReadOnlyCacheInvalidationEnable =
         flags & PIPE_CONTROL_L3_READ_ONLY_CACHE_INVALIDATE;
#endif
      pc.VFCacheInvalidationEnable = flags & PIPE_CONTROL_VF_CACHE_INVALIDATE;
      pc.ConstantCacheInvalidationEnable =
         flags & PIPE_CONTROL_CONST_CACHE_INVALIDATE;
      pc.PostSyncOperation = flags_to_post_sync_op(flags);
      pc.DepthStallEnable = flags & PIPE_CONTROL_DEPTH_STALL;
      pc.InstructionCacheInvalidateEnable =
         flags & PIPE_CONTROL_INSTRUCTION_INVALIDATE;
      pc.NotifyEnable = flags & PIPE_CONTROL_NOTIFY_ENABLE;
      pc.IndirectStatePointersDisable =
         flags & PIPE_CONTROL_INDIRECT_STATE_POINTERS_DISABLE;
      pc.TextureCacheInvalidationEnable =
         flags & PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE;
      pc.Address = rw_bo(bo, offset, IRIS_DOMAIN_OTHER_WRITE);
      pc.ImmediateData = imm;
   }

   if (trace_pc) {
      trace_intel_end_stall(&batch->trace, flags,
                            iris_utrace_pipe_flush_bit_to_ds_stall_flag,
                            reason);
   }

   iris_batch_sync_region_end(batch);
}

#if GFX_VER == 9
/**
 * Preemption on Gfx9 has to be enabled or disabled in various cases.
 *
 * See these workarounds for preemption:
 *  - WaDisableMidObjectPreemptionForGSLineStripAdj
 *  - WaDisableMidObjectPreemptionForTrifanOrPolygon
 *  - WaDisableMidObjectPreemptionForLineLoop
 *  - WA#0798
 *
 * We don't put this in the vtable because it's only used on Gfx9.
 */
void
gfx9_toggle_preemption(struct iris_context *ice,
                       struct iris_batch *batch,
                       const struct pipe_draw_info *draw)
{
   struct iris_genx_state *genx = ice->state.genx;
   bool object_preemption = true;

   /* WaDisableMidObjectPreemptionForGSLineStripAdj
    *
    *    "WA: Disable mid-draw preemption when draw-call is a linestrip_adj
    *     and GS is enabled."
    */
   if (draw->mode == MESA_PRIM_LINE_STRIP_ADJACENCY &&
       ice->shaders.prog[MESA_SHADER_GEOMETRY])
      object_preemption = false;

   /* WaDisableMidObjectPreemptionForTrifanOrPolygon
    *
    *    "TriFan miscompare in Execlist Preemption test. Cut index that is
    *     on a previous context. End the previous, the resume another context
    *     with a tri-fan or polygon, and the vertex count is corrupted. If we
    *     prempt again we will cause corruption.
    *
    *     WA: Disable mid-draw preemption when draw-call has a tri-fan."
    */
   if (draw->mode == MESA_PRIM_TRIANGLE_FAN)
      object_preemption = false;

   /* WaDisableMidObjectPreemptionForLineLoop
    *
    *    "VF Stats Counters Missing a vertex when preemption enabled.
    *
    *     WA: Disable mid-draw preemption when the draw uses a lineloop
    *     topology."
    */
   if (draw->mode == MESA_PRIM_LINE_LOOP)
      object_preemption = false;

   /* WA#0798
    *
    *    "VF is corrupting GAFS data when preempted on an instance boundary
    *     and replayed with instancing enabled.
    *
    *     WA: Disable preemption when using instanceing."
    */
   if (draw->instance_count > 1)
      object_preemption = false;

   if (genx->object_preemption != object_preemption) {
      iris_enable_obj_preemption(batch, object_preemption);
      genx->object_preemption = object_preemption;
   }
}
#endif

static void
iris_lost_genx_state(struct iris_context *ice, struct iris_batch *batch)
{
   struct iris_genx_state *genx = ice->state.genx;

#if INTEL_NEEDS_WA_1808121037
   genx->depth_reg_mode = IRIS_DEPTH_REG_MODE_UNKNOWN;
#endif

   memset(genx->last_index_buffer, 0, sizeof(genx->last_index_buffer));
}

static void
iris_emit_mi_report_perf_count(struct iris_batch *batch,
                               struct iris_bo *bo,
                               uint32_t offset_in_bytes,
                               uint32_t report_id)
{
   iris_batch_sync_region_start(batch);
   iris_emit_cmd(batch, GENX(MI_REPORT_PERF_COUNT), mi_rpc) {
      mi_rpc.MemoryAddress = rw_bo(bo, offset_in_bytes,
                                   IRIS_DOMAIN_OTHER_WRITE);
      mi_rpc.ReportID = report_id;
   }
   iris_batch_sync_region_end(batch);
}

/**
 * Update the pixel hashing modes that determine the balancing of PS threads
 * across subslices and slices.
 *
 * \param width Width bound of the rendering area (already scaled down if \p
 *              scale is greater than 1).
 * \param height Height bound of the rendering area (already scaled down if \p
 *               scale is greater than 1).
 * \param scale The number of framebuffer samples that could potentially be
 *              affected by an individual channel of the PS thread.  This is
 *              typically one for single-sampled rendering, but for operations
 *              like CCS resolves and fast clears a single PS invocation may
 *              update a huge number of pixels, in which case a finer
 *              balancing is desirable in order to maximally utilize the
 *              bandwidth available.  UINT_MAX can be used as shorthand for
 *              "finest hashing mode available".
 */
void
genX(emit_hashing_mode)(struct iris_context *ice, struct iris_batch *batch,
                        unsigned width, unsigned height, unsigned scale)
{
#if GFX_VER == 9
   const struct intel_device_info *devinfo = batch->screen->devinfo;
   const unsigned slice_hashing[] = {
      /* Because all Gfx9 platforms with more than one slice require
       * three-way subslice hashing, a single "normal" 16x16 slice hashing
       * block is guaranteed to suffer from substantial imbalance, with one
       * subslice receiving twice as much work as the other two in the
       * slice.
       *
       * The performance impact of that would be particularly severe when
       * three-way hashing is also in use for slice balancing (which is the
       * case for all Gfx9 GT4 platforms), because one of the slices
       * receives one every three 16x16 blocks in either direction, which
       * is roughly the periodicity of the underlying subslice imbalance
       * pattern ("roughly" because in reality the hardware's
       * implementation of three-way hashing doesn't do exact modulo 3
       * arithmetic, which somewhat decreases the magnitude of this effect
       * in practice).  This leads to a systematic subslice imbalance
       * within that slice regardless of the size of the primitive.  The
       * 32x32 hashing mode guarantees that the subslice imbalance within a
       * single slice hashing block is minimal, largely eliminating this
       * effect.
       */
      _32x32,
      /* Finest slice hashing mode available. */
      NORMAL
   };
   const unsigned subslice_hashing[] = {
      /* 16x16 would provide a slight cache locality benefit especially
       * visible in the sampler L1 cache efficiency of low-bandwidth
       * non-LLC platforms, but it comes at the cost of greater subslice
       * imbalance for primitives of dimensions approximately intermediate
       * between 16x4 and 16x16.
       */
      _16x4,
      /* Finest subslice hashing mode available. */
      _8x4
   };
   /* Dimensions of the smallest hashing block of a given hashing mode.  If
    * the rendering area is smaller than this there can't possibly be any
    * benefit from switching to this mode, so we optimize out the
    * transition.
    */
   const unsigned min_size[][2] = {
      { 16, 4 },
      { 8, 4 }
   };
   const unsigned idx = scale > 1;

   if (width > min_size[idx][0] || height > min_size[idx][1]) {
      iris_emit_raw_pipe_control(batch,
                                 "workaround: CS stall before GT_MODE LRI",
                                 PIPE_CONTROL_STALL_AT_SCOREBOARD |
                                 PIPE_CONTROL_CS_STALL,
                                 NULL, 0, 0);

      iris_emit_reg(batch, GENX(GT_MODE), reg) {
         reg.SliceHashing = (devinfo->num_slices > 1 ? slice_hashing[idx] : 0);
         reg.SliceHashingMask = (devinfo->num_slices > 1 ? -1 : 0);
         reg.SubsliceHashing = subslice_hashing[idx];
         reg.SubsliceHashingMask = -1;
      };

      ice->state.current_hash_scale = scale;
   }
#endif
}

static void
iris_set_frontend_noop(struct pipe_context *ctx, bool enable)
{
   struct iris_context *ice = (struct iris_context *) ctx;

   if (iris_batch_prepare_noop(&ice->batches[IRIS_BATCH_RENDER], enable)) {
      ice->state.dirty |= IRIS_ALL_DIRTY_FOR_RENDER;
      ice->state.stage_dirty |= IRIS_ALL_STAGE_DIRTY_FOR_RENDER;
   }

   if (iris_batch_prepare_noop(&ice->batches[IRIS_BATCH_COMPUTE], enable)) {
      ice->state.dirty |= IRIS_ALL_DIRTY_FOR_COMPUTE;
      ice->state.stage_dirty |= IRIS_ALL_STAGE_DIRTY_FOR_COMPUTE;
   }
}

void
genX(init_screen_state)(struct iris_screen *screen)
{
   assert(screen->devinfo->verx10 == GFX_VERx10);
   screen->vtbl.destroy_state = iris_destroy_state;
   screen->vtbl.init_render_context = iris_init_render_context;
   screen->vtbl.init_compute_context = iris_init_compute_context;
   screen->vtbl.init_copy_context = iris_init_copy_context;
   screen->vtbl.upload_render_state = iris_upload_render_state;
   screen->vtbl.upload_indirect_render_state = iris_upload_indirect_render_state;
   screen->vtbl.update_binder_address = iris_update_binder_address;
   screen->vtbl.upload_compute_state = iris_upload_compute_state;
   screen->vtbl.emit_raw_pipe_control = iris_emit_raw_pipe_control;
   screen->vtbl.rewrite_compute_walker_pc = iris_rewrite_compute_walker_pc;
   screen->vtbl.emit_mi_report_perf_count = iris_emit_mi_report_perf_count;
   screen->vtbl.rebind_buffer = iris_rebind_buffer;
   screen->vtbl.load_register_reg32 = iris_load_register_reg32;
   screen->vtbl.load_register_reg64 = iris_load_register_reg64;
   screen->vtbl.load_register_imm32 = iris_load_register_imm32;
   screen->vtbl.load_register_imm64 = iris_load_register_imm64;
   screen->vtbl.load_register_mem32 = iris_load_register_mem32;
   screen->vtbl.load_register_mem64 = iris_load_register_mem64;
   screen->vtbl.store_register_mem32 = iris_store_register_mem32;
   screen->vtbl.store_register_mem64 = iris_store_register_mem64;
   screen->vtbl.store_data_imm32 = iris_store_data_imm32;
   screen->vtbl.store_data_imm64 = iris_store_data_imm64;
   screen->vtbl.copy_mem_mem = iris_copy_mem_mem;
   screen->vtbl.derived_program_state_size = iris_derived_program_state_size;
   screen->vtbl.store_derived_program_state = iris_store_derived_program_state;
   screen->vtbl.create_so_decl_list = iris_create_so_decl_list;
   screen->vtbl.populate_vs_key = iris_populate_vs_key;
   screen->vtbl.populate_tcs_key = iris_populate_tcs_key;
   screen->vtbl.populate_tes_key = iris_populate_tes_key;
   screen->vtbl.populate_gs_key = iris_populate_gs_key;
   screen->vtbl.populate_fs_key = iris_populate_fs_key;
   screen->vtbl.populate_cs_key = iris_populate_cs_key;
   screen->vtbl.lost_genx_state = iris_lost_genx_state;
   screen->vtbl.disable_rhwo_optimization = iris_disable_rhwo_optimization;
}

void
genX(init_state)(struct iris_context *ice)
{
   struct pipe_context *ctx = &ice->ctx;
   struct iris_screen *screen = (struct iris_screen *)ctx->screen;

   ctx->create_blend_state = iris_create_blend_state;
   ctx->create_depth_stencil_alpha_state = iris_create_zsa_state;
   ctx->create_rasterizer_state = iris_create_rasterizer_state;
   ctx->create_sampler_state = iris_create_sampler_state;
   ctx->create_sampler_view = iris_create_sampler_view;
   ctx->create_surface = iris_create_surface;
   ctx->create_vertex_elements_state = iris_create_vertex_elements;
   ctx->bind_blend_state = iris_bind_blend_state;
   ctx->bind_depth_stencil_alpha_state = iris_bind_zsa_state;
   ctx->bind_sampler_states = iris_bind_sampler_states;
   ctx->bind_rasterizer_state = iris_bind_rasterizer_state;
   ctx->bind_vertex_elements_state = iris_bind_vertex_elements_state;
   ctx->delete_blend_state = iris_delete_state;
   ctx->delete_depth_stencil_alpha_state = iris_delete_state;
   ctx->delete_rasterizer_state = iris_delete_state;
   ctx->delete_sampler_state = iris_delete_state;
   ctx->delete_vertex_elements_state = iris_delete_state;
   ctx->set_blend_color = iris_set_blend_color;
   ctx->set_clip_state = iris_set_clip_state;
   ctx->set_constant_buffer = iris_set_constant_buffer;
   ctx->set_shader_buffers = iris_set_shader_buffers;
   ctx->set_shader_images = iris_set_shader_images;
   ctx->set_sampler_views = iris_set_sampler_views;
   ctx->set_compute_resources = iris_set_compute_resources;
   ctx->set_global_binding = iris_set_global_binding;
   ctx->set_tess_state = iris_set_tess_state;
   ctx->set_patch_vertices = iris_set_patch_vertices;
   ctx->set_framebuffer_state = iris_set_framebuffer_state;
   ctx->set_polygon_stipple = iris_set_polygon_stipple;
   ctx->set_sample_mask = iris_set_sample_mask;
   ctx->set_scissor_states = iris_set_scissor_states;
   ctx->set_stencil_ref = iris_set_stencil_ref;
   ctx->set_vertex_buffers = iris_set_vertex_buffers;
   ctx->set_viewport_states = iris_set_viewport_states;
   ctx->sampler_view_destroy = iris_sampler_view_destroy;
   ctx->surface_destroy = iris_surface_destroy;
   ctx->draw_vbo = iris_draw_vbo;
   ctx->launch_grid = iris_launch_grid;
   ctx->create_stream_output_target = iris_create_stream_output_target;
   ctx->stream_output_target_destroy = iris_stream_output_target_destroy;
   ctx->set_stream_output_targets = iris_set_stream_output_targets;
   ctx->set_frontend_noop = iris_set_frontend_noop;

   ice->state.dirty = ~0ull;
   ice->state.stage_dirty = ~0ull;

   ice->state.statistics_counters_enabled = true;

   ice->state.sample_mask = 0xffff;
   ice->state.num_viewports = 1;
   ice->state.prim_mode = MESA_PRIM_COUNT;
   ice->state.genx = calloc(1, sizeof(struct iris_genx_state));
   ice->draw.derived_params.drawid = -1;

#if GFX_VERx10 >= 120
   ice->state.genx->object_preemption = true;
#endif

   /* Make a 1x1x1 null surface for unbound textures */
   void *null_surf_map =
      upload_state(ice->state.surface_uploader, &ice->state.unbound_tex,
                   4 * GENX(RENDER_SURFACE_STATE_length), 64);
   isl_null_fill_state(&screen->isl_dev, null_surf_map,
                       .size = isl_extent3d(1, 1, 1));
   ice->state.unbound_tex.offset +=
      iris_bo_offset_from_base_address(iris_resource_bo(ice->state.unbound_tex.res));

   /* Default all scissor rectangles to be empty regions. */
   for (int i = 0; i < IRIS_MAX_VIEWPORTS; i++) {
      ice->state.scissors[i] = (struct pipe_scissor_state) {
         .minx = 1, .maxx = 0, .miny = 1, .maxy = 0,
      };
   }
}
