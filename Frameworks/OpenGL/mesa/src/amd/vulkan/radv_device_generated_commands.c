/*
 * Copyright © 2021 Google
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

#include "meta/radv_meta.h"
#include "radv_private.h"

#include "ac_rgp.h"

#include "nir_builder.h"

#include "vk_common_entrypoints.h"

static void
radv_get_sequence_size_compute(const struct radv_indirect_command_layout *layout,
                               const struct radv_compute_pipeline *pipeline, uint32_t *cmd_size)
{
   const struct radv_device *device = container_of(layout->base.device, struct radv_device, vk);
   struct radv_shader *cs = radv_get_shader(pipeline->base.shaders, MESA_SHADER_COMPUTE);

   /* dispatch */
   *cmd_size += 5 * 4;

   const struct radv_userdata_info *loc = radv_get_user_sgpr(cs, AC_UD_CS_GRID_SIZE);
   if (loc->sgpr_idx != -1) {
      if (device->load_grid_size_from_user_sgpr) {
         /* PKT3_SET_SH_REG for immediate values */
         *cmd_size += 5 * 4;
      } else {
         /* PKT3_SET_SH_REG for pointer */
         *cmd_size += 4 * 4;
      }
   }

   if (device->sqtt.bo) {
      /* sqtt markers */
      *cmd_size += 8 * 3 * 4;
   }
}

static void
radv_get_sequence_size_graphics(const struct radv_indirect_command_layout *layout,
                                const struct radv_graphics_pipeline *pipeline, uint32_t *cmd_size,
                                uint32_t *upload_size)
{
   const struct radv_device *device = container_of(layout->base.device, struct radv_device, vk);
   const struct radv_shader *vs = radv_get_shader(pipeline->base.shaders, MESA_SHADER_VERTEX);

   if (layout->bind_vbo_mask) {
      *upload_size += 16 * util_bitcount(vs->info.vs.vb_desc_usage_mask);

      /* One PKT3_SET_SH_REG for emitting VBO pointer (32-bit) */
      *cmd_size += 3 * 4;
   }

   if (layout->binds_index_buffer) {
      /* Index type write (normal reg write) + index buffer base write (64-bits, but special packet
       * so only 1 word overhead) + index buffer size (again, special packet so only 1 word
       * overhead)
       */
      *cmd_size += (3 + 3 + 2) * 4;
   }

   if (layout->indexed) {
      if (layout->binds_index_buffer) {
         /* userdata writes + instance count + indexed draw */
         *cmd_size += (5 + 2 + 5) * 4;
      } else {
         /* PKT3_SET_BASE + PKT3_DRAW_{INDEX}_INDIRECT_MULTI */
         *cmd_size += (4 + (pipeline->uses_drawid ? 10 : 5)) * 4;
      }
   } else {
      if (layout->draw_mesh_tasks) {
         /* userdata writes + instance count + non-indexed draw */
         *cmd_size += (6 + 2 + (device->mesh_fast_launch_2 ? 5 : 3)) * 4;
      } else {
         /* userdata writes + instance count + non-indexed draw */
         *cmd_size += (5 + 2 + 3) * 4;
      }
   }

   if (device->sqtt.bo) {
      /* sqtt markers */
      *cmd_size += 5 * 3 * 4;
   }
}

static void
radv_get_sequence_size(const struct radv_indirect_command_layout *layout, struct radv_pipeline *pipeline,
                       uint32_t *cmd_size, uint32_t *upload_size)
{
   const struct radv_device *device = container_of(layout->base.device, struct radv_device, vk);

   *cmd_size = 0;
   *upload_size = 0;

   if (layout->push_constant_mask) {
      bool need_copy = false;

      for (unsigned i = 0; i < ARRAY_SIZE(pipeline->shaders); ++i) {
         if (!pipeline->shaders[i])
            continue;

         struct radv_userdata_locations *locs = &pipeline->shaders[i]->info.user_sgprs_locs;
         if (locs->shader_data[AC_UD_PUSH_CONSTANTS].sgpr_idx >= 0) {
            /* One PKT3_SET_SH_REG for emitting push constants pointer (32-bit) */
            *cmd_size += 3 * 4;
            need_copy = true;
         }
         if (locs->shader_data[AC_UD_INLINE_PUSH_CONSTANTS].sgpr_idx >= 0)
            /* One PKT3_SET_SH_REG writing all inline push constants. */
            *cmd_size += (3 * util_bitcount64(layout->push_constant_mask)) * 4;
      }
      if (need_copy)
         *upload_size += align(pipeline->push_constant_size + 16 * pipeline->dynamic_offset_count, 16);
   }

   if (device->sqtt.bo) {
      /* THREAD_TRACE_MARKER */
      *cmd_size += 2 * 4;
   }

   if (layout->pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
      struct radv_graphics_pipeline *graphics_pipeline = radv_pipeline_to_graphics(pipeline);
      radv_get_sequence_size_graphics(layout, graphics_pipeline, cmd_size, upload_size);
   } else {
      assert(layout->pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE);
      struct radv_compute_pipeline *compute_pipeline = radv_pipeline_to_compute(pipeline);
      radv_get_sequence_size_compute(layout, compute_pipeline, cmd_size);
   }
}

static uint32_t
radv_align_cmdbuf_size(const struct radv_device *device, uint32_t size, enum amd_ip_type ip_type)
{
   const uint32_t ib_alignment = device->physical_device->rad_info.ip[ip_type].ib_alignment;

   return align(size, ib_alignment);
}

static unsigned
radv_dgc_preamble_cmdbuf_size(const struct radv_device *device)
{
   return radv_align_cmdbuf_size(device, 16, AMD_IP_GFX);
}

static bool
radv_dgc_use_preamble(const VkGeneratedCommandsInfoNV *cmd_info)
{
   /* Heuristic on when the overhead for the preamble (i.e. double jump) is worth it. Obviously
    * a bit of a guess as it depends on the actual count which we don't know. */
   return cmd_info->sequencesCountBuffer != VK_NULL_HANDLE && cmd_info->sequencesCount >= 64;
}

uint32_t
radv_get_indirect_cmdbuf_size(const VkGeneratedCommandsInfoNV *cmd_info)
{
   VK_FROM_HANDLE(radv_indirect_command_layout, layout, cmd_info->indirectCommandsLayout);
   VK_FROM_HANDLE(radv_pipeline, pipeline, cmd_info->pipeline);
   const struct radv_device *device = container_of(layout->base.device, struct radv_device, vk);

   if (radv_dgc_use_preamble(cmd_info))
      return radv_dgc_preamble_cmdbuf_size(device);

   uint32_t cmd_size, upload_size;
   radv_get_sequence_size(layout, pipeline, &cmd_size, &upload_size);
   return radv_align_cmdbuf_size(device, cmd_size * cmd_info->sequencesCount, AMD_IP_GFX);
}

struct radv_dgc_params {
   uint32_t cmd_buf_stride;
   uint32_t cmd_buf_size;
   uint32_t upload_stride;
   uint32_t upload_addr;
   uint32_t sequence_count;
   uint32_t stream_stride;
   uint64_t stream_addr;

   /* draw info */
   uint16_t draw_indexed;
   uint16_t draw_params_offset;
   uint16_t binds_index_buffer;
   uint16_t vtx_base_sgpr;
   uint32_t max_index_count;
   uint8_t draw_mesh_tasks;

   /* dispatch info */
   uint32_t dispatch_initiator;
   uint16_t dispatch_params_offset;
   uint16_t grid_base_sgpr;

   /* bind index buffer info. Valid if binds_index_buffer == true && draw_indexed */
   uint16_t index_buffer_offset;

   uint8_t vbo_cnt;

   uint8_t const_copy;

   /* Which VBOs are set in this indirect layout. */
   uint32_t vbo_bind_mask;

   uint16_t vbo_reg;
   uint16_t const_copy_size;

   uint64_t push_constant_mask;

   uint32_t ibo_type_32;
   uint32_t ibo_type_8;

   uint16_t push_constant_shader_cnt;

   uint8_t is_dispatch;
   uint8_t use_preamble;
};

enum {
   DGC_USES_DRAWID = 1u << 14,
   DGC_USES_BASEINSTANCE = 1u << 15,
   DGC_USES_GRID_SIZE = DGC_USES_BASEINSTANCE, /* Mesh shader only */
};

enum {
   DGC_DYNAMIC_STRIDE = 1u << 15,
};

enum {
   DGC_DESC_STREAM,
   DGC_DESC_PREPARE,
   DGC_DESC_PARAMS,
   DGC_DESC_COUNT,
   DGC_NUM_DESCS,
};

struct dgc_cmdbuf {
   nir_def *descriptor;
   nir_variable *offset;

   enum amd_gfx_level gfx_level;
   bool sqtt_enabled;
};

static void
dgc_emit(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *value)
{
   assert(value->bit_size >= 32);
   nir_def *offset = nir_load_var(b, cs->offset);
   nir_store_ssbo(b, value, cs->descriptor, offset, .access = ACCESS_NON_READABLE);
   nir_store_var(b, cs->offset, nir_iadd_imm(b, offset, value->num_components * value->bit_size / 8), 0x1);
}

#define load_param32(b, field)                                                                                         \
   nir_load_push_constant((b), 1, 32, nir_imm_int((b), 0), .base = offsetof(struct radv_dgc_params, field), .range = 4)

#define load_param16(b, field)                                                                                         \
   nir_ubfe_imm((b),                                                                                                   \
                nir_load_push_constant((b), 1, 32, nir_imm_int((b), 0),                                                \
                                       .base = (offsetof(struct radv_dgc_params, field) & ~3), .range = 4),            \
                (offsetof(struct radv_dgc_params, field) & 2) * 8, 16)

#define load_param8(b, field)                                                                                          \
   nir_ubfe_imm((b),                                                                                                   \
                nir_load_push_constant((b), 1, 32, nir_imm_int((b), 0),                                                \
                                       .base = (offsetof(struct radv_dgc_params, field) & ~3), .range = 4),            \
                (offsetof(struct radv_dgc_params, field) & 3) * 8, 8)

#define load_param64(b, field)                                                                                         \
   nir_pack_64_2x32((b), nir_load_push_constant((b), 2, 32, nir_imm_int((b), 0),                                       \
                                                .base = offsetof(struct radv_dgc_params, field), .range = 8))

static nir_def *
nir_pkt3_base(nir_builder *b, unsigned op, nir_def *len, bool predicate)
{
   len = nir_iand_imm(b, len, 0x3fff);
   return nir_ior_imm(b, nir_ishl_imm(b, len, 16), PKT_TYPE_S(3) | PKT3_IT_OPCODE_S(op) | PKT3_PREDICATE(predicate));
}

static nir_def *
nir_pkt3(nir_builder *b, unsigned op, nir_def *len)
{
   return nir_pkt3_base(b, op, len, false);
}

static nir_def *
dgc_get_nop_packet(nir_builder *b, const struct radv_device *device)
{
   if (device->physical_device->rad_info.gfx_ib_pad_with_type2) {
      return nir_imm_int(b, PKT2_NOP_PAD);
   } else {
      return nir_imm_int(b, PKT3_NOP_PAD);
   }
}

