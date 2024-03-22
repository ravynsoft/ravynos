/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
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
/*
 * Authors:
 *   Keith Whitwell <keithw@vmware.com>
 */

#include "util/u_memory.h"
#include "i915_context.h"
#include "i915_reg.h"
#include "i915_state.h"
#include "i915_state_inlines.h"

/* Convinience function to check immediate state.
 */

static inline void
set_immediate(struct i915_context *i915, unsigned offset, const unsigned state)
{
   if (i915->current.immediate[offset] == state)
      return;

   i915->current.immediate[offset] = state;
   i915->immediate_dirty |= 1 << offset;
   i915->hardware_dirty |= I915_HW_IMMEDIATE;
}

/***********************************************************************
 * S0,S1: Vertex buffer state.
 */
static void
upload_S0S1(struct i915_context *i915)
{
   unsigned LIS0, LIS1;

   /* I915_NEW_VBO
    */
   LIS0 = i915->vbo_offset;

   /* Need to force this */
   if (i915->dirty & I915_NEW_VBO) {
      i915->immediate_dirty |= 1 << I915_IMMEDIATE_S0;
      i915->hardware_dirty |= I915_HW_IMMEDIATE;
   }

   /* I915_NEW_VERTEX_SIZE
    */
   {
      unsigned vertex_size = i915->current.vertex_info.draw.size;

      LIS1 = ((vertex_size << 24) | (vertex_size << 16));
   }

   set_immediate(i915, I915_IMMEDIATE_S0, LIS0);
   set_immediate(i915, I915_IMMEDIATE_S1, LIS1);
}

const struct i915_tracked_state i915_upload_S0S1 = {
   "imm S0 S1", upload_S0S1, I915_NEW_VBO | I915_NEW_VERTEX_FORMAT};

/***********************************************************************
 * S4: Vertex format, rasterization state
 */
static void
upload_S2S4(struct i915_context *i915)
{
   unsigned LIS2, LIS4;

   /* I915_NEW_VERTEX_FORMAT
    */
   {
      LIS2 = i915->current.vertex_info.hwfmt[1];
      LIS4 = i915->current.vertex_info.hwfmt[0];
      assert(LIS4); /* should never be zero? */
   }

   LIS4 |= i915->rasterizer->LIS4;

   set_immediate(i915, I915_IMMEDIATE_S2, LIS2);
   set_immediate(i915, I915_IMMEDIATE_S4, LIS4);
}

const struct i915_tracked_state i915_upload_S2S4 = {
   "imm S2 S4", upload_S2S4, I915_NEW_RASTERIZER | I915_NEW_VERTEX_FORMAT};

/***********************************************************************
 */
static void
upload_S5(struct i915_context *i915)
{
   unsigned LIS5 = 0;
   bool stencil_ccw = i915_stencil_ccw(i915);

   /* I915_NEW_DEPTH_STENCIL
    */
   if (stencil_ccw)
      LIS5 |= i915->depth_stencil->stencil_LIS5_ccw;
   else
      LIS5 |= i915->depth_stencil->stencil_LIS5_cw;
   /* hope it's safe to set stencil ref value even if stencil test is disabled?
    */
   LIS5 |= i915->stencil_ref.ref_value[stencil_ccw] << S5_STENCIL_REF_SHIFT;

   /* I915_NEW_BLEND
    */
   LIS5 |= i915->blend->LIS5;

#if 0
   /* I915_NEW_RASTERIZER
    */
   if (i915->rasterizer->LIS7) {
      LIS5 |= S5_GLOBAL_DEPTH_OFFSET_ENABLE;
   }
#endif

   set_immediate(i915, I915_IMMEDIATE_S5, LIS5);
}

const struct i915_tracked_state i915_upload_S5 = {
   "imm S5", upload_S5,
   I915_NEW_DEPTH_STENCIL | I915_NEW_BLEND | I915_NEW_RASTERIZER};

/***********************************************************************
 */
static void
upload_S6(struct i915_context *i915)
{
   unsigned LIS6 = 0;

   /* I915_NEW_FRAMEBUFFER
    */
   if (i915->framebuffer.cbufs[0])
      LIS6 |= S6_COLOR_WRITE_ENABLE;

   /* I915_NEW_BLEND
    */
   if (i915->blend) {
      struct i915_surface *cbuf = i915_surface(i915->framebuffer.cbufs[0]);
      if (cbuf && cbuf->alpha_in_g)
         LIS6 |= i915->blend->LIS6_alpha_in_g;
      else if (cbuf && cbuf->alpha_is_x)
         LIS6 |= i915->blend->LIS6_alpha_is_x;
      else
         LIS6 |= i915->blend->LIS6;
   }

   /* I915_NEW_DEPTH
    */
   if (i915->depth_stencil)
      LIS6 |= i915->depth_stencil->depth_LIS6;

   if (i915->rasterizer)
      LIS6 |= i915->rasterizer->LIS6;

   set_immediate(i915, I915_IMMEDIATE_S6, LIS6);
}

const struct i915_tracked_state i915_upload_S6 = {
   "imm S6", upload_S6,
   I915_NEW_BLEND | I915_NEW_DEPTH_STENCIL | I915_NEW_FRAMEBUFFER |
      I915_NEW_RASTERIZER};

/***********************************************************************
 */
static void
upload_S7(struct i915_context *i915)
{
#if 0
   unsigned LIS7;

   /* I915_NEW_RASTERIZER
    */
   LIS7 = i915->rasterizer->LIS7;

   set_immediate(i915, I915_IMMEDIATE_S7, LIS7);
#endif
}

const struct i915_tracked_state i915_upload_S7 = {"imm S7", upload_S7,
                                                  I915_NEW_RASTERIZER};

/***********************************************************************
 */
static const struct i915_tracked_state *atoms[] = {
   &i915_upload_S0S1, &i915_upload_S2S4, &i915_upload_S5, &i915_upload_S6,
   &i915_upload_S7};

static void
update_immediate(struct i915_context *i915)
{
   int i;

   for (i = 0; i < ARRAY_SIZE(atoms); i++)
      if (i915->dirty & atoms[i]->dirty)
         atoms[i]->update(i915);
}

struct i915_tracked_state i915_hw_immediate = {
   "immediate", update_immediate,
   ~0 /* all state atoms, because we do internal checking */
};
