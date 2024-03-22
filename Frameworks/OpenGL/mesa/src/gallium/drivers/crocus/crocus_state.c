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
 * @file crocus_state.c
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
 * crocus_upload_render_state() and crocus_upload_compute_state(), which read
 * the context state and emit the commands into the actual batch.
 */

#include <errno.h>
#include <stdio.h>

#if HAVE_VALGRIND
#include <memcheck.h>
#include <valgrind.h>
#define VG(x) x
#else
#define VG(x)
#endif

#include "drm-uapi/i915_drm.h"
#include "intel/common/intel_l3_config.h"
#include "intel/common/intel_sample_positions.h"
#include "intel/compiler/brw_compiler.h"
#include "compiler/shader_info.h"
#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h"
#include "util/format/u_format.h"
#include "util/half_float.h"
#include "util/u_dual_blend.h"
#include "util/u_framebuffer.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "util/u_transfer.h"
#include "util/u_upload_mgr.h"
#include "util/u_viewport.h"
#include "crocus_batch.h"
#include "crocus_context.h"
#include "crocus_defines.h"
#include "crocus_pipe.h"
#include "crocus_resource.h"

#include "crocus_genx_macros.h"
#include "intel/common/intel_genX_state.h"
#include "intel/common/intel_guardband.h"
#include "main/macros.h" /* UNCLAMPED_* */

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

#if GFX_VER >= 6
   /* pipe_sprite_coord_mode happens to match 3DSTATE_SBE */
   PIPE_ASSERT(PIPE_SPRITE_COORD_UPPER_LEFT == UPPERLEFT);
   PIPE_ASSERT(PIPE_SPRITE_COORD_LOWER_LEFT == LOWERLEFT);
#endif
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
#if GFX_VER >= 6
      [MESA_PRIM_LINES_ADJACENCY]          = _3DPRIM_LINELIST_ADJ,
      [MESA_PRIM_LINE_STRIP_ADJACENCY]     = _3DPRIM_LINESTRIP_ADJ,
      [MESA_PRIM_TRIANGLES_ADJACENCY]      = _3DPRIM_TRILIST_ADJ,
      [MESA_PRIM_TRIANGLE_STRIP_ADJACENCY] = _3DPRIM_TRISTRIP_ADJ,
#endif
#if GFX_VER >= 7
      [MESA_PRIM_PATCHES]                  = _3DPRIM_PATCHLIST_1 - 1,
#endif
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

#if GFX_VER >= 6
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
#endif

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
translate_wrap(unsigned pipe_wrap, bool either_nearest)
{
   static const unsigned map[] = {
      [PIPE_TEX_WRAP_REPEAT]                 = TCM_WRAP,
#if GFX_VER == 8
      [PIPE_TEX_WRAP_CLAMP]                  = TCM_HALF_BORDER,
#else
      [PIPE_TEX_WRAP_CLAMP]                  = TCM_CLAMP_BORDER,
#endif
      [PIPE_TEX_WRAP_CLAMP_TO_EDGE]          = TCM_CLAMP,
      [PIPE_TEX_WRAP_CLAMP_TO_BORDER]        = TCM_CLAMP_BORDER,
      [PIPE_TEX_WRAP_MIRROR_REPEAT]          = TCM_MIRROR,
      [PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE]   = TCM_MIRROR_ONCE,

      /* These are unsupported. */
      [PIPE_TEX_WRAP_MIRROR_CLAMP]           = -1,
      [PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER] = -1,
   };
#if GFX_VER < 8
   if (pipe_wrap == PIPE_TEX_WRAP_CLAMP && either_nearest)
      return TCM_CLAMP;
#endif
   return map[pipe_wrap];
}

/**
 * Equiv if brw_state_batch
 */
static uint32_t *
stream_state(struct crocus_batch *batch,
             unsigned size,
             unsigned alignment,
             uint32_t *out_offset)
{
   uint32_t offset = ALIGN(batch->state.used, alignment);

   if (offset + size >= STATE_SZ && !batch->no_wrap) {
      crocus_batch_flush(batch);
      offset = ALIGN(batch->state.used, alignment);
   } else if (offset + size >= batch->state.bo->size) {
      const unsigned new_size =
         MIN2(batch->state.bo->size + batch->state.bo->size / 2,
              MAX_STATE_SIZE);
      crocus_grow_buffer(batch, true, batch->state.used, new_size);
      assert(offset + size < batch->state.bo->size);
   }

   crocus_record_state_size(batch->state_sizes, offset, size);

   batch->state.used = offset + size;
   *out_offset = offset;

   return (uint32_t *)batch->state.map + (offset >> 2);
}

/**
 * stream_state() + memcpy.
 */
static uint32_t
emit_state(struct crocus_batch *batch, const void *data, unsigned size,
           unsigned alignment)
{
   unsigned offset = 0;
   uint32_t *map = stream_state(batch, size, alignment, &offset);

   if (map)
      memcpy(map, data, size);

   return offset;
}

#if GFX_VER <= 5
static void
upload_pipelined_state_pointers(struct crocus_batch *batch,
                                bool gs_active, uint32_t gs_offset,
                                uint32_t vs_offset, uint32_t sf_offset,
                                uint32_t clip_offset, uint32_t wm_offset, uint32_t cc_offset)
{
#if GFX_VER == 5
   /* Need to flush before changing clip max threads for errata. */
   crocus_emit_cmd(batch, GENX(MI_FLUSH), foo);
#endif

   crocus_emit_cmd(batch, GENX(3DSTATE_PIPELINED_POINTERS), pp) {
      pp.PointertoVSState = ro_bo(batch->state.bo, vs_offset);
      pp.GSEnable = gs_active;
      if (gs_active)
         pp.PointertoGSState = ro_bo(batch->state.bo, gs_offset);
      pp.ClipEnable = true;
      pp.PointertoCLIPState = ro_bo(batch->state.bo, clip_offset);
      pp.PointertoSFState = ro_bo(batch->state.bo, sf_offset);
      pp.PointertoWMState = ro_bo(batch->state.bo, wm_offset);
      pp.PointertoColorCalcState = ro_bo(batch->state.bo, cc_offset);
   }
}

#endif
/**
 * Did field 'x' change between 'old_cso' and 'new_cso'?
 *
 * (If so, we may want to set some dirty flags.)
 */
#define cso_changed(x) (!old_cso || (old_cso->x != new_cso->x))
#define cso_changed_memcmp(x) \
   (!old_cso || memcmp(old_cso->x, new_cso->x, sizeof(old_cso->x)) != 0)

static void
flush_before_state_base_change(struct crocus_batch *batch)
{
#if GFX_VER >= 6
   /* Flush before emitting STATE_BASE_ADDRESS.
    *
    * This isn't documented anywhere in the PRM.  However, it seems to be
    * necessary prior to changing the surface state base adress.  We've
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
   const unsigned dc_flush =
      GFX_VER >= 7 ? PIPE_CONTROL_DATA_CACHE_FLUSH : 0;
   crocus_emit_end_of_pipe_sync(batch,
                                "change STATE_BASE_ADDRESS (flushes)",
                                PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                dc_flush |
                                PIPE_CONTROL_DEPTH_CACHE_FLUSH);
#endif
}

static void
flush_after_state_base_change(struct crocus_batch *batch)
{
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
    */
#if GFX_VER >= 6
   crocus_emit_end_of_pipe_sync(batch,
                                "change STATE_BASE_ADDRESS (invalidates)",
                                PIPE_CONTROL_INSTRUCTION_INVALIDATE |
                                PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
                                PIPE_CONTROL_CONST_CACHE_INVALIDATE |
                                PIPE_CONTROL_STATE_CACHE_INVALIDATE);
#endif
}

#if GFX_VER >= 6
static void
crocus_store_register_mem32(struct crocus_batch *batch, uint32_t reg,
                            struct crocus_bo *bo, uint32_t offset,
                            bool predicated)
{
   crocus_emit_cmd(batch, GENX(MI_STORE_REGISTER_MEM), srm) {
      srm.RegisterAddress = reg;
      srm.MemoryAddress = ggtt_bo(bo, offset);
#if GFX_VERx10 >= 75
      srm.PredicateEnable = predicated;
#else
      if (predicated)
         unreachable("unsupported predication");
#endif
   }
}

static void
crocus_store_register_mem64(struct crocus_batch *batch, uint32_t reg,
                            struct crocus_bo *bo, uint32_t offset,
                            bool predicated)
{
   crocus_store_register_mem32(batch, reg + 0, bo, offset + 0, predicated);
   crocus_store_register_mem32(batch, reg + 4, bo, offset + 4, predicated);
}
#endif

#if GFX_VER >= 7
static void
_crocus_emit_lri(struct crocus_batch *batch, uint32_t reg, uint32_t val)
{
   crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_IMM), lri) {
      lri.RegisterOffset = reg;
      lri.DataDWord      = val;
   }
}
#define crocus_emit_lri(b, r, v) _crocus_emit_lri(b, GENX(r##_num), v)

#if GFX_VERx10 >= 75
static void
_crocus_emit_lrr(struct crocus_batch *batch, uint32_t dst, uint32_t src)
{
   crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_REG), lrr) {
      lrr.SourceRegisterAddress = src;
      lrr.DestinationRegisterAddress = dst;
   }
}

static void
crocus_load_register_reg32(struct crocus_batch *batch, uint32_t dst,
                           uint32_t src)
{
   _crocus_emit_lrr(batch, dst, src);
}

static void
crocus_load_register_reg64(struct crocus_batch *batch, uint32_t dst,
                           uint32_t src)
{
   _crocus_emit_lrr(batch, dst, src);
   _crocus_emit_lrr(batch, dst + 4, src + 4);
}
#endif

static void
crocus_load_register_imm32(struct crocus_batch *batch, uint32_t reg,
                           uint32_t val)
{
   _crocus_emit_lri(batch, reg, val);
}

static void
crocus_load_register_imm64(struct crocus_batch *batch, uint32_t reg,
                           uint64_t val)
{
   _crocus_emit_lri(batch, reg + 0, val & 0xffffffff);
   _crocus_emit_lri(batch, reg + 4, val >> 32);
}

/**
 * Emit MI_LOAD_REGISTER_MEM to load a 32-bit MMIO register from a buffer.
 */
static void
crocus_load_register_mem32(struct crocus_batch *batch, uint32_t reg,
                           struct crocus_bo *bo, uint32_t offset)
{
   crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
      lrm.RegisterAddress = reg;
      lrm.MemoryAddress = ro_bo(bo, offset);
   }
}

/**
 * Load a 64-bit value from a buffer into a MMIO register via
 * two MI_LOAD_REGISTER_MEM commands.
 */
static void
crocus_load_register_mem64(struct crocus_batch *batch, uint32_t reg,
                           struct crocus_bo *bo, uint32_t offset)
{
   crocus_load_register_mem32(batch, reg + 0, bo, offset + 0);
   crocus_load_register_mem32(batch, reg + 4, bo, offset + 4);
}

#if GFX_VERx10 >= 75
static void
crocus_store_data_imm32(struct crocus_batch *batch,
                        struct crocus_bo *bo, uint32_t offset,
                        uint32_t imm)
{
   crocus_emit_cmd(batch, GENX(MI_STORE_DATA_IMM), sdi) {
      sdi.Address = rw_bo(bo, offset);
#if GFX_VER >= 6
      sdi.ImmediateData = imm;
#endif
   }
}

static void
crocus_store_data_imm64(struct crocus_batch *batch,
                        struct crocus_bo *bo, uint32_t offset,
                        uint64_t imm)
{
   /* Can't use crocus_emit_cmd because MI_STORE_DATA_IMM has a length of
    * 2 in genxml but it's actually variable length and we need 5 DWords.
    */
   void *map = crocus_get_command_space(batch, 4 * 5);
   _crocus_pack_command(batch, GENX(MI_STORE_DATA_IMM), map, sdi) {
      sdi.DWordLength = 5 - 2;
      sdi.Address = rw_bo(bo, offset);
#if GFX_VER >= 6
      sdi.ImmediateData = imm;
#endif
   }
}
#endif

static void
crocus_copy_mem_mem(struct crocus_batch *batch,
                    struct crocus_bo *dst_bo, uint32_t dst_offset,
                    struct crocus_bo *src_bo, uint32_t src_offset,
                    unsigned bytes)
{
   assert(bytes % 4 == 0);
   assert(dst_offset % 4 == 0);
   assert(src_offset % 4 == 0);

#define CROCUS_TEMP_REG 0x2440 /* GEN7_3DPRIM_BASE_VERTEX */
   for (unsigned i = 0; i < bytes; i += 4) {
      crocus_load_register_mem32(batch, CROCUS_TEMP_REG,
                                 src_bo, src_offset + i);
      crocus_store_register_mem32(batch, CROCUS_TEMP_REG,
                                  dst_bo, dst_offset + i, false);
   }
}
#endif

/**
 * Gallium CSO for rasterizer state.
 */
struct crocus_rasterizer_state {
   struct pipe_rasterizer_state cso;
#if GFX_VER >= 6
   uint32_t sf[GENX(3DSTATE_SF_length)];
   uint32_t clip[GENX(3DSTATE_CLIP_length)];
#endif
#if GFX_VER >= 8
   uint32_t raster[GENX(3DSTATE_RASTER_length)];
#endif
   uint32_t line_stipple[GENX(3DSTATE_LINE_STIPPLE_length)];

   uint8_t num_clip_plane_consts;
   bool fill_mode_point_or_line;
};

#if GFX_VER <= 5
#define URB_VS 0
#define URB_GS 1
#define URB_CLP 2
#define URB_SF 3
#define URB_CS 4

static const struct {
   uint32_t min_nr_entries;
   uint32_t preferred_nr_entries;
   uint32_t min_entry_size;
   uint32_t  max_entry_size;
} limits[URB_CS+1] = {
   { 16, 32, 1, 5 },                        /* vs */
   { 4, 8,  1, 5 },                        /* gs */
   { 5, 10,  1, 5 },                        /* clp */
   { 1, 8,  1, 12 },                        /* sf */
   { 1, 4,  1, 32 }                        /* cs */
};

static bool check_urb_layout(struct crocus_context *ice)
{
   ice->urb.vs_start = 0;
   ice->urb.gs_start = ice->urb.nr_vs_entries * ice->urb.vsize;
   ice->urb.clip_start = ice->urb.gs_start + ice->urb.nr_gs_entries * ice->urb.vsize;
   ice->urb.sf_start = ice->urb.clip_start + ice->urb.nr_clip_entries * ice->urb.vsize;
   ice->urb.cs_start = ice->urb.sf_start + ice->urb.nr_sf_entries * ice->urb.sfsize;

   return ice->urb.cs_start + ice->urb.nr_cs_entries *
      ice->urb.csize <= ice->urb.size;
}


static bool
crocus_calculate_urb_fence(struct crocus_batch *batch, unsigned csize,
                           unsigned vsize, unsigned sfsize)
{
   struct crocus_context *ice = batch->ice;
   if (csize < limits[URB_CS].min_entry_size)
      csize = limits[URB_CS].min_entry_size;

   if (vsize < limits[URB_VS].min_entry_size)
      vsize = limits[URB_VS].min_entry_size;

   if (sfsize < limits[URB_SF].min_entry_size)
      sfsize = limits[URB_SF].min_entry_size;

   if (ice->urb.vsize < vsize ||
       ice->urb.sfsize < sfsize ||
       ice->urb.csize < csize ||
       (ice->urb.constrained && (ice->urb.vsize > vsize ||
                                 ice->urb.sfsize > sfsize ||
                                 ice->urb.csize > csize))) {


      ice->urb.csize = csize;
      ice->urb.sfsize = sfsize;
      ice->urb.vsize = vsize;

      ice->urb.nr_vs_entries = limits[URB_VS].preferred_nr_entries;
      ice->urb.nr_gs_entries = limits[URB_GS].preferred_nr_entries;
      ice->urb.nr_clip_entries = limits[URB_CLP].preferred_nr_entries;
      ice->urb.nr_sf_entries = limits[URB_SF].preferred_nr_entries;
      ice->urb.nr_cs_entries = limits[URB_CS].preferred_nr_entries;

      ice->urb.constrained = 0;

      if (GFX_VER == 5) {
         ice->urb.nr_vs_entries = 128;
         ice->urb.nr_sf_entries = 48;
         if (check_urb_layout(ice)) {
            goto done;
         } else {
            ice->urb.constrained = 1;
            ice->urb.nr_vs_entries = limits[URB_VS].preferred_nr_entries;
            ice->urb.nr_sf_entries = limits[URB_SF].preferred_nr_entries;
         }
      } else if (GFX_VERx10 == 45) {
         ice->urb.nr_vs_entries = 64;
         if (check_urb_layout(ice)) {
            goto done;
         } else {
            ice->urb.constrained = 1;
            ice->urb.nr_vs_entries = limits[URB_VS].preferred_nr_entries;
         }
      }

      if (!check_urb_layout(ice)) {
         ice->urb.nr_vs_entries = limits[URB_VS].min_nr_entries;
         ice->urb.nr_gs_entries = limits[URB_GS].min_nr_entries;
         ice->urb.nr_clip_entries = limits[URB_CLP].min_nr_entries;
         ice->urb.nr_sf_entries = limits[URB_SF].min_nr_entries;
         ice->urb.nr_cs_entries = limits[URB_CS].min_nr_entries;

         /* Mark us as operating with constrained nr_entries, so that next
          * time we recalculate we'll resize the fences in the hope of
          * escaping constrained mode and getting back to normal performance.
          */
         ice->urb.constrained = 1;

         if (!check_urb_layout(ice)) {
            /* This is impossible, given the maximal sizes of urb
             * entries and the values for minimum nr of entries
             * provided above.
             */
            fprintf(stderr, "couldn't calculate URB layout!\n");
            exit(1);
         }

         if (INTEL_DEBUG(DEBUG_URB|DEBUG_PERF))
            fprintf(stderr, "URB CONSTRAINED\n");
      }

done:
      if (INTEL_DEBUG(DEBUG_URB))
         fprintf(stderr,
                 "URB fence: %d ..VS.. %d ..GS.. %d ..CLP.. %d ..SF.. %d ..CS.. %d\n",
                 ice->urb.vs_start,
                 ice->urb.gs_start,
                 ice->urb.clip_start,
                 ice->urb.sf_start,
                 ice->urb.cs_start,
                 ice->urb.size);
      return true;
   }
   return false;
}

static void
crocus_upload_urb_fence(struct crocus_batch *batch)
{
   uint32_t urb_fence[3];
   _crocus_pack_command(batch, GENX(URB_FENCE), urb_fence, urb) {
      urb.VSUnitURBReallocationRequest = 1;
      urb.GSUnitURBReallocationRequest = 1;
      urb.CLIPUnitURBReallocationRequest = 1;
      urb.SFUnitURBReallocationRequest = 1;
      urb.VFEUnitURBReallocationRequest = 1;
      urb.CSUnitURBReallocationRequest = 1;

      urb.VSFence = batch->ice->urb.gs_start;
      urb.GSFence = batch->ice->urb.clip_start;
      urb.CLIPFence = batch->ice->urb.sf_start;
      urb.SFFence = batch->ice->urb.cs_start;
      urb.CSFence = batch->ice->urb.size;
   }

   /* erratum: URB_FENCE must not cross a 64byte cacheline */
   if ((crocus_batch_bytes_used(batch) & 15) > 12) {
      int pad = 16 - (crocus_batch_bytes_used(batch) & 15);
      do {
         *(uint32_t *)batch->command.map_next = 0;
         batch->command.map_next += sizeof(uint32_t);
      } while (--pad);
   }

   crocus_batch_emit(batch, urb_fence, sizeof(uint32_t) * 3);
}

static bool
calculate_curbe_offsets(struct crocus_batch *batch)
{
   struct crocus_context *ice = batch->ice;

   unsigned nr_fp_regs, nr_vp_regs, nr_clip_regs = 0;
   unsigned total_regs;

   nr_fp_regs = 0;
   for (int i = 0; i < 4; i++) {
      const struct brw_ubo_range *range = &ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data->ubo_ranges[i];
      if (range->length == 0)
         continue;

      /* ubo range tracks at 256-bit, we need 512-bit */
      nr_fp_regs += (range->length + 1) / 2;
   }

   if (ice->state.cso_rast->cso.clip_plane_enable) {
      unsigned nr_planes = 6 + util_bitcount(ice->state.cso_rast->cso.clip_plane_enable);
      nr_clip_regs = (nr_planes * 4 + 15) / 16;
   }

   nr_vp_regs = 0;
   for (int i = 0; i < 4; i++) {
      const struct brw_ubo_range *range = &ice->shaders.prog[MESA_SHADER_VERTEX]->prog_data->ubo_ranges[i];
      if (range->length == 0)
         continue;

      /* ubo range tracks at 256-bit, we need 512-bit */
      nr_vp_regs += (range->length + 1) / 2;
   }
   if (nr_vp_regs == 0) {
      /* The pre-gen6 VS requires that some push constants get loaded no
       * matter what, or the GPU would hang.
       */
      nr_vp_regs = 1;
   }
   total_regs = nr_fp_regs + nr_vp_regs + nr_clip_regs;

   /* The CURBE allocation size is limited to 32 512-bit units (128 EU
    * registers, or 1024 floats).  See CS_URB_STATE in the gen4 or gen5
    * (volume 1, part 1) PRMs.
    *
    * Note that in brw_fs.cpp we're only loading up to 16 EU registers of
    * values as push constants before spilling to pull constants, and in
    * brw_vec4.cpp we're loading up to 32 registers of push constants.  An EU
    * register is 1/2 of one of these URB entry units, so that leaves us 16 EU
    * regs for clip.
    */
   assert(total_regs <= 32);

   /* Lazy resize:
    */
   if (nr_fp_regs > ice->curbe.wm_size ||
       nr_vp_regs > ice->curbe.vs_size ||
       nr_clip_regs != ice->curbe.clip_size ||
       (total_regs < ice->curbe.total_size / 4 &&
        ice->curbe.total_size > 16)) {

      GLuint reg = 0;

      /* Calculate a new layout:
       */
      reg = 0;
      ice->curbe.wm_start = reg;
      ice->curbe.wm_size = nr_fp_regs; reg += nr_fp_regs;
      ice->curbe.clip_start = reg;
      ice->curbe.clip_size = nr_clip_regs; reg += nr_clip_regs;
      ice->curbe.vs_start = reg;
      ice->curbe.vs_size = nr_vp_regs; reg += nr_vp_regs;
      ice->curbe.total_size = reg;

      if (0)
         fprintf(stderr, "curbe wm %d+%d clip %d+%d vs %d+%d\n",
                 ice->curbe.wm_start,
                 ice->curbe.wm_size,
                 ice->curbe.clip_start,
                 ice->curbe.clip_size,
                 ice->curbe.vs_start,
                 ice->curbe.vs_size );
      return true;
   }
   return false;
}

static void
upload_shader_consts(struct crocus_context *ice,
                     gl_shader_stage stage,
                     uint32_t *map,
                     unsigned start)
{
   struct crocus_compiled_shader *shader = ice->shaders.prog[stage];
   struct brw_stage_prog_data *prog_data = (void *) shader->prog_data;
   uint32_t *cmap;
   bool found = false;
   unsigned offset = start * 16;
   int total = 0;
   for (int i = 0; i < 4; i++) {
      const struct brw_ubo_range *range = &prog_data->ubo_ranges[i];

      if (range->length == 0)
         continue;

      unsigned block_index = crocus_bti_to_group_index(
         &shader->bt, CROCUS_SURFACE_GROUP_UBO, range->block);
      unsigned len = range->length * 8 * sizeof(float);
      unsigned start = range->start * 8 * sizeof(float);
      struct pipe_transfer *transfer;

      cmap = pipe_buffer_map_range(&ice->ctx, ice->state.shaders[stage].constbufs[block_index].buffer,
                                   ice->state.shaders[stage].constbufs[block_index].buffer_offset + start, len,
                                   PIPE_MAP_READ | PIPE_MAP_UNSYNCHRONIZED, &transfer);
      if (cmap)
         memcpy(&map[offset + (total * 8)], cmap, len);
      pipe_buffer_unmap(&ice->ctx, transfer);
      total += range->length;
      found = true;
   }

   if (stage == MESA_SHADER_VERTEX && !found) {
      /* The pre-gen6 VS requires that some push constants get loaded no
       * matter what, or the GPU would hang.
       */
      unsigned len = 16;
      memset(&map[offset], 0, len);
   }
}

static const float fixed_plane[6][4] = {
   { 0,    0,   -1, 1 },
   { 0,    0,    1, 1 },
   { 0,   -1,    0, 1 },
   { 0,    1,    0, 1 },
   {-1,    0,    0, 1 },
   { 1,    0,    0, 1 }
};

static void
gen4_upload_curbe(struct crocus_batch *batch)
{
   struct crocus_context *ice = batch->ice;
   const unsigned sz = ice->curbe.total_size;
   const unsigned buf_sz = sz * 16 * sizeof(float);

   if (sz == 0)
      goto emit;

   uint32_t *map;
   u_upload_alloc(ice->ctx.const_uploader, 0, buf_sz, 64,
                  &ice->curbe.curbe_offset, (struct pipe_resource **)&ice->curbe.curbe_res, (void **) &map);

   /* fragment shader constants */
   if (ice->curbe.wm_size) {
      upload_shader_consts(ice, MESA_SHADER_FRAGMENT, map, ice->curbe.wm_start);
   }

   /* clipper constants */
   if (ice->curbe.clip_size) {
      unsigned offset = ice->curbe.clip_start * 16;
      float *fmap = (float *)map;
      unsigned i;
      /* If any planes are going this way, send them all this way:
       */
      for (i = 0; i < 6; i++) {
         fmap[offset + i * 4 + 0] = fixed_plane[i][0];
         fmap[offset + i * 4 + 1] = fixed_plane[i][1];
         fmap[offset + i * 4 + 2] = fixed_plane[i][2];
         fmap[offset + i * 4 + 3] = fixed_plane[i][3];
      }

      unsigned mask = ice->state.cso_rast->cso.clip_plane_enable;
      struct pipe_clip_state *cp = &ice->state.clip_planes;
      while (mask) {
         const int j = u_bit_scan(&mask);
         fmap[offset + i * 4 + 0] = cp->ucp[j][0];
         fmap[offset + i * 4 + 1] = cp->ucp[j][1];
         fmap[offset + i * 4 + 2] = cp->ucp[j][2];
         fmap[offset + i * 4 + 3] = cp->ucp[j][3];
         i++;
      }
   }

   /* vertex shader constants */
   if (ice->curbe.vs_size) {
      upload_shader_consts(ice, MESA_SHADER_VERTEX, map, ice->curbe.vs_start);
   }
   if (0) {
      for (int i = 0; i < sz*16; i+=4) {
         float *f = (float *)map;
         fprintf(stderr, "curbe %d.%d: %f %f %f %f\n", i/8, i&4,
                 f[i+0], f[i+1], f[i+2], f[i+3]);
      }
   }

emit:
   crocus_emit_cmd(batch, GENX(CONSTANT_BUFFER), cb) {
      if (ice->curbe.curbe_res) {
         cb.BufferLength = ice->curbe.total_size - 1;
         cb.Valid = 1;
         cb.BufferStartingAddress = ro_bo(ice->curbe.curbe_res->bo, ice->curbe.curbe_offset);
      }
   }

#if GFX_VER == 4 && GFX_VERx10 != 45
   /* Work around a Broadwater/Crestline depth interpolator bug.  The
    * following sequence will cause GPU hangs:
    *
    * 1. Change state so that all depth related fields in CC_STATE are
    *    disabled, and in WM_STATE, only "PS Use Source Depth" is enabled.
    * 2. Emit a CONSTANT_BUFFER packet.
    * 3. Draw via 3DPRIMITIVE.
    *
    * The recommended workaround is to emit a non-pipelined state change after
    * emitting CONSTANT_BUFFER, in order to drain the windowizer pipeline.
    *
    * We arbitrarily choose 3DSTATE_GLOBAL_DEPTH_CLAMP_OFFSET (as it's small),
    * and always emit it when "PS Use Source Depth" is set.  We could be more
    * precise, but the additional complexity is probably not worth it.
    *
    */
   const struct shader_info *fs_info =
      crocus_get_shader_info(ice, MESA_SHADER_FRAGMENT);

   if (BITSET_TEST(fs_info->system_values_read, SYSTEM_VALUE_FRAG_COORD)) {
      ice->state.global_depth_offset_clamp = 0;
      crocus_emit_cmd(batch, GENX(3DSTATE_GLOBAL_DEPTH_OFFSET_CLAMP), clamp);
   }
#endif
}
#endif

#if GFX_VER >= 7

#define IVB_L3SQCREG1_SQGHPCI_DEFAULT     0x00730000
#define VLV_L3SQCREG1_SQGHPCI_DEFAULT     0x00d30000
#define HSW_L3SQCREG1_SQGHPCI_DEFAULT     0x00610000

static void
setup_l3_config(struct crocus_batch *batch, const struct intel_l3_config *cfg)
{
#if GFX_VER == 7
   const struct intel_device_info *devinfo = &batch->screen->devinfo;
   const bool has_dc = cfg->n[INTEL_L3P_DC] || cfg->n[INTEL_L3P_ALL];
   const bool has_is = cfg->n[INTEL_L3P_IS] || cfg->n[INTEL_L3P_RO] ||
                       cfg->n[INTEL_L3P_ALL];
   const bool has_c = cfg->n[INTEL_L3P_C] || cfg->n[INTEL_L3P_RO] ||
                      cfg->n[INTEL_L3P_ALL];
   const bool has_t = cfg->n[INTEL_L3P_T] || cfg->n[INTEL_L3P_RO] ||
                      cfg->n[INTEL_L3P_ALL];
   const bool has_slm = cfg->n[INTEL_L3P_SLM];
#endif

   /* According to the hardware docs, the L3 partitioning can only be changed
    * while the pipeline is completely drained and the caches are flushed,
    * which involves a first PIPE_CONTROL flush which stalls the pipeline...
    */
   crocus_emit_pipe_control_flush(batch, "l3_config",
                                  PIPE_CONTROL_DATA_CACHE_FLUSH |
                                  PIPE_CONTROL_CS_STALL);

   /* ...followed by a second pipelined PIPE_CONTROL that initiates
    * invalidation of the relevant caches.  Note that because RO invalidation
    * happens at the top of the pipeline (i.e. right away as the PIPE_CONTROL
    * command is processed by the CS) we cannot combine it with the previous
    * stalling flush as the hardware documentation suggests, because that
    * would cause the CS to stall on previous rendering *after* RO
    * invalidation and wouldn't prevent the RO caches from being polluted by
    * concurrent rendering before the stall completes.  This intentionally
    * doesn't implement the SKL+ hardware workaround suggesting to enable CS
    * stall on PIPE_CONTROLs with the texture cache invalidation bit set for
    * GPGPU workloads because the previous and subsequent PIPE_CONTROLs
    * already guarantee that there is no concurrent GPGPU kernel execution
    * (see SKL HSD 2132585).
    */
   crocus_emit_pipe_control_flush(batch, "l3 config",
                                  PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
                                  PIPE_CONTROL_CONST_CACHE_INVALIDATE |
                                  PIPE_CONTROL_INSTRUCTION_INVALIDATE |
                                  PIPE_CONTROL_STATE_CACHE_INVALIDATE);

   /* Now send a third stalling flush to make sure that invalidation is
    * complete when the L3 configuration registers are modified.
    */
   crocus_emit_pipe_control_flush(batch, "l3 config",
                                  PIPE_CONTROL_DATA_CACHE_FLUSH |
                                  PIPE_CONTROL_CS_STALL);

#if GFX_VER == 8
   assert(!cfg->n[INTEL_L3P_IS] && !cfg->n[INTEL_L3P_C] && !cfg->n[INTEL_L3P_T]);
   crocus_emit_reg(batch, GENX(L3CNTLREG), reg) {
      reg.SLMEnable = cfg->n[INTEL_L3P_SLM] > 0;
      reg.URBAllocation = cfg->n[INTEL_L3P_URB];
      reg.ROAllocation = cfg->n[INTEL_L3P_RO];
      reg.DCAllocation = cfg->n[INTEL_L3P_DC];
      reg.AllAllocation = cfg->n[INTEL_L3P_ALL];
   }
#else
   assert(!cfg->n[INTEL_L3P_ALL]);

   /* When enabled SLM only uses a portion of the L3 on half of the banks,
    * the matching space on the remaining banks has to be allocated to a
    * client (URB for all validated configurations) set to the
    * lower-bandwidth 2-bank address hashing mode.
    */
   const bool urb_low_bw = has_slm && devinfo->platform != INTEL_PLATFORM_BYT;
   assert(!urb_low_bw || cfg->n[INTEL_L3P_URB] == cfg->n[INTEL_L3P_SLM]);

   /* Minimum number of ways that can be allocated to the URB. */
   const unsigned n0_urb = (devinfo->platform == INTEL_PLATFORM_BYT ? 32 : 0);
   assert(cfg->n[INTEL_L3P_URB] >= n0_urb);

   uint32_t l3sqcr1, l3cr2, l3cr3;

   crocus_pack_state(GENX(L3SQCREG1), &l3sqcr1, reg) {
      reg.ConvertDC_UC = !has_dc;
      reg.ConvertIS_UC = !has_is;
      reg.ConvertC_UC = !has_c;
      reg.ConvertT_UC = !has_t;
#if GFX_VERx10 == 75
      reg.L3SQGeneralPriorityCreditInitialization = SQGPCI_DEFAULT;
#else
      reg.L3SQGeneralPriorityCreditInitialization =
         devinfo->platform == INTEL_PLATFORM_BYT ? BYT_SQGPCI_DEFAULT : SQGPCI_DEFAULT;
#endif
      reg.L3SQHighPriorityCreditInitialization = SQHPCI_DEFAULT;
   };

   crocus_pack_state(GENX(L3CNTLREG2), &l3cr2, reg) {
      reg.SLMEnable = has_slm;
      reg.URBLowBandwidth = urb_low_bw;
      reg.URBAllocation = cfg->n[INTEL_L3P_URB] - n0_urb;
#if !(GFX_VERx10 == 75)
      reg.ALLAllocation = cfg->n[INTEL_L3P_ALL];
#endif
      reg.ROAllocation = cfg->n[INTEL_L3P_RO];
      reg.DCAllocation = cfg->n[INTEL_L3P_DC];
   };

   crocus_pack_state(GENX(L3CNTLREG3), &l3cr3, reg) {
      reg.ISAllocation = cfg->n[INTEL_L3P_IS];
      reg.ISLowBandwidth = 0;
      reg.CAllocation = cfg->n[INTEL_L3P_C];
      reg.CLowBandwidth = 0;
      reg.TAllocation = cfg->n[INTEL_L3P_T];
      reg.TLowBandwidth = 0;
   };

   /* Set up the L3 partitioning. */
   crocus_emit_lri(batch, L3SQCREG1, l3sqcr1);
   crocus_emit_lri(batch, L3CNTLREG2, l3cr2);
   crocus_emit_lri(batch, L3CNTLREG3, l3cr3);

#if GFX_VERx10 == 75
   /* TODO: Fail screen creation if command parser version < 4 */
   uint32_t scratch1, chicken3;
   crocus_pack_state(GENX(SCRATCH1), &scratch1, reg) {
      reg.L3AtomicDisable = !has_dc;
   }
   crocus_pack_state(GENX(CHICKEN3), &chicken3, reg) {
      reg.L3AtomicDisableMask = true;
      reg.L3AtomicDisable = !has_dc;
   }
   crocus_emit_lri(batch, SCRATCH1, scratch1);
   crocus_emit_lri(batch, CHICKEN3, chicken3);
#endif
#endif
}

static void
emit_l3_state(struct crocus_batch *batch, bool compute)
{
   const struct intel_l3_config *const cfg =
      compute ? batch->screen->l3_config_cs : batch->screen->l3_config_3d;

   setup_l3_config(batch, cfg);
   if (INTEL_DEBUG(DEBUG_L3)) {
      intel_dump_l3_config(cfg, stderr);
   }
}

/**
 * Emit a PIPE_CONTROL command for gen7 with the CS Stall bit set.
 */
static void
gen7_emit_cs_stall_flush(struct crocus_batch *batch)
{
   crocus_emit_pipe_control_write(batch,
                                  "workaround",
                                  PIPE_CONTROL_CS_STALL
                                  | PIPE_CONTROL_WRITE_IMMEDIATE,
                                  batch->ice->workaround_bo,
                                  batch->ice->workaround_offset, 0);
}
#endif

static void
emit_pipeline_select(struct crocus_batch *batch, uint32_t pipeline)
{
#if GFX_VER == 8
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
      crocus_emit_cmd(batch, GENX(3DSTATE_CC_STATE_POINTERS), t);
#endif

#if GFX_VER >= 6
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
   const unsigned dc_flush =
      GFX_VER >= 7 ? PIPE_CONTROL_DATA_CACHE_FLUSH : 0;
   crocus_emit_pipe_control_flush(batch,
                                  "workaround: PIPELINE_SELECT flushes (1/2)",
                                  PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                  PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                  dc_flush |
                                  PIPE_CONTROL_CS_STALL);

   crocus_emit_pipe_control_flush(batch,
                                  "workaround: PIPELINE_SELECT flushes (2/2)",
                                  PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE |
                                  PIPE_CONTROL_CONST_CACHE_INVALIDATE |
                                  PIPE_CONTROL_STATE_CACHE_INVALIDATE |
                                  PIPE_CONTROL_INSTRUCTION_INVALIDATE);
#else
   /* From "BXML » GT » MI » vol1a GPU Overview » [Instruction]
    * PIPELINE_SELECT [DevBWR+]":
    *
    *   Project: PRE-DEVSNB
    *
    *   Software must ensure the current pipeline is flushed via an
    *   MI_FLUSH or PIPE_CONTROL prior to the execution of PIPELINE_SELECT.
    */
   crocus_emit_cmd(batch, GENX(MI_FLUSH), foo);
#endif

   crocus_emit_cmd(batch, GENX(PIPELINE_SELECT), sel) {
      sel.PipelineSelection = pipeline;
   }

#if GFX_VER == 7 && !(GFX_VERx10 == 75)
   if (pipeline == _3D) {
      gen7_emit_cs_stall_flush(batch);

      crocus_emit_cmd(batch, GENX(3DPRIMITIVE), prim) {
         prim.PrimitiveTopologyType = _3DPRIM_POINTLIST;
      };
   }
#endif
}

/**
 * The following diagram shows how we partition the URB:
 *
 *        16kB or 32kB               Rest of the URB space
 *   __________-__________   _________________-_________________
 *  /                     \ /                                   \
 * +-------------------------------------------------------------+
 * |  VS/HS/DS/GS/FS Push  |           VS/HS/DS/GS URB           |
 * |       Constants       |               Entries               |
 * +-------------------------------------------------------------+
 *
 * Notably, push constants must be stored at the beginning of the URB
 * space, while entries can be stored anywhere.  Ivybridge and Haswell
 * GT1/GT2 have a maximum constant buffer size of 16kB, while Haswell GT3
 * doubles this (32kB).
 *
 * Ivybridge and Haswell GT1/GT2 allow push constants to be located (and
 * sized) in increments of 1kB.  Haswell GT3 requires them to be located and
 * sized in increments of 2kB.
 *
 * Currently we split the constant buffer space evenly among whatever stages
 * are active.  This is probably not ideal, but simple.
 *
 * Ivybridge GT1 and Haswell GT1 have 128kB of URB space.
 * Ivybridge GT2 and Haswell GT2 have 256kB of URB space.
 * Haswell GT3 has 512kB of URB space.
 *
 * See "Volume 2a: 3D Pipeline," section 1.8, "Volume 1b: Configurations",
 * and the documentation for 3DSTATE_PUSH_CONSTANT_ALLOC_xS.
 */
#if GFX_VER >= 7
static void
crocus_alloc_push_constants(struct crocus_batch *batch)
{
   const unsigned push_constant_kb =
      batch->screen->devinfo.max_constant_urb_size_kb;
   unsigned size_per_stage = push_constant_kb / 5;

   /* For now, we set a static partitioning of the push constant area,
    * assuming that all stages could be in use.
    *
    * TODO: Try lazily allocating the HS/DS/GS sections as needed, and
    *       see if that improves performance by offering more space to
    *       the VS/FS when those aren't in use.  Also, try dynamically
    *       enabling/disabling it like i965 does.  This would be more
    *       stalls and may not actually help; we don't know yet.
    */
   for (int i = 0; i <= MESA_SHADER_FRAGMENT; i++) {
      crocus_emit_cmd(batch, GENX(3DSTATE_PUSH_CONSTANT_ALLOC_VS), alloc) {
         alloc._3DCommandSubOpcode = 18 + i;
         alloc.ConstantBufferOffset = size_per_stage * i;
         alloc.ConstantBufferSize = i == MESA_SHADER_FRAGMENT ? (push_constant_kb - 4 * size_per_stage) : size_per_stage;
      }
   }

   /* From p292 of the Ivy Bridge PRM (11.2.4 3DSTATE_PUSH_CONSTANT_ALLOC_PS):
    *
    *     A PIPE_CONTROL command with the CS Stall bit set must be programmed
    *     in the ring after this instruction.
    *
    * No such restriction exists for Haswell or Baytrail.
    */
   if (batch->screen->devinfo.platform == INTEL_PLATFORM_IVB)
      gen7_emit_cs_stall_flush(batch);
}
#endif

/**
 * Upload the initial GPU state for a render context.
 *
 * This sets some invariant state that needs to be programmed a particular
 * way, but we never actually change.
 */
static void
crocus_init_render_context(struct crocus_batch *batch)
{
   UNUSED const struct intel_device_info *devinfo = &batch->screen->devinfo;

   emit_pipeline_select(batch, _3D);

   crocus_emit_cmd(batch, GENX(STATE_SIP), foo);

#if GFX_VER >= 7
   emit_l3_state(batch, false);
#endif
#if (GFX_VERx10 == 70 || GFX_VERx10 == 80)
   crocus_emit_reg(batch, GENX(INSTPM), reg) {
      reg.CONSTANT_BUFFERAddressOffsetDisable = true;
      reg.CONSTANT_BUFFERAddressOffsetDisableMask = true;
   }
#endif
#if GFX_VER >= 5 || GFX_VERx10 == 45
   /* Use the legacy AA line coverage computation. */
   crocus_emit_cmd(batch, GENX(3DSTATE_AA_LINE_PARAMETERS), foo);
#endif

   /* No polygon stippling offsets are necessary. */
   /* TODO: may need to set an offset for origin-UL framebuffers */
   crocus_emit_cmd(batch, GENX(3DSTATE_POLY_STIPPLE_OFFSET), foo);

#if GFX_VER >= 7
   crocus_alloc_push_constants(batch);
#endif

#if GFX_VER == 8
   /* Set the initial MSAA sample positions. */
   crocus_emit_cmd(batch, GENX(3DSTATE_SAMPLE_PATTERN), pat) {
      INTEL_SAMPLE_POS_1X(pat._1xSample);
      INTEL_SAMPLE_POS_2X(pat._2xSample);
      INTEL_SAMPLE_POS_4X(pat._4xSample);
      INTEL_SAMPLE_POS_8X(pat._8xSample);
   }

   /* Disable chromakeying (it's for media) */
   crocus_emit_cmd(batch, GENX(3DSTATE_WM_CHROMAKEY), foo);

   /* We want regular rendering, not special HiZ operations. */
   crocus_emit_cmd(batch, GENX(3DSTATE_WM_HZ_OP), foo);
#endif
}

#if GFX_VER >= 7
static void
crocus_init_compute_context(struct crocus_batch *batch)
{
   UNUSED const struct intel_device_info *devinfo = &batch->screen->devinfo;

   emit_pipeline_select(batch, GPGPU);

#if GFX_VER >= 7
   emit_l3_state(batch, true);
#endif
}
#endif

/**
 * Generation-specific context state (ice->state.genx->...).
 *
 * Most state can go in crocus_context directly, but these encode hardware
 * packets which vary by generation.
 */
struct crocus_genx_state {
   struct {
#if GFX_VER >= 7
      struct brw_image_param image_param[PIPE_MAX_SHADER_IMAGES];
#endif
   } shaders[MESA_SHADER_STAGES];

#if GFX_VER == 8
   bool pma_fix_enabled;
#endif
};

/**
 * The pipe->set_blend_color() driver hook.
 *
 * This corresponds to our COLOR_CALC_STATE.
 */
static void
crocus_set_blend_color(struct pipe_context *ctx,
                       const struct pipe_blend_color *state)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;

   /* Our COLOR_CALC_STATE is exactly pipe_blend_color, so just memcpy */
   memcpy(&ice->state.blend_color, state, sizeof(struct pipe_blend_color));
#if GFX_VER <= 5
   ice->state.dirty |= CROCUS_DIRTY_GEN4_CONSTANT_COLOR;
#else
   ice->state.dirty |= CROCUS_DIRTY_COLOR_CALC_STATE;
#endif
}

/**
 * Gallium CSO for blend state (see pipe_blend_state).
 */
struct crocus_blend_state {
#if GFX_VER == 8
   /** Partial 3DSTATE_PS_BLEND */
   uint32_t ps_blend[GENX(3DSTATE_PS_BLEND_length)];
#endif

   /** copy of BLEND_STATE */
   struct pipe_blend_state cso;

   /** Bitfield of whether blending is enabled for RT[i] - for aux resolves */
   uint8_t blend_enables;

   /** Bitfield of whether color writes are enabled for RT[i] */
   uint8_t color_write_enables;

   /** Does RT[0] use dual color blending? */
   bool dual_color_blending;
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

#if GFX_VER >= 6
typedef struct GENX(BLEND_STATE_ENTRY) BLEND_ENTRY_GENXML;
#else
typedef struct GENX(COLOR_CALC_STATE) BLEND_ENTRY_GENXML;
#endif

static bool
can_emit_logic_op(struct crocus_context *ice)
{
   /* all pre gen8 have logicop restricted to unorm */
   enum pipe_format pformat = PIPE_FORMAT_NONE;
   for (unsigned i = 0; i < ice->state.framebuffer.nr_cbufs; i++) {
      if (ice->state.framebuffer.cbufs[i]) {
         pformat = ice->state.framebuffer.cbufs[i]->format;
         break;
      }
   }
   return (pformat == PIPE_FORMAT_NONE || util_format_is_unorm(pformat));
}

static bool
set_blend_entry_bits(struct crocus_batch *batch, BLEND_ENTRY_GENXML *entry,
                     struct crocus_blend_state *cso_blend,
                     int idx)
{
   struct crocus_context *ice = batch->ice;
   bool independent_alpha_blend = false;
   const struct pipe_rt_blend_state *rt =
      &cso_blend->cso.rt[cso_blend->cso.independent_blend_enable ? idx : 0];
   const unsigned blend_enabled = rt->blend_enable;

   enum pipe_blendfactor src_rgb =
      fix_blendfactor(rt->rgb_src_factor, cso_blend->cso.alpha_to_one);
   enum pipe_blendfactor src_alpha =
      fix_blendfactor(rt->alpha_src_factor, cso_blend->cso.alpha_to_one);
   enum pipe_blendfactor dst_rgb =
      fix_blendfactor(rt->rgb_dst_factor, cso_blend->cso.alpha_to_one);
   enum pipe_blendfactor dst_alpha =
      fix_blendfactor(rt->alpha_dst_factor, cso_blend->cso.alpha_to_one);

   if (rt->rgb_func != rt->alpha_func ||
       src_rgb != src_alpha || dst_rgb != dst_alpha)
      independent_alpha_blend = true;
   if (cso_blend->cso.logicop_enable) {
      if (GFX_VER >= 8 || can_emit_logic_op(ice)) {
         entry->LogicOpEnable = cso_blend->cso.logicop_enable;
         entry->LogicOpFunction = cso_blend->cso.logicop_func;
      }
   } else if (blend_enabled) {
      if (idx == 0) {
         struct crocus_compiled_shader *shader = ice->shaders.prog[MESA_SHADER_FRAGMENT];
         struct brw_wm_prog_data *wm_prog_data = (void *) shader->prog_data;
         entry->ColorBufferBlendEnable =
            (!cso_blend->dual_color_blending || wm_prog_data->dual_src_blend);
      } else
         entry->ColorBufferBlendEnable = 1;

      entry->ColorBlendFunction          = rt->rgb_func;
      entry->AlphaBlendFunction          = rt->alpha_func;
      entry->SourceBlendFactor           = (int) src_rgb;
      entry->SourceAlphaBlendFactor      = (int) src_alpha;
      entry->DestinationBlendFactor      = (int) dst_rgb;
      entry->DestinationAlphaBlendFactor = (int) dst_alpha;
   }
#if GFX_VER <= 5
   /*
    * Gen4/GM45/ILK can't handle have ColorBufferBlendEnable == 0
    * when a dual src blend shader is in use. Setup dummy blending.
    */
   struct crocus_compiled_shader *shader = ice->shaders.prog[MESA_SHADER_FRAGMENT];
   struct brw_wm_prog_data *wm_prog_data = (void *) shader->prog_data;
   if (idx == 0 && !blend_enabled && wm_prog_data->dual_src_blend) {
      entry->ColorBufferBlendEnable = 1;
      entry->ColorBlendFunction = PIPE_BLEND_ADD;
      entry->AlphaBlendFunction = PIPE_BLEND_ADD;
      entry->SourceBlendFactor = PIPE_BLENDFACTOR_ONE;
      entry->SourceAlphaBlendFactor = PIPE_BLENDFACTOR_ONE;
      entry->DestinationBlendFactor = PIPE_BLENDFACTOR_ZERO;
      entry->DestinationAlphaBlendFactor = PIPE_BLENDFACTOR_ZERO;
   }
#endif
   return independent_alpha_blend;
}

/**
 * The pipe->create_blend_state() driver hook.
 *
 * Translates a pipe_blend_state into crocus_blend_state.
 */
static void *
crocus_create_blend_state(struct pipe_context *ctx,
                          const struct pipe_blend_state *state)
{
   struct crocus_blend_state *cso = malloc(sizeof(struct crocus_blend_state));

   cso->blend_enables = 0;
   cso->color_write_enables = 0;
   STATIC_ASSERT(BRW_MAX_DRAW_BUFFERS <= 8);

   cso->cso = *state;
   cso->dual_color_blending = util_blend_state_is_dual(state, 0);

#if GFX_VER == 8
   bool indep_alpha_blend = false;
#endif
   for (int i = 0; i < BRW_MAX_DRAW_BUFFERS; i++) {
      const struct pipe_rt_blend_state *rt =
         &state->rt[state->independent_blend_enable ? i : 0];
      if (rt->blend_enable)
         cso->blend_enables |= 1u << i;
      if (rt->colormask)
         cso->color_write_enables |= 1u << i;
#if GFX_VER == 8
      enum pipe_blendfactor src_rgb =
         fix_blendfactor(rt->rgb_src_factor, state->alpha_to_one);
      enum pipe_blendfactor src_alpha =
         fix_blendfactor(rt->alpha_src_factor, state->alpha_to_one);
      enum pipe_blendfactor dst_rgb =
         fix_blendfactor(rt->rgb_dst_factor, state->alpha_to_one);
      enum pipe_blendfactor dst_alpha =
         fix_blendfactor(rt->alpha_dst_factor, state->alpha_to_one);

      if (rt->rgb_func != rt->alpha_func ||
          src_rgb != src_alpha || dst_rgb != dst_alpha)
         indep_alpha_blend = true;
#endif
   }

#if GFX_VER == 8
   crocus_pack_command(GENX(3DSTATE_PS_BLEND), cso->ps_blend, pb) {
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
      pb.DestinationBlendFactor =
         (int) fix_blendfactor(state->rt[0].rgb_dst_factor, state->alpha_to_one);
      pb.DestinationAlphaBlendFactor =
         (int) fix_blendfactor(state->rt[0].alpha_dst_factor, state->alpha_to_one);
   }
#endif
   return cso;
}

/**
 * The pipe->bind_blend_state() driver hook.
 *
 * Bind a blending CSO and flag related dirty bits.
 */
static void
crocus_bind_blend_state(struct pipe_context *ctx, void *state)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_blend_state *cso = state;

   ice->state.cso_blend = cso;
   ice->state.blend_enables = cso ? cso->blend_enables : 0;

   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_BINDINGS_FS;
   ice->state.dirty |= CROCUS_DIRTY_WM;
#if GFX_VER >= 6
   ice->state.dirty |= CROCUS_DIRTY_GEN6_BLEND_STATE;
#endif
#if GFX_VER >= 7
   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_FS;
#endif
#if GFX_VER == 8
   ice->state.dirty |= CROCUS_DIRTY_GEN8_PMA_FIX;
   ice->state.dirty |= CROCUS_DIRTY_GEN8_PS_BLEND;
#endif
   ice->state.dirty |= CROCUS_DIRTY_COLOR_CALC_STATE;
   ice->state.dirty |= CROCUS_DIRTY_RENDER_RESOLVES_AND_FLUSHES;
   ice->state.stage_dirty |= ice->state.stage_dirty_for_nos[CROCUS_NOS_BLEND];
}

/**
 * Return true if the FS writes to any color outputs which are not disabled
 * via color masking.
 */
static bool
has_writeable_rt(const struct crocus_blend_state *cso_blend,
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
struct crocus_depth_stencil_alpha_state {
   struct pipe_depth_stencil_alpha_state cso;

   bool depth_writes_enabled;
   bool stencil_writes_enabled;
};

/**
 * The pipe->create_depth_stencil_alpha_state() driver hook.
 *
 * We encode most of 3DSTATE_WM_DEPTH_STENCIL, and just save off the alpha
 * testing state since we need pieces of it in a variety of places.
 */
static void *
crocus_create_zsa_state(struct pipe_context *ctx,
                        const struct pipe_depth_stencil_alpha_state *state)
{
   struct crocus_depth_stencil_alpha_state *cso =
      malloc(sizeof(struct crocus_depth_stencil_alpha_state));

   bool two_sided_stencil = state->stencil[1].enabled;
   cso->cso = *state;

   cso->depth_writes_enabled = state->depth_writemask;
   cso->stencil_writes_enabled =
      state->stencil[0].writemask != 0 ||
      (two_sided_stencil && state->stencil[1].writemask != 0);

   /* The state tracker needs to optimize away EQUAL writes for us. */
   assert(!(state->depth_func == PIPE_FUNC_EQUAL && state->depth_writemask));

   return cso;
}

/**
 * The pipe->bind_depth_stencil_alpha_state() driver hook.
 *
 * Bind a depth/stencil/alpha CSO and flag related dirty bits.
 */
static void
crocus_bind_zsa_state(struct pipe_context *ctx, void *state)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_depth_stencil_alpha_state *old_cso = ice->state.cso_zsa;
   struct crocus_depth_stencil_alpha_state *new_cso = state;

   if (new_cso) {
      if (cso_changed(cso.alpha_ref_value))
         ice->state.dirty |= CROCUS_DIRTY_COLOR_CALC_STATE;

      if (cso_changed(cso.alpha_enabled))
         ice->state.dirty |= CROCUS_DIRTY_WM;
#if GFX_VER >= 6
      if (cso_changed(cso.alpha_enabled))
         ice->state.dirty |= CROCUS_DIRTY_GEN6_BLEND_STATE;

      if (cso_changed(cso.alpha_func))
         ice->state.dirty |= CROCUS_DIRTY_GEN6_BLEND_STATE;
#endif
#if GFX_VER == 8
      if (cso_changed(cso.alpha_enabled))
         ice->state.dirty |= CROCUS_DIRTY_GEN8_PS_BLEND;
#endif

      if (cso_changed(depth_writes_enabled))
         ice->state.dirty |= CROCUS_DIRTY_RENDER_RESOLVES_AND_FLUSHES;

      ice->state.depth_writes_enabled = new_cso->depth_writes_enabled;
      ice->state.stencil_writes_enabled = new_cso->stencil_writes_enabled;

#if GFX_VER <= 5
      ice->state.dirty |= CROCUS_DIRTY_COLOR_CALC_STATE;
#endif
   }

   ice->state.cso_zsa = new_cso;
   ice->state.dirty |= CROCUS_DIRTY_CC_VIEWPORT;
#if GFX_VER >= 6
   ice->state.dirty |= CROCUS_DIRTY_GEN6_WM_DEPTH_STENCIL;
#endif
#if GFX_VER == 8
   ice->state.dirty |= CROCUS_DIRTY_GEN8_PMA_FIX;
#endif
   ice->state.stage_dirty |= ice->state.stage_dirty_for_nos[CROCUS_NOS_DEPTH_STENCIL_ALPHA];
}

#if GFX_VER == 8
static bool
want_pma_fix(struct crocus_context *ice)
{
   UNUSED struct crocus_screen *screen = (void *) ice->ctx.screen;
   UNUSED const struct intel_device_info *devinfo = &screen->devinfo;
   const struct brw_wm_prog_data *wm_prog_data = (void *)
      ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data;
   const struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
   const struct crocus_depth_stencil_alpha_state *cso_zsa = ice->state.cso_zsa;
   const struct crocus_blend_state *cso_blend = ice->state.cso_blend;

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

   struct crocus_resource *zres, *sres;
   crocus_get_depth_stencil_resources(devinfo,
                                      cso_fb->zsbuf->texture, &zres, &sres);

   /* 3DSTATE_DEPTH_BUFFER::SURFACE_TYPE != NULL &&
    * 3DSTATE_DEPTH_BUFFER::HIZ Enable &&
    */
   if (!zres || !crocus_resource_level_has_hiz(zres, cso_fb->zsbuf->u.tex.level))
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
                     cso_blend->cso.alpha_to_coverage || cso_zsa->cso.alpha_enabled;

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
   if (!cso_zsa->cso.depth_enabled)
      return false;

   return wm_prog_data->computed_depth_mode != PSCDEPTH_OFF ||
          (killpixels && (cso_zsa->depth_writes_enabled ||
                          (sres && cso_zsa->stencil_writes_enabled)));
}
#endif
void
genX(crocus_update_pma_fix)(struct crocus_context *ice,
                            struct crocus_batch *batch,
                            bool enable)
{
#if GFX_VER == 8
   struct crocus_genx_state *genx = ice->state.genx;

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
   crocus_emit_pipe_control_flush(batch, "PMA fix change (1/2)",
                                  PIPE_CONTROL_CS_STALL |
                                  PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                  PIPE_CONTROL_RENDER_TARGET_FLUSH);

   crocus_emit_reg(batch, GENX(CACHE_MODE_1), reg) {
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
   crocus_emit_pipe_control_flush(batch, "PMA fix change (1/2)",
                                  PIPE_CONTROL_DEPTH_STALL |
                                  PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                                  PIPE_CONTROL_RENDER_TARGET_FLUSH);
#endif
}

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
      /* hack around this for gfx4/5 fps counters in hud. */
      line_width = GFX_VER < 6 ? 1.5f : 0.0f;
   }
   return line_width;
}

/**
 * The pipe->create_rasterizer_state() driver hook.
 */
static void *
crocus_create_rasterizer_state(struct pipe_context *ctx,
                               const struct pipe_rasterizer_state *state)
{
   struct crocus_rasterizer_state *cso =
      malloc(sizeof(struct crocus_rasterizer_state));

   cso->fill_mode_point_or_line =
      state->fill_front == PIPE_POLYGON_MODE_LINE ||
      state->fill_front == PIPE_POLYGON_MODE_POINT ||
      state->fill_back == PIPE_POLYGON_MODE_LINE ||
      state->fill_back == PIPE_POLYGON_MODE_POINT;

   if (state->clip_plane_enable != 0)
      cso->num_clip_plane_consts = util_logbase2(state->clip_plane_enable) + 1;
   else
      cso->num_clip_plane_consts = 0;

   cso->cso = *state;

#if GFX_VER >= 6
   float line_width = get_line_width(state);

   crocus_pack_command(GENX(3DSTATE_SF), cso->sf, sf) {
      sf.StatisticsEnable = true;
      sf.AALineDistanceMode = AALINEDISTANCE_TRUE;
      sf.LineEndCapAntialiasingRegionWidth =
         state->line_smooth ? _10pixels : _05pixels;
      sf.LastPixelEnable = state->line_last_pixel;
#if GFX_VER <= 7
      sf.AntialiasingEnable = state->line_smooth;
#endif
#if GFX_VER == 8
      struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
      if (screen->devinfo.platform == INTEL_PLATFORM_CHV)
         sf.CHVLineWidth = line_width;
      else
         sf.LineWidth = line_width;
#else
      sf.LineWidth = line_width;
#endif
      sf.PointWidthSource = state->point_size_per_vertex ? Vertex : State;
      sf.PointWidth = state->point_size;

      if (state->flatshade_first) {
         sf.TriangleFanProvokingVertexSelect = 1;
      } else {
         sf.TriangleStripListProvokingVertexSelect = 2;
         sf.TriangleFanProvokingVertexSelect = 2;
         sf.LineStripListProvokingVertexSelect = 1;
      }

#if GFX_VER == 6
      sf.AttributeSwizzleEnable = true;
      if (state->sprite_coord_mode == PIPE_SPRITE_COORD_LOWER_LEFT)
         sf.PointSpriteTextureCoordinateOrigin = LOWERLEFT;
      else
         sf.PointSpriteTextureCoordinateOrigin = UPPERLEFT;
#endif

#if GFX_VER <= 7
      sf.FrontWinding = state->front_ccw ? 1 : 0; // Or the other way...

#if GFX_VER >= 6
      sf.GlobalDepthOffsetEnableSolid = state->offset_tri;
      sf.GlobalDepthOffsetEnableWireframe = state->offset_line;
      sf.GlobalDepthOffsetEnablePoint = state->offset_point;
      sf.GlobalDepthOffsetConstant = state->offset_units * 2;
      sf.GlobalDepthOffsetScale = state->offset_scale;
      sf.GlobalDepthOffsetClamp = state->offset_clamp;

      sf.FrontFaceFillMode = translate_fill_mode(state->fill_front);
      sf.BackFaceFillMode = translate_fill_mode(state->fill_back);
#endif

      sf.CullMode = translate_cull_mode(state->cull_face);
      sf.ScissorRectangleEnable = true;

#if GFX_VERx10 == 75
      sf.LineStippleEnable = state->line_stipple_enable;
#endif
#endif
   }
#endif

#if GFX_VER == 8
   crocus_pack_command(GENX(3DSTATE_RASTER), cso->raster, rr) {
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
      rr.AntialiasingEnable = state->line_smooth;
      rr.ScissorRectangleEnable = state->scissor;
      rr.ViewportZClipTestEnable = (state->depth_clip_near || state->depth_clip_far);
   }
#endif

#if GFX_VER >= 6
   crocus_pack_command(GENX(3DSTATE_CLIP), cso->clip, cl) {
      /* cl.NonPerspectiveBarycentricEnable is filled in at draw time from
       * the FS program; cl.ForceZeroRTAIndexEnable is filled in from the FB.
       */
#if GFX_VER >= 7
      cl.EarlyCullEnable = true;
#endif

#if GFX_VER == 7
      cl.FrontWinding = state->front_ccw ? 1 : 0;
      cl.CullMode = translate_cull_mode(state->cull_face);
#endif
      cl.UserClipDistanceClipTestEnableBitmask = state->clip_plane_enable;
#if GFX_VER < 8
      cl.ViewportZClipTestEnable = (state->depth_clip_near || state->depth_clip_far);
#endif
      cl.APIMode = state->clip_halfz ? APIMODE_D3D : APIMODE_OGL;
      cl.GuardbandClipTestEnable = true;
      cl.ClipEnable = true;
      cl.MinimumPointWidth = 0.125;
      cl.MaximumPointWidth = 255.875;

#if GFX_VER == 8
      cl.ForceUserClipDistanceClipTestEnableBitmask = true;
#endif

      if (state->flatshade_first) {
         cl.TriangleFanProvokingVertexSelect = 1;
      } else {
         cl.TriangleStripListProvokingVertexSelect = 2;
         cl.TriangleFanProvokingVertexSelect = 2;
         cl.LineStripListProvokingVertexSelect = 1;
      }
   }
#endif

   /* Remap from 0..255 back to 1..256 */
   const unsigned line_stipple_factor = state->line_stipple_factor + 1;

   crocus_pack_command(GENX(3DSTATE_LINE_STIPPLE), cso->line_stipple, line) {
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
crocus_bind_rasterizer_state(struct pipe_context *ctx, void *state)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_rasterizer_state *old_cso = ice->state.cso_rast;
   struct crocus_rasterizer_state *new_cso = state;

   if (new_cso) {
      /* Try to avoid re-emitting 3DSTATE_LINE_STIPPLE, it's non-pipelined */
      if (cso_changed_memcmp(line_stipple))
         ice->state.dirty |= CROCUS_DIRTY_LINE_STIPPLE;
#if GFX_VER >= 6
      if (cso_changed(cso.half_pixel_center))
         ice->state.dirty |= CROCUS_DIRTY_GEN6_MULTISAMPLE;
      if (cso_changed(cso.scissor))
         ice->state.dirty |= CROCUS_DIRTY_GEN6_SCISSOR_RECT;
      if (cso_changed(cso.multisample))
	 ice->state.dirty |= CROCUS_DIRTY_WM;
#else
      if (cso_changed(cso.scissor))
         ice->state.dirty |= CROCUS_DIRTY_SF_CL_VIEWPORT;
#endif

      if (cso_changed(cso.line_stipple_enable) || cso_changed(cso.poly_stipple_enable))
         ice->state.dirty |= CROCUS_DIRTY_WM;

#if GFX_VER >= 6
      if (cso_changed(cso.rasterizer_discard))
         ice->state.dirty |= CROCUS_DIRTY_STREAMOUT | CROCUS_DIRTY_CLIP;

      if (cso_changed(cso.flatshade_first))
         ice->state.dirty |= CROCUS_DIRTY_STREAMOUT;
#endif

      if (cso_changed(cso.depth_clip_near) || cso_changed(cso.depth_clip_far) ||
          cso_changed(cso.clip_halfz))
         ice->state.dirty |= CROCUS_DIRTY_CC_VIEWPORT;

#if GFX_VER >= 7
      if (cso_changed(cso.sprite_coord_enable) ||
          cso_changed(cso.sprite_coord_mode) ||
          cso_changed(cso.light_twoside))
         ice->state.dirty |= CROCUS_DIRTY_GEN7_SBE;
#endif
#if GFX_VER <= 5
      if (cso_changed(cso.clip_plane_enable))
         ice->state.dirty |= CROCUS_DIRTY_GEN4_CURBE;
#endif
   }

   ice->state.cso_rast = new_cso;
   ice->state.dirty |= CROCUS_DIRTY_RASTER;
   ice->state.dirty |= CROCUS_DIRTY_CLIP;
#if GFX_VER <= 5
   ice->state.dirty |= CROCUS_DIRTY_GEN4_CLIP_PROG | CROCUS_DIRTY_GEN4_SF_PROG;
   ice->state.dirty |= CROCUS_DIRTY_WM;
#endif
#if GFX_VER <= 6
   ice->state.dirty |= CROCUS_DIRTY_GEN4_FF_GS_PROG;
#endif
   ice->state.stage_dirty |= ice->state.stage_dirty_for_nos[CROCUS_NOS_RASTERIZER];
}

/**
 * Return true if the given wrap mode requires the border color to exist.
 *
 * (We can skip uploading it if the sampler isn't going to use it.)
 */
static bool
wrap_mode_needs_border_color(unsigned wrap_mode)
{
#if GFX_VER == 8
   return wrap_mode == TCM_CLAMP_BORDER || wrap_mode == TCM_HALF_BORDER;
#else
   return wrap_mode == TCM_CLAMP_BORDER;
#endif
}

/**
 * Gallium CSO for sampler state.
 */
struct crocus_sampler_state {
   struct pipe_sampler_state pstate;
   union pipe_color_union border_color;
   bool needs_border_color;
   unsigned wrap_s;
   unsigned wrap_t;
   unsigned wrap_r;
   unsigned mag_img_filter;
   float min_lod;
};

/**
 * The pipe->create_sampler_state() driver hook.
 *
 * We fill out SAMPLER_STATE (except for the border color pointer), and
 * store that on the CPU.  It doesn't make sense to upload it to a GPU
 * buffer object yet, because 3DSTATE_SAMPLER_STATE_POINTERS requires
 * all bound sampler states to be in contiguous memor.
 */
static void *
crocus_create_sampler_state(struct pipe_context *ctx,
                            const struct pipe_sampler_state *state)
{
   struct crocus_sampler_state *cso = CALLOC_STRUCT(crocus_sampler_state);

   if (!cso)
      return NULL;

   STATIC_ASSERT(PIPE_TEX_FILTER_NEAREST == MAPFILTER_NEAREST);
   STATIC_ASSERT(PIPE_TEX_FILTER_LINEAR == MAPFILTER_LINEAR);

   bool either_nearest = state->min_img_filter == PIPE_TEX_FILTER_NEAREST ||
      state->mag_img_filter == PIPE_TEX_FILTER_NEAREST;
   cso->wrap_s = translate_wrap(state->wrap_s, either_nearest);
   cso->wrap_t = translate_wrap(state->wrap_t, either_nearest);
   cso->wrap_r = translate_wrap(state->wrap_r, either_nearest);

   cso->pstate = *state;

   memcpy(&cso->border_color, &state->border_color, sizeof(cso->border_color));

   cso->needs_border_color = wrap_mode_needs_border_color(cso->wrap_s) ||
                             wrap_mode_needs_border_color(cso->wrap_t) ||
                             wrap_mode_needs_border_color(cso->wrap_r);

   cso->min_lod = state->min_lod;
   cso->mag_img_filter = state->mag_img_filter;

   // XXX: explain this code ported from ilo...I don't get it at all...
   if (state->min_mip_filter == PIPE_TEX_MIPFILTER_NONE &&
       state->min_lod > 0.0f) {
      cso->min_lod = 0.0f;
      cso->mag_img_filter = state->min_img_filter;
   }

   return cso;
}

/**
 * The pipe->bind_sampler_states() driver hook.
 */
static void
crocus_bind_sampler_states(struct pipe_context *ctx,
                           enum pipe_shader_type p_stage,
                           unsigned start, unsigned count,
                           void **states)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct crocus_shader_state *shs = &ice->state.shaders[stage];

   assert(start + count <= CROCUS_MAX_TEXTURE_SAMPLERS);

   bool dirty = false;

   for (int i = 0; i < count; i++) {
      if (shs->samplers[start + i] != states[i]) {
         shs->samplers[start + i] = states[i];
         dirty = true;
      }
   }

   if (dirty) {
#if GFX_VER <= 5
      if (p_stage == PIPE_SHADER_FRAGMENT)
         ice->state.dirty |= CROCUS_DIRTY_WM;
      else if (p_stage == PIPE_SHADER_VERTEX)
         ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_VS;
#endif
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_SAMPLER_STATES_VS << stage;
      ice->state.stage_dirty |= ice->state.stage_dirty_for_nos[CROCUS_NOS_TEXTURES];
   }
}

enum samp_workaround {
   SAMP_NORMAL,
   SAMP_CUBE_CLAMP,
   SAMP_CUBE_CUBE,
   SAMP_T_WRAP,
};

static void
crocus_upload_sampler_state(struct crocus_batch *batch,
                            struct crocus_sampler_state *cso,
                            uint32_t border_color_offset,
                            enum samp_workaround samp_workaround,
                            uint32_t first_level,
                            void *map)
{
   struct pipe_sampler_state *state = &cso->pstate;
   uint32_t wrap_s, wrap_t, wrap_r;

   wrap_s = cso->wrap_s;
   wrap_t = cso->wrap_t;
   wrap_r = cso->wrap_r;

   switch (samp_workaround) {
   case SAMP_CUBE_CLAMP:
      wrap_s = TCM_CLAMP;
      wrap_t = TCM_CLAMP;
      wrap_r = TCM_CLAMP;
      break;
   case SAMP_CUBE_CUBE:
      wrap_s = TCM_CUBE;
      wrap_t = TCM_CUBE;
      wrap_r = TCM_CUBE;
      break;
   case SAMP_T_WRAP:
      wrap_t = TCM_WRAP;
      break;
   default:
      break;
   }

   _crocus_pack_state(batch, GENX(SAMPLER_STATE), map, samp) {
      samp.TCXAddressControlMode = wrap_s;
      samp.TCYAddressControlMode = wrap_t;
      samp.TCZAddressControlMode = wrap_r;

#if GFX_VER >= 6
      samp.NonnormalizedCoordinateEnable = state->unnormalized_coords;
#endif
      samp.MinModeFilter = state->min_img_filter;
      samp.MagModeFilter = cso->mag_img_filter;
      samp.MipModeFilter = translate_mip_filter(state->min_mip_filter);
      samp.MaximumAnisotropy = RATIO21;

      if (state->max_anisotropy >= 2) {
         if (state->min_img_filter == PIPE_TEX_FILTER_LINEAR) {
            samp.MinModeFilter = MAPFILTER_ANISOTROPIC;
#if GFX_VER >= 7
            samp.AnisotropicAlgorithm = EWAApproximation;
#endif
         }

         if (state->mag_img_filter == PIPE_TEX_FILTER_LINEAR)
            samp.MagModeFilter = MAPFILTER_ANISOTROPIC;

         samp.MaximumAnisotropy =
            MIN2((state->max_anisotropy - 2) / 2, RATIO161);
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

#if GFX_VER == 8
      samp.LODPreClampMode = CLAMP_MODE_OGL;
#else
      samp.LODPreClampEnable = true;
#endif
      samp.MinLOD = CLAMP(cso->min_lod, 0, hw_max_lod);
      samp.MaxLOD = CLAMP(state->max_lod, 0, hw_max_lod);
      samp.TextureLODBias = CLAMP(state->lod_bias, -16, 15);

#if GFX_VER == 6
      samp.BaseMipLevel = CLAMP(first_level, 0, hw_max_lod);
      samp.MinandMagStateNotEqual = samp.MinModeFilter != samp.MagModeFilter;
#endif

#if GFX_VER < 6
      samp.BorderColorPointer =
         ro_bo(batch->state.bo, border_color_offset);
#else
      samp.BorderColorPointer = border_color_offset;
#endif
   }
}

static void
crocus_upload_border_color(struct crocus_batch *batch,
                           struct crocus_sampler_state *cso,
                           struct crocus_sampler_view *tex,
                           uint32_t *bc_offset)
{
   /* We may need to swizzle the border color for format faking.
    * A/LA formats are faked as R/RG with 000R or R00G swizzles.
    * This means we need to move the border color's A channel into
    * the R or G channels so that those read swizzles will move it
    * back into A.
    */
   enum pipe_format internal_format = PIPE_FORMAT_NONE;
   union pipe_color_union *color = &cso->border_color;
   union pipe_color_union tmp;
   if (tex) {
      internal_format = tex->res->internal_format;

      if (util_format_is_alpha(internal_format)) {
         unsigned char swz[4] = {
            PIPE_SWIZZLE_0, PIPE_SWIZZLE_0,
            PIPE_SWIZZLE_0, PIPE_SWIZZLE_W,
         };
         util_format_apply_color_swizzle(&tmp, color, swz, true);
         color = &tmp;
      } else if (util_format_is_luminance_alpha(internal_format) &&
                 internal_format != PIPE_FORMAT_L8A8_SRGB) {
         unsigned char swz[4] = {
            PIPE_SWIZZLE_X, PIPE_SWIZZLE_X,
            PIPE_SWIZZLE_X, PIPE_SWIZZLE_W
         };
         util_format_apply_color_swizzle(&tmp, color, swz, true);
         color = &tmp;
      }
   }
   bool is_integer_format = util_format_is_pure_integer(internal_format);
   unsigned sbc_size = GENX(SAMPLER_BORDER_COLOR_STATE_length) * 4;
   const int sbc_align = (GFX_VER == 8 ? 64 : ((GFX_VERx10 == 75 && is_integer_format) ? 512 : 32));
   uint32_t *sbc = stream_state(batch, sbc_size, sbc_align, bc_offset);

   struct GENX(SAMPLER_BORDER_COLOR_STATE) state = { 0 };

#define ASSIGN(dst, src)                        \
   do {                                         \
      dst = src;                                \
   } while (0)

#define ASSIGNu16(dst, src)                     \
   do {                                         \
      dst = (uint16_t)src;                      \
   } while (0)

#define ASSIGNu8(dst, src)                      \
   do {                                         \
      dst = (uint8_t)src;                       \
   } while (0)

#define BORDER_COLOR_ATTR(macro, _color_type, src)              \
   macro(state.BorderColor ## _color_type ## Red, src[0]);      \
   macro(state.BorderColor ## _color_type ## Green, src[1]);    \
   macro(state.BorderColor ## _color_type ## Blue, src[2]);     \
   macro(state.BorderColor ## _color_type ## Alpha, src[3]);

#if GFX_VER >= 8
   /* On Broadwell, the border color is represented as four 32-bit floats,
    * integers, or unsigned values, interpreted according to the surface
    * format.  This matches the sampler->BorderColor union exactly; just
    * memcpy the values.
    */
   BORDER_COLOR_ATTR(ASSIGN, 32bit, color->ui);
#elif GFX_VERx10 == 75
   if (is_integer_format) {
      const struct util_format_description *format_desc =
         util_format_description(internal_format);

      /* From the Haswell PRM, "Command Reference: Structures", Page 36:
       * "If any color channel is missing from the surface format,
       *  corresponding border color should be programmed as zero and if
       *  alpha channel is missing, corresponding Alpha border color should
       *  be programmed as 1."
       */
      unsigned c[4] = { 0, 0, 0, 1 };
      for (int i = 0; i < 4; i++) {
         if (format_desc->channel[i].size)
            c[i] = color->ui[i];
      }

      switch (format_desc->channel[0].size) {
      case 8:
         /* Copy RGBA in order. */
         BORDER_COLOR_ATTR(ASSIGNu8, 8bit, c);
         break;
      case 10:
         /* R10G10B10A2_UINT is treated like a 16-bit format. */
      case 16:
         BORDER_COLOR_ATTR(ASSIGNu16, 16bit, c);
         break;
      case 32:
         if (format_desc->channel[1].size && !format_desc->channel[2].size) {
            /* Careful inspection of the tables reveals that for RG32 formats,
             * the green channel needs to go where blue normally belongs.
             */
            state.BorderColor32bitRed = c[0];
            state.BorderColor32bitBlue = c[1];
            state.BorderColor32bitAlpha = 1;
         } else {
            /* Copy RGBA in order. */
            BORDER_COLOR_ATTR(ASSIGN, 32bit, c);
         }
         break;
      default:
         assert(!"Invalid number of bits per channel in integer format.");
         break;
      }
   } else {
      BORDER_COLOR_ATTR(ASSIGN, Float, color->f);
   }
#elif GFX_VER == 5 || GFX_VER == 6
   BORDER_COLOR_ATTR(UNCLAMPED_FLOAT_TO_UBYTE, Unorm, color->f);
   BORDER_COLOR_ATTR(UNCLAMPED_FLOAT_TO_USHORT, Unorm16, color->f);
   BORDER_COLOR_ATTR(UNCLAMPED_FLOAT_TO_SHORT, Snorm16, color->f);

#define MESA_FLOAT_TO_HALF(dst, src)            \
   dst = _mesa_float_to_half(src);

   BORDER_COLOR_ATTR(MESA_FLOAT_TO_HALF, Float16, color->f);

#undef MESA_FLOAT_TO_HALF

   state.BorderColorSnorm8Red   = state.BorderColorSnorm16Red >> 8;
   state.BorderColorSnorm8Green = state.BorderColorSnorm16Green >> 8;
   state.BorderColorSnorm8Blue  = state.BorderColorSnorm16Blue >> 8;
   state.BorderColorSnorm8Alpha = state.BorderColorSnorm16Alpha >> 8;

   BORDER_COLOR_ATTR(ASSIGN, Float, color->f);

#elif GFX_VER == 4
   BORDER_COLOR_ATTR(ASSIGN, , color->f);
#else
   BORDER_COLOR_ATTR(ASSIGN, Float, color->f);
#endif

#undef ASSIGN
#undef BORDER_COLOR_ATTR

   GENX(SAMPLER_BORDER_COLOR_STATE_pack)(batch, sbc, &state);
}

/**
 * Upload the sampler states into a contiguous area of GPU memory, for
 * for 3DSTATE_SAMPLER_STATE_POINTERS_*.
 *
 * Also fill out the border color state pointers.
 */
static void
crocus_upload_sampler_states(struct crocus_context *ice,
                             struct crocus_batch *batch, gl_shader_stage stage)
{
   struct crocus_shader_state *shs = &ice->state.shaders[stage];
   const struct shader_info *info = crocus_get_shader_info(ice, stage);

   /* We assume the state tracker will call pipe->bind_sampler_states()
    * if the program's number of textures changes.
    */
   unsigned count = info ? BITSET_LAST_BIT(info->textures_used) : 0;

   if (!count)
      return;

   /* Assemble the SAMPLER_STATEs into a contiguous table that lives
    * in the dynamic state memory zone, so we can point to it via the
    * 3DSTATE_SAMPLER_STATE_POINTERS_* commands.
    */
   unsigned size = count * 4 * GENX(SAMPLER_STATE_length);
   uint32_t *map = stream_state(batch, size, 32, &shs->sampler_offset);

   if (unlikely(!map))
      return;

   for (int i = 0; i < count; i++) {
      struct crocus_sampler_state *state = shs->samplers[i];
      struct crocus_sampler_view *tex = shs->textures[i];

      if (!state || !tex) {
         memset(map, 0, 4 * GENX(SAMPLER_STATE_length));
      } else {
         unsigned border_color_offset = 0;
         if (state->needs_border_color) {
            crocus_upload_border_color(batch, state, tex, &border_color_offset);
         }

         enum samp_workaround wa = SAMP_NORMAL;
         /* There's a bug in 1D texture sampling - it actually pays
          * attention to the wrap_t value, though it should not.
          * Override the wrap_t value here to GL_REPEAT to keep
          * any nonexistent border pixels from floating in.
          */
         if (tex->base.target == PIPE_TEXTURE_1D)
            wa = SAMP_T_WRAP;
         else if (tex->base.target == PIPE_TEXTURE_CUBE ||
                  tex->base.target == PIPE_TEXTURE_CUBE_ARRAY) {
            /* Cube maps must use the same wrap mode for all three coordinate
             * dimensions.  Prior to Haswell, only CUBE and CLAMP are valid.
             *
             * Ivybridge and Baytrail seem to have problems with CUBE mode and
             * integer formats.  Fall back to CLAMP for now.
             */
            if (state->pstate.seamless_cube_map &&
                !(GFX_VERx10 == 70 && util_format_is_pure_integer(tex->base.format)))
               wa = SAMP_CUBE_CUBE;
            else
               wa = SAMP_CUBE_CLAMP;
         }

         uint32_t first_level = 0;
         if (tex->base.target != PIPE_BUFFER)
            first_level = tex->base.u.tex.first_level;

         crocus_upload_sampler_state(batch, state, border_color_offset, wa, first_level, map);
      }

      map += GENX(SAMPLER_STATE_length);
   }
}

/**
 * The pipe->create_sampler_view() driver hook.
 */
static struct pipe_sampler_view *
crocus_create_sampler_view(struct pipe_context *ctx,
                           struct pipe_resource *tex,
                           const struct pipe_sampler_view *tmpl)
{
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_sampler_view *isv = calloc(1, sizeof(struct crocus_sampler_view));

   if (!isv)
      return NULL;

   /* initialize base object */
   isv->base = *tmpl;
   isv->base.context = ctx;
   isv->base.texture = NULL;
   pipe_reference_init(&isv->base.reference, 1);
   pipe_resource_reference(&isv->base.texture, tex);

   if (util_format_is_depth_or_stencil(tmpl->format)) {
      struct crocus_resource *zres, *sres;
      const struct util_format_description *desc =
         util_format_description(tmpl->format);

      crocus_get_depth_stencil_resources(devinfo, tex, &zres, &sres);

      tex = util_format_has_depth(desc) ? &zres->base.b : &sres->base.b;

      if (tex->format == PIPE_FORMAT_S8_UINT)
         if (GFX_VER == 7 && sres->shadow)
            tex = &sres->shadow->base.b;
   }

   isv->res = (struct crocus_resource *) tex;

   isl_surf_usage_flags_t usage = ISL_SURF_USAGE_TEXTURE_BIT;

   if (isv->base.target == PIPE_TEXTURE_CUBE ||
       isv->base.target == PIPE_TEXTURE_CUBE_ARRAY)
      usage |= ISL_SURF_USAGE_CUBE_BIT;

   const struct crocus_format_info fmt =
      crocus_format_for_usage(devinfo, tmpl->format, usage);

   enum pipe_swizzle vswz[4] = { tmpl->swizzle_r, tmpl->swizzle_g, tmpl->swizzle_b, tmpl->swizzle_a };
   crocus_combine_swizzle(isv->swizzle, fmt.swizzles, vswz);

   /* hardcode stencil swizzles - hw returns 0G01, we want GGGG */
   if (GFX_VER < 6 &&
       (tmpl->format == PIPE_FORMAT_X32_S8X24_UINT ||
        tmpl->format == PIPE_FORMAT_X24S8_UINT)) {
      isv->swizzle[0] = tmpl->swizzle_g;
      isv->swizzle[1] = tmpl->swizzle_g;
      isv->swizzle[2] = tmpl->swizzle_g;
      isv->swizzle[3] = tmpl->swizzle_g;
   }

   isv->clear_color = isv->res->aux.clear_color;

   isv->view = (struct isl_view) {
      .format = fmt.fmt,
#if GFX_VERx10 >= 75
      .swizzle = (struct isl_swizzle) {
         .r = pipe_to_isl_swizzle(isv->swizzle[0], false),
         .g = pipe_to_isl_swizzle(isv->swizzle[1], false),
         .b = pipe_to_isl_swizzle(isv->swizzle[2], false),
         .a = pipe_to_isl_swizzle(isv->swizzle[3], false),
      },
#else
      /* swizzling handled in shader code */
      .swizzle = ISL_SWIZZLE_IDENTITY,
#endif
      .usage = usage,
   };

   /* Fill out SURFACE_STATE for this view. */
   if (tmpl->target != PIPE_BUFFER) {
      isv->view.base_level = tmpl->u.tex.first_level;
      isv->view.levels = tmpl->u.tex.last_level - tmpl->u.tex.first_level + 1;

      /* Hardware older than skylake ignores this value */
      assert(tex->target != PIPE_TEXTURE_3D || !tmpl->u.tex.first_layer);

      // XXX: do I need to port f9fd0cf4790cb2a530e75d1a2206dbb9d8af7cb2?
      isv->view.base_array_layer = tmpl->u.tex.first_layer;
      isv->view.array_len =
         tmpl->u.tex.last_layer - tmpl->u.tex.first_layer + 1;
   }
#if GFX_VER >= 6
   /* just create a second view struct for texture gather just in case */
   isv->gather_view = isv->view;

#if GFX_VER == 7
   if (fmt.fmt == ISL_FORMAT_R32G32_FLOAT ||
       fmt.fmt == ISL_FORMAT_R32G32_SINT ||
       fmt.fmt == ISL_FORMAT_R32G32_UINT) {
      isv->gather_view.format = ISL_FORMAT_R32G32_FLOAT_LD;
#if GFX_VERx10 >= 75
      isv->gather_view.swizzle = (struct isl_swizzle) {
         .r = pipe_to_isl_swizzle(isv->swizzle[0], GFX_VERx10 == 75),
         .g = pipe_to_isl_swizzle(isv->swizzle[1], GFX_VERx10 == 75),
         .b = pipe_to_isl_swizzle(isv->swizzle[2], GFX_VERx10 == 75),
         .a = pipe_to_isl_swizzle(isv->swizzle[3], GFX_VERx10 == 75),
      };
#endif
   }
#endif
#if GFX_VER == 6
   /* Sandybridge's gather4 message is broken for integer formats.
    * To work around this, we pretend the surface is UNORM for
    * 8 or 16-bit formats, and emit shader instructions to recover
    * the real INT/UINT value.  For 32-bit formats, we pretend
    * the surface is FLOAT, and simply reinterpret the resulting
    * bits.
    */
   switch (fmt.fmt) {
   case ISL_FORMAT_R8_SINT:
   case ISL_FORMAT_R8_UINT:
      isv->gather_view.format = ISL_FORMAT_R8_UNORM;
      break;

   case ISL_FORMAT_R16_SINT:
   case ISL_FORMAT_R16_UINT:
      isv->gather_view.format = ISL_FORMAT_R16_UNORM;
      break;

   case ISL_FORMAT_R32_SINT:
   case ISL_FORMAT_R32_UINT:
      isv->gather_view.format = ISL_FORMAT_R32_FLOAT;
      break;

   default:
      break;
   }
#endif
#endif

   return &isv->base;
}

static void
crocus_sampler_view_destroy(struct pipe_context *ctx,
                            struct pipe_sampler_view *state)
{
   struct crocus_sampler_view *isv = (void *) state;
   pipe_resource_reference(&state->texture, NULL);
   free(isv);
}

/**
 * The pipe->create_surface() driver hook.
 *
 * In Gallium nomenclature, "surfaces" are a view of a resource that
 * can be bound as a render target or depth/stencil buffer.
 */
static struct pipe_surface *
crocus_create_surface(struct pipe_context *ctx,
                      struct pipe_resource *tex,
                      const struct pipe_surface *tmpl)
{
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   isl_surf_usage_flags_t usage = 0;
   if (tmpl->writable)
      usage = ISL_SURF_USAGE_STORAGE_BIT;
   else if (util_format_is_depth_or_stencil(tmpl->format))
      usage = ISL_SURF_USAGE_DEPTH_BIT;
   else
      usage = ISL_SURF_USAGE_RENDER_TARGET_BIT;

   const struct crocus_format_info fmt =
      crocus_format_for_usage(devinfo, tmpl->format, usage);

   if ((usage & ISL_SURF_USAGE_RENDER_TARGET_BIT) &&
       !isl_format_supports_rendering(devinfo, fmt.fmt)) {
      /* Framebuffer validation will reject this invalid case, but it
       * hasn't had the opportunity yet.  In the meantime, we need to
       * avoid hitting ISL asserts about unsupported formats below.
       */
      return NULL;
   }

   struct crocus_surface *surf = calloc(1, sizeof(struct crocus_surface));
   struct pipe_surface *psurf = &surf->base;
   struct crocus_resource *res = (struct crocus_resource *) tex;

   if (!surf)
      return NULL;

   pipe_reference_init(&psurf->reference, 1);
   pipe_resource_reference(&psurf->texture, tex);
   psurf->context = ctx;
   psurf->format = tmpl->format;
   psurf->width = tex->width0;
   psurf->height = tex->height0;
   psurf->u.tex.first_layer = tmpl->u.tex.first_layer;
   psurf->u.tex.last_layer = tmpl->u.tex.last_layer;
   psurf->u.tex.level = tmpl->u.tex.level;

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

#if GFX_VER >= 6
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
#endif

   surf->clear_color = res->aux.clear_color;

   /* Bail early for depth/stencil - we don't want SURFACE_STATE for them. */
   if (res->surf.usage & (ISL_SURF_USAGE_DEPTH_BIT |
                          ISL_SURF_USAGE_STENCIL_BIT))
      return psurf;

   if (!isl_format_is_compressed(res->surf.format)) {
      memcpy(&surf->surf, &res->surf, sizeof(surf->surf));
      uint64_t temp_offset;
      uint32_t temp_x, temp_y;

      isl_surf_get_image_offset_B_tile_sa(&res->surf, tmpl->u.tex.level,
                                          res->base.b.target == PIPE_TEXTURE_3D ? 0 : tmpl->u.tex.first_layer,
                                          res->base.b.target == PIPE_TEXTURE_3D ? tmpl->u.tex.first_layer : 0,
                                          &temp_offset, &temp_x, &temp_y);
      if (!devinfo->has_surface_tile_offset &&
          (temp_x || temp_y)) {
         /* Original gfx4 hardware couldn't draw to a non-tile-aligned
          * destination.
          */
         /* move to temp */
         struct pipe_resource wa_templ = (struct pipe_resource) {
            .width0 = u_minify(res->base.b.width0, tmpl->u.tex.level),
            .height0 = u_minify(res->base.b.height0, tmpl->u.tex.level),
            .depth0 = 1,
            .array_size = 1,
            .format = res->base.b.format,
            .target = PIPE_TEXTURE_2D,
            .bind = (usage & ISL_SURF_USAGE_DEPTH_BIT ? PIPE_BIND_DEPTH_STENCIL : PIPE_BIND_RENDER_TARGET) | PIPE_BIND_SAMPLER_VIEW,
         };
         surf->align_res = screen->base.resource_create(&screen->base, &wa_templ);
         view->base_level = 0;
         view->base_array_layer = 0;
         view->array_len = 1;
         struct crocus_resource *align_res = (struct crocus_resource *)surf->align_res;
         memcpy(&surf->surf, &align_res->surf, sizeof(surf->surf));
      }
      return psurf;
   }

   /* The resource has a compressed format, which is not renderable, but we
    * have a renderable view format.  We must be attempting to upload blocks
    * of compressed data via an uncompressed view.
    *
    * In this case, we can assume there are no auxiliary buffers, a single
    * miplevel, and that the resource is single-sampled.  Gallium may try
    * and create an uncompressed view with multiple layers, however.
    */
   assert(!isl_format_is_compressed(fmt.fmt));
   assert(res->surf.samples == 1);
   assert(view->levels == 1);

   /* TODO: compressed pbo uploads aren't working here */
   pipe_surface_reference(&psurf, NULL);
   return NULL;

   uint64_t offset_B = 0;
   uint32_t tile_x_sa = 0, tile_y_sa = 0;

   if (view->base_level > 0) {
      /* We can't rely on the hardware's miplevel selection with such
       * a substantial lie about the format, so we select a single image
       * using the Tile X/Y Offset fields.  In this case, we can't handle
       * multiple array slices.
       *
       * On Broadwell, HALIGN and VALIGN are specified in pixels and are
       * hard-coded to align to exactly the block size of the compressed
       * texture.  This means that, when reinterpreted as a non-compressed
       * texture, the tile offsets may be anything and we can't rely on
       * X/Y Offset.
       *
       * Return NULL to force the state tracker to take fallback paths.
       */
      // TODO: check if the gen7 check is right, originally gen8
      if (view->array_len > 1 || GFX_VER == 7) {
         pipe_surface_reference(&psurf, NULL);
         return NULL;
      }

      const bool is_3d = res->surf.dim == ISL_SURF_DIM_3D;
      isl_surf_get_image_surf(&screen->isl_dev, &res->surf,
                              view->base_level,
                              is_3d ? 0 : view->base_array_layer,
                              is_3d ? view->base_array_layer : 0,
                              &surf->surf,
                              &offset_B, &tile_x_sa, &tile_y_sa);

      /* We use address and tile offsets to access a single level/layer
       * as a subimage, so reset level/layer so it doesn't offset again.
       */
      view->base_array_layer = 0;
      view->base_level = 0;
   } else {
      /* Level 0 doesn't require tile offsets, and the hardware can find
       * array slices using QPitch even with the format override, so we
       * can allow layers in this case.  Copy the original ISL surface.
       */
      memcpy(&surf->surf, &res->surf, sizeof(surf->surf));
   }

   /* Scale down the image dimensions by the block size. */
   const struct isl_format_layout *fmtl =
      isl_format_get_layout(res->surf.format);
   surf->surf.format = fmt.fmt;
   surf->surf.logical_level0_px = isl_surf_get_logical_level0_el(&surf->surf);
   surf->surf.phys_level0_sa = isl_surf_get_phys_level0_el(&surf->surf);
   tile_x_sa /= fmtl->bw;
   tile_y_sa /= fmtl->bh;

   psurf->width = surf->surf.logical_level0_px.width;
   psurf->height = surf->surf.logical_level0_px.height;

   return psurf;
}

#if GFX_VER >= 7
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

#endif

/**
 * The pipe->set_shader_images() driver hook.
 */
static void
crocus_set_shader_images(struct pipe_context *ctx,
                         enum pipe_shader_type p_stage,
                         unsigned start_slot, unsigned count,
                         unsigned unbind_num_trailing_slots,
                         const struct pipe_image_view *p_images)
{
#if GFX_VER >= 7
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct crocus_shader_state *shs = &ice->state.shaders[stage];
   struct crocus_genx_state *genx = ice->state.genx;
   struct brw_image_param *image_params = genx->shaders[stage].image_param;

   shs->bound_image_views &= ~u_bit_consecutive(start_slot, count);

   for (unsigned i = 0; i < count; i++) {
      struct crocus_image_view *iv = &shs->image[start_slot + i];

      if (p_images && p_images[i].resource) {
         const struct pipe_image_view *img = &p_images[i];
         struct crocus_resource *res = (void *) img->resource;

         util_copy_image_view(&iv->base, img);

         shs->bound_image_views |= 1 << (start_slot + i);

         res->bind_history |= PIPE_BIND_SHADER_IMAGE;
         res->bind_stages |= 1 << stage;

         isl_surf_usage_flags_t usage = ISL_SURF_USAGE_STORAGE_BIT;
         struct crocus_format_info fmt =
            crocus_format_for_usage(devinfo, img->format, usage);

         struct isl_swizzle swiz = pipe_to_isl_swizzles(fmt.swizzles);
         if (img->shader_access & PIPE_IMAGE_ACCESS_READ) {
            /* On Gen8, try to use typed surfaces reads (which support a
             * limited number of formats), and if not possible, fall back
             * to untyped reads.
             */
            if (!isl_has_matching_typed_storage_image_format(devinfo, fmt.fmt))
               fmt.fmt = ISL_FORMAT_RAW;
            else
               fmt.fmt = isl_lower_storage_image_format(devinfo, fmt.fmt);
         }

         if (res->base.b.target != PIPE_BUFFER) {
            struct isl_view view = {
               .format = fmt.fmt,
               .base_level = img->u.tex.level,
               .levels = 1,
               .base_array_layer = img->u.tex.first_layer,
               .array_len = img->u.tex.last_layer - img->u.tex.first_layer + 1,
               .swizzle = swiz,
               .usage = usage,
            };

            iv->view = view;

            isl_surf_fill_image_param(&screen->isl_dev,
                                      &image_params[start_slot + i],
                                      &res->surf, &view);
         } else {
            struct isl_view view = {
               .format = fmt.fmt,
               .swizzle = swiz,
               .usage = usage,
            };
            iv->view = view;

            util_range_add(&res->base.b, &res->valid_buffer_range, img->u.buf.offset,
                           img->u.buf.offset + img->u.buf.size);
            fill_buffer_image_param(&image_params[start_slot + i],
                                    img->format, img->u.buf.size);
         }
      } else {
         pipe_resource_reference(&iv->base.resource, NULL);
         fill_default_image_param(&image_params[start_slot + i]);
      }
   }

   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_BINDINGS_VS << stage;
   ice->state.dirty |=
      stage == MESA_SHADER_COMPUTE ? CROCUS_DIRTY_COMPUTE_RESOLVES_AND_FLUSHES
                                   : CROCUS_DIRTY_RENDER_RESOLVES_AND_FLUSHES;

   /* Broadwell also needs brw_image_params re-uploaded */
   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_VS << stage;
   shs->sysvals_need_upload = true;
#endif
}


/**
 * The pipe->set_sampler_views() driver hook.
 */
static void
crocus_set_sampler_views(struct pipe_context *ctx,
                         enum pipe_shader_type p_stage,
                         unsigned start, unsigned count,
                         unsigned unbind_num_trailing_slots,
                         bool take_ownership,
                         struct pipe_sampler_view **views)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct crocus_shader_state *shs = &ice->state.shaders[stage];

   shs->bound_sampler_views &= ~u_bit_consecutive(start, count);

   for (unsigned i = 0; i < count; i++) {
      struct pipe_sampler_view *pview = views ? views[i] : NULL;

      if (take_ownership) {
         pipe_sampler_view_reference((struct pipe_sampler_view **)
                                     &shs->textures[start + i], NULL);
         shs->textures[start + i] = (struct crocus_sampler_view *)pview;
      } else {
         pipe_sampler_view_reference((struct pipe_sampler_view **)
                                     &shs->textures[start + i], pview);
      }

      struct crocus_sampler_view *view = (void *) pview;
      if (view) {
         view->res->bind_history |= PIPE_BIND_SAMPLER_VIEW;
         view->res->bind_stages |= 1 << stage;

         shs->bound_sampler_views |= 1 << (start + i);
      }
   }
#if GFX_VER == 6
   /* first level parameters to crocus_upload_sampler_state is gfx6 only */
   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_SAMPLER_STATES_VS << stage;
#endif
   ice->state.stage_dirty |= (CROCUS_STAGE_DIRTY_BINDINGS_VS << stage);
   ice->state.dirty |=
      stage == MESA_SHADER_COMPUTE ? CROCUS_DIRTY_COMPUTE_RESOLVES_AND_FLUSHES
                                   : CROCUS_DIRTY_RENDER_RESOLVES_AND_FLUSHES;
   ice->state.stage_dirty |= ice->state.stage_dirty_for_nos[CROCUS_NOS_TEXTURES];
}

/**
 * The pipe->set_tess_state() driver hook.
 */
static void
crocus_set_tess_state(struct pipe_context *ctx,
                      const float default_outer_level[4],
                      const float default_inner_level[2])
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_shader_state *shs = &ice->state.shaders[MESA_SHADER_TESS_CTRL];

   memcpy(&ice->state.default_outer_level[0], &default_outer_level[0], 4 * sizeof(float));
   memcpy(&ice->state.default_inner_level[0], &default_inner_level[0], 2 * sizeof(float));

   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_TCS;
   shs->sysvals_need_upload = true;
}

static void
crocus_set_patch_vertices(struct pipe_context *ctx, uint8_t patch_vertices)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;

   ice->state.patch_vertices = patch_vertices;
}

static void
crocus_surface_destroy(struct pipe_context *ctx, struct pipe_surface *p_surf)
{
   struct crocus_surface *surf = (void *) p_surf;
   pipe_resource_reference(&p_surf->texture, NULL);

   pipe_resource_reference(&surf->align_res, NULL);
   free(surf);
}

static void
crocus_set_clip_state(struct pipe_context *ctx,
                      const struct pipe_clip_state *state)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_shader_state *shs = &ice->state.shaders[MESA_SHADER_VERTEX];
   struct crocus_shader_state *gshs = &ice->state.shaders[MESA_SHADER_GEOMETRY];
   struct crocus_shader_state *tshs = &ice->state.shaders[MESA_SHADER_TESS_EVAL];

   memcpy(&ice->state.clip_planes, state, sizeof(*state));

#if GFX_VER <= 5
   ice->state.dirty |= CROCUS_DIRTY_GEN4_CURBE;
#endif
   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_VS | CROCUS_STAGE_DIRTY_CONSTANTS_GS |
                             CROCUS_STAGE_DIRTY_CONSTANTS_TES;
   shs->sysvals_need_upload = true;
   gshs->sysvals_need_upload = true;
   tshs->sysvals_need_upload = true;
}

/**
 * The pipe->set_polygon_stipple() driver hook.
 */
static void
crocus_set_polygon_stipple(struct pipe_context *ctx,
                           const struct pipe_poly_stipple *state)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   memcpy(&ice->state.poly_stipple, state, sizeof(*state));
   ice->state.dirty |= CROCUS_DIRTY_POLYGON_STIPPLE;
}

/**
 * The pipe->set_sample_mask() driver hook.
 */
static void
crocus_set_sample_mask(struct pipe_context *ctx, unsigned sample_mask)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;

   /* We only support 16x MSAA, so we have 16 bits of sample maks.
    * st/mesa may pass us 0xffffffff though, meaning "enable all samples".
    */
   ice->state.sample_mask = sample_mask & 0xff;
   ice->state.dirty |= CROCUS_DIRTY_GEN6_SAMPLE_MASK;
}

static void
crocus_fill_scissor_rect(struct crocus_context *ice,
                         int idx,
                         struct pipe_scissor_state *ss)
{
   struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
   struct pipe_rasterizer_state *cso_state = &ice->state.cso_rast->cso;
   const struct pipe_viewport_state *vp = &ice->state.viewports[idx];
   struct pipe_scissor_state scissor = (struct pipe_scissor_state) {
      .minx = MAX2(-fabsf(vp->scale[0]) + vp->translate[0], 0),
      .maxx = MIN2( fabsf(vp->scale[0]) + vp->translate[0], cso_fb->width) - 1,
      .miny = MAX2(-fabsf(vp->scale[1]) + vp->translate[1], 0),
      .maxy = MIN2( fabsf(vp->scale[1]) + vp->translate[1], cso_fb->height) - 1,
   };
   if (cso_state->scissor) {
      struct pipe_scissor_state *s = &ice->state.scissors[idx];
      scissor.minx = MAX2(scissor.minx, s->minx);
      scissor.miny = MAX2(scissor.miny, s->miny);
      scissor.maxx = MIN2(scissor.maxx, s->maxx);
      scissor.maxy = MIN2(scissor.maxy, s->maxy);
   }
   *ss = scissor;
}

/**
 * The pipe->set_scissor_states() driver hook.
 *
 * This corresponds to our SCISSOR_RECT state structures.  It's an
 * exact match, so we just store them, and memcpy them out later.
 */
static void
crocus_set_scissor_states(struct pipe_context *ctx,
                          unsigned start_slot,
                          unsigned num_scissors,
                          const struct pipe_scissor_state *rects)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;

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

#if GFX_VER < 6
   ice->state.dirty |= CROCUS_DIRTY_RASTER; /* SF state */
#else
   ice->state.dirty |= CROCUS_DIRTY_GEN6_SCISSOR_RECT;
#endif
   ice->state.dirty |= CROCUS_DIRTY_SF_CL_VIEWPORT;

}

/**
 * The pipe->set_stencil_ref() driver hook.
 *
 * This is added to 3DSTATE_WM_DEPTH_STENCIL dynamically at draw time.
 */
static void
crocus_set_stencil_ref(struct pipe_context *ctx,
                       const struct pipe_stencil_ref ref)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   ice->state.stencil_ref = ref;
   ice->state.dirty |= CROCUS_DIRTY_COLOR_CALC_STATE;
}

#if GFX_VER == 8
static float
viewport_extent(const struct pipe_viewport_state *state, int axis, float sign)
{
   return copysignf(state->scale[axis], sign) + state->translate[axis];
}
#endif

/**
 * The pipe->set_viewport_states() driver hook.
 *
 * This corresponds to our SF_CLIP_VIEWPORT states.  We can't calculate
 * the guardband yet, as we need the framebuffer dimensions, but we can
 * at least fill out the rest.
 */
static void
crocus_set_viewport_states(struct pipe_context *ctx,
                           unsigned start_slot,
                           unsigned count,
                           const struct pipe_viewport_state *states)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;

   memcpy(&ice->state.viewports[start_slot], states, sizeof(*states) * count);

   /* Fix depth test misrenderings by lowering translated depth range */
   if (screen->driconf.lower_depth_range_rate != 1.0f)
      ice->state.viewports[start_slot].translate[2] *=
         screen->driconf.lower_depth_range_rate;

   ice->state.dirty |= CROCUS_DIRTY_SF_CL_VIEWPORT;
   ice->state.dirty |= CROCUS_DIRTY_RASTER;
#if GFX_VER >= 6
   ice->state.dirty |= CROCUS_DIRTY_GEN6_SCISSOR_RECT;
#endif

   if (ice->state.cso_rast && (!ice->state.cso_rast->cso.depth_clip_near ||
                               !ice->state.cso_rast->cso.depth_clip_far))
      ice->state.dirty |= CROCUS_DIRTY_CC_VIEWPORT;
}

/**
 * The pipe->set_framebuffer_state() driver hook.
 *
 * Sets the current draw FBO, including color render targets, depth,
 * and stencil buffers.
 */
static void
crocus_set_framebuffer_state(struct pipe_context *ctx,
                             const struct pipe_framebuffer_state *state)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct pipe_framebuffer_state *cso = &ice->state.framebuffer;
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
#if 0
   struct isl_device *isl_dev = &screen->isl_dev;
   struct crocus_resource *zres;
   struct crocus_resource *stencil_res;
#endif

   unsigned samples = util_framebuffer_get_num_samples(state);
   unsigned layers = util_framebuffer_get_num_layers(state);

#if GFX_VER >= 6
   if (cso->samples != samples) {
      ice->state.dirty |= CROCUS_DIRTY_GEN6_MULTISAMPLE;
      ice->state.dirty |= CROCUS_DIRTY_GEN6_SAMPLE_MASK;
      ice->state.dirty |= CROCUS_DIRTY_RASTER;
#if GFX_VERx10 == 75
      ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_FS;
#endif
   }
#endif

#if GFX_VER >= 6 && GFX_VER < 8
   ice->state.dirty |= CROCUS_DIRTY_GEN6_BLEND_STATE;
#endif

   if ((cso->layers == 0) != (layers == 0)) {
      ice->state.dirty |= CROCUS_DIRTY_CLIP;
   }

   if (cso->width != state->width || cso->height != state->height) {
      ice->state.dirty |= CROCUS_DIRTY_SF_CL_VIEWPORT;
      ice->state.dirty |= CROCUS_DIRTY_RASTER;
      ice->state.dirty |= CROCUS_DIRTY_DRAWING_RECTANGLE;
#if GFX_VER >= 6
      ice->state.dirty |= CROCUS_DIRTY_GEN6_SCISSOR_RECT;
#endif
   }

   if (cso->zsbuf || state->zsbuf) {
      ice->state.dirty |= CROCUS_DIRTY_DEPTH_BUFFER;

      /* update SF's depth buffer format */
      if (GFX_VER == 7 && cso->zsbuf)
         ice->state.dirty |= CROCUS_DIRTY_RASTER;
   }

   /* wm thread dispatch enable */
   ice->state.dirty |= CROCUS_DIRTY_WM;
   util_copy_framebuffer_state(cso, state);
   cso->samples = samples;
   cso->layers = layers;

   if (cso->zsbuf) {
      struct crocus_resource *zres;
      struct crocus_resource *stencil_res;
      enum isl_aux_usage aux_usage = ISL_AUX_USAGE_NONE;
      crocus_get_depth_stencil_resources(devinfo, cso->zsbuf->texture, &zres,
                                         &stencil_res);
      if (zres && crocus_resource_level_has_hiz(zres, cso->zsbuf->u.tex.level)) {
         aux_usage = zres->aux.usage;
      }
      ice->state.hiz_usage = aux_usage;
   }

   /* Render target change */
   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_BINDINGS_FS;

   ice->state.dirty |= CROCUS_DIRTY_RENDER_RESOLVES_AND_FLUSHES;

   ice->state.stage_dirty |= ice->state.stage_dirty_for_nos[CROCUS_NOS_FRAMEBUFFER];
}

/**
 * The pipe->set_constant_buffer() driver hook.
 *
 * This uploads any constant data in user buffers, and references
 * any UBO resources containing constant data.
 */
static void
crocus_set_constant_buffer(struct pipe_context *ctx,
                           enum pipe_shader_type p_stage, unsigned index,
                           bool take_ownership,
                           const struct pipe_constant_buffer *input)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct crocus_shader_state *shs = &ice->state.shaders[stage];
   struct pipe_constant_buffer *cbuf = &shs->constbufs[index];

   util_copy_constant_buffer(&shs->constbufs[index], input, take_ownership);

   if (input && input->buffer_size && (input->buffer || input->user_buffer)) {
      shs->bound_cbufs |= 1u << index;

      if (input->user_buffer) {
         void *map = NULL;
         pipe_resource_reference(&cbuf->buffer, NULL);
         u_upload_alloc(ice->ctx.const_uploader, 0, input->buffer_size, 64,
                        &cbuf->buffer_offset, &cbuf->buffer, (void **) &map);

         if (!cbuf->buffer) {
            /* Allocation was unsuccessful - just unbind */
            crocus_set_constant_buffer(ctx, p_stage, index, false, NULL);
            return;
         }

         assert(map);
         memcpy(map, input->user_buffer, input->buffer_size);
      }
      cbuf->buffer_size =
         MIN2(input->buffer_size,
              crocus_resource_bo(cbuf->buffer)->size - cbuf->buffer_offset);

      struct crocus_resource *res = (void *) cbuf->buffer;
      res->bind_history |= PIPE_BIND_CONSTANT_BUFFER;
      res->bind_stages |= 1 << stage;
   } else {
      shs->bound_cbufs &= ~(1u << index);
   }

   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_VS << stage;
}

static void
upload_sysvals(struct crocus_context *ice,
               gl_shader_stage stage)
{
   UNUSED struct crocus_genx_state *genx = ice->state.genx;
   struct crocus_shader_state *shs = &ice->state.shaders[stage];

   struct crocus_compiled_shader *shader = ice->shaders.prog[stage];
   if (!shader || shader->num_system_values == 0)
      return;

   assert(shader->num_cbufs > 0);

   unsigned sysval_cbuf_index = shader->num_cbufs - 1;
   struct pipe_constant_buffer *cbuf = &shs->constbufs[sysval_cbuf_index];
   unsigned upload_size = shader->num_system_values * sizeof(uint32_t);
   uint32_t *map = NULL;

   assert(sysval_cbuf_index < PIPE_MAX_CONSTANT_BUFFERS);
   u_upload_alloc(ice->ctx.const_uploader, 0, upload_size, 64,
                  &cbuf->buffer_offset, &cbuf->buffer, (void **) &map);

   for (int i = 0; i < shader->num_system_values; i++) {
      uint32_t sysval = shader->system_values[i];
      uint32_t value = 0;

      if (BRW_PARAM_DOMAIN(sysval) == BRW_PARAM_DOMAIN_IMAGE) {
#if GFX_VER >= 7
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
               crocus_get_shader_info(ice, MESA_SHADER_TESS_CTRL);
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
      } else {
         assert(!"unhandled system value");
      }

      *map++ = value;
   }

   cbuf->buffer_size = upload_size;
   shs->sysvals_need_upload = false;
}

/**
 * The pipe->set_shader_buffers() driver hook.
 *
 * This binds SSBOs and ABOs.  Unfortunately, we need to stream out
 * SURFACE_STATE here, as the buffer offset may change each time.
 */
static void
crocus_set_shader_buffers(struct pipe_context *ctx,
                          enum pipe_shader_type p_stage,
                          unsigned start_slot, unsigned count,
                          const struct pipe_shader_buffer *buffers,
                          unsigned writable_bitmask)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   gl_shader_stage stage = stage_from_pipe(p_stage);
   struct crocus_shader_state *shs = &ice->state.shaders[stage];

   unsigned modified_bits = u_bit_consecutive(start_slot, count);

   shs->bound_ssbos &= ~modified_bits;
   shs->writable_ssbos &= ~modified_bits;
   shs->writable_ssbos |= writable_bitmask << start_slot;

   for (unsigned i = 0; i < count; i++) {
      if (buffers && buffers[i].buffer) {
         struct crocus_resource *res = (void *) buffers[i].buffer;
         struct pipe_shader_buffer *ssbo = &shs->ssbo[start_slot + i];
         pipe_resource_reference(&ssbo->buffer, &res->base.b);
         ssbo->buffer_offset = buffers[i].buffer_offset;
         ssbo->buffer_size =
            MIN2(buffers[i].buffer_size, res->bo->size - ssbo->buffer_offset);

         shs->bound_ssbos |= 1 << (start_slot + i);

         res->bind_history |= PIPE_BIND_SHADER_BUFFER;
         res->bind_stages |= 1 << stage;

         util_range_add(&res->base.b, &res->valid_buffer_range, ssbo->buffer_offset,
                        ssbo->buffer_offset + ssbo->buffer_size);
      } else {
         pipe_resource_reference(&shs->ssbo[start_slot + i].buffer, NULL);
      }
   }

   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_BINDINGS_VS << stage;
}

static void
crocus_delete_state(struct pipe_context *ctx, void *state)
{
   free(state);
}

/**
 * The pipe->set_vertex_buffers() driver hook.
 *
 * This translates pipe_vertex_buffer to our 3DSTATE_VERTEX_BUFFERS packet.
 */
static void
crocus_set_vertex_buffers(struct pipe_context *ctx,
                          unsigned count,
                          unsigned unbind_num_trailing_slots,
                          bool take_ownership,
                          const struct pipe_vertex_buffer *buffers)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_screen *screen = (struct crocus_screen *) ctx->screen;
   const unsigned padding =
      (GFX_VERx10 < 75 && screen->devinfo.platform != INTEL_PLATFORM_BYT) * 2;
   ice->state.bound_vertex_buffers &=
      ~u_bit_consecutive64(0, count + unbind_num_trailing_slots);

   util_set_vertex_buffers_mask(ice->state.vertex_buffers, &ice->state.bound_vertex_buffers,
                                buffers, count, unbind_num_trailing_slots,
                                take_ownership);

   for (unsigned i = 0; i < count; i++) {
      struct pipe_vertex_buffer *state =
         &ice->state.vertex_buffers[i];

      if (!state->is_user_buffer && state->buffer.resource) {
         struct crocus_resource *res = (void *)state->buffer.resource;
         res->bind_history |= PIPE_BIND_VERTEX_BUFFER;
      }

      uint32_t end = 0;
      if (state->buffer.resource)
         end = state->buffer.resource->width0 + padding;
      ice->state.vb_end[i] = end;
   }
   ice->state.dirty |= CROCUS_DIRTY_VERTEX_BUFFERS;
}

#if GFX_VERx10 < 75
static uint8_t get_wa_flags(enum isl_format format)
{
   uint8_t wa_flags = 0;

   switch (format) {
   case ISL_FORMAT_R10G10B10A2_USCALED:
      wa_flags = BRW_ATTRIB_WA_SCALE;
      break;
   case ISL_FORMAT_R10G10B10A2_SSCALED:
      wa_flags = BRW_ATTRIB_WA_SIGN | BRW_ATTRIB_WA_SCALE;
      break;
   case ISL_FORMAT_R10G10B10A2_UNORM:
      wa_flags = BRW_ATTRIB_WA_NORMALIZE;
      break;
   case ISL_FORMAT_R10G10B10A2_SNORM:
      wa_flags = BRW_ATTRIB_WA_SIGN | BRW_ATTRIB_WA_NORMALIZE;
      break;
   case ISL_FORMAT_R10G10B10A2_SINT:
      wa_flags = BRW_ATTRIB_WA_SIGN;
      break;
   case ISL_FORMAT_B10G10R10A2_USCALED:
      wa_flags = BRW_ATTRIB_WA_SCALE | BRW_ATTRIB_WA_BGRA;
      break;
   case ISL_FORMAT_B10G10R10A2_SSCALED:
      wa_flags = BRW_ATTRIB_WA_SIGN | BRW_ATTRIB_WA_SCALE | BRW_ATTRIB_WA_BGRA;
      break;
   case ISL_FORMAT_B10G10R10A2_UNORM:
      wa_flags = BRW_ATTRIB_WA_NORMALIZE | BRW_ATTRIB_WA_BGRA;
      break;
   case ISL_FORMAT_B10G10R10A2_SNORM:
      wa_flags = BRW_ATTRIB_WA_SIGN | BRW_ATTRIB_WA_NORMALIZE | BRW_ATTRIB_WA_BGRA;
      break;
   case ISL_FORMAT_B10G10R10A2_SINT:
      wa_flags = BRW_ATTRIB_WA_SIGN | BRW_ATTRIB_WA_BGRA;
      break;
   case ISL_FORMAT_B10G10R10A2_UINT:
      wa_flags = BRW_ATTRIB_WA_BGRA;
      break;
   default:
      break;
   }
   return wa_flags;
}
#endif

/**
 * Gallium CSO for vertex elements.
 */
struct crocus_vertex_element_state {
   uint32_t vertex_elements[1 + 33 * GENX(VERTEX_ELEMENT_STATE_length)];
#if GFX_VER == 8
   uint32_t vf_instancing[33 * GENX(3DSTATE_VF_INSTANCING_length)];
#endif
   uint32_t edgeflag_ve[GENX(VERTEX_ELEMENT_STATE_length)];
#if GFX_VER == 8
   uint32_t edgeflag_vfi[GENX(3DSTATE_VF_INSTANCING_length)];
#endif
   uint32_t step_rate[16];
   uint8_t wa_flags[33];
   uint16_t strides[16];
   unsigned count;
};

/**
 * The pipe->create_vertex_elements() driver hook.
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
crocus_create_vertex_elements(struct pipe_context *ctx,
                              unsigned count,
                              const struct pipe_vertex_element *state)
{
   struct crocus_screen *screen = (struct crocus_screen *)ctx->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_vertex_element_state *cso =
      calloc(1, sizeof(struct crocus_vertex_element_state));

   cso->count = count;

   crocus_pack_command(GENX(3DSTATE_VERTEX_ELEMENTS), cso->vertex_elements, ve) {
      ve.DWordLength =
         1 + GENX(VERTEX_ELEMENT_STATE_length) * MAX2(count, 1) - 2;
   }

   uint32_t *ve_pack_dest = &cso->vertex_elements[1];
#if GFX_VER == 8
   uint32_t *vfi_pack_dest = cso->vf_instancing;
#endif

   if (count == 0) {
      crocus_pack_state(GENX(VERTEX_ELEMENT_STATE), ve_pack_dest, ve) {
         ve.Valid = true;
         ve.SourceElementFormat = ISL_FORMAT_R32G32B32A32_FLOAT;
         ve.Component0Control = VFCOMP_STORE_0;
         ve.Component1Control = VFCOMP_STORE_0;
         ve.Component2Control = VFCOMP_STORE_0;
         ve.Component3Control = VFCOMP_STORE_1_FP;
      }
#if GFX_VER == 8
      crocus_pack_command(GENX(3DSTATE_VF_INSTANCING), vfi_pack_dest, vi) {
      }
#endif
   }

   for (int i = 0; i < count; i++) {
      const struct crocus_format_info fmt =
         crocus_format_for_usage(devinfo, state[i].src_format, 0);
      unsigned comp[4] = { VFCOMP_STORE_SRC, VFCOMP_STORE_SRC,
                           VFCOMP_STORE_SRC, VFCOMP_STORE_SRC };
      enum isl_format actual_fmt = fmt.fmt;

#if GFX_VERx10 < 75
      cso->wa_flags[i] = get_wa_flags(fmt.fmt);

      if (fmt.fmt == ISL_FORMAT_R10G10B10A2_USCALED ||
          fmt.fmt == ISL_FORMAT_R10G10B10A2_SSCALED ||
          fmt.fmt == ISL_FORMAT_R10G10B10A2_UNORM ||
          fmt.fmt == ISL_FORMAT_R10G10B10A2_SNORM ||
          fmt.fmt == ISL_FORMAT_R10G10B10A2_SINT ||
          fmt.fmt == ISL_FORMAT_B10G10R10A2_USCALED ||
          fmt.fmt == ISL_FORMAT_B10G10R10A2_SSCALED ||
          fmt.fmt == ISL_FORMAT_B10G10R10A2_UNORM ||
          fmt.fmt == ISL_FORMAT_B10G10R10A2_SNORM ||
          fmt.fmt == ISL_FORMAT_B10G10R10A2_UINT ||
          fmt.fmt == ISL_FORMAT_B10G10R10A2_SINT)
         actual_fmt = ISL_FORMAT_R10G10B10A2_UINT;
      if (fmt.fmt == ISL_FORMAT_R8G8B8_SINT)
         actual_fmt = ISL_FORMAT_R8G8B8A8_SINT;
      if (fmt.fmt == ISL_FORMAT_R8G8B8_UINT)
         actual_fmt = ISL_FORMAT_R8G8B8A8_UINT;
      if (fmt.fmt == ISL_FORMAT_R16G16B16_SINT)
         actual_fmt = ISL_FORMAT_R16G16B16A16_SINT;
      if (fmt.fmt == ISL_FORMAT_R16G16B16_UINT)
         actual_fmt = ISL_FORMAT_R16G16B16A16_UINT;
#endif

      cso->step_rate[state[i].vertex_buffer_index] = state[i].instance_divisor;
      cso->strides[state[i].vertex_buffer_index] = state[i].src_stride;

      switch (isl_format_get_num_channels(fmt.fmt)) {
      case 0: comp[0] = VFCOMP_STORE_0; FALLTHROUGH;
      case 1: comp[1] = VFCOMP_STORE_0; FALLTHROUGH;
      case 2: comp[2] = VFCOMP_STORE_0; FALLTHROUGH;
      case 3:
         comp[3] = isl_format_has_int_channel(fmt.fmt) ? VFCOMP_STORE_1_INT
            : VFCOMP_STORE_1_FP;
         break;
      }
      crocus_pack_state(GENX(VERTEX_ELEMENT_STATE), ve_pack_dest, ve) {
#if GFX_VER >= 6
         ve.EdgeFlagEnable = false;
#endif
         ve.VertexBufferIndex = state[i].vertex_buffer_index;
         ve.Valid = true;
         ve.SourceElementOffset = state[i].src_offset;
         ve.SourceElementFormat = actual_fmt;
         ve.Component0Control = comp[0];
         ve.Component1Control = comp[1];
         ve.Component2Control = comp[2];
         ve.Component3Control = comp[3];
#if GFX_VER < 5
         ve.DestinationElementOffset = i * 4;
#endif
      }

#if GFX_VER == 8
      crocus_pack_command(GENX(3DSTATE_VF_INSTANCING), vfi_pack_dest, vi) {
         vi.VertexElementIndex = i;
         vi.InstancingEnable = state[i].instance_divisor > 0;
         vi.InstanceDataStepRate = state[i].instance_divisor;
      }
#endif
      ve_pack_dest += GENX(VERTEX_ELEMENT_STATE_length);
#if GFX_VER == 8
      vfi_pack_dest += GENX(3DSTATE_VF_INSTANCING_length);
#endif
   }

   /* An alternative version of the last VE and VFI is stored so it
    * can be used at draw time in case Vertex Shader uses EdgeFlag
    */
   if (count) {
      const unsigned edgeflag_index = count - 1;
      const struct crocus_format_info fmt =
         crocus_format_for_usage(devinfo, state[edgeflag_index].src_format, 0);
      crocus_pack_state(GENX(VERTEX_ELEMENT_STATE), cso->edgeflag_ve, ve) {
#if GFX_VER >= 6
         ve.EdgeFlagEnable = true;
#endif
         ve.VertexBufferIndex = state[edgeflag_index].vertex_buffer_index;
         ve.Valid = true;
         ve.SourceElementOffset = state[edgeflag_index].src_offset;
         ve.SourceElementFormat = fmt.fmt;
         ve.Component0Control = VFCOMP_STORE_SRC;
         ve.Component1Control = VFCOMP_STORE_0;
         ve.Component2Control = VFCOMP_STORE_0;
         ve.Component3Control = VFCOMP_STORE_0;
      }
#if GFX_VER == 8
      crocus_pack_command(GENX(3DSTATE_VF_INSTANCING), cso->edgeflag_vfi, vi) {
         /* The vi.VertexElementIndex of the EdgeFlag Vertex Element is filled
          * at draw time, as it should change if SGVs are emitted.
          */
         vi.InstancingEnable = state[edgeflag_index].instance_divisor > 0;
         vi.InstanceDataStepRate = state[edgeflag_index].instance_divisor;
      }
#endif
   }

   return cso;
}

/**
 * The pipe->bind_vertex_elements_state() driver hook.
 */
static void
crocus_bind_vertex_elements_state(struct pipe_context *ctx, void *state)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
#if GFX_VER == 8
   struct crocus_vertex_element_state *old_cso = ice->state.cso_vertex_elements;
   struct crocus_vertex_element_state *new_cso = state;

   if (new_cso && cso_changed(count))
      ice->state.dirty |= CROCUS_DIRTY_GEN8_VF_SGVS;
#endif
   ice->state.cso_vertex_elements = state;
   ice->state.dirty |= CROCUS_DIRTY_VERTEX_ELEMENTS | CROCUS_DIRTY_VERTEX_BUFFERS;
   ice->state.stage_dirty |= ice->state.stage_dirty_for_nos[CROCUS_NOS_VERTEX_ELEMENTS];
}

#if GFX_VER >= 6
struct crocus_streamout_counter {
   uint32_t offset_start;
   uint32_t offset_end;

   uint64_t accum;
};

/**
 * Gallium CSO for stream output (transform feedback) targets.
 */
struct crocus_stream_output_target {
   struct pipe_stream_output_target base;

   /** Stride (bytes-per-vertex) during this transform feedback operation */
   uint16_t stride;

   /** Has 3DSTATE_SO_BUFFER actually been emitted, zeroing the offsets? */
   bool zeroed;

   struct crocus_resource *offset_res;
   uint32_t offset_offset;

#if GFX_VER == 6
   void *prim_map;
   struct crocus_streamout_counter prev_count;
   struct crocus_streamout_counter count;
#endif
#if GFX_VER == 8
   /** Does the next 3DSTATE_SO_BUFFER need to zero the offsets? */
   bool zero_offset;
#endif
};

#if GFX_VER >= 7
static uint32_t
crocus_get_so_offset(struct pipe_stream_output_target *so)
{
   struct crocus_stream_output_target *tgt = (void *)so;
   struct pipe_transfer *transfer;
   struct pipe_box box;
   uint32_t result;
   u_box_1d(tgt->offset_offset, 4, &box);
   void *val = so->context->buffer_map(so->context, &tgt->offset_res->base.b,
                                       0, PIPE_MAP_DIRECTLY,
                                       &box, &transfer);
   assert(val);
   result = *(uint32_t *)val;
   so->context->buffer_unmap(so->context, transfer);

   return result / tgt->stride;
}
#endif

#if GFX_VER == 6
static void
compute_vertices_written_so_far(struct crocus_context *ice,
                                struct crocus_stream_output_target *tgt,
                                struct crocus_streamout_counter *count,
                                uint64_t *svbi);

static uint32_t
crocus_get_so_offset(struct pipe_stream_output_target *so)
{
   struct crocus_stream_output_target *tgt = (void *)so;
   struct crocus_context *ice = (void *)so->context;

   uint64_t vert_written;
   compute_vertices_written_so_far(ice, tgt, &tgt->prev_count, &vert_written);
   return vert_written;
}
#endif

/**
 * The pipe->create_stream_output_target() driver hook.
 *
 * "Target" here refers to a destination buffer.  We translate this into
 * a 3DSTATE_SO_BUFFER packet.  We can handle most fields, but don't yet
 * know which buffer this represents, or whether we ought to zero the
 * write-offsets, or append.  Those are handled in the set() hook.
 */
static struct pipe_stream_output_target *
crocus_create_stream_output_target(struct pipe_context *ctx,
                                   struct pipe_resource *p_res,
                                   unsigned buffer_offset,
                                   unsigned buffer_size)
{
   struct crocus_resource *res = (void *) p_res;
   struct crocus_stream_output_target *cso = calloc(1, sizeof(*cso));
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
#if GFX_VER >= 7
   struct crocus_context *ice = (struct crocus_context *) ctx;
   void *temp;
   u_upload_alloc(ice->ctx.stream_uploader, 0, sizeof(uint32_t), 4,
                  &cso->offset_offset,
                  (struct pipe_resource **)&cso->offset_res,
                  &temp);
#endif

   return &cso->base;
}

static void
crocus_stream_output_target_destroy(struct pipe_context *ctx,
                                    struct pipe_stream_output_target *state)
{
   struct crocus_stream_output_target *cso = (void *) state;

   pipe_resource_reference((struct pipe_resource **)&cso->offset_res, NULL);
   pipe_resource_reference(&cso->base.buffer, NULL);

   free(cso);
}

#define GEN6_SO_NUM_PRIMS_WRITTEN       0x2288
#define GEN7_SO_WRITE_OFFSET(n)         (0x5280 + (n) * 4)

#if GFX_VER == 6
static void
aggregate_stream_counter(struct crocus_batch *batch, struct crocus_stream_output_target *tgt,
                         struct crocus_streamout_counter *counter)
{
   uint64_t *prim_counts = tgt->prim_map;

   if (crocus_batch_references(batch, tgt->offset_res->bo)) {
      struct pipe_fence_handle *out_fence = NULL;
      batch->ice->ctx.flush(&batch->ice->ctx, &out_fence, 0);
      batch->screen->base.fence_finish(&batch->screen->base, &batch->ice->ctx, out_fence, UINT64_MAX);
      batch->screen->base.fence_reference(&batch->screen->base, &out_fence, NULL);
   }

   for (unsigned i = counter->offset_start / sizeof(uint64_t); i < counter->offset_end / sizeof(uint64_t); i += 2) {
      counter->accum += prim_counts[i + 1] - prim_counts[i];
   }
   tgt->count.offset_start = tgt->count.offset_end = 0;
}

static void
crocus_stream_store_prims_written(struct crocus_batch *batch,
                                  struct crocus_stream_output_target *tgt)
{
   if (!tgt->offset_res) {
      u_upload_alloc(batch->ice->ctx.stream_uploader, 0, 4096, 4,
                     &tgt->offset_offset,
                     (struct pipe_resource **)&tgt->offset_res,
                     &tgt->prim_map);
      tgt->count.offset_start = tgt->count.offset_end = 0;
   }

   if (tgt->count.offset_end + 16 >= 4096) {
      aggregate_stream_counter(batch, tgt, &tgt->prev_count);
      aggregate_stream_counter(batch, tgt, &tgt->count);
   }

   crocus_emit_mi_flush(batch);
   crocus_store_register_mem64(batch, GEN6_SO_NUM_PRIMS_WRITTEN,
                               tgt->offset_res->bo,
                               tgt->count.offset_end + tgt->offset_offset, false);
   tgt->count.offset_end += 8;
}

static void
compute_vertices_written_so_far(struct crocus_context *ice,
                                struct crocus_stream_output_target *tgt,
                                struct crocus_streamout_counter *counter,
                                uint64_t *svbi)
{
   //TODO vertices per prim
   aggregate_stream_counter(&ice->batches[0], tgt, counter);

   *svbi = counter->accum * ice->state.last_xfb_verts_per_prim;
}
#endif
/**
 * The pipe->set_stream_output_targets() driver hook.
 *
 * At this point, we know which targets are bound to a particular index,
 * and also whether we want to append or start over.  We can finish the
 * 3DSTATE_SO_BUFFER packets we started earlier.
 */
static void
crocus_set_stream_output_targets(struct pipe_context *ctx,
                                 unsigned num_targets,
                                 struct pipe_stream_output_target **targets,
                                 const unsigned *offsets)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_RENDER];
   struct pipe_stream_output_target *old_tgt[4] = { NULL, NULL, NULL, NULL };
   const bool active = num_targets > 0;
   if (ice->state.streamout_active != active) {
      ice->state.streamout_active = active;
#if GFX_VER >= 7
      ice->state.dirty |= CROCUS_DIRTY_STREAMOUT;
#else
      ice->state.dirty |= CROCUS_DIRTY_GEN4_FF_GS_PROG;
#endif

      /* We only emit 3DSTATE_SO_DECL_LIST when streamout is active, because
       * it's a non-pipelined command.  If we're switching streamout on, we
       * may have missed emitting it earlier, so do so now.  (We're already
       * taking a stall to update 3DSTATE_SO_BUFFERS anyway...)
       */
      if (active) {
#if GFX_VER >= 7
         ice->state.dirty |= CROCUS_DIRTY_SO_DECL_LIST;
#endif
      } else {
         uint32_t flush = 0;
         for (int i = 0; i < PIPE_MAX_SO_BUFFERS; i++) {
            struct crocus_stream_output_target *tgt =
               (void *) ice->state.so_target[i];
            if (tgt) {
               struct crocus_resource *res = (void *) tgt->base.buffer;

               flush |= crocus_flush_bits_for_history(res);
               crocus_dirty_for_history(ice, res);
            }
         }
         crocus_emit_pipe_control_flush(&ice->batches[CROCUS_BATCH_RENDER],
                                        "make streamout results visible", flush);
      }
   }

   ice->state.so_targets = num_targets;
   for (int i = 0; i < 4; i++) {
      pipe_so_target_reference(&old_tgt[i], ice->state.so_target[i]);
      pipe_so_target_reference(&ice->state.so_target[i],
                               i < num_targets ? targets[i] : NULL);
   }

#if GFX_VER == 6
   bool stored_num_prims = false;
   for (int i = 0; i < PIPE_MAX_SO_BUFFERS; i++) {
      if (num_targets) {
         struct crocus_stream_output_target *tgt =
            (void *) ice->state.so_target[i];

         if (!tgt)
            continue;
         if (offsets[i] == 0) {
            // This means that we're supposed to ignore anything written to
            // the buffer before. We can do this by just clearing out the
            // count of writes to the prim count buffer.
            tgt->count.offset_start = tgt->count.offset_end;
            tgt->count.accum = 0;
            ice->state.svbi = 0;
         } else {
            if (tgt->offset_res) {
               compute_vertices_written_so_far(ice, tgt, &tgt->count, &ice->state.svbi);
               tgt->count.offset_start = tgt->count.offset_end;
            }
         }

         if (!stored_num_prims) {
            crocus_stream_store_prims_written(batch, tgt);
            stored_num_prims = true;
         }
      } else {
         struct crocus_stream_output_target *tgt =
            (void *) old_tgt[i];
         if (tgt) {
            if (!stored_num_prims) {
               crocus_stream_store_prims_written(batch, tgt);
               stored_num_prims = true;
            }

            if (tgt->offset_res) {
               tgt->prev_count = tgt->count;
            }
         }
      }
      pipe_so_target_reference(&old_tgt[i], NULL);
   }
   ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_BINDINGS_GS;
#else
   for (int i = 0; i < PIPE_MAX_SO_BUFFERS; i++) {
      if (num_targets) {
         struct crocus_stream_output_target *tgt =
            (void *) ice->state.so_target[i];

         if (offsets[i] == 0) {
#if GFX_VER == 8
            if (tgt)
               tgt->zero_offset = true;
#endif
            crocus_load_register_imm32(batch, GEN7_SO_WRITE_OFFSET(i), 0);
         }
         else if (tgt)
            crocus_load_register_mem32(batch, GEN7_SO_WRITE_OFFSET(i),
                                       tgt->offset_res->bo,
                                       tgt->offset_offset);
      } else {
         struct crocus_stream_output_target *tgt =
            (void *) old_tgt[i];
         if (tgt)
            crocus_store_register_mem32(batch, GEN7_SO_WRITE_OFFSET(i),
                                        tgt->offset_res->bo,
                                        tgt->offset_offset, false);
      }
      pipe_so_target_reference(&old_tgt[i], NULL);
   }
#endif
   /* No need to update 3DSTATE_SO_BUFFER unless SOL is active. */
   if (!active)
      return;
#if GFX_VER >= 7
   ice->state.dirty |= CROCUS_DIRTY_GEN7_SO_BUFFERS;
#elif GFX_VER == 6
   ice->state.dirty |= CROCUS_DIRTY_GEN6_SVBI;
#endif
}

#endif

#if GFX_VER >= 7
/**
 * An crocus-vtable helper for encoding the 3DSTATE_SO_DECL_LIST and
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
crocus_create_so_decl_list(const struct pipe_stream_output_info *info,
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

   crocus_pack_command(GENX(3DSTATE_STREAMOUT), map, sol) {
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

      // TODO: Double-check that stride == 0 means no buffer. Probably this
      // needs to go elsewhere, where the buffer enable stuff is actually
      // known.
#if GFX_VER < 8
      sol.SOBufferEnable0 = !!info->stride[0];
      sol.SOBufferEnable1 = !!info->stride[1];
      sol.SOBufferEnable2 = !!info->stride[2];
      sol.SOBufferEnable3 = !!info->stride[3];
#else
      /* Set buffer pitches; 0 means unbound. */
      sol.Buffer0SurfacePitch = 4 * info->stride[0];
      sol.Buffer1SurfacePitch = 4 * info->stride[1];
      sol.Buffer2SurfacePitch = 4 * info->stride[2];
      sol.Buffer3SurfacePitch = 4 * info->stride[3];
#endif
   }

   crocus_pack_command(GENX(3DSTATE_SO_DECL_LIST), so_decl_map, list) {
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
      crocus_pack_state(GENX(SO_DECL_ENTRY), so_decl_map + 3 + i * 2, entry) {
         entry.Stream0Decl = so_decl[0][i];
         entry.Stream1Decl = so_decl[1][i];
         entry.Stream2Decl = so_decl[2][i];
         entry.Stream3Decl = so_decl[3][i];
      }
   }

   return map;
}
#endif

#if GFX_VER == 6
static void
crocus_emit_so_svbi(struct crocus_context *ice)
{
   struct crocus_batch *batch = &ice->batches[CROCUS_BATCH_RENDER];

   unsigned max_vertex = 0xffffffff;
   for (int i = 0; i < PIPE_MAX_SO_BUFFERS; i++) {
      struct crocus_stream_output_target *tgt =
         (void *) ice->state.so_target[i];
      if (tgt)
         max_vertex = MIN2(max_vertex, tgt->base.buffer_size / tgt->stride);
   }

   crocus_emit_cmd(batch, GENX(3DSTATE_GS_SVB_INDEX), svbi) {
      svbi.IndexNumber = 0;
      svbi.StreamedVertexBufferIndex = (uint32_t)ice->state.svbi; /* fix when resuming, based on target's prim count */
      svbi.MaximumIndex = max_vertex;
   }

   /* initialize the rest of the SVBI's to reasonable values so that we don't
    * run out of room writing the regular data.
    */
   for (int i = 1; i < 4; i++) {
      crocus_emit_cmd(batch, GENX(3DSTATE_GS_SVB_INDEX), svbi) {
         svbi.IndexNumber = i;
         svbi.StreamedVertexBufferIndex = 0;
         svbi.MaximumIndex = 0xffffffff;
      }
   }
}

#endif


#if GFX_VER >= 6
static bool
crocus_is_drawing_points(const struct crocus_context *ice)
{
   const struct crocus_rasterizer_state *cso_rast = ice->state.cso_rast;

   if (cso_rast->cso.fill_front == PIPE_POLYGON_MODE_POINT ||
       cso_rast->cso.fill_back == PIPE_POLYGON_MODE_POINT)
      return true;

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
#endif

#if GFX_VER >= 6
static void
get_attr_override(
   struct GENX(SF_OUTPUT_ATTRIBUTE_DETAIL) *attr,
   const struct brw_vue_map *vue_map,
   int urb_entry_read_offset, int fs_attr,
   bool two_side_color, uint32_t *max_source_attr)
{
   /* Find the VUE slot for this attribute. */
   int slot = vue_map->varying_to_slot[fs_attr];

   /* Viewport and Layer are stored in the VUE header.  We need to override
    * them to zero if earlier stages didn't write them, as GL requires that
    * they read back as zero when not explicitly set.
    */
   if (fs_attr == VARYING_SLOT_VIEWPORT || fs_attr == VARYING_SLOT_LAYER) {
      attr->ComponentOverrideX = true;
      attr->ComponentOverrideW = true;
      attr->ConstantSource = CONST_0000;

      if (!(vue_map->slots_valid & VARYING_BIT_LAYER))
         attr->ComponentOverrideY = true;
      if (!(vue_map->slots_valid & VARYING_BIT_VIEWPORT))
         attr->ComponentOverrideZ = true;

      return;
   }

   /* If there was only a back color written but not front, use back
    * as the color instead of undefined
    */
   if (slot == -1 && fs_attr == VARYING_SLOT_COL0)
      slot = vue_map->varying_to_slot[VARYING_SLOT_BFC0];
   if (slot == -1 && fs_attr == VARYING_SLOT_COL1)
      slot = vue_map->varying_to_slot[VARYING_SLOT_BFC1];

   if (slot == -1) {
      /* This attribute does not exist in the VUE--that means that the vertex
       * shader did not write to it.  This means that either:
       *
       * (a) This attribute is a texture coordinate, and it is going to be
       * replaced with point coordinates (as a consequence of a call to
       * glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE)), so the
       * hardware will ignore whatever attribute override we supply.
       *
       * (b) This attribute is read by the fragment shader but not written by
       * the vertex shader, so its value is undefined.  Therefore the
       * attribute override we supply doesn't matter.
       *
       * (c) This attribute is gl_PrimitiveID, and it wasn't written by the
       * previous shader stage.
       *
       * Note that we don't have to worry about the cases where the attribute
       * is gl_PointCoord or is undergoing point sprite coordinate
       * replacement, because in those cases, this function isn't called.
       *
       * In case (c), we need to program the attribute overrides so that the
       * primitive ID will be stored in this slot.  In every other case, the
       * attribute override we supply doesn't matter.  So just go ahead and
       * program primitive ID in every case.
       */
      attr->ComponentOverrideW = true;
      attr->ComponentOverrideX = true;
      attr->ComponentOverrideY = true;
      attr->ComponentOverrideZ = true;
      attr->ConstantSource = PRIM_ID;
      return;
   }

   /* Compute the location of the attribute relative to urb_entry_read_offset.
    * Each increment of urb_entry_read_offset represents a 256-bit value, so
    * it counts for two 128-bit VUE slots.
    */
   int source_attr = slot - 2 * urb_entry_read_offset;
   assert(source_attr >= 0 && source_attr < 32);

   /* If we are doing two-sided color, and the VUE slot following this one
    * represents a back-facing color, then we need to instruct the SF unit to
    * do back-facing swizzling.
    */
   bool swizzling = two_side_color &&
      ((vue_map->slot_to_varying[slot] == VARYING_SLOT_COL0 &&
        vue_map->slot_to_varying[slot+1] == VARYING_SLOT_BFC0) ||
       (vue_map->slot_to_varying[slot] == VARYING_SLOT_COL1 &&
        vue_map->slot_to_varying[slot+1] == VARYING_SLOT_BFC1));

   /* Update max_source_attr.  If swizzling, the SF will read this slot + 1. */
   if (*max_source_attr < source_attr + swizzling)
      *max_source_attr = source_attr + swizzling;

   attr->SourceAttribute = source_attr;
   if (swizzling)
      attr->SwizzleSelect = INPUTATTR_FACING;
}

static void
calculate_attr_overrides(
   const struct crocus_context *ice,
   struct GENX(SF_OUTPUT_ATTRIBUTE_DETAIL) *attr_overrides,
   uint32_t *point_sprite_enables,
   uint32_t *urb_entry_read_length,
   uint32_t *urb_entry_read_offset)
{
   const struct brw_wm_prog_data *wm_prog_data = (void *)
      ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data;
   const struct brw_vue_map *vue_map = ice->shaders.last_vue_map;
   const struct crocus_rasterizer_state *cso_rast = ice->state.cso_rast;
   uint32_t max_source_attr = 0;
   const struct shader_info *fs_info =
      crocus_get_shader_info(ice, MESA_SHADER_FRAGMENT);

   int first_slot =
      brw_compute_first_urb_slot_required(fs_info->inputs_read, vue_map);

   /* Each URB offset packs two varying slots */
   assert(first_slot % 2 == 0);
   *urb_entry_read_offset = first_slot / 2;
   *point_sprite_enables = 0;

   for (int fs_attr = 0; fs_attr < VARYING_SLOT_MAX; fs_attr++) {
      const int input_index = wm_prog_data->urb_setup[fs_attr];

      if (input_index < 0)
         continue;

      bool point_sprite = false;
      if (crocus_is_drawing_points(ice)) {
         if (fs_attr >= VARYING_SLOT_TEX0 &&
             fs_attr <= VARYING_SLOT_TEX7 &&
             cso_rast->cso.sprite_coord_enable & (1 << (fs_attr - VARYING_SLOT_TEX0)))
            point_sprite = true;

         if (fs_attr == VARYING_SLOT_PNTC)
            point_sprite = true;

         if (point_sprite)
            *point_sprite_enables |= 1U << input_index;
      }

      struct GENX(SF_OUTPUT_ATTRIBUTE_DETAIL) attribute = { 0 };
      if (!point_sprite) {
         get_attr_override(&attribute, vue_map, *urb_entry_read_offset, fs_attr,
                           cso_rast->cso.light_twoside, &max_source_attr);
      }

      /* The hardware can only do the overrides on 16 overrides at a
       * time, and the other up to 16 have to be lined up so that the
       * input index = the output index.  We'll need to do some
       * tweaking to make sure that's the case.
       */
      if (input_index < 16)
         attr_overrides[input_index] = attribute;
      else
         assert(attribute.SourceAttribute == input_index);
   }

   /* From the Sandy Bridge PRM, Volume 2, Part 1, documentation for
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
    */
   *urb_entry_read_length = DIV_ROUND_UP(max_source_attr + 1, 2);
}
#endif

#if GFX_VER >= 7
static void
crocus_emit_sbe(struct crocus_batch *batch, const struct crocus_context *ice)
{
   const struct crocus_rasterizer_state *cso_rast = ice->state.cso_rast;
   const struct brw_wm_prog_data *wm_prog_data = (void *)
      ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data;
#if GFX_VER >= 8
   struct GENX(SF_OUTPUT_ATTRIBUTE_DETAIL) attr_overrides[16] = { { 0 } };
#else
#define attr_overrides sbe.Attribute
#endif

   uint32_t urb_entry_read_length;
   uint32_t urb_entry_read_offset;
   uint32_t point_sprite_enables;

   crocus_emit_cmd(batch, GENX(3DSTATE_SBE), sbe) {
      sbe.AttributeSwizzleEnable = true;
      sbe.NumberofSFOutputAttributes = wm_prog_data->num_varying_inputs;
      sbe.PointSpriteTextureCoordinateOrigin = cso_rast->cso.sprite_coord_mode;

      calculate_attr_overrides(ice,
                               attr_overrides,
                               &point_sprite_enables,
                               &urb_entry_read_length,
                               &urb_entry_read_offset);
      sbe.VertexURBEntryReadOffset = urb_entry_read_offset;
      sbe.VertexURBEntryReadLength = urb_entry_read_length;
      sbe.ConstantInterpolationEnable = wm_prog_data->flat_inputs;
      sbe.PointSpriteTextureCoordinateEnable = point_sprite_enables;
#if GFX_VER >= 8
      sbe.ForceVertexURBEntryReadLength = true;
      sbe.ForceVertexURBEntryReadOffset = true;
#endif
   }
#if GFX_VER >= 8
   crocus_emit_cmd(batch, GENX(3DSTATE_SBE_SWIZ), sbes) {
      for (int i = 0; i < 16; i++)
         sbes.Attribute[i] = attr_overrides[i];
   }
#endif
}
#endif

/* ------------------------------------------------------------------- */

/**
 * Populate VS program key fields based on the current state.
 */
static void
crocus_populate_vs_key(const struct crocus_context *ice,
                       const struct shader_info *info,
                       gl_shader_stage last_stage,
                       struct brw_vs_prog_key *key)
{
   const struct crocus_rasterizer_state *cso_rast = ice->state.cso_rast;

   if (info->clip_distance_array_size == 0 &&
       (info->outputs_written & (VARYING_BIT_POS | VARYING_BIT_CLIP_VERTEX)) &&
       last_stage == MESA_SHADER_VERTEX)
      key->nr_userclip_plane_consts = cso_rast->num_clip_plane_consts;

   if (last_stage == MESA_SHADER_VERTEX &&
       info->outputs_written & (VARYING_BIT_PSIZ))
      key->clamp_pointsize = 1;

#if GFX_VER <= 5
   key->copy_edgeflag = (cso_rast->cso.fill_back != PIPE_POLYGON_MODE_FILL ||
                         cso_rast->cso.fill_front != PIPE_POLYGON_MODE_FILL);
   key->point_coord_replace = cso_rast->cso.sprite_coord_enable & 0xff;
#endif

   key->clamp_vertex_color = cso_rast->cso.clamp_vertex_color;

#if GFX_VERx10 < 75
   uint64_t inputs_read = info->inputs_read;
   int ve_idx = 0;
   while (inputs_read) {
      int i = u_bit_scan64(&inputs_read);
      key->gl_attrib_wa_flags[i] = ice->state.cso_vertex_elements->wa_flags[ve_idx];
      ve_idx++;
   }
#endif
}

/**
 * Populate TCS program key fields based on the current state.
 */
static void
crocus_populate_tcs_key(const struct crocus_context *ice,
                        struct brw_tcs_prog_key *key)
{
}

/**
 * Populate TES program key fields based on the current state.
 */
static void
crocus_populate_tes_key(const struct crocus_context *ice,
                        const struct shader_info *info,
                        gl_shader_stage last_stage,
                        struct brw_tes_prog_key *key)
{
   const struct crocus_rasterizer_state *cso_rast = ice->state.cso_rast;

   if (info->clip_distance_array_size == 0 &&
       (info->outputs_written & (VARYING_BIT_POS | VARYING_BIT_CLIP_VERTEX)) &&
       last_stage == MESA_SHADER_TESS_EVAL)
      key->nr_userclip_plane_consts = cso_rast->num_clip_plane_consts;

   if (last_stage == MESA_SHADER_TESS_EVAL &&
       info->outputs_written & (VARYING_BIT_PSIZ))
      key->clamp_pointsize = 1;
}

/**
 * Populate GS program key fields based on the current state.
 */
static void
crocus_populate_gs_key(const struct crocus_context *ice,
                       const struct shader_info *info,
                       gl_shader_stage last_stage,
                       struct brw_gs_prog_key *key)
{
   const struct crocus_rasterizer_state *cso_rast = ice->state.cso_rast;

   if (info->clip_distance_array_size == 0 &&
       (info->outputs_written & (VARYING_BIT_POS | VARYING_BIT_CLIP_VERTEX)) &&
       last_stage == MESA_SHADER_GEOMETRY)
      key->nr_userclip_plane_consts = cso_rast->num_clip_plane_consts;

   if (last_stage == MESA_SHADER_GEOMETRY &&
       info->outputs_written & (VARYING_BIT_PSIZ))
      key->clamp_pointsize = 1;
}

/**
 * Populate FS program key fields based on the current state.
 */
static void
crocus_populate_fs_key(const struct crocus_context *ice,
                       const struct shader_info *info,
                       struct brw_wm_prog_key *key)
{
   struct crocus_screen *screen = (void *) ice->ctx.screen;
   const struct pipe_framebuffer_state *fb = &ice->state.framebuffer;
   const struct crocus_depth_stencil_alpha_state *zsa = ice->state.cso_zsa;
   const struct crocus_rasterizer_state *rast = ice->state.cso_rast;
   const struct crocus_blend_state *blend = ice->state.cso_blend;

#if GFX_VER < 6
   uint32_t lookup = 0;

   if (info->fs.uses_discard || zsa->cso.alpha_enabled)
      lookup |= BRW_WM_IZ_PS_KILL_ALPHATEST_BIT;

   if (info->outputs_written & BITFIELD64_BIT(FRAG_RESULT_DEPTH))
      lookup |= BRW_WM_IZ_PS_COMPUTES_DEPTH_BIT;

   if (fb->zsbuf && zsa->cso.depth_enabled) {
      lookup |= BRW_WM_IZ_DEPTH_TEST_ENABLE_BIT;

      if (zsa->cso.depth_writemask)
         lookup |= BRW_WM_IZ_DEPTH_WRITE_ENABLE_BIT;

   }
   if (zsa->cso.stencil[0].enabled || zsa->cso.stencil[1].enabled) {
      lookup |= BRW_WM_IZ_STENCIL_TEST_ENABLE_BIT;
      if (zsa->cso.stencil[0].writemask || zsa->cso.stencil[1].writemask)
         lookup |= BRW_WM_IZ_STENCIL_WRITE_ENABLE_BIT;
   }
   key->iz_lookup = lookup;
   key->stats_wm = ice->state.stats_wm;
#endif

   uint32_t line_aa = BRW_NEVER;
   if (rast->cso.line_smooth) {
      int reduced_prim = ice->state.reduced_prim_mode;
      if (reduced_prim == MESA_PRIM_LINES)
         line_aa = BRW_ALWAYS;
      else if (reduced_prim == MESA_PRIM_TRIANGLES) {
         if (rast->cso.fill_front == PIPE_POLYGON_MODE_LINE) {
            line_aa = BRW_SOMETIMES;

            if (rast->cso.fill_back == PIPE_POLYGON_MODE_LINE ||
                rast->cso.cull_face == PIPE_FACE_BACK)
               line_aa = BRW_ALWAYS;
         } else if (rast->cso.fill_back == PIPE_POLYGON_MODE_LINE) {
            line_aa = BRW_SOMETIMES;

            if (rast->cso.cull_face == PIPE_FACE_FRONT)
               line_aa = BRW_ALWAYS;
         }
      }
   }
   key->line_aa = line_aa;

   key->nr_color_regions = fb->nr_cbufs;

   key->clamp_fragment_color = rast->cso.clamp_fragment_color;

   key->alpha_to_coverage = blend->cso.alpha_to_coverage ?
      BRW_ALWAYS : BRW_NEVER;

   key->alpha_test_replicate_alpha = fb->nr_cbufs > 1 && zsa->cso.alpha_enabled;

   key->flat_shade = rast->cso.flatshade &&
      (info->inputs_read & (VARYING_BIT_COL0 | VARYING_BIT_COL1));

   const bool multisample_fbo = rast->cso.multisample && fb->samples > 1;
   key->multisample_fbo = multisample_fbo ? BRW_ALWAYS : BRW_NEVER;
   key->persample_interp =
      rast->cso.force_persample_interp ? BRW_ALWAYS : BRW_NEVER;

   key->ignore_sample_mask_out = !multisample_fbo;
   key->coherent_fb_fetch = false; // TODO: needed?

   key->force_dual_color_blend =
      screen->driconf.dual_color_blend_by_location &&
      (blend->blend_enables & 1) && blend->dual_color_blending;

#if GFX_VER <= 5
   if (fb->nr_cbufs > 1 && zsa->cso.alpha_enabled) {
      key->emit_alpha_test = true;
      key->alpha_test_func = zsa->cso.alpha_func;
      key->alpha_test_ref = zsa->cso.alpha_ref_value;
   }
#endif
}

static void
crocus_populate_cs_key(const struct crocus_context *ice,
                       struct brw_cs_prog_key *key)
{
}

#if GFX_VER == 4
#define KSP(ice, shader) ro_bo((ice)->shaders.cache_bo, (shader)->offset);
#elif GFX_VER >= 5
static uint64_t
KSP(const struct crocus_context *ice, const struct crocus_compiled_shader *shader)
{
   return shader->offset;
}
#endif

/* Gen11 workaround table #2056 WABTPPrefetchDisable suggests to disable
 * prefetching of binding tables in A0 and B0 steppings.  XXX: Revisit
 * this WA on C0 stepping.
 *
 * TODO: Fill out SamplerCount for prefetching?
 */

#define INIT_THREAD_DISPATCH_FIELDS(pkt, prefix, stage)                 \
   pkt.KernelStartPointer = KSP(ice, shader);                           \
   pkt.BindingTableEntryCount = shader->bt.size_bytes / 4;              \
   pkt.FloatingPointMode = prog_data->use_alt_mode;                     \
                                                                        \
   pkt.DispatchGRFStartRegisterForURBData =                             \
      prog_data->dispatch_grf_start_reg;                                \
   pkt.prefix##URBEntryReadLength = vue_prog_data->urb_read_length;     \
   pkt.prefix##URBEntryReadOffset = 0;                                  \
                                                                        \
   pkt.StatisticsEnable = true;                                         \
   pkt.Enable           = true;                                         \
                                                                        \
   if (prog_data->total_scratch) {                                      \
      struct crocus_bo *bo =                                            \
         crocus_get_scratch_space(ice, prog_data->total_scratch, stage); \
      pkt.PerThreadScratchSpace = ffs(prog_data->total_scratch) - 11;   \
      pkt.ScratchSpaceBasePointer = rw_bo(bo, 0);                       \
   }

/* ------------------------------------------------------------------- */
#if GFX_VER >= 6
static const uint32_t push_constant_opcodes[] = {
   [MESA_SHADER_VERTEX]    = 21,
   [MESA_SHADER_TESS_CTRL] = 25, /* HS */
   [MESA_SHADER_TESS_EVAL] = 26, /* DS */
   [MESA_SHADER_GEOMETRY]  = 22,
   [MESA_SHADER_FRAGMENT]  = 23,
   [MESA_SHADER_COMPUTE]   = 0,
};
#endif

static void
emit_sized_null_surface(struct crocus_batch *batch,
                        unsigned width, unsigned height,
                        unsigned layers, unsigned levels,
                        unsigned minimum_array_element,
                        uint32_t *out_offset)
{
   struct isl_device *isl_dev = &batch->screen->isl_dev;
   uint32_t *surf = stream_state(batch, isl_dev->ss.size,
                                 isl_dev->ss.align,
                                 out_offset);
   //TODO gen 6 multisample crash
   isl_null_fill_state(isl_dev, surf,
                       .size = isl_extent3d(width, height, layers),
                       .levels = levels,
                       .minimum_array_element = minimum_array_element);
}
static void
emit_null_surface(struct crocus_batch *batch,
                  uint32_t *out_offset)
{
   emit_sized_null_surface(batch, 1, 1, 1, 0, 0, out_offset);
}

static void
emit_null_fb_surface(struct crocus_batch *batch,
                     struct crocus_context *ice,
                     uint32_t *out_offset)
{
   uint32_t width, height, layers, level, layer;
   /* If set_framebuffer_state() was never called, fall back to 1x1x1 */
   if (ice->state.framebuffer.width == 0 && ice->state.framebuffer.height == 0) {
      emit_null_surface(batch, out_offset);
      return;
   }

   struct pipe_framebuffer_state *cso = &ice->state.framebuffer;
   width = MAX2(cso->width, 1);
   height = MAX2(cso->height, 1);
   layers = cso->layers ? cso->layers : 1;
   level = 0;
   layer = 0;

   if (cso->nr_cbufs == 0 && cso->zsbuf) {
      width = cso->zsbuf->width;
      height = cso->zsbuf->height;
      level = cso->zsbuf->u.tex.level;
      layer = cso->zsbuf->u.tex.first_layer;
   }
   emit_sized_null_surface(batch, width, height,
                           layers, level, layer,
                           out_offset);
}

static void
emit_surface_state(struct crocus_batch *batch,
                   struct crocus_resource *res,
                   const struct isl_surf *in_surf,
                   bool adjust_surf,
                   struct isl_view *in_view,
                   bool writeable,
                   enum isl_aux_usage aux_usage,
                   bool blend_enable,
                   uint32_t write_disables,
                   uint32_t *surf_state,
                   uint32_t addr_offset)
{
   struct isl_device *isl_dev = &batch->screen->isl_dev;
   uint32_t reloc = RELOC_32BIT;
   uint64_t offset_B = res->offset;
   uint32_t tile_x_sa = 0, tile_y_sa = 0;

   if (writeable)
      reloc |= RELOC_WRITE;

   struct isl_surf surf = *in_surf;
   struct isl_view view = *in_view;
   if (adjust_surf) {
      if (res->base.b.target == PIPE_TEXTURE_3D && view.array_len == 1) {
         isl_surf_get_image_surf(isl_dev, in_surf,
                                 view.base_level, 0,
                                 view.base_array_layer,
                                 &surf, &offset_B,
                                 &tile_x_sa, &tile_y_sa);
         view.base_array_layer = 0;
         view.base_level = 0;
      } else if (res->base.b.target == PIPE_TEXTURE_CUBE && GFX_VER == 4) {
         isl_surf_get_image_surf(isl_dev, in_surf,
                                 view.base_level, view.base_array_layer,
                                 0,
                                 &surf, &offset_B,
                                 &tile_x_sa, &tile_y_sa);
         view.base_array_layer = 0;
         view.base_level = 0;
      } else if (res->base.b.target == PIPE_TEXTURE_1D_ARRAY)
         surf.dim = ISL_SURF_DIM_2D;
   }

   union isl_color_value clear_color = { .u32 = { 0, 0, 0, 0 } };
   struct crocus_bo *aux_bo = NULL;
   uint32_t aux_offset = 0;
   struct isl_surf *aux_surf = NULL;
   if (aux_usage != ISL_AUX_USAGE_NONE) {
      aux_surf = &res->aux.surf;
      aux_offset = res->aux.offset;
      aux_bo = res->aux.bo;

      clear_color = crocus_resource_get_clear_color(res);
   }

   isl_surf_fill_state(isl_dev, surf_state,
                       .surf = &surf,
                       .view = &view,
                       .address = crocus_state_reloc(batch,
                                                     addr_offset + isl_dev->ss.addr_offset,
                                                     res->bo, offset_B, reloc),
                       .aux_surf = aux_surf,
                       .aux_usage = aux_usage,
                       .aux_address = aux_offset,
                       .mocs = crocus_mocs(res->bo, isl_dev),
                       .clear_color = clear_color,
                       .use_clear_address = false,
                       .clear_address = 0,
                       .x_offset_sa = tile_x_sa,
                       .y_offset_sa = tile_y_sa,
#if GFX_VER <= 5
                       .blend_enable = blend_enable,
                       .write_disables = write_disables,
#endif
      );

   if (aux_surf) {
      /* On gen7 and prior, the upper 20 bits of surface state DWORD 6 are the
       * upper 20 bits of the GPU address of the MCS buffer; the lower 12 bits
       * contain other control information.  Since buffer addresses are always
       * on 4k boundaries (and thus have their lower 12 bits zero), we can use
       * an ordinary reloc to do the necessary address translation.
       *
       * FIXME: move to the point of assignment.
       */
      if (GFX_VER == 8) {
         uint64_t *aux_addr = (uint64_t *)(surf_state + (isl_dev->ss.aux_addr_offset / 4));
         *aux_addr = crocus_state_reloc(batch,
                                        addr_offset + isl_dev->ss.aux_addr_offset,
                                        aux_bo, *aux_addr,
                                        reloc);
      } else {
         uint32_t *aux_addr = surf_state + (isl_dev->ss.aux_addr_offset / 4);
         *aux_addr = crocus_state_reloc(batch,
                                        addr_offset + isl_dev->ss.aux_addr_offset,
                                        aux_bo, *aux_addr,
                                        reloc);
      }
   }

}

static uint32_t
emit_surface(struct crocus_batch *batch,
             struct crocus_surface *surf,
             enum isl_aux_usage aux_usage,
             bool blend_enable,
             uint32_t write_disables)
{
   struct isl_device *isl_dev = &batch->screen->isl_dev;
   struct crocus_resource *res = (struct crocus_resource *)surf->base.texture;
   struct isl_view *view = &surf->view;
   uint32_t offset = 0;
   enum pipe_texture_target target = res->base.b.target;
   bool adjust_surf = false;

   if (GFX_VER == 4 && target == PIPE_TEXTURE_CUBE)
      adjust_surf = true;

   if (surf->align_res)
      res = (struct crocus_resource *)surf->align_res;

   uint32_t *surf_state = stream_state(batch, isl_dev->ss.size, isl_dev->ss.align, &offset);

   emit_surface_state(batch, res, &surf->surf, adjust_surf, view, true,
                      aux_usage, blend_enable,
                      write_disables,
                      surf_state, offset);
   return offset;
}

static uint32_t
emit_rt_surface(struct crocus_batch *batch,
                struct crocus_surface *surf,
                enum isl_aux_usage aux_usage)
{
   struct isl_device *isl_dev = &batch->screen->isl_dev;
   struct crocus_resource *res = (struct crocus_resource *)surf->base.texture;
   struct isl_view *view = &surf->read_view;
   uint32_t offset = 0;
   uint32_t *surf_state = stream_state(batch, isl_dev->ss.size, isl_dev->ss.align, &offset);

   emit_surface_state(batch, res, &surf->surf, true, view, false,
                      aux_usage, 0, false,
                      surf_state, offset);
   return offset;
}

static uint32_t
emit_grid(struct crocus_context *ice,
          struct crocus_batch *batch)
{
   UNUSED struct isl_device *isl_dev = &batch->screen->isl_dev;
   uint32_t offset = 0;
   struct crocus_state_ref *grid_ref = &ice->state.grid_size;
   uint32_t *surf_state = stream_state(batch, isl_dev->ss.size,
                                       isl_dev->ss.align, &offset);
   isl_buffer_fill_state(isl_dev, surf_state,
                         .address = crocus_state_reloc(batch, offset + isl_dev->ss.addr_offset,
                                                       crocus_resource_bo(grid_ref->res),
                                                       grid_ref->offset,
                                                       RELOC_32BIT),
                         .size_B = 12,
                         .format = ISL_FORMAT_RAW,
                         .stride_B = 1,
                         .mocs = crocus_mocs(crocus_resource_bo(grid_ref->res), isl_dev));
   return offset;
}

static uint32_t
emit_ubo_buffer(struct crocus_context *ice,
                struct crocus_batch *batch,
                struct pipe_constant_buffer *buffer)
{
   UNUSED struct isl_device *isl_dev = &batch->screen->isl_dev;
   uint32_t offset = 0;

   uint32_t *surf_state = stream_state(batch, isl_dev->ss.size,
                                       isl_dev->ss.align, &offset);
   isl_buffer_fill_state(isl_dev, surf_state,
                         .address = crocus_state_reloc(batch, offset + isl_dev->ss.addr_offset,
                                                       crocus_resource_bo(buffer->buffer),
                                                       buffer->buffer_offset,
                                                       RELOC_32BIT),
                         .size_B = buffer->buffer_size,
                         .format = 0,
                         .swizzle = ISL_SWIZZLE_IDENTITY,
                         .stride_B = 1,
                         .mocs = crocus_mocs(crocus_resource_bo(buffer->buffer), isl_dev));

   return offset;
}

static uint32_t
emit_ssbo_buffer(struct crocus_context *ice,
                 struct crocus_batch *batch,
                 struct pipe_shader_buffer *buffer, bool writeable)
{
   UNUSED struct isl_device *isl_dev = &batch->screen->isl_dev;
   uint32_t offset = 0;
   uint32_t reloc = RELOC_32BIT;

   if (writeable)
      reloc |= RELOC_WRITE;
   uint32_t *surf_state = stream_state(batch, isl_dev->ss.size,
                                       isl_dev->ss.align, &offset);
   isl_buffer_fill_state(isl_dev, surf_state,
                         .address = crocus_state_reloc(batch, offset + isl_dev->ss.addr_offset,
                                                       crocus_resource_bo(buffer->buffer),
                                                       buffer->buffer_offset,
                                                       reloc),
                         .size_B = buffer->buffer_size,
                         .format = ISL_FORMAT_RAW,
                         .swizzle = ISL_SWIZZLE_IDENTITY,
                         .stride_B = 1,
                         .mocs = crocus_mocs(crocus_resource_bo(buffer->buffer), isl_dev));

   return offset;
}

static uint32_t
emit_sampler_view(struct crocus_context *ice,
                  struct crocus_batch *batch,
                  bool for_gather,
                  struct crocus_sampler_view *isv)
{
   UNUSED struct isl_device *isl_dev = &batch->screen->isl_dev;
   uint32_t offset = 0;

   uint32_t *surf_state = stream_state(batch, isl_dev->ss.size,
                                       isl_dev->ss.align, &offset);

   if (isv->base.target == PIPE_BUFFER) {
      const struct isl_format_layout *fmtl = isl_format_get_layout(isv->view.format);
      const unsigned cpp = isv->view.format == ISL_FORMAT_RAW ? 1 : fmtl->bpb / 8;
      unsigned final_size =
         MIN3(isv->base.u.buf.size, isv->res->bo->size - isv->res->offset,
              CROCUS_MAX_TEXTURE_BUFFER_SIZE * cpp);
      isl_buffer_fill_state(isl_dev, surf_state,
                            .address = crocus_state_reloc(batch, offset + isl_dev->ss.addr_offset,
                                                          isv->res->bo,
                                                          isv->res->offset + isv->base.u.buf.offset, RELOC_32BIT),
                            .size_B = final_size,
                            .format = isv->view.format,
                            .swizzle = isv->view.swizzle,
                            .stride_B = cpp,
                            .mocs = crocus_mocs(isv->res->bo, isl_dev)
         );
   } else {
      enum isl_aux_usage aux_usage =
         crocus_resource_texture_aux_usage(isv->res);

      emit_surface_state(batch, isv->res, &isv->res->surf, false,
                         for_gather ? &isv->gather_view : &isv->view,
                         false, aux_usage, false,
                         0, surf_state, offset);
   }
   return offset;
}

static uint32_t
emit_image_view(struct crocus_context *ice,
                struct crocus_batch *batch,
                struct crocus_image_view *iv)
{
   UNUSED struct isl_device *isl_dev = &batch->screen->isl_dev;
   uint32_t offset = 0;

   struct crocus_resource *res = (struct crocus_resource *)iv->base.resource;
   uint32_t *surf_state = stream_state(batch, isl_dev->ss.size,
                                       isl_dev->ss.align, &offset);
   bool write = iv->base.shader_access & PIPE_IMAGE_ACCESS_WRITE;
   uint32_t reloc = RELOC_32BIT | (write ? RELOC_WRITE : 0);
   if (res->base.b.target == PIPE_BUFFER) {
      const struct isl_format_layout *fmtl = isl_format_get_layout(iv->view.format);
      const unsigned cpp = iv->view.format == ISL_FORMAT_RAW ? 1 : fmtl->bpb / 8;
      unsigned final_size =
         MIN3(iv->base.u.buf.size, res->bo->size - res->offset - iv->base.u.buf.offset,
              CROCUS_MAX_TEXTURE_BUFFER_SIZE * cpp);
      isl_buffer_fill_state(isl_dev, surf_state,
                            .address = crocus_state_reloc(batch, offset + isl_dev->ss.addr_offset,
                                                          res->bo,
                                                          res->offset + iv->base.u.buf.offset, reloc),
                            .size_B = final_size,
                            .format = iv->view.format,
                            .swizzle = iv->view.swizzle,
                            .stride_B = cpp,
                            .mocs = crocus_mocs(res->bo, isl_dev)
         );
   } else {
      if (iv->view.format == ISL_FORMAT_RAW) {
         isl_buffer_fill_state(isl_dev, surf_state,
                               .address = crocus_state_reloc(batch, offset + isl_dev->ss.addr_offset,
                                                             res->bo,
                                                             res->offset, reloc),
                               .size_B = res->bo->size - res->offset,
                               .format = iv->view.format,
                               .swizzle = iv->view.swizzle,
                               .stride_B = 1,
                               .mocs = crocus_mocs(res->bo, isl_dev),
            );


      } else {
         emit_surface_state(batch, res,
                            &res->surf, false, &iv->view,
                            write, 0, false,
                            0, surf_state, offset);
      }
   }

   return offset;
}

#if GFX_VER == 6
static uint32_t
emit_sol_surface(struct crocus_batch *batch,
                 struct pipe_stream_output_info *so_info,
                 uint32_t idx)
{
   struct crocus_context *ice = batch->ice;

   if (idx >= so_info->num_outputs || !ice->state.streamout_active)
      return 0;
   const struct pipe_stream_output *output = &so_info->output[idx];
   const int buffer = output->output_buffer;
   assert(output->stream == 0);

   struct crocus_resource *buf = (struct crocus_resource *)ice->state.so_target[buffer]->buffer;
   unsigned stride_dwords = so_info->stride[buffer];
   unsigned offset_dwords = ice->state.so_target[buffer]->buffer_offset / 4 + output->dst_offset;

   size_t size_dwords = (ice->state.so_target[buffer]->buffer_offset + ice->state.so_target[buffer]->buffer_size) / 4;
   unsigned num_vector_components = output->num_components;
   unsigned num_elements;
   /* FIXME: can we rely on core Mesa to ensure that the buffer isn't
    * too big to map using a single binding table entry?
    */
   //   assert((size_dwords - offset_dwords) / stride_dwords
   //          <= BRW_MAX_NUM_BUFFER_ENTRIES);

   if (size_dwords > offset_dwords + num_vector_components) {
      /* There is room for at least 1 transform feedback output in the buffer.
       * Compute the number of additional transform feedback outputs the
       * buffer has room for.
       */
      num_elements =
         (size_dwords - offset_dwords - num_vector_components);
   } else {
      /* There isn't even room for a single transform feedback output in the
       * buffer.  We can't configure the binding table entry to prevent output
       * entirely; we'll have to rely on the geometry shader to detect
       * overflow.  But to minimize the damage in case of a bug, set up the
       * binding table entry to just allow a single output.
       */
      num_elements = 0;
   }
   num_elements += stride_dwords;

   uint32_t surface_format;
   switch (num_vector_components) {
   case 1:
      surface_format = ISL_FORMAT_R32_FLOAT;
      break;
   case 2:
      surface_format = ISL_FORMAT_R32G32_FLOAT;
      break;
   case 3:
      surface_format = ISL_FORMAT_R32G32B32_FLOAT;
      break;
   case 4:
      surface_format = ISL_FORMAT_R32G32B32A32_FLOAT;
      break;
   default:
      unreachable("Invalid vector size for transform feedback output");
   }

   UNUSED struct isl_device *isl_dev = &batch->screen->isl_dev;
   uint32_t offset = 0;

   uint32_t *surf_state = stream_state(batch, isl_dev->ss.size,
                                       isl_dev->ss.align, &offset);
   isl_buffer_fill_state(isl_dev, surf_state,
                         .address = crocus_state_reloc(batch, offset + isl_dev->ss.addr_offset,
                                                       crocus_resource_bo(&buf->base.b),
                                                       offset_dwords * 4, RELOC_32BIT|RELOC_WRITE),
                         .size_B = num_elements * 4,
                         .stride_B = stride_dwords * 4,
                         .swizzle = ISL_SWIZZLE_IDENTITY,
                         .format = surface_format);
   return offset;
}
#endif

#define foreach_surface_used(index, group)                      \
   for (int index = 0; index < bt->sizes[group]; index++)       \
      if (crocus_group_index_to_bti(bt, group, index) !=        \
          CROCUS_SURFACE_NOT_USED)

static void
crocus_populate_binding_table(struct crocus_context *ice,
                              struct crocus_batch *batch,
                              gl_shader_stage stage, bool ff_gs)
{
   struct crocus_compiled_shader *shader = ff_gs ? ice->shaders.ff_gs_prog : ice->shaders.prog[stage];
   struct crocus_shader_state *shs = ff_gs ? NULL : &ice->state.shaders[stage];
   if (!shader)
      return;

   struct crocus_binding_table *bt = &shader->bt;
   int s = 0;
   uint32_t *surf_offsets = shader->surf_offset;

#if GFX_VER < 8
   const struct shader_info *info = crocus_get_shader_info(ice, stage);
#endif

   if (stage == MESA_SHADER_FRAGMENT) {
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      /* Note that cso_fb->nr_cbufs == fs_key->nr_color_regions. */
      if (cso_fb->nr_cbufs) {
         for (unsigned i = 0; i < cso_fb->nr_cbufs; i++) {
            uint32_t write_disables = 0;
            bool blend_enable = false;
#if GFX_VER <= 5
            const struct pipe_rt_blend_state *rt =
               &ice->state.cso_blend->cso.rt[ice->state.cso_blend->cso.independent_blend_enable ? i : 0];
            struct crocus_compiled_shader *shader = ice->shaders.prog[MESA_SHADER_FRAGMENT];
            struct brw_wm_prog_data *wm_prog_data = (void *) shader->prog_data;
            write_disables |= (rt->colormask & PIPE_MASK_A) ? 0x0 : 0x8;
            write_disables |= (rt->colormask & PIPE_MASK_R) ? 0x0 : 0x4;
            write_disables |= (rt->colormask & PIPE_MASK_G) ? 0x0 : 0x2;
            write_disables |= (rt->colormask & PIPE_MASK_B) ? 0x0 : 0x1;
            /* Gen4/5 can't handle blending off when a dual src blend wm is enabled. */
            blend_enable = rt->blend_enable || wm_prog_data->dual_src_blend;
#endif
            if (cso_fb->cbufs[i]) {
               surf_offsets[s] = emit_surface(batch,
                                              (struct crocus_surface *)cso_fb->cbufs[i],
                                              ice->state.draw_aux_usage[i],
                                              blend_enable,
                                              write_disables);
            } else {
               emit_null_fb_surface(batch, ice, &surf_offsets[s]);
            }
            s++;
         }
      } else {
         emit_null_fb_surface(batch, ice, &surf_offsets[s]);
         s++;
      }

      foreach_surface_used(i, CROCUS_SURFACE_GROUP_RENDER_TARGET_READ) {
         struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
         if (cso_fb->cbufs[i]) {
            surf_offsets[s++] = emit_rt_surface(batch,
                                                (struct crocus_surface *)cso_fb->cbufs[i],
                                                ice->state.draw_aux_usage[i]);
         }
      }
   }

   if (stage == MESA_SHADER_COMPUTE) {
      foreach_surface_used(i, CROCUS_SURFACE_GROUP_CS_WORK_GROUPS) {
         surf_offsets[s] = emit_grid(ice, batch);
         s++;
      }
   }

#if GFX_VER == 6
   if (stage == MESA_SHADER_GEOMETRY) {
      struct pipe_stream_output_info *so_info;
      if (ice->shaders.uncompiled[MESA_SHADER_GEOMETRY])
         so_info = &ice->shaders.uncompiled[MESA_SHADER_GEOMETRY]->stream_output;
      else
         so_info = &ice->shaders.uncompiled[MESA_SHADER_VERTEX]->stream_output;

      foreach_surface_used(i, CROCUS_SURFACE_GROUP_SOL) {
         surf_offsets[s] = emit_sol_surface(batch, so_info, i);
         s++;
      }
   }
#endif

   foreach_surface_used(i, CROCUS_SURFACE_GROUP_TEXTURE) {
      struct crocus_sampler_view *view = shs->textures[i];
      if (view)
         surf_offsets[s] = emit_sampler_view(ice, batch, false, view);
      else
         emit_null_surface(batch, &surf_offsets[s]);
      s++;
   }

#if GFX_VER < 8
   if (info && info->uses_texture_gather) {
      foreach_surface_used(i, CROCUS_SURFACE_GROUP_TEXTURE_GATHER) {
         struct crocus_sampler_view *view = shs->textures[i];
         if (view)
            surf_offsets[s] = emit_sampler_view(ice, batch, true, view);
         else
            emit_null_surface(batch, &surf_offsets[s]);
         s++;
      }
   }
#endif

   foreach_surface_used(i, CROCUS_SURFACE_GROUP_IMAGE) {
      struct crocus_image_view *view = &shs->image[i];
      if (view->base.resource)
         surf_offsets[s] = emit_image_view(ice, batch, view);
      else
         emit_null_surface(batch, &surf_offsets[s]);
      s++;
   }
   foreach_surface_used(i, CROCUS_SURFACE_GROUP_UBO) {
      if (shs->constbufs[i].buffer)
         surf_offsets[s] = emit_ubo_buffer(ice, batch, &shs->constbufs[i]);
      else
         emit_null_surface(batch, &surf_offsets[s]);
      s++;
   }
   foreach_surface_used(i, CROCUS_SURFACE_GROUP_SSBO) {
      if (shs->ssbo[i].buffer)
         surf_offsets[s] = emit_ssbo_buffer(ice, batch, &shs->ssbo[i],
                                            !!(shs->writable_ssbos & (1 << i)));
      else
         emit_null_surface(batch, &surf_offsets[s]);
      s++;
   }

}
/* ------------------------------------------------------------------- */
static uint32_t
crocus_upload_binding_table(struct crocus_context *ice,
                            struct crocus_batch *batch,
                            uint32_t *table,
                            uint32_t size)

{
   if (size == 0)
      return 0;
   return emit_state(batch, table, size, 32);
}

/**
 * Possibly emit STATE_BASE_ADDRESS to update Surface State Base Address.
 */

static void
crocus_update_surface_base_address(struct crocus_batch *batch)
{
   if (batch->state_base_address_emitted)
      return;

   UNUSED uint32_t mocs = batch->screen->isl_dev.mocs.internal;

   flush_before_state_base_change(batch);

   crocus_emit_cmd(batch, GENX(STATE_BASE_ADDRESS), sba) {
      /* Set base addresses */
      sba.GeneralStateBaseAddressModifyEnable = true;

#if GFX_VER >= 6
      sba.DynamicStateBaseAddressModifyEnable = true;
      sba.DynamicStateBaseAddress = ro_bo(batch->state.bo, 0);
#endif

      sba.SurfaceStateBaseAddressModifyEnable = true;
      sba.SurfaceStateBaseAddress = ro_bo(batch->state.bo, 0);

      sba.IndirectObjectBaseAddressModifyEnable = true;

#if GFX_VER >= 5
      sba.InstructionBaseAddressModifyEnable = true;
      sba.InstructionBaseAddress = ro_bo(batch->ice->shaders.cache_bo, 0); // TODO!
#endif

      /* Set buffer sizes on Gen8+ or upper bounds on Gen4-7 */
#if GFX_VER == 8
      sba.GeneralStateBufferSize   = 0xfffff;
      sba.IndirectObjectBufferSize = 0xfffff;
      sba.InstructionBufferSize    = 0xfffff;
      sba.DynamicStateBufferSize   = MAX_STATE_SIZE;

      sba.GeneralStateBufferSizeModifyEnable    = true;
      sba.DynamicStateBufferSizeModifyEnable    = true;
      sba.IndirectObjectBufferSizeModifyEnable  = true;
      sba.InstructionBuffersizeModifyEnable     = true;
#else
      sba.GeneralStateAccessUpperBoundModifyEnable = true;
      sba.IndirectObjectAccessUpperBoundModifyEnable = true;

#if GFX_VER >= 5
      sba.InstructionAccessUpperBoundModifyEnable = true;
#endif

#if GFX_VER >= 6
      /* Dynamic state upper bound.  Although the documentation says that
       * programming it to zero will cause it to be ignored, that is a lie.
       * If this isn't programmed to a real bound, the sampler border color
       * pointer is rejected, causing border color to mysteriously fail.
       */
      sba.DynamicStateAccessUpperBound = ro_bo(NULL, 0xfffff000);
      sba.DynamicStateAccessUpperBoundModifyEnable = true;
#else
      /* Same idea but using General State Base Address on Gen4-5 */
      sba.GeneralStateAccessUpperBound = ro_bo(NULL, 0xfffff000);
#endif
#endif

#if GFX_VER >= 6
      /* The hardware appears to pay attention to the MOCS fields even
       * if you don't set the "Address Modify Enable" bit for the base.
       */
      sba.GeneralStateMOCS            = mocs;
      sba.StatelessDataPortAccessMOCS = mocs;
      sba.DynamicStateMOCS            = mocs;
      sba.IndirectObjectMOCS          = mocs;
      sba.InstructionMOCS             = mocs;
      sba.SurfaceStateMOCS            = mocs;
#endif
   }

   flush_after_state_base_change(batch);

   /* According to section 3.6.1 of VOL1 of the 965 PRM,
    * STATE_BASE_ADDRESS updates require a reissue of:
    *
    * 3DSTATE_PIPELINE_POINTERS
    * 3DSTATE_BINDING_TABLE_POINTERS
    * MEDIA_STATE_POINTERS
    *
    * and this continues through Ironlake.  The Sandy Bridge PRM, vol
    * 1 part 1 says that the folowing packets must be reissued:
    *
    * 3DSTATE_CC_POINTERS
    * 3DSTATE_BINDING_TABLE_POINTERS
    * 3DSTATE_SAMPLER_STATE_POINTERS
    * 3DSTATE_VIEWPORT_STATE_POINTERS
    * MEDIA_STATE_POINTERS
    *
    * Those are always reissued following SBA updates anyway (new
    * batch time), except in the case of the program cache BO
    * changing.  Having a separate state flag makes the sequence more
    * obvious.
    */
#if GFX_VER <= 5
   batch->ice->state.dirty |= CROCUS_DIRTY_GEN5_PIPELINED_POINTERS | CROCUS_DIRTY_GEN5_BINDING_TABLE_POINTERS;
#elif GFX_VER == 6
   batch->ice->state.dirty |= CROCUS_DIRTY_GEN5_BINDING_TABLE_POINTERS | CROCUS_DIRTY_GEN6_SAMPLER_STATE_POINTERS;
#endif
   batch->state_base_address_emitted = true;
}

static inline void
crocus_viewport_zmin_zmax(const struct pipe_viewport_state *vp, bool halfz,
                          bool window_space_position, float *zmin, float *zmax)
{
   if (window_space_position) {
      *zmin = 0.f;
      *zmax = 1.f;
      return;
   }
   util_viewport_zmin_zmax(vp, halfz, zmin, zmax);
}

struct push_bos {
   struct {
      struct crocus_address addr;
      uint32_t length;
   } buffers[4];
   int buffer_count;
   uint32_t max_length;
};

#if GFX_VER >= 6
static void
setup_constant_buffers(struct crocus_context *ice,
                       struct crocus_batch *batch,
                       int stage,
                       struct push_bos *push_bos)
{
   struct crocus_shader_state *shs = &ice->state.shaders[stage];
   struct crocus_compiled_shader *shader = ice->shaders.prog[stage];
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
      unsigned block_index = crocus_bti_to_group_index(
         &shader->bt, CROCUS_SURFACE_GROUP_UBO, range->block);
      assert(block_index != CROCUS_SURFACE_NOT_USED);

      struct pipe_constant_buffer *cbuf = &shs->constbufs[block_index];
      struct crocus_resource *res = (void *) cbuf->buffer;

      assert(cbuf->buffer_offset % 32 == 0);

      push_bos->buffers[n].length = range->length;
      push_bos->buffers[n].addr =
         res ? ro_bo(res->bo, range->start * 32 + cbuf->buffer_offset)
         : ro_bo(batch->ice->workaround_bo,
                 batch->ice->workaround_offset);
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

#if GFX_VER == 7
static void
gen7_emit_vs_workaround_flush(struct crocus_batch *batch)
{
   crocus_emit_pipe_control_write(batch,
                                  "vs workaround",
                                  PIPE_CONTROL_WRITE_IMMEDIATE
                                  | PIPE_CONTROL_DEPTH_STALL,
                                  batch->ice->workaround_bo,
                                  batch->ice->workaround_offset, 0);
}
#endif

static void
emit_push_constant_packets(struct crocus_context *ice,
                           struct crocus_batch *batch,
                           int stage,
                           const struct push_bos *push_bos)
{
   struct crocus_compiled_shader *shader = ice->shaders.prog[stage];
   struct brw_stage_prog_data *prog_data = shader ? (void *) shader->prog_data : NULL;
   UNUSED uint32_t mocs = crocus_mocs(NULL, &batch->screen->isl_dev);

#if GFX_VER == 7
   if (stage == MESA_SHADER_VERTEX) {
      if (batch->screen->devinfo.platform == INTEL_PLATFORM_IVB)
         gen7_emit_vs_workaround_flush(batch);
   }
#endif
   crocus_emit_cmd(batch, GENX(3DSTATE_CONSTANT_VS), pkt) {
      pkt._3DCommandSubOpcode = push_constant_opcodes[stage];
#if GFX_VER >= 7
#if GFX_VER != 8
      /* MOCS is MBZ on Gen8 so we skip it there */
      pkt.ConstantBody.MOCS = mocs;
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
#if GFX_VERx10 >= 75
         const unsigned shift = 4 - n;
#else
         const unsigned shift = 0;
#endif
         for (int i = 0; i < n; i++) {
            pkt.ConstantBody.ReadLength[i + shift] =
               push_bos->buffers[i].length;
            pkt.ConstantBody.Buffer[i + shift] = push_bos->buffers[i].addr;
         }
      }
#else
      if (prog_data) {
         int n = push_bos->buffer_count;
         assert (n <= 1);
         if (n == 1) {
            pkt.Buffer0Valid = true;
            pkt.ConstantBody.PointertoConstantBuffer0 = push_bos->buffers[0].addr.offset;
            pkt.ConstantBody.ConstantBuffer0ReadLength = push_bos->buffers[0].length - 1;
         }
      }
#endif
   }
}

#endif

#if GFX_VER == 8
typedef struct GENX(3DSTATE_WM_DEPTH_STENCIL) DEPTH_STENCIL_GENXML;
#elif GFX_VER >= 6
typedef struct GENX(DEPTH_STENCIL_STATE)      DEPTH_STENCIL_GENXML;
#else
typedef struct GENX(COLOR_CALC_STATE)         DEPTH_STENCIL_GENXML;
#endif

static inline void
set_depth_stencil_bits(struct crocus_context *ice, DEPTH_STENCIL_GENXML *ds)
{
   struct crocus_depth_stencil_alpha_state *cso = ice->state.cso_zsa;
   ds->DepthTestEnable = cso->cso.depth_enabled;
   ds->DepthBufferWriteEnable = cso->cso.depth_writemask;
   ds->DepthTestFunction = translate_compare_func(cso->cso.depth_func);

   ds->StencilFailOp = cso->cso.stencil[0].fail_op;
   ds->StencilPassDepthFailOp = cso->cso.stencil[0].zfail_op;
   ds->StencilPassDepthPassOp = cso->cso.stencil[0].zpass_op;
   ds->StencilTestFunction = translate_compare_func(cso->cso.stencil[0].func);

   ds->StencilTestMask = cso->cso.stencil[0].valuemask;
   ds->StencilWriteMask = cso->cso.stencil[0].writemask;

   ds->BackfaceStencilFailOp = cso->cso.stencil[1].fail_op;
   ds->BackfaceStencilPassDepthFailOp = cso->cso.stencil[1].zfail_op;
   ds->BackfaceStencilPassDepthPassOp = cso->cso.stencil[1].zpass_op;
   ds->BackfaceStencilTestFunction = translate_compare_func(cso->cso.stencil[1].func);

   ds->BackfaceStencilTestMask = cso->cso.stencil[1].valuemask;
   ds->BackfaceStencilWriteMask = cso->cso.stencil[1].writemask;
   ds->DoubleSidedStencilEnable = cso->cso.stencil[1].enabled;
   ds->StencilTestEnable = cso->cso.stencil[0].enabled;
   ds->StencilBufferWriteEnable =
      cso->cso.stencil[0].writemask != 0 ||
      (cso->cso.stencil[1].enabled && cso->cso.stencil[1].writemask != 0);
}

static void
emit_vertex_buffer_state(struct crocus_batch *batch,
                         unsigned buffer_id,
                         struct crocus_bo *bo,
                         unsigned start_offset,
                         unsigned end_offset,
                         unsigned stride,
                         unsigned step_rate,
                         uint32_t **map)
{
   const unsigned vb_dwords = GENX(VERTEX_BUFFER_STATE_length);
   _crocus_pack_state(batch, GENX(VERTEX_BUFFER_STATE), *map, vb) {
      vb.BufferStartingAddress = ro_bo(bo, start_offset);
#if GFX_VER >= 8
      vb.BufferSize = end_offset - start_offset;
#endif
      vb.VertexBufferIndex = buffer_id;
      vb.BufferPitch = stride;
#if GFX_VER >= 7
      vb.AddressModifyEnable = true;
#endif
#if GFX_VER >= 6
      vb.MOCS = crocus_mocs(bo, &batch->screen->isl_dev);
#endif
#if GFX_VER < 8
      vb.BufferAccessType = step_rate ? INSTANCEDATA : VERTEXDATA;
      vb.InstanceDataStepRate = step_rate;
#if GFX_VER >= 5
      vb.EndAddress = ro_bo(bo, end_offset - 1);
#endif
#endif
   }
   *map += vb_dwords;
}

#if GFX_VER >= 6
static uint32_t
determine_sample_mask(struct crocus_context *ice)
{
   uint32_t num_samples = ice->state.framebuffer.samples;

   if (num_samples <= 1)
      return 1;

   uint32_t fb_mask = (1 << num_samples) - 1;
   return ice->state.sample_mask & fb_mask;
}
#endif

static void
crocus_upload_dirty_render_state(struct crocus_context *ice,
                               struct crocus_batch *batch,
                               const struct pipe_draw_info *draw)
{
   uint64_t dirty = ice->state.dirty;
   uint64_t stage_dirty = ice->state.stage_dirty;

   if (!(dirty & CROCUS_ALL_DIRTY_FOR_RENDER) &&
       !(stage_dirty & CROCUS_ALL_STAGE_DIRTY_FOR_RENDER))
      return;

   if (dirty & CROCUS_DIRTY_VF_STATISTICS) {
      crocus_emit_cmd(batch, GENX(3DSTATE_VF_STATISTICS), vf) {
         vf.StatisticsEnable = true;
      }
   }

#if GFX_VER <= 5
   if (stage_dirty & (CROCUS_STAGE_DIRTY_CONSTANTS_VS |
                      CROCUS_STAGE_DIRTY_CONSTANTS_FS)) {
      bool ret = calculate_curbe_offsets(batch);
      if (ret) {
         dirty |= CROCUS_DIRTY_GEN4_CURBE | CROCUS_DIRTY_WM | CROCUS_DIRTY_CLIP;
         stage_dirty |= CROCUS_STAGE_DIRTY_VS;
      }
   }

   if (dirty & (CROCUS_DIRTY_GEN4_CURBE | CROCUS_DIRTY_RASTER) ||
       stage_dirty & CROCUS_STAGE_DIRTY_VS) {
     bool ret = crocus_calculate_urb_fence(batch, ice->curbe.total_size,
                                           brw_vue_prog_data(ice->shaders.prog[MESA_SHADER_VERTEX]->prog_data)->urb_entry_size,
                                           ((struct brw_sf_prog_data *)ice->shaders.sf_prog->prog_data)->urb_entry_size);
     if (ret) {
	dirty |= CROCUS_DIRTY_GEN5_PIPELINED_POINTERS | CROCUS_DIRTY_RASTER | CROCUS_DIRTY_CLIP;
	stage_dirty |= CROCUS_STAGE_DIRTY_GS | CROCUS_STAGE_DIRTY_VS;
     }
   }
#endif
   if (dirty & CROCUS_DIRTY_CC_VIEWPORT) {
      const struct crocus_rasterizer_state *cso_rast = ice->state.cso_rast;
      uint32_t cc_vp_address;

      /* XXX: could avoid streaming for depth_clip [0,1] case. */
      uint32_t *cc_vp_map =
         stream_state(batch,
                      4 * ice->state.num_viewports *
                      GENX(CC_VIEWPORT_length), 32, &cc_vp_address);
      for (int i = 0; i < ice->state.num_viewports; i++) {
         float zmin, zmax;
         crocus_viewport_zmin_zmax(&ice->state.viewports[i], cso_rast->cso.clip_halfz,
                                 ice->state.window_space_position,
                                 &zmin, &zmax);
         if (cso_rast->cso.depth_clip_near)
            zmin = 0.0;
         if (cso_rast->cso.depth_clip_far)
            zmax = 1.0;

         crocus_pack_state(GENX(CC_VIEWPORT), cc_vp_map, ccv) {
            ccv.MinimumDepth = zmin;
            ccv.MaximumDepth = zmax;
         }

         cc_vp_map += GENX(CC_VIEWPORT_length);
      }

#if GFX_VER >= 7
      crocus_emit_cmd(batch, GENX(3DSTATE_VIEWPORT_STATE_POINTERS_CC), ptr) {
         ptr.CCViewportPointer = cc_vp_address;
      }
#elif GFX_VER == 6
      crocus_emit_cmd(batch, GENX(3DSTATE_VIEWPORT_STATE_POINTERS), vp) {
         vp.CCViewportStateChange = 1;
         vp.PointertoCC_VIEWPORT = cc_vp_address;
      }
#else
      ice->state.cc_vp_address = cc_vp_address;
      dirty |= CROCUS_DIRTY_COLOR_CALC_STATE;
#endif
   }

   if (dirty & CROCUS_DIRTY_SF_CL_VIEWPORT) {
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
#if GFX_VER >= 7
      uint32_t sf_cl_vp_address;
      uint32_t *vp_map =
         stream_state(batch,
                      4 * ice->state.num_viewports *
                      GENX(SF_CLIP_VIEWPORT_length), 64, &sf_cl_vp_address);
#else
      uint32_t *vp_map =
         stream_state(batch,
                      4 * ice->state.num_viewports * GENX(SF_VIEWPORT_length),
                      32, &ice->state.sf_vp_address);
      uint32_t *clip_map =
         stream_state(batch,
                      4 * ice->state.num_viewports * GENX(CLIP_VIEWPORT_length),
                      32, &ice->state.clip_vp_address);
#endif

      for (unsigned i = 0; i < ice->state.num_viewports; i++) {
         const struct pipe_viewport_state *state = &ice->state.viewports[i];
         float gb_xmin, gb_xmax, gb_ymin, gb_ymax;

#if GFX_VER == 8
         float vp_xmin = viewport_extent(state, 0, -1.0f);
         float vp_xmax = viewport_extent(state, 0,  1.0f);
         float vp_ymin = viewport_extent(state, 1, -1.0f);
         float vp_ymax = viewport_extent(state, 1,  1.0f);
#endif
         intel_calculate_guardband_size(0, cso_fb->width, 0, cso_fb->height,
                                        state->scale[0], state->scale[1],
                                        state->translate[0], state->translate[1],
                                        &gb_xmin, &gb_xmax, &gb_ymin, &gb_ymax);
#if GFX_VER >= 7
         crocus_pack_state(GENX(SF_CLIP_VIEWPORT), vp_map, vp)
#else
         crocus_pack_state(GENX(SF_VIEWPORT), vp_map, vp)
#endif
         {
            vp.ViewportMatrixElementm00 = state->scale[0];
            vp.ViewportMatrixElementm11 = state->scale[1];
            vp.ViewportMatrixElementm22 = state->scale[2];
            vp.ViewportMatrixElementm30 = state->translate[0];
            vp.ViewportMatrixElementm31 = state->translate[1];
            vp.ViewportMatrixElementm32 = state->translate[2];
#if GFX_VER < 6
            struct pipe_scissor_state scissor;
            crocus_fill_scissor_rect(ice, 0, &scissor);
            vp.ScissorRectangle.ScissorRectangleXMin = scissor.minx;
            vp.ScissorRectangle.ScissorRectangleXMax = scissor.maxx;
            vp.ScissorRectangle.ScissorRectangleYMin = scissor.miny;
            vp.ScissorRectangle.ScissorRectangleYMax = scissor.maxy;
#endif

#if GFX_VER >= 7
            vp.XMinClipGuardband = gb_xmin;
            vp.XMaxClipGuardband = gb_xmax;
            vp.YMinClipGuardband = gb_ymin;
            vp.YMaxClipGuardband = gb_ymax;
#endif
#if GFX_VER == 8
            vp.XMinViewPort = MAX2(vp_xmin, 0);
            vp.XMaxViewPort = MIN2(vp_xmax, cso_fb->width) - 1;
            vp.YMinViewPort = MAX2(vp_ymin, 0);
            vp.YMaxViewPort = MIN2(vp_ymax, cso_fb->height) - 1;
#endif
         }
#if GFX_VER < 7
         crocus_pack_state(GENX(CLIP_VIEWPORT), clip_map, clip) {
            clip.XMinClipGuardband = gb_xmin;
            clip.XMaxClipGuardband = gb_xmax;
            clip.YMinClipGuardband = gb_ymin;
            clip.YMaxClipGuardband = gb_ymax;
         }
#endif
#if GFX_VER >= 7
         vp_map += GENX(SF_CLIP_VIEWPORT_length);
#else
         vp_map += GENX(SF_VIEWPORT_length);
         clip_map += GENX(CLIP_VIEWPORT_length);
#endif
      }
#if GFX_VER >= 7
      crocus_emit_cmd(batch, GENX(3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP), ptr) {
         ptr.SFClipViewportPointer = sf_cl_vp_address;
      }
#elif GFX_VER == 6
      crocus_emit_cmd(batch, GENX(3DSTATE_VIEWPORT_STATE_POINTERS), vp) {
         vp.SFViewportStateChange = 1;
         vp.CLIPViewportStateChange = 1;
         vp.PointertoCLIP_VIEWPORT = ice->state.clip_vp_address;
         vp.PointertoSF_VIEWPORT = ice->state.sf_vp_address;
      }
#endif
   }

#if GFX_VER >= 6
   if (dirty & CROCUS_DIRTY_GEN6_URB) {
#if GFX_VER == 6
      bool gs_present = ice->shaders.prog[MESA_SHADER_GEOMETRY] != NULL
         || ice->shaders.ff_gs_prog;

      struct brw_vue_prog_data *vue_prog_data =
         (void *) ice->shaders.prog[MESA_SHADER_VERTEX]->prog_data;
      const unsigned vs_size = vue_prog_data->urb_entry_size;
      unsigned gs_size = vs_size;
      if (ice->shaders.prog[MESA_SHADER_GEOMETRY]) {
         struct brw_vue_prog_data *gs_vue_prog_data =
            (void *) ice->shaders.prog[MESA_SHADER_GEOMETRY]->prog_data;
         gs_size = gs_vue_prog_data->urb_entry_size;
      }

      genX(crocus_upload_urb)(batch, vs_size, gs_present, gs_size);
#endif
#if GFX_VER >= 7
      const struct intel_device_info *devinfo = &batch->screen->devinfo;
      bool gs_present = ice->shaders.prog[MESA_SHADER_GEOMETRY] != NULL;
      bool tess_present = ice->shaders.prog[MESA_SHADER_TESS_EVAL] != NULL;
      unsigned entry_size[4];

      for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
         if (!ice->shaders.prog[i]) {
            entry_size[i] = 1;
         } else {
            struct brw_vue_prog_data *vue_prog_data =
               (void *) ice->shaders.prog[i]->prog_data;
            entry_size[i] = vue_prog_data->urb_entry_size;
         }
         assert(entry_size[i] != 0);
      }

      /* If we're just switching between programs with the same URB requirements,
       * skip the rest of the logic.
       */
      bool no_change = false;
      if (ice->urb.vsize == entry_size[MESA_SHADER_VERTEX] &&
          ice->urb.gs_present == gs_present &&
          ice->urb.gsize == entry_size[MESA_SHADER_GEOMETRY] &&
          ice->urb.tess_present == tess_present &&
          ice->urb.hsize == entry_size[MESA_SHADER_TESS_CTRL] &&
          ice->urb.dsize == entry_size[MESA_SHADER_TESS_EVAL]) {
         no_change = true;
      }

      if (!no_change) {
         ice->urb.vsize = entry_size[MESA_SHADER_VERTEX];
         ice->urb.gs_present = gs_present;
         ice->urb.gsize = entry_size[MESA_SHADER_GEOMETRY];
         ice->urb.tess_present = tess_present;
         ice->urb.hsize = entry_size[MESA_SHADER_TESS_CTRL];
         ice->urb.dsize = entry_size[MESA_SHADER_TESS_EVAL];

         unsigned entries[4];
         unsigned start[4];
         bool constrained;
         intel_get_urb_config(devinfo,
                              batch->screen->l3_config_3d,
                              tess_present,
                              gs_present,
                              entry_size,
                              entries, start, NULL, &constrained);

#if GFX_VER == 7
         if (devinfo->platform == INTEL_PLATFORM_IVB)
            gen7_emit_vs_workaround_flush(batch);
#endif
         for (int i = MESA_SHADER_VERTEX; i <= MESA_SHADER_GEOMETRY; i++) {
            crocus_emit_cmd(batch, GENX(3DSTATE_URB_VS), urb) {
               urb._3DCommandSubOpcode += i;
               urb.VSURBStartingAddress     = start[i];
               urb.VSURBEntryAllocationSize = entry_size[i] - 1;
               urb.VSNumberofURBEntries     = entries[i];
            }
         }
      }
#endif
   }

   if (dirty & CROCUS_DIRTY_GEN6_BLEND_STATE) {
      struct crocus_blend_state *cso_blend = ice->state.cso_blend;
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      struct crocus_depth_stencil_alpha_state *cso_zsa = ice->state.cso_zsa;

      STATIC_ASSERT(GENX(BLEND_STATE_ENTRY_length) == 2);
      int rt_dwords =
         MAX2(cso_fb->nr_cbufs, 1) * GENX(BLEND_STATE_ENTRY_length);
#if GFX_VER >= 8
      rt_dwords += GENX(BLEND_STATE_length);
#endif
      uint32_t blend_offset;
      uint32_t *blend_map =
         stream_state(batch,
                      4 * rt_dwords, 64, &blend_offset);

#if GFX_VER >= 8
   struct GENX(BLEND_STATE) be = { 0 };
   {
#else
   for (int i = 0; i < BRW_MAX_DRAW_BUFFERS; i++) {
      struct GENX(BLEND_STATE_ENTRY) entry = { 0 };
#define be entry
#endif

      be.AlphaTestEnable = cso_zsa->cso.alpha_enabled;
      be.AlphaTestFunction = translate_compare_func(cso_zsa->cso.alpha_func);
      be.AlphaToCoverageEnable = cso_blend->cso.alpha_to_coverage;
      be.AlphaToOneEnable = cso_blend->cso.alpha_to_one;
      be.AlphaToCoverageDitherEnable = GFX_VER >= 7 && cso_blend->cso.alpha_to_coverage_dither;
      be.ColorDitherEnable = cso_blend->cso.dither;

#if GFX_VER >= 8
      for (int i = 0; i < BRW_MAX_DRAW_BUFFERS; i++) {
         struct GENX(BLEND_STATE_ENTRY) entry = { 0 };
#else
      {
#endif
         const struct pipe_rt_blend_state *rt =
            &cso_blend->cso.rt[cso_blend->cso.independent_blend_enable ? i : 0];

         be.IndependentAlphaBlendEnable = set_blend_entry_bits(batch, &entry, cso_blend, i) ||
            be.IndependentAlphaBlendEnable;

         if (GFX_VER >= 8 || can_emit_logic_op(ice)) {
            entry.LogicOpEnable = cso_blend->cso.logicop_enable;
            entry.LogicOpFunction = cso_blend->cso.logicop_func;
         }

         entry.ColorClampRange = COLORCLAMP_RTFORMAT;
         entry.PreBlendColorClampEnable = true;
         entry.PostBlendColorClampEnable = true;

         entry.WriteDisableRed   = !(rt->colormask & PIPE_MASK_R);
         entry.WriteDisableGreen = !(rt->colormask & PIPE_MASK_G);
         entry.WriteDisableBlue  = !(rt->colormask & PIPE_MASK_B);
         entry.WriteDisableAlpha = !(rt->colormask & PIPE_MASK_A);

#if GFX_VER >= 8
         GENX(BLEND_STATE_ENTRY_pack)(NULL, &blend_map[1 + i * 2], &entry);
#else
         GENX(BLEND_STATE_ENTRY_pack)(NULL, &blend_map[i * 2], &entry);
#endif
      }
   }
#if GFX_VER >= 8
   GENX(BLEND_STATE_pack)(NULL, blend_map, &be);
#endif
#if GFX_VER < 7
      crocus_emit_cmd(batch, GENX(3DSTATE_CC_STATE_POINTERS), ptr) {
         ptr.PointertoBLEND_STATE = blend_offset;
         ptr.BLEND_STATEChange = true;
      }
#else
      crocus_emit_cmd(batch, GENX(3DSTATE_BLEND_STATE_POINTERS), ptr) {
         ptr.BlendStatePointer = blend_offset;
#if GFX_VER >= 8
         ptr.BlendStatePointerValid = true;
#endif
      }
#endif
   }
#endif

   if (dirty & CROCUS_DIRTY_COLOR_CALC_STATE) {
      struct crocus_depth_stencil_alpha_state *cso = ice->state.cso_zsa;
      UNUSED struct crocus_blend_state *cso_blend = ice->state.cso_blend;
      struct pipe_stencil_ref *p_stencil_refs = &ice->state.stencil_ref;
      uint32_t cc_offset;
      void *cc_map =
         stream_state(batch,
                      sizeof(uint32_t) * GENX(COLOR_CALC_STATE_length),
                      64, &cc_offset);
#if GFX_VER <= 5
      dirty |= CROCUS_DIRTY_GEN5_PIPELINED_POINTERS;
#endif
      _crocus_pack_state(batch, GENX(COLOR_CALC_STATE), cc_map, cc) {
         cc.AlphaTestFormat = ALPHATEST_FLOAT32;
         cc.AlphaReferenceValueAsFLOAT32 = cso->cso.alpha_ref_value;

#if GFX_VER <= 5

         set_depth_stencil_bits(ice, &cc);

         if (cso_blend->cso.logicop_enable) {
            if (can_emit_logic_op(ice)) {
               cc.LogicOpEnable = cso_blend->cso.logicop_enable;
               cc.LogicOpFunction = cso_blend->cso.logicop_func;
            }
         }
         cc.ColorDitherEnable = cso_blend->cso.dither;

         cc.IndependentAlphaBlendEnable = set_blend_entry_bits(batch, &cc, cso_blend, 0);

         if (cso->cso.alpha_enabled && ice->state.framebuffer.nr_cbufs <= 1) {
            cc.AlphaTestEnable = cso->cso.alpha_enabled;
            cc.AlphaTestFunction = translate_compare_func(cso->cso.alpha_func);
         }
         cc.StatisticsEnable = ice->state.stats_wm ? 1 : 0;
         cc.CCViewportStatePointer = ro_bo(batch->state.bo, ice->state.cc_vp_address);
#else
         cc.AlphaTestFormat = ALPHATEST_FLOAT32;
         cc.AlphaReferenceValueAsFLOAT32 = cso->cso.alpha_ref_value;

         cc.BlendConstantColorRed   = ice->state.blend_color.color[0];
         cc.BlendConstantColorGreen = ice->state.blend_color.color[1];
         cc.BlendConstantColorBlue  = ice->state.blend_color.color[2];
         cc.BlendConstantColorAlpha = ice->state.blend_color.color[3];
#endif
         cc.StencilReferenceValue = p_stencil_refs->ref_value[0];
         cc.BackfaceStencilReferenceValue = p_stencil_refs->ref_value[1];
      }
      ice->shaders.cc_offset = cc_offset;
#if GFX_VER >= 6
      crocus_emit_cmd(batch, GENX(3DSTATE_CC_STATE_POINTERS), ptr) {
         ptr.ColorCalcStatePointer = cc_offset;
#if GFX_VER != 7
         ptr.ColorCalcStatePointerValid = true;
#endif
      }
#endif
   }
#if GFX_VER <= 5
   if (dirty & CROCUS_DIRTY_GEN4_CONSTANT_COLOR) {
      crocus_emit_cmd(batch, GENX(3DSTATE_CONSTANT_COLOR), blend_cc) {
         blend_cc.BlendConstantColorRed = ice->state.blend_color.color[0];
         blend_cc.BlendConstantColorGreen = ice->state.blend_color.color[1];
         blend_cc.BlendConstantColorBlue = ice->state.blend_color.color[2];
         blend_cc.BlendConstantColorAlpha = ice->state.blend_color.color[3];
      }
   }
#endif
   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (!(stage_dirty & (CROCUS_STAGE_DIRTY_CONSTANTS_VS << stage)))
         continue;

      struct crocus_shader_state *shs = &ice->state.shaders[stage];
      struct crocus_compiled_shader *shader = ice->shaders.prog[stage];

      if (!shader)
         continue;

      if (shs->sysvals_need_upload)
         upload_sysvals(ice, stage);

#if GFX_VER <= 5
      dirty |= CROCUS_DIRTY_GEN4_CURBE;
#endif
#if GFX_VER >= 7
      struct push_bos push_bos = {};
      setup_constant_buffers(ice, batch, stage, &push_bos);

      emit_push_constant_packets(ice, batch, stage, &push_bos);
#endif
   }

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (stage_dirty & (CROCUS_STAGE_DIRTY_BINDINGS_VS << stage)) {
         if (ice->shaders.prog[stage]) {
#if GFX_VER <= 6
            dirty |= CROCUS_DIRTY_GEN5_BINDING_TABLE_POINTERS;
#endif
            crocus_populate_binding_table(ice, batch, stage, false);
            ice->shaders.prog[stage]->bind_bo_offset =
               crocus_upload_binding_table(ice, batch,
                                           ice->shaders.prog[stage]->surf_offset,
                                           ice->shaders.prog[stage]->bt.size_bytes);

#if GFX_VER >= 7
            crocus_emit_cmd(batch, GENX(3DSTATE_BINDING_TABLE_POINTERS_VS), ptr) {
               ptr._3DCommandSubOpcode = 38 + stage;
               ptr.PointertoVSBindingTable = ice->shaders.prog[stage]->bind_bo_offset;
            }
#endif
#if GFX_VER == 6
         } else if (stage == MESA_SHADER_GEOMETRY && ice->shaders.ff_gs_prog) {
            dirty |= CROCUS_DIRTY_GEN5_BINDING_TABLE_POINTERS;
            crocus_populate_binding_table(ice, batch, stage, true);
            ice->shaders.ff_gs_prog->bind_bo_offset =
               crocus_upload_binding_table(ice, batch,
                                           ice->shaders.ff_gs_prog->surf_offset,
                                           ice->shaders.ff_gs_prog->bt.size_bytes);
#endif
         }
      }
   }
#if GFX_VER <= 6
   if (dirty & CROCUS_DIRTY_GEN5_BINDING_TABLE_POINTERS) {
      struct crocus_compiled_shader *gs = ice->shaders.prog[MESA_SHADER_GEOMETRY];
      if (gs == NULL)
         gs = ice->shaders.ff_gs_prog;
      crocus_emit_cmd(batch, GENX(3DSTATE_BINDING_TABLE_POINTERS), ptr) {
         ptr.PointertoVSBindingTable = ice->shaders.prog[MESA_SHADER_VERTEX]->bind_bo_offset;
         ptr.PointertoPSBindingTable = ice->shaders.prog[MESA_SHADER_FRAGMENT]->bind_bo_offset;
#if GFX_VER == 6
         ptr.VSBindingTableChange = true;
         ptr.PSBindingTableChange = true;
         ptr.GSBindingTableChange = gs ? true : false;
         ptr.PointertoGSBindingTable = gs ? gs->bind_bo_offset : 0;
#endif
      }
   }
#endif

   bool sampler_updates = dirty & CROCUS_DIRTY_GEN6_SAMPLER_STATE_POINTERS;
   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (!(stage_dirty & (CROCUS_STAGE_DIRTY_SAMPLER_STATES_VS << stage)) ||
          !ice->shaders.prog[stage])
         continue;

      crocus_upload_sampler_states(ice, batch, stage);

      sampler_updates = true;

#if GFX_VER >= 7
      struct crocus_shader_state *shs = &ice->state.shaders[stage];

      crocus_emit_cmd(batch, GENX(3DSTATE_SAMPLER_STATE_POINTERS_VS), ptr) {
         ptr._3DCommandSubOpcode = 43 + stage;
         ptr.PointertoVSSamplerState = shs->sampler_offset;
      }
#endif
   }

   if (sampler_updates) {
#if GFX_VER == 6
      struct crocus_shader_state *shs_vs = &ice->state.shaders[MESA_SHADER_VERTEX];
      struct crocus_shader_state *shs_gs = &ice->state.shaders[MESA_SHADER_GEOMETRY];
      struct crocus_shader_state *shs_fs = &ice->state.shaders[MESA_SHADER_FRAGMENT];
      crocus_emit_cmd(batch, GENX(3DSTATE_SAMPLER_STATE_POINTERS), ptr) {
         if (ice->shaders.prog[MESA_SHADER_VERTEX] &&
             (dirty & CROCUS_DIRTY_GEN6_SAMPLER_STATE_POINTERS ||
              stage_dirty & (CROCUS_STAGE_DIRTY_SAMPLER_STATES_VS << MESA_SHADER_VERTEX))) {
            ptr.VSSamplerStateChange = true;
            ptr.PointertoVSSamplerState = shs_vs->sampler_offset;
         }
         if (ice->shaders.prog[MESA_SHADER_GEOMETRY] &&
             (dirty & CROCUS_DIRTY_GEN6_SAMPLER_STATE_POINTERS ||
              stage_dirty & (CROCUS_STAGE_DIRTY_SAMPLER_STATES_VS << MESA_SHADER_GEOMETRY))) {
            ptr.GSSamplerStateChange = true;
            ptr.PointertoGSSamplerState = shs_gs->sampler_offset;
         }
         if (ice->shaders.prog[MESA_SHADER_FRAGMENT] &&
             (dirty & CROCUS_DIRTY_GEN6_SAMPLER_STATE_POINTERS ||
              stage_dirty & (CROCUS_STAGE_DIRTY_SAMPLER_STATES_VS << MESA_SHADER_FRAGMENT))) {
            ptr.PSSamplerStateChange = true;
            ptr.PointertoPSSamplerState = shs_fs->sampler_offset;
         }
      }
#endif
   }

#if GFX_VER >= 6
   if (dirty & CROCUS_DIRTY_GEN6_MULTISAMPLE) {
      crocus_emit_cmd(batch, GENX(3DSTATE_MULTISAMPLE), ms) {
         ms.PixelLocation =
            ice->state.cso_rast->cso.half_pixel_center ? CENTER : UL_CORNER;
         if (ice->state.framebuffer.samples > 0)
            ms.NumberofMultisamples = ffs(ice->state.framebuffer.samples) - 1;
#if GFX_VER == 6
         INTEL_SAMPLE_POS_4X(ms.Sample);
#elif GFX_VER == 7
         switch (ice->state.framebuffer.samples) {
         case 1:
            INTEL_SAMPLE_POS_1X(ms.Sample);
            break;
         case 2:
            INTEL_SAMPLE_POS_2X(ms.Sample);
            break;
         case 4:
            INTEL_SAMPLE_POS_4X(ms.Sample);
            break;
         case 8:
            INTEL_SAMPLE_POS_8X(ms.Sample);
            break;
         default:
            break;
         }
#endif
      }
   }

   if (dirty & CROCUS_DIRTY_GEN6_SAMPLE_MASK) {
      crocus_emit_cmd(batch, GENX(3DSTATE_SAMPLE_MASK), ms) {
         ms.SampleMask = determine_sample_mask(ice);
      }
   }
#endif

#if GFX_VER >= 7
   struct crocus_compiled_shader *shader = ice->shaders.prog[MESA_SHADER_FRAGMENT];
   if ((stage_dirty & CROCUS_STAGE_DIRTY_FS) && shader) {
      struct brw_stage_prog_data *prog_data = shader->prog_data;
      struct brw_wm_prog_data *wm_prog_data = (void *) shader->prog_data;

      crocus_emit_cmd(batch, GENX(3DSTATE_PS), ps) {

         /* Initialize the execution mask with VMask.  Otherwise, derivatives are
          * incorrect for subspans where some of the pixels are unlit.  We believe
          * the bit just didn't take effect in previous generations.
          */
         ps.VectorMaskEnable = GFX_VER >= 8 && wm_prog_data->uses_vmask;

         intel_set_ps_dispatch_state(&ps, &batch->screen->devinfo,
                                     wm_prog_data,
                                     ice->state.framebuffer.samples,
                                     0 /* msaa_flags */);

         ps.DispatchGRFStartRegisterForConstantSetupData0 =
            brw_wm_prog_data_dispatch_grf_start_reg(wm_prog_data, ps, 0);
         ps.DispatchGRFStartRegisterForConstantSetupData1 =
            brw_wm_prog_data_dispatch_grf_start_reg(wm_prog_data, ps, 1);
         ps.DispatchGRFStartRegisterForConstantSetupData2 =
            brw_wm_prog_data_dispatch_grf_start_reg(wm_prog_data, ps, 2);

         ps.KernelStartPointer0 = KSP(ice, shader) +
            brw_wm_prog_data_prog_offset(wm_prog_data, ps, 0);
         ps.KernelStartPointer1 = KSP(ice, shader) +
            brw_wm_prog_data_prog_offset(wm_prog_data, ps, 1);
         ps.KernelStartPointer2 = KSP(ice, shader) +
            brw_wm_prog_data_prog_offset(wm_prog_data, ps, 2);

#if GFX_VERx10 == 75
         ps.SampleMask = determine_sample_mask(ice);
#endif
         // XXX: WABTPPrefetchDisable, see above, drop at C0
         ps.BindingTableEntryCount = shader->bt.size_bytes / 4;
         ps.FloatingPointMode = prog_data->use_alt_mode;
#if GFX_VER >= 8
         ps.MaximumNumberofThreadsPerPSD =
            batch->screen->devinfo.max_threads_per_psd - 2;
#else
         ps.MaximumNumberofThreads = batch->screen->devinfo.max_wm_threads - 1;
#endif

         ps.PushConstantEnable = prog_data->ubo_ranges[0].length > 0;

#if GFX_VER < 8
         ps.oMaskPresenttoRenderTarget = wm_prog_data->uses_omask;
         ps.DualSourceBlendEnable = wm_prog_data->dual_src_blend && ice->state.cso_blend->dual_color_blending;
         ps.AttributeEnable = (wm_prog_data->num_varying_inputs != 0);
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

         if (wm_prog_data->base.total_scratch) {
            struct crocus_bo *bo = crocus_get_scratch_space(ice, wm_prog_data->base.total_scratch, MESA_SHADER_FRAGMENT);
            ps.PerThreadScratchSpace = ffs(wm_prog_data->base.total_scratch) - 11;
            ps.ScratchSpaceBasePointer = rw_bo(bo, 0);
         }
      }
#if GFX_VER == 8
      const struct shader_info *fs_info =
         crocus_get_shader_info(ice, MESA_SHADER_FRAGMENT);
      crocus_emit_cmd(batch, GENX(3DSTATE_PS_EXTRA), psx) {
         psx.PixelShaderValid = true;
         psx.PixelShaderComputedDepthMode = wm_prog_data->computed_depth_mode;
         psx.PixelShaderKillsPixel = wm_prog_data->uses_kill;
         psx.AttributeEnable = wm_prog_data->num_varying_inputs != 0;
         psx.PixelShaderUsesSourceDepth = wm_prog_data->uses_src_depth;
         psx.PixelShaderUsesSourceW = wm_prog_data->uses_src_w;
         psx.PixelShaderIsPerSample =
            brw_wm_prog_data_is_persample(wm_prog_data, 0);

         /* _NEW_MULTISAMPLE | BRW_NEW_CONSERVATIVE_RASTERIZATION */
         if (wm_prog_data->uses_sample_mask)
            psx.PixelShaderUsesInputCoverageMask = true;

         psx.oMaskPresenttoRenderTarget = wm_prog_data->uses_omask;

         /* The stricter cross-primitive coherency guarantees that the hardware
          * gives us with the "Accesses UAV" bit set for at least one shader stage
          * and the "UAV coherency required" bit set on the 3DPRIMITIVE command
          * are redundant within the current image, atomic counter and SSBO GL
          * APIs, which all have very loose ordering and coherency requirements
          * and generally rely on the application to insert explicit barriers when
          * a shader invocation is expected to see the memory writes performed by
          * the invocations of some previous primitive.  Regardless of the value
          * of "UAV coherency required", the "Accesses UAV" bits will implicitly
          * cause an in most cases useless DC flush when the lowermost stage with
          * the bit set finishes execution.
          *
          * It would be nice to disable it, but in some cases we can't because on
          * Gfx8+ it also has an influence on rasterization via the PS UAV-only
          * signal (which could be set independently from the coherency mechanism
          * in the 3DSTATE_WM command on Gfx7), and because in some cases it will
          * determine whether the hardware skips execution of the fragment shader
          * or not via the ThreadDispatchEnable signal.  However if we know that
          * GFX8_PS_BLEND_HAS_WRITEABLE_RT is going to be set and
          * GFX8_PSX_PIXEL_SHADER_NO_RT_WRITE is not set it shouldn't make any
          * difference so we may just disable it here.
          *
          * Gfx8 hardware tries to compute ThreadDispatchEnable for us but doesn't
          * take into account KillPixels when no depth or stencil writes are
          * enabled.  In order for occlusion queries to work correctly with no
          * attachments, we need to force-enable here.
          *
          */
         if ((wm_prog_data->has_side_effects || wm_prog_data->uses_kill) &&
             !(has_writeable_rt(ice->state.cso_blend, fs_info)))
            psx.PixelShaderHasUAV = true;
      }
#endif
   }
#endif

#if GFX_VER >= 7
   if (ice->state.streamout_active) {
      if (dirty & CROCUS_DIRTY_GEN7_SO_BUFFERS) {
         for (int i = 0; i < 4; i++) {
            struct crocus_stream_output_target *tgt =
               (void *) ice->state.so_target[i];

            if (!tgt) {
               crocus_emit_cmd(batch, GENX(3DSTATE_SO_BUFFER), sob) {
                  sob.SOBufferIndex = i;
                  sob.MOCS = crocus_mocs(NULL, &batch->screen->isl_dev);
               }
               continue;
            }
            struct crocus_resource *res = (void *) tgt->base.buffer;
            uint32_t start = tgt->base.buffer_offset;
#if GFX_VER < 8
            uint32_t end = ALIGN(start + tgt->base.buffer_size, 4);
#endif
            crocus_emit_cmd(batch, GENX(3DSTATE_SO_BUFFER), sob) {
               sob.SOBufferIndex = i;

               sob.SurfaceBaseAddress = rw_bo(res->bo, start);
               sob.MOCS = crocus_mocs(res->bo, &batch->screen->isl_dev);
#if GFX_VER < 8
               sob.SurfacePitch = tgt->stride;
               sob.SurfaceEndAddress = rw_bo(res->bo, end);
#else
               sob.SOBufferEnable = true;
               sob.StreamOffsetWriteEnable = true;
               sob.StreamOutputBufferOffsetAddressEnable = true;

               sob.SurfaceSize = MAX2(tgt->base.buffer_size / 4, 1) - 1;
               sob.StreamOutputBufferOffsetAddress =
                  rw_bo(crocus_resource_bo(&tgt->offset_res->base.b), tgt->offset_offset);
               if (tgt->zero_offset) {
                  sob.StreamOffset = 0;
                  tgt->zero_offset = false;
               } else
                  sob.StreamOffset = 0xFFFFFFFF; /* not offset, see above */
#endif
            }
         }
      }

      if ((dirty & CROCUS_DIRTY_SO_DECL_LIST) && ice->state.streamout) {
         uint32_t *decl_list =
            ice->state.streamout + GENX(3DSTATE_STREAMOUT_length);
         crocus_batch_emit(batch, decl_list, 4 * ((decl_list[0] & 0xff) + 2));
      }

      if (dirty & CROCUS_DIRTY_STREAMOUT) {
         const struct crocus_rasterizer_state *cso_rast = ice->state.cso_rast;

         uint32_t dynamic_sol[GENX(3DSTATE_STREAMOUT_length)];
         crocus_pack_command(GENX(3DSTATE_STREAMOUT), dynamic_sol, sol) {
            sol.SOFunctionEnable = true;
            sol.SOStatisticsEnable = true;

            sol.RenderingDisable = cso_rast->cso.rasterizer_discard &&
                                   !ice->state.prims_generated_query_active;
            sol.ReorderMode = cso_rast->cso.flatshade_first ? LEADING : TRAILING;
         }

         assert(ice->state.streamout);

         crocus_emit_merge(batch, ice->state.streamout, dynamic_sol,
                         GENX(3DSTATE_STREAMOUT_length));
      }
   } else {
      if (dirty & CROCUS_DIRTY_STREAMOUT) {
         crocus_emit_cmd(batch, GENX(3DSTATE_STREAMOUT), sol);
      }
   }
#endif
#if GFX_VER == 6
   if (ice->state.streamout_active) {
      if (dirty & CROCUS_DIRTY_GEN6_SVBI) {
         crocus_emit_so_svbi(ice);
      }
   }
#endif

   if (dirty & CROCUS_DIRTY_CLIP) {
#if GFX_VER < 6
      const struct brw_clip_prog_data *clip_prog_data = (struct brw_clip_prog_data *)ice->shaders.clip_prog->prog_data;
      struct pipe_rasterizer_state *cso_state = &ice->state.cso_rast->cso;

      uint32_t *clip_ptr = stream_state(batch, GENX(CLIP_STATE_length) * 4, 32, &ice->shaders.clip_offset);
      dirty |= CROCUS_DIRTY_GEN5_PIPELINED_POINTERS;
      _crocus_pack_state(batch, GENX(CLIP_STATE), clip_ptr, clip) {
         clip.KernelStartPointer = KSP(ice, ice->shaders.clip_prog);
         clip.FloatingPointMode = FLOATING_POINT_MODE_Alternate;
         clip.SingleProgramFlow = true;
         clip.GRFRegisterCount = DIV_ROUND_UP(clip_prog_data->total_grf, 16) - 1;

         clip.VertexURBEntryReadLength = clip_prog_data->urb_read_length;
         clip.ConstantURBEntryReadLength = clip_prog_data->curb_read_length;

         clip.DispatchGRFStartRegisterForURBData = 1;
         clip.VertexURBEntryReadOffset = 0;
         clip.ConstantURBEntryReadOffset = ice->curbe.clip_start * 2;

         clip.NumberofURBEntries = batch->ice->urb.nr_clip_entries;
         clip.URBEntryAllocationSize = batch->ice->urb.vsize - 1;

         if (batch->ice->urb.nr_clip_entries >= 10) {
            /* Half of the URB entries go to each thread, and it has to be an
             * even number.
             */
            assert(batch->ice->urb.nr_clip_entries % 2 == 0);

            /* Although up to 16 concurrent Clip threads are allowed on Ironlake,
             * only 2 threads can output VUEs at a time.
             */
            clip.MaximumNumberofThreads = (GFX_VER == 5 ? 16 : 2) - 1;
         } else {
            assert(batch->ice->urb.nr_clip_entries >= 5);
            clip.MaximumNumberofThreads = 1 - 1;
         }
         clip.VertexPositionSpace = VPOS_NDCSPACE;
         clip.UserClipFlagsMustClipEnable = true;
         clip.GuardbandClipTestEnable = true;

         clip.ClipperViewportStatePointer = ro_bo(batch->state.bo, ice->state.clip_vp_address);
         clip.ScreenSpaceViewportXMin = -1.0;
         clip.ScreenSpaceViewportXMax = 1.0;
         clip.ScreenSpaceViewportYMin = -1.0;
         clip.ScreenSpaceViewportYMax = 1.0;
         clip.ViewportXYClipTestEnable = true;
         clip.ViewportZClipTestEnable = (cso_state->depth_clip_near || cso_state->depth_clip_far);

#if GFX_VER == 5 || GFX_VERx10 == 45
         clip.UserClipDistanceClipTestEnableBitmask = cso_state->clip_plane_enable;
#else
         /* Up to 6 actual clip flags, plus the 7th for the negative RHW
          * workaround.
          */
         clip.UserClipDistanceClipTestEnableBitmask = (cso_state->clip_plane_enable & 0x3f) | 0x40;
#endif

         clip.APIMode = cso_state->clip_halfz ? APIMODE_D3D : APIMODE_OGL;
         clip.GuardbandClipTestEnable = true;

         clip.ClipMode = clip_prog_data->clip_mode;
#if GFX_VERx10 == 45
         clip.NegativeWClipTestEnable = true;
#endif
      }

#else //if GFX_VER >= 6
      struct crocus_rasterizer_state *cso_rast = ice->state.cso_rast;
      const struct brw_wm_prog_data *wm_prog_data = brw_wm_prog_data(ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data );
      struct pipe_framebuffer_state *cso_fb = &ice->state.framebuffer;
      bool gs_or_tes = ice->shaders.prog[MESA_SHADER_GEOMETRY] ||
                       ice->shaders.prog[MESA_SHADER_TESS_EVAL];
      bool points_or_lines = cso_rast->fill_mode_point_or_line ||
         (gs_or_tes ? ice->shaders.output_topology_is_points_or_lines
                    : ice->state.prim_is_points_or_lines);
      uint32_t dynamic_clip[GENX(3DSTATE_CLIP_length)];
      crocus_pack_command(GENX(3DSTATE_CLIP), &dynamic_clip, cl) {
         cl.StatisticsEnable = ice->state.statistics_counters_enabled;
         if (cso_rast->cso.rasterizer_discard)
            cl.ClipMode = CLIPMODE_REJECT_ALL;
         else if (ice->state.window_space_position)
            cl.ClipMode = CLIPMODE_ACCEPT_ALL;
         else
            cl.ClipMode = CLIPMODE_NORMAL;

         cl.PerspectiveDivideDisable = ice->state.window_space_position;
         cl.ViewportXYClipTestEnable = !points_or_lines;

         cl.UserClipDistanceCullTestEnableBitmask =
            brw_vue_prog_data(ice->shaders.prog[MESA_SHADER_VERTEX]->prog_data)->cull_distance_mask;

         cl.NonPerspectiveBarycentricEnable = wm_prog_data->uses_nonperspective_interp_modes;

         cl.ForceZeroRTAIndexEnable = cso_fb->layers <= 1;
         cl.MaximumVPIndex = ice->state.num_viewports - 1;
      }
      crocus_emit_merge(batch, cso_rast->clip, dynamic_clip,
                      ARRAY_SIZE(cso_rast->clip));
#endif
   }

   if (stage_dirty & CROCUS_STAGE_DIRTY_VS) {
      struct crocus_compiled_shader *shader = ice->shaders.prog[MESA_SHADER_VERTEX];
      const struct brw_vue_prog_data *vue_prog_data = brw_vue_prog_data(shader->prog_data);
      const struct brw_stage_prog_data *prog_data = &vue_prog_data->base;
#if GFX_VER == 7
      if (batch->screen->devinfo.platform == INTEL_PLATFORM_IVB)
         gen7_emit_vs_workaround_flush(batch);
#endif


#if GFX_VER == 6
      struct push_bos push_bos = {};
      setup_constant_buffers(ice, batch, MESA_SHADER_VERTEX, &push_bos);

      emit_push_constant_packets(ice, batch, MESA_SHADER_VERTEX, &push_bos);
#endif
#if GFX_VER >= 6
      crocus_emit_cmd(batch, GENX(3DSTATE_VS), vs)
#else
      uint32_t *vs_ptr = stream_state(batch,
                                      GENX(VS_STATE_length) * 4, 32, &ice->shaders.vs_offset);
      dirty |= CROCUS_DIRTY_GEN5_PIPELINED_POINTERS;
      _crocus_pack_state(batch, GENX(VS_STATE), vs_ptr, vs)
#endif
      {
         INIT_THREAD_DISPATCH_FIELDS(vs, Vertex, MESA_SHADER_VERTEX);

         vs.MaximumNumberofThreads = batch->screen->devinfo.max_vs_threads - 1;

#if GFX_VER < 6
         vs.GRFRegisterCount = DIV_ROUND_UP(vue_prog_data->total_grf, 16) - 1;
         vs.ConstantURBEntryReadLength = vue_prog_data->base.curb_read_length;
         vs.ConstantURBEntryReadOffset = ice->curbe.vs_start * 2;

         vs.NumberofURBEntries = batch->ice->urb.nr_vs_entries >> (GFX_VER == 5 ? 2 : 0);
         vs.URBEntryAllocationSize = batch->ice->urb.vsize - 1;

         vs.MaximumNumberofThreads =
            CLAMP(batch->ice->urb.nr_vs_entries / 2, 1, batch->screen->devinfo.max_vs_threads) - 1;
         vs.StatisticsEnable = false;
         vs.SamplerStatePointer = ro_bo(batch->state.bo, ice->state.shaders[MESA_SHADER_VERTEX].sampler_offset);
#endif
#if GFX_VER == 5
         /* Force single program flow on Ironlake.  We cannot reliably get
          * all applications working without it.  See:
          * https://bugs.freedesktop.org/show_bug.cgi?id=29172
          *
          * The most notable and reliably failing application is the Humus
          * demo "CelShading"
          */
         vs.SingleProgramFlow = true;
         vs.SamplerCount = 0; /* hardware requirement */

#endif
#if GFX_VER >= 8
         vs.SIMD8DispatchEnable =
            vue_prog_data->dispatch_mode == DISPATCH_MODE_SIMD8;

         vs.UserClipDistanceCullTestEnableBitmask =
            vue_prog_data->cull_distance_mask;
#endif
      }

#if GFX_VER == 6
      crocus_emit_pipe_control_flush(batch,
                                     "post VS const",
                                     PIPE_CONTROL_DEPTH_STALL |
                                     PIPE_CONTROL_INSTRUCTION_INVALIDATE |
                                     PIPE_CONTROL_STATE_CACHE_INVALIDATE);
#endif
   }

   if (stage_dirty & CROCUS_STAGE_DIRTY_GS) {
      struct crocus_compiled_shader *shader = ice->shaders.prog[MESA_SHADER_GEOMETRY];
      bool active = GFX_VER >= 6 && shader;
#if GFX_VER == 6
      struct push_bos push_bos = {};
      if (shader)
         setup_constant_buffers(ice, batch, MESA_SHADER_GEOMETRY, &push_bos);

      emit_push_constant_packets(ice, batch, MESA_SHADER_GEOMETRY, &push_bos);
#endif
#if GFX_VERx10 == 70
   /**
    * From Graphics BSpec: 3D-Media-GPGPU Engine > 3D Pipeline Stages >
    * Geometry > Geometry Shader > State:
    *
    *     "Note: Because of corruption in IVB:GT2, software needs to flush the
    *     whole fixed function pipeline when the GS enable changes value in
    *     the 3DSTATE_GS."
    *
    * The hardware architects have clarified that in this context "flush the
    * whole fixed function pipeline" means to emit a PIPE_CONTROL with the "CS
    * Stall" bit set.
    */
   if (batch->screen->devinfo.gt == 2 && ice->state.gs_enabled != active)
      gen7_emit_cs_stall_flush(batch);
#endif
#if GFX_VER >= 6
      crocus_emit_cmd(batch, GENX(3DSTATE_GS), gs)
#else
      uint32_t *gs_ptr = stream_state(batch,
                                      GENX(GS_STATE_length) * 4, 32, &ice->shaders.gs_offset);
      dirty |= CROCUS_DIRTY_GEN5_PIPELINED_POINTERS;
      _crocus_pack_state(batch, GENX(GS_STATE), gs_ptr, gs)
#endif
     {
#if GFX_VER >= 6
         if (active) {
            const struct brw_gs_prog_data *gs_prog_data = brw_gs_prog_data(shader->prog_data);
            const struct brw_vue_prog_data *vue_prog_data = brw_vue_prog_data(shader->prog_data);
            const struct brw_stage_prog_data *prog_data = &gs_prog_data->base.base;

            INIT_THREAD_DISPATCH_FIELDS(gs, Vertex, MESA_SHADER_GEOMETRY);
#if GFX_VER >= 7
            gs.OutputVertexSize = gs_prog_data->output_vertex_size_hwords * 2 - 1;
            gs.OutputTopology = gs_prog_data->output_topology;
            gs.ControlDataHeaderSize =
               gs_prog_data->control_data_header_size_hwords;

            gs.InstanceControl = gs_prog_data->invocations - 1;
            gs.DispatchMode = vue_prog_data->dispatch_mode;

            gs.IncludePrimitiveID = gs_prog_data->include_primitive_id;

            gs.ControlDataFormat = gs_prog_data->control_data_format;
#endif

            /* Note: the meaning of the GEN7_GS_REORDER_TRAILING bit changes between
             * Ivy Bridge and Haswell.
             *
             * On Ivy Bridge, setting this bit causes the vertices of a triangle
             * strip to be delivered to the geometry shader in an order that does
             * not strictly follow the OpenGL spec, but preserves triangle
             * orientation.  For example, if the vertices are (1, 2, 3, 4, 5), then
             * the geometry shader sees triangles:
             *
             * (1, 2, 3), (2, 4, 3), (3, 4, 5)
             *
             * (Clearing the bit is even worse, because it fails to preserve
             * orientation).
             *
             * Triangle strips with adjacency always ordered in a way that preserves
             * triangle orientation but does not strictly follow the OpenGL spec,
             * regardless of the setting of this bit.
             *
             * On Haswell, both triangle strips and triangle strips with adjacency
             * are always ordered in a way that preserves triangle orientation.
             * Setting this bit causes the ordering to strictly follow the OpenGL
             * spec.
             *
             * So in either case we want to set the bit.  Unfortunately on Ivy
             * Bridge this will get the order close to correct but not perfect.
             */
            gs.ReorderMode = TRAILING;
            gs.MaximumNumberofThreads =
               GFX_VER == 8 ? (batch->screen->devinfo.max_gs_threads / 2 - 1) :
               (batch->screen->devinfo.max_gs_threads - 1);
#if GFX_VER < 7
            gs.SOStatisticsEnable = true;
            if (gs_prog_data->num_transform_feedback_bindings)
               gs.SVBIPayloadEnable = ice->state.streamout_active;

            /* GEN6_GS_SPF_MODE and GEN6_GS_VECTOR_MASK_ENABLE are enabled as it
             * was previously done for gen6.
             *
             * TODO: test with both disabled to see if the HW is behaving
             * as expected, like in gen7.
             */
            gs.SingleProgramFlow = true;
            gs.VectorMaskEnable = true;
#endif
#if GFX_VER >= 8
            gs.ExpectedVertexCount = gs_prog_data->vertices_in;

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
#endif
         }
#endif
#if GFX_VER <= 6
         if (!active && ice->shaders.ff_gs_prog) {
            const struct brw_ff_gs_prog_data *gs_prog_data = (struct brw_ff_gs_prog_data *)ice->shaders.ff_gs_prog->prog_data;
            /* In gen6, transform feedback for the VS stage is done with an
             * ad-hoc GS program. This function provides the needed 3DSTATE_GS
             * for this.
             */
            gs.KernelStartPointer = KSP(ice, ice->shaders.ff_gs_prog);
            gs.SingleProgramFlow = true;
            gs.DispatchGRFStartRegisterForURBData = GFX_VER == 6 ? 2 : 1;
            gs.VertexURBEntryReadLength = gs_prog_data->urb_read_length;

#if GFX_VER <= 5
            gs.GRFRegisterCount =
               DIV_ROUND_UP(gs_prog_data->total_grf, 16) - 1;
            /* BRW_NEW_URB_FENCE */
            gs.NumberofURBEntries = batch->ice->urb.nr_gs_entries;
            gs.URBEntryAllocationSize = batch->ice->urb.vsize - 1;
            gs.MaximumNumberofThreads = batch->ice->urb.nr_gs_entries >= 8 ? 1 : 0;
            gs.FloatingPointMode = FLOATING_POINT_MODE_Alternate;
#else
            gs.Enable = true;
            gs.VectorMaskEnable = true;
            gs.SVBIPayloadEnable = true;
            gs.SVBIPostIncrementEnable = true;
            gs.SVBIPostIncrementValue = gs_prog_data->svbi_postincrement_value;
            gs.SOStatisticsEnable = true;
            gs.MaximumNumberofThreads = batch->screen->devinfo.max_gs_threads - 1;
#endif
         }
#endif
         if (!active && !ice->shaders.ff_gs_prog) {
#if GFX_VER < 8
            gs.DispatchGRFStartRegisterForURBData = 1;
#if GFX_VER >= 7
            gs.IncludeVertexHandles = true;
#endif
#endif
         }
#if GFX_VER >= 6
         gs.StatisticsEnable = true;
#endif
#if GFX_VER == 5 || GFX_VER == 6
         gs.RenderingEnabled = true;
#endif
#if GFX_VER <= 5
         gs.MaximumVPIndex = ice->state.num_viewports - 1;
#endif
      }
      ice->state.gs_enabled = active;
   }

#if GFX_VER >= 7
   if (stage_dirty & CROCUS_STAGE_DIRTY_TCS) {
      struct crocus_compiled_shader *shader = ice->shaders.prog[MESA_SHADER_TESS_CTRL];

      if (shader) {
         const struct brw_tcs_prog_data *tcs_prog_data = brw_tcs_prog_data(shader->prog_data);
         const struct brw_vue_prog_data *vue_prog_data = brw_vue_prog_data(shader->prog_data);
         const struct brw_stage_prog_data *prog_data = &tcs_prog_data->base.base;

         crocus_emit_cmd(batch, GENX(3DSTATE_HS), hs) {
            INIT_THREAD_DISPATCH_FIELDS(hs, Vertex, MESA_SHADER_TESS_CTRL);
            hs.InstanceCount = tcs_prog_data->instances - 1;
            hs.IncludeVertexHandles = true;
            hs.MaximumNumberofThreads = batch->screen->devinfo.max_tcs_threads - 1;
         }
      } else {
         crocus_emit_cmd(batch, GENX(3DSTATE_HS), hs);
      }

   }

   if (stage_dirty & CROCUS_STAGE_DIRTY_TES) {
      struct crocus_compiled_shader *shader = ice->shaders.prog[MESA_SHADER_TESS_EVAL];
      if (shader) {
         const struct brw_tes_prog_data *tes_prog_data = brw_tes_prog_data(shader->prog_data);
         const struct brw_vue_prog_data *vue_prog_data = brw_vue_prog_data(shader->prog_data);
         const struct brw_stage_prog_data *prog_data = &tes_prog_data->base.base;

         crocus_emit_cmd(batch, GENX(3DSTATE_TE), te) {
            te.Partitioning = tes_prog_data->partitioning;
            te.OutputTopology = tes_prog_data->output_topology;
            te.TEDomain = tes_prog_data->domain;
            te.TEEnable = true;
            te.MaximumTessellationFactorOdd = 63.0;
            te.MaximumTessellationFactorNotOdd = 64.0;
         };
         crocus_emit_cmd(batch, GENX(3DSTATE_DS), ds) {
            INIT_THREAD_DISPATCH_FIELDS(ds, Patch, MESA_SHADER_TESS_EVAL);

            ds.MaximumNumberofThreads = batch->screen->devinfo.max_tes_threads - 1;
            ds.ComputeWCoordinateEnable =
               tes_prog_data->domain == BRW_TESS_DOMAIN_TRI;

#if GFX_VER >= 8
            if (vue_prog_data->dispatch_mode == DISPATCH_MODE_SIMD8)
               ds.DispatchMode = DISPATCH_MODE_SIMD8_SINGLE_PATCH;
            ds.UserClipDistanceCullTestEnableBitmask =
               vue_prog_data->cull_distance_mask;
#endif
         };
      } else {
         crocus_emit_cmd(batch, GENX(3DSTATE_TE), te);
         crocus_emit_cmd(batch, GENX(3DSTATE_DS), ds);
      }
   }
#endif
   if (dirty & CROCUS_DIRTY_RASTER) {

#if GFX_VER < 6
      const struct brw_sf_prog_data *sf_prog_data = (struct brw_sf_prog_data *)ice->shaders.sf_prog->prog_data;
      struct pipe_rasterizer_state *cso_state = &ice->state.cso_rast->cso;
      uint32_t *sf_ptr = stream_state(batch,
                                      GENX(SF_STATE_length) * 4, 32, &ice->shaders.sf_offset);
      dirty |= CROCUS_DIRTY_GEN5_PIPELINED_POINTERS;
      _crocus_pack_state(batch, GENX(SF_STATE), sf_ptr, sf) {
         sf.KernelStartPointer = KSP(ice, ice->shaders.sf_prog);
         sf.FloatingPointMode = FLOATING_POINT_MODE_Alternate;
         sf.GRFRegisterCount = DIV_ROUND_UP(sf_prog_data->total_grf, 16) - 1;
         sf.DispatchGRFStartRegisterForURBData = 3;
         sf.VertexURBEntryReadOffset = BRW_SF_URB_ENTRY_READ_OFFSET;
         sf.VertexURBEntryReadLength = sf_prog_data->urb_read_length;
         sf.URBEntryAllocationSize = batch->ice->urb.sfsize - 1;
         sf.NumberofURBEntries = batch->ice->urb.nr_sf_entries;
         sf.PointRasterizationRule = RASTRULE_UPPER_RIGHT;

         sf.SetupViewportStateOffset = ro_bo(batch->state.bo, ice->state.sf_vp_address);

         sf.MaximumNumberofThreads =
            MIN2(GFX_VER == 5 ? 48 : 24, batch->ice->urb.nr_sf_entries) - 1;

         sf.SpritePointEnable = cso_state->point_quad_rasterization;
         sf.DestinationOriginHorizontalBias = 0.5;
         sf.DestinationOriginVerticalBias = 0.5;

	 sf.LineEndCapAntialiasingRegionWidth =
            cso_state->line_smooth ? _10pixels : _05pixels;
         sf.LastPixelEnable = cso_state->line_last_pixel;
         sf.AntialiasingEnable = cso_state->line_smooth;

         sf.LineWidth = get_line_width(cso_state);
         sf.PointWidth = cso_state->point_size;
         sf.PointWidthSource = cso_state->point_size_per_vertex ? Vertex : State;
#if GFX_VERx10 >= 45
         sf.AALineDistanceMode = AALINEDISTANCE_TRUE;
#endif
         sf.ViewportTransformEnable = true;
         sf.FrontWinding = cso_state->front_ccw ? 1 : 0;
         sf.ScissorRectangleEnable = true;
         sf.CullMode = translate_cull_mode(cso_state->cull_face);

         if (cso_state->flatshade_first) {
            sf.TriangleFanProvokingVertexSelect = 1;
         } else {
            sf.TriangleStripListProvokingVertexSelect = 2;
            sf.TriangleFanProvokingVertexSelect = 2;
            sf.LineStripListProvokingVertexSelect = 1;
         }
      }
#else
      struct crocus_rasterizer_state *cso = ice->state.cso_rast;
      uint32_t dynamic_sf[GENX(3DSTATE_SF_length)];
      crocus_pack_command(GENX(3DSTATE_SF), &dynamic_sf, sf) {
         sf.ViewportTransformEnable = !ice->state.window_space_position;

#if GFX_VER == 6
         const struct brw_wm_prog_data *wm_prog_data = brw_wm_prog_data(ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data);
         uint32_t urb_entry_read_length;
         uint32_t urb_entry_read_offset;
         uint32_t point_sprite_enables;
         calculate_attr_overrides(ice, sf.Attribute, &point_sprite_enables,
                                  &urb_entry_read_length,
                                  &urb_entry_read_offset);
         sf.VertexURBEntryReadLength = urb_entry_read_length;
         sf.VertexURBEntryReadOffset = urb_entry_read_offset;
         sf.PointSpriteTextureCoordinateEnable = point_sprite_enables;
         sf.ConstantInterpolationEnable = wm_prog_data->flat_inputs;
         sf.NumberofSFOutputAttributes = wm_prog_data->num_varying_inputs;
#endif

#if GFX_VER >= 6 && GFX_VER < 8
         if (ice->state.framebuffer.samples > 1 && ice->state.cso_rast->cso.multisample)
            sf.MultisampleRasterizationMode = MSRASTMODE_ON_PATTERN;
#endif
#if GFX_VER == 7
         if (ice->state.framebuffer.zsbuf) {
            struct crocus_resource *zres, *sres;
               crocus_get_depth_stencil_resources(&batch->screen->devinfo,
                                                  ice->state.framebuffer.zsbuf->texture,
                                                  &zres, &sres);
            /* ANV thinks that the stencil-ness doesn't matter, this is just
             * about handling polygon offset scaling.
             */
            sf.DepthBufferSurfaceFormat = zres ? isl_format_get_depth_format(zres->surf.format, false) : D16_UNORM;
         }
#endif
      }
      crocus_emit_merge(batch, cso->sf, dynamic_sf,
                      ARRAY_SIZE(dynamic_sf));
#if GFX_VER == 8
      crocus_batch_emit(batch, cso->raster, sizeof(cso->raster));
#endif
#endif
   }

   if (dirty & CROCUS_DIRTY_WM) {
      struct crocus_rasterizer_state *cso = ice->state.cso_rast;
      const struct brw_wm_prog_data *wm_prog_data = brw_wm_prog_data(ice->shaders.prog[MESA_SHADER_FRAGMENT]->prog_data);
      UNUSED bool writes_depth = wm_prog_data->computed_depth_mode != BRW_PSCDEPTH_OFF;
      UNUSED const struct shader_info *fs_info =
         crocus_get_shader_info(ice, MESA_SHADER_FRAGMENT);

#if GFX_VER == 6
      struct push_bos push_bos = {};
      setup_constant_buffers(ice, batch, MESA_SHADER_FRAGMENT, &push_bos);

      emit_push_constant_packets(ice, batch, MESA_SHADER_FRAGMENT, &push_bos);
#endif
#if GFX_VER >= 6
      crocus_emit_cmd(batch, GENX(3DSTATE_WM), wm)
#else
      uint32_t *wm_ptr = stream_state(batch,
                                      GENX(WM_STATE_length) * 4, 32, &ice->shaders.wm_offset);

      dirty |= CROCUS_DIRTY_GEN5_PIPELINED_POINTERS;

      _crocus_pack_state(batch, GENX(WM_STATE), wm_ptr, wm)
#endif
     {
#if GFX_VER <= 6
         wm._8PixelDispatchEnable = wm_prog_data->dispatch_8;
         wm._16PixelDispatchEnable = wm_prog_data->dispatch_16;
         wm._32PixelDispatchEnable = wm_prog_data->dispatch_32;
#endif
#if GFX_VER == 4
      /* On gen4, we only have one shader kernel */
         if (brw_wm_state_has_ksp(wm, 0)) {
            wm.KernelStartPointer0 = KSP(ice, ice->shaders.prog[MESA_SHADER_FRAGMENT]);
            wm.GRFRegisterCount0 = brw_wm_prog_data_reg_blocks(wm_prog_data, wm, 0);
            wm.DispatchGRFStartRegisterForConstantSetupData0 =
               wm_prog_data->base.dispatch_grf_start_reg;
         }
#elif GFX_VER == 5
         wm.KernelStartPointer0 = KSP(ice, ice->shaders.prog[MESA_SHADER_FRAGMENT]) +
            brw_wm_prog_data_prog_offset(wm_prog_data, wm, 0);
         wm.KernelStartPointer1 = KSP(ice, ice->shaders.prog[MESA_SHADER_FRAGMENT]) +
            brw_wm_prog_data_prog_offset(wm_prog_data, wm, 1);
         wm.KernelStartPointer2 = KSP(ice, ice->shaders.prog[MESA_SHADER_FRAGMENT]) +
            brw_wm_prog_data_prog_offset(wm_prog_data, wm, 2);

         wm.GRFRegisterCount0 = brw_wm_prog_data_reg_blocks(wm_prog_data, wm, 0);
         wm.GRFRegisterCount1 = brw_wm_prog_data_reg_blocks(wm_prog_data, wm, 1);
         wm.GRFRegisterCount2 = brw_wm_prog_data_reg_blocks(wm_prog_data, wm, 2);

         wm.DispatchGRFStartRegisterForConstantSetupData0 =
            wm_prog_data->base.dispatch_grf_start_reg;
#elif GFX_VER == 6
         wm.KernelStartPointer0 = KSP(ice, ice->shaders.prog[MESA_SHADER_FRAGMENT]) +
            brw_wm_prog_data_prog_offset(wm_prog_data, wm, 0);
         wm.KernelStartPointer1 = KSP(ice, ice->shaders.prog[MESA_SHADER_FRAGMENT]) +
            brw_wm_prog_data_prog_offset(wm_prog_data, wm, 1);
         wm.KernelStartPointer2 = KSP(ice, ice->shaders.prog[MESA_SHADER_FRAGMENT]) +
            brw_wm_prog_data_prog_offset(wm_prog_data, wm, 2);

         wm.DispatchGRFStartRegisterForConstantSetupData0 =
           brw_wm_prog_data_dispatch_grf_start_reg(wm_prog_data, wm, 0);
         wm.DispatchGRFStartRegisterForConstantSetupData1 =
           brw_wm_prog_data_dispatch_grf_start_reg(wm_prog_data, wm, 1);
         wm.DispatchGRFStartRegisterForConstantSetupData2 =
           brw_wm_prog_data_dispatch_grf_start_reg(wm_prog_data, wm, 2);
#endif
#if GFX_VER <= 5
         wm.ConstantURBEntryReadLength = wm_prog_data->base.curb_read_length;
         wm.ConstantURBEntryReadOffset = ice->curbe.wm_start * 2;
         wm.SetupURBEntryReadLength = wm_prog_data->num_varying_inputs * 2;
         wm.SetupURBEntryReadOffset = 0;
         wm.EarlyDepthTestEnable = true;
         wm.LineAntialiasingRegionWidth = _05pixels;
         wm.LineEndCapAntialiasingRegionWidth = _10pixels;
         wm.DepthCoefficientURBReadOffset = 1;

         if (cso->cso.offset_tri) {
            wm.GlobalDepthOffsetEnable = true;

         /* Something weird going on with legacy_global_depth_bias,
          * offset_constant, scaling and MRD.  This value passes glean
          * but gives some odd results elsewere (eg. the
          * quad-offset-units test).
          */
            wm.GlobalDepthOffsetConstant = cso->cso.offset_units * 2;
            wm.GlobalDepthOffsetScale = cso->cso.offset_scale;
         }
         wm.SamplerStatePointer = ro_bo(batch->state.bo,
                                        ice->state.shaders[MESA_SHADER_FRAGMENT].sampler_offset);
#endif

         wm.StatisticsEnable = (GFX_VER >= 6 || ice->state.stats_wm) ?
            ice->state.statistics_counters_enabled : 0;

#if GFX_VER >= 6
         wm.LineAntialiasingRegionWidth = _10pixels;
         wm.LineEndCapAntialiasingRegionWidth = _05pixels;

         wm.PointRasterizationRule = RASTRULE_UPPER_RIGHT;
         wm.BarycentricInterpolationMode = wm_prog_data->barycentric_interp_modes;
#endif
#if GFX_VER == 6
      wm.DualSourceBlendEnable = wm_prog_data->dual_src_blend &&
         ice->state.cso_blend->dual_color_blending;
      wm.oMaskPresenttoRenderTarget = wm_prog_data->uses_omask;
      wm.NumberofSFOutputAttributes = wm_prog_data->num_varying_inputs;

      /* From the SNB PRM, volume 2 part 1, page 281:
       * "If the PS kernel does not need the Position XY Offsets
       * to compute a Position XY value, then this field should be
       * programmed to POSOFFSET_NONE."
       *
       * "SW Recommendation: If the PS kernel needs the Position Offsets
       * to compute a Position XY value, this field should match Position
       * ZW Interpolation Mode to ensure a consistent position.xyzw
       * computation."
       * We only require XY sample offsets. So, this recommendation doesn't
       * look useful at the moment. We might need this in future.
       */
      if (wm_prog_data->uses_pos_offset)
         wm.PositionXYOffsetSelect = POSOFFSET_SAMPLE;
      else
         wm.PositionXYOffsetSelect = POSOFFSET_NONE;
#endif
         wm.LineStippleEnable = cso->cso.line_stipple_enable;
         wm.PolygonStippleEnable = cso->cso.poly_stipple_enable;

#if GFX_VER < 7
         if (wm_prog_data->base.use_alt_mode)
            wm.FloatingPointMode = FLOATING_POINT_MODE_Alternate;
         wm.BindingTableEntryCount = ice->shaders.prog[MESA_SHADER_FRAGMENT]->bt.size_bytes / 4;
         wm.MaximumNumberofThreads = batch->screen->devinfo.max_wm_threads - 1;
#endif

#if GFX_VER < 8
#if GFX_VER >= 6
         wm.PixelShaderUsesSourceW = wm_prog_data->uses_src_w;

         struct pipe_framebuffer_state *fb = &ice->state.framebuffer;
         if (fb->samples > 1) {
            if (cso->cso.multisample)
               wm.MultisampleRasterizationMode = MSRASTMODE_ON_PATTERN;
            else
               wm.MultisampleRasterizationMode = MSRASTMODE_OFF_PIXEL;

            if (brw_wm_prog_data_is_persample(wm_prog_data, 0))
               wm.MultisampleDispatchMode = MSDISPMODE_PERSAMPLE;
            else
               wm.MultisampleDispatchMode = MSDISPMODE_PERPIXEL;
         } else {
            wm.MultisampleRasterizationMode = MSRASTMODE_OFF_PIXEL;
            wm.MultisampleDispatchMode = MSDISPMODE_PERSAMPLE;
         }
#endif

         wm.PixelShaderUsesSourceDepth = wm_prog_data->uses_src_depth;

         if (wm_prog_data->uses_kill ||
             ice->state.cso_zsa->cso.alpha_enabled ||
             ice->state.cso_blend->cso.alpha_to_coverage ||
             (GFX_VER >= 6 && wm_prog_data->uses_omask))
            wm.PixelShaderKillsPixel = true;

         if (has_writeable_rt(ice->state.cso_blend, fs_info) ||
             writes_depth || wm.PixelShaderKillsPixel ||
             (GFX_VER >= 6 && wm_prog_data->has_side_effects))
            wm.ThreadDispatchEnable = true;

#if GFX_VER >= 7
         wm.PixelShaderComputedDepthMode = wm_prog_data->computed_depth_mode;
         wm.PixelShaderUsesInputCoverageMask = wm_prog_data->uses_sample_mask;
#else
         if (wm_prog_data->base.total_scratch) {
            struct crocus_bo *bo = crocus_get_scratch_space(ice, wm_prog_data->base.total_scratch,
                                                            MESA_SHADER_FRAGMENT);
            wm.PerThreadScratchSpace = ffs(wm_prog_data->base.total_scratch) - 11;
            wm.ScratchSpaceBasePointer = rw_bo(bo, 0);
         }

         wm.PixelShaderComputedDepth = writes_depth;

#endif
         /* The "UAV access enable" bits are unnecessary on HSW because they only
          * seem to have an effect on the HW-assisted coherency mechanism which we
          * don't need, and the rasterization-related UAV_ONLY flag and the
          * DISPATCH_ENABLE bit can be set independently from it.
          * C.f. gen8_upload_ps_extra().
          *
          * BRW_NEW_FRAGMENT_PROGRAM | BRW_NEW_FS_PROG_DATA | _NEW_BUFFERS |
          * _NEW_COLOR
          */
#if GFX_VERx10 == 75
         if (!(has_writeable_rt(ice->state.cso_blend, fs_info) || writes_depth) &&
             wm_prog_data->has_side_effects)
            wm.PSUAVonly = ON;
#endif
#endif
#if GFX_VER >= 7
      /* BRW_NEW_FS_PROG_DATA */
         if (wm_prog_data->early_fragment_tests)
           wm.EarlyDepthStencilControl = EDSC_PREPS;
         else if (wm_prog_data->has_side_effects)
           wm.EarlyDepthStencilControl = EDSC_PSEXEC;
#endif
#if GFX_VER == 8
         /* We could skip this bit if color writes are enabled. */
         if (wm_prog_data->has_side_effects || wm_prog_data->uses_kill)
            wm.ForceThreadDispatchEnable = ForceON;
#endif
      };

#if GFX_VER <= 5
      if (ice->state.global_depth_offset_clamp != cso->cso.offset_clamp) {
         crocus_emit_cmd(batch, GENX(3DSTATE_GLOBAL_DEPTH_OFFSET_CLAMP), clamp) {
            clamp.GlobalDepthOffsetClamp = cso->cso.offset_clamp;
         }
         ice->state.global_depth_offset_clamp = cso->cso.offset_clamp;
      }
#endif
   }

#if GFX_VER >= 7
   if (dirty & CROCUS_DIRTY_GEN7_SBE) {
      crocus_emit_sbe(batch, ice);
   }
#endif

#if GFX_VER >= 8
   if (dirty & CROCUS_DIRTY_GEN8_PS_BLEND) {
      struct crocus_compiled_shader *shader = ice->shaders.prog[MESA_SHADER_FRAGMENT];
      struct crocus_blend_state *cso_blend = ice->state.cso_blend;
      struct crocus_depth_stencil_alpha_state *cso_zsa = ice->state.cso_zsa;
      struct brw_wm_prog_data *wm_prog_data = (void *) shader->prog_data;
      const struct shader_info *fs_info =
         crocus_get_shader_info(ice, MESA_SHADER_FRAGMENT);
      uint32_t dynamic_pb[GENX(3DSTATE_PS_BLEND_length)];
      crocus_pack_command(GENX(3DSTATE_PS_BLEND), &dynamic_pb, pb) {
         pb.HasWriteableRT = has_writeable_rt(cso_blend, fs_info);
         pb.AlphaTestEnable = cso_zsa->cso.alpha_enabled;
         pb.ColorBufferBlendEnable = (cso_blend->blend_enables & 1) &&
            (!cso_blend->dual_color_blending || wm_prog_data->dual_src_blend);
      }
      crocus_emit_merge(batch, cso_blend->ps_blend, dynamic_pb,
                        ARRAY_SIZE(cso_blend->ps_blend));
   }
#endif

#if GFX_VER >= 6
   if (dirty & CROCUS_DIRTY_GEN6_WM_DEPTH_STENCIL) {

#if GFX_VER >= 8
      crocus_emit_cmd(batch, GENX(3DSTATE_WM_DEPTH_STENCIL), wmds) {
         set_depth_stencil_bits(ice, &wmds);
      }
#else
      uint32_t ds_offset;
      void *ds_map = stream_state(batch,
                                  sizeof(uint32_t) * GENX(DEPTH_STENCIL_STATE_length),
                                  64, &ds_offset);
      _crocus_pack_state(batch, GENX(DEPTH_STENCIL_STATE), ds_map, ds) {
         set_depth_stencil_bits(ice, &ds);
      }

#if GFX_VER == 6
      crocus_emit_cmd(batch, GENX(3DSTATE_CC_STATE_POINTERS), ptr) {
         ptr.PointertoDEPTH_STENCIL_STATE = ds_offset;
         ptr.DEPTH_STENCIL_STATEChange = true;
      }
#else
      crocus_emit_cmd(batch, GENX(3DSTATE_DEPTH_STENCIL_STATE_POINTERS), ptr) {
         ptr.PointertoDEPTH_STENCIL_STATE = ds_offset;
      }
#endif
#endif
   }

   if (dirty & CROCUS_DIRTY_GEN6_SCISSOR_RECT) {
      /* Align to 64-byte boundary as per anv. */
      uint32_t scissor_offset;
      struct pipe_scissor_state *scissor_map = (void *)
         stream_state(batch, sizeof(struct pipe_scissor_state) * ice->state.num_viewports,
                      64, &scissor_offset);
      for (int i = 0; i < ice->state.num_viewports; i++) {
         struct pipe_scissor_state scissor;
         crocus_fill_scissor_rect(ice, i, &scissor);
         scissor_map[i] = scissor;
      }

      crocus_emit_cmd(batch, GENX(3DSTATE_SCISSOR_STATE_POINTERS), ptr) {
         ptr.ScissorRectPointer = scissor_offset;
      }
   }
#endif

   if (dirty & CROCUS_DIRTY_DEPTH_BUFFER) {
      struct isl_device *isl_dev = &batch->screen->isl_dev;
#if GFX_VER >= 6
      crocus_emit_depth_stall_flushes(batch);
#endif
      void *batch_ptr;
      struct crocus_resource *zres, *sres;
      struct pipe_framebuffer_state *cso = &ice->state.framebuffer;
      batch_ptr = crocus_get_command_space(batch, isl_dev->ds.size);

      struct isl_view view = {
                              .base_level = 0,
                              .levels = 1,
                              .base_array_layer = 0,
                              .array_len = 1,
                              .swizzle = ISL_SWIZZLE_IDENTITY,
      };
      struct isl_depth_stencil_hiz_emit_info info = {
         .view = &view,
         .mocs = crocus_mocs(NULL, isl_dev),
      };

      if (cso->zsbuf) {
         crocus_get_depth_stencil_resources(&batch->screen->devinfo, cso->zsbuf->texture, &zres, &sres);
         struct crocus_surface *zsbuf = (struct crocus_surface *)cso->zsbuf;
         if (zsbuf->align_res) {
            zres = (struct crocus_resource *)zsbuf->align_res;
         }
         view.base_level = cso->zsbuf->u.tex.level;
         view.base_array_layer = cso->zsbuf->u.tex.first_layer;
         view.array_len = cso->zsbuf->u.tex.last_layer - cso->zsbuf->u.tex.first_layer + 1;

         if (zres) {
            view.usage |= ISL_SURF_USAGE_DEPTH_BIT;

            info.depth_surf = &zres->surf;
            info.depth_address = crocus_command_reloc(batch,
                                                      (batch_ptr - batch->command.map) + isl_dev->ds.depth_offset,
                                                      zres->bo, 0, RELOC_32BIT);

            info.mocs = crocus_mocs(zres->bo, isl_dev);
            view.format = zres->surf.format;

            if (crocus_resource_level_has_hiz(zres, view.base_level)) {
               info.hiz_usage = zres->aux.usage;
               info.hiz_surf = &zres->aux.surf;
               uint64_t hiz_offset = 0;

#if GFX_VER == 6
               /* HiZ surfaces on Sandy Bridge technically don't support
                * mip-mapping.  However, we can fake it by offsetting to the
                * first slice of LOD0 in the HiZ surface.
                */
               isl_surf_get_image_offset_B_tile_sa(&zres->aux.surf,
                                                   view.base_level, 0, 0,
                                                   &hiz_offset, NULL, NULL);
#endif
               info.hiz_address = crocus_command_reloc(batch,
                                                       (batch_ptr - batch->command.map) + isl_dev->ds.hiz_offset,
                                                       zres->aux.bo, zres->aux.offset + hiz_offset,
                                                       RELOC_32BIT);
               info.depth_clear_value = crocus_resource_get_clear_color(zres).f32[0];
            }
         }

#if GFX_VER >= 6
         if (sres) {
            view.usage |= ISL_SURF_USAGE_STENCIL_BIT;
            info.stencil_aux_usage = sres->aux.usage;
            info.stencil_surf = &sres->surf;

            uint64_t stencil_offset = 0;
#if GFX_VER == 6
            /* Stencil surfaces on Sandy Bridge technically don't support
             * mip-mapping.  However, we can fake it by offsetting to the
             * first slice of LOD0 in the stencil surface.
             */
            isl_surf_get_image_offset_B_tile_sa(&sres->surf,
                                                view.base_level, 0, 0,
                                                &stencil_offset, NULL, NULL);
#endif

            info.stencil_address = crocus_command_reloc(batch,
                                                        (batch_ptr - batch->command.map) + isl_dev->ds.stencil_offset,
                                                        sres->bo, stencil_offset, RELOC_32BIT);
            if (!zres) {
               view.format = sres->surf.format;
               info.mocs = crocus_mocs(sres->bo, isl_dev);
            }
         }
#endif
      }
      isl_emit_depth_stencil_hiz_s(isl_dev, batch_ptr, &info);
   }

   /* TODO: Disable emitting this until something uses a stipple. */
   if (dirty & CROCUS_DIRTY_POLYGON_STIPPLE) {
      crocus_emit_cmd(batch, GENX(3DSTATE_POLY_STIPPLE_PATTERN), poly) {
         for (int i = 0; i < 32; i++) {
            poly.PatternRow[i] = ice->state.poly_stipple.stipple[i];
         }
      }
   }

   if (dirty & CROCUS_DIRTY_LINE_STIPPLE) {
      struct crocus_rasterizer_state *cso = ice->state.cso_rast;
      crocus_batch_emit(batch, cso->line_stipple, sizeof(cso->line_stipple));
   }

#if GFX_VER >= 8
   if (dirty & CROCUS_DIRTY_GEN8_VF_TOPOLOGY) {
      crocus_emit_cmd(batch, GENX(3DSTATE_VF_TOPOLOGY), topo) {
         topo.PrimitiveTopologyType =
            translate_prim_type(draw->mode, ice->state.patch_vertices);
      }
   }
#endif

#if GFX_VER <= 5
   if (dirty & CROCUS_DIRTY_GEN5_PIPELINED_POINTERS) {
      upload_pipelined_state_pointers(batch, ice->shaders.ff_gs_prog ? true : false, ice->shaders.gs_offset,
                                      ice->shaders.vs_offset, ice->shaders.sf_offset,
                                      ice->shaders.clip_offset, ice->shaders.wm_offset, ice->shaders.cc_offset);
      crocus_upload_urb_fence(batch);

      crocus_emit_cmd(batch, GENX(CS_URB_STATE), cs) {
        cs.NumberofURBEntries = ice->urb.nr_cs_entries;
        cs.URBEntryAllocationSize = ice->urb.csize - 1;
      }
      dirty |= CROCUS_DIRTY_GEN4_CURBE;
   }
#endif
   if (dirty & CROCUS_DIRTY_DRAWING_RECTANGLE) {
      struct pipe_framebuffer_state *fb = &ice->state.framebuffer;
      if (fb->width && fb->height) {
         crocus_emit_cmd(batch, GENX(3DSTATE_DRAWING_RECTANGLE), rect) {
            rect.ClippedDrawingRectangleXMax = fb->width - 1;
            rect.ClippedDrawingRectangleYMax = fb->height - 1;
         }
      }
   }

   if (dirty & CROCUS_DIRTY_VERTEX_BUFFERS) {
      const uint32_t user_count = util_bitcount(ice->state.bound_vertex_buffers);
      const uint32_t count = user_count +
         ice->state.vs_uses_draw_params + ice->state.vs_uses_derived_draw_params;
      uint32_t dynamic_bound = ice->state.bound_vertex_buffers;

      if (count) {
         const unsigned vb_dwords = GENX(VERTEX_BUFFER_STATE_length);

         uint32_t *map =
            crocus_get_command_space(batch, 4 * (1 + vb_dwords * count));
         _crocus_pack_command(batch, GENX(3DSTATE_VERTEX_BUFFERS), map, vb) {
            vb.DWordLength = (vb_dwords * count + 1) - 2;
         }
         map += 1;

         uint32_t bound = dynamic_bound;
         int i;
         while (bound) {
            i = u_bit_scan(&bound);
            struct pipe_vertex_buffer *buf = &ice->state.vertex_buffers[i];
            struct crocus_bo *bo = crocus_resource_bo(buf->buffer.resource);
            uint32_t step_rate = ice->state.cso_vertex_elements->step_rate[i];

            emit_vertex_buffer_state(batch, i, bo,
                                     buf->buffer_offset,
                                     ice->state.vb_end[i],
                                     ice->state.cso_vertex_elements->strides[i],
                                     step_rate,
                                     &map);
         }
         i = user_count;
         if (ice->state.vs_uses_draw_params) {
            struct crocus_resource *res = (struct crocus_resource *)ice->draw.draw_params.res;
            emit_vertex_buffer_state(batch, i++,
                                     res->bo,
                                     ice->draw.draw_params.offset,
                                     ice->draw.draw_params.res->width0,
                                     0, 0, &map);
         }
         if (ice->state.vs_uses_derived_draw_params) {
            struct crocus_resource *res = (struct crocus_resource *)ice->draw.derived_draw_params.res;
            emit_vertex_buffer_state(batch, i++,
                                     res->bo,
                                     ice->draw.derived_draw_params.offset,
                                     ice->draw.derived_draw_params.res->width0,
                                     0, 0, &map);
         }
      }
   }

   if (dirty & CROCUS_DIRTY_VERTEX_ELEMENTS) {
      struct crocus_vertex_element_state *cso = ice->state.cso_vertex_elements;
      const unsigned entries = MAX2(cso->count, 1);
      if (!(ice->state.vs_needs_sgvs_element ||
            ice->state.vs_uses_derived_draw_params ||
            ice->state.vs_needs_edge_flag)) {
         crocus_batch_emit(batch, cso->vertex_elements, sizeof(uint32_t) *
                         (1 + entries * GENX(VERTEX_ELEMENT_STATE_length)));
      } else {
         uint32_t dynamic_ves[1 + 33 * GENX(VERTEX_ELEMENT_STATE_length)];
         const unsigned dyn_count = cso->count +
            ice->state.vs_needs_sgvs_element +
            ice->state.vs_uses_derived_draw_params;

         crocus_pack_command(GENX(3DSTATE_VERTEX_ELEMENTS),
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
            crocus_pack_state(GENX(VERTEX_ELEMENT_STATE), ve_pack_dest, ve) {
               ve.Valid = true;
               ve.VertexBufferIndex =
                  util_bitcount64(ice->state.bound_vertex_buffers);
               ve.SourceElementFormat = ISL_FORMAT_R32G32_UINT;
               ve.Component0Control = base_ctrl;
               ve.Component1Control = base_ctrl;
#if GFX_VER < 8
               ve.Component2Control = ice->state.vs_uses_vertexid ? VFCOMP_STORE_VID : VFCOMP_STORE_0;
               ve.Component3Control = ice->state.vs_uses_instanceid ? VFCOMP_STORE_IID : VFCOMP_STORE_0;
#else
               ve.Component2Control = VFCOMP_STORE_0;
               ve.Component3Control = VFCOMP_STORE_0;
#endif
#if GFX_VER < 5
               ve.DestinationElementOffset = cso->count * 4;
#endif
            }
            ve_pack_dest += GENX(VERTEX_ELEMENT_STATE_length);
         }
         if (ice->state.vs_uses_derived_draw_params) {
            crocus_pack_state(GENX(VERTEX_ELEMENT_STATE), ve_pack_dest, ve) {
               ve.Valid = true;
               ve.VertexBufferIndex =
                  util_bitcount64(ice->state.bound_vertex_buffers) +
                  ice->state.vs_uses_draw_params;
               ve.SourceElementFormat = ISL_FORMAT_R32G32_UINT;
               ve.Component0Control = VFCOMP_STORE_SRC;
               ve.Component1Control = VFCOMP_STORE_SRC;
               ve.Component2Control = VFCOMP_STORE_0;
               ve.Component3Control = VFCOMP_STORE_0;
#if GFX_VER < 5
               ve.DestinationElementOffset = (cso->count + ice->state.vs_needs_sgvs_element) * 4;
#endif
            }
            ve_pack_dest += GENX(VERTEX_ELEMENT_STATE_length);
         }
         if (ice->state.vs_needs_edge_flag) {
            for (int i = 0; i < GENX(VERTEX_ELEMENT_STATE_length);  i++)
               ve_pack_dest[i] = cso->edgeflag_ve[i];
         }

         crocus_batch_emit(batch, &dynamic_ves, sizeof(uint32_t) *
                         (1 + dyn_count * GENX(VERTEX_ELEMENT_STATE_length)));
      }

#if GFX_VER == 8
      if (!ice->state.vs_needs_edge_flag) {
         crocus_batch_emit(batch, cso->vf_instancing, sizeof(uint32_t) *
                         entries * GENX(3DSTATE_VF_INSTANCING_length));
      } else {
         assert(cso->count > 0);
         const unsigned edgeflag_index = cso->count - 1;
         uint32_t dynamic_vfi[33 * GENX(3DSTATE_VF_INSTANCING_length)];
         memcpy(&dynamic_vfi[0], cso->vf_instancing, edgeflag_index *
                GENX(3DSTATE_VF_INSTANCING_length) * sizeof(uint32_t));

         uint32_t *vfi_pack_dest = &dynamic_vfi[0] +
            edgeflag_index * GENX(3DSTATE_VF_INSTANCING_length);
         crocus_pack_command(GENX(3DSTATE_VF_INSTANCING), vfi_pack_dest, vi) {
            vi.VertexElementIndex = edgeflag_index +
               ice->state.vs_needs_sgvs_element +
               ice->state.vs_uses_derived_draw_params;
         }
         for (int i = 0; i < GENX(3DSTATE_VF_INSTANCING_length);  i++)
            vfi_pack_dest[i] |= cso->edgeflag_vfi[i];

         crocus_batch_emit(batch, &dynamic_vfi[0], sizeof(uint32_t) *
                         entries * GENX(3DSTATE_VF_INSTANCING_length));
      }
#endif
   }

#if GFX_VER == 8
   if (dirty & CROCUS_DIRTY_GEN8_VF_SGVS) {
      const struct brw_vs_prog_data *vs_prog_data = (void *)
         ice->shaders.prog[MESA_SHADER_VERTEX]->prog_data;
      struct crocus_vertex_element_state *cso = ice->state.cso_vertex_elements;

      crocus_emit_cmd(batch, GENX(3DSTATE_VF_SGVS), sgv) {
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
#endif
#if GFX_VERx10 >= 75
   if (dirty & CROCUS_DIRTY_GEN75_VF) {
      crocus_emit_cmd(batch, GENX(3DSTATE_VF), vf) {
         if (draw->primitive_restart) {
            vf.IndexedDrawCutIndexEnable = true;
            vf.CutIndex = draw->restart_index;
         }
      }
   }
#endif

#if GFX_VER == 8
   if (dirty & CROCUS_DIRTY_GEN8_PMA_FIX) {
      bool enable = want_pma_fix(ice);
      genX(crocus_update_pma_fix)(ice, batch, enable);
   }
#endif

#if GFX_VER <= 5
   if (dirty & CROCUS_DIRTY_GEN4_CURBE) {
      gen4_upload_curbe(batch);
   }
#endif
}

static void
crocus_upload_render_state(struct crocus_context *ice,
                           struct crocus_batch *batch,
                           const struct pipe_draw_info *draw,
                           unsigned drawid_offset,
                           const struct pipe_draw_indirect_info *indirect,
                           const struct pipe_draw_start_count_bias *sc)
{
#if GFX_VER >= 7
   bool use_predicate = ice->state.predicate == CROCUS_PREDICATE_STATE_USE_BIT;
#endif

   batch->no_wrap = true;
   batch->contains_draw = true;

   crocus_update_surface_base_address(batch);

   crocus_upload_dirty_render_state(ice, batch, draw);

   batch->no_wrap = false;
   if (draw->index_size > 0) {
      unsigned offset;
      unsigned size;
      bool emit_index = false;

      if (draw->has_user_indices) {
         unsigned start_offset = draw->index_size * sc->start;
         u_upload_data(ice->ctx.stream_uploader, 0,
                       sc->count * draw->index_size, 4,
                       (char *)draw->index.user + start_offset,
                       &offset, &ice->state.index_buffer.res);
         offset -= start_offset;
         size = start_offset + sc->count * draw->index_size;
         emit_index = true;
      } else {
         struct crocus_resource *res = (void *) draw->index.resource;

         if (ice->state.index_buffer.res != draw->index.resource) {
            res->bind_history |= PIPE_BIND_INDEX_BUFFER;
            pipe_resource_reference(&ice->state.index_buffer.res,
                                    draw->index.resource);
            emit_index = true;
         }
         offset = 0;
         size = draw->index.resource->width0;
      }

      if (!emit_index &&
          (ice->state.index_buffer.size != size ||
           ice->state.index_buffer.index_size != draw->index_size
#if GFX_VERx10 < 75
           || ice->state.index_buffer.prim_restart != draw->primitive_restart
#endif
	   )
	  )
         emit_index = true;

      if (emit_index) {
         struct crocus_bo *bo = crocus_resource_bo(ice->state.index_buffer.res);

         crocus_emit_cmd(batch, GENX(3DSTATE_INDEX_BUFFER), ib) {
#if GFX_VERx10 < 75
            ib.CutIndexEnable = draw->primitive_restart;
#endif
            ib.IndexFormat = draw->index_size >> 1;
            ib.BufferStartingAddress = ro_bo(bo, offset);
#if GFX_VER >= 8
            ib.BufferSize = bo->size - offset;
#else
            ib.BufferEndingAddress = ro_bo(bo, offset + size - 1);
#endif
#if GFX_VER >= 6
            ib.MOCS = crocus_mocs(bo, &batch->screen->isl_dev);
#endif
         }
         ice->state.index_buffer.size = size;
         ice->state.index_buffer.offset = offset;
         ice->state.index_buffer.index_size = draw->index_size;
#if GFX_VERx10 < 75
         ice->state.index_buffer.prim_restart = draw->primitive_restart;
#endif
      }
   }

#define _3DPRIM_END_OFFSET          0x2420
#define _3DPRIM_START_VERTEX        0x2430
#define _3DPRIM_VERTEX_COUNT        0x2434
#define _3DPRIM_INSTANCE_COUNT      0x2438
#define _3DPRIM_START_INSTANCE      0x243C
#define _3DPRIM_BASE_VERTEX         0x2440

#if GFX_VER >= 7
   if (indirect && !indirect->count_from_stream_output) {
      if (indirect->indirect_draw_count) {
         use_predicate = true;

         struct crocus_bo *draw_count_bo =
            crocus_resource_bo(indirect->indirect_draw_count);
         unsigned draw_count_offset =
            indirect->indirect_draw_count_offset;

         crocus_emit_pipe_control_flush(batch,
                                        "ensure indirect draw buffer is flushed",
                                        PIPE_CONTROL_FLUSH_ENABLE);
         if (ice->state.predicate == CROCUS_PREDICATE_STATE_USE_BIT) {
#if GFX_VERx10 >= 75
            struct mi_builder b;
            mi_builder_init(&b, &batch->screen->devinfo, batch);

            /* comparison = draw id < draw count */
            struct mi_value comparison =
               mi_ult(&b, mi_imm(drawid_offset),
                      mi_mem32(ro_bo(draw_count_bo,
                                     draw_count_offset)));
#if GFX_VER == 8
            /* predicate = comparison & conditional rendering predicate */
            mi_store(&b, mi_reg32(MI_PREDICATE_RESULT),
                         mi_iand(&b, comparison, mi_reg32(CS_GPR(15))));
#else
            /* predicate = comparison & conditional rendering predicate */
            struct mi_value pred = mi_iand(&b, comparison,
                                           mi_reg32(CS_GPR(15)));

            mi_store(&b, mi_reg64(MI_PREDICATE_SRC0), pred);
            mi_store(&b, mi_reg64(MI_PREDICATE_SRC1), mi_imm(0));

            unsigned mi_predicate = MI_PREDICATE | MI_PREDICATE_LOADOP_LOADINV |
               MI_PREDICATE_COMBINEOP_SET |
               MI_PREDICATE_COMPAREOP_SRCS_EQUAL;

            crocus_batch_emit(batch, &mi_predicate, sizeof(uint32_t));
#endif
#endif
         } else {
            uint32_t mi_predicate;

            /* Upload the id of the current primitive to MI_PREDICATE_SRC1. */
            crocus_load_register_imm64(batch, MI_PREDICATE_SRC1, drawid_offset);
            /* Upload the current draw count from the draw parameters buffer
             * to MI_PREDICATE_SRC0.
             */
            crocus_load_register_mem32(batch, MI_PREDICATE_SRC0,
                                       draw_count_bo, draw_count_offset);
            /* Zero the top 32-bits of MI_PREDICATE_SRC0 */
            crocus_load_register_imm32(batch, MI_PREDICATE_SRC0 + 4, 0);

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
            crocus_batch_emit(batch, &mi_predicate, sizeof(uint32_t));
         }
      }

#if GFX_VER >= 7
      struct crocus_bo *bo = crocus_resource_bo(indirect->buffer);
      assert(bo);

      crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
         lrm.RegisterAddress = _3DPRIM_VERTEX_COUNT;
         lrm.MemoryAddress = ro_bo(bo, indirect->offset + 0);
      }
      crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
         lrm.RegisterAddress = _3DPRIM_INSTANCE_COUNT;
         lrm.MemoryAddress = ro_bo(bo, indirect->offset + 4);
      }
      crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
         lrm.RegisterAddress = _3DPRIM_START_VERTEX;
         lrm.MemoryAddress = ro_bo(bo, indirect->offset + 8);
      }
      if (draw->index_size) {
         crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
            lrm.RegisterAddress = _3DPRIM_BASE_VERTEX;
            lrm.MemoryAddress = ro_bo(bo, indirect->offset + 12);
         }
         crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
            lrm.RegisterAddress = _3DPRIM_START_INSTANCE;
            lrm.MemoryAddress = ro_bo(bo, indirect->offset + 16);
         }
      } else {
         crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
            lrm.RegisterAddress = _3DPRIM_START_INSTANCE;
            lrm.MemoryAddress = ro_bo(bo, indirect->offset + 12);
         }
         crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_IMM), lri) {
            lri.RegisterOffset = _3DPRIM_BASE_VERTEX;
            lri.DataDWord = 0;
         }
      }
#endif
   } else if (indirect && indirect->count_from_stream_output) {
#if GFX_VERx10 >= 75
      struct crocus_stream_output_target *so =
         (void *) indirect->count_from_stream_output;

      /* XXX: Replace with actual cache tracking */
      crocus_emit_pipe_control_flush(batch,
                                     "draw count from stream output stall",
                                     PIPE_CONTROL_CS_STALL);

      struct mi_builder b;
      mi_builder_init(&b, &batch->screen->devinfo, batch);

      struct crocus_address addr =
         ro_bo(crocus_resource_bo(&so->offset_res->base.b), so->offset_offset);
      struct mi_value offset =
         mi_iadd_imm(&b, mi_mem32(addr), -so->base.buffer_offset);

      mi_store(&b, mi_reg32(_3DPRIM_VERTEX_COUNT),
               mi_udiv32_imm(&b, offset, so->stride));

      _crocus_emit_lri(batch, _3DPRIM_START_VERTEX, 0);
      _crocus_emit_lri(batch, _3DPRIM_BASE_VERTEX, 0);
      _crocus_emit_lri(batch, _3DPRIM_START_INSTANCE, 0);
      _crocus_emit_lri(batch, _3DPRIM_INSTANCE_COUNT, draw->instance_count);
#endif
   }
#else
   assert(!indirect);
#endif

   crocus_emit_cmd(batch, GENX(3DPRIMITIVE), prim) {
      prim.VertexAccessType = draw->index_size > 0 ? RANDOM : SEQUENTIAL;
#if GFX_VER >= 7
      prim.PredicateEnable = use_predicate;
#endif

      prim.PrimitiveTopologyType = translate_prim_type(ice->state.prim_mode, ice->state.patch_vertices);
      if (indirect) {
         // XXX Probably have to do something for gen6 here?
#if GFX_VER >= 7
         prim.IndirectParameterEnable = true;
#endif
      } else {
#if GFX_VER >= 5
         prim.StartInstanceLocation = draw->start_instance;
#endif
         prim.InstanceCount = draw->instance_count;
         prim.VertexCountPerInstance = sc->count;

         prim.StartVertexLocation = sc->start;

         if (draw->index_size) {
            prim.BaseVertexLocation += sc->index_bias;
         }
      }
   }
}

#if GFX_VER >= 7

static void
crocus_upload_compute_state(struct crocus_context *ice,
                            struct crocus_batch *batch,
                            const struct pipe_grid_info *grid)
{
   const uint64_t stage_dirty = ice->state.stage_dirty;
   struct crocus_screen *screen = batch->screen;
   const struct intel_device_info *devinfo = &screen->devinfo;
   struct crocus_shader_state *shs = &ice->state.shaders[MESA_SHADER_COMPUTE];
   struct crocus_compiled_shader *shader =
      ice->shaders.prog[MESA_SHADER_COMPUTE];
   struct brw_stage_prog_data *prog_data = shader->prog_data;
   struct brw_cs_prog_data *cs_prog_data = (void *) prog_data;
   const struct brw_cs_dispatch_info dispatch =
      brw_cs_get_dispatch_info(devinfo, cs_prog_data, grid->block);

   crocus_update_surface_base_address(batch);
   if ((stage_dirty & CROCUS_STAGE_DIRTY_CONSTANTS_CS) && shs->sysvals_need_upload)
      upload_sysvals(ice, MESA_SHADER_COMPUTE);

   if (stage_dirty & CROCUS_STAGE_DIRTY_BINDINGS_CS) {
      crocus_populate_binding_table(ice, batch, MESA_SHADER_COMPUTE, false);
      ice->shaders.prog[MESA_SHADER_COMPUTE]->bind_bo_offset =
         crocus_upload_binding_table(ice, batch,
                                     ice->shaders.prog[MESA_SHADER_COMPUTE]->surf_offset,
                                     ice->shaders.prog[MESA_SHADER_COMPUTE]->bt.size_bytes);
   }

   if (stage_dirty & CROCUS_STAGE_DIRTY_SAMPLER_STATES_CS)
      crocus_upload_sampler_states(ice, batch, MESA_SHADER_COMPUTE);

   if ((stage_dirty & CROCUS_STAGE_DIRTY_CS) ||
       cs_prog_data->local_size[0] == 0 /* Variable local group size */) {
      /* The MEDIA_VFE_STATE documentation for Gen8+ says:
       *
       *   "A stalling PIPE_CONTROL is required before MEDIA_VFE_STATE unless
       *    the only bits that are changed are scoreboard related: Scoreboard
       *    Enable, Scoreboard Type, Scoreboard Mask, Scoreboard Delta.  For
       *    these scoreboard related states, a MEDIA_STATE_FLUSH is
       *    sufficient."
       */
      crocus_emit_pipe_control_flush(batch,
                                     "workaround: stall before MEDIA_VFE_STATE",
                                     PIPE_CONTROL_CS_STALL);

      crocus_emit_cmd(batch, GENX(MEDIA_VFE_STATE), vfe) {
         if (prog_data->total_scratch) {
            struct crocus_bo *bo =
               crocus_get_scratch_space(ice, prog_data->total_scratch,
                                        MESA_SHADER_COMPUTE);
#if GFX_VER == 8
            /* Broadwell's Per Thread Scratch Space is in the range [0, 11]
             * where 0 = 1k, 1 = 2k, 2 = 4k, ..., 11 = 2M.
             */
            vfe.PerThreadScratchSpace = ffs(prog_data->total_scratch) - 11;
#elif GFX_VERx10 == 75
            /* Haswell's Per Thread Scratch Space is in the range [0, 10]
             * where 0 = 2k, 1 = 4k, 2 = 8k, ..., 10 = 2M.
             */
            vfe.PerThreadScratchSpace = ffs(prog_data->total_scratch) - 12;
#else
            /* Earlier platforms use the range [0, 11] to mean [1kB, 12kB]
             * where 0 = 1kB, 1 = 2kB, 2 = 3kB, ..., 11 = 12kB.
             */
            vfe.PerThreadScratchSpace = prog_data->total_scratch / 1024 - 1;
#endif
            vfe.ScratchSpaceBasePointer = rw_bo(bo, 0);
         }

         vfe.MaximumNumberofThreads =
            devinfo->max_cs_threads * devinfo->subslice_total - 1;
         vfe.ResetGatewayTimer =
            Resettingrelativetimerandlatchingtheglobaltimestamp;
         vfe.BypassGatewayControl = true;
#if GFX_VER == 7
         vfe.GPGPUMode = true;
#endif
#if GFX_VER == 8
         vfe.BypassGatewayControl = true;
#endif
         vfe.NumberofURBEntries = GFX_VER == 8 ? 2 : 0;
         vfe.URBEntryAllocationSize = GFX_VER == 8 ? 2 : 0;

         vfe.CURBEAllocationSize =
            ALIGN(cs_prog_data->push.per_thread.regs * dispatch.threads +
                  cs_prog_data->push.cross_thread.regs, 2);
      }
   }

   /* TODO: Combine subgroup-id with cbuf0 so we can push regular uniforms */
   if ((stage_dirty & CROCUS_STAGE_DIRTY_CS) ||
       cs_prog_data->local_size[0] == 0 /* Variable local group size */) {
      uint32_t curbe_data_offset = 0;
      assert(cs_prog_data->push.cross_thread.dwords == 0 &&
             cs_prog_data->push.per_thread.dwords == 1 &&
             cs_prog_data->base.param[0] == BRW_PARAM_BUILTIN_SUBGROUP_ID);
      const unsigned push_const_size =
         brw_cs_push_const_total_size(cs_prog_data, dispatch.threads);
      uint32_t *curbe_data_map =
         stream_state(batch,
                      ALIGN(push_const_size, 64), 64,
                      &curbe_data_offset);
      assert(curbe_data_map);
      memset(curbe_data_map, 0x5a, ALIGN(push_const_size, 64));
      crocus_fill_cs_push_const_buffer(cs_prog_data, dispatch.threads,
                                       curbe_data_map);

      crocus_emit_cmd(batch, GENX(MEDIA_CURBE_LOAD), curbe) {
         curbe.CURBETotalDataLength = ALIGN(push_const_size, 64);
         curbe.CURBEDataStartAddress = curbe_data_offset;
      }
   }

   if (stage_dirty & (CROCUS_STAGE_DIRTY_SAMPLER_STATES_CS |
                      CROCUS_STAGE_DIRTY_BINDINGS_CS |
                      CROCUS_STAGE_DIRTY_CONSTANTS_CS |
                      CROCUS_STAGE_DIRTY_CS)) {
      uint32_t desc[GENX(INTERFACE_DESCRIPTOR_DATA_length)];
      const uint64_t ksp = KSP(ice,shader) + brw_cs_prog_data_prog_offset(cs_prog_data, dispatch.simd_size);
      crocus_pack_state(GENX(INTERFACE_DESCRIPTOR_DATA), desc, idd) {
         idd.KernelStartPointer = ksp;
         idd.SamplerStatePointer = shs->sampler_offset;
         idd.BindingTablePointer = ice->shaders.prog[MESA_SHADER_COMPUTE]->bind_bo_offset;
         idd.BindingTableEntryCount = MIN2(shader->bt.size_bytes / 4, 31);
         idd.NumberofThreadsinGPGPUThreadGroup = dispatch.threads;
         idd.ConstantURBEntryReadLength = cs_prog_data->push.per_thread.regs;
         idd.BarrierEnable = cs_prog_data->uses_barrier;
         idd.SharedLocalMemorySize = encode_slm_size(GFX_VER,
                                                     prog_data->total_shared);
#if GFX_VERx10 >= 75
         idd.CrossThreadConstantDataReadLength = cs_prog_data->push.cross_thread.regs;
#endif
      }

      crocus_emit_cmd(batch, GENX(MEDIA_INTERFACE_DESCRIPTOR_LOAD), load) {
         load.InterfaceDescriptorTotalLength =
            GENX(INTERFACE_DESCRIPTOR_DATA_length) * sizeof(uint32_t);
         load.InterfaceDescriptorDataStartAddress =
            emit_state(batch, desc, sizeof(desc), 64);
      }
   }

#define GPGPU_DISPATCHDIMX 0x2500
#define GPGPU_DISPATCHDIMY 0x2504
#define GPGPU_DISPATCHDIMZ 0x2508

   if (grid->indirect) {
      struct crocus_state_ref *grid_size = &ice->state.grid_size;
      struct crocus_bo *bo = crocus_resource_bo(grid_size->res);
      crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
         lrm.RegisterAddress = GPGPU_DISPATCHDIMX;
         lrm.MemoryAddress = ro_bo(bo, grid_size->offset + 0);
      }
      crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
         lrm.RegisterAddress = GPGPU_DISPATCHDIMY;
         lrm.MemoryAddress = ro_bo(bo, grid_size->offset + 4);
      }
      crocus_emit_cmd(batch, GENX(MI_LOAD_REGISTER_MEM), lrm) {
         lrm.RegisterAddress = GPGPU_DISPATCHDIMZ;
         lrm.MemoryAddress = ro_bo(bo, grid_size->offset + 8);
      }

#if GFX_VER == 7
      /* Clear upper 32-bits of SRC0 and all 64-bits of SRC1 */
      _crocus_emit_lri(batch, MI_PREDICATE_SRC0 + 4, 0);
      crocus_load_register_imm64(batch, MI_PREDICATE_SRC1, 0);

      /* Load compute_dispatch_indirect_x_size into SRC0 */
      crocus_load_register_mem32(batch, MI_PREDICATE_SRC0, bo, grid_size->offset + 0);

      /* predicate = (compute_dispatch_indirect_x_size == 0); */
      crocus_emit_cmd(batch, GENX(MI_PREDICATE), mip) {
         mip.LoadOperation    = LOAD_LOAD;
         mip.CombineOperation = COMBINE_SET;
         mip.CompareOperation = COMPARE_SRCS_EQUAL;
      };

      /* Load compute_dispatch_indirect_y_size into SRC0 */
      crocus_load_register_mem32(batch, MI_PREDICATE_SRC0, bo, grid_size->offset + 4);

      /* predicate = (compute_dispatch_indirect_y_size == 0); */
      crocus_emit_cmd(batch, GENX(MI_PREDICATE), mip) {
         mip.LoadOperation    = LOAD_LOAD;
         mip.CombineOperation = COMBINE_OR;
         mip.CompareOperation = COMPARE_SRCS_EQUAL;
      };

      /* Load compute_dispatch_indirect_z_size into SRC0 */
      crocus_load_register_mem32(batch, MI_PREDICATE_SRC0, bo, grid_size->offset + 8);

      /* predicate = (compute_dispatch_indirect_z_size == 0); */
      crocus_emit_cmd(batch, GENX(MI_PREDICATE), mip) {
         mip.LoadOperation    = LOAD_LOAD;
         mip.CombineOperation = COMBINE_OR;
         mip.CompareOperation = COMPARE_SRCS_EQUAL;
      };

      /* predicate = !predicate; */
#define COMPARE_FALSE                           1
      crocus_emit_cmd(batch, GENX(MI_PREDICATE), mip) {
         mip.LoadOperation    = LOAD_LOADINV;
         mip.CombineOperation = COMBINE_OR;
         mip.CompareOperation = COMPARE_FALSE;
      }
#endif
   }

   crocus_emit_cmd(batch, GENX(GPGPU_WALKER), ggw) {
      ggw.IndirectParameterEnable    = grid->indirect != NULL;
      ggw.PredicateEnable            = GFX_VER <= 7 && grid->indirect != NULL;
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

   crocus_emit_cmd(batch, GENX(MEDIA_STATE_FLUSH), msf);

   batch->contains_draw = true;
}

#endif /* GFX_VER >= 7 */

/**
 * State module teardown.
 */
static void
crocus_destroy_state(struct crocus_context *ice)
{
   struct pipe_framebuffer_state *cso = &ice->state.framebuffer;

   pipe_resource_reference(&ice->draw.draw_params.res, NULL);
   pipe_resource_reference(&ice->draw.derived_draw_params.res, NULL);

   free(ice->state.genx);

   for (int i = 0; i < 4; i++) {
      pipe_so_target_reference(&ice->state.so_target[i], NULL);
   }

   util_unreference_framebuffer_state(cso);

   for (int stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      struct crocus_shader_state *shs = &ice->state.shaders[stage];
      for (int i = 0; i < PIPE_MAX_CONSTANT_BUFFERS; i++) {
         pipe_resource_reference(&shs->constbufs[i].buffer, NULL);
      }
      for (int i = 0; i < PIPE_MAX_SHADER_IMAGES; i++) {
         pipe_resource_reference(&shs->image[i].base.resource, NULL);
      }
      for (int i = 0; i < PIPE_MAX_SHADER_BUFFERS; i++) {
         pipe_resource_reference(&shs->ssbo[i].buffer, NULL);
      }
      for (int i = 0; i < CROCUS_MAX_TEXTURE_SAMPLERS; i++) {
         pipe_sampler_view_reference((struct pipe_sampler_view **)
                                     &shs->textures[i], NULL);
      }
   }

   for (int i = 0; i < 16; i++)
      pipe_resource_reference(&ice->state.vertex_buffers[i].buffer.resource, NULL);
   pipe_resource_reference(&ice->state.grid_size.res, NULL);

   pipe_resource_reference(&ice->state.index_buffer.res, NULL);
}

/* ------------------------------------------------------------------- */

static void
crocus_rebind_buffer(struct crocus_context *ice,
                     struct crocus_resource *res)
{
   struct pipe_context *ctx = &ice->ctx;

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
         struct pipe_vertex_buffer *buffer = &ice->state.vertex_buffers[i];

         if (!buffer->is_user_buffer && &res->base.b == buffer->buffer.resource)
            ice->state.dirty |= CROCUS_DIRTY_VERTEX_BUFFERS;
      }
   }

   if ((res->bind_history & PIPE_BIND_INDEX_BUFFER) &&
       ice->state.index_buffer.res) {
      if (res->bo == crocus_resource_bo(ice->state.index_buffer.res))
         pipe_resource_reference(&ice->state.index_buffer.res, NULL);
   }
   /* There is no need to handle these:
    * - PIPE_BIND_COMMAND_ARGS_BUFFER (emitted for every indirect draw)
    * - PIPE_BIND_QUERY_BUFFER (no persistent state references)
    */

   if (res->bind_history & PIPE_BIND_STREAM_OUTPUT) {
      /* XXX: be careful about resetting vs appending... */
      for (int i = 0; i < 4; i++) {
         if (ice->state.so_target[i] &&
             (ice->state.so_target[i]->buffer == &res->base.b)) {
#if GFX_VER == 6
            ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_BINDINGS_GS;
#else
            ice->state.dirty |= CROCUS_DIRTY_GEN7_SO_BUFFERS;
#endif
         }
      }
   }

   for (int s = MESA_SHADER_VERTEX; s < MESA_SHADER_STAGES; s++) {
      struct crocus_shader_state *shs = &ice->state.shaders[s];
      enum pipe_shader_type p_stage = stage_to_pipe(s);

      if (!(res->bind_stages & (1 << s)))
         continue;

      if (res->bind_history & PIPE_BIND_CONSTANT_BUFFER) {
         /* Skip constant buffer 0, it's for regular uniforms, not UBOs */
         uint32_t bound_cbufs = shs->bound_cbufs & ~1u;
         while (bound_cbufs) {
            const int i = u_bit_scan(&bound_cbufs);
            struct pipe_constant_buffer *cbuf = &shs->constbufs[i];

            if (res->bo == crocus_resource_bo(cbuf->buffer)) {
               ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_VS << s;
            }
         }
      }

      if (res->bind_history & PIPE_BIND_SHADER_BUFFER) {
         uint32_t bound_ssbos = shs->bound_ssbos;
         while (bound_ssbos) {
            const int i = u_bit_scan(&bound_ssbos);
            struct pipe_shader_buffer *ssbo = &shs->ssbo[i];

            if (res->bo == crocus_resource_bo(ssbo->buffer)) {
               struct pipe_shader_buffer buf = {
                  .buffer = &res->base.b,
                  .buffer_offset = ssbo->buffer_offset,
                  .buffer_size = ssbo->buffer_size,
               };
               crocus_set_shader_buffers(ctx, p_stage, i, 1, &buf,
                                         (shs->writable_ssbos >> i) & 1);
            }
         }
      }

      if (res->bind_history & PIPE_BIND_SAMPLER_VIEW) {
         uint32_t bound_sampler_views = shs->bound_sampler_views;
         while (bound_sampler_views) {
            const int i = u_bit_scan(&bound_sampler_views);
            struct crocus_sampler_view *isv = shs->textures[i];
            struct crocus_bo *bo = isv->res->bo;

            if (res->bo == bo) {
               ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_BINDINGS_VS << s;
            }
         }
      }

      if (res->bind_history & PIPE_BIND_SHADER_IMAGE) {
         uint32_t bound_image_views = shs->bound_image_views;
         while (bound_image_views) {
            const int i = u_bit_scan(&bound_image_views);
            struct crocus_image_view *iv = &shs->image[i];
            struct crocus_bo *bo = crocus_resource_bo(iv->base.resource);

            if (res->bo == bo)
               ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_BINDINGS_VS << s;
         }
      }
   }
}

/* ------------------------------------------------------------------- */

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

/*
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

#define IS_COMPUTE_PIPELINE(batch) (batch->name == CROCUS_BATCH_COMPUTE)

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
 * crocus_pipe_control.c instead, which may split the pipe control further.
 */
static void
crocus_emit_raw_pipe_control(struct crocus_batch *batch,
                             const char *reason,
                             uint32_t flags,
                             struct crocus_bo *bo,
                             uint32_t offset,
                             uint64_t imm)
{
   UNUSED const struct intel_device_info *devinfo = &batch->screen->devinfo;
   enum pipe_control_flags post_sync_flags = get_post_sync_flags(flags);
   UNUSED enum pipe_control_flags non_lri_post_sync_flags =
      post_sync_flags & ~PIPE_CONTROL_LRI_POST_SYNC_OP;

   /* Recursive PIPE_CONTROL workarounds --------------------------------
    * (http://knowyourmeme.com/memes/xzibit-yo-dawg)
    *
    * We do these first because we want to look at the original operation,
    * rather than any workarounds we set.
    */

   /* "Flush Types" workarounds ---------------------------------------------
    * We do these now because they may add post-sync operations or CS stalls.
    */

   if (GFX_VER == 6 && (flags & PIPE_CONTROL_RENDER_TARGET_FLUSH)) {
      /* Hardware workaround: SNB B-Spec says:
       *
       *    "[Dev-SNB{W/A}]: Before a PIPE_CONTROL with Write Cache Flush
       *     Enable = 1, a PIPE_CONTROL with any non-zero post-sync-op is
       *     required."
       */
      crocus_emit_post_sync_nonzero_flush(batch);
   }

#if GFX_VER == 8
   if (flags & PIPE_CONTROL_VF_CACHE_INVALIDATE) {
      /* Project: BDW, SKL+ (stopping at CNL) / Argument: VF Invalidate
       *
       * "'Post Sync Operation' must be enabled to 'Write Immediate Data' or
       *  'Write PS Depth Count' or 'Write Timestamp'."
       */
      if (!bo) {
         flags |= PIPE_CONTROL_WRITE_IMMEDIATE;
         post_sync_flags |= PIPE_CONTROL_WRITE_IMMEDIATE;
         non_lri_post_sync_flags |= PIPE_CONTROL_WRITE_IMMEDIATE;
         bo = batch->ice->workaround_bo;
         offset = batch->ice->workaround_offset;
      }
   }
#endif

#if GFX_VERx10 < 75
   if (flags & PIPE_CONTROL_DEPTH_STALL) {
      /* Project: PRE-HSW / Argument: Depth Stall
       *
       * "The following bits must be clear:
       *  - Render Target Cache Flush Enable ([12] of DW1)
       *  - Depth Cache Flush Enable ([0] of DW1)"
       */
      assert(!(flags & (PIPE_CONTROL_RENDER_TARGET_FLUSH |
                        PIPE_CONTROL_DEPTH_CACHE_FLUSH)));
   }
#endif
   if (GFX_VER >= 6 && (flags & PIPE_CONTROL_DEPTH_STALL)) {
      /* From the PIPE_CONTROL instruction table, bit 13 (Depth Stall Enable):
       *
       *    "This bit must be DISABLED for operations other than writing
       *     PS_DEPTH_COUNT."
       *
       * This seems like nonsense.  An Ivybridge workaround requires us to
       * emit a PIPE_CONTROL with a depth stall and write immediate post-sync
       * operation.  Gen8+ requires us to emit depth stalls and depth cache
       * flushes together.  So, it's hard to imagine this means anything other
       * than "we originally intended this to be used for PS_DEPTH_COUNT".
       *
       * We ignore the supposed restriction and do nothing.
       */
   }

   if (GFX_VERx10 < 75 && (flags & PIPE_CONTROL_DEPTH_CACHE_FLUSH)) {
      /* Project: PRE-HSW / Argument: Depth Cache Flush
       *
       * "Depth Stall must be clear ([13] of DW1)."
       */
      assert(!(flags & PIPE_CONTROL_DEPTH_STALL));
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

   if (flags & PIPE_CONTROL_STALL_AT_SCOREBOARD) {
      /* From the PIPE_CONTROL instruction table, bit 1:
       *
       *    "This bit is ignored if Depth Stall Enable is set.
       *     Further, the render cache is not flushed even if Write Cache
       *     Flush Enable bit is set."
       *
       * We assert that the caller doesn't do this combination, to try and
       * prevent mistakes.  It shouldn't hurt the GPU, though.
       *
       * We skip this check on Gen11+ as the "Stall at Pixel Scoreboard"
       * and "Render Target Flush" combo is explicitly required for BTI
       * update workarounds.
       */
      assert(!(flags & (PIPE_CONTROL_DEPTH_STALL |
                        PIPE_CONTROL_RENDER_TARGET_FLUSH)));
   }

   /* PIPE_CONTROL page workarounds ------------------------------------- */

   if (GFX_VER >= 7 && (flags & PIPE_CONTROL_STATE_CACHE_INVALIDATE)) {
      /* From the PIPE_CONTROL page itself:
       *
       *    "IVB, HSW, BDW
       *     Restriction: Pipe_control with CS-stall bit set must be issued
       *     before a pipe-control command that has the State Cache
       *     Invalidate bit set."
       */
      flags |= PIPE_CONTROL_CS_STALL;
   }

   if ((GFX_VERx10 == 75)) {
      /* From the PIPE_CONTROL page itself:
       *
       *    "HSW - Programming Note: PIPECONTROL with RO Cache Invalidation:
       *     Prior to programming a PIPECONTROL command with any of the RO
       *     cache invalidation bit set, program a PIPECONTROL flush command
       *     with “CS stall” bit and “HDC Flush” bit set."
       *
       * TODO: Actually implement this.  What's an HDC Flush?
       */
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

   if (GFX_VER >= 6 && GFX_VER < 8 && (flags & PIPE_CONTROL_TLB_INVALIDATE)) {
      /* Project: SNB, IVB, HSW / Argument: TLB inv
       *
       * "{All SKUs}{All Steppings}: Post-Sync Operation ([15:14] of DW1)
       *  must be set to something other than '0'."
       *
       * For now, we just assert that the caller does this.
       */
      assert(non_lri_post_sync_flags != 0);
   }

   if (GFX_VER >= 7 && (flags & PIPE_CONTROL_TLB_INVALIDATE)) {
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
#if GFX_VER == 8
   if (IS_COMPUTE_PIPELINE(batch)) {
      if (post_sync_flags ||
          (flags & (PIPE_CONTROL_NOTIFY_ENABLE |
                    PIPE_CONTROL_DEPTH_STALL |
                    PIPE_CONTROL_RENDER_TARGET_FLUSH |
                    PIPE_CONTROL_DEPTH_CACHE_FLUSH |
                    PIPE_CONTROL_DATA_CACHE_FLUSH))) {
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
          *
          * (The docs have separate table rows for each bit, with essentially
          * the same workaround text.  We've combined them here.)
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
#endif
   /* Implement the WaCsStallAtEveryFourthPipecontrol workaround on IVB, BYT:
    *
    * "Every 4th PIPE_CONTROL command, not counting the PIPE_CONTROL with
    *  only read-cache-invalidate bit(s) set, must have a CS_STALL bit set."
    *
    * Note that the kernel does CS stalls between batches, so we only need
    * to count them within a batch.  We currently naively count every 4, and
    * don't skip the ones with only read-cache-invalidate bits set.  This
    * may or may not be a problem...
    */
   if (GFX_VER == 7 && !(GFX_VERx10 == 75)) {
      if (flags & PIPE_CONTROL_CS_STALL) {
         /* If we're doing a CS stall, reset the counter and carry on. */
         batch->pipe_controls_since_last_cs_stall = 0;
      }

      /* If this is the fourth pipe control without a CS stall, do one now. */
      if (++batch->pipe_controls_since_last_cs_stall == 4) {
         batch->pipe_controls_since_last_cs_stall = 0;
         flags |= PIPE_CONTROL_CS_STALL;
      }
   }

   /* "Stall" workarounds ----------------------------------------------
    * These have to come after the earlier ones because we may have added
    * some additional CS stalls above.
    */

   if (flags & PIPE_CONTROL_CS_STALL) {
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

   /* Emit --------------------------------------------------------------- */

   if (INTEL_DEBUG(DEBUG_PIPE_CONTROL)) {
      fprintf(stderr,
              "  PC [%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%"PRIx64"]: %s\n",
              (flags & PIPE_CONTROL_FLUSH_ENABLE) ? "PipeCon " : "",
              (flags & PIPE_CONTROL_CS_STALL) ? "CS " : "",
              (flags & PIPE_CONTROL_STALL_AT_SCOREBOARD) ? "Scoreboard " : "",
              (flags & PIPE_CONTROL_VF_CACHE_INVALIDATE) ? "VF " : "",
              (flags & PIPE_CONTROL_RENDER_TARGET_FLUSH) ? "RT " : "",
              (flags & PIPE_CONTROL_CONST_CACHE_INVALIDATE) ? "Const " : "",
              (flags & PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE) ? "TC " : "",
              (flags & PIPE_CONTROL_DATA_CACHE_FLUSH) ? "DC " : "",
              (flags & PIPE_CONTROL_DEPTH_CACHE_FLUSH) ? "ZFlush " : "",
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
              imm, reason);
   }

   crocus_emit_cmd(batch, GENX(PIPE_CONTROL), pc) {
#if GFX_VER >= 7
      pc.LRIPostSyncOperation = NoLRIOperation;
      pc.PipeControlFlushEnable = flags & PIPE_CONTROL_FLUSH_ENABLE;
      pc.DCFlushEnable = flags & PIPE_CONTROL_DATA_CACHE_FLUSH;
#endif
#if GFX_VER >= 6
      pc.StoreDataIndex = 0;
      pc.CommandStreamerStallEnable = flags & PIPE_CONTROL_CS_STALL;
      pc.GlobalSnapshotCountReset =
         flags & PIPE_CONTROL_GLOBAL_SNAPSHOT_COUNT_RESET;
      pc.TLBInvalidate = flags & PIPE_CONTROL_TLB_INVALIDATE;
      pc.GenericMediaStateClear = flags & PIPE_CONTROL_MEDIA_STATE_CLEAR;
      pc.StallAtPixelScoreboard = flags & PIPE_CONTROL_STALL_AT_SCOREBOARD;
      pc.RenderTargetCacheFlushEnable =
         flags & PIPE_CONTROL_RENDER_TARGET_FLUSH;
      pc.DepthCacheFlushEnable = flags & PIPE_CONTROL_DEPTH_CACHE_FLUSH;
      pc.StateCacheInvalidationEnable =
         flags & PIPE_CONTROL_STATE_CACHE_INVALIDATE;
      pc.VFCacheInvalidationEnable = flags & PIPE_CONTROL_VF_CACHE_INVALIDATE;
      pc.ConstantCacheInvalidationEnable =
         flags & PIPE_CONTROL_CONST_CACHE_INVALIDATE;
#else
      pc.WriteCacheFlush = flags & PIPE_CONTROL_RENDER_TARGET_FLUSH;
#endif
      pc.PostSyncOperation = flags_to_post_sync_op(flags);
      pc.DepthStallEnable = flags & PIPE_CONTROL_DEPTH_STALL;
      pc.InstructionCacheInvalidateEnable =
         flags & PIPE_CONTROL_INSTRUCTION_INVALIDATE;
      pc.NotifyEnable = flags & PIPE_CONTROL_NOTIFY_ENABLE;
#if GFX_VER >= 5 || GFX_VERx10 == 45
      pc.IndirectStatePointersDisable =
         flags & PIPE_CONTROL_INDIRECT_STATE_POINTERS_DISABLE;
#endif
#if GFX_VER >= 6
      pc.TextureCacheInvalidationEnable =
         flags & PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE;
#elif GFX_VER == 5 || GFX_VERx10 == 45
      pc.TextureCacheFlushEnable =
         flags & PIPE_CONTROL_TEXTURE_CACHE_INVALIDATE;
#endif
      pc.Address = ggtt_bo(bo, offset);
      if (GFX_VER < 7 && bo)
         pc.DestinationAddressType = DAT_GGTT;
      pc.ImmediateData = imm;
   }
}

#if GFX_VER == 6
void
genX(crocus_upload_urb)(struct crocus_batch *batch,
                        unsigned vs_size,
                        bool gs_present,
                        unsigned gs_size)
{
   struct crocus_context *ice = batch->ice;
   int nr_vs_entries, nr_gs_entries;
   int total_urb_size = ice->urb.size * 1024; /* in bytes */
   const struct intel_device_info *devinfo = &batch->screen->devinfo;

   /* Calculate how many entries fit in each stage's section of the URB */
   if (gs_present) {
      nr_vs_entries = (total_urb_size/2) / (vs_size * 128);
      nr_gs_entries = (total_urb_size/2) / (gs_size * 128);
   } else {
      nr_vs_entries = total_urb_size / (vs_size * 128);
      nr_gs_entries = 0;
   }

   /* Then clamp to the maximum allowed by the hardware */
   if (nr_vs_entries > devinfo->urb.max_entries[MESA_SHADER_VERTEX])
      nr_vs_entries = devinfo->urb.max_entries[MESA_SHADER_VERTEX];

   if (nr_gs_entries > devinfo->urb.max_entries[MESA_SHADER_GEOMETRY])
      nr_gs_entries = devinfo->urb.max_entries[MESA_SHADER_GEOMETRY];

   /* Finally, both must be a multiple of 4 (see 3DSTATE_URB in the PRM). */
   ice->urb.nr_vs_entries = ROUND_DOWN_TO(nr_vs_entries, 4);
   ice->urb.nr_gs_entries = ROUND_DOWN_TO(nr_gs_entries, 4);

   assert(ice->urb.nr_vs_entries >=
          devinfo->urb.min_entries[MESA_SHADER_VERTEX]);
   assert(ice->urb.nr_vs_entries % 4 == 0);
   assert(ice->urb.nr_gs_entries % 4 == 0);
   assert(vs_size <= 5);
   assert(gs_size <= 5);

   crocus_emit_cmd(batch, GENX(3DSTATE_URB), urb) {
      urb.VSNumberofURBEntries = ice->urb.nr_vs_entries;
      urb.VSURBEntryAllocationSize = vs_size - 1;

      urb.GSNumberofURBEntries = ice->urb.nr_gs_entries;
      urb.GSURBEntryAllocationSize = gs_size - 1;
   };
   /* From the PRM Volume 2 part 1, section 1.4.7:
    *
    *   Because of a urb corruption caused by allocating a previous gsunit’s
    *   urb entry to vsunit software is required to send a "GS NULL
    *   Fence"(Send URB fence with VS URB size == 1 and GS URB size == 0) plus
    *   a dummy DRAW call before any case where VS will be taking over GS URB
    *   space.
    *
    * It is not clear exactly what this means ("URB fence" is a command that
    * doesn't exist on Gen6).  So for now we just do a full pipeline flush as
    * a workaround.
    */
   if (ice->urb.gs_present && !gs_present)
      crocus_emit_mi_flush(batch);
   ice->urb.gs_present = gs_present;
}
#endif

static void
crocus_lost_genx_state(struct crocus_context *ice, struct crocus_batch *batch)
{
}

static void
crocus_emit_mi_report_perf_count(struct crocus_batch *batch,
                                 struct crocus_bo *bo,
                                 uint32_t offset_in_bytes,
                                 uint32_t report_id)
{
#if GFX_VER >= 7
   crocus_emit_cmd(batch, GENX(MI_REPORT_PERF_COUNT), mi_rpc) {
      mi_rpc.MemoryAddress = rw_bo(bo, offset_in_bytes);
      mi_rpc.ReportID = report_id;
   }
#endif
}

/**
 * From the PRM, Volume 2a:
 *
 *    "Indirect State Pointers Disable
 *
 *    At the completion of the post-sync operation associated with this pipe
 *    control packet, the indirect state pointers in the hardware are
 *    considered invalid; the indirect pointers are not saved in the context.
 *    If any new indirect state commands are executed in the command stream
 *    while the pipe control is pending, the new indirect state commands are
 *    preserved.
 *
 *    [DevIVB+]: Using Invalidate State Pointer (ISP) only inhibits context
 *    restoring of Push Constant (3DSTATE_CONSTANT_*) commands. Push Constant
 *    commands are only considered as Indirect State Pointers. Once ISP is
 *    issued in a context, SW must initialize by programming push constant
 *    commands for all the shaders (at least to zero length) before attempting
 *    any rendering operation for the same context."
 *
 * 3DSTATE_CONSTANT_* packets are restored during a context restore,
 * even though they point to a BO that has been already unreferenced at
 * the end of the previous batch buffer. This has been fine so far since
 * we are protected by these scratch page (every address not covered by
 * a BO should be pointing to the scratch page). But on CNL, it is
 * causing a GPU hang during context restore at the 3DSTATE_CONSTANT_*
 * instruction.
 *
 * The flag "Indirect State Pointers Disable" in PIPE_CONTROL tells the
 * hardware to ignore previous 3DSTATE_CONSTANT_* packets during a
 * context restore, so the mentioned hang doesn't happen. However,
 * software must program push constant commands for all stages prior to
 * rendering anything, so we flag them as dirty.
 *
 * Finally, we also make sure to stall at pixel scoreboard to make sure the
 * constants have been loaded into the EUs prior to disable the push constants
 * so that it doesn't hang a previous 3DPRIMITIVE.
 */
#if GFX_VER >= 7
static void
gen7_emit_isp_disable(struct crocus_batch *batch)
{
   crocus_emit_raw_pipe_control(batch, "isp disable",
                                PIPE_CONTROL_STALL_AT_SCOREBOARD |
                                PIPE_CONTROL_CS_STALL,
                                NULL, 0, 0);
   crocus_emit_raw_pipe_control(batch, "isp disable",
                                PIPE_CONTROL_INDIRECT_STATE_POINTERS_DISABLE |
                                PIPE_CONTROL_CS_STALL,
                                NULL, 0, 0);

   struct crocus_context *ice = batch->ice;
   ice->state.stage_dirty |= (CROCUS_STAGE_DIRTY_CONSTANTS_VS |
                              CROCUS_STAGE_DIRTY_CONSTANTS_TCS |
                              CROCUS_STAGE_DIRTY_CONSTANTS_TES |
                              CROCUS_STAGE_DIRTY_CONSTANTS_GS |
                              CROCUS_STAGE_DIRTY_CONSTANTS_FS);
}
#endif

#if GFX_VER >= 7
static void
crocus_state_finish_batch(struct crocus_batch *batch)
{
#if GFX_VERx10 == 75
   if (batch->name == CROCUS_BATCH_RENDER) {
      crocus_emit_mi_flush(batch);
      crocus_emit_cmd(batch, GENX(3DSTATE_CC_STATE_POINTERS), ptr) {
         ptr.ColorCalcStatePointer = batch->ice->shaders.cc_offset;
      }

      crocus_emit_pipe_control_flush(batch, "hsw wa", PIPE_CONTROL_RENDER_TARGET_FLUSH |
                                     PIPE_CONTROL_CS_STALL);
   }
#endif
   gen7_emit_isp_disable(batch);
}
#endif

static void
crocus_batch_reset_dirty(struct crocus_batch *batch)
{
   /* unreference any index buffer so it get reemitted. */
   pipe_resource_reference(&batch->ice->state.index_buffer.res, NULL);

   /* for GEN4/5 need to reemit anything that ends up in the state batch that points to anything in the state batch
    * as the old state batch won't still be available.
    */
   batch->ice->state.dirty |= CROCUS_DIRTY_DEPTH_BUFFER |
      CROCUS_DIRTY_COLOR_CALC_STATE;

   batch->ice->state.dirty |= CROCUS_DIRTY_VERTEX_ELEMENTS | CROCUS_DIRTY_VERTEX_BUFFERS;

   batch->ice->state.stage_dirty |= CROCUS_ALL_STAGE_DIRTY_BINDINGS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_SAMPLER_STATES_VS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_SAMPLER_STATES_TES;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_SAMPLER_STATES_TCS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_SAMPLER_STATES_GS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_SAMPLER_STATES_PS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_SAMPLER_STATES_CS;

   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_VS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_TES;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_TCS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_GS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_FS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CONSTANTS_CS;

   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_VS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_GS;
   batch->ice->state.stage_dirty |= CROCUS_STAGE_DIRTY_CS;
   batch->ice->state.dirty |= CROCUS_DIRTY_CC_VIEWPORT | CROCUS_DIRTY_SF_CL_VIEWPORT;

#if GFX_VER >= 6
   /* SCISSOR_STATE */
   batch->ice->state.dirty |= CROCUS_DIRTY_GEN6_BLEND_STATE;
   batch->ice->state.dirty |= CROCUS_DIRTY_GEN6_SCISSOR_RECT;
   batch->ice->state.dirty |= CROCUS_DIRTY_GEN6_WM_DEPTH_STENCIL;

#endif
#if GFX_VER <= 5
   /* dirty the SF state on gen4/5 */
   batch->ice->state.dirty |= CROCUS_DIRTY_RASTER;
   batch->ice->state.dirty |= CROCUS_DIRTY_GEN4_CURBE;
   batch->ice->state.dirty |= CROCUS_DIRTY_CLIP;
   batch->ice->state.dirty |= CROCUS_DIRTY_WM;
#endif
#if GFX_VER >= 7
   /* Streamout dirty */
   batch->ice->state.dirty |= CROCUS_DIRTY_STREAMOUT;
   batch->ice->state.dirty |= CROCUS_DIRTY_SO_DECL_LIST;
   batch->ice->state.dirty |= CROCUS_DIRTY_GEN7_SO_BUFFERS;
#endif
}

#if GFX_VERx10 == 75
struct pipe_rasterizer_state *crocus_get_rast_state(struct crocus_context *ice)
{
   return &ice->state.cso_rast->cso;
}
#endif

#if GFX_VER >= 6
static void update_so_strides(struct crocus_context *ice,
                              uint16_t *strides)
{
   for (int i = 0; i < PIPE_MAX_SO_BUFFERS; i++) {
      struct crocus_stream_output_target *so = (void *)ice->state.so_target[i];
      if (so)
         so->stride = strides[i] * sizeof(uint32_t);
   }
}
#endif

static void crocus_fill_clamp_mask(const struct crocus_sampler_state *samp,
                                   int s,
                                   uint32_t *clamp_mask)
{
#if GFX_VER < 8
   if (samp->pstate.min_img_filter != PIPE_TEX_FILTER_NEAREST &&
       samp->pstate.mag_img_filter != PIPE_TEX_FILTER_NEAREST) {
      if (samp->pstate.wrap_s == PIPE_TEX_WRAP_CLAMP)
         clamp_mask[0] |= (1 << s);
      if (samp->pstate.wrap_t == PIPE_TEX_WRAP_CLAMP)
         clamp_mask[1] |= (1 << s);
      if (samp->pstate.wrap_r == PIPE_TEX_WRAP_CLAMP)
         clamp_mask[2] |= (1 << s);
   }
#endif
}

static void
crocus_set_frontend_noop(struct pipe_context *ctx, bool enable)
{
   struct crocus_context *ice = (struct crocus_context *) ctx;

   if (crocus_batch_prepare_noop(&ice->batches[CROCUS_BATCH_RENDER], enable)) {
      ice->state.dirty |= CROCUS_ALL_DIRTY_FOR_RENDER;
      ice->state.stage_dirty |= CROCUS_ALL_STAGE_DIRTY_FOR_RENDER;
   }

   if (ice->batch_count == 1)
      return;

   if (crocus_batch_prepare_noop(&ice->batches[CROCUS_BATCH_COMPUTE], enable)) {
      ice->state.dirty |= CROCUS_ALL_DIRTY_FOR_COMPUTE;
      ice->state.stage_dirty |= CROCUS_ALL_STAGE_DIRTY_FOR_COMPUTE;
   }
}

void
genX(crocus_init_screen_state)(struct crocus_screen *screen)
{
   assert(screen->devinfo.verx10 == GFX_VERx10);
   assert(screen->devinfo.ver == GFX_VER);
   screen->vtbl.destroy_state = crocus_destroy_state;
   screen->vtbl.init_render_context = crocus_init_render_context;
   screen->vtbl.upload_render_state = crocus_upload_render_state;
#if GFX_VER >= 7
   screen->vtbl.init_compute_context = crocus_init_compute_context;
   screen->vtbl.upload_compute_state = crocus_upload_compute_state;
#endif
   screen->vtbl.emit_raw_pipe_control = crocus_emit_raw_pipe_control;
   screen->vtbl.emit_mi_report_perf_count = crocus_emit_mi_report_perf_count;
   screen->vtbl.rebind_buffer = crocus_rebind_buffer;
#if GFX_VERx10 >= 75
   screen->vtbl.load_register_reg32 = crocus_load_register_reg32;
   screen->vtbl.load_register_reg64 = crocus_load_register_reg64;
   screen->vtbl.load_register_imm32 = crocus_load_register_imm32;
   screen->vtbl.load_register_imm64 = crocus_load_register_imm64;
   screen->vtbl.store_data_imm32 = crocus_store_data_imm32;
   screen->vtbl.store_data_imm64 = crocus_store_data_imm64;
#endif
#if GFX_VER >= 7
   screen->vtbl.load_register_mem32 = crocus_load_register_mem32;
   screen->vtbl.load_register_mem64 = crocus_load_register_mem64;
   screen->vtbl.copy_mem_mem = crocus_copy_mem_mem;
   screen->vtbl.create_so_decl_list = crocus_create_so_decl_list;
#endif
   screen->vtbl.update_surface_base_address = crocus_update_surface_base_address;
#if GFX_VER >= 6
   screen->vtbl.store_register_mem32 = crocus_store_register_mem32;
   screen->vtbl.store_register_mem64 = crocus_store_register_mem64;
#endif
   screen->vtbl.populate_vs_key = crocus_populate_vs_key;
   screen->vtbl.populate_tcs_key = crocus_populate_tcs_key;
   screen->vtbl.populate_tes_key = crocus_populate_tes_key;
   screen->vtbl.populate_gs_key = crocus_populate_gs_key;
   screen->vtbl.populate_fs_key = crocus_populate_fs_key;
   screen->vtbl.populate_cs_key = crocus_populate_cs_key;
   screen->vtbl.lost_genx_state = crocus_lost_genx_state;
#if GFX_VER >= 7
   screen->vtbl.finish_batch = crocus_state_finish_batch;
#endif
#if GFX_VER <= 5
   screen->vtbl.upload_urb_fence = crocus_upload_urb_fence;
   screen->vtbl.calculate_urb_fence = crocus_calculate_urb_fence;
#endif
   screen->vtbl.fill_clamp_mask = crocus_fill_clamp_mask;
   screen->vtbl.batch_reset_dirty = crocus_batch_reset_dirty;
   screen->vtbl.translate_prim_type = translate_prim_type;
#if GFX_VER >= 6
   screen->vtbl.update_so_strides = update_so_strides;
   screen->vtbl.get_so_offset = crocus_get_so_offset;
#endif

   genX(crocus_init_blt)(screen);
}

void
genX(crocus_init_state)(struct crocus_context *ice)
{
   struct pipe_context *ctx = &ice->ctx;

   ctx->create_blend_state = crocus_create_blend_state;
   ctx->create_depth_stencil_alpha_state = crocus_create_zsa_state;
   ctx->create_rasterizer_state = crocus_create_rasterizer_state;
   ctx->create_sampler_state = crocus_create_sampler_state;
   ctx->create_sampler_view = crocus_create_sampler_view;
   ctx->create_surface = crocus_create_surface;
   ctx->create_vertex_elements_state = crocus_create_vertex_elements;
   ctx->bind_blend_state = crocus_bind_blend_state;
   ctx->bind_depth_stencil_alpha_state = crocus_bind_zsa_state;
   ctx->bind_sampler_states = crocus_bind_sampler_states;
   ctx->bind_rasterizer_state = crocus_bind_rasterizer_state;
   ctx->bind_vertex_elements_state = crocus_bind_vertex_elements_state;
   ctx->delete_blend_state = crocus_delete_state;
   ctx->delete_depth_stencil_alpha_state = crocus_delete_state;
   ctx->delete_rasterizer_state = crocus_delete_state;
   ctx->delete_sampler_state = crocus_delete_state;
   ctx->delete_vertex_elements_state = crocus_delete_state;
   ctx->set_blend_color = crocus_set_blend_color;
   ctx->set_clip_state = crocus_set_clip_state;
   ctx->set_constant_buffer = crocus_set_constant_buffer;
   ctx->set_shader_buffers = crocus_set_shader_buffers;
   ctx->set_shader_images = crocus_set_shader_images;
   ctx->set_sampler_views = crocus_set_sampler_views;
   ctx->set_tess_state = crocus_set_tess_state;
   ctx->set_patch_vertices = crocus_set_patch_vertices;
   ctx->set_framebuffer_state = crocus_set_framebuffer_state;
   ctx->set_polygon_stipple = crocus_set_polygon_stipple;
   ctx->set_sample_mask = crocus_set_sample_mask;
   ctx->set_scissor_states = crocus_set_scissor_states;
   ctx->set_stencil_ref = crocus_set_stencil_ref;
   ctx->set_vertex_buffers = crocus_set_vertex_buffers;
   ctx->set_viewport_states = crocus_set_viewport_states;
   ctx->sampler_view_destroy = crocus_sampler_view_destroy;
   ctx->surface_destroy = crocus_surface_destroy;
   ctx->draw_vbo = crocus_draw_vbo;
   ctx->launch_grid = crocus_launch_grid;

   ctx->set_frontend_noop = crocus_set_frontend_noop;

#if GFX_VER >= 6
   ctx->create_stream_output_target = crocus_create_stream_output_target;
   ctx->stream_output_target_destroy = crocus_stream_output_target_destroy;
   ctx->set_stream_output_targets = crocus_set_stream_output_targets;
#endif

   ice->state.dirty = ~0ull;
   ice->state.stage_dirty = ~0ull;

   ice->state.statistics_counters_enabled = true;

   ice->state.sample_mask = 0xff;
   ice->state.num_viewports = 1;
   ice->state.prim_mode = MESA_PRIM_COUNT;
   ice->state.reduced_prim_mode = MESA_PRIM_COUNT;
   ice->state.genx = calloc(1, sizeof(struct crocus_genx_state));
   ice->draw.derived_params.drawid = -1;

   /* Default all scissor rectangles to be empty regions. */
   for (int i = 0; i < CROCUS_MAX_VIEWPORTS; i++) {
      ice->state.scissors[i] = (struct pipe_scissor_state) {
         .minx = 1, .maxx = 0, .miny = 1, .maxy = 0,
      };
   }
}
