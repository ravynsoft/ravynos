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

#include "common/intel_decoder.h"
#include "intel_disasm.h"
#include "util/macros.h"
#include "util/u_debug.h"
#include "util/u_dynarray.h"
#include "util/u_math.h" /* Needed for ROUND_DOWN_TO */

#include <string.h>

static const struct debug_control debug_control[] = {
   { "color",      INTEL_BATCH_DECODE_IN_COLOR },
   { "full",       INTEL_BATCH_DECODE_FULL },
   { "offsets",    INTEL_BATCH_DECODE_OFFSETS },
   { "floats",     INTEL_BATCH_DECODE_FLOATS },
   { "surfaces",   INTEL_BATCH_DECODE_SURFACES },
   { "accumulate", INTEL_BATCH_DECODE_ACCUMULATE },
   { NULL,    0 }
};

void
intel_batch_decode_ctx_init(struct intel_batch_decode_ctx *ctx,
                            const struct brw_isa_info *isa,
                            const struct intel_device_info *devinfo,
                            FILE *fp, enum intel_batch_decode_flags flags,
                            const char *xml_path,
                            struct intel_batch_decode_bo (*get_bo)(void *,
                                                                   bool,
                                                                   uint64_t),
                            unsigned (*get_state_size)(void *, uint64_t,
                                                       uint64_t),
                            void *user_data)
{
   memset(ctx, 0, sizeof(*ctx));

   ctx->isa = isa;
   ctx->devinfo = *devinfo;
   ctx->get_bo = get_bo;
   ctx->get_state_size = get_state_size;
   ctx->user_data = user_data;
   ctx->fp = fp;
   ctx->flags = parse_enable_string(getenv("INTEL_DECODE"), flags, debug_control);
   ctx->max_vbo_decoded_lines = -1; /* No limit! */
   ctx->engine = INTEL_ENGINE_CLASS_RENDER;

   if (xml_path == NULL)
      ctx->spec = intel_spec_load(devinfo);
   else
      ctx->spec = intel_spec_load_from_path(devinfo, xml_path);

   ctx->commands =
      _mesa_hash_table_create(NULL, _mesa_hash_pointer, _mesa_key_pointer_equal);
   ctx->stats =
      _mesa_hash_table_create(NULL, _mesa_hash_string, _mesa_key_string_equal);
}

void
intel_batch_decode_ctx_finish(struct intel_batch_decode_ctx *ctx)
{
   _mesa_hash_table_destroy(ctx->commands, NULL);
   _mesa_hash_table_destroy(ctx->stats, NULL);
   intel_spec_destroy(ctx->spec);
}

#define CSI "\e["
#define RED_COLOR    CSI "31m"
#define BLUE_HEADER  CSI "0;44m" CSI "1;37m"
#define GREEN_HEADER CSI "1;42m"
#define NORMAL       CSI "0m"

static void
ctx_print_group(struct intel_batch_decode_ctx *ctx,
                struct intel_group *group,
                uint64_t address, const void *map)
{
   intel_print_group(ctx->fp, group, address, map, 0,
                   (ctx->flags & INTEL_BATCH_DECODE_IN_COLOR) != 0);
}

static struct intel_batch_decode_bo
ctx_get_bo(struct intel_batch_decode_ctx *ctx, bool ppgtt, uint64_t addr)
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
      bo.map += offset;
      bo.addr += offset;
      bo.size -= offset;
   }

   return bo;
}

static int
update_count(struct intel_batch_decode_ctx *ctx,
             uint64_t address,
             uint64_t base_address,
             unsigned element_dwords,
             unsigned guess)
{
   unsigned size = 0;

   if (ctx->get_state_size)
      size = ctx->get_state_size(ctx->user_data, address, base_address);

   if (size > 0)
      return size / (sizeof(uint32_t) * element_dwords);

   /* In the absence of any information, just guess arbitrarily. */
   return guess;
}

static void
ctx_disassemble_program(struct intel_batch_decode_ctx *ctx,
                        uint32_t ksp,
                        const char *short_name,
                        const char *name)
{
   uint64_t addr = ctx->instruction_base + ksp;
   struct intel_batch_decode_bo bo = ctx_get_bo(ctx, true, addr);
   if (!bo.map)
      return;

   fprintf(ctx->fp, "\nReferenced %s:\n", name);
   intel_disassemble(ctx->isa, bo.map, 0, ctx->fp);

   if (ctx->shader_binary) {
      int size = intel_disassemble_find_end(ctx->isa, bo.map, 0);

      ctx->shader_binary(ctx->user_data, short_name, addr,
                         bo.map, size);
   }
}

/* Heuristic to determine whether a uint32_t is probably actually a float
 * (http://stackoverflow.com/a/2953466)
 */

static bool
probably_float(uint32_t bits)
{
   int exp = ((bits & 0x7f800000U) >> 23) - 127;
   uint32_t mant = bits & 0x007fffff;

   /* +- 0.0 */
   if (exp == -127 && mant == 0)
      return true;

   /* +- 1 billionth to 1 billion */
   if (-30 <= exp && exp <= 30)
      return true;

   /* some value with only a few binary digits */
   if ((mant & 0x0000ffff) == 0)
      return true;

   return false;
}

static void
ctx_print_buffer(struct intel_batch_decode_ctx *ctx,
                 struct intel_batch_decode_bo bo,
                 uint32_t read_length,
                 uint32_t pitch,
                 int max_lines)
{
   const uint32_t *dw_end =
         bo.map + ROUND_DOWN_TO(MIN2(bo.size, read_length), 4);

   int column_count = 0, pitch_col_count = 0, line_count = -1;
   for (const uint32_t *dw = bo.map; dw < dw_end; dw++) {
      if (pitch_col_count * 4 == pitch || column_count == 8) {
         fprintf(ctx->fp, "\n");
         column_count = 0;
         if (pitch_col_count * 4 == pitch)
            pitch_col_count = 0;
         line_count++;

         if (max_lines >= 0 && line_count >= max_lines)
            break;
      }
      fprintf(ctx->fp, column_count == 0 ? "  " : " ");

      if ((ctx->flags & INTEL_BATCH_DECODE_FLOATS) && probably_float(*dw))
         fprintf(ctx->fp, "  %8.2f", *(float *) dw);
      else
         fprintf(ctx->fp, "  0x%08x", *dw);

      column_count++;
      pitch_col_count++;
   }
   fprintf(ctx->fp, "\n");
}

static struct intel_group *
intel_ctx_find_instruction(struct intel_batch_decode_ctx *ctx, const uint32_t *p)
{
   return intel_spec_find_instruction(ctx->spec, ctx->engine, p);
}

static void
handle_state_base_address(struct intel_batch_decode_ctx *ctx, const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);

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
handle_binding_table_pool_alloc(struct intel_batch_decode_ctx *ctx,
                                const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);

   uint64_t bt_pool_base = 0;
   bool bt_pool_enable = false;

   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "Binding Table Pool Base Address") == 0) {
         bt_pool_base = iter.raw_value;
      } else if (strcmp(iter.name, "Binding Table Pool Enable") == 0) {
         bt_pool_enable = iter.raw_value;
      }
   }

   if (bt_pool_enable || ctx->devinfo.verx10 >= 125) {
      ctx->bt_pool_base = bt_pool_base;
   } else {
      ctx->bt_pool_base = 0;
   }
}

