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

#include <string.h>

#include "util/macros.h"

#include "aubinator_viewer.h"

void
aub_viewer_decode_ctx_init(struct aub_viewer_decode_ctx *ctx,
                           struct aub_viewer_cfg *cfg,
                           struct aub_viewer_decode_cfg *decode_cfg,
                           const struct intel_device_info *devinfo,
                           struct intel_spec *spec,
                           struct intel_batch_decode_bo (*get_bo)(void *, bool, uint64_t),
                           unsigned (*get_state_size)(void *, uint32_t),
                           void *user_data)
{
   memset(ctx, 0, sizeof(*ctx));

   ctx->get_bo = get_bo;
   ctx->get_state_size = get_state_size;
   ctx->user_data = user_data;
   ctx->devinfo = devinfo;
   ctx->engine = INTEL_ENGINE_CLASS_RENDER;

   ctx->cfg = cfg;
   ctx->decode_cfg = decode_cfg;
   ctx->spec = spec;
}

static void
aub_viewer_print_group(struct aub_viewer_decode_ctx *ctx,
                       struct intel_group *group,
                       uint64_t address, const void *map)
{
   struct intel_field_iterator iter;
   int last_dword = -1;
   const uint32_t *p = (const uint32_t *) map;

   intel_field_iterator_init(&iter, group, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (ctx->decode_cfg->show_dwords) {
         int iter_dword = iter.end_bit / 32;
         if (last_dword != iter_dword) {
            for (int i = last_dword + 1; i <= iter_dword; i++) {
               ImGui::TextColored(ctx->cfg->dwords_color,
                                  "0x%012" PRIx64 ":  0x%012x : Dword %d",
                                  address + 4 * i, iter.p[i], i);
            }
            last_dword = iter_dword;
         }
      }
      if (!intel_field_is_header(iter.field)) {
         if (ctx->decode_cfg->field_filter.PassFilter(iter.name)) {
            if (iter.field->type.kind == intel_type::INTEL_TYPE_BOOL && iter.raw_value) {
               ImGui::Text("%s: ", iter.name); ImGui::SameLine();
               ImGui::TextColored(ctx->cfg->boolean_color, "true");
            } else {
               ImGui::Text("%s: %s", iter.name, iter.value);
            }
            if (iter.struct_desc) {
               int struct_dword = iter.start_bit / 32;
               uint64_t struct_address = address + 4 * struct_dword;
               aub_viewer_print_group(ctx, iter.struct_desc, struct_address,
                                      &p[struct_dword]);
            }
         }
      }
   }
}

static struct intel_batch_decode_bo
ctx_get_bo(struct aub_viewer_decode_ctx *ctx, bool ppgtt, uint64_t addr)
{
   if (intel_spec_get_gen(ctx->spec) >= intel_make_gen(8,0)) {
      /* On Broadwell and above, we have 48-bit addresses which consume two
       * dwords.  Some packets require that these get stored in a "canonical
       * form" which means that bit 47 is sign-extended through the upper
       * bits. In order to correctly handle those aub dumps, we need to mask
       * off the top 16 bits.
       */
      addr &= (~0ull >> 16);
   }

   struct intel_batch_decode_bo bo = ctx->get_bo(ctx->user_data, ppgtt, addr);

   if (intel_spec_get_gen(ctx->spec) >= intel_make_gen(8,0))
      bo.addr &= (~0ull >> 16);

   /* We may actually have an offset into the bo */
   if (bo.map != NULL) {
      assert(bo.addr <= addr);
      uint64_t offset = addr - bo.addr;
      bo.map = (const uint8_t *)bo.map + offset;
      bo.addr += offset;
      bo.size -= offset;
   }

   return bo;
}

static int
update_count(struct aub_viewer_decode_ctx *ctx,
             uint32_t offset_from_dsba,
             unsigned element_dwords,
             unsigned guess)
{
   unsigned size = 0;

   if (ctx->get_state_size)
      size = ctx->get_state_size(ctx->user_data, offset_from_dsba);

   if (size > 0)
      return size / (sizeof(uint32_t) * element_dwords);

   /* In the absence of any information, just guess arbitrarily. */
   return guess;
}

static void
ctx_disassemble_program(struct aub_viewer_decode_ctx *ctx,
                        uint32_t ksp, const char *type)
{
   uint64_t addr = ctx->instruction_base + ksp;
   struct intel_batch_decode_bo bo = ctx_get_bo(ctx, true, addr);
   if (!bo.map) {
      ImGui::TextColored(ctx->cfg->missing_color,
                         "Shader unavailable addr=0x%012" PRIx64, addr);
      return;
   }

   ImGui::PushID(addr);
   if (ImGui::Button(type) && ctx->display_shader)
      ctx->display_shader(ctx->user_data, type, addr);
   ImGui::PopID();
}

