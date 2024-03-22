/*
 * Copyright © 2022 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef GENX_CMD_DRAW_HELPERS_H
#define GENX_CMD_DRAW_HELPERS_H

#include <assert.h>
#include <stdbool.h>

#include "anv_private.h"

#if GFX_VER < 11
static void
emit_vertex_bo(struct anv_cmd_buffer *cmd_buffer,
               struct anv_address addr,
               uint32_t size, uint32_t index)
{
   uint32_t *p = anv_batch_emitn(&cmd_buffer->batch, 5,
                                 GENX(3DSTATE_VERTEX_BUFFERS));

   GENX(VERTEX_BUFFER_STATE_pack)(&cmd_buffer->batch, p + 1,
      &(struct GENX(VERTEX_BUFFER_STATE)) {
         .VertexBufferIndex = index,
         .AddressModifyEnable = true,
         .BufferPitch = 0,
         .MOCS = anv_mocs(cmd_buffer->device, addr.bo,
                          ISL_SURF_USAGE_VERTEX_BUFFER_BIT),
         .NullVertexBuffer = size == 0,
         .BufferStartingAddress = addr,
         .BufferSize = size
      });

#if GFX_VER == 9
   genX(cmd_buffer_set_binding_for_gfx8_vb_flush)(cmd_buffer,
                                                  index, addr, size);
#endif
}

static void
emit_base_vertex_instance_bo(struct anv_cmd_buffer *cmd_buffer,
                             struct anv_address addr)
{
   emit_vertex_bo(cmd_buffer, addr, addr.bo ? 8 : 0, ANV_SVGS_VB_INDEX);
}

static void
emit_base_vertex_instance(struct anv_cmd_buffer *cmd_buffer,
                          uint32_t base_vertex, uint32_t base_instance)
{
   if (base_vertex == 0 && base_instance == 0) {
      emit_base_vertex_instance_bo(cmd_buffer, ANV_NULL_ADDRESS);
      return;
   }

   struct anv_state id_state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, 8, 4);

   ((uint32_t *)id_state.map)[0] = base_vertex;
   ((uint32_t *)id_state.map)[1] = base_instance;

   struct anv_address addr =
      anv_state_pool_state_address(&cmd_buffer->device->dynamic_state_pool,
                                    id_state);

   emit_base_vertex_instance_bo(cmd_buffer, addr);
}

static void
emit_draw_index(struct anv_cmd_buffer *cmd_buffer, uint32_t draw_index)
{
   struct anv_state state =
      anv_cmd_buffer_alloc_dynamic_state(cmd_buffer, 4, 4);

   ((uint32_t *)state.map)[0] = draw_index;

   struct anv_address addr =
      anv_state_pool_state_address(&cmd_buffer->device->dynamic_state_pool,
                                   state);

   emit_vertex_bo(cmd_buffer, addr, 4, ANV_DRAWID_VB_INDEX);
}
#endif /* GFX_VER <= 11 */

static void
update_dirty_vbs_for_gfx8_vb_flush(struct anv_cmd_buffer *cmd_buffer,
                                   uint32_t access_type)
{
#if GFX_VER == 9
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;
   struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(cmd_buffer->state.gfx.base.pipeline);
   const struct brw_vs_prog_data *vs_prog_data = get_vs_prog_data(pipeline);

   uint64_t vb_used = dyn->vi->bindings_valid;
   if (vs_prog_data->uses_firstvertex ||
       vs_prog_data->uses_baseinstance)
      vb_used |= 1ull << ANV_SVGS_VB_INDEX;
   if (vs_prog_data->uses_drawid)
      vb_used |= 1ull << ANV_DRAWID_VB_INDEX;

   genX(cmd_buffer_update_dirty_vbs_for_gfx8_vb_flush)(cmd_buffer,
                                                       access_type,
                                                       vb_used);
#endif
}

#if GFX_VER < 11
ALWAYS_INLINE static void
cmd_buffer_emit_vertex_constants_and_flush(struct anv_cmd_buffer *cmd_buffer,
                                           const struct brw_vs_prog_data *vs_prog_data,
                                           uint32_t base_vertex,
                                           uint32_t base_instance,
                                           uint32_t draw_id,
                                           bool force_flush)
{
   bool emitted = false;
   if (vs_prog_data->uses_firstvertex ||
       vs_prog_data->uses_baseinstance) {
      emit_base_vertex_instance(cmd_buffer, base_vertex, base_instance);
      emitted = true;
   }
   if (vs_prog_data->uses_drawid) {
      emit_draw_index(cmd_buffer, draw_id);
      emitted = true;
   }
   /* Emitting draw index or vertex index BOs may result in needing
    * additional VF cache flushes.
    */
   if (emitted || force_flush)
      genX(cmd_buffer_apply_pipe_flushes)(cmd_buffer);
}
#endif

#endif /* GENX_CMD_DRAW_HELPERS_H */
