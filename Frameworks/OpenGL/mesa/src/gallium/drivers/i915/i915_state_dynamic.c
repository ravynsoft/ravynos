/**************************************************************************
 *
 * Copyright 2003 VMware, Inc.
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

#include "i915_batch.h"
#include "i915_context.h"
#include "i915_reg.h"
#include "i915_state.h"
#include "i915_state_inlines.h"

#include "util/u_memory.h"
#include "util/u_pack_color.h"

/* State that we have chosen to store in the DYNAMIC segment of the
 * i915 indirect state mechanism.
 *
 * Can't cache these in the way we do the static state, as there is no
 * start/size in the command packet, instead an 'end' value that gets
 * incremented.
 *
 * Additionally, there seems to be a requirement to re-issue the full
 * (active) state every time a 4kb boundary is crossed.
 */

static inline void
set_dynamic(struct i915_context *i915, unsigned offset, const unsigned state)
{
   if (i915->current.dynamic[offset] == state)
      return;

   i915->current.dynamic[offset] = state;
   i915->dynamic_dirty |= 1 << offset;
   i915->hardware_dirty |= I915_HW_DYNAMIC;
}

static inline void
set_dynamic_array(struct i915_context *i915, unsigned offset,
                  const unsigned *src, unsigned dwords)
{
   unsigned i;

   if (!memcmp(src, &i915->current.dynamic[offset], dwords * 4))
      return;

   for (i = 0; i < dwords; i++) {
      i915->current.dynamic[offset + i] = src[i];
      i915->dynamic_dirty |= 1 << (offset + i);
   }

   i915->hardware_dirty |= I915_HW_DYNAMIC;
}

/***********************************************************************
 * Modes4: stencil masks and logicop
 */
static void
upload_MODES4(struct i915_context *i915)
{
   bool stencil_ccw = i915_stencil_ccw(i915);

   unsigned modes4 = 0;

   /* I915_NEW_STENCIL
    */
   if (stencil_ccw)
      modes4 |= i915->depth_stencil->stencil_modes4_ccw;
   else
      modes4 |= i915->depth_stencil->stencil_modes4_cw;

   /* I915_NEW_BLEND
    */
   modes4 |= i915->blend->modes4;

   set_dynamic(i915, I915_DYNAMIC_MODES4, modes4);
}

const struct i915_tracked_state i915_upload_MODES4 = {
   "MODES4", upload_MODES4,
   I915_NEW_BLEND | I915_NEW_DEPTH_STENCIL | I915_NEW_RASTERIZER};

/***********************************************************************
 */
static void
upload_BFO(struct i915_context *i915)
{
   bool stencil_ccw = i915_stencil_ccw(i915);

   unsigned bfo[2];
   if (stencil_ccw) {
      bfo[0] = i915->depth_stencil->bfo_ccw[0];
      bfo[1] = i915->depth_stencil->bfo_ccw[1];
   } else {
      bfo[0] = i915->depth_stencil->bfo_cw[0];
      bfo[1] = i915->depth_stencil->bfo_cw[1];
   }
   /* I don't get it only allowed to set a ref mask when the enable bit is set?
    */
   if (bfo[0] & BFO_ENABLE_STENCIL_REF) {
      bfo[0] |= i915->stencil_ref.ref_value[!stencil_ccw]
                << BFO_STENCIL_REF_SHIFT;
   }

   set_dynamic_array(i915, I915_DYNAMIC_BFO_0, bfo, 2);
}

const struct i915_tracked_state i915_upload_BFO = {
   "BFO", upload_BFO, I915_NEW_DEPTH_STENCIL | I915_NEW_RASTERIZER};

/***********************************************************************
 */
static void
upload_BLENDCOLOR(struct i915_context *i915)
{
   unsigned bc[2];

   memset(bc, 0, sizeof(bc));

   /* I915_NEW_BLEND
    */
   {
      const float *color = i915->blend_color.color;

      bc[0] = _3DSTATE_CONST_BLEND_COLOR_CMD;
      bc[1] = pack_ui32_float4(color[i915->current.color_swizzle[2]],
                               color[i915->current.color_swizzle[1]],
                               color[i915->current.color_swizzle[0]],
                               color[i915->current.color_swizzle[3]]);
   }

   set_dynamic_array(i915, I915_DYNAMIC_BC_0, bc, 2);
}

const struct i915_tracked_state i915_upload_BLENDCOLOR = {
   "BLENDCOLOR", upload_BLENDCOLOR, I915_NEW_BLEND | I915_NEW_COLOR_SWIZZLE};

/***********************************************************************
 */