static void
handle_state_base_address(struct aub_viewer_decode_ctx *ctx,
                          struct intel_group *inst,
                          const uint32_t *p)
{
   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);

   uint64_t surface_base = 0, dynamic_base = 0, instruction_base = 0;
   bool surface_modify = 0, dynamic_modify = 0, instruction_modify = 0;

   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "Surface State Base Address") == 0) {
         surface_base = iter.raw_value;
      } else if (strcmp(iter.name, "Dynamic State Base Address") == 0) {
         dynamic_base = iter.raw_value;
      } else if (strcmp(iter.name, "Instruction Base Address") == 0) {
         instruction_base = iter.raw_value;
      } else if (strcmp(iter.name, "Surface State Base Address Modify Enable") == 0) {
         surface_modify = iter.raw_value;
      } else if (strcmp(iter.name, "Dynamic State Base Address Modify Enable") == 0) {
         dynamic_modify = iter.raw_value;
      } else if (strcmp(iter.name, "Instruction Base Address Modify Enable") == 0) {
         instruction_modify = iter.raw_value;
      }
   }

   if (dynamic_modify)
      ctx->dynamic_base = dynamic_base;

   if (surface_modify)
      ctx->surface_base = surface_base;

   if (instruction_modify)
      ctx->instruction_base = instruction_base;
}

static void
dump_binding_table(struct aub_viewer_decode_ctx *ctx, uint32_t offset, int count)
{
   struct intel_group *strct =
      intel_spec_find_struct(ctx->spec, "RENDER_SURFACE_STATE");
   if (strct == NULL) {
      ImGui::TextColored(ctx->cfg->missing_color, "did not find RENDER_SURFACE_STATE info");
      return;
   }

   if (count < 0)
      count = update_count(ctx, offset, 1, 8);

   if (offset % 32 != 0 || offset >= UINT16_MAX) {
      ImGui::TextColored(ctx->cfg->missing_color, "invalid binding table pointer");
      return;
   }

   struct intel_batch_decode_bo bind_bo =
      ctx_get_bo(ctx, true, ctx->surface_base + offset);

   if (bind_bo.map == NULL) {
      ImGui::TextColored(ctx->cfg->missing_color,
                         "binding table unavailable addr=0x%012" PRIx64,
                         ctx->surface_base + offset);
      return;
   }

   const uint32_t *pointers = (const uint32_t *) bind_bo.map;
   for (int i = 0; i < count; i++) {
      if (pointers[i] == 0)
         continue;

      uint64_t addr = ctx->surface_base + pointers[i];
      struct intel_batch_decode_bo bo = ctx_get_bo(ctx, true, addr);
      uint32_t size = strct->dw_length * 4;

      if (pointers[i] % 32 != 0 ||
          addr < bo.addr || addr + size >= bo.addr + bo.size) {
         ImGui::TextColored(ctx->cfg->missing_color,
                            "pointer %u: %012x <not valid>", i, pointers[i]);
         continue;
      }

      const uint8_t *state = (const uint8_t *) bo.map + (addr - bo.addr);
      if (ImGui::TreeNodeEx(&pointers[i], ImGuiTreeNodeFlags_Framed,
                            "pointer %u: %012x", i, pointers[i])) {
         aub_viewer_print_group(ctx, strct, addr, state);
         ImGui::TreePop();
      }
   }
}

static void
dump_samplers(struct aub_viewer_decode_ctx *ctx, uint32_t offset, int count)
{
   struct intel_group *strct = intel_spec_find_struct(ctx->spec, "SAMPLER_STATE");

   uint64_t state_addr = ctx->dynamic_base + offset;
   struct intel_batch_decode_bo bo = ctx_get_bo(ctx, true, state_addr);
   const uint8_t *state_map = (const uint8_t *) bo.map;

   if (state_map == NULL) {
      ImGui::TextColored(ctx->cfg->missing_color,
                         "samplers unavailable addr=0x%012" PRIx64, state_addr);
      return;
   }

   if (offset % 32 != 0) {
      ImGui::TextColored(ctx->cfg->missing_color, "invalid sampler state pointer");
      return;
   }

   const unsigned sampler_state_size = strct->dw_length * 4;

   if (count * sampler_state_size >= bo.size) {
      ImGui::TextColored(ctx->cfg->missing_color, "sampler state ends after bo ends");
      return;
   }

   for (int i = 0; i < count; i++) {
      if (ImGui::TreeNodeEx(state_map, ImGuiTreeNodeFlags_Framed,
                            "sampler state %d", i)) {
         aub_viewer_print_group(ctx, strct, state_addr, state_map);
         ImGui::TreePop();
      }
      state_addr += sampler_state_size;
      state_map += sampler_state_size;
   }
}

