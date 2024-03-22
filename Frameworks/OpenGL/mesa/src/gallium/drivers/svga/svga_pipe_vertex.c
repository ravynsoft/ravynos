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
#include "util/format/u_format.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_transfer.h"

#include "svga_context.h"
#include "svga_cmd.h"
#include "svga_format.h"
#include "svga_resource_buffer.h"
#include "svga_screen.h"


static void
svga_set_vertex_buffers(struct pipe_context *pipe,
                        unsigned count,
                        unsigned unbind_num_trailing_slots,
                        bool take_ownership,
                        const struct pipe_vertex_buffer *buffers)
{
   struct svga_context *svga = svga_context(pipe);

   util_set_vertex_buffers_count(svga->curr.vb,
                                 &svga->curr.num_vertex_buffers,
                                 buffers, count,
                                 unbind_num_trailing_slots,
                                 take_ownership);

   svga->dirty |= SVGA_NEW_VBUFFER;
}


/**
 * Does the given vertex attrib format need range adjustment in the VS?
 * Range adjustment scales and biases values from [0,1] to [-1,1].
 * This lets us avoid the swtnl path.
 */
static bool
attrib_needs_range_adjustment(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_R8G8B8_SNORM:
      return true;
   default:
      return false;
   }
}


/**
 * Given a gallium vertex element format, return the corresponding
 * SVGA3dDeclType.
 */
static SVGA3dDeclType
translate_vertex_format_to_decltype(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_R32_FLOAT:            return SVGA3D_DECLTYPE_FLOAT1;
   case PIPE_FORMAT_R32G32_FLOAT:         return SVGA3D_DECLTYPE_FLOAT2;
   case PIPE_FORMAT_R32G32B32_FLOAT:      return SVGA3D_DECLTYPE_FLOAT3;
   case PIPE_FORMAT_R32G32B32A32_FLOAT:   return SVGA3D_DECLTYPE_FLOAT4;
   case PIPE_FORMAT_B8G8R8A8_UNORM:       return SVGA3D_DECLTYPE_D3DCOLOR;
   case PIPE_FORMAT_R8G8B8A8_USCALED:     return SVGA3D_DECLTYPE_UBYTE4;
   case PIPE_FORMAT_R16G16_SSCALED:       return SVGA3D_DECLTYPE_SHORT2;
   case PIPE_FORMAT_R16G16B16A16_SSCALED: return SVGA3D_DECLTYPE_SHORT4;
   case PIPE_FORMAT_R8G8B8A8_UNORM:       return SVGA3D_DECLTYPE_UBYTE4N;
   case PIPE_FORMAT_R16G16_SNORM:         return SVGA3D_DECLTYPE_SHORT2N;
   case PIPE_FORMAT_R16G16B16A16_SNORM:   return SVGA3D_DECLTYPE_SHORT4N;
   case PIPE_FORMAT_R16G16_UNORM:         return SVGA3D_DECLTYPE_USHORT2N;
   case PIPE_FORMAT_R16G16B16A16_UNORM:   return SVGA3D_DECLTYPE_USHORT4N;
   case PIPE_FORMAT_R10G10B10X2_USCALED:  return SVGA3D_DECLTYPE_UDEC3;
   case PIPE_FORMAT_R10G10B10X2_SNORM:    return SVGA3D_DECLTYPE_DEC3N;
   case PIPE_FORMAT_R16G16_FLOAT:         return SVGA3D_DECLTYPE_FLOAT16_2;
   case PIPE_FORMAT_R16G16B16A16_FLOAT:   return SVGA3D_DECLTYPE_FLOAT16_4;

   /* See attrib_needs_adjustment() and attrib_needs_w_to_1() above */
   case PIPE_FORMAT_R8G8B8_SNORM:         return SVGA3D_DECLTYPE_UBYTE4N;

   /* See attrib_needs_w_to_1() above */
   case PIPE_FORMAT_R16G16B16_SNORM:      return SVGA3D_DECLTYPE_SHORT4N;
   case PIPE_FORMAT_R16G16B16_UNORM:      return SVGA3D_DECLTYPE_USHORT4N;
   case PIPE_FORMAT_R8G8B8_UNORM:         return SVGA3D_DECLTYPE_UBYTE4N;

   default:
      /* There are many formats without hardware support.  This case
       * will be hit regularly, meaning we'll need swvfetch.
       */
      return SVGA3D_DECLTYPE_MAX;
   }
}