static void
dgc_emit_userdata_vertex(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *vtx_base_sgpr, nir_def *first_vertex,
                         nir_def *first_instance, nir_def *drawid, const struct radv_device *device)
{
   vtx_base_sgpr = nir_u2u32(b, vtx_base_sgpr);
   nir_def *has_drawid = nir_test_mask(b, vtx_base_sgpr, DGC_USES_DRAWID);
   nir_def *has_baseinstance = nir_test_mask(b, vtx_base_sgpr, DGC_USES_BASEINSTANCE);

   nir_def *pkt_cnt = nir_imm_int(b, 1);
   pkt_cnt = nir_bcsel(b, has_drawid, nir_iadd_imm(b, pkt_cnt, 1), pkt_cnt);
   pkt_cnt = nir_bcsel(b, has_baseinstance, nir_iadd_imm(b, pkt_cnt, 1), pkt_cnt);

   nir_def *values[5] = {
      nir_pkt3(b, PKT3_SET_SH_REG, pkt_cnt), nir_iand_imm(b, vtx_base_sgpr, 0x3FFF), first_vertex,
      dgc_get_nop_packet(b, device),         dgc_get_nop_packet(b, device),
   };

   values[3] = nir_bcsel(b, nir_ior(b, has_drawid, has_baseinstance), nir_bcsel(b, has_drawid, drawid, first_instance),
                         values[4]);
   values[4] = nir_bcsel(b, nir_iand(b, has_drawid, has_baseinstance), first_instance, values[4]);

   dgc_emit(b, cs, nir_vec(b, values, 5));
}

static void
dgc_emit_userdata_mesh(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *vtx_base_sgpr, nir_def *x, nir_def *y,
                       nir_def *z, nir_def *drawid, const struct radv_device *device)
{
   vtx_base_sgpr = nir_u2u32(b, vtx_base_sgpr);
   nir_def *has_grid_size = nir_test_mask(b, vtx_base_sgpr, DGC_USES_GRID_SIZE);
   nir_def *has_drawid = nir_test_mask(b, vtx_base_sgpr, DGC_USES_DRAWID);

   nir_push_if(b, nir_ior(b, has_grid_size, has_drawid));
   {
      nir_def *pkt_cnt = nir_imm_int(b, 0);
      pkt_cnt = nir_bcsel(b, has_grid_size, nir_iadd_imm(b, pkt_cnt, 3), pkt_cnt);
      pkt_cnt = nir_bcsel(b, has_drawid, nir_iadd_imm(b, pkt_cnt, 1), pkt_cnt);

      nir_def *values[6] = {
         nir_pkt3(b, PKT3_SET_SH_REG, pkt_cnt), nir_iand_imm(b, vtx_base_sgpr, 0x3FFF), dgc_get_nop_packet(b, device),
         dgc_get_nop_packet(b, device),         dgc_get_nop_packet(b, device),          dgc_get_nop_packet(b, device),
      };

      /* DrawID needs to be first if no GridSize. */
      values[2] = nir_bcsel(b, has_grid_size, x, drawid);
      values[3] = nir_bcsel(b, has_grid_size, y, values[3]);
      values[4] = nir_bcsel(b, has_grid_size, z, values[4]);
      values[5] = nir_bcsel(b, has_drawid, drawid, values[5]);

      for (uint32_t i = 0; i < ARRAY_SIZE(values); i++)
         dgc_emit(b, cs, values[i]);
   }
   nir_pop_if(b, NULL);
}

static void
dgc_emit_sqtt_userdata(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *data)
{
   if (!cs->sqtt_enabled)
      return;

   nir_def *values[3] = {
      nir_pkt3_base(b, PKT3_SET_UCONFIG_REG, nir_imm_int(b, 1), cs->gfx_level >= GFX10),
      nir_imm_int(b, (R_030D08_SQ_THREAD_TRACE_USERDATA_2 - CIK_UCONFIG_REG_OFFSET) >> 2),
      data,
   };

   dgc_emit(b, cs, nir_vec(b, values, 3));
}

static void
dgc_emit_sqtt_thread_trace_marker(nir_builder *b, struct dgc_cmdbuf *cs)
{
   if (!cs->sqtt_enabled)
      return;

   nir_def *values[2] = {
      nir_pkt3(b, PKT3_EVENT_WRITE, nir_imm_int(b, 0)),
      nir_imm_int(b, EVENT_TYPE(V_028A90_THREAD_TRACE_MARKER | EVENT_INDEX(0))),
   };

   dgc_emit(b, cs, nir_vec(b, values, 2));
}

static void
dgc_emit_sqtt_marker_event(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *sequence_id,
                           enum rgp_sqtt_marker_event_type event)
{
   struct rgp_sqtt_marker_event marker = {0};

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_EVENT;
   marker.api_type = event;

   dgc_emit_sqtt_userdata(b, cs, nir_imm_int(b, marker.dword01));
   dgc_emit_sqtt_userdata(b, cs, nir_imm_int(b, marker.dword02));
   dgc_emit_sqtt_userdata(b, cs, sequence_id);
}

static void
dgc_emit_sqtt_marker_event_with_dims(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *sequence_id, nir_def *x,
                                     nir_def *y, nir_def *z, enum rgp_sqtt_marker_event_type event)
{
   struct rgp_sqtt_marker_event_with_dims marker = {0};

   marker.event.identifier = RGP_SQTT_MARKER_IDENTIFIER_EVENT;
   marker.event.api_type = event;
   marker.event.has_thread_dims = 1;

   dgc_emit_sqtt_userdata(b, cs, nir_imm_int(b, marker.event.dword01));
   dgc_emit_sqtt_userdata(b, cs, nir_imm_int(b, marker.event.dword02));
   dgc_emit_sqtt_userdata(b, cs, sequence_id);
   dgc_emit_sqtt_userdata(b, cs, x);
   dgc_emit_sqtt_userdata(b, cs, y);
   dgc_emit_sqtt_userdata(b, cs, z);
}

static void
dgc_emit_sqtt_begin_api_marker(nir_builder *b, struct dgc_cmdbuf *cs, enum rgp_sqtt_marker_general_api_type api_type)
{
   struct rgp_sqtt_marker_general_api marker = {0};

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_GENERAL_API;
   marker.api_type = api_type;

   dgc_emit_sqtt_userdata(b, cs, nir_imm_int(b, marker.dword01));
}

static void
dgc_emit_sqtt_end_api_marker(nir_builder *b, struct dgc_cmdbuf *cs, enum rgp_sqtt_marker_general_api_type api_type)
{
   struct rgp_sqtt_marker_general_api marker = {0};

   marker.identifier = RGP_SQTT_MARKER_IDENTIFIER_GENERAL_API;
   marker.api_type = api_type;
   marker.is_end = 1;

   dgc_emit_sqtt_userdata(b, cs, nir_imm_int(b, marker.dword01));
}

static void
dgc_emit_instance_count(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *instance_count)
{
   nir_def *values[2] = {nir_imm_int(b, PKT3(PKT3_NUM_INSTANCES, 0, false)), instance_count};

   dgc_emit(b, cs, nir_vec(b, values, 2));
}

static void
dgc_emit_draw_index_offset_2(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *index_offset, nir_def *index_count,
                             nir_def *max_index_count)
{
   nir_def *values[5] = {nir_imm_int(b, PKT3(PKT3_DRAW_INDEX_OFFSET_2, 3, false)), max_index_count, index_offset,
                         index_count, nir_imm_int(b, V_0287F0_DI_SRC_SEL_DMA)};

   dgc_emit(b, cs, nir_vec(b, values, 5));
}

static void
dgc_emit_draw_index_auto(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *vertex_count)
{
   nir_def *values[3] = {nir_imm_int(b, PKT3(PKT3_DRAW_INDEX_AUTO, 1, false)), vertex_count,
                         nir_imm_int(b, V_0287F0_DI_SRC_SEL_AUTO_INDEX)};

   dgc_emit(b, cs, nir_vec(b, values, 3));
}

static void
dgc_emit_dispatch_direct(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *wg_x, nir_def *wg_y, nir_def *wg_z,
                         nir_def *dispatch_initiator)
{
   nir_def *values[5] = {nir_imm_int(b, PKT3(PKT3_DISPATCH_DIRECT, 3, false) | PKT3_SHADER_TYPE_S(1)), wg_x, wg_y, wg_z,
                         dispatch_initiator};

   dgc_emit(b, cs, nir_vec(b, values, 5));
}

static void
dgc_emit_dispatch_mesh_direct(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *x, nir_def *y, nir_def *z)
{
   nir_def *values[5] = {nir_imm_int(b, PKT3(PKT3_DISPATCH_MESH_DIRECT, 3, false)), x, y, z,
                         nir_imm_int(b, S_0287F0_SOURCE_SELECT(V_0287F0_DI_SRC_SEL_AUTO_INDEX))};

   dgc_emit(b, cs, nir_vec(b, values, 5));
}

static void
dgc_emit_grid_size_user_sgpr(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *grid_base_sgpr, nir_def *wg_x,
                             nir_def *wg_y, nir_def *wg_z)
{
   nir_def *values[5] = {
      nir_imm_int(b, PKT3(PKT3_SET_SH_REG, 3, false)), grid_base_sgpr, wg_x, wg_y, wg_z,
   };

   dgc_emit(b, cs, nir_vec(b, values, 5));
}

static void
dgc_emit_grid_size_pointer(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *grid_base_sgpr, nir_def *stream_offset)
{
   nir_def *stream_addr = load_param64(b, stream_addr);
   nir_def *va = nir_iadd(b, stream_addr, nir_u2u64(b, stream_offset));

   nir_def *va_lo = nir_unpack_64_2x32_split_x(b, va);
   nir_def *va_hi = nir_unpack_64_2x32_split_y(b, va);

   nir_def *values[4] = {nir_imm_int(b, PKT3(PKT3_SET_SH_REG, 2, false)), grid_base_sgpr, va_lo, va_hi};

   dgc_emit(b, cs, nir_vec(b, values, 4));
}

static void
dgc_emit_pkt3_set_base(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *va)
{
   nir_def *va_lo = nir_unpack_64_2x32_split_x(b, va);
   nir_def *va_hi = nir_unpack_64_2x32_split_y(b, va);

   nir_def *values[4] = {nir_imm_int(b, PKT3(PKT3_SET_BASE, 2, false)), nir_imm_int(b, 1), va_lo, va_hi};

   dgc_emit(b, cs, nir_vec(b, values, 4));
}

static void
dgc_emit_pkt3_draw_indirect(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *vtx_base_sgpr, bool indexed)
{
   const unsigned di_src_sel = indexed ? V_0287F0_DI_SRC_SEL_DMA : V_0287F0_DI_SRC_SEL_AUTO_INDEX;

   vtx_base_sgpr = nir_iand_imm(b, nir_u2u32(b, vtx_base_sgpr), 0x3FFF);

   nir_def *has_drawid = nir_test_mask(b, vtx_base_sgpr, DGC_USES_DRAWID);
   nir_def *has_baseinstance = nir_test_mask(b, vtx_base_sgpr, DGC_USES_BASEINSTANCE);

   /* vertex_offset_reg = (base_reg - SI_SH_REG_OFFSET) >> 2 */
   nir_def *vertex_offset_reg = vtx_base_sgpr;

   /* start_instance_reg = (base_reg + (draw_id_enable ? 8 : 4) - SI_SH_REG_OFFSET) >> 2 */
   nir_def *start_instance_offset = nir_bcsel(b, has_drawid, nir_imm_int(b, 2), nir_imm_int(b, 1));
   nir_def *start_instance_reg = nir_iadd(b, vtx_base_sgpr, start_instance_offset);

   /* draw_id_reg = (base_reg + 4 - SI_SH_REG_OFFSET) >> 2 */
   nir_def *draw_id_reg = nir_iadd(b, vtx_base_sgpr, nir_imm_int(b, 1));

   nir_if *if_drawid = nir_push_if(b, has_drawid);
   {
      const unsigned pkt3_op = indexed ? PKT3_DRAW_INDEX_INDIRECT_MULTI : PKT3_DRAW_INDIRECT_MULTI;

      nir_def *values[8];
      values[0] = nir_imm_int(b, PKT3(pkt3_op, 8, false));
      values[1] = nir_imm_int(b, 0);
      values[2] = vertex_offset_reg;
      values[3] = nir_bcsel(b, has_baseinstance, start_instance_reg, nir_imm_int(b, 0));
      values[4] = nir_ior(b, draw_id_reg, nir_imm_int(b, S_2C3_DRAW_INDEX_ENABLE(1)));
      values[5] = nir_imm_int(b, 1); /* draw count */
      values[6] = nir_imm_int(b, 0); /* count va low */
      values[7] = nir_imm_int(b, 0); /* count va high */

      dgc_emit(b, cs, nir_vec(b, values, 8));

      values[0] = nir_imm_int(b, 0); /* stride */
      values[1] = nir_imm_int(b, V_0287F0_DI_SRC_SEL_AUTO_INDEX);

      dgc_emit(b, cs, nir_vec(b, values, 2));
   }
   nir_push_else(b, if_drawid);
   {
      const unsigned pkt3_op = indexed ? PKT3_DRAW_INDEX_INDIRECT : PKT3_DRAW_INDIRECT;

      nir_def *values[5];
      values[0] = nir_imm_int(b, PKT3(pkt3_op, 3, false));
      values[1] = nir_imm_int(b, 0);
      values[2] = vertex_offset_reg;
      values[3] = nir_bcsel(b, has_baseinstance, start_instance_reg, nir_imm_int(b, 0));
      values[4] = nir_imm_int(b, di_src_sel);

      dgc_emit(b, cs, nir_vec(b, values, 5));
   }
   nir_pop_if(b, if_drawid);
}