static void
dump_binding_table(struct intel_batch_decode_ctx *ctx,
                   uint32_t offset, int count)
{
   struct intel_group *strct =
      intel_spec_find_struct(ctx->spec, "RENDER_SURFACE_STATE");
   if (strct == NULL) {
      fprintf(ctx->fp, "did not find RENDER_SURFACE_STATE info\n");
      return;
   }

   /* Most platforms use a 16-bit pointer with 32B alignment in bits 15:5. */
   uint32_t btp_alignment = 32;
   uint32_t btp_pointer_bits = 16;

   if (ctx->devinfo.verx10 >= 125) {
      /* The pointer is now 21-bit with 32B alignment in bits 20:5. */
      btp_pointer_bits = 21;
   } else if (ctx->use_256B_binding_tables) {
      /* When 256B binding tables are enabled, we have to shift the offset
       * which is stored in bits 15:5 but interpreted as bits 18:8 of the
       * actual offset.  The effective pointer is 19-bit with 256B alignment.
       */
      offset <<= 3;
      btp_pointer_bits = 19;
      btp_alignment = 256;
   }

   const uint64_t bt_pool_base = ctx->bt_pool_base ? ctx->bt_pool_base :
                                                     ctx->surface_base;

   if (count < 0) {
      count = update_count(ctx, bt_pool_base + offset,
                           bt_pool_base, 1, 32);
   }

   if (offset % btp_alignment != 0 || offset >= (1u << btp_pointer_bits)) {
      fprintf(ctx->fp, "  invalid binding table pointer\n");
      return;
   }

   struct intel_batch_decode_bo bind_bo =
      ctx_get_bo(ctx, true, bt_pool_base + offset);

   if (bind_bo.map == NULL) {
      fprintf(ctx->fp, "  binding table unavailable\n");
      return;
   }

   const uint32_t *pointers = bind_bo.map;
   for (int i = 0; i < count; i++) {
      if (((uintptr_t)&pointers[i] >= ((uintptr_t)bind_bo.map + bind_bo.size)) ||
          pointers[i] == 0)
         break;

      uint64_t addr = ctx->surface_base + pointers[i];
      struct intel_batch_decode_bo bo = ctx_get_bo(ctx, true, addr);
      uint32_t size = strct->dw_length * 4;

      if (pointers[i] % 32 != 0 ||
          addr < bo.addr || addr + size >= bo.addr + bo.size) {
         fprintf(ctx->fp, "pointer %u: 0x%08x <not valid>\n", i, pointers[i]);
         continue;
      }

      fprintf(ctx->fp, "pointer %u: 0x%08x\n", i, pointers[i]);
      if (ctx->flags & INTEL_BATCH_DECODE_SURFACES)
         ctx_print_group(ctx, strct, addr, bo.map + (addr - bo.addr));
   }
}

static void
dump_samplers(struct intel_batch_decode_ctx *ctx, uint32_t offset, int count)
{
   struct intel_group *strct = intel_spec_find_struct(ctx->spec, "SAMPLER_STATE");
   uint64_t state_addr = ctx->dynamic_base + offset;

   assert(count > 0);

   struct intel_batch_decode_bo bo = ctx_get_bo(ctx, true, state_addr);
   const void *state_map = bo.map;

   if (state_map == NULL) {
      fprintf(ctx->fp, "  samplers unavailable\n");
      return;
   }

   if (offset % 32 != 0) {
      fprintf(ctx->fp, "  invalid sampler state pointer\n");
      return;
   }

   const unsigned sampler_state_size = strct->dw_length * 4;

   if (count * sampler_state_size >= bo.size) {
      fprintf(ctx->fp, "  sampler state ends after bo ends\n");
      assert(!"sampler state ends after bo ends");
      return;
   }

   for (int i = 0; i < count; i++) {
      fprintf(ctx->fp, "sampler state %d\n", i);
      if (ctx->flags & INTEL_BATCH_DECODE_SAMPLERS)
         ctx_print_group(ctx, strct, state_addr, state_map);
      state_addr += sampler_state_size;
      state_map += sampler_state_size;
   }
}

static void
handle_interface_descriptor_data(struct intel_batch_decode_ctx *ctx,
                                 struct intel_group *desc, const uint32_t *p)
{
   uint64_t ksp = 0;
   uint32_t sampler_offset = 0, sampler_count = 0;
   uint32_t binding_table_offset = 0, binding_entry_count = 0;

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, desc, p, 0, false);
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

   ctx_disassemble_program(ctx, ksp, "CS", "compute shader");
   fprintf(ctx->fp, "\n");

   if (sampler_count)
      dump_samplers(ctx, sampler_offset, sampler_count);
   if (binding_entry_count)
      dump_binding_table(ctx, binding_table_offset, binding_entry_count);
}

static void
handle_media_interface_descriptor_load(struct intel_batch_decode_ctx *ctx,
                                       const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);
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
   const void *desc_map = bo.map;

   if (desc_map == NULL) {
      fprintf(ctx->fp, "  interface descriptors unavailable\n");
      return;
   }

   for (int i = 0; i < descriptor_count; i++) {
      fprintf(ctx->fp, "descriptor %d: %08x\n", i, descriptor_offset);

      ctx_print_group(ctx, desc, desc_addr, desc_map);

      handle_interface_descriptor_data(ctx, desc, desc_map);

      desc_map += desc->dw_length;
      desc_addr += desc->dw_length * 4;
   }
}

static void
handle_compute_walker(struct intel_batch_decode_ctx *ctx,
                      const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "Interface Descriptor") == 0) {
         handle_interface_descriptor_data(ctx, iter.struct_desc,
                                          &iter.p[iter.start_bit / 32]);
      }
   }
}

static void
handle_media_curbe_load(struct intel_batch_decode_ctx *ctx,
                        const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);

   uint32_t dynamic_state_offset = 0;
   uint32_t dynamic_state_length = 0;

   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "CURBE Data Start Address") == 0) {
         dynamic_state_offset = iter.raw_value;
      } else if (strcmp(iter.name, "CURBE Total Data Length") == 0) {
         dynamic_state_length = iter.raw_value;
      }
   }

   if (dynamic_state_length > 0) {
      struct intel_batch_decode_bo buffer =
         ctx_get_bo(ctx, true, ctx->dynamic_base + dynamic_state_offset);
      if (buffer.map != NULL)
         ctx_print_buffer(ctx, buffer, dynamic_state_length, 0, -1);
   }
}