static void
handle_media_interface_descriptor_load(struct aub_viewer_decode_ctx *ctx,
                                       struct intel_group *inst,
                                       const uint32_t *p)
{
   struct intel_group *desc =
      intel_spec_find_struct(ctx->spec, "INTERFACE_DESCRIPTOR_DATA");

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   uint32_t descriptor_offset = 0;
   int descriptor_count = 0;
   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "Interface Descriptor Data Start Address") == 0) {
         descriptor_offset = strtol(iter.value, NULL, 16);
      } else if (strcmp(iter.name, "Interface Descriptor Total Length") == 0) {
         descriptor_count =
            strtol(iter.value, NULL, 16) / (desc->dw_length * 4);
      }
   }

   uint64_t desc_addr = ctx->dynamic_base + descriptor_offset;
   struct intel_batch_decode_bo bo = ctx_get_bo(ctx, true, desc_addr);
   const uint32_t *desc_map = (const uint32_t *) bo.map;

   if (desc_map == NULL) {
      ImGui::TextColored(ctx->cfg->missing_color,
                         "interface descriptors unavailable addr=0x%012" PRIx64, desc_addr);
      return;
   }

   for (int i = 0; i < descriptor_count; i++) {
      ImGui::Text("descriptor %d: %012x", i, descriptor_offset);

      aub_viewer_print_group(ctx, desc, desc_addr, desc_map);

      intel_field_iterator_init(&iter, desc, desc_map, 0, false);
      uint64_t ksp = 0;
      uint32_t sampler_offset = 0, sampler_count = 0;
      uint32_t binding_table_offset = 0, binding_entry_count = 0;
      while (intel_field_iterator_next(&iter)) {
         if (strcmp(iter.name, "Kernel Start Pointer") == 0) {
            ksp = strtoll(iter.value, NULL, 16);
         } else if (strcmp(iter.name, "Sampler State Pointer") == 0) {
            sampler_offset = strtol(iter.value, NULL, 16);
         } else if (strcmp(iter.name, "Sampler Count") == 0) {
            sampler_count = strtol(iter.value, NULL, 10);
         } else if (strcmp(iter.name, "Binding Table Pointer") == 0) {
            binding_table_offset = strtol(iter.value, NULL, 16);
         } else if (strcmp(iter.name, "Binding Table Entry Count") == 0) {
            binding_entry_count = strtol(iter.value, NULL, 10);
         }
      }

      ctx_disassemble_program(ctx, ksp, "compute shader");

      dump_samplers(ctx, sampler_offset, sampler_count);
      dump_binding_table(ctx, binding_table_offset, binding_entry_count);

      desc_map += desc->dw_length;
      desc_addr += desc->dw_length * 4;
   }
}

static void
handle_3dstate_vertex_buffers(struct aub_viewer_decode_ctx *ctx,
                              struct intel_group *inst,
                              const uint32_t *p)
{
   struct intel_group *vbs = intel_spec_find_struct(ctx->spec, "VERTEX_BUFFER_STATE");

   struct intel_batch_decode_bo vb = {};
   uint32_t vb_size = 0;
   int index = -1;
   int pitch = -1;
   bool ready = false;

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (iter.struct_desc != vbs)
         continue;

      uint64_t buffer_addr = 0;

      struct intel_field_iterator vbs_iter;
      intel_field_iterator_init(&vbs_iter, vbs, &iter.p[iter.start_bit / 32], 0, false);
      while (intel_field_iterator_next(&vbs_iter)) {
         if (strcmp(vbs_iter.name, "Vertex Buffer Index") == 0) {
            index = vbs_iter.raw_value;
         } else if (strcmp(vbs_iter.name, "Buffer Pitch") == 0) {
            pitch = vbs_iter.raw_value;
         } else if (strcmp(vbs_iter.name, "Buffer Starting Address") == 0) {
            buffer_addr = vbs_iter.raw_value;
            vb = ctx_get_bo(ctx, true, buffer_addr);
         } else if (strcmp(vbs_iter.name, "Buffer Size") == 0) {
            vb_size = vbs_iter.raw_value;
            ready = true;
         } else if (strcmp(vbs_iter.name, "End Address") == 0) {
            if (vb.map && vbs_iter.raw_value >= vb.addr)
               vb_size = vbs_iter.raw_value - vb.addr;
            else
               vb_size = 0;
            ready = true;
         }

         if (!ready)
            continue;

         ImGui::Text("vertex buffer %d, size %d, pitch %d", index, vb_size, pitch);

         if (vb.map == NULL) {
            ImGui::TextColored(ctx->cfg->missing_color,
                               "buffer contents unavailable addr=0x%012" PRIx64, buffer_addr);
            continue;
         }

         if (vb.map == 0 || vb_size == 0)
            continue;

         vb.map = NULL;
         vb_size = 0;
         index = -1;
         pitch = -1;
         ready = false;
      }
   }
}