static void
dgc_emit_draw_indirect(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *stream_base, nir_def *draw_params_offset,
                       bool indexed)
{
   nir_def *vtx_base_sgpr = load_param16(b, vtx_base_sgpr);
   nir_def *stream_offset = nir_iadd(b, draw_params_offset, stream_base);

   nir_def *stream_addr = load_param64(b, stream_addr);
   nir_def *va = nir_iadd(b, stream_addr, nir_u2u64(b, stream_offset));

   dgc_emit_pkt3_set_base(b, cs, va);
   dgc_emit_pkt3_draw_indirect(b, cs, vtx_base_sgpr, indexed);
}

static nir_def *
dgc_cmd_buf_size(nir_builder *b, nir_def *sequence_count, const struct radv_device *device)
{
   nir_def *use_preamble = nir_ine_imm(b, load_param8(b, use_preamble), 0);
   nir_def *cmd_buf_size = load_param32(b, cmd_buf_size);
   nir_def *cmd_buf_stride = load_param32(b, cmd_buf_stride);
   nir_def *size = nir_imul(b, cmd_buf_stride, sequence_count);
   unsigned align_mask = radv_align_cmdbuf_size(device, 1, AMD_IP_GFX) - 1;

   size = nir_iand_imm(b, nir_iadd_imm(b, size, align_mask), ~align_mask);

   /* Ensure we don't have to deal with a jump to an empty IB in the preamble. */
   size = nir_imax(b, size, nir_imm_int(b, align_mask + 1));

   return nir_bcsel(b, use_preamble, size, cmd_buf_size);
}

static nir_def *
dgc_main_cmd_buf_offset(nir_builder *b, const struct radv_device *device)
{
   nir_def *use_preamble = nir_ine_imm(b, load_param8(b, use_preamble), 0);
   nir_def *base_offset = nir_imm_int(b, radv_dgc_preamble_cmdbuf_size(device));
   return nir_bcsel(b, use_preamble, base_offset, nir_imm_int(b, 0));
}

static void
build_dgc_buffer_tail(nir_builder *b, nir_def *sequence_count, const struct radv_device *device)
{
   nir_def *global_id = get_global_ids(b, 1);

   nir_def *cmd_buf_stride = load_param32(b, cmd_buf_stride);
   nir_def *cmd_buf_size = dgc_cmd_buf_size(b, sequence_count, device);

   nir_push_if(b, nir_ieq_imm(b, global_id, 0));
   {
      nir_def *base_offset = dgc_main_cmd_buf_offset(b, device);
      nir_def *cmd_buf_tail_start = nir_imul(b, cmd_buf_stride, sequence_count);

      nir_variable *offset = nir_variable_create(b->shader, nir_var_shader_temp, glsl_uint_type(), "offset");
      nir_store_var(b, offset, cmd_buf_tail_start, 0x1);

      nir_def *dst_buf = radv_meta_load_descriptor(b, 0, DGC_DESC_PREPARE);
      nir_push_loop(b);
      {
         nir_def *curr_offset = nir_load_var(b, offset);
         const unsigned MAX_PACKET_WORDS = 0x3FFC;

         nir_push_if(b, nir_ieq(b, curr_offset, cmd_buf_size));
         {
            nir_jump(b, nir_jump_break);
         }
         nir_pop_if(b, NULL);

         nir_def *packet, *packet_size;

         if (device->physical_device->rad_info.gfx_ib_pad_with_type2) {
            packet_size = nir_imm_int(b, 4);
            packet = nir_imm_int(b, PKT2_NOP_PAD);
         } else {
            packet_size = nir_isub(b, cmd_buf_size, curr_offset);
            packet_size = nir_umin(b, packet_size, nir_imm_int(b, MAX_PACKET_WORDS * 4));

            nir_def *len = nir_ushr_imm(b, packet_size, 2);
            len = nir_iadd_imm(b, len, -2);
            packet = nir_pkt3(b, PKT3_NOP, len);
         }

         nir_store_ssbo(b, packet, dst_buf, nir_iadd(b, curr_offset, base_offset), .access = ACCESS_NON_READABLE);
         nir_store_var(b, offset, nir_iadd(b, curr_offset, packet_size), 0x1);
      }
      nir_pop_loop(b, NULL);
   }
   nir_pop_if(b, NULL);
}

static void
build_dgc_buffer_preamble(nir_builder *b, nir_def *sequence_count, const struct radv_device *device)
{
   nir_def *global_id = get_global_ids(b, 1);
   nir_def *use_preamble = nir_ine_imm(b, load_param8(b, use_preamble), 0);

   nir_push_if(b, nir_iand(b, nir_ieq_imm(b, global_id, 0), use_preamble));
   {
      unsigned preamble_size = radv_dgc_preamble_cmdbuf_size(device);
      nir_def *cmd_buf_size = dgc_cmd_buf_size(b, sequence_count, device);
      nir_def *dst_buf = radv_meta_load_descriptor(b, 0, DGC_DESC_PREPARE);

      nir_def *words = nir_ushr_imm(b, cmd_buf_size, 2);

      nir_def *addr = nir_iadd_imm(b, load_param32(b, upload_addr), preamble_size);

      nir_def *nop_packet = dgc_get_nop_packet(b, device);

      nir_def *nop_packets[] = {
         nop_packet,
         nop_packet,
         nop_packet,
         nop_packet,
      };

      const unsigned jump_size = 16;
      unsigned offset;

      /* Do vectorized store if possible */
      for (offset = 0; offset + 16 <= preamble_size - jump_size; offset += 16) {
         nir_store_ssbo(b, nir_vec(b, nop_packets, 4), dst_buf, nir_imm_int(b, offset), .access = ACCESS_NON_READABLE);
      }

      for (; offset + 4 <= preamble_size - jump_size; offset += 4) {
         nir_store_ssbo(b, nop_packet, dst_buf, nir_imm_int(b, offset), .access = ACCESS_NON_READABLE);
      }

      nir_def *chain_packets[] = {
         nir_imm_int(b, PKT3(PKT3_INDIRECT_BUFFER, 2, 0)),
         addr,
         nir_imm_int(b, device->physical_device->rad_info.address32_hi),
         nir_ior_imm(b, words, S_3F2_CHAIN(1) | S_3F2_VALID(1) | S_3F2_PRE_ENA(false)),
      };

      nir_store_ssbo(b, nir_vec(b, chain_packets, 4), dst_buf, nir_imm_int(b, preamble_size - jump_size),
                     .access = ACCESS_NON_READABLE);
   }
   nir_pop_if(b, NULL);
}

/**
 * Emit VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NV.
 */
static void
dgc_emit_draw(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *stream_buf, nir_def *stream_base,
              nir_def *draw_params_offset, nir_def *sequence_id, const struct radv_device *device)
{
   nir_def *vtx_base_sgpr = load_param16(b, vtx_base_sgpr);
   nir_def *stream_offset = nir_iadd(b, draw_params_offset, stream_base);

   nir_def *draw_data0 = nir_load_ssbo(b, 4, 32, stream_buf, stream_offset);
   nir_def *vertex_count = nir_channel(b, draw_data0, 0);
   nir_def *instance_count = nir_channel(b, draw_data0, 1);
   nir_def *vertex_offset = nir_channel(b, draw_data0, 2);
   nir_def *first_instance = nir_channel(b, draw_data0, 3);

   nir_push_if(b, nir_iand(b, nir_ine_imm(b, vertex_count, 0), nir_ine_imm(b, instance_count, 0)));
   {
      dgc_emit_sqtt_begin_api_marker(b, cs, ApiCmdDraw);
      dgc_emit_sqtt_marker_event(b, cs, sequence_id, EventCmdDraw);

      dgc_emit_userdata_vertex(b, cs, vtx_base_sgpr, vertex_offset, first_instance, sequence_id, device);
      dgc_emit_instance_count(b, cs, instance_count);
      dgc_emit_draw_index_auto(b, cs, vertex_count);

      dgc_emit_sqtt_thread_trace_marker(b, cs);
      dgc_emit_sqtt_end_api_marker(b, cs, ApiCmdDraw);
   }
   nir_pop_if(b, 0);
}

/**
 * Emit VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_NV.
 */
static void
dgc_emit_draw_indexed(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *stream_buf, nir_def *stream_base,
                      nir_def *draw_params_offset, nir_def *sequence_id, nir_def *max_index_count,
                      const struct radv_device *device)
{
   nir_def *vtx_base_sgpr = load_param16(b, vtx_base_sgpr);
   nir_def *stream_offset = nir_iadd(b, draw_params_offset, stream_base);

   nir_def *draw_data0 = nir_load_ssbo(b, 4, 32, stream_buf, stream_offset);
   nir_def *draw_data1 = nir_load_ssbo(b, 1, 32, stream_buf, nir_iadd_imm(b, stream_offset, 16));
   nir_def *index_count = nir_channel(b, draw_data0, 0);
   nir_def *instance_count = nir_channel(b, draw_data0, 1);
   nir_def *first_index = nir_channel(b, draw_data0, 2);
   nir_def *vertex_offset = nir_channel(b, draw_data0, 3);
   nir_def *first_instance = nir_channel(b, draw_data1, 0);

   nir_push_if(b, nir_iand(b, nir_ine_imm(b, index_count, 0), nir_ine_imm(b, instance_count, 0)));
   {
      dgc_emit_sqtt_begin_api_marker(b, cs, ApiCmdDrawIndexed);
      dgc_emit_sqtt_marker_event(b, cs, sequence_id, EventCmdDrawIndexed);

      dgc_emit_userdata_vertex(b, cs, vtx_base_sgpr, vertex_offset, first_instance, sequence_id, device);
      dgc_emit_instance_count(b, cs, instance_count);
      dgc_emit_draw_index_offset_2(b, cs, first_index, index_count, max_index_count);

      dgc_emit_sqtt_thread_trace_marker(b, cs);
      dgc_emit_sqtt_end_api_marker(b, cs, ApiCmdDrawIndexed);
   }
   nir_pop_if(b, 0);
}

/**
 * Emit VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_NV.
 */