static void
upload_IAB(struct i915_context *i915)
{
   unsigned iab = 0;

   if (i915->blend) {
      struct i915_surface *cbuf = i915_surface(i915->framebuffer.cbufs[0]);
      if (cbuf && cbuf->alpha_in_g)
         iab |= i915->blend->iab_alpha_in_g;
      else if (cbuf && cbuf->alpha_is_x)
         iab |= i915->blend->iab_alpha_is_x;
      else
         iab |= i915->blend->iab;
   }

   set_dynamic(i915, I915_DYNAMIC_IAB, iab);
}

const struct i915_tracked_state i915_upload_IAB = {
   "IAB", upload_IAB, I915_NEW_BLEND | I915_NEW_FRAMEBUFFER};

/***********************************************************************
 */
static void
upload_DEPTHSCALE(struct i915_context *i915)
{
   set_dynamic_array(i915, I915_DYNAMIC_DEPTHSCALE_0,
                     &i915->rasterizer->ds[0].u, 2);
}

const struct i915_tracked_state i915_upload_DEPTHSCALE = {
   "DEPTHSCALE", upload_DEPTHSCALE, I915_NEW_RASTERIZER};

/***********************************************************************
 * Polygon stipple
 *
 * The i915 supports a 4x4 stipple natively, GL wants 32x32.
 * Fortunately stipple is usually a repeating pattern.
 *
 * XXX: does stipple pattern need to be adjusted according to
 * the window position?
 *
 * XXX: possibly need workaround for conform paths test.
 */
static void
upload_STIPPLE(struct i915_context *i915)
{
   unsigned st[2];

   st[0] = _3DSTATE_STIPPLE;
   st[1] = 0;

   /* I915_NEW_RASTERIZER
    */
   if (i915->rasterizer)
      st[1] |= i915->rasterizer->st;

   /* I915_NEW_STIPPLE
    */
   {
      const uint8_t *mask = (const uint8_t *)i915->poly_stipple.stipple;
      uint8_t p[4];

      p[0] = mask[12] & 0xf;
      p[1] = mask[8] & 0xf;
      p[2] = mask[4] & 0xf;
      p[3] = mask[0] & 0xf;

      /* Not sure what to do about fallbacks, so for now just dont:
       */
      st[1] |= ((p[0] << 0) | (p[1] << 4) | (p[2] << 8) | (p[3] << 12));
   }

   set_dynamic_array(i915, I915_DYNAMIC_STP_0, st, 2);
}

const struct i915_tracked_state i915_upload_STIPPLE = {
   "STIPPLE", upload_STIPPLE, I915_NEW_RASTERIZER | I915_NEW_STIPPLE};

/***********************************************************************
 * Scissor enable
 */
static void
upload_SCISSOR_ENABLE(struct i915_context *i915)
{
   set_dynamic(i915, I915_DYNAMIC_SC_ENA_0, i915->rasterizer->sc[0]);
}

const struct i915_tracked_state i915_upload_SCISSOR_ENABLE = {
   "SCISSOR ENABLE", upload_SCISSOR_ENABLE, I915_NEW_RASTERIZER};

/***********************************************************************
 * Scissor rect
 */
static void
upload_SCISSOR_RECT(struct i915_context *i915)
{
   unsigned x1 = i915->scissor.minx;
   unsigned y1 = i915->scissor.miny;
   unsigned x2 = i915->scissor.maxx - 1;
   unsigned y2 = i915->scissor.maxy - 1;
   unsigned sc[3];

   sc[0] = _3DSTATE_SCISSOR_RECT_0_CMD;
   sc[1] = (y1 << 16) | (x1 & 0xffff);
   sc[2] = (y2 << 16) | (x2 & 0xffff);

   set_dynamic_array(i915, I915_DYNAMIC_SC_RECT_0, sc, 3);
}

const struct i915_tracked_state i915_upload_SCISSOR_RECT = {
   "SCISSOR RECT", upload_SCISSOR_RECT, I915_NEW_SCISSOR};

/***********************************************************************
 */
static const struct i915_tracked_state *atoms[] = {
   &i915_upload_MODES4,         &i915_upload_BFO,
   &i915_upload_BLENDCOLOR,     &i915_upload_IAB,
   &i915_upload_DEPTHSCALE,     &i915_upload_STIPPLE,
   &i915_upload_SCISSOR_ENABLE, &i915_upload_SCISSOR_RECT};

/* These will be dynamic indirect state commands, but for now just end
 * up on the batch buffer with everything else.
 */
static void
update_dynamic(struct i915_context *i915)
{
   int i;

   for (i = 0; i < ARRAY_SIZE(atoms); i++)
      if (i915->dirty & atoms[i]->dirty)
         atoms[i]->update(i915);
}

struct i915_tracked_state i915_hw_dynamic = {
   "dynamic", update_dynamic,
   ~0 /* all state atoms, because we do internal checking */
};