static void
handle_3dstate_index_buffer(struct aub_viewer_decode_ctx *ctx,
                            struct intel_group *inst,
                            const uint32_t *p)
{
   struct intel_batch_decode_bo ib = {};
   uint64_t buffer_addr = 0;
   uint32_t ib_size = 0;
   uint32_t format = 0;

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "Index Format") == 0) {
         format = iter.raw_value;
      } else if (strcmp(iter.name, "Buffer Starting Address") == 0) {
         buffer_addr = iter.raw_value;
         ib = ctx_get_bo(ctx, true, buffer_addr);
      } else if (strcmp(iter.name, "Buffer Size") == 0) {
         ib_size = iter.raw_value;
      }
   }

   if (ib.map == NULL) {
      ImGui::TextColored(ctx->cfg->missing_color,
                         "buffer contents unavailable addr=0x%012" PRIx64,
                         buffer_addr);
      return;
   }

   const uint8_t *m = (const uint8_t *) ib.map;
   const uint8_t *ib_end = m + MIN2(ib.size, ib_size);
   for (int i = 0; m < ib_end && i < 10; i++) {
      switch (format) {
      case 0:
         m += 1;
         break;
      case 1:
         m += 2;
         break;
      case 2:
         m += 4;
         break;
      }
   }
}

static void
decode_single_ksp(struct aub_viewer_decode_ctx *ctx,
                  struct intel_group *inst,
                  const uint32_t *p)
{
   uint64_t ksp = 0;
   bool is_simd8 = false; /* vertex shaders on Gfx8+ only */
   bool is_enabled = true;

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "Kernel Start Pointer") == 0) {
         ksp = iter.raw_value;
      } else if (strcmp(iter.name, "SIMD8 Dispatch Enable") == 0) {
         is_simd8 = iter.raw_value;
      } else if (strcmp(iter.name, "Dispatch Mode") == 0) {
         is_simd8 = strcmp(iter.value, "SIMD8") == 0;
      } else if (strcmp(iter.name, "Dispatch Enable") == 0) {
         is_simd8 = strcmp(iter.value, "SIMD8") == 0;
      } else if (strcmp(iter.name, "Enable") == 0) {
         is_enabled = iter.raw_value;
      }
   }

   const char *type =
      strcmp(inst->name,   "VS_STATE") == 0 ? "vertex shader" :
      strcmp(inst->name,   "GS_STATE") == 0 ? "geometry shader" :
      strcmp(inst->name,   "SF_STATE") == 0 ? "strips and fans shader" :
      strcmp(inst->name, "CLIP_STATE") == 0 ? "clip shader" :
      strcmp(inst->name, "3DSTATE_DS") == 0 ? "tessellation evaluation shader" :
      strcmp(inst->name, "3DSTATE_HS") == 0 ? "tessellation control shader" :
      strcmp(inst->name, "3DSTATE_VS") == 0 ? (is_simd8 ? "SIMD8 vertex shader" : "vec4 vertex shader") :
      strcmp(inst->name, "3DSTATE_GS") == 0 ? (is_simd8 ? "SIMD8 geometry shader" : "vec4 geometry shader") :
      NULL;

   if (is_enabled)
      ctx_disassemble_program(ctx, ksp, type);
}

static void
decode_ps_kernels(struct aub_viewer_decode_ctx *ctx,
                  struct intel_group *inst,
                  const uint32_t *p)
{
   uint64_t ksp[3] = {0, 0, 0};
   bool enabled[3] = {false, false, false};

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (strncmp(iter.name, "Kernel Start Pointer ",
                  strlen("Kernel Start Pointer ")) == 0) {
         int idx = iter.name[strlen("Kernel Start Pointer ")] - '0';
         ksp[idx] = strtol(iter.value, NULL, 16);
      } else if (strcmp(iter.name, "8 Pixel Dispatch Enable") == 0) {
         enabled[0] = strcmp(iter.value, "true") == 0;
      } else if (strcmp(iter.name, "16 Pixel Dispatch Enable") == 0) {
         enabled[1] = strcmp(iter.value, "true") == 0;
      } else if (strcmp(iter.name, "32 Pixel Dispatch Enable") == 0) {
         enabled[2] = strcmp(iter.value, "true") == 0;
      }
   }

   /* Reorder KSPs to be [8, 16, 32] instead of the hardware order. */
   if (enabled[0] + enabled[1] + enabled[2] == 1) {
      if (enabled[1]) {
         ksp[1] = ksp[0];
         ksp[0] = 0;
      } else if (enabled[2]) {
         ksp[2] = ksp[0];
         ksp[0] = 0;
      }
   } else {
      uint64_t tmp = ksp[1];
      ksp[1] = ksp[2];
      ksp[2] = tmp;
   }

   if (enabled[0])
      ctx_disassemble_program(ctx, ksp[0], "SIMD8 fragment shader");
   if (enabled[1])
      ctx_disassemble_program(ctx, ksp[1], "SIMD16 fragment shader");
   if (enabled[2])
      ctx_disassemble_program(ctx, ksp[2], "SIMD32 fragment shader");
}

