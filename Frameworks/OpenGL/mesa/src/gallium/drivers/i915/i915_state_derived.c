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

#include "draw/draw_context.h"
#include "draw/draw_vertex.h"
#include "pipe/p_shader_tokens.h"
#include "util/log.h"
#include "util/u_memory.h"
#include "i915_context.h"
#include "i915_debug.h"
#include "i915_fpc.h"
#include "i915_reg.h"
#include "i915_state.h"

/***********************************************************************
 * Determine the hardware vertex layout.
 * Depends on vertex/fragment shader state.
 */
static void
calculate_vertex_layout(struct i915_context *i915)
{
   const struct i915_fragment_shader *fs = i915->fs;
   struct i915_vertex_info vinfo;
   bool colors[2], fog, needW, face;
   uint32_t i;
   int src;

   colors[0] = colors[1] = fog = needW = face = false;
   memset(&vinfo, 0, sizeof(vinfo));

   /* Determine which fragment program inputs are needed.  Setup HW vertex
    * layout below, in the HW-specific attribute order.
    */
   for (i = 0; i < fs->info.num_inputs; i++) {
      switch (fs->info.input_semantic_name[i]) {
      case TGSI_SEMANTIC_POSITION:
      case TGSI_SEMANTIC_PCOORD:
      case TGSI_SEMANTIC_FACE:
         /* Handled as texcoord inputs below */
         break;
      case TGSI_SEMANTIC_COLOR:
         assert(fs->info.input_semantic_index[i] < 2);
         colors[fs->info.input_semantic_index[i]] = true;
         break;
      case TGSI_SEMANTIC_TEXCOORD:
      case TGSI_SEMANTIC_GENERIC:
         needW = true;
         break;
      case TGSI_SEMANTIC_FOG:
         fog = true;
         break;
      default:
         debug_printf("Unknown input type %d\n",
                      fs->info.input_semantic_name[i]);
         assert(0);
      }
   }

   /* pos */
   src = draw_find_shader_output(i915->draw, TGSI_SEMANTIC_POSITION, 0);
   if (needW) {
      draw_emit_vertex_attr(&vinfo.draw, EMIT_4F, src);
      vinfo.hwfmt[0] |= S4_VFMT_XYZW;
      vinfo.draw.attrib[0].emit = EMIT_4F;
   } else {
      draw_emit_vertex_attr(&vinfo.draw, EMIT_3F, src);
      vinfo.hwfmt[0] |= S4_VFMT_XYZ;
      vinfo.draw.attrib[0].emit = EMIT_3F;
   }

   /* point size.  if not emitted here, then point size comes from LIS4. */
   if (i915->rasterizer->templ.point_size_per_vertex) {
      src = draw_find_shader_output(i915->draw, TGSI_SEMANTIC_PSIZE, 0);
      if (src != -1) {
         draw_emit_vertex_attr(&vinfo.draw, EMIT_1F, src);
         vinfo.hwfmt[0] |= S4_VFMT_POINT_WIDTH;
      }
   }

   /* primary color */
   if (colors[0]) {
      src = draw_find_shader_output(i915->draw, TGSI_SEMANTIC_COLOR, 0);
      draw_emit_vertex_attr(&vinfo.draw, EMIT_4UB_BGRA, src);
      vinfo.hwfmt[0] |= S4_VFMT_COLOR;
   }

   /* secondary color */
   if (colors[1]) {
      src = draw_find_shader_output(i915->draw, TGSI_SEMANTIC_COLOR, 1);
      draw_emit_vertex_attr(&vinfo.draw, EMIT_4UB_BGRA, src);
      vinfo.hwfmt[0] |= S4_VFMT_SPEC_FOG;
   }

   /* fog coord, not fog blend factor */
   if (fog) {
      src = draw_find_shader_output(i915->draw, TGSI_SEMANTIC_FOG, 0);
      draw_emit_vertex_attr(&vinfo.draw, EMIT_1F, src);
      vinfo.hwfmt[0] |= S4_VFMT_FOG_PARAM;
   }

   /* texcoords/varyings */
   for (i = 0; i < I915_TEX_UNITS; i++) {
      uint32_t hwtc;
      if (fs->texcoords[i].semantic != -1) {
         src = draw_find_shader_output(i915->draw, fs->texcoords[i].semantic,
                                       fs->texcoords[i].index);
         if (fs->texcoords[i].semantic == TGSI_SEMANTIC_FACE) {
            /* XXX Because of limitations in the draw module, currently src will
             * be 0 for SEMANTIC_FACE, so this aliases to POS. We need to fix in
             * the draw module by adding an extra shader output.
             */
            mesa_loge("Front/back face is broken\n");
            draw_emit_vertex_attr(&vinfo.draw, EMIT_1F, src);
            hwtc = TEXCOORDFMT_1D;
         } else {
            hwtc = TEXCOORDFMT_4D;
            draw_emit_vertex_attr(&vinfo.draw, EMIT_4F, src);
         }
      } else {
         hwtc = TEXCOORDFMT_NOT_PRESENT;
      }
      vinfo.hwfmt[1] |= hwtc << (i * 4);
   }

   draw_compute_vertex_size(&vinfo.draw);

   if (memcmp(&i915->current.vertex_info, &vinfo, sizeof(vinfo))) {
      /* Need to set this flag so that the LIS2/4 registers get set.
       * It also means the i915_update_immediate() function must be called
       * after this one, in i915_update_derived().
       */
      i915->dirty |= I915_NEW_VERTEX_FORMAT;

      memcpy(&i915->current.vertex_info, &vinfo, sizeof(vinfo));
   }
}

struct i915_tracked_state i915_update_vertex_layout = {
   "vertex_layout", calculate_vertex_layout,
   I915_NEW_RASTERIZER | I915_NEW_FS | I915_NEW_VS};

/***********************************************************************
 */
static struct i915_tracked_state *atoms[] = {
   &i915_update_vertex_layout, &i915_hw_samplers,  &i915_hw_immediate,
   &i915_hw_dynamic,           &i915_hw_fs,        &i915_hw_framebuffer,
   &i915_hw_dst_buf_vars,      &i915_hw_constants, NULL,
};

void
i915_update_derived(struct i915_context *i915)
{
   int i;

   if (I915_DBG_ON(DBG_ATOMS))
      i915_dump_dirty(i915, __func__);

   if (!i915->fs) {
      i915->dirty &= ~(I915_NEW_FS_CONSTANTS | I915_NEW_FS);
      i915->hardware_dirty &= ~(I915_HW_PROGRAM | I915_HW_CONSTANTS);
   }

   if (!i915->vs)
      i915->dirty &= ~I915_NEW_VS;

   if (!i915->blend)
      i915->dirty &= ~I915_NEW_BLEND;

   if (!i915->rasterizer)
      i915->dirty &= ~I915_NEW_RASTERIZER;

   if (!i915->depth_stencil)
      i915->dirty &= ~I915_NEW_DEPTH_STENCIL;

   for (i = 0; atoms[i]; i++)
      if (atoms[i]->dirty & i915->dirty)
         atoms[i]->update(i915);

   i915->dirty = 0;
}