static void
handle_3dstate_vertex_buffers(struct intel_batch_decode_ctx *ctx,
                              const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);
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

      struct intel_field_iterator vbs_iter;
      intel_field_iterator_init(&vbs_iter, vbs, &iter.p[iter.start_bit / 32], 0, false);
      while (intel_field_iterator_next(&vbs_iter)) {
         if (strcmp(vbs_iter.name, "Vertex Buffer Index") == 0) {
            index = vbs_iter.raw_value;
         } else if (strcmp(vbs_iter.name, "Buffer Pitch") == 0) {
            pitch = vbs_iter.raw_value;
         } else if (strcmp(vbs_iter.name, "Buffer Starting Address") == 0) {
            vb = ctx_get_bo(ctx, true, vbs_iter.raw_value);
         } else if (strcmp(vbs_iter.name, "Buffer Size") == 0) {
            vb_size = vbs_iter.raw_value;
            ready = true;
         } else if (strcmp(vbs_iter.name, "End Address") == 0) {
            if (vb.map && vbs_iter.raw_value >= vb.addr)
               vb_size = (vbs_iter.raw_value + 1) - vb.addr;
            else
               vb_size = 0;
            ready = true;
         }

         if (!ready)
            continue;

         fprintf(ctx->fp, "vertex buffer %d, size %d\n", index, vb_size);

         if (vb.map == NULL) {
            fprintf(ctx->fp, "  buffer contents unavailable\n");
            continue;
         }

         if (vb.map == 0 || vb_size == 0)
            continue;

         ctx_print_buffer(ctx, vb, vb_size, pitch, ctx->max_vbo_decoded_lines);

         vb.map = NULL;
         vb_size = 0;
         index = -1;
         pitch = -1;
         ready = false;
      }
   }
}

static void
handle_3dstate_index_buffer(struct intel_batch_decode_ctx *ctx,
                            const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);

   struct intel_batch_decode_bo ib = {};
   uint32_t ib_size = 0;
   uint32_t format = 0;

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "Index Format") == 0) {
         format = iter.raw_value;
      } else if (strcmp(iter.name, "Buffer Starting Address") == 0) {
         ib = ctx_get_bo(ctx, true, iter.raw_value);
      } else if (strcmp(iter.name, "Buffer Size") == 0) {
         ib_size = iter.raw_value;
      }
   }

   if (ib.map == NULL) {
      fprintf(ctx->fp, "  buffer contents unavailable\n");
      return;
   }

   const void *m = ib.map;
   const void *ib_end = ib.map + MIN2(ib.size, ib_size);
   for (int i = 0; m < ib_end && i < 10; i++) {
      switch (format) {
      case 0:
         fprintf(ctx->fp, "%3d ", *(uint8_t *)m);
         m += 1;
         break;
      case 1:
         fprintf(ctx->fp, "%3d ", *(uint16_t *)m);
         m += 2;
         break;
      case 2:
         fprintf(ctx->fp, "%3d ", *(uint32_t *)m);
         m += 4;
         break;
      }
   }

   if (m < ib_end)
      fprintf(ctx->fp, "...");
   fprintf(ctx->fp, "\n");
}

static void
decode_single_ksp(struct intel_batch_decode_ctx *ctx, const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);

   uint64_t ksp = 0;
   bool is_simd8 = ctx->devinfo.ver >= 11; /* vertex shaders on Gfx8+ only */
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
   const char *short_name =
      strcmp(inst->name,   "VS_STATE") == 0 ? "VS" :
      strcmp(inst->name,   "GS_STATE") == 0 ? "GS" :
      strcmp(inst->name,   "SF_STATE") == 0 ? "SF" :
      strcmp(inst->name, "CLIP_STATE") == 0 ? "CL" :
      strcmp(inst->name, "3DSTATE_DS") == 0 ? "DS" :
      strcmp(inst->name, "3DSTATE_HS") == 0 ? "HS" :
      strcmp(inst->name, "3DSTATE_VS") == 0 ? "VS" :
      strcmp(inst->name, "3DSTATE_GS") == 0 ? "GS" :
      NULL;

   if (is_enabled) {
      ctx_disassemble_program(ctx, ksp, short_name, type);
      fprintf(ctx->fp, "\n");
   }
}

static void
decode_mesh_task_ksp(struct intel_batch_decode_ctx *ctx, const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);

   uint64_t ksp = 0;
   uint64_t local_x_maximum = 0;
   uint64_t threads = 0;

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "Kernel Start Pointer") == 0) {
         ksp = iter.raw_value;
      } else if (strcmp(iter.name, "Local X Maximum") == 0) {
         local_x_maximum = iter.raw_value;
      } else if (strcmp(iter.name, "Number of Threads in GPGPU Thread Group") == 0) {
         threads = iter.raw_value;
      }
   }

   const char *type =
      strcmp(inst->name,   "3DSTATE_MESH_SHADER") == 0 ? "mesh shader" :
      strcmp(inst->name,   "3DSTATE_TASK_SHADER") == 0 ? "task shader" :
      NULL;
   const char *short_name =
      strcmp(inst->name,   "3DSTATE_MESH_SHADER") == 0 ? "MS" :
      strcmp(inst->name,   "3DSTATE_TASK_SHADER") == 0 ? "TS" :
      NULL;

   if (threads && local_x_maximum) {
      ctx_disassemble_program(ctx, ksp, short_name, type);
      fprintf(ctx->fp, "\n");
   }
}

static void
decode_ps_kern(struct intel_batch_decode_ctx *ctx,
               struct intel_group *inst, const uint32_t *p)
{
   bool single_ksp = ctx->devinfo.ver == 4;
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

   if (single_ksp)
      ksp[1] = ksp[2] = ksp[0];

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
      ctx_disassemble_program(ctx, ksp[0], "FS8", "SIMD8 fragment shader");
   if (enabled[1])
      ctx_disassemble_program(ctx, ksp[1], "FS16", "SIMD16 fragment shader");
   if (enabled[2])
      ctx_disassemble_program(ctx, ksp[2], "FS32", "SIMD32 fragment shader");

   if (enabled[0] || enabled[1] || enabled[2])
      fprintf(ctx->fp, "\n");
}

static void
decode_ps_kern_xe2(struct intel_batch_decode_ctx *ctx,
                     struct intel_group *inst, const uint32_t *p)
{
   uint64_t ksp[2] = {0, 0};
   bool enabled[2] = {false, false};
   int width[2] = {0, 0};

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (strncmp(iter.name, "Kernel Start Pointer ",
                  strlen("Kernel Start Pointer ")) == 0) {
         int idx = iter.name[strlen("Kernel Start Pointer ")] - '0';
         ksp[idx] = strtol(iter.value, NULL, 16);
      } else if (strcmp(iter.name, "Kernel 0 Enable") == 0) {
         enabled[0] = strcmp(iter.value, "true") == 0;
      } else if (strcmp(iter.name, "Kernel 1 Enable") == 0) {
         enabled[1] = strcmp(iter.value, "true") == 0;
      } else if (strcmp(iter.name, "Kernel[0] : SIMD Width") == 0) {
         width[0] = strncmp(iter.value, "0 ", 2) == 0 ? 16 : 32;
      } else if (strcmp(iter.name, "Kernel[1] : SIMD Width") == 0) {
         width[1] = strncmp(iter.value, "0 ", 2) == 0 ? 16 : 32;
      }
   }

   for (int i = 0; i < 2; i++) {
      if (enabled[i])
         ctx_disassemble_program(ctx, ksp[i], "FS",
                                 width[i] == 16 ?
                                 "SIMD16 fragment shader" :
                                 "SIMD32 fragment shader");
   }

   if (enabled[0] || enabled[1])
      fprintf(ctx->fp, "\n");
}

static void
decode_ps_kernels(struct intel_batch_decode_ctx *ctx,
                  const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);
   if (ctx->devinfo.ver >= 20)
      decode_ps_kern_xe2(ctx, inst, p);
   else
      decode_ps_kern(ctx, inst, p);
}