static void
decode_3dstate_constant(struct aub_viewer_decode_ctx *ctx,
                        struct intel_group *inst,
                        const uint32_t *p)
{
   struct intel_group *body =
      intel_spec_find_struct(ctx->spec, "3DSTATE_CONSTANT_BODY");

   uint32_t read_length[4] = {0};
   uint64_t read_addr[4];

   struct intel_field_iterator outer;
   intel_field_iterator_init(&outer, inst, p, 0, false);
   while (intel_field_iterator_next(&outer)) {
      if (outer.struct_desc != body)
         continue;

      struct intel_field_iterator iter;
      intel_field_iterator_init(&iter, body, &outer.p[outer.start_bit / 32],
                                0, false);

      while (intel_field_iterator_next(&iter)) {
         int idx;
         if (sscanf(iter.name, "Read Length[%d]", &idx) == 1) {
            read_length[idx] = iter.raw_value;
         } else if (sscanf(iter.name, "Buffer[%d]", &idx) == 1) {
            read_addr[idx] = iter.raw_value;
         }
      }

      for (int i = 0; i < 4; i++) {
         if (read_length[i] == 0)
            continue;

         struct intel_batch_decode_bo buffer = ctx_get_bo(ctx, true, read_addr[i]);
         if (!buffer.map) {
            ImGui::TextColored(ctx->cfg->missing_color,
                               "constant buffer %d unavailable addr=0x%012" PRIx64,
                               i, read_addr[i]);
            continue;
         }

         unsigned size = read_length[i] * 32;
         ImGui::Text("constant buffer %d, size %u", i, size);

         if (ctx->edit_address) {
            if (ImGui::Button("Show/Edit buffer"))
               ctx->edit_address(ctx->user_data, read_addr[i], size);
         }
      }
   }
}

static void
decode_3dstate_binding_table_pointers(struct aub_viewer_decode_ctx *ctx,
                                      struct intel_group *inst,
                                      const uint32_t *p)
{
   dump_binding_table(ctx, p[1], -1);
}

static void
decode_3dstate_sampler_state_pointers(struct aub_viewer_decode_ctx *ctx,
                                      struct intel_group *inst,
                                      const uint32_t *p)
{
   dump_samplers(ctx, p[1], 1);
}

static void
decode_3dstate_sampler_state_pointers_gfx6(struct aub_viewer_decode_ctx *ctx,
                                           struct intel_group *inst,
                                           const uint32_t *p)
{
   dump_samplers(ctx, p[1], 1);
   dump_samplers(ctx, p[2], 1);
   dump_samplers(ctx, p[3], 1);
}

static bool
str_ends_with(const char *str, const char *end)
{
   int offset = strlen(str) - strlen(end);
   if (offset < 0)
      return false;

   return strcmp(str + offset, end) == 0;
}

static void
decode_dynamic_state_pointers(struct aub_viewer_decode_ctx *ctx,
                              struct intel_group *inst, const uint32_t *p,
                              const char *struct_type,  int count)
{
   uint32_t state_offset = 0;

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (str_ends_with(iter.name, "Pointer")) {
         state_offset = iter.raw_value;
         break;
      }
   }

   uint64_t state_addr = ctx->dynamic_base + state_offset;
   struct intel_batch_decode_bo bo = ctx_get_bo(ctx, true, state_addr);
   const uint8_t *state_map = (const uint8_t *) bo.map;

   if (state_map == NULL) {
      ImGui::TextColored(ctx->cfg->missing_color,
                         "dynamic %s state unavailable addr=0x%012" PRIx64,
                         struct_type, state_addr);
      return;
   }

   struct intel_group *state = intel_spec_find_struct(ctx->spec, struct_type);
   if (strcmp(struct_type, "BLEND_STATE") == 0) {
      /* Blend states are different from the others because they have a header
       * struct called BLEND_STATE which is followed by a variable number of
       * BLEND_STATE_ENTRY structs.
       */
      ImGui::Text("%s", struct_type);
      aub_viewer_print_group(ctx, state, state_addr, state_map);

      state_addr += state->dw_length * 4;
      state_map += state->dw_length * 4;

      struct_type = "BLEND_STATE_ENTRY";
      state = intel_spec_find_struct(ctx->spec, struct_type);
   }

   for (int i = 0; i < count; i++) {
      ImGui::Text("%s %d", struct_type, i);
      aub_viewer_print_group(ctx, state, state_addr, state_map);

      state_addr += state->dw_length * 4;
      state_map += state->dw_length * 4;
   }
}

static void
decode_3dstate_viewport_state_pointers_cc(struct aub_viewer_decode_ctx *ctx,
                                          struct intel_group *inst,
                                          const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, inst, p, "CC_VIEWPORT", 4);
}

static void
decode_3dstate_viewport_state_pointers_sf_clip(struct aub_viewer_decode_ctx *ctx,
                                               struct intel_group *inst,
                                               const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, inst, p, "SF_CLIP_VIEWPORT", 4);
}

