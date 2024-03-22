/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "pipe/p_defines.h"
#include "util/u_bitmask.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "svga_context.h"
#include "svga_hw_reg.h"
#include "svga_cmd.h"


static inline unsigned
svga_translate_compare_func(unsigned func)
{
   switch (func) {
   case PIPE_FUNC_NEVER:     return SVGA3D_CMP_NEVER;
   case PIPE_FUNC_LESS:      return SVGA3D_CMP_LESS;
   case PIPE_FUNC_LEQUAL:    return SVGA3D_CMP_LESSEQUAL;
   case PIPE_FUNC_GREATER:   return SVGA3D_CMP_GREATER;
   case PIPE_FUNC_GEQUAL:    return SVGA3D_CMP_GREATEREQUAL;
   case PIPE_FUNC_NOTEQUAL:  return SVGA3D_CMP_NOTEQUAL;
   case PIPE_FUNC_EQUAL:     return SVGA3D_CMP_EQUAL;
   case PIPE_FUNC_ALWAYS:    return SVGA3D_CMP_ALWAYS;
   default:
      assert(0);
      return SVGA3D_CMP_ALWAYS;
   }
}

static inline unsigned
svga_translate_stencil_op(unsigned op)
{
   switch (op) {
   case PIPE_STENCIL_OP_KEEP:      return SVGA3D_STENCILOP_KEEP;
   case PIPE_STENCIL_OP_ZERO:      return SVGA3D_STENCILOP_ZERO;
   case PIPE_STENCIL_OP_REPLACE:   return SVGA3D_STENCILOP_REPLACE;
   case PIPE_STENCIL_OP_INCR:      return SVGA3D_STENCILOP_INCRSAT;
   case PIPE_STENCIL_OP_DECR:      return SVGA3D_STENCILOP_DECRSAT;
   case PIPE_STENCIL_OP_INCR_WRAP: return SVGA3D_STENCILOP_INCR;
   case PIPE_STENCIL_OP_DECR_WRAP: return SVGA3D_STENCILOP_DECR;
   case PIPE_STENCIL_OP_INVERT:    return SVGA3D_STENCILOP_INVERT;
   default:
      assert(0);
      return SVGA3D_STENCILOP_KEEP;
   }
}


/**
 * Define a vgpu10 depth/stencil state object for the given
 * svga depth/stencil state.
 */
static void
define_depth_stencil_state_object(struct svga_context *svga,
                                  struct svga_depth_stencil_state *ds)
{
   assert(svga_have_vgpu10(svga));

   ds->id = util_bitmask_add(svga->ds_object_id_bm);

   /* spot check that these comparision tokens are the same */
   STATIC_ASSERT(SVGA3D_COMPARISON_NEVER == SVGA3D_CMP_NEVER);
   STATIC_ASSERT(SVGA3D_COMPARISON_LESS == SVGA3D_CMP_LESS);
   STATIC_ASSERT(SVGA3D_COMPARISON_NOT_EQUAL == SVGA3D_CMP_NOTEQUAL);

   /* Note: we use the ds->stencil[0].enabled value for both the front
    * and back-face enables.  If single-side stencil is used, we'll have
    * set the back state the same as the front state.
    */
   SVGA_RETRY(svga, SVGA3D_vgpu10_DefineDepthStencilState
              (svga->swc,
               ds->id,
               /* depth/Z */
               ds->zenable,
               ds->zwriteenable,
               ds->zfunc,
               /* Stencil */
               ds->stencil[0].enabled, /*f|b*/
               ds->stencil[0].enabled, /*f*/
               ds->stencil[0].enabled, /*b*/
               ds->stencil_mask,
               ds->stencil_writemask,
               /* front stencil */
               ds->stencil[0].fail,
               ds->stencil[0].zfail,
               ds->stencil[0].pass,
               ds->stencil[0].func,
               /* back stencil */
               ds->stencil[1].fail,
               ds->stencil[1].zfail,
               ds->stencil[1].pass,
               ds->stencil[1].func));
}