static void
define_input_element_object(struct svga_context *svga,
                            struct svga_velems_state *velems)
{
   SVGA3dInputElementDesc elements[PIPE_MAX_ATTRIBS];
   unsigned i;

   assert(velems->count <= PIPE_MAX_ATTRIBS);
   assert(svga_have_vgpu10(svga));

   for (i = 0; i < velems->count; i++) {
      const struct pipe_vertex_element *elem = velems->velem + i;
      SVGA3dSurfaceFormat svga_format;
      unsigned vf_flags;

      svga_translate_vertex_format_vgpu10(elem->src_format,
                                          &svga_format, &vf_flags);

      velems->decl_type[i] =
         translate_vertex_format_to_decltype(elem->src_format);
      elements[i].inputSlot = elem->vertex_buffer_index;
      elements[i].alignedByteOffset = elem->src_offset;
      elements[i].format = svga_format;

      if (elem->instance_divisor) {
         elements[i].inputSlotClass = SVGA3D_INPUT_PER_INSTANCE_DATA;
         elements[i].instanceDataStepRate = elem->instance_divisor;
      }
      else {
         elements[i].inputSlotClass = SVGA3D_INPUT_PER_VERTEX_DATA;
         elements[i].instanceDataStepRate = 0;
      }
      elements[i].inputRegister = i;

      if (elements[i].format == SVGA3D_FORMAT_INVALID) {
         velems->need_swvfetch = true;
      }

      if (util_format_is_pure_integer(elem->src_format)) {
         velems->attrib_is_pure_int |= (1 << i);
      }

      if (vf_flags & VF_W_TO_1) {
         velems->adjust_attrib_w_1 |= (1 << i);
      }

      if (vf_flags & VF_U_TO_F_CAST) {
         velems->adjust_attrib_utof |= (1 << i);
      }
      else if (vf_flags & VF_I_TO_F_CAST) {
         velems->adjust_attrib_itof |= (1 << i);
      }

      if (vf_flags & VF_BGRA) {
         velems->attrib_is_bgra |= (1 << i);
      }

      if (vf_flags & VF_PUINT_TO_SNORM) {
         velems->attrib_puint_to_snorm |= (1 << i);
      }
      else if (vf_flags & VF_PUINT_TO_USCALED) {
         velems->attrib_puint_to_uscaled |= (1 << i);
      }
      else if (vf_flags & VF_PUINT_TO_SSCALED) {
         velems->attrib_puint_to_sscaled |= (1 << i);
      }
   }

   velems->id = util_bitmask_add(svga->input_element_object_id_bm);

   SVGA_RETRY(svga, SVGA3D_vgpu10_DefineElementLayout(svga->swc, velems->count,
                                                      velems->id, elements));
}


/**
 * Translate the vertex element types to SVGA3dDeclType and check
 * for VS-based vertex attribute adjustments.
 */
static void
translate_vertex_decls(struct svga_context *svga,
                       struct svga_velems_state *velems)
{
   unsigned i;

   assert(!svga_have_vgpu10(svga));