static void
decode_3dstate_blend_state_pointers(struct aub_viewer_decode_ctx *ctx,
                                    struct intel_group *inst,
                                    const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, inst, p, "BLEND_STATE", 1);
}

static void
decode_3dstate_cc_state_pointers(struct aub_viewer_decode_ctx *ctx,
                                 struct intel_group *inst,
                                 const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, inst, p, "COLOR_CALC_STATE", 1);
}

static void
decode_3dstate_scissor_state_pointers(struct aub_viewer_decode_ctx *ctx,
                                      struct intel_group *inst,
                                      const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, inst, p, "SCISSOR_RECT", 1);
}

static void
decode_load_register_imm(struct aub_viewer_decode_ctx *ctx,
                         struct intel_group *inst,
                         const uint32_t *p)
{
   struct intel_group *reg = intel_spec_find_register(ctx->spec, p[1]);

   if (reg != NULL &&
       ImGui::TreeNodeEx(&p[1], ImGuiTreeNodeFlags_Framed,
                         "%s (0x%x) = 0x%x",
                         reg->name, reg->register_offset, p[2])) {
      aub_viewer_print_group(ctx, reg, reg->register_offset, &p[2]);
      ImGui::TreePop();
   }
}

static void
decode_3dprimitive(struct aub_viewer_decode_ctx *ctx,
                   struct intel_group *inst,
                   const uint32_t *p)
{
   if (ctx->display_urb) {
      if (ImGui::Button("Show URB"))
         ctx->display_urb(ctx->user_data, ctx->urb_stages);
   }
}

static void
handle_urb(struct aub_viewer_decode_ctx *ctx,
           struct intel_group *inst,
           const uint32_t *p)
{
   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (strstr(iter.name, "URB Starting Address")) {
         ctx->urb_stages[ctx->stage].start = iter.raw_value * 8192;
      } else if (strstr(iter.name, "URB Entry Allocation Size")) {
         ctx->urb_stages[ctx->stage].size = (iter.raw_value + 1) * 64;
      } else if (strstr(iter.name, "Number of URB Entries")) {
         ctx->urb_stages[ctx->stage].n_entries = iter.raw_value;
      }
   }

   ctx->end_urb_offset = MAX2(ctx->urb_stages[ctx->stage].start +
                              ctx->urb_stages[ctx->stage].n_entries *
                              ctx->urb_stages[ctx->stage].size,
                              ctx->end_urb_offset);
}

static void
handle_urb_read(struct aub_viewer_decode_ctx *ctx,
                struct intel_group *inst,
                const uint32_t *p)
{
   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      /* Workaround the "Force * URB Entry Read Length" fields */
      if (iter.end_bit - iter.start_bit < 2)
         continue;

      if (strstr(iter.name, "URB Entry Read Offset")) {
         ctx->urb_stages[ctx->stage].rd_offset = iter.raw_value * 32;
      } else if (strstr(iter.name, "URB Entry Read Length")) {
         ctx->urb_stages[ctx->stage].rd_length = iter.raw_value * 32;
      } else if (strstr(iter.name, "URB Entry Output Read Offset")) {
         ctx->urb_stages[ctx->stage].wr_offset = iter.raw_value * 32;
      } else if (strstr(iter.name, "URB Entry Output Length")) {
         ctx->urb_stages[ctx->stage].wr_length = iter.raw_value * 32;
      }
   }
}

static void
handle_urb_constant(struct aub_viewer_decode_ctx *ctx,
                    struct intel_group *inst,
                    const uint32_t *p)
{
   struct intel_group *body =
      intel_spec_find_struct(ctx->spec, "3DSTATE_CONSTANT_BODY");

   struct intel_field_iterator outer;
   intel_field_iterator_init(&outer, inst, p, 0, false);
   while (intel_field_iterator_next(&outer)) {
      if (outer.struct_desc != body)
         continue;

      struct intel_field_iterator iter;
      intel_field_iterator_init(&iter, body, &outer.p[outer.start_bit / 32],
                                0, false);

      ctx->urb_stages[ctx->stage].const_rd_length = 0;
      while (intel_field_iterator_next(&iter)) {
         int idx;
         if (sscanf(iter.name, "Read Length[%d]", &idx) == 1) {
            ctx->urb_stages[ctx->stage].const_rd_length += iter.raw_value * 32;
         }
      }
   }
}