static void
dgc_emit_index_buffer(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *stream_buf, nir_def *stream_base,
                      nir_def *index_buffer_offset, nir_def *ibo_type_32, nir_def *ibo_type_8,
                      nir_variable *max_index_count_var, const struct radv_device *device)
{
   nir_def *index_stream_offset = nir_iadd(b, index_buffer_offset, stream_base);
   nir_def *data = nir_load_ssbo(b, 4, 32, stream_buf, index_stream_offset);

   nir_def *vk_index_type = nir_channel(b, data, 3);
   nir_def *index_type = nir_bcsel(b, nir_ieq(b, vk_index_type, ibo_type_32), nir_imm_int(b, V_028A7C_VGT_INDEX_32),
                                   nir_imm_int(b, V_028A7C_VGT_INDEX_16));
   index_type = nir_bcsel(b, nir_ieq(b, vk_index_type, ibo_type_8), nir_imm_int(b, V_028A7C_VGT_INDEX_8), index_type);

   nir_def *index_size = nir_iand_imm(b, nir_ushr(b, nir_imm_int(b, 0x142), nir_imul_imm(b, index_type, 4)), 0xf);

   nir_def *max_index_count = nir_udiv(b, nir_channel(b, data, 2), index_size);
   nir_store_var(b, max_index_count_var, max_index_count, 0x1);

   nir_def *cmd_values[3 + 2 + 3];

   if (device->physical_device->rad_info.gfx_level >= GFX9) {
      unsigned opcode = PKT3_SET_UCONFIG_REG_INDEX;
      if (device->physical_device->rad_info.gfx_level < GFX9 ||
          (device->physical_device->rad_info.gfx_level == GFX9 && device->physical_device->rad_info.me_fw_version < 26))
         opcode = PKT3_SET_UCONFIG_REG;
      cmd_values[0] = nir_imm_int(b, PKT3(opcode, 1, 0));
      cmd_values[1] = nir_imm_int(b, (R_03090C_VGT_INDEX_TYPE - CIK_UCONFIG_REG_OFFSET) >> 2 | (2u << 28));
      cmd_values[2] = index_type;
   } else {
      cmd_values[0] = nir_imm_int(b, PKT3(PKT3_INDEX_TYPE, 0, 0));
      cmd_values[1] = index_type;
      cmd_values[2] = dgc_get_nop_packet(b, device);
   }

   nir_def *addr_upper = nir_channel(b, data, 1);
   addr_upper = nir_ishr_imm(b, nir_ishl_imm(b, addr_upper, 16), 16);

   cmd_values[3] = nir_imm_int(b, PKT3(PKT3_INDEX_BASE, 1, 0));
   cmd_values[4] = nir_channel(b, data, 0);
   cmd_values[5] = addr_upper;
   cmd_values[6] = nir_imm_int(b, PKT3(PKT3_INDEX_BUFFER_SIZE, 0, 0));
   cmd_values[7] = max_index_count;

   dgc_emit(b, cs, nir_vec(b, cmd_values, 8));
}

/**
 * Emit VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NV.
 */
static void
dgc_emit_push_constant(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *stream_buf, nir_def *stream_base,
                       nir_def *push_const_mask, nir_variable *upload_offset)
{
   nir_def *vbo_cnt = load_param8(b, vbo_cnt);
   nir_def *const_copy = nir_ine_imm(b, load_param8(b, const_copy), 0);
   nir_def *const_copy_size = load_param16(b, const_copy_size);
   nir_def *const_copy_words = nir_ushr_imm(b, const_copy_size, 2);
   const_copy_words = nir_bcsel(b, const_copy, const_copy_words, nir_imm_int(b, 0));

   nir_variable *idx = nir_variable_create(b->shader, nir_var_shader_temp, glsl_uint_type(), "const_copy_idx");
   nir_store_var(b, idx, nir_imm_int(b, 0), 0x1);

   nir_def *param_buf = radv_meta_load_descriptor(b, 0, DGC_DESC_PARAMS);
   nir_def *param_offset = nir_imul_imm(b, vbo_cnt, 24);
   nir_def *param_offset_offset = nir_iadd_imm(b, param_offset, MESA_VULKAN_SHADER_STAGES * 12);
   nir_def *param_const_offset =
      nir_iadd_imm(b, param_offset, MAX_PUSH_CONSTANTS_SIZE + MESA_VULKAN_SHADER_STAGES * 12);
   nir_push_loop(b);
   {
      nir_def *cur_idx = nir_load_var(b, idx);
      nir_push_if(b, nir_uge(b, cur_idx, const_copy_words));
      {
         nir_jump(b, nir_jump_break);
      }
      nir_pop_if(b, NULL);

      nir_variable *data = nir_variable_create(b->shader, nir_var_shader_temp, glsl_uint_type(), "copy_data");

      nir_def *update = nir_iand(b, push_const_mask, nir_ishl(b, nir_imm_int64(b, 1), cur_idx));
      update = nir_bcsel(b, nir_ult_imm(b, cur_idx, 64 /* bits in push_const_mask */), update, nir_imm_int64(b, 0));

      nir_push_if(b, nir_ine_imm(b, update, 0));
      {
         nir_def *stream_offset =
            nir_load_ssbo(b, 1, 32, param_buf, nir_iadd(b, param_offset_offset, nir_ishl_imm(b, cur_idx, 2)));
         nir_def *new_data = nir_load_ssbo(b, 1, 32, stream_buf, nir_iadd(b, stream_base, stream_offset));
         nir_store_var(b, data, new_data, 0x1);
      }
      nir_push_else(b, NULL);
      {
         nir_store_var(b, data,
                       nir_load_ssbo(b, 1, 32, param_buf, nir_iadd(b, param_const_offset, nir_ishl_imm(b, cur_idx, 2))),
                       0x1);
      }
      nir_pop_if(b, NULL);

      nir_store_ssbo(b, nir_load_var(b, data), cs->descriptor,
                     nir_iadd(b, nir_load_var(b, upload_offset), nir_ishl_imm(b, cur_idx, 2)),
                     .access = ACCESS_NON_READABLE);

      nir_store_var(b, idx, nir_iadd_imm(b, cur_idx, 1), 0x1);
   }
   nir_pop_loop(b, NULL);

   nir_variable *shader_idx = nir_variable_create(b->shader, nir_var_shader_temp, glsl_uint_type(), "shader_idx");
   nir_store_var(b, shader_idx, nir_imm_int(b, 0), 0x1);
   nir_def *shader_cnt = load_param16(b, push_constant_shader_cnt);

   nir_push_loop(b);
   {
      nir_def *cur_shader_idx = nir_load_var(b, shader_idx);
      nir_push_if(b, nir_uge(b, cur_shader_idx, shader_cnt));
      {
         nir_jump(b, nir_jump_break);
      }
      nir_pop_if(b, NULL);

      nir_def *reg_info =
         nir_load_ssbo(b, 3, 32, param_buf, nir_iadd(b, param_offset, nir_imul_imm(b, cur_shader_idx, 12)));
      nir_def *upload_sgpr = nir_ubfe_imm(b, nir_channel(b, reg_info, 0), 0, 16);
      nir_def *inline_sgpr = nir_ubfe_imm(b, nir_channel(b, reg_info, 0), 16, 16);
      nir_def *inline_mask = nir_pack_64_2x32(b, nir_channels(b, reg_info, 0x6));

      nir_push_if(b, nir_ine_imm(b, upload_sgpr, 0));
      {
         nir_def *pkt[3] = {nir_imm_int(b, PKT3(PKT3_SET_SH_REG, 1, 0)), upload_sgpr,
                            nir_iadd(b, load_param32(b, upload_addr), nir_load_var(b, upload_offset))};

         dgc_emit(b, cs, nir_vec(b, pkt, 3));
      }
      nir_pop_if(b, NULL);

      nir_push_if(b, nir_ine_imm(b, inline_sgpr, 0));
      {
         nir_store_var(b, idx, nir_imm_int(b, 0), 0x1);

         nir_variable *pc_idx = nir_variable_create(b->shader, nir_var_shader_temp, glsl_uint_type(), "pc_idx");
         nir_store_var(b, pc_idx, nir_imm_int(b, 0), 0x1);

         nir_push_loop(b);
         {
            nir_def *cur_idx = nir_load_var(b, idx);
            nir_push_if(b, nir_uge_imm(b, cur_idx, 64 /* bits in inline_mask */));
            {
               nir_jump(b, nir_jump_break);
            }
            nir_pop_if(b, NULL);

            nir_def *l = nir_ishl(b, nir_imm_int64(b, 1), cur_idx);
            nir_push_if(b, nir_ieq_imm(b, nir_iand(b, l, inline_mask), 0));
            {
               nir_store_var(b, idx, nir_iadd_imm(b, cur_idx, 1), 0x1);
               nir_jump(b, nir_jump_continue);
            }
            nir_pop_if(b, NULL);

            nir_variable *data = nir_variable_create(b->shader, nir_var_shader_temp, glsl_uint_type(), "copy_data");

            nir_def *update = nir_iand(b, push_const_mask, nir_ishl(b, nir_imm_int64(b, 1), cur_idx));
            update =
               nir_bcsel(b, nir_ult_imm(b, cur_idx, 64 /* bits in push_const_mask */), update, nir_imm_int64(b, 0));

            nir_push_if(b, nir_ine_imm(b, update, 0));
            {
               nir_def *stream_offset =
                  nir_load_ssbo(b, 1, 32, param_buf, nir_iadd(b, param_offset_offset, nir_ishl_imm(b, cur_idx, 2)));
               nir_def *new_data = nir_load_ssbo(b, 1, 32, stream_buf, nir_iadd(b, stream_base, stream_offset));
               nir_store_var(b, data, new_data, 0x1);

               nir_def *pkt[3] = {nir_pkt3(b, PKT3_SET_SH_REG, nir_imm_int(b, 1)),
                                  nir_iadd(b, inline_sgpr, nir_load_var(b, pc_idx)), nir_load_var(b, data)};

               dgc_emit(b, cs, nir_vec(b, pkt, 3));
            }
            nir_pop_if(b, NULL);

            nir_store_var(b, idx, nir_iadd_imm(b, cur_idx, 1), 0x1);
            nir_store_var(b, pc_idx, nir_iadd_imm(b, nir_load_var(b, pc_idx), 1), 0x1);
         }
         nir_pop_loop(b, NULL);
      }
      nir_pop_if(b, NULL);
      nir_store_var(b, shader_idx, nir_iadd_imm(b, cur_shader_idx, 1), 0x1);
   }
   nir_pop_loop(b, NULL);
}

/**
 * For emitting VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_NV.
 */