static void
decode_3dstate_constant_all(struct intel_batch_decode_ctx *ctx, const uint32_t *p)
{
   struct intel_group *inst =
      intel_spec_find_instruction(ctx->spec, ctx->engine, p);
   struct intel_group *body =
      intel_spec_find_struct(ctx->spec, "3DSTATE_CONSTANT_ALL_DATA");

   uint32_t read_length[4] = {0};
   struct intel_batch_decode_bo buffer[4];
   memset(buffer, 0, sizeof(buffer));

   struct intel_field_iterator outer;
   intel_field_iterator_init(&outer, inst, p, 0, false);
   int idx = 0;
   while (intel_field_iterator_next(&outer)) {
      if (outer.struct_desc != body)
         continue;

      struct intel_field_iterator iter;
      intel_field_iterator_init(&iter, body, &outer.p[outer.start_bit / 32],
                              0, false);
      while (intel_field_iterator_next(&iter)) {
         if (!strcmp(iter.name, "Pointer To Constant Buffer")) {
            buffer[idx] = ctx_get_bo(ctx, true, iter.raw_value);
         } else if (!strcmp(iter.name, "Constant Buffer Read Length")) {
            read_length[idx] = iter.raw_value;
         }
      }
      idx++;
   }

   for (int i = 0; i < 4; i++) {
      if (read_length[i] == 0 || buffer[i].map == NULL)
         continue;

      unsigned size = read_length[i] * 32;
      fprintf(ctx->fp, "constant buffer %d, size %u\n", i, size);

      ctx_print_buffer(ctx, buffer[i], size, 0, -1);
   }
}

static void
decode_3dstate_constant(struct intel_batch_decode_ctx *ctx, const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);
   struct intel_group *body =
      intel_spec_find_struct(ctx->spec, "3DSTATE_CONSTANT_BODY");

   uint32_t read_length[4] = {0};
   uint64_t read_addr[4] = {0};

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
            fprintf(ctx->fp, "constant buffer %d unavailable\n", i);
            continue;
         }

         unsigned size = read_length[i] * 32;
         fprintf(ctx->fp, "constant buffer %d, size %u\n", i, size);

         ctx_print_buffer(ctx, buffer, size, 0, -1);
      }
   }
}

static void
decode_gfx4_constant_buffer(struct intel_batch_decode_ctx *ctx, const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);
   uint64_t read_length = 0, read_addr = 0, valid = 0;
   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);

   while (intel_field_iterator_next(&iter)) {
      if (!strcmp(iter.name, "Buffer Length")) {
         read_length = iter.raw_value;
      } else if (!strcmp(iter.name, "Valid")) {
         valid = iter.raw_value;
      } else if (!strcmp(iter.name, "Buffer Starting Address")) {
         read_addr = iter.raw_value;
      }
   }

   if (!valid)
      return;

   struct intel_batch_decode_bo buffer = ctx_get_bo(ctx, true, read_addr);
   if (!buffer.map) {
      fprintf(ctx->fp, "constant buffer unavailable\n");
      return;
   }
   unsigned size = (read_length + 1) * 16 * sizeof(float);
   fprintf(ctx->fp, "constant buffer size %u\n", size);

   ctx_print_buffer(ctx, buffer, size, 0, -1);
}


static void
decode_gfx4_3dstate_binding_table_pointers(struct intel_batch_decode_ctx *ctx,
                                           const uint32_t *p)
{
   fprintf(ctx->fp, "VS Binding Table:\n");
   dump_binding_table(ctx, p[1], -1);

   fprintf(ctx->fp, "GS Binding Table:\n");
   dump_binding_table(ctx, p[2], -1);

   if (ctx->devinfo.ver < 6) {
      fprintf(ctx->fp, "CLIP Binding Table:\n");
      dump_binding_table(ctx, p[3], -1);
      fprintf(ctx->fp, "SF Binding Table:\n");
      dump_binding_table(ctx, p[4], -1);
      fprintf(ctx->fp, "PS Binding Table:\n");
      dump_binding_table(ctx, p[5], -1);
   } else {
      fprintf(ctx->fp, "PS Binding Table:\n");
      dump_binding_table(ctx, p[3], -1);
   }
}

static void
decode_3dstate_binding_table_pointers(struct intel_batch_decode_ctx *ctx,
                                      const uint32_t *p)
{
   dump_binding_table(ctx, p[1], -1);
}

static void
decode_3dstate_sampler_state_pointers(struct intel_batch_decode_ctx *ctx,
                                      const uint32_t *p)
{
   dump_samplers(ctx, p[1], 1);
}

static void
decode_3dstate_sampler_state_pointers_gfx6(struct intel_batch_decode_ctx *ctx,
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
decode_dynamic_state(struct intel_batch_decode_ctx *ctx,
                       const char *struct_type, uint32_t state_offset,
                       int count)
{
   uint64_t state_addr = ctx->dynamic_base + state_offset;
   struct intel_batch_decode_bo bo = ctx_get_bo(ctx, true, state_addr);
   const void *state_map = bo.map;

   if (state_map == NULL) {
      fprintf(ctx->fp, "  dynamic %s state unavailable\n", struct_type);
      return;
   }

   struct intel_group *state = intel_spec_find_struct(ctx->spec, struct_type);
   if (strcmp(struct_type, "BLEND_STATE") == 0) {
      /* Blend states are different from the others because they have a header
       * struct called BLEND_STATE which is followed by a variable number of
       * BLEND_STATE_ENTRY structs.
       */
      fprintf(ctx->fp, "%s\n", struct_type);
      ctx_print_group(ctx, state, state_addr, state_map);

      state_addr += state->dw_length * 4;
      state_map += state->dw_length * 4;

      struct_type = "BLEND_STATE_ENTRY";
      state = intel_spec_find_struct(ctx->spec, struct_type);
   }

   count = update_count(ctx, ctx->dynamic_base + state_offset,
                        ctx->dynamic_base, state->dw_length, count);

   for (int i = 0; i < count; i++) {
      fprintf(ctx->fp, "%s %d\n", struct_type, i);
      ctx_print_group(ctx, state, state_addr, state_map);

      state_addr += state->dw_length * 4;
      state_map += state->dw_length * 4;
   }
}

static void
decode_dynamic_state_pointers(struct intel_batch_decode_ctx *ctx,
                              const char *struct_type, const uint32_t *p,
                              int count)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);

   uint32_t state_offset = 0;

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (str_ends_with(iter.name, "Pointer") || !strncmp(iter.name, "Pointer", 7)) {
         state_offset = iter.raw_value;
         break;
      }
   }
   decode_dynamic_state(ctx, struct_type, state_offset, count);
}

static void
decode_3dstate_viewport_state_pointers(struct intel_batch_decode_ctx *ctx,
                                       const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);
   uint32_t state_offset = 0;
   bool clip = false, sf = false, cc = false;
   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (!strcmp(iter.name, "CLIP Viewport State Change"))
         clip = iter.raw_value;
      if (!strcmp(iter.name, "SF Viewport State Change"))
         sf = iter.raw_value;
      if (!strcmp(iter.name, "CC Viewport State Change"))
         cc = iter.raw_value;
      else if (!strcmp(iter.name, "Pointer to CLIP_VIEWPORT") && clip) {
         state_offset = iter.raw_value;
         decode_dynamic_state(ctx, "CLIP_VIEWPORT", state_offset, 1);
      }
      else if (!strcmp(iter.name, "Pointer to SF_VIEWPORT") && sf) {
         state_offset = iter.raw_value;
         decode_dynamic_state(ctx, "SF_VIEWPORT", state_offset, 1);
      }
      else if (!strcmp(iter.name, "Pointer to CC_VIEWPORT") && cc) {
         state_offset = iter.raw_value;
         decode_dynamic_state(ctx, "CC_VIEWPORT", state_offset, 1);
      }
   }
}