struct custom_decoder {
   const char *cmd_name;
   void (*decode)(struct aub_viewer_decode_ctx *ctx,
                  struct intel_group *inst,
                  const uint32_t *p);
   enum aub_decode_stage stage;
} display_decoders[] = {
   { "STATE_BASE_ADDRESS", handle_state_base_address },
   { "MEDIA_INTERFACE_DESCRIPTOR_LOAD", handle_media_interface_descriptor_load },
   { "3DSTATE_VERTEX_BUFFERS", handle_3dstate_vertex_buffers },
   { "3DSTATE_INDEX_BUFFER", handle_3dstate_index_buffer },
   { "3DSTATE_VS", decode_single_ksp, AUB_DECODE_STAGE_VS, },
   { "3DSTATE_GS", decode_single_ksp, AUB_DECODE_STAGE_GS, },
   { "3DSTATE_DS", decode_single_ksp, AUB_DECODE_STAGE_DS, },
   { "3DSTATE_HS", decode_single_ksp, AUB_DECODE_STAGE_HS, },
   { "3DSTATE_PS", decode_ps_kernels, AUB_DECODE_STAGE_PS, },
   { "3DSTATE_CONSTANT_VS", decode_3dstate_constant, AUB_DECODE_STAGE_VS, },
   { "3DSTATE_CONSTANT_GS", decode_3dstate_constant, AUB_DECODE_STAGE_GS, },
   { "3DSTATE_CONSTANT_DS", decode_3dstate_constant, AUB_DECODE_STAGE_DS, },
   { "3DSTATE_CONSTANT_HS", decode_3dstate_constant, AUB_DECODE_STAGE_HS, },
   { "3DSTATE_CONSTANT_PS", decode_3dstate_constant, AUB_DECODE_STAGE_PS, },

   { "3DSTATE_BINDING_TABLE_POINTERS_VS", decode_3dstate_binding_table_pointers, AUB_DECODE_STAGE_VS, },
   { "3DSTATE_BINDING_TABLE_POINTERS_GS", decode_3dstate_binding_table_pointers, AUB_DECODE_STAGE_GS, },
   { "3DSTATE_BINDING_TABLE_POINTERS_HS", decode_3dstate_binding_table_pointers, AUB_DECODE_STAGE_HS, },
   { "3DSTATE_BINDING_TABLE_POINTERS_DS", decode_3dstate_binding_table_pointers, AUB_DECODE_STAGE_DS, },
   { "3DSTATE_BINDING_TABLE_POINTERS_PS", decode_3dstate_binding_table_pointers, AUB_DECODE_STAGE_PS, },

   { "3DSTATE_SAMPLER_STATE_POINTERS_VS", decode_3dstate_sampler_state_pointers, AUB_DECODE_STAGE_VS, },
   { "3DSTATE_SAMPLER_STATE_POINTERS_GS", decode_3dstate_sampler_state_pointers, AUB_DECODE_STAGE_GS, },
   { "3DSTATE_SAMPLER_STATE_POINTERS_DS", decode_3dstate_sampler_state_pointers, AUB_DECODE_STAGE_DS, },
   { "3DSTATE_SAMPLER_STATE_POINTERS_HS", decode_3dstate_sampler_state_pointers, AUB_DECODE_STAGE_HS, },
   { "3DSTATE_SAMPLER_STATE_POINTERS_PS", decode_3dstate_sampler_state_pointers, AUB_DECODE_STAGE_PS, },
   { "3DSTATE_SAMPLER_STATE_POINTERS", decode_3dstate_sampler_state_pointers_gfx6 },

   { "3DSTATE_VIEWPORT_STATE_POINTERS_CC", decode_3dstate_viewport_state_pointers_cc },
   { "3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP", decode_3dstate_viewport_state_pointers_sf_clip },
   { "3DSTATE_BLEND_STATE_POINTERS", decode_3dstate_blend_state_pointers },
   { "3DSTATE_CC_STATE_POINTERS", decode_3dstate_cc_state_pointers },
   { "3DSTATE_SCISSOR_STATE_POINTERS", decode_3dstate_scissor_state_pointers },
   { "MI_LOAD_REGISTER_IMM", decode_load_register_imm },
   { "3DPRIMITIVE", decode_3dprimitive },
};

struct custom_decoder info_decoders[] = {
   { "STATE_BASE_ADDRESS", handle_state_base_address },
   { "3DSTATE_URB_VS", handle_urb, AUB_DECODE_STAGE_VS, },
   { "3DSTATE_URB_GS", handle_urb, AUB_DECODE_STAGE_GS, },
   { "3DSTATE_URB_DS", handle_urb, AUB_DECODE_STAGE_DS, },
   { "3DSTATE_URB_HS", handle_urb, AUB_DECODE_STAGE_HS, },
   { "3DSTATE_VS", handle_urb_read, AUB_DECODE_STAGE_VS, },
   { "3DSTATE_GS", handle_urb_read, AUB_DECODE_STAGE_GS, },
   { "3DSTATE_DS", handle_urb_read, AUB_DECODE_STAGE_DS, },
   { "3DSTATE_HS", handle_urb_read, AUB_DECODE_STAGE_HS, },
   { "3DSTATE_PS", handle_urb_read, AUB_DECODE_STAGE_PS, },
   { "3DSTATE_CONSTANT_VS", handle_urb_constant, AUB_DECODE_STAGE_VS, },
   { "3DSTATE_CONSTANT_GS", handle_urb_constant, AUB_DECODE_STAGE_GS, },
   { "3DSTATE_CONSTANT_DS", handle_urb_constant, AUB_DECODE_STAGE_DS, },
   { "3DSTATE_CONSTANT_HS", handle_urb_constant, AUB_DECODE_STAGE_HS, },
   { "3DSTATE_CONSTANT_PS", handle_urb_constant, AUB_DECODE_STAGE_PS, },
};