static void *
svga_create_depth_stencil_state(struct pipe_context *pipe,
				const struct pipe_depth_stencil_alpha_state *templ)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_depth_stencil_state *ds = CALLOC_STRUCT(svga_depth_stencil_state);

   if (!ds)
      return NULL;

   /* Don't try to figure out CW/CCW correspondence with
    * stencil[0]/[1] at this point.  Presumably this can change as
    * back/front face are modified.
    */
   ds->stencil[0].enabled = templ->stencil[0].enabled;
   if (ds->stencil[0].enabled) {
      ds->stencil[0].func  = svga_translate_compare_func(templ->stencil[0].func);
      ds->stencil[0].fail  = svga_translate_stencil_op(templ->stencil[0].fail_op);
      ds->stencil[0].zfail = svga_translate_stencil_op(templ->stencil[0].zfail_op);
      ds->stencil[0].pass  = svga_translate_stencil_op(templ->stencil[0].zpass_op);

      /* SVGA3D has one ref/mask/writemask triple shared between front &
       * back face stencil.  We really need two:
       */
      ds->stencil_mask      = templ->stencil[0].valuemask & 0xff;
      ds->stencil_writemask = templ->stencil[0].writemask & 0xff;
   }
   else {
      ds->stencil[0].func = SVGA3D_CMP_ALWAYS;
      ds->stencil[0].fail = SVGA3D_STENCILOP_KEEP;
      ds->stencil[0].zfail = SVGA3D_STENCILOP_KEEP;
      ds->stencil[0].pass = SVGA3D_STENCILOP_KEEP;
   }

   ds->stencil[1].enabled = templ->stencil[1].enabled;
   if (templ->stencil[1].enabled) {
      assert(templ->stencil[0].enabled);
      /* two-sided stencil */
      ds->stencil[1].func   = svga_translate_compare_func(templ->stencil[1].func);
      ds->stencil[1].fail   = svga_translate_stencil_op(templ->stencil[1].fail_op);
      ds->stencil[1].zfail  = svga_translate_stencil_op(templ->stencil[1].zfail_op);
      ds->stencil[1].pass   = svga_translate_stencil_op(templ->stencil[1].zpass_op);

      ds->stencil_mask      = templ->stencil[1].valuemask & 0xff;
      ds->stencil_writemask = templ->stencil[1].writemask & 0xff;

      if (templ->stencil[1].valuemask != templ->stencil[0].valuemask) {
         util_debug_message(&svga->debug.callback, CONFORMANCE,
                            "two-sided stencil mask not supported "
                            "(front=0x%x, back=0x%x)",
                            templ->stencil[0].valuemask,
                            templ->stencil[1].valuemask);
      }
      if (templ->stencil[1].writemask != templ->stencil[0].writemask) {
         util_debug_message(&svga->debug.callback, CONFORMANCE,
                            "two-sided stencil writemask not supported "
                            "(front=0x%x, back=0x%x)",
                            templ->stencil[0].writemask,
                            templ->stencil[1].writemask);
      }
   }
   else {
      /* back face state is same as front-face state */
      ds->stencil[1].func = ds->stencil[0].func;
      ds->stencil[1].fail = ds->stencil[0].fail;
      ds->stencil[1].zfail = ds->stencil[0].zfail;
      ds->stencil[1].pass = ds->stencil[0].pass;
   }


   ds->zenable = templ->depth_enabled;
   if (ds->zenable) {
      ds->zfunc = svga_translate_compare_func(templ->depth_func);
      ds->zwriteenable = templ->depth_writemask;
   }
   else {
      ds->zfunc = SVGA3D_CMP_ALWAYS;
   }

   ds->alphatestenable = templ->alpha_enabled;
   if (ds->alphatestenable) {
      ds->alphafunc = svga_translate_compare_func(templ->alpha_func);
      ds->alpharef = templ->alpha_ref_value;
   }
   else {
      ds->alphafunc = SVGA3D_CMP_ALWAYS;
   }

   if (svga_have_vgpu10(svga)) {
      define_depth_stencil_state_object(svga, ds);
   }

   svga->hud.num_depthstencil_objects++;

   SVGA_STATS_COUNT_INC(svga_screen(svga->pipe.screen)->sws,
                        SVGA_STATS_COUNT_DEPTHSTENCILSTATE);

   return ds;
}


static void
svga_bind_depth_stencil_state(struct pipe_context *pipe, void *depth_stencil)
{
   struct svga_context *svga = svga_context(pipe);

   if (svga_have_vgpu10(svga)) {
      /* flush any previously queued drawing before changing state */
      svga_hwtnl_flush_retry(svga);
   }

   svga->curr.depth = (const struct svga_depth_stencil_state *)depth_stencil;
   svga->dirty |= SVGA_NEW_DEPTH_STENCIL_ALPHA;
}


static void
svga_delete_depth_stencil_state(struct pipe_context *pipe, void *depth_stencil)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_depth_stencil_state *ds =
      (struct svga_depth_stencil_state *) depth_stencil;

   if (svga_have_vgpu10(svga)) {
      svga_hwtnl_flush_retry(svga);

      assert(ds->id != SVGA3D_INVALID_ID);

      SVGA_RETRY(svga, SVGA3D_vgpu10_DestroyDepthStencilState(svga->swc,
                                                              ds->id));

      if (ds->id == svga->state.hw_draw.depth_stencil_id)
         svga->state.hw_draw.depth_stencil_id = SVGA3D_INVALID_ID;

      util_bitmask_clear(svga->ds_object_id_bm, ds->id);
      ds->id = SVGA3D_INVALID_ID;
   }

   FREE(depth_stencil);
   svga->hud.num_depthstencil_objects--;
}


static void
svga_set_stencil_ref(struct pipe_context *pipe,
                     const struct pipe_stencil_ref stencil_ref)
{
   struct svga_context *svga = svga_context(pipe);

   if (svga_have_vgpu10(svga)) {
      /* flush any previously queued drawing before changing state */
      svga_hwtnl_flush_retry(svga);
   }

   svga->curr.stencil_ref = stencil_ref;

   svga->dirty |= SVGA_NEW_STENCIL_REF;
}


static void
svga_set_sample_mask(struct pipe_context *pipe,
                     unsigned sample_mask)
{
   struct svga_context *svga = svga_context(pipe);

   svga->curr.sample_mask = sample_mask;

   svga->dirty |= SVGA_NEW_BLEND; /* See emit_rss_vgpu10() */
}


static void
svga_set_min_samples(struct pipe_context *pipe, unsigned min_samples)
{
   /* This specifies the minimum number of times the fragment shader
    * must run when doing per-sample shading for a MSAA render target.
    * For our SVGA3D device, the FS is automatically run in per-sample
    * mode if it uses the sample ID or sample position registers.
    */
}


void
svga_init_depth_stencil_functions(struct svga_context *svga)
{
   svga->pipe.create_depth_stencil_alpha_state = svga_create_depth_stencil_state;
   svga->pipe.bind_depth_stencil_alpha_state = svga_bind_depth_stencil_state;
   svga->pipe.delete_depth_stencil_alpha_state = svga_delete_depth_stencil_state;

   svga->pipe.set_stencil_ref = svga_set_stencil_ref;
   svga->pipe.set_sample_mask = svga_set_sample_mask;
   svga->pipe.set_min_samples = svga_set_min_samples;
}