static void
decode_3dstate_viewport_state_pointers_cc(struct intel_batch_decode_ctx *ctx,
                                          const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, "CC_VIEWPORT", p, 4);
}

static void
decode_3dstate_viewport_state_pointers_sf_clip(struct intel_batch_decode_ctx *ctx,
                                               const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, "SF_CLIP_VIEWPORT", p, 4);
}

static void
decode_3dstate_blend_state_pointers(struct intel_batch_decode_ctx *ctx,
                                    const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, "BLEND_STATE", p, 1);
}

static void
decode_3dstate_cc_state_pointers(struct intel_batch_decode_ctx *ctx,
                                 const uint32_t *p)
{
   if (ctx->devinfo.ver != 6) {
      decode_dynamic_state_pointers(ctx, "COLOR_CALC_STATE", p, 1);
      return;
   }

   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);

   uint32_t state_offset = 0;
   bool blend_change = false, ds_change = false, cc_change = false;
   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, inst, p, 0, false);
   while (intel_field_iterator_next(&iter)) {
      if (!strcmp(iter.name, "BLEND_STATE Change"))
         blend_change = iter.raw_value;
      else if (!strcmp(iter.name, "DEPTH_STENCIL_STATE Change"))
         ds_change = iter.raw_value;
      else if (!strcmp(iter.name, "Color Calc State Pointer Valid"))
         cc_change = iter.raw_value;
      else if (!strcmp(iter.name, "Pointer to DEPTH_STENCIL_STATE") && ds_change) {
         state_offset = iter.raw_value;
         decode_dynamic_state(ctx, "DEPTH_STENCIL_STATE", state_offset, 1);
      }
      else if (!strcmp(iter.name, "Pointer to BLEND_STATE") && blend_change) {
         state_offset = iter.raw_value;
         decode_dynamic_state(ctx, "BLEND_STATE", state_offset, 1);
      }
      else if (!strcmp(iter.name, "Color Calc State Pointer") && cc_change) {
         state_offset = iter.raw_value;
         decode_dynamic_state(ctx, "COLOR_CALC_STATE", state_offset, 1);
      }
   }
}

static void
decode_3dstate_ds_state_pointers(struct intel_batch_decode_ctx *ctx,
                                 const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, "DEPTH_STENCIL_STATE", p, 1);
}

static void
decode_3dstate_scissor_state_pointers(struct intel_batch_decode_ctx *ctx,
                                      const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, "SCISSOR_RECT", p, 1);
}

static void
decode_3dstate_slice_table_state_pointers(struct intel_batch_decode_ctx *ctx,
                                          const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, "SLICE_HASH_TABLE", p, 1);
}

static void
handle_gt_mode(struct intel_batch_decode_ctx *ctx,
               uint32_t reg_addr, uint32_t val)
{
   struct intel_group *reg = intel_spec_find_register(ctx->spec, reg_addr);

   assert(intel_group_get_length(reg, &val) == 1);

   struct intel_field_iterator iter;
   intel_field_iterator_init(&iter, reg, &val, 0, false);

   uint32_t bt_alignment;
   bool bt_alignment_mask = 0;

   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "Binding Table Alignment") == 0) {
         bt_alignment = iter.raw_value;
      } else if (strcmp(iter.name, "Binding Table Alignment Mask") == 0) {
         bt_alignment_mask = iter.raw_value;
      }
   }

   if (bt_alignment_mask)
      ctx->use_256B_binding_tables = bt_alignment;
}

struct reg_handler {
   const char *name;
   void (*handler)(struct intel_batch_decode_ctx *ctx,
                   uint32_t reg_addr, uint32_t val);
} reg_handlers[] = {
   { "GT_MODE", handle_gt_mode }
};

static void
decode_load_register_imm(struct intel_batch_decode_ctx *ctx, const uint32_t *p)
{
   struct intel_group *inst = intel_ctx_find_instruction(ctx, p);
   const unsigned length = intel_group_get_length(inst, p);
   assert(length & 1);
   const unsigned nr_regs = (length - 1) / 2;

   for (unsigned i = 0; i < nr_regs; i++) {
      struct intel_group *reg = intel_spec_find_register(ctx->spec, p[i * 2 + 1]);
      if (reg != NULL) {
         fprintf(ctx->fp, "register %s (0x%x): 0x%x\n",
                 reg->name, reg->register_offset, p[2]);
         ctx_print_group(ctx, reg, reg->register_offset, &p[2]);

         for (unsigned i = 0; i < ARRAY_SIZE(reg_handlers); i++) {
            if (strcmp(reg->name, reg_handlers[i].name) == 0)
               reg_handlers[i].handler(ctx, p[1], p[2]);
         }
      }
   }
}

static void
disasm_program_from_group(struct intel_batch_decode_ctx *ctx,
                          struct intel_group *strct, const void *map,
                          const char *short_name, const char *type)
{
   uint64_t ksp = 0;
   bool is_enabled = true;
   struct intel_field_iterator iter;

   intel_field_iterator_init(&iter, strct, map, 0, false);

   while (intel_field_iterator_next(&iter)) {
      if (strcmp(iter.name, "Kernel Start Pointer") == 0) {
         ksp = iter.raw_value;
      } else if (strcmp(iter.name, "Enable") == 0) {
         is_enabled = iter.raw_value;
      }
   }

   if (is_enabled) {
      ctx_disassemble_program(ctx, ksp, short_name, type);
      fprintf(ctx->fp, "\n");
   }
}

static void
decode_vs_state(struct intel_batch_decode_ctx *ctx, uint32_t offset)
{
   struct intel_group *strct =
      intel_spec_find_struct(ctx->spec, "VS_STATE");
   if (strct == NULL) {
      fprintf(ctx->fp, "did not find VS_STATE info\n");
      return;
   }

   struct intel_batch_decode_bo bind_bo =
      ctx_get_bo(ctx, true, offset);

   if (bind_bo.map == NULL) {
      fprintf(ctx->fp, " vs state unavailable\n");
      return;
   }

   ctx_print_group(ctx, strct, offset, bind_bo.map);
   disasm_program_from_group(ctx, strct, bind_bo.map, "VS", "vertex shader");
}

static void
decode_gs_state(struct intel_batch_decode_ctx *ctx, uint32_t offset)
{
   struct intel_group *strct =
      intel_spec_find_struct(ctx->spec, "GS_STATE");
   if (strct == NULL) {
      fprintf(ctx->fp, "did not find GS_STATE info\n");
      return;
   }

   struct intel_batch_decode_bo bind_bo =
      ctx_get_bo(ctx, true, offset);

   if (bind_bo.map == NULL) {
      fprintf(ctx->fp, " gs state unavailable\n");
      return;
   }

   ctx_print_group(ctx, strct, offset, bind_bo.map);
   disasm_program_from_group(ctx, strct, bind_bo.map, "GS", "geometry shader");
}