void
aub_viewer_render_batch(struct aub_viewer_decode_ctx *ctx,
                        const void *_batch, uint32_t batch_size,
                        uint64_t batch_addr, bool from_ring)
{
   struct intel_group *inst;
   const uint32_t *p, *batch = (const uint32_t *) _batch, *end = batch + batch_size / sizeof(uint32_t);
   int length;

   if (ctx->n_batch_buffer_start >= 100) {
      ImGui::TextColored(ctx->cfg->error_color,
                         "0x%08" PRIx64 ": Max batch buffer jumps exceeded", batch_addr);
      return;
   }

   ctx->n_batch_buffer_start++;

   for (p = batch; p < end; p += length) {
      inst = intel_spec_find_instruction(ctx->spec, ctx->engine, p);
      length = intel_group_get_length(inst, p);
      assert(inst == NULL || length > 0);
      length = MAX2(1, length);

      uint64_t offset = batch_addr + ((char *)p - (char *)batch);

      if (inst == NULL) {
         ImGui::TextColored(ctx->cfg->error_color,
                            "0x%012" PRIx64 ": unknown instruction %012x",
                            offset, p[0]);
         continue;
      }

      const char *inst_name = intel_group_get_name(inst);

      for (unsigned i = 0; i < ARRAY_SIZE(info_decoders); i++) {
         if (strcmp(inst_name, info_decoders[i].cmd_name) == 0) {
            ctx->stage = info_decoders[i].stage;
            info_decoders[i].decode(ctx, inst, p);
            break;
         }
      }

      if (ctx->decode_cfg->command_filter.PassFilter(inst->name) &&
          ImGui::TreeNodeEx(p,
                            ImGuiTreeNodeFlags_Framed,
                            "0x%012" PRIx64 ":  %s",
                            offset, inst->name)) {
         aub_viewer_print_group(ctx, inst, offset, p);

         for (unsigned i = 0; i < ARRAY_SIZE(display_decoders); i++) {
            if (strcmp(inst_name, display_decoders[i].cmd_name) == 0) {
               ctx->stage = display_decoders[i].stage;
               display_decoders[i].decode(ctx, inst, p);
               break;
            }
         }

         if (ctx->edit_address) {
            if (ImGui::Button("Edit instruction"))
               ctx->edit_address(ctx->user_data, offset, length * 4);
         }

         ImGui::TreePop();
      }

      if (strcmp(inst_name, "MI_BATCH_BUFFER_START") == 0) {
         uint64_t next_batch_addr = 0xd0d0d0d0;
         bool ppgtt = false;
         bool second_level = false;
         struct intel_field_iterator iter;
         intel_field_iterator_init(&iter, inst, p, 0, false);
         while (intel_field_iterator_next(&iter)) {
            if (strcmp(iter.name, "Batch Buffer Start Address") == 0) {
               next_batch_addr = iter.raw_value;
            } else if (strcmp(iter.name, "Second Level Batch Buffer") == 0) {
               second_level = iter.raw_value;
            } else if (strcmp(iter.name, "Address Space Indicator") == 0) {
               ppgtt = iter.raw_value;
            }
         }

         struct intel_batch_decode_bo next_batch = ctx_get_bo(ctx, ppgtt, next_batch_addr);

         if (next_batch.map == NULL) {
            ImGui::TextColored(ctx->cfg->missing_color,
                               "Secondary batch at 0x%012" PRIx64 " unavailable",
                               next_batch_addr);
         } else {
            aub_viewer_render_batch(ctx, next_batch.map, next_batch.size,
                                    next_batch.addr, false);
         }
         if (second_level) {
            /* MI_BATCH_BUFFER_START with "2nd Level Batch Buffer" set acts
             * like a subroutine call.  Commands that come afterwards get
             * processed once the 2nd level batch buffer returns with
             * MI_BATCH_BUFFER_END.
             */
            continue;
         } else if (!from_ring) {
            /* MI_BATCH_BUFFER_START with "2nd Level Batch Buffer" unset acts
             * like a goto.  Nothing after it will ever get processed.  In
             * order to prevent the recursion from growing, we just reset the
             * loop and continue;
             */
            break;
         }
      } else if (strcmp(inst_name, "MI_BATCH_BUFFER_END") == 0) {
         break;
      }
   }

   ctx->n_batch_buffer_start--;
}