   for (i = 0; i < velems->count; i++) {
      const enum pipe_format f = velems->velem[i].src_format;
      SVGA3dSurfaceFormat svga_format;
      unsigned vf_flags;

      svga_translate_vertex_format_vgpu10(f, &svga_format, &vf_flags);

      velems->decl_type[i] = translate_vertex_format_to_decltype(f);
      if (velems->decl_type[i] == SVGA3D_DECLTYPE_MAX) {
         /* Unsupported format - use software fetch */
         velems->need_swvfetch = true;
      }

      /* Check for VS-based adjustments */
      if (attrib_needs_range_adjustment(f)) {
         velems->adjust_attrib_range |= (1 << i);
      }

      if (vf_flags & VF_W_TO_1) {
         velems->adjust_attrib_w_1 |= (1 << i);
      }
   }
}


static void *
svga_create_vertex_elements_state(struct pipe_context *pipe,
                                  unsigned count,
                                  const struct pipe_vertex_element *attribs)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_velems_state *velems;

   assert(count <= PIPE_MAX_ATTRIBS);
   velems = (struct svga_velems_state *) MALLOC(sizeof(struct svga_velems_state));
   if (velems) {
      velems->count = count;
      memcpy(velems->velem, attribs, sizeof(*attribs) * count);

      velems->need_swvfetch = false;
      velems->adjust_attrib_range = 0x0;
      velems->attrib_is_pure_int = 0x0;
      velems->adjust_attrib_w_1 = 0x0;
      velems->adjust_attrib_itof = 0x0;
      velems->adjust_attrib_utof = 0x0;
      velems->attrib_is_bgra = 0x0;
      velems->attrib_puint_to_snorm = 0x0;
      velems->attrib_puint_to_uscaled = 0x0;
      velems->attrib_puint_to_sscaled = 0x0;

      if (svga_have_vgpu10(svga)) {
         define_input_element_object(svga, velems);
      }
      else {
         translate_vertex_decls(svga, velems);
      }
      for (unsigned i = 0; i < count; i++)
         velems->strides[attribs[i].vertex_buffer_index] = attribs[i].src_stride;
   }

   svga->hud.num_vertexelement_objects++;
   SVGA_STATS_COUNT_INC(svga_screen(svga->pipe.screen)->sws,
                        SVGA_STATS_COUNT_VERTEXELEMENT);

   return velems;
}


static void
svga_bind_vertex_elements_state(struct pipe_context *pipe, void *state)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_velems_state *velems = (struct svga_velems_state *) state;

   svga->curr.velems = velems;
   svga->dirty |= SVGA_NEW_VELEMENT;
}


static void
svga_delete_vertex_elements_state(struct pipe_context *pipe, void *state)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_velems_state *velems = (struct svga_velems_state *) state;

   if (svga_have_vgpu10(svga)) {
      svga_hwtnl_flush_retry(svga);

      SVGA_RETRY(svga, SVGA3D_vgpu10_DestroyElementLayout(svga->swc,
                                                          velems->id));

      if (velems->id == svga->state.hw_draw.layout_id)
         svga->state.hw_draw.layout_id = SVGA3D_INVALID_ID;

      util_bitmask_clear(svga->input_element_object_id_bm, velems->id);
      velems->id = SVGA3D_INVALID_ID;
   }

   FREE(velems);
   svga->hud.num_vertexelement_objects--;
}


void
svga_cleanup_vertex_state(struct svga_context *svga)
{
   unsigned i;

   for (i = 0 ; i < svga->curr.num_vertex_buffers; i++)
      pipe_vertex_buffer_unreference(&svga->curr.vb[i]);

   pipe_resource_reference(&svga->state.hw_draw.ib, NULL);

   for (i = 0; i < svga->state.hw_draw.num_vbuffers; i++)
      pipe_resource_reference(&svga->state.hw_draw.vbuffers[i], NULL);
}


void
svga_init_vertex_functions(struct svga_context *svga)
{
   svga->pipe.set_vertex_buffers = svga_set_vertex_buffers;
   svga->pipe.create_vertex_elements_state = svga_create_vertex_elements_state;
   svga->pipe.bind_vertex_elements_state = svga_bind_vertex_elements_state;
   svga->pipe.delete_vertex_elements_state = svga_delete_vertex_elements_state;
}