static void
decode_clip_state(struct intel_batch_decode_ctx *ctx, uint32_t offset)
{
   struct intel_group *strct =
      intel_spec_find_struct(ctx->spec, "CLIP_STATE");
   if (strct == NULL) {
      fprintf(ctx->fp, "did not find CLIP_STATE info\n");
      return;
   }

   struct intel_batch_decode_bo bind_bo =
      ctx_get_bo(ctx, true, offset);

   if (bind_bo.map == NULL) {
      fprintf(ctx->fp, " clip state unavailable\n");
      return;
   }

   ctx_print_group(ctx, strct, offset, bind_bo.map);
   disasm_program_from_group(ctx, strct, bind_bo.map, "CL", "clip shader");

   struct intel_group *vp_strct =
      intel_spec_find_struct(ctx->spec, "CLIP_VIEWPORT");
   if (vp_strct == NULL) {
      fprintf(ctx->fp, "did not find CLIP_VIEWPORT info\n");
      return;
   }
   uint32_t clip_vp_offset = ((uint32_t *)bind_bo.map)[6] & ~0x3;
   struct intel_batch_decode_bo vp_bo =
      ctx_get_bo(ctx, true, clip_vp_offset);
   if (vp_bo.map == NULL) {
      fprintf(ctx->fp, " clip vp state unavailable\n");
      return;
   }
   ctx_print_group(ctx, vp_strct, clip_vp_offset, vp_bo.map);
}

static void
decode_sf_state(struct intel_batch_decode_ctx *ctx, uint32_t offset)
{
   struct intel_group *strct =
      intel_spec_find_struct(ctx->spec, "SF_STATE");
   if (strct == NULL) {
      fprintf(ctx->fp, "did not find SF_STATE info\n");
      return;
   }

   struct intel_batch_decode_bo bind_bo =
      ctx_get_bo(ctx, true, offset);

   if (bind_bo.map == NULL) {
      fprintf(ctx->fp, " sf state unavailable\n");
      return;
   }

   ctx_print_group(ctx, strct, offset, bind_bo.map);
   disasm_program_from_group(ctx, strct, bind_bo.map, "SF", "strips and fans shader");

   struct intel_group *vp_strct =
      intel_spec_find_struct(ctx->spec, "SF_VIEWPORT");
   if (vp_strct == NULL) {
      fprintf(ctx->fp, "did not find SF_VIEWPORT info\n");
      return;
   }

   uint32_t sf_vp_offset = ((uint32_t *)bind_bo.map)[5] & ~0x3;
   struct intel_batch_decode_bo vp_bo =
      ctx_get_bo(ctx, true, sf_vp_offset);
   if (vp_bo.map == NULL) {
      fprintf(ctx->fp, " sf vp state unavailable\n");
      return;
   }
   ctx_print_group(ctx, vp_strct, sf_vp_offset, vp_bo.map);
}

static void
decode_wm_state(struct intel_batch_decode_ctx *ctx, uint32_t offset)
{
   struct intel_group *strct =
      intel_spec_find_struct(ctx->spec, "WM_STATE");
   if (strct == NULL) {
      fprintf(ctx->fp, "did not find WM_STATE info\n");
      return;
   }

   struct intel_batch_decode_bo bind_bo =
      ctx_get_bo(ctx, true, offset);

   if (bind_bo.map == NULL) {
      fprintf(ctx->fp, " wm state unavailable\n");
      return;
   }

   ctx_print_group(ctx, strct, offset, bind_bo.map);

   decode_ps_kern(ctx, strct, bind_bo.map);
}

static void
decode_cc_state(struct intel_batch_decode_ctx *ctx, uint32_t offset)
{
   struct intel_group *strct =
      intel_spec_find_struct(ctx->spec, "COLOR_CALC_STATE");
   if (strct == NULL) {
      fprintf(ctx->fp, "did not find COLOR_CALC_STATE info\n");
      return;
   }

   struct intel_batch_decode_bo bind_bo =
      ctx_get_bo(ctx, true, offset);

   if (bind_bo.map == NULL) {
      fprintf(ctx->fp, " cc state unavailable\n");
      return;
   }

   ctx_print_group(ctx, strct, offset, bind_bo.map);

   struct intel_group *vp_strct =
      intel_spec_find_struct(ctx->spec, "CC_VIEWPORT");
   if (vp_strct == NULL) {
      fprintf(ctx->fp, "did not find CC_VIEWPORT info\n");
      return;
   }
   uint32_t cc_vp_offset = ((uint32_t *)bind_bo.map)[4] & ~0x3;
   struct intel_batch_decode_bo vp_bo =
      ctx_get_bo(ctx, true, cc_vp_offset);
   if (vp_bo.map == NULL) {
      fprintf(ctx->fp, " cc vp state unavailable\n");
      return;
   }
   ctx_print_group(ctx, vp_strct, cc_vp_offset, vp_bo.map);
}
static void
decode_pipelined_pointers(struct intel_batch_decode_ctx *ctx, const uint32_t *p)
{
   fprintf(ctx->fp, "VS State Table:\n");
   decode_vs_state(ctx, p[1]);
   if (p[2] & 1) {
      fprintf(ctx->fp, "GS State Table:\n");
      decode_gs_state(ctx, p[2] & ~1);
   }
   fprintf(ctx->fp, "Clip State Table:\n");
   decode_clip_state(ctx, p[3] & ~1);
   fprintf(ctx->fp, "SF State Table:\n");
   decode_sf_state(ctx, p[4]);
   fprintf(ctx->fp, "WM State Table:\n");
   decode_wm_state(ctx, p[5]);
   fprintf(ctx->fp, "CC State Table:\n");
   decode_cc_state(ctx, p[6]);
}

static void
decode_cps_pointers(struct intel_batch_decode_ctx *ctx, const uint32_t *p)
{
   decode_dynamic_state_pointers(ctx, "CPS_STATE", p, 1);
}