static void
dgc_emit_vertex_buffer(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *stream_buf, nir_def *stream_base,
                       nir_def *vbo_bind_mask, nir_variable *upload_offset, const struct radv_device *device)
{
   nir_def *vbo_cnt = load_param8(b, vbo_cnt);
   nir_variable *vbo_idx = nir_variable_create(b->shader, nir_var_shader_temp, glsl_uint_type(), "vbo_idx");
   nir_store_var(b, vbo_idx, nir_imm_int(b, 0), 0x1);

   nir_push_loop(b);
   {
      nir_push_if(b, nir_uge(b, nir_load_var(b, vbo_idx), vbo_cnt));
      {
         nir_jump(b, nir_jump_break);
      }
      nir_pop_if(b, NULL);

      nir_def *vbo_offset = nir_imul_imm(b, nir_load_var(b, vbo_idx), 16);
      nir_variable *vbo_data = nir_variable_create(b->shader, nir_var_shader_temp, glsl_uvec4_type(), "vbo_data");

      nir_def *param_buf = radv_meta_load_descriptor(b, 0, DGC_DESC_PARAMS);
      nir_store_var(b, vbo_data, nir_load_ssbo(b, 4, 32, param_buf, vbo_offset), 0xf);

      nir_def *vbo_override =
         nir_ine_imm(b, nir_iand(b, vbo_bind_mask, nir_ishl(b, nir_imm_int(b, 1), nir_load_var(b, vbo_idx))), 0);
      nir_push_if(b, vbo_override);
      {
         nir_def *vbo_offset_offset =
            nir_iadd(b, nir_imul_imm(b, vbo_cnt, 16), nir_imul_imm(b, nir_load_var(b, vbo_idx), 8));
         nir_def *vbo_over_data = nir_load_ssbo(b, 2, 32, param_buf, vbo_offset_offset);
         nir_def *stream_offset = nir_iadd(b, stream_base, nir_iand_imm(b, nir_channel(b, vbo_over_data, 0), 0x7FFF));
         nir_def *stream_data = nir_load_ssbo(b, 4, 32, stream_buf, stream_offset);

         nir_def *va = nir_pack_64_2x32(b, nir_trim_vector(b, stream_data, 2));
         nir_def *size = nir_channel(b, stream_data, 2);
         nir_def *stride = nir_channel(b, stream_data, 3);

         nir_def *dyn_stride = nir_test_mask(b, nir_channel(b, vbo_over_data, 0), DGC_DYNAMIC_STRIDE);
         nir_def *old_stride = nir_ubfe_imm(b, nir_channel(b, nir_load_var(b, vbo_data), 1), 16, 14);
         stride = nir_bcsel(b, dyn_stride, stride, old_stride);

         nir_def *use_per_attribute_vb_descs = nir_test_mask(b, nir_channel(b, vbo_over_data, 0), 1u << 31);
         nir_variable *num_records =
            nir_variable_create(b->shader, nir_var_shader_temp, glsl_uint_type(), "num_records");
         nir_store_var(b, num_records, size, 0x1);

         nir_push_if(b, use_per_attribute_vb_descs);
         {
            nir_def *attrib_end = nir_ubfe_imm(b, nir_channel(b, vbo_over_data, 1), 16, 16);
            nir_def *attrib_index_offset = nir_ubfe_imm(b, nir_channel(b, vbo_over_data, 1), 0, 16);

            nir_push_if(b, nir_ult(b, nir_load_var(b, num_records), attrib_end));
            {
               nir_store_var(b, num_records, nir_imm_int(b, 0), 0x1);
            }
            nir_push_else(b, NULL);
            nir_push_if(b, nir_ieq_imm(b, stride, 0));
            {
               nir_store_var(b, num_records, nir_imm_int(b, 1), 0x1);
            }
            nir_push_else(b, NULL);
            {
               nir_def *r = nir_iadd(
                  b, nir_iadd_imm(b, nir_udiv(b, nir_isub(b, nir_load_var(b, num_records), attrib_end), stride), 1),
                  attrib_index_offset);
               nir_store_var(b, num_records, r, 0x1);
            }
            nir_pop_if(b, NULL);
            nir_pop_if(b, NULL);

            nir_def *convert_cond = nir_ine_imm(b, nir_load_var(b, num_records), 0);
            if (device->physical_device->rad_info.gfx_level == GFX9)
               convert_cond = nir_imm_false(b);
            else if (device->physical_device->rad_info.gfx_level != GFX8)
               convert_cond = nir_iand(b, convert_cond, nir_ieq_imm(b, stride, 0));

            nir_def *new_records =
               nir_iadd(b, nir_imul(b, nir_iadd_imm(b, nir_load_var(b, num_records), -1), stride), attrib_end);
            new_records = nir_bcsel(b, convert_cond, new_records, nir_load_var(b, num_records));
            nir_store_var(b, num_records, new_records, 0x1);
         }
         nir_push_else(b, NULL);
         {
            if (device->physical_device->rad_info.gfx_level != GFX8) {
               nir_push_if(b, nir_ine_imm(b, stride, 0));
               {
                  nir_def *r = nir_iadd(b, nir_load_var(b, num_records), nir_iadd_imm(b, stride, -1));
                  nir_store_var(b, num_records, nir_udiv(b, r, stride), 0x1);
               }
               nir_pop_if(b, NULL);
            }
         }
         nir_pop_if(b, NULL);

         nir_def *rsrc_word3 = nir_channel(b, nir_load_var(b, vbo_data), 3);
         if (device->physical_device->rad_info.gfx_level >= GFX10) {
            nir_def *oob_select = nir_bcsel(b, nir_ieq_imm(b, stride, 0), nir_imm_int(b, V_008F0C_OOB_SELECT_RAW),
                                            nir_imm_int(b, V_008F0C_OOB_SELECT_STRUCTURED));
            rsrc_word3 = nir_iand_imm(b, rsrc_word3, C_008F0C_OOB_SELECT);
            rsrc_word3 = nir_ior(b, rsrc_word3, nir_ishl_imm(b, oob_select, 28));
         }

         nir_def *va_hi = nir_iand_imm(b, nir_unpack_64_2x32_split_y(b, va), 0xFFFF);
         stride = nir_iand_imm(b, stride, 0x3FFF);
         nir_def *new_vbo_data[4] = {nir_unpack_64_2x32_split_x(b, va), nir_ior(b, nir_ishl_imm(b, stride, 16), va_hi),
                                     nir_load_var(b, num_records), rsrc_word3};
         nir_store_var(b, vbo_data, nir_vec(b, new_vbo_data, 4), 0xf);
      }
      nir_pop_if(b, NULL);

      /* On GFX9, it seems bounds checking is disabled if both
       * num_records and stride are zero. This doesn't seem necessary on GFX8, GFX10 and
       * GFX10.3 but it doesn't hurt.
       */
      nir_def *num_records = nir_channel(b, nir_load_var(b, vbo_data), 2);
      nir_def *buf_va =
         nir_iand_imm(b, nir_pack_64_2x32(b, nir_trim_vector(b, nir_load_var(b, vbo_data), 2)), (1ull << 48) - 1ull);
      nir_push_if(b, nir_ior(b, nir_ieq_imm(b, num_records, 0), nir_ieq_imm(b, buf_va, 0)));
      {
         nir_def *new_vbo_data[4] = {nir_imm_int(b, 0), nir_imm_int(b, 0), nir_imm_int(b, 0), nir_imm_int(b, 0)};
         nir_store_var(b, vbo_data, nir_vec(b, new_vbo_data, 4), 0xf);
      }
      nir_pop_if(b, NULL);

      nir_def *upload_off = nir_iadd(b, nir_load_var(b, upload_offset), vbo_offset);
      nir_store_ssbo(b, nir_load_var(b, vbo_data), cs->descriptor, upload_off, .access = ACCESS_NON_READABLE);
      nir_store_var(b, vbo_idx, nir_iadd_imm(b, nir_load_var(b, vbo_idx), 1), 0x1);
   }
   nir_pop_loop(b, NULL);
   nir_def *packet[3] = {nir_imm_int(b, PKT3(PKT3_SET_SH_REG, 1, 0)), load_param16(b, vbo_reg),
                         nir_iadd(b, load_param32(b, upload_addr), nir_load_var(b, upload_offset))};

   dgc_emit(b, cs, nir_vec(b, packet, 3));

   nir_store_var(b, upload_offset, nir_iadd(b, nir_load_var(b, upload_offset), nir_imul_imm(b, vbo_cnt, 16)), 0x1);
}

/**
 * For emitting VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_NV.
 */
static void
dgc_emit_dispatch(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *stream_buf, nir_def *stream_base,
                  nir_def *dispatch_params_offset, nir_def *sequence_id, const struct radv_device *device)
{
   nir_def *stream_offset = nir_iadd(b, dispatch_params_offset, stream_base);

   nir_def *dispatch_data = nir_load_ssbo(b, 3, 32, stream_buf, stream_offset);
   nir_def *wg_x = nir_channel(b, dispatch_data, 0);
   nir_def *wg_y = nir_channel(b, dispatch_data, 1);
   nir_def *wg_z = nir_channel(b, dispatch_data, 2);

   nir_def *grid_sgpr = load_param16(b, grid_base_sgpr);
   nir_push_if(b, nir_ine_imm(b, grid_sgpr, 0));
   {
      if (device->load_grid_size_from_user_sgpr) {
         dgc_emit_grid_size_user_sgpr(b, cs, grid_sgpr, wg_x, wg_y, wg_z);
      } else {
         dgc_emit_grid_size_pointer(b, cs, grid_sgpr, stream_offset);
      }
   }
   nir_pop_if(b, 0);

   nir_push_if(b, nir_iand(b, nir_ine_imm(b, wg_x, 0), nir_iand(b, nir_ine_imm(b, wg_y, 0), nir_ine_imm(b, wg_z, 0))));
   {
      dgc_emit_sqtt_begin_api_marker(b, cs, ApiCmdDispatch);
      dgc_emit_sqtt_marker_event_with_dims(b, cs, sequence_id, wg_x, wg_y, wg_z, EventCmdDispatch);

      dgc_emit_dispatch_direct(b, cs, wg_x, wg_y, wg_z, load_param32(b, dispatch_initiator));

      dgc_emit_sqtt_thread_trace_marker(b, cs);
      dgc_emit_sqtt_end_api_marker(b, cs, ApiCmdDispatch);
   }
   nir_pop_if(b, 0);
}

/**
 * Emit VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV.
 */
static void
dgc_emit_draw_mesh_tasks(nir_builder *b, struct dgc_cmdbuf *cs, nir_def *stream_buf, nir_def *stream_base,
                         nir_def *draw_params_offset, nir_def *sequence_id, const struct radv_device *device)
{
   nir_def *vtx_base_sgpr = load_param16(b, vtx_base_sgpr);
   nir_def *stream_offset = nir_iadd(b, draw_params_offset, stream_base);

   nir_def *draw_data = nir_load_ssbo(b, 4, 32, stream_buf, stream_offset);
   nir_def *x = nir_channel(b, draw_data, 0);
   nir_def *y = nir_channel(b, draw_data, 1);
   nir_def *z = nir_channel(b, draw_data, 2);

   nir_push_if(b, nir_iand(b, nir_ine_imm(b, x, 0), nir_iand(b, nir_ine_imm(b, y, 0), nir_ine_imm(b, z, 0))));
   {
      dgc_emit_sqtt_begin_api_marker(b, cs, ApiCmdDrawMeshTasksEXT);
      dgc_emit_sqtt_marker_event(b, cs, sequence_id, EventCmdDrawMeshTasksEXT);

      dgc_emit_userdata_mesh(b, cs, vtx_base_sgpr, x, y, z, sequence_id, device);
      dgc_emit_instance_count(b, cs, nir_imm_int(b, 1));

      if (device->mesh_fast_launch_2) {
         dgc_emit_dispatch_mesh_direct(b, cs, x, y, z);
      } else {
         nir_def *vertex_count = nir_imul(b, x, nir_imul(b, y, z));
         dgc_emit_draw_index_auto(b, cs, vertex_count);
      }

      dgc_emit_sqtt_thread_trace_marker(b, cs);
      dgc_emit_sqtt_end_api_marker(b, cs, ApiCmdDrawMeshTasksEXT);
   }
   nir_pop_if(b, NULL);
}

static nir_shader *
build_dgc_prepare_shader(struct radv_device *dev)
{
   nir_builder b = radv_meta_init_shader(dev, MESA_SHADER_COMPUTE, "meta_dgc_prepare");
   b.shader->info.workgroup_size[0] = 64;

   nir_def *global_id = get_global_ids(&b, 1);

   nir_def *sequence_id = global_id;

   nir_def *cmd_buf_stride = load_param32(&b, cmd_buf_stride);
   nir_def *sequence_count = load_param32(&b, sequence_count);
   nir_def *stream_stride = load_param32(&b, stream_stride);

   nir_def *use_count = nir_iand_imm(&b, sequence_count, 1u << 31);
   sequence_count = nir_iand_imm(&b, sequence_count, UINT32_MAX >> 1);

   nir_def *cmd_buf_base_offset = dgc_main_cmd_buf_offset(&b, dev);

   /* The effective number of draws is
    * min(sequencesCount, sequencesCountBuffer[sequencesCountOffset]) when
    * using sequencesCountBuffer. Otherwise it is sequencesCount. */
   nir_variable *count_var = nir_variable_create(b.shader, nir_var_shader_temp, glsl_uint_type(), "sequence_count");
   nir_store_var(&b, count_var, sequence_count, 0x1);

   nir_push_if(&b, nir_ine_imm(&b, use_count, 0));
   {
      nir_def *count_buf = radv_meta_load_descriptor(&b, 0, DGC_DESC_COUNT);
      nir_def *cnt = nir_load_ssbo(&b, 1, 32, count_buf, nir_imm_int(&b, 0));
      /* Must clamp count against the API count explicitly.
       * The workgroup potentially contains more threads than maxSequencesCount from API,
       * and we have to ensure these threads write NOP packets to pad out the IB. */
      cnt = nir_umin(&b, cnt, sequence_count);
      nir_store_var(&b, count_var, cnt, 0x1);
   }
   nir_pop_if(&b, NULL);

   sequence_count = nir_load_var(&b, count_var);

   nir_push_if(&b, nir_ult(&b, sequence_id, sequence_count));
   {
      struct dgc_cmdbuf cmd_buf = {
         .descriptor = radv_meta_load_descriptor(&b, 0, DGC_DESC_PREPARE),
         .offset = nir_variable_create(b.shader, nir_var_shader_temp, glsl_uint_type(), "cmd_buf_offset"),
         .gfx_level = dev->physical_device->rad_info.gfx_level,
         .sqtt_enabled = !!dev->sqtt.bo,
      };
      nir_store_var(&b, cmd_buf.offset, nir_iadd(&b, nir_imul(&b, global_id, cmd_buf_stride), cmd_buf_base_offset), 1);
      nir_def *cmd_buf_end = nir_iadd(&b, nir_load_var(&b, cmd_buf.offset), cmd_buf_stride);

      nir_def *stream_buf = radv_meta_load_descriptor(&b, 0, DGC_DESC_STREAM);
      nir_def *stream_base = nir_imul(&b, sequence_id, stream_stride);

      nir_variable *upload_offset =
         nir_variable_create(b.shader, nir_var_shader_temp, glsl_uint_type(), "upload_offset");
      nir_def *upload_offset_init = nir_iadd(&b, nir_iadd(&b, load_param32(&b, cmd_buf_size), cmd_buf_base_offset),
                                             nir_imul(&b, load_param32(&b, upload_stride), sequence_id));
      nir_store_var(&b, upload_offset, upload_offset_init, 0x1);

      nir_def *vbo_bind_mask = load_param32(&b, vbo_bind_mask);
      nir_push_if(&b, nir_ine_imm(&b, vbo_bind_mask, 0));
      {
         dgc_emit_vertex_buffer(&b, &cmd_buf, stream_buf, stream_base, vbo_bind_mask, upload_offset, dev);
      }
      nir_pop_if(&b, NULL);

      nir_def *push_const_mask = load_param64(&b, push_constant_mask);
      nir_push_if(&b, nir_ine_imm(&b, push_const_mask, 0));
      {
         dgc_emit_push_constant(&b, &cmd_buf, stream_buf, stream_base, push_const_mask, upload_offset);
      }
      nir_pop_if(&b, 0);

      nir_push_if(&b, nir_ieq_imm(&b, load_param8(&b, is_dispatch), 0));
      {
         nir_push_if(&b, nir_ieq_imm(&b, load_param16(&b, draw_indexed), 0));
         {
            nir_def *draw_mesh_tasks = load_param8(&b, draw_mesh_tasks);
            nir_push_if(&b, nir_ieq_imm(&b, draw_mesh_tasks, 0));
            {
               dgc_emit_draw(&b, &cmd_buf, stream_buf, stream_base, load_param16(&b, draw_params_offset), sequence_id,
                             dev);
            }
            nir_push_else(&b, NULL);
            {
               dgc_emit_draw_mesh_tasks(&b, &cmd_buf, stream_buf, stream_base, load_param16(&b, draw_params_offset),
                                        sequence_id, dev);
            }
            nir_pop_if(&b, NULL);
         }
         nir_push_else(&b, NULL);
         {
            /* Emit direct draws when index buffers are also updated by DGC. Otherwise, emit
             * indirect draws to remove the dependency on the cmdbuf state in order to enable
             * preprocessing.
             */
            nir_def *binds_index_buffer = nir_ine_imm(&b, load_param16(&b, binds_index_buffer), 0);
            nir_push_if(&b, binds_index_buffer);
            {
               nir_variable *max_index_count_var =
                  nir_variable_create(b.shader, nir_var_shader_temp, glsl_uint_type(), "max_index_count");

               dgc_emit_index_buffer(&b, &cmd_buf, stream_buf, stream_base, load_param16(&b, index_buffer_offset),
                                     load_param32(&b, ibo_type_32), load_param32(&b, ibo_type_8), max_index_count_var,
                                     dev);

               nir_def *max_index_count = nir_load_var(&b, max_index_count_var);

               dgc_emit_draw_indexed(&b, &cmd_buf, stream_buf, stream_base, load_param16(&b, draw_params_offset),
                                     sequence_id, max_index_count, dev);
            }
            nir_push_else(&b, NULL);
            {
               dgc_emit_draw_indirect(&b, &cmd_buf, stream_base, load_param16(&b, draw_params_offset), true);
            }

            nir_pop_if(&b, NULL);
         }
         nir_pop_if(&b, NULL);
      }
      nir_push_else(&b, NULL);
      {
         dgc_emit_dispatch(&b, &cmd_buf, stream_buf, stream_base, load_param16(&b, dispatch_params_offset), sequence_id,
                           dev);
      }
      nir_pop_if(&b, NULL);

      /* Pad the cmdbuffer if we did not use the whole stride */
      nir_push_if(&b, nir_ine(&b, nir_load_var(&b, cmd_buf.offset), cmd_buf_end));
      {
         if (dev->physical_device->rad_info.gfx_ib_pad_with_type2) {
            nir_push_loop(&b);
            {
               nir_def *curr_offset = nir_load_var(&b, cmd_buf.offset);

               nir_push_if(&b, nir_ieq(&b, curr_offset, cmd_buf_end));
               {
                  nir_jump(&b, nir_jump_break);
               }
               nir_pop_if(&b, NULL);

               nir_def *pkt = nir_imm_int(&b, PKT2_NOP_PAD);

               dgc_emit(&b, &cmd_buf, pkt);
            }
            nir_pop_loop(&b, NULL);
         } else {
            nir_def *cnt = nir_isub(&b, cmd_buf_end, nir_load_var(&b, cmd_buf.offset));
            cnt = nir_ushr_imm(&b, cnt, 2);
            cnt = nir_iadd_imm(&b, cnt, -2);
            nir_def *pkt = nir_pkt3(&b, PKT3_NOP, cnt);

            dgc_emit(&b, &cmd_buf, pkt);
         }
      }
      nir_pop_if(&b, NULL);
   }
   nir_pop_if(&b, NULL);

   build_dgc_buffer_tail(&b, sequence_count, dev);
   build_dgc_buffer_preamble(&b, sequence_count, dev);
   return b.shader;
}

void
radv_device_finish_dgc_prepare_state(struct radv_device *device)
{
   radv_DestroyPipeline(radv_device_to_handle(device), device->meta_state.dgc_prepare.pipeline,
                        &device->meta_state.alloc);
   radv_DestroyPipelineLayout(radv_device_to_handle(device), device->meta_state.dgc_prepare.p_layout,
                              &device->meta_state.alloc);
   device->vk.dispatch_table.DestroyDescriptorSetLayout(
      radv_device_to_handle(device), device->meta_state.dgc_prepare.ds_layout, &device->meta_state.alloc);
}

VkResult
radv_device_init_dgc_prepare_state(struct radv_device *device)
{
   VkResult result;
   nir_shader *cs = build_dgc_prepare_shader(device);

   VkDescriptorSetLayoutCreateInfo ds_create_info = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                                     .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
                                                     .bindingCount = DGC_NUM_DESCS,
                                                     .pBindings = (VkDescriptorSetLayoutBinding[]){
                                                        {.binding = DGC_DESC_STREAM,
                                                         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                         .descriptorCount = 1,
                                                         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                                                         .pImmutableSamplers = NULL},
                                                        {.binding = DGC_DESC_PREPARE,
                                                         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                         .descriptorCount = 1,
                                                         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                                                         .pImmutableSamplers = NULL},
                                                        {.binding = DGC_DESC_PARAMS,
                                                         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                         .descriptorCount = 1,
                                                         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                                                         .pImmutableSamplers = NULL},
                                                        {.binding = DGC_DESC_COUNT,
                                                         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                         .descriptorCount = 1,
                                                         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                                                         .pImmutableSamplers = NULL},
                                                     }};

   result = radv_CreateDescriptorSetLayout(radv_device_to_handle(device), &ds_create_info, &device->meta_state.alloc,
                                           &device->meta_state.dgc_prepare.ds_layout);
   if (result != VK_SUCCESS)
      goto cleanup;

   const VkPipelineLayoutCreateInfo leaf_pl_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &device->meta_state.dgc_prepare.ds_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &(VkPushConstantRange){VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(struct radv_dgc_params)},
   };

   result = radv_CreatePipelineLayout(radv_device_to_handle(device), &leaf_pl_create_info, &device->meta_state.alloc,
                                      &device->meta_state.dgc_prepare.p_layout);
   if (result != VK_SUCCESS)
      goto cleanup;

   VkPipelineShaderStageCreateInfo shader_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = vk_shader_module_handle_from_nir(cs),
      .pName = "main",
      .pSpecializationInfo = NULL,
   };

   VkComputePipelineCreateInfo pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = shader_stage,
      .flags = 0,
      .layout = device->meta_state.dgc_prepare.p_layout,
   };

   result = radv_compute_pipeline_create(radv_device_to_handle(device), device->meta_state.cache, &pipeline_info,
                                         &device->meta_state.alloc, &device->meta_state.dgc_prepare.pipeline);
   if (result != VK_SUCCESS)
      goto cleanup;