struct custom_decoder {
   const char *cmd_name;
   void (*decode)(struct intel_batch_decode_ctx *ctx, const uint32_t *p);
} custom_decoders[] = {
   { "STATE_BASE_ADDRESS", handle_state_base_address },
   { "3DSTATE_BINDING_TABLE_POOL_ALLOC", handle_binding_table_pool_alloc },
   { "MEDIA_INTERFACE_DESCRIPTOR_LOAD", handle_media_interface_descriptor_load },
   { "COMPUTE_WALKER", handle_compute_walker },
   { "MEDIA_CURBE_LOAD", handle_media_curbe_load },
   { "3DSTATE_VERTEX_BUFFERS", handle_3dstate_vertex_buffers },
   { "3DSTATE_INDEX_BUFFER", handle_3dstate_index_buffer },
   { "3DSTATE_VS", decode_single_ksp },
   { "3DSTATE_GS", decode_single_ksp },
   { "3DSTATE_DS", decode_single_ksp },
   { "3DSTATE_HS", decode_single_ksp },
   { "3DSTATE_PS", decode_ps_kernels },
   { "3DSTATE_WM", decode_ps_kernels },
   { "3DSTATE_MESH_SHADER", decode_mesh_task_ksp },
   { "3DSTATE_TASK_SHADER", decode_mesh_task_ksp },
   { "3DSTATE_CONSTANT_VS", decode_3dstate_constant },
   { "3DSTATE_CONSTANT_GS", decode_3dstate_constant },
   { "3DSTATE_CONSTANT_PS", decode_3dstate_constant },
   { "3DSTATE_CONSTANT_HS", decode_3dstate_constant },
   { "3DSTATE_CONSTANT_DS", decode_3dstate_constant },
   { "3DSTATE_CONSTANT_ALL", decode_3dstate_constant_all },

   { "3DSTATE_BINDING_TABLE_POINTERS", decode_gfx4_3dstate_binding_table_pointers },
   { "3DSTATE_BINDING_TABLE_POINTERS_VS", decode_3dstate_binding_table_pointers },
   { "3DSTATE_BINDING_TABLE_POINTERS_HS", decode_3dstate_binding_table_pointers },
   { "3DSTATE_BINDING_TABLE_POINTERS_DS", decode_3dstate_binding_table_pointers },
   { "3DSTATE_BINDING_TABLE_POINTERS_GS", decode_3dstate_binding_table_pointers },
   { "3DSTATE_BINDING_TABLE_POINTERS_PS", decode_3dstate_binding_table_pointers },

   { "3DSTATE_SAMPLER_STATE_POINTERS_VS", decode_3dstate_sampler_state_pointers },
   { "3DSTATE_SAMPLER_STATE_POINTERS_HS", decode_3dstate_sampler_state_pointers },
   { "3DSTATE_SAMPLER_STATE_POINTERS_DS", decode_3dstate_sampler_state_pointers },
   { "3DSTATE_SAMPLER_STATE_POINTERS_GS", decode_3dstate_sampler_state_pointers },
   { "3DSTATE_SAMPLER_STATE_POINTERS_PS", decode_3dstate_sampler_state_pointers },
   { "3DSTATE_SAMPLER_STATE_POINTERS", decode_3dstate_sampler_state_pointers_gfx6 },

   { "3DSTATE_VIEWPORT_STATE_POINTERS", decode_3dstate_viewport_state_pointers },
   { "3DSTATE_VIEWPORT_STATE_POINTERS_CC", decode_3dstate_viewport_state_pointers_cc },
   { "3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP", decode_3dstate_viewport_state_pointers_sf_clip },
   { "3DSTATE_BLEND_STATE_POINTERS", decode_3dstate_blend_state_pointers },
   { "3DSTATE_CC_STATE_POINTERS", decode_3dstate_cc_state_pointers },
   { "3DSTATE_DEPTH_STENCIL_STATE_POINTERS", decode_3dstate_ds_state_pointers },
   { "3DSTATE_SCISSOR_STATE_POINTERS", decode_3dstate_scissor_state_pointers },
   { "3DSTATE_SLICE_TABLE_STATE_POINTERS", decode_3dstate_slice_table_state_pointers },
   { "MI_LOAD_REGISTER_IMM", decode_load_register_imm },
   { "3DSTATE_PIPELINED_POINTERS", decode_pipelined_pointers },
   { "3DSTATE_CPS_POINTERS", decode_cps_pointers },
   { "CONSTANT_BUFFER", decode_gfx4_constant_buffer },
};

static void
get_inst_color(const struct intel_batch_decode_ctx *ctx,
               const struct intel_group *inst,
               char **const out_color,
               char **const out_reset_color)
{
   const char *inst_name = intel_group_get_name(inst);
   if (ctx->flags & INTEL_BATCH_DECODE_IN_COLOR) {
      *out_reset_color = NORMAL;
      if (ctx->flags & INTEL_BATCH_DECODE_FULL) {
         if (strcmp(inst_name, "MI_BATCH_BUFFER_START") == 0 ||
             strcmp(inst_name, "MI_BATCH_BUFFER_END") == 0)
            *out_color = GREEN_HEADER;
         else
            *out_color = BLUE_HEADER;
      } else {
         *out_color = NORMAL;
      }
   } else {
      *out_color = "";
      *out_reset_color = "";
   }
}

struct inst_ptr {
   struct intel_group *inst;
   uint32_t           *ptr;
};

static int
compare_inst_ptr(const void *v1, const void *v2)
{
   const struct inst_ptr *i1 = v1, *i2 = v2;
   return strcmp(i1->inst->name, i2->inst->name);
}

static void
intel_print_accumulated_instrs(struct intel_batch_decode_ctx *ctx)
{
   struct util_dynarray arr;
   util_dynarray_init(&arr, NULL);

   hash_table_foreach(ctx->commands, entry) {
      struct inst_ptr inst = {
         .inst = (struct intel_group *)entry->key,
         .ptr  = entry->data,
      };
      util_dynarray_append(&arr, struct inst_ptr, inst);
   }
   qsort(util_dynarray_begin(&arr),
         util_dynarray_num_elements(&arr, struct inst_ptr),
         sizeof(struct inst_ptr),
         compare_inst_ptr);

   fprintf(ctx->fp, "----\n");
   util_dynarray_foreach(&arr, struct inst_ptr, i) {
      char *begin_color;
      char *end_color;
      get_inst_color(ctx, i->inst, &begin_color, &end_color);

      uint64_t offset = 0;
      fprintf(ctx->fp, "%s0x%08"PRIx64":  0x%08x:  %-80s%s\n",
              begin_color, offset, i->ptr[0], i->inst->name, end_color);
      if (ctx->flags & INTEL_BATCH_DECODE_FULL) {
         ctx_print_group(ctx, i->inst, 0, i->ptr);
         for (int d = 0; d < ARRAY_SIZE(custom_decoders); d++) {
            if (strcmp(i->inst->name, custom_decoders[d].cmd_name) == 0) {
               custom_decoders[d].decode(ctx, i->ptr);
               break;
            }
         }
      }
   }
   util_dynarray_fini(&arr);
}