cleanup:
   ralloc_free(cs);
   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_CreateIndirectCommandsLayoutNV(VkDevice _device, const VkIndirectCommandsLayoutCreateInfoNV *pCreateInfo,
                                    const VkAllocationCallbacks *pAllocator,
                                    VkIndirectCommandsLayoutNV *pIndirectCommandsLayout)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   struct radv_indirect_command_layout *layout;

   size_t size = sizeof(*layout) + pCreateInfo->tokenCount * sizeof(VkIndirectCommandsLayoutTokenNV);

   layout = vk_zalloc2(&device->vk.alloc, pAllocator, size, alignof(struct radv_indirect_command_layout),
                       VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!layout)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_object_base_init(&device->vk, &layout->base, VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV);

   layout->flags = pCreateInfo->flags;
   layout->pipeline_bind_point = pCreateInfo->pipelineBindPoint;
   layout->input_stride = pCreateInfo->pStreamStrides[0];
   layout->token_count = pCreateInfo->tokenCount;
   typed_memcpy(layout->tokens, pCreateInfo->pTokens, pCreateInfo->tokenCount);

   layout->ibo_type_32 = VK_INDEX_TYPE_UINT32;
   layout->ibo_type_8 = VK_INDEX_TYPE_UINT8_EXT;

   for (unsigned i = 0; i < pCreateInfo->tokenCount; ++i) {
      switch (pCreateInfo->pTokens[i].tokenType) {
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_NV:
         layout->draw_params_offset = pCreateInfo->pTokens[i].offset;
         break;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_INDEXED_NV:
         layout->indexed = true;
         layout->draw_params_offset = pCreateInfo->pTokens[i].offset;
         break;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DISPATCH_NV:
         layout->dispatch_params_offset = pCreateInfo->pTokens[i].offset;
         break;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_INDEX_BUFFER_NV:
         layout->binds_index_buffer = true;
         layout->index_buffer_offset = pCreateInfo->pTokens[i].offset;
         /* 16-bit is implied if we find no match. */
         for (unsigned j = 0; j < pCreateInfo->pTokens[i].indexTypeCount; j++) {
            if (pCreateInfo->pTokens[i].pIndexTypes[j] == VK_INDEX_TYPE_UINT32)
               layout->ibo_type_32 = pCreateInfo->pTokens[i].pIndexTypeValues[j];
            else if (pCreateInfo->pTokens[i].pIndexTypes[j] == VK_INDEX_TYPE_UINT8_EXT)
               layout->ibo_type_8 = pCreateInfo->pTokens[i].pIndexTypeValues[j];
         }
         break;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_VERTEX_BUFFER_NV:
         layout->bind_vbo_mask |= 1u << pCreateInfo->pTokens[i].vertexBindingUnit;
         layout->vbo_offsets[pCreateInfo->pTokens[i].vertexBindingUnit] = pCreateInfo->pTokens[i].offset;
         if (pCreateInfo->pTokens[i].vertexDynamicStride)
            layout->vbo_offsets[pCreateInfo->pTokens[i].vertexBindingUnit] |= DGC_DYNAMIC_STRIDE;
         break;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_PUSH_CONSTANT_NV:
         for (unsigned j = pCreateInfo->pTokens[i].pushconstantOffset / 4, k = 0;
              k < pCreateInfo->pTokens[i].pushconstantSize / 4; ++j, ++k) {
            layout->push_constant_mask |= 1ull << j;
            layout->push_constant_offsets[j] = pCreateInfo->pTokens[i].offset + k * 4;
         }
         break;
      case VK_INDIRECT_COMMANDS_TOKEN_TYPE_DRAW_MESH_TASKS_NV:
         layout->draw_mesh_tasks = true;
         layout->draw_params_offset = pCreateInfo->pTokens[i].offset;
         break;
      default:
         unreachable("Unhandled token type");
      }
   }
   if (!layout->indexed)
      layout->binds_index_buffer = false;

   *pIndirectCommandsLayout = radv_indirect_command_layout_to_handle(layout);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
radv_DestroyIndirectCommandsLayoutNV(VkDevice _device, VkIndirectCommandsLayoutNV indirectCommandsLayout,
                                     const VkAllocationCallbacks *pAllocator)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   VK_FROM_HANDLE(radv_indirect_command_layout, layout, indirectCommandsLayout);

   if (!layout)
      return;

   vk_object_base_finish(&layout->base);
   vk_free2(&device->vk.alloc, pAllocator, layout);
}

VKAPI_ATTR void VKAPI_CALL
radv_GetGeneratedCommandsMemoryRequirementsNV(VkDevice _device,
                                              const VkGeneratedCommandsMemoryRequirementsInfoNV *pInfo,
                                              VkMemoryRequirements2 *pMemoryRequirements)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   VK_FROM_HANDLE(radv_indirect_command_layout, layout, pInfo->indirectCommandsLayout);
   VK_FROM_HANDLE(radv_pipeline, pipeline, pInfo->pipeline);

   uint32_t cmd_stride, upload_stride;
   radv_get_sequence_size(layout, pipeline, &cmd_stride, &upload_stride);

   VkDeviceSize cmd_buf_size = radv_align_cmdbuf_size(device, cmd_stride * pInfo->maxSequencesCount, AMD_IP_GFX) +
                               radv_dgc_preamble_cmdbuf_size(device);
   VkDeviceSize upload_buf_size = upload_stride * pInfo->maxSequencesCount;

   pMemoryRequirements->memoryRequirements.memoryTypeBits = device->physical_device->memory_types_32bit;
   pMemoryRequirements->memoryRequirements.alignment =
      MAX2(device->physical_device->rad_info.ip[AMD_IP_GFX].ib_alignment,
           device->physical_device->rad_info.ip[AMD_IP_COMPUTE].ib_alignment);
   pMemoryRequirements->memoryRequirements.size =
      align(cmd_buf_size + upload_buf_size, pMemoryRequirements->memoryRequirements.alignment);
}

bool
radv_use_dgc_predication(struct radv_cmd_buffer *cmd_buffer, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo)
{
   VK_FROM_HANDLE(radv_buffer, seq_count_buffer, pGeneratedCommandsInfo->sequencesCountBuffer);

   /* Enable conditional rendering (if not enabled by user) to skip prepare/execute DGC calls when
    * the indirect sequence count might be zero. This can only be enabled on GFX because on ACE it's
    * not possible to skip the execute DGC call (ie. no INDIRECT_PACKET)
    */
   return cmd_buffer->qf == RADV_QUEUE_GENERAL && seq_count_buffer && !cmd_buffer->state.predicating;
}

static bool
radv_dgc_need_push_constants_copy(const struct radv_pipeline *pipeline)
{
   for (unsigned i = 0; i < ARRAY_SIZE(pipeline->shaders); ++i) {
      const struct radv_shader *shader = pipeline->shaders[i];

      if (!shader)
         continue;

      const struct radv_userdata_locations *locs = &shader->info.user_sgprs_locs;
      if (locs->shader_data[AC_UD_PUSH_CONSTANTS].sgpr_idx >= 0)
         return true;
   }

   return false;
}