void
intel_print_batch(struct intel_batch_decode_ctx *ctx,
                  const uint32_t *batch, uint32_t batch_size,
                  uint64_t batch_addr, bool from_ring)
{
   const uint32_t *p, *end = batch + batch_size / sizeof(uint32_t);
   int length;
   struct intel_group *inst;
   const char *reset_color = ctx->flags & INTEL_BATCH_DECODE_IN_COLOR ? NORMAL : "";

   if (ctx->n_batch_buffer_start >= 100) {
      fprintf(ctx->fp, "%s0x%08"PRIx64": Max batch buffer jumps exceeded%s\n",
              (ctx->flags & INTEL_BATCH_DECODE_IN_COLOR) ? RED_COLOR : "",
              (ctx->flags & INTEL_BATCH_DECODE_OFFSETS) ? batch_addr : 0,
              reset_color);
      return;
   }

   ctx->n_batch_buffer_start++;

   for (p = batch; p < end; p += length) {
      inst = intel_ctx_find_instruction(ctx, p);
      length = intel_group_get_length(inst, p);
      assert(inst == NULL || length > 0);
      length = MAX2(1, length);

      uint64_t offset;
      if (ctx->flags & INTEL_BATCH_DECODE_OFFSETS)
         offset = batch_addr + ((char *)p - (char *)batch);
      else
         offset = 0;

      if (inst == NULL) {
         fprintf(ctx->fp, "%s0x%08"PRIx64": unknown instruction %08x%s\n",
                 (ctx->flags & INTEL_BATCH_DECODE_IN_COLOR) ? RED_COLOR : "",
                 offset, p[0], reset_color);

         for (int i=1; i < length; i++) {
            fprintf(ctx->fp, "%s0x%08"PRIx64": -- %08x%s\n",
                 (ctx->flags & INTEL_BATCH_DECODE_IN_COLOR) ? RED_COLOR : "",
                 offset + i * 4, p[i], reset_color);
         }

         continue;
      }

      if (ctx->flags & INTEL_BATCH_DECODE_ACCUMULATE) {
         struct hash_entry *entry = _mesa_hash_table_search(ctx->commands, inst);
         if (entry != NULL) {
            entry->data = (void *)p;
         } else {
            _mesa_hash_table_insert(ctx->commands, inst, (void *)p);
         }

         if (!strcmp(inst->name, "3DPRIMITIVE") ||
             !strcmp(inst->name, "3DPRIMITIVE_EXTENDED") ||
             !strcmp(inst->name, "GPGPU_WALKER") ||
             !strcmp(inst->name, "3DSTATE_WM_HZ_OP") ||
             !strcmp(inst->name, "COMPUTE_WALKER")) {
            intel_print_accumulated_instrs(ctx);
         }
      } else {
         char *begin_color;
         char *end_color;
         get_inst_color(ctx, inst, &begin_color, &end_color);

         fprintf(ctx->fp, "%s0x%08"PRIx64"%s:  0x%08x:  %-80s%s\n",
                 begin_color, offset,
                 ctx->acthd && offset == ctx->acthd ? " (ACTHD)" : "", p[0],
                 inst->name, end_color);

         if (ctx->flags & INTEL_BATCH_DECODE_FULL) {
            ctx_print_group(ctx, inst, offset, p);

            for (int i = 0; i < ARRAY_SIZE(custom_decoders); i++) {
               if (strcmp(inst->name, custom_decoders[i].cmd_name) == 0) {
                  custom_decoders[i].decode(ctx, p);
                  break;
               }
            }
         }
      }

      if (strcmp(inst->name, "MI_BATCH_BUFFER_START") == 0) {
         uint64_t next_batch_addr = 0;
         bool ppgtt = false;
         bool second_level = false;
         bool predicate = false;
         struct intel_field_iterator iter;
         intel_field_iterator_init(&iter, inst, p, 0, false);
         while (intel_field_iterator_next(&iter)) {
            if (strcmp(iter.name, "Batch Buffer Start Address") == 0) {
               next_batch_addr = iter.raw_value;
            } else if (strcmp(iter.name, "Second Level Batch Buffer") == 0) {
               second_level = iter.raw_value;
            } else if (strcmp(iter.name, "Address Space Indicator") == 0) {
               ppgtt = iter.raw_value;
            } else if (strcmp(iter.name, "Predication Enable") == 0) {
               predicate = iter.raw_value;
            }
         }

         if (!predicate) {
            struct intel_batch_decode_bo next_batch = ctx_get_bo(ctx, ppgtt, next_batch_addr);

            if (next_batch.map == NULL) {
               fprintf(ctx->fp, "Secondary batch at 0x%08"PRIx64" unavailable\n",
                       next_batch_addr);
            } else {
               intel_print_batch(ctx, next_batch.map, next_batch.size,
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
         }
      } else if (strcmp(inst->name, "MI_BATCH_BUFFER_END") == 0) {
         break;
      }
   }

   ctx->n_batch_buffer_start--;
}

void
intel_batch_stats_reset(struct intel_batch_decode_ctx *ctx)
{
   _mesa_hash_table_clear(ctx->stats, NULL);
}

void
intel_batch_stats(struct intel_batch_decode_ctx *ctx,
                  const uint32_t *batch, uint32_t batch_size,
                  uint64_t batch_addr, bool from_ring)
{
   const uint32_t *p, *end = batch + batch_size / sizeof(uint32_t);
   int length;
   struct intel_group *inst;

   if (ctx->n_batch_buffer_start >= 100) {
      fprintf(stderr, "Max batch buffer jumps exceeded\n");
      return;
   }

   ctx->n_batch_buffer_start++;

   for (p = batch; p < end; p += length) {
      inst = intel_ctx_find_instruction(ctx, p);
      length = intel_group_get_length(inst, p);
      assert(inst == NULL || length > 0);
      length = MAX2(1, length);

      const char *name =
         inst != NULL ? inst->name : "unknown";

      struct hash_entry *entry = _mesa_hash_table_search(ctx->stats, name);
      if (entry != NULL) {
         entry->data = (void *)((uintptr_t)entry->data + 1);
      } else {
         _mesa_hash_table_insert(ctx->stats, name, (void *)(uintptr_t)1);
      }

      if (inst == NULL)
         continue;

      if (strcmp(inst->name, "MI_BATCH_BUFFER_START") == 0) {
         uint64_t next_batch_addr = 0;
         bool ppgtt = false;
         bool second_level = false;
         bool predicate = false;
         struct intel_field_iterator iter;
         intel_field_iterator_init(&iter, inst, p, 0, false);
         while (intel_field_iterator_next(&iter)) {
            if (strcmp(iter.name, "Batch Buffer Start Address") == 0) {
               next_batch_addr = iter.raw_value;
            } else if (strcmp(iter.name, "Second Level Batch Buffer") == 0) {
               second_level = iter.raw_value;
            } else if (strcmp(iter.name, "Address Space Indicator") == 0) {
               ppgtt = iter.raw_value;
            } else if (strcmp(iter.name, "Predication Enable") == 0) {
               predicate = iter.raw_value;
            }
         }

         if (!predicate) {
            struct intel_batch_decode_bo next_batch =
               ctx_get_bo(ctx, ppgtt, next_batch_addr);

            if (next_batch.map == NULL) {
               fprintf(stderr, "Secondary batch at 0x%08"PRIx64" unavailable\n",
                       next_batch_addr);
            } else {
               intel_batch_stats(ctx, next_batch.map, next_batch.size,
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
         }
      } else if (strcmp(inst->name, "MI_BATCH_BUFFER_END") == 0) {
         break;
      }
   }

   ctx->n_batch_buffer_start--;
}

struct inst_stat {
   const char *name;
   uint32_t    count;
};

static int
compare_inst_stat(const void *v1, const void *v2)
{
   const struct inst_stat *i1 = v1, *i2 = v2;
   return strcmp(i1->name, i2->name);
}

void
intel_batch_print_stats(struct intel_batch_decode_ctx *ctx)
{
   struct util_dynarray arr;
   util_dynarray_init(&arr, NULL);

   hash_table_foreach(ctx->stats, entry) {
      struct inst_stat inst = {
         .name = (const char *)entry->key,
         .count = (uintptr_t)entry->data,
      };
      util_dynarray_append(&arr, struct inst_stat, inst);
   }
   qsort(util_dynarray_begin(&arr),
         util_dynarray_num_elements(&arr, struct inst_stat),
         sizeof(struct inst_stat),
         compare_inst_stat);
   util_dynarray_foreach(&arr, struct inst_stat, i)
      fprintf(ctx->fp, "%-40s: %u\n", i->name, i->count);

   util_dynarray_fini(&arr);
}