bool
radv_dgc_can_preprocess(const struct radv_indirect_command_layout *layout, struct radv_pipeline *pipeline)
{
   if (!(layout->flags & VK_INDIRECT_COMMANDS_LAYOUT_USAGE_EXPLICIT_PREPROCESS_BIT_NV))
      return false;

   /* From the Vulkan spec (1.3.269, chapter 32):
    * "The bound descriptor sets and push constants that will be used with indirect command generation for the compute
    * piplines must already be specified at the time of preprocessing commands with vkCmdPreprocessGeneratedCommandsNV.
    * They must not change until the execution of indirect commands is submitted with vkCmdExecuteGeneratedCommandsNV."
    *
    * So we can always preprocess compute layouts.
    */
   if (layout->pipeline_bind_point != VK_PIPELINE_BIND_POINT_COMPUTE) {
      /* VBO binding (in particular partial VBO binding) uses some draw state which we don't generate at preprocess time
       * yet. */
      if (layout->bind_vbo_mask)
         return false;

      /* Do not preprocess when all push constants can't be inlined because they need to be copied
       * to the upload BO.
       */
      if (layout->push_constant_mask && radv_dgc_need_push_constants_copy(pipeline))
         return false;
   }

   return true;
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdPreprocessGeneratedCommandsNV(VkCommandBuffer commandBuffer,
                                      const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo)
{
   VK_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   VK_FROM_HANDLE(radv_indirect_command_layout, layout, pGeneratedCommandsInfo->indirectCommandsLayout);
   VK_FROM_HANDLE(radv_pipeline, pipeline, pGeneratedCommandsInfo->pipeline);

   if (!radv_dgc_can_preprocess(layout, pipeline))
      return;

   const bool use_predication = radv_use_dgc_predication(cmd_buffer, pGeneratedCommandsInfo);

   if (use_predication) {
      VK_FROM_HANDLE(radv_buffer, seq_count_buffer, pGeneratedCommandsInfo->sequencesCountBuffer);
      const uint64_t va = radv_buffer_get_va(seq_count_buffer->bo) + seq_count_buffer->offset +
                          pGeneratedCommandsInfo->sequencesCountOffset;

      radv_begin_conditional_rendering(cmd_buffer, va, true);
      cmd_buffer->state.predicating = true;
   }

   radv_prepare_dgc(cmd_buffer, pGeneratedCommandsInfo);

   if (use_predication) {
      cmd_buffer->state.predicating = false;
      radv_end_conditional_rendering(cmd_buffer);
   }
}

/* Always need to call this directly before draw due to dependence on bound state. */
static void
radv_prepare_dgc_graphics(struct radv_cmd_buffer *cmd_buffer, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo,
                          unsigned *upload_size, unsigned *upload_offset, void **upload_data,
                          struct radv_dgc_params *params)
{
   VK_FROM_HANDLE(radv_indirect_command_layout, layout, pGeneratedCommandsInfo->indirectCommandsLayout);
   VK_FROM_HANDLE(radv_pipeline, pipeline, pGeneratedCommandsInfo->pipeline);
   struct radv_graphics_pipeline *graphics_pipeline = radv_pipeline_to_graphics(pipeline);
   struct radv_shader *vs = radv_get_shader(graphics_pipeline->base.shaders, MESA_SHADER_VERTEX);
   unsigned vb_size = layout->bind_vbo_mask ? util_bitcount(vs->info.vs.vb_desc_usage_mask) * 24 : 0;

   *upload_size = MAX2(*upload_size + vb_size, 16);

   if (!radv_cmd_buffer_upload_alloc(cmd_buffer, *upload_size, upload_offset, upload_data)) {
      vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
      return;
   }

   uint16_t vtx_base_sgpr = 0;

   if (cmd_buffer->state.graphics_pipeline->vtx_base_sgpr)
      vtx_base_sgpr = (cmd_buffer->state.graphics_pipeline->vtx_base_sgpr - SI_SH_REG_OFFSET) >> 2;

   if (cmd_buffer->state.graphics_pipeline->uses_drawid)
      vtx_base_sgpr |= DGC_USES_DRAWID;

   if (layout->draw_mesh_tasks) {
      struct radv_shader *mesh_shader = radv_get_shader(graphics_pipeline->base.shaders, MESA_SHADER_MESH);
      if (mesh_shader->info.cs.uses_grid_size)
         vtx_base_sgpr |= DGC_USES_GRID_SIZE;
   } else {
      if (cmd_buffer->state.graphics_pipeline->uses_baseinstance)
         vtx_base_sgpr |= DGC_USES_BASEINSTANCE;
   }

   params->draw_indexed = layout->indexed;
   params->draw_params_offset = layout->draw_params_offset;
   params->binds_index_buffer = layout->binds_index_buffer;
   params->vtx_base_sgpr = vtx_base_sgpr;
   params->max_index_count = cmd_buffer->state.max_index_count;
   params->index_buffer_offset = layout->index_buffer_offset;
   params->ibo_type_32 = layout->ibo_type_32;
   params->ibo_type_8 = layout->ibo_type_8;
   params->draw_mesh_tasks = layout->draw_mesh_tasks;

   if (layout->bind_vbo_mask) {
      uint32_t mask = vs->info.vs.vb_desc_usage_mask;
      unsigned vb_desc_alloc_size = util_bitcount(mask) * 16;

      radv_write_vertex_descriptors(cmd_buffer, graphics_pipeline, true, *upload_data);

      uint32_t *vbo_info = (uint32_t *)((char *)*upload_data + vb_desc_alloc_size);

      unsigned idx = 0;
      while (mask) {
         unsigned i = u_bit_scan(&mask);
         unsigned binding = vs->info.vs.use_per_attribute_vb_descs ? graphics_pipeline->attrib_bindings[i] : i;
         uint32_t attrib_end = graphics_pipeline->attrib_ends[i];

         params->vbo_bind_mask |= ((layout->bind_vbo_mask >> binding) & 1u) << idx;
         vbo_info[2 * idx] = ((vs->info.vs.use_per_attribute_vb_descs ? 1u : 0u) << 31) | layout->vbo_offsets[binding];
         vbo_info[2 * idx + 1] = graphics_pipeline->attrib_index_offset[i] | (attrib_end << 16);
         ++idx;
      }
      params->vbo_cnt = idx;
      params->vbo_reg =
         ((radv_get_user_sgpr(vs, AC_UD_VS_VERTEX_BUFFERS)->sgpr_idx * 4 + vs->info.user_data_0) - SI_SH_REG_OFFSET) >>
         2;
      *upload_data = (char *)*upload_data + vb_size;
   }
}

static void
radv_prepare_dgc_compute(struct radv_cmd_buffer *cmd_buffer, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo,
                         unsigned *upload_size, unsigned *upload_offset, void **upload_data,
                         struct radv_dgc_params *params)
{
   VK_FROM_HANDLE(radv_indirect_command_layout, layout, pGeneratedCommandsInfo->indirectCommandsLayout);
   VK_FROM_HANDLE(radv_pipeline, pipeline, pGeneratedCommandsInfo->pipeline);
   struct radv_compute_pipeline *compute_pipeline = radv_pipeline_to_compute(pipeline);
   struct radv_shader *cs = radv_get_shader(compute_pipeline->base.shaders, MESA_SHADER_COMPUTE);

   *upload_size = MAX2(*upload_size, 16);

   if (!radv_cmd_buffer_upload_alloc(cmd_buffer, *upload_size, upload_offset, upload_data)) {
      vk_command_buffer_set_error(&cmd_buffer->vk, VK_ERROR_OUT_OF_HOST_MEMORY);
      return;
   }

   uint32_t dispatch_initiator = cmd_buffer->device->dispatch_initiator;
   dispatch_initiator |= S_00B800_FORCE_START_AT_000(1);
   if (cs->info.wave_size == 32) {
      assert(cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX10);
      dispatch_initiator |= S_00B800_CS_W32_EN(1);
   }

   params->dispatch_params_offset = layout->dispatch_params_offset;
   params->dispatch_initiator = dispatch_initiator;
   params->is_dispatch = 1;

   const struct radv_userdata_info *loc = radv_get_user_sgpr(cs, AC_UD_CS_GRID_SIZE);
   if (loc->sgpr_idx != -1) {
      params->grid_base_sgpr = (cs->info.user_data_0 + 4 * loc->sgpr_idx - SI_SH_REG_OFFSET) >> 2;
   }
}

void
radv_prepare_dgc(struct radv_cmd_buffer *cmd_buffer, const VkGeneratedCommandsInfoNV *pGeneratedCommandsInfo)
{
   VK_FROM_HANDLE(radv_indirect_command_layout, layout, pGeneratedCommandsInfo->indirectCommandsLayout);
   VK_FROM_HANDLE(radv_pipeline, pipeline, pGeneratedCommandsInfo->pipeline);
   VK_FROM_HANDLE(radv_buffer, prep_buffer, pGeneratedCommandsInfo->preprocessBuffer);
   VK_FROM_HANDLE(radv_buffer, stream_buffer, pGeneratedCommandsInfo->pStreams[0].buffer);
   struct radv_meta_saved_state saved_state;
   unsigned upload_offset, upload_size;
   struct radv_buffer token_buffer;
   void *upload_data;

   uint32_t cmd_stride, upload_stride;
   radv_get_sequence_size(layout, pipeline, &cmd_stride, &upload_stride);

   unsigned cmd_buf_size =
      radv_align_cmdbuf_size(cmd_buffer->device, cmd_stride * pGeneratedCommandsInfo->sequencesCount, AMD_IP_GFX);

   uint64_t upload_addr =
      radv_buffer_get_va(prep_buffer->bo) + prep_buffer->offset + pGeneratedCommandsInfo->preprocessOffset;

   uint64_t stream_addr =
      radv_buffer_get_va(stream_buffer->bo) + stream_buffer->offset + pGeneratedCommandsInfo->pStreams[0].offset;

   struct radv_dgc_params params = {
      .cmd_buf_stride = cmd_stride,
      .cmd_buf_size = cmd_buf_size,
      .upload_addr = (uint32_t)upload_addr,
      .upload_stride = upload_stride,
      .sequence_count = pGeneratedCommandsInfo->sequencesCount,
      .stream_stride = layout->input_stride,
      .use_preamble = radv_dgc_use_preamble(pGeneratedCommandsInfo),
      .stream_addr = stream_addr,
   };

   upload_size = pipeline->push_constant_size + 16 * pipeline->dynamic_offset_count +
                 sizeof(layout->push_constant_offsets) + ARRAY_SIZE(pipeline->shaders) * 12;
   if (!layout->push_constant_mask)
      upload_size = 0;

   if (layout->pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS) {
      radv_prepare_dgc_graphics(cmd_buffer, pGeneratedCommandsInfo, &upload_size, &upload_offset, &upload_data,
                                &params);
   } else {
      assert(layout->pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE);
      radv_prepare_dgc_compute(cmd_buffer, pGeneratedCommandsInfo, &upload_size, &upload_offset, &upload_data, &params);
   }

   if (layout->push_constant_mask) {
      uint32_t *desc = upload_data;
      upload_data = (char *)upload_data + ARRAY_SIZE(pipeline->shaders) * 12;

      unsigned idx = 0;
      for (unsigned i = 0; i < ARRAY_SIZE(pipeline->shaders); ++i) {
         if (!pipeline->shaders[i])
            continue;

         const struct radv_shader *shader = pipeline->shaders[i];
         const struct radv_userdata_locations *locs = &shader->info.user_sgprs_locs;
         if (locs->shader_data[AC_UD_PUSH_CONSTANTS].sgpr_idx >= 0)
            params.const_copy = 1;

         if (locs->shader_data[AC_UD_PUSH_CONSTANTS].sgpr_idx >= 0 ||
             locs->shader_data[AC_UD_INLINE_PUSH_CONSTANTS].sgpr_idx >= 0) {
            unsigned upload_sgpr = 0;
            unsigned inline_sgpr = 0;

            if (locs->shader_data[AC_UD_PUSH_CONSTANTS].sgpr_idx >= 0) {
               upload_sgpr = (shader->info.user_data_0 + 4 * locs->shader_data[AC_UD_PUSH_CONSTANTS].sgpr_idx -
                              SI_SH_REG_OFFSET) >>
                             2;
            }

            if (locs->shader_data[AC_UD_INLINE_PUSH_CONSTANTS].sgpr_idx >= 0) {
               inline_sgpr = (shader->info.user_data_0 + 4 * locs->shader_data[AC_UD_INLINE_PUSH_CONSTANTS].sgpr_idx -
                              SI_SH_REG_OFFSET) >>
                             2;
               desc[idx * 3 + 1] = pipeline->shaders[i]->info.inline_push_constant_mask;
               desc[idx * 3 + 2] = pipeline->shaders[i]->info.inline_push_constant_mask >> 32;
            }
            desc[idx * 3] = upload_sgpr | (inline_sgpr << 16);
            ++idx;
         }
      }

      params.push_constant_shader_cnt = idx;

      params.const_copy_size = pipeline->push_constant_size + 16 * pipeline->dynamic_offset_count;
      params.push_constant_mask = layout->push_constant_mask;

      memcpy(upload_data, layout->push_constant_offsets, sizeof(layout->push_constant_offsets));
      upload_data = (char *)upload_data + sizeof(layout->push_constant_offsets);

      memcpy(upload_data, cmd_buffer->push_constants, pipeline->push_constant_size);
      upload_data = (char *)upload_data + pipeline->push_constant_size;

      struct radv_descriptor_state *descriptors_state =
         radv_get_descriptors_state(cmd_buffer, pGeneratedCommandsInfo->pipelineBindPoint);
      memcpy(upload_data, descriptors_state->dynamic_buffers, 16 * pipeline->dynamic_offset_count);
      upload_data = (char *)upload_data + 16 * pipeline->dynamic_offset_count;
   }

   radv_buffer_init(&token_buffer, cmd_buffer->device, cmd_buffer->upload.upload_bo, upload_size, upload_offset);

   VkWriteDescriptorSet ds_writes[5];
   VkDescriptorBufferInfo buf_info[ARRAY_SIZE(ds_writes)];
   int ds_cnt = 0;
   buf_info[ds_cnt] =
      (VkDescriptorBufferInfo){.buffer = radv_buffer_to_handle(&token_buffer), .offset = 0, .range = upload_size};
   ds_writes[ds_cnt] = (VkWriteDescriptorSet){.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                              .dstBinding = DGC_DESC_PARAMS,
                                              .dstArrayElement = 0,
                                              .descriptorCount = 1,
                                              .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                              .pBufferInfo = &buf_info[ds_cnt]};
   ++ds_cnt;

   buf_info[ds_cnt] = (VkDescriptorBufferInfo){.buffer = pGeneratedCommandsInfo->preprocessBuffer,
                                               .offset = pGeneratedCommandsInfo->preprocessOffset,
                                               .range = pGeneratedCommandsInfo->preprocessSize};
   ds_writes[ds_cnt] = (VkWriteDescriptorSet){.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                              .dstBinding = DGC_DESC_PREPARE,
                                              .dstArrayElement = 0,
                                              .descriptorCount = 1,
                                              .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                              .pBufferInfo = &buf_info[ds_cnt]};
   ++ds_cnt;

   if (pGeneratedCommandsInfo->streamCount > 0) {
      buf_info[ds_cnt] = (VkDescriptorBufferInfo){.buffer = pGeneratedCommandsInfo->pStreams[0].buffer,
                                                  .offset = pGeneratedCommandsInfo->pStreams[0].offset,
                                                  .range = VK_WHOLE_SIZE};
      ds_writes[ds_cnt] = (VkWriteDescriptorSet){.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                 .dstBinding = DGC_DESC_STREAM,
                                                 .dstArrayElement = 0,
                                                 .descriptorCount = 1,
                                                 .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                 .pBufferInfo = &buf_info[ds_cnt]};
      ++ds_cnt;
   }

   if (pGeneratedCommandsInfo->sequencesCountBuffer != VK_NULL_HANDLE) {
      buf_info[ds_cnt] = (VkDescriptorBufferInfo){.buffer = pGeneratedCommandsInfo->sequencesCountBuffer,
                                                  .offset = pGeneratedCommandsInfo->sequencesCountOffset,
                                                  .range = VK_WHOLE_SIZE};
      ds_writes[ds_cnt] = (VkWriteDescriptorSet){.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                                 .dstBinding = DGC_DESC_COUNT,
                                                 .dstArrayElement = 0,
                                                 .descriptorCount = 1,
                                                 .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                                 .pBufferInfo = &buf_info[ds_cnt]};
      ++ds_cnt;
      params.sequence_count |= 1u << 31;
   }

   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_COMPUTE_PIPELINE | RADV_META_SAVE_DESCRIPTORS | RADV_META_SAVE_CONSTANTS);

   radv_CmdBindPipeline(radv_cmd_buffer_to_handle(cmd_buffer), VK_PIPELINE_BIND_POINT_COMPUTE,
                        cmd_buffer->device->meta_state.dgc_prepare.pipeline);

   vk_common_CmdPushConstants(radv_cmd_buffer_to_handle(cmd_buffer),
                              cmd_buffer->device->meta_state.dgc_prepare.p_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                              sizeof(params), &params);

   radv_meta_push_descriptor_set(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                 cmd_buffer->device->meta_state.dgc_prepare.p_layout, 0, ds_cnt, ds_writes);

   unsigned block_count = MAX2(1, DIV_ROUND_UP(pGeneratedCommandsInfo->sequencesCount, 64));
   vk_common_CmdDispatch(radv_cmd_buffer_to_handle(cmd_buffer), block_count, 1, 1);

   radv_buffer_finish(&token_buffer);
   radv_meta_restore(&saved_state, cmd_buffer);
}

/* VK_NV_device_generated_commands_compute */
VKAPI_ATTR void VKAPI_CALL
radv_GetPipelineIndirectMemoryRequirementsNV(VkDevice device, const VkComputePipelineCreateInfo *pCreateInfo,
                                             VkMemoryRequirements2 *pMemoryRequirements)
{
   unreachable("radv: unimplemented vkGetPipelineIndirectMemoryRequirementsNV");
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL
radv_GetPipelineIndirectDeviceAddressNV(VkDevice device, const VkPipelineIndirectDeviceAddressInfoNV *pInfo)
{
   unreachable("radv: unimplemented vkGetPipelineIndirectDeviceAddressNV");
}
