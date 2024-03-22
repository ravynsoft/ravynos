/*
 * Copyright © 2021 Intel Corporation
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

#include <list>
#include <vector>
#include "brw_compiler.h"
#include "brw_fs.h"
#include "brw_nir.h"
#include "brw_private.h"
#include "compiler/nir/nir_builder.h"
#include "dev/intel_debug.h"

#include <memory>

using namespace brw;

static bool
brw_nir_lower_load_uniforms_filter(const nir_instr *instr,
                                   UNUSED const void *data)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   return intrin->intrinsic == nir_intrinsic_load_uniform;
}

static nir_def *
brw_nir_lower_load_uniforms_impl(nir_builder *b, nir_instr *instr,
                                 UNUSED void *data)
{
   assert(instr->type == nir_instr_type_intrinsic);
   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   assert(intrin->intrinsic == nir_intrinsic_load_uniform);

   /* Read the first few 32-bit scalars from InlineData. */
   if (nir_src_is_const(intrin->src[0]) &&
       intrin->def.bit_size == 32 &&
       intrin->def.num_components == 1) {
      unsigned off = nir_intrinsic_base(intrin) + nir_src_as_uint(intrin->src[0]);
      unsigned off_dw = off / 4;
      if (off % 4 == 0 && off_dw < BRW_TASK_MESH_PUSH_CONSTANTS_SIZE_DW) {
         off_dw += BRW_TASK_MESH_PUSH_CONSTANTS_START_DW;
         return nir_load_mesh_inline_data_intel(b, 32, off_dw);
      }
   }

   return brw_nir_load_global_const(b, intrin,
                                    nir_load_mesh_inline_data_intel(b, 64, 0), 0);
}

static bool
brw_nir_lower_load_uniforms(nir_shader *nir)
{
   return nir_shader_lower_instructions(nir, brw_nir_lower_load_uniforms_filter,
                                        brw_nir_lower_load_uniforms_impl, NULL);
}

static inline int
type_size_scalar_dwords(const struct glsl_type *type, bool bindless)
{
   return glsl_count_dword_slots(type, bindless);
}

/* TODO(mesh): Make this a common function. */
static void
shared_type_info(const struct glsl_type *type, unsigned *size, unsigned *align)
{
   assert(glsl_type_is_vector_or_scalar(type));

   uint32_t comp_size = glsl_type_is_boolean(type)
      ? 4 : glsl_get_bit_size(type) / 8;
   unsigned length = glsl_get_vector_elements(type);
   *size = comp_size * length,
   *align = comp_size * (length == 3 ? 4 : length);
}

static bool
brw_nir_lower_launch_mesh_workgroups_instr(nir_builder *b,
                                           nir_intrinsic_instr *intrin,
                                           void *data)
{
   if (intrin->intrinsic != nir_intrinsic_launch_mesh_workgroups)
      return false;

   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *local_invocation_index = nir_load_local_invocation_index(b);

   /* Make sure that the mesh workgroup size is taken from the first invocation
    * (nir_intrinsic_launch_mesh_workgroups requirement)
    */
   nir_def *cmp = nir_ieq_imm(b, local_invocation_index, 0);
   nir_if *if_stmt = nir_push_if(b, cmp);
   {
      /* TUE header contains 4 words:
       *
       * - Word 0 for Task Count.
       *
       * - Words 1-3 used for "Dispatch Dimensions" feature, to allow mapping a
       *   3D dispatch into the 1D dispatch supported by HW.
       */
      nir_def *x = nir_channel(b, intrin->src[0].ssa, 0);
      nir_def *y = nir_channel(b, intrin->src[0].ssa, 1);
      nir_def *z = nir_channel(b, intrin->src[0].ssa, 2);
      nir_def *task_count = nir_imul(b, x, nir_imul(b, y, z));
      nir_def *tue_header = nir_vec4(b, task_count, x, y, z);
      nir_store_task_payload(b, tue_header, nir_imm_int(b, 0));
   }
   nir_pop_if(b, if_stmt);

   nir_instr_remove(&intrin->instr);

   return true;
}

static bool
brw_nir_lower_launch_mesh_workgroups(nir_shader *nir)
{
   return nir_shader_intrinsics_pass(nir,
                                       brw_nir_lower_launch_mesh_workgroups_instr,
                                       nir_metadata_none,
                                       NULL);
}

static void
brw_nir_lower_tue_outputs(nir_shader *nir, brw_tue_map *map)
{
   memset(map, 0, sizeof(*map));

   NIR_PASS(_, nir, nir_lower_io, nir_var_shader_out,
            type_size_scalar_dwords, nir_lower_io_lower_64bit_to_32);

   /* From bspec: "It is suggested that SW reserve the 16 bytes following the
    * TUE Header, and therefore start the SW-defined data structure at 32B
    * alignment.  This allows the TUE Header to always be written as 32 bytes
    * with 32B alignment, the most optimal write performance case."
    */
   map->per_task_data_start_dw = 8;

   /* Lowering to explicit types will start offsets from task_payload_size, so
    * set it to start after the header.
    */
   nir->info.task_payload_size = map->per_task_data_start_dw * 4;
   NIR_PASS(_, nir, nir_lower_vars_to_explicit_types,
            nir_var_mem_task_payload, shared_type_info);
   NIR_PASS(_, nir, nir_lower_explicit_io,
            nir_var_mem_task_payload, nir_address_format_32bit_offset);

   map->size_dw = ALIGN(DIV_ROUND_UP(nir->info.task_payload_size, 4), 8);
}

static void
brw_print_tue_map(FILE *fp, const struct brw_tue_map *map)
{
   fprintf(fp, "TUE (%d dwords)\n\n", map->size_dw);
}

static bool
brw_nir_adjust_task_payload_offsets_instr(struct nir_builder *b,
                                          nir_intrinsic_instr *intrin,
                                          void *data)
{
   switch (intrin->intrinsic) {
   case nir_intrinsic_store_task_payload:
   case nir_intrinsic_load_task_payload: {
      nir_src *offset_src = nir_get_io_offset_src(intrin);

      if (nir_src_is_const(*offset_src))
         assert(nir_src_as_uint(*offset_src) % 4 == 0);

      b->cursor = nir_before_instr(&intrin->instr);

      /* Regular I/O uses dwords while explicit I/O used for task payload uses
       * bytes.  Normalize it to dwords.
       *
       * TODO(mesh): Figure out how to handle 8-bit, 16-bit.
       */

      nir_def *offset = nir_ishr_imm(b, offset_src->ssa, 2);
      nir_src_rewrite(offset_src, offset);

      unsigned base = nir_intrinsic_base(intrin);
      assert(base % 4 == 0);
      nir_intrinsic_set_base(intrin, base / 4);

      return true;
   }

   default:
      return false;
   }
}

static bool
brw_nir_adjust_task_payload_offsets(nir_shader *nir)
{
   return nir_shader_intrinsics_pass(nir,
                                       brw_nir_adjust_task_payload_offsets_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}

void
brw_nir_adjust_payload(nir_shader *shader)
{
   /* Adjustment of task payload offsets must be performed *after* last pass
    * which interprets them as bytes, because it changes their unit.
    */
   bool adjusted = false;
   NIR_PASS(adjusted, shader, brw_nir_adjust_task_payload_offsets);
   if (adjusted) /* clean up the mess created by offset adjustments */
      NIR_PASS(_, shader, nir_opt_constant_folding);
}

static bool
brw_nir_align_launch_mesh_workgroups_instr(nir_builder *b,
                                           nir_intrinsic_instr *intrin,
                                           void *data)
{
   if (intrin->intrinsic != nir_intrinsic_launch_mesh_workgroups)
      return false;

   /* nir_lower_task_shader uses "range" as task payload size. */
   unsigned range = nir_intrinsic_range(intrin);
   /* This will avoid special case in nir_lower_task_shader dealing with
    * not vec4-aligned payload when payload_in_shared workaround is enabled.
    */
   nir_intrinsic_set_range(intrin, ALIGN(range, 16));

   return true;
}

static bool
brw_nir_align_launch_mesh_workgroups(nir_shader *nir)
{
   return nir_shader_intrinsics_pass(nir,
                                       brw_nir_align_launch_mesh_workgroups_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}

const unsigned *
brw_compile_task(const struct brw_compiler *compiler,
                 struct brw_compile_task_params *params)
{
   struct nir_shader *nir = params->base.nir;
   const struct brw_task_prog_key *key = params->key;
   struct brw_task_prog_data *prog_data = params->prog_data;
   const bool debug_enabled = brw_should_print_shader(nir, DEBUG_TASK);

   brw_nir_lower_tue_outputs(nir, &prog_data->map);

   NIR_PASS(_, nir, brw_nir_align_launch_mesh_workgroups);

   nir_lower_task_shader_options lower_ts_opt = {
      .payload_to_shared_for_atomics = true,
      .payload_to_shared_for_small_types = true,
      /* The actual payload data starts after the TUE header and padding,
       * so skip those when copying.
       */
      .payload_offset_in_bytes = prog_data->map.per_task_data_start_dw * 4,
   };
   NIR_PASS(_, nir, nir_lower_task_shader, lower_ts_opt);

   NIR_PASS(_, nir, brw_nir_lower_launch_mesh_workgroups);

   prog_data->base.base.stage = MESA_SHADER_TASK;
   prog_data->base.base.total_shared = nir->info.shared_size;
   prog_data->base.base.total_scratch = 0;

   prog_data->base.local_size[0] = nir->info.workgroup_size[0];
   prog_data->base.local_size[1] = nir->info.workgroup_size[1];
   prog_data->base.local_size[2] = nir->info.workgroup_size[2];

   prog_data->uses_drawid =
      BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_DRAW_ID);

   brw_simd_selection_state simd_state{
      .devinfo = compiler->devinfo,
      .prog_data = &prog_data->base,
      .required_width = brw_required_dispatch_width(&nir->info),
   };

   std::unique_ptr<fs_visitor> v[3];

   for (unsigned simd = 0; simd < 3; simd++) {
      if (!brw_simd_should_compile(simd_state, simd))
         continue;

      const unsigned dispatch_width = 8 << simd;

      nir_shader *shader = nir_shader_clone(params->base.mem_ctx, nir);
      brw_nir_apply_key(shader, compiler, &key->base, dispatch_width);

      NIR_PASS(_, shader, brw_nir_lower_load_uniforms);
      NIR_PASS(_, shader, brw_nir_lower_simd, dispatch_width);

      brw_postprocess_nir(shader, compiler, debug_enabled,
                          key->base.robust_flags);

      v[simd] = std::make_unique<fs_visitor>(compiler, &params->base,
                                             &key->base,
                                             &prog_data->base.base,
                                             shader, dispatch_width,
                                             params->base.stats != NULL,
                                             debug_enabled);

      if (prog_data->base.prog_mask) {
         unsigned first = ffs(prog_data->base.prog_mask) - 1;
         v[simd]->import_uniforms(v[first].get());
      }

      const bool allow_spilling = !brw_simd_any_compiled(simd_state);
      if (v[simd]->run_task(allow_spilling))
         brw_simd_mark_compiled(simd_state, simd, v[simd]->spilled_any_registers);
      else
         simd_state.error[simd] = ralloc_strdup(params->base.mem_ctx, v[simd]->fail_msg);
   }

   int selected_simd = brw_simd_select(simd_state);
   if (selected_simd < 0) {
      params->base.error_str =
         ralloc_asprintf(params->base.mem_ctx,
                         "Can't compile shader: "
                         "SIMD8 '%s', SIMD16 '%s' and SIMD32 '%s'.\n",
                         simd_state.error[0], simd_state.error[1],
                         simd_state.error[2]);
      return NULL;
   }

   fs_visitor *selected = v[selected_simd].get();
   prog_data->base.prog_mask = 1 << selected_simd;

   if (unlikely(debug_enabled)) {
      fprintf(stderr, "Task Output ");
      brw_print_tue_map(stderr, &prog_data->map);
   }

   fs_generator g(compiler, &params->base, &prog_data->base.base,
                  false, MESA_SHADER_TASK);
   if (unlikely(debug_enabled)) {
      g.enable_debug(ralloc_asprintf(params->base.mem_ctx,
                                     "%s task shader %s",
                                     nir->info.label ? nir->info.label
                                                     : "unnamed",
                                     nir->info.name));
   }

   g.generate_code(selected->cfg, selected->dispatch_width, selected->shader_stats,
                   selected->performance_analysis.require(), params->base.stats);
   g.add_const_data(nir->constant_data, nir->constant_data_size);
   return g.get_assembly();
}

static void
brw_nir_lower_tue_inputs(nir_shader *nir, const brw_tue_map *map)
{
   if (!map)
      return;

   nir->info.task_payload_size = map->per_task_data_start_dw * 4;

   bool progress = false;

   NIR_PASS(progress, nir, nir_lower_vars_to_explicit_types,
            nir_var_mem_task_payload, shared_type_info);

   if (progress) {
      /* The types for Task Output and Mesh Input should match, so their sizes
       * should also match.
       */
      assert(map->size_dw == ALIGN(DIV_ROUND_UP(nir->info.task_payload_size, 4), 8));
   } else {
      /* Mesh doesn't read any input, to make it clearer set the
       * task_payload_size to zero instead of keeping an incomplete size that
       * just includes the header.
       */
      nir->info.task_payload_size = 0;
   }

   NIR_PASS(_, nir, nir_lower_explicit_io, nir_var_mem_task_payload,
            nir_address_format_32bit_offset);
}

/* Attribute types. Flat attributes have to be a separate class because
 * flat and interpolated attributes can't share the same vec4 slot
 * (see 3DSTATE_SBE.ConstantInterpolationEnable).
 */
enum {
   PRIM, /* per primitive */
   VERT, /* per vertex interpolated */
   VERT_FLAT, /* per vertex flat */
};

struct attr_desc {
   int location;
   const struct glsl_type *type;
   unsigned dwords;
   unsigned slots;
};

struct attr_type_info {
   /* order of attributes, negative values are holes */
   std::list<struct attr_desc> *order;

   /* attributes after which there's hole of size equal to array index */
   std::list<int> holes[5];
};

static void
brw_mue_assign_position(const struct attr_desc *attr,
                        struct brw_mue_map *map,
                        unsigned start_dw)
{
   bool is_array = glsl_type_is_array(attr->type);
   int location = attr->location;
   unsigned remaining = attr->dwords;

   for (unsigned slot = 0; slot < attr->slots; ++slot) {
      map->start_dw[location + slot] = start_dw;

      unsigned sz;

      if (is_array) {
         assert(attr->dwords % attr->slots == 0);
         sz = attr->dwords / attr->slots;
      } else {
         sz = MIN2(remaining, 4);
      }

      map->len_dw[location + slot] = sz;
      start_dw += sz;
      remaining -= sz;
   }
}

static nir_variable *
brw_nir_find_complete_variable_with_location(nir_shader *shader,
                                             nir_variable_mode mode,
                                             int location)
{
   nir_variable *best_var = NULL;
   unsigned last_size = 0;

   nir_foreach_variable_with_modes(var, shader, mode) {
      if (var->data.location != location)
         continue;

      unsigned new_size = glsl_count_dword_slots(var->type, false);
      if (new_size > last_size) {
         best_var = var;
         last_size = new_size;
      }
   }

   return best_var;
}

static unsigned
brw_sum_size(const std::list<struct attr_desc> &orders)
{
   unsigned sz = 0;
   for (auto it = orders.cbegin(); it != orders.cend(); ++it)
      sz += (*it).dwords;
   return sz;
}

/* Finds order of outputs which require minimum size, without splitting
 * of URB read/write messages (which operate on vec4-aligned memory).
 */
static void
brw_compute_mue_layout(const struct brw_compiler *compiler,
                       std::list<struct attr_desc> *orders,
                       uint64_t outputs_written,
                       struct nir_shader *nir,
                       bool *pack_prim_data_into_header,
                       bool *pack_vert_data_into_header)
{
   const struct shader_info *info = &nir->info;

   struct attr_type_info data[3];

   if ((compiler->mesh.mue_header_packing & 1) == 0)
      *pack_prim_data_into_header = false;
   if ((compiler->mesh.mue_header_packing & 2) == 0)
      *pack_vert_data_into_header = false;

   for (unsigned i = PRIM; i <= VERT_FLAT; ++i)
      data[i].order = &orders[i];

   /* If packing into header is enabled, add a hole of size 4 and add
    * a virtual location to keep the algorithm happy (it expects holes
    * to be preceded by some location). We'll remove those virtual
    * locations at the end.
    */
   const gl_varying_slot virtual_header_location = VARYING_SLOT_POS;
   assert((outputs_written & BITFIELD64_BIT(virtual_header_location)) == 0);

   struct attr_desc d;
   d.location = virtual_header_location;
   d.type = NULL;
   d.dwords = 0;
   d.slots = 0;

   struct attr_desc h;
   h.location = -1;
   h.type = NULL;
   h.dwords = 4;
   h.slots = 0;

   if (*pack_prim_data_into_header) {
      orders[PRIM].push_back(d);
      orders[PRIM].push_back(h);
      data[PRIM].holes[4].push_back(virtual_header_location);
   }

   if (*pack_vert_data_into_header) {
      orders[VERT].push_back(d);
      orders[VERT].push_back(h);
      data[VERT].holes[4].push_back(virtual_header_location);
   }

   u_foreach_bit64(location, outputs_written) {
      if ((BITFIELD64_BIT(location) & outputs_written) == 0)
         continue;

      /* At this point there are both complete and split variables as
       * outputs. We need the complete variable to compute the required
       * size.
       */
      nir_variable *var =
            brw_nir_find_complete_variable_with_location(nir,
                                                         nir_var_shader_out,
                                                         location);

      d.location = location;
      d.type     = brw_nir_get_var_type(nir, var);
      d.dwords   = glsl_count_dword_slots(d.type, false);
      d.slots    = glsl_count_attribute_slots(d.type, false);

      struct attr_type_info *type_data;

      if (BITFIELD64_BIT(location) & info->per_primitive_outputs)
         type_data = &data[PRIM];
      else if (var->data.interpolation == INTERP_MODE_FLAT)
         type_data = &data[VERT_FLAT];
      else
         type_data = &data[VERT];

      std::list<struct attr_desc> *order = type_data->order;
      std::list<int> *holes = type_data->holes;

      outputs_written &= ~BITFIELD64_RANGE(location, d.slots);

      /* special case to use hole of size 4 */
      if (d.dwords == 4 && !holes[4].empty()) {
         holes[4].pop_back();

         assert(order->front().location == virtual_header_location);
         order->pop_front();

         assert(order->front().location == -1);
         assert(order->front().dwords == 4);
         order->front() = d;

         continue;
      }

      int mod = d.dwords % 4;
      if (mod == 0) {
         order->push_back(d);
         continue;
      }

      h.location = -1;
      h.type = NULL;
      h.dwords = 4 - mod;
      h.slots = 0;

      if (!compiler->mesh.mue_compaction) {
         order->push_back(d);
         order->push_back(h);
         continue;
      }

      if (d.dwords > 4) {
         order->push_back(d);
         order->push_back(h);
         holes[h.dwords].push_back(location);
         continue;
      }

      assert(d.dwords < 4);

      unsigned found = 0;
      /* try to find the smallest hole big enough to hold this attribute */
      for (unsigned sz = d.dwords; sz <= 4; sz++){
         if (!holes[sz].empty()) {
            found = sz;
            break;
         }
      }

      /* append at the end if not found */
      if (found == 0) {
         order->push_back(d);
         order->push_back(h);
         holes[h.dwords].push_back(location);

         continue;
      }

      assert(found <= 4);
      assert(!holes[found].empty());
      int after_loc = holes[found].back();
      holes[found].pop_back();

      bool inserted_back = false;

      for (auto it = order->begin(); it != order->end(); ++it) {
         if ((*it).location != after_loc)
            continue;

         ++it;
         /* must be a hole */
         assert((*it).location < 0);
         /* and it must be big enough */
         assert(d.dwords <= (*it).dwords);

         if (d.dwords == (*it).dwords) {
            /* exact size, just replace */
            *it = d;
         } else {
            /* inexact size, shrink hole */
            (*it).dwords -= d.dwords;
            /* and insert new attribute before it */
            order->insert(it, d);

            /* Insert shrunk hole in a spot so that the order of attributes
             * is preserved.
             */
            std::list<int> &hole_list = holes[(*it).dwords];
            std::list<int>::iterator insert_before = hole_list.end();

            for (auto it2 = hole_list.begin(); it2 != hole_list.end(); ++it2) {
               if ((*it2) >= (int)location) {
                  insert_before = it2;
                  break;
               }
            }

            hole_list.insert(insert_before, location);
         }

         inserted_back = true;
         break;
      }

      assert(inserted_back);
   }

   if (*pack_prim_data_into_header) {
      if (orders[PRIM].front().location == virtual_header_location)
         orders[PRIM].pop_front();

      if (!data[PRIM].holes[4].empty()) {
         *pack_prim_data_into_header = false;

         assert(orders[PRIM].front().location == -1);
         assert(orders[PRIM].front().dwords == 4);
         orders[PRIM].pop_front();
      }

      if (*pack_prim_data_into_header) {
         unsigned sz = brw_sum_size(orders[PRIM]);

         if (sz % 8 == 0 || sz % 8 > 4)
            *pack_prim_data_into_header = false;
      }
   }

   if (*pack_vert_data_into_header) {
      if (orders[VERT].front().location == virtual_header_location)
         orders[VERT].pop_front();

      if (!data[VERT].holes[4].empty()) {
         *pack_vert_data_into_header = false;

         assert(orders[VERT].front().location == -1);
         assert(orders[VERT].front().dwords == 4);
         orders[VERT].pop_front();
      }

      if (*pack_vert_data_into_header) {
         unsigned sz = brw_sum_size(orders[VERT]) +
                       brw_sum_size(orders[VERT_FLAT]);

         if (sz % 8 == 0 || sz % 8 > 4)
            *pack_vert_data_into_header = false;
      }
   }


   if (INTEL_DEBUG(DEBUG_MESH)) {
      fprintf(stderr, "MUE attribute order:\n");
      for (unsigned i = PRIM; i <= VERT_FLAT; ++i) {
         if (!orders[i].empty())
            fprintf(stderr, "%d: ", i);
         for (auto it = orders[i].cbegin(); it != orders[i].cend(); ++it) {
            fprintf(stderr, "%d(%d) ", (*it).location, (*it).dwords);
         }
         if (!orders[i].empty())
            fprintf(stderr, "\n");
      }
   }
}

/* Mesh URB Entry consists of an initial section
 *
 *  - Primitive Count
 *  - Primitive Indices (from 0 to Max-1)
 *  - Padding to 32B if needed
 *
 * optionally followed by a section for per-primitive data,
 * in which each primitive (from 0 to Max-1) gets
 *
 *  - Primitive Header (e.g. ViewportIndex)
 *  - Primitive Custom Attributes
 *
 * then followed by a section for per-vertex data
 *
 *  - Vertex Header (e.g. Position)
 *  - Vertex Custom Attributes
 *
 * Each per-element section has a pitch and a starting offset.  All the
 * individual attributes offsets in start_dw are considering the first entry
 * of the section (i.e. where the Position for first vertex, or ViewportIndex
 * for first primitive).  Attributes for other elements are calculated using
 * the pitch.
 */
static void
brw_compute_mue_map(const struct brw_compiler *compiler,
                    struct nir_shader *nir, struct brw_mue_map *map,
                    enum brw_mesh_index_format index_format, bool compact_mue)
{
   memset(map, 0, sizeof(*map));

   memset(&map->start_dw[0], -1, sizeof(map->start_dw));
   memset(&map->len_dw[0], 0, sizeof(map->len_dw));

   unsigned vertices_per_primitive =
      mesa_vertices_per_prim(nir->info.mesh.primitive_type);

   map->max_primitives = nir->info.mesh.max_primitives_out;
   map->max_vertices = nir->info.mesh.max_vertices_out;

   uint64_t outputs_written = nir->info.outputs_written;

   /* One dword for primitives count then K extra dwords for each primitive. */
   switch (index_format) {
   case BRW_INDEX_FORMAT_U32:
      map->per_primitive_indices_dw = vertices_per_primitive;
      break;
   case BRW_INDEX_FORMAT_U888X:
      map->per_primitive_indices_dw = 1;
      break;
   default:
      unreachable("invalid index format");
   }

   map->per_primitive_start_dw = ALIGN(map->per_primitive_indices_dw *
                                       map->max_primitives + 1, 8);

   /* Assign initial section. */
   if (BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_COUNT) & outputs_written) {
      map->start_dw[VARYING_SLOT_PRIMITIVE_COUNT] = 0;
      map->len_dw[VARYING_SLOT_PRIMITIVE_COUNT] = 1;
      outputs_written &= ~BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_COUNT);
   }
   if (BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_INDICES) & outputs_written) {
      map->start_dw[VARYING_SLOT_PRIMITIVE_INDICES] = 1;
      map->len_dw[VARYING_SLOT_PRIMITIVE_INDICES] =
            map->per_primitive_indices_dw * map->max_primitives;
      outputs_written &= ~BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_INDICES);
   }

   const uint64_t per_primitive_header_bits =
         BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_SHADING_RATE) |
         BITFIELD64_BIT(VARYING_SLOT_LAYER) |
         BITFIELD64_BIT(VARYING_SLOT_VIEWPORT) |
         BITFIELD64_BIT(VARYING_SLOT_CULL_PRIMITIVE);

   const uint64_t per_vertex_header_bits =
         BITFIELD64_BIT(VARYING_SLOT_PSIZ) |
         BITFIELD64_BIT(VARYING_SLOT_POS) |
         BITFIELD64_BIT(VARYING_SLOT_CLIP_DIST0) |
         BITFIELD64_BIT(VARYING_SLOT_CLIP_DIST1);

   std::list<struct attr_desc> orders[3];
   uint64_t regular_outputs = outputs_written &
         ~(per_primitive_header_bits | per_vertex_header_bits);

   /* packing into prim header is possible only if prim header is present */
   map->user_data_in_primitive_header = compact_mue &&
         (outputs_written & per_primitive_header_bits) != 0;

   /* Packing into vert header is always possible, but we allow it only
    * if full vec4 is available (so point size is not used) and there's
    * nothing between it and normal vertex data (so no clip distances).
    */
   map->user_data_in_vertex_header = compact_mue &&
         (outputs_written & per_vertex_header_bits) ==
               BITFIELD64_BIT(VARYING_SLOT_POS);

   if (outputs_written & per_primitive_header_bits) {
      if (outputs_written & BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_SHADING_RATE)) {
         map->start_dw[VARYING_SLOT_PRIMITIVE_SHADING_RATE] =
               map->per_primitive_start_dw + 0;
         map->len_dw[VARYING_SLOT_PRIMITIVE_SHADING_RATE] = 1;
      }

      if (outputs_written & BITFIELD64_BIT(VARYING_SLOT_LAYER)) {
         map->start_dw[VARYING_SLOT_LAYER] =
               map->per_primitive_start_dw + 1; /* RTAIndex */
         map->len_dw[VARYING_SLOT_LAYER] = 1;
      }

      if (outputs_written & BITFIELD64_BIT(VARYING_SLOT_VIEWPORT)) {
         map->start_dw[VARYING_SLOT_VIEWPORT] =
               map->per_primitive_start_dw + 2;
         map->len_dw[VARYING_SLOT_VIEWPORT] = 1;
      }

      if (outputs_written & BITFIELD64_BIT(VARYING_SLOT_CULL_PRIMITIVE)) {
         map->start_dw[VARYING_SLOT_CULL_PRIMITIVE] =
               map->per_primitive_start_dw + 3;
         map->len_dw[VARYING_SLOT_CULL_PRIMITIVE] = 1;
      }

      map->per_primitive_header_size_dw = 8;
      outputs_written &= ~per_primitive_header_bits;
   } else {
      map->per_primitive_header_size_dw = 0;
   }

   map->per_primitive_data_size_dw = 0;

   /* For fast linked libraries, we can't pack the MUE, as the fragment shader
    * will be compiled without access to the MUE map and won't be able to find
    * out where everything is.
    * Instead, keep doing things as we did before the packing, just laying out
    * everything in varying order, which is how the FS will expect them.
    */
   if (compact_mue) {
      brw_compute_mue_layout(compiler, orders, regular_outputs, nir,
                             &map->user_data_in_primitive_header,
                             &map->user_data_in_vertex_header);

      unsigned start_dw = map->per_primitive_start_dw;
      if (map->user_data_in_primitive_header)
         start_dw += 4; /* first 4 dwords are used */
      else
         start_dw += map->per_primitive_header_size_dw;
      unsigned header_used_dw = 0;

      for (auto it = orders[PRIM].cbegin(); it != orders[PRIM].cend(); ++it) {
         int location = (*it).location;
         if (location < 0) {
            start_dw += (*it).dwords;
            if (map->user_data_in_primitive_header && header_used_dw < 4)
               header_used_dw += (*it).dwords;
            else
               map->per_primitive_data_size_dw += (*it).dwords;
            assert(header_used_dw <= 4);
            continue;
         }

         assert(map->start_dw[location] == -1);

         assert(location == VARYING_SLOT_PRIMITIVE_ID ||
                location >= VARYING_SLOT_VAR0);

         brw_mue_assign_position(&*it, map, start_dw);

         start_dw += (*it).dwords;
         if (map->user_data_in_primitive_header && header_used_dw < 4)
            header_used_dw += (*it).dwords;
         else
            map->per_primitive_data_size_dw += (*it).dwords;
         assert(header_used_dw <= 4);
         outputs_written &= ~BITFIELD64_RANGE(location, (*it).slots);
      }
   } else {
      unsigned start_dw = map->per_primitive_start_dw +
                          map->per_primitive_header_size_dw;

      uint64_t per_prim_outputs = outputs_written & nir->info.per_primitive_outputs;
      while (per_prim_outputs) {
         uint64_t location = ffsll(per_prim_outputs) - 1;

         assert(map->start_dw[location] == -1);
         assert(location == VARYING_SLOT_PRIMITIVE_ID ||
                location >= VARYING_SLOT_VAR0);

         nir_variable *var =
            brw_nir_find_complete_variable_with_location(nir,
                                                         nir_var_shader_out,
                                                         location);
         struct attr_desc d;
         d.location = location;
         d.type     = brw_nir_get_var_type(nir, var);
         d.dwords   = glsl_count_dword_slots(d.type, false);
         d.slots    = glsl_count_attribute_slots(d.type, false);

         brw_mue_assign_position(&d, map, start_dw);

         map->per_primitive_data_size_dw += ALIGN(d.dwords, 4);
         start_dw += ALIGN(d.dwords, 4);

         per_prim_outputs &= ~BITFIELD64_RANGE(location, d.slots);
      }
   }

   map->per_primitive_pitch_dw = ALIGN(map->per_primitive_header_size_dw +
                                       map->per_primitive_data_size_dw, 8);

   map->per_vertex_start_dw = ALIGN(map->per_primitive_start_dw +
                                    map->per_primitive_pitch_dw *
                                    map->max_primitives, 8);

   /* TODO(mesh): Multiview. */
   unsigned fixed_header_size = 8;
   map->per_vertex_header_size_dw = ALIGN(fixed_header_size +
                                          nir->info.clip_distance_array_size +
                                          nir->info.cull_distance_array_size, 8);

   if (outputs_written & per_vertex_header_bits) {
      if (outputs_written & BITFIELD64_BIT(VARYING_SLOT_PSIZ)) {
         map->start_dw[VARYING_SLOT_PSIZ] = map->per_vertex_start_dw + 3;
         map->len_dw[VARYING_SLOT_PSIZ] = 1;
      }

      if (outputs_written & BITFIELD64_BIT(VARYING_SLOT_POS)) {
         map->start_dw[VARYING_SLOT_POS] = map->per_vertex_start_dw + 4;
         map->len_dw[VARYING_SLOT_POS] = 4;
      }

      if (outputs_written & BITFIELD64_BIT(VARYING_SLOT_CLIP_DIST0)) {
         map->start_dw[VARYING_SLOT_CLIP_DIST0] =
               map->per_vertex_start_dw + fixed_header_size + 0;
         map->len_dw[VARYING_SLOT_CLIP_DIST0] = 4;
      }

      if (outputs_written & BITFIELD64_BIT(VARYING_SLOT_CLIP_DIST1)) {
         map->start_dw[VARYING_SLOT_CLIP_DIST1] =
               map->per_vertex_start_dw + fixed_header_size + 4;
         map->len_dw[VARYING_SLOT_CLIP_DIST1] = 4;
      }

      outputs_written &= ~per_vertex_header_bits;
   }

   /* cull distances should be lowered earlier */
   assert(!(outputs_written & BITFIELD64_BIT(VARYING_SLOT_CULL_DIST0)));
   assert(!(outputs_written & BITFIELD64_BIT(VARYING_SLOT_CULL_DIST1)));

   map->per_vertex_data_size_dw = 0;

   /* For fast linked libraries, we can't pack the MUE, as the fragment shader
    * will be compiled without access to the MUE map and won't be able to find
    * out where everything is.
    * Instead, keep doing things as we did before the packing, just laying out
    * everything in varying order, which is how the FS will expect them.
    */
   if (compact_mue) {
      unsigned start_dw = map->per_vertex_start_dw;
      if (!map->user_data_in_vertex_header)
         start_dw += map->per_vertex_header_size_dw;

      unsigned header_used_dw = 0;
      for (unsigned type = VERT; type <= VERT_FLAT; ++type) {
         for (auto it = orders[type].cbegin(); it != orders[type].cend(); ++it) {
            int location = (*it).location;
            if (location < 0) {
               start_dw += (*it).dwords;
               if (map->user_data_in_vertex_header && header_used_dw < 4) {
                  header_used_dw += (*it).dwords;
                  assert(header_used_dw <= 4);
                  if (header_used_dw == 4)
                     start_dw += 4; /* jump over gl_position */
               } else {
                  map->per_vertex_data_size_dw += (*it).dwords;
               }
               continue;
            }

            assert(map->start_dw[location] == -1);

            assert(location >= VARYING_SLOT_VAR0);

            brw_mue_assign_position(&*it, map, start_dw);

            start_dw += (*it).dwords;
            if (map->user_data_in_vertex_header && header_used_dw < 4) {
               header_used_dw += (*it).dwords;
               assert(header_used_dw <= 4);
               if (header_used_dw == 4)
                  start_dw += 4; /* jump over gl_position */
            } else {
               map->per_vertex_data_size_dw += (*it).dwords;
            }
            outputs_written &= ~BITFIELD64_RANGE(location, (*it).slots);
         }
      }
   } else {
      unsigned start_dw = map->per_vertex_start_dw +
                          map->per_vertex_header_size_dw;

      uint64_t per_vertex_outputs = outputs_written & ~nir->info.per_primitive_outputs;
      while (per_vertex_outputs) {
         uint64_t location = ffsll(per_vertex_outputs) - 1;

         assert(map->start_dw[location] == -1);
         assert(location >= VARYING_SLOT_VAR0);

         nir_variable *var =
            brw_nir_find_complete_variable_with_location(nir,
                                                         nir_var_shader_out,
                                                         location);
         struct attr_desc d;
         d.location = location;
         d.type     = brw_nir_get_var_type(nir, var);
         d.dwords   = glsl_count_dword_slots(d.type, false);
         d.slots    = glsl_count_attribute_slots(d.type, false);

         brw_mue_assign_position(&d, map, start_dw);

         map->per_vertex_data_size_dw += ALIGN(d.dwords, 4);
         start_dw += ALIGN(d.dwords, 4);

         per_vertex_outputs &= ~BITFIELD64_RANGE(location, d.slots);
      }
   }

   map->per_vertex_pitch_dw = ALIGN(map->per_vertex_header_size_dw +
                                    map->per_vertex_data_size_dw, 8);

   map->size_dw =
      map->per_vertex_start_dw + map->per_vertex_pitch_dw * map->max_vertices;

   assert(map->size_dw % 8 == 0);
}

static void
brw_print_mue_map(FILE *fp, const struct brw_mue_map *map, struct nir_shader *nir)
{
   fprintf(fp, "MUE map (%d dwords, %d primitives, %d vertices)\n",
           map->size_dw, map->max_primitives, map->max_vertices);
   fprintf(fp, "  <%4d, %4d>: VARYING_SLOT_PRIMITIVE_COUNT\n",
           map->start_dw[VARYING_SLOT_PRIMITIVE_COUNT],
           map->start_dw[VARYING_SLOT_PRIMITIVE_COUNT] +
           map->len_dw[VARYING_SLOT_PRIMITIVE_COUNT] - 1);
   fprintf(fp, "  <%4d, %4d>: VARYING_SLOT_PRIMITIVE_INDICES\n",
           map->start_dw[VARYING_SLOT_PRIMITIVE_INDICES],
           map->start_dw[VARYING_SLOT_PRIMITIVE_INDICES] +
           map->len_dw[VARYING_SLOT_PRIMITIVE_INDICES] - 1);

   fprintf(fp, "  ----- per primitive (start %d, header_size %d, data_size %d, pitch %d)\n",
           map->per_primitive_start_dw,
           map->per_primitive_header_size_dw,
           map->per_primitive_data_size_dw,
           map->per_primitive_pitch_dw);

   for (unsigned i = 0; i < VARYING_SLOT_MAX; i++) {
      if (map->start_dw[i] < 0)
         continue;

      const unsigned offset = map->start_dw[i];
      const unsigned len = map->len_dw[i];

      if (offset < map->per_primitive_start_dw ||
          offset >= map->per_primitive_start_dw + map->per_primitive_pitch_dw)
         continue;

      const char *name =
            gl_varying_slot_name_for_stage((gl_varying_slot)i,
                                           MESA_SHADER_MESH);

      fprintf(fp, "  <%4d, %4d>: %s (%d)\n", offset, offset + len - 1,
              name, i);
   }

   fprintf(fp, "  ----- per vertex (start %d, header_size %d, data_size %d, pitch %d)\n",
           map->per_vertex_start_dw,
           map->per_vertex_header_size_dw,
           map->per_vertex_data_size_dw,
           map->per_vertex_pitch_dw);

   for (unsigned i = 0; i < VARYING_SLOT_MAX; i++) {
      if (map->start_dw[i] < 0)
         continue;

      const unsigned offset = map->start_dw[i];
      const unsigned len = map->len_dw[i];

      if (offset < map->per_vertex_start_dw ||
          offset >= map->per_vertex_start_dw + map->per_vertex_pitch_dw)
         continue;

      nir_variable *var =
            nir_find_variable_with_location(nir, nir_var_shader_out, i);
      bool flat = var->data.interpolation == INTERP_MODE_FLAT;

      const char *name =
            gl_varying_slot_name_for_stage((gl_varying_slot)i,
                                           MESA_SHADER_MESH);

      fprintf(fp, "  <%4d, %4d>: %s (%d)%s\n", offset, offset + len - 1,
              name, i, flat ? " (flat)" : "");
   }

   fprintf(fp, "\n");
}

static void
brw_nir_lower_mue_outputs(nir_shader *nir, const struct brw_mue_map *map)
{
   nir_foreach_shader_out_variable(var, nir) {
      int location = var->data.location;
      assert(location >= 0);
      assert(map->start_dw[location] != -1);
      var->data.driver_location = map->start_dw[location];
   }

   NIR_PASS(_, nir, nir_lower_io, nir_var_shader_out,
            type_size_scalar_dwords, nir_lower_io_lower_64bit_to_32);
}

static void
brw_nir_initialize_mue(nir_shader *nir,
                       const struct brw_mue_map *map,
                       unsigned dispatch_width)
{
   assert(map->per_primitive_header_size_dw > 0);

   nir_builder b;
   nir_function_impl *entrypoint = nir_shader_get_entrypoint(nir);
   b = nir_builder_at(nir_before_impl(entrypoint));

   nir_def *dw_off = nir_imm_int(&b, 0);
   nir_def *zerovec = nir_imm_vec4(&b, 0, 0, 0, 0);

   /* TODO(mesh): can we write in bigger batches, generating fewer SENDs? */

   assert(!nir->info.workgroup_size_variable);
   const unsigned workgroup_size = nir->info.workgroup_size[0] *
                                   nir->info.workgroup_size[1] *
                                   nir->info.workgroup_size[2];

   /* Invocations from a single workgroup will cooperate in zeroing MUE. */

   /* How many prims each invocation needs to cover without checking its index? */
   unsigned prims_per_inv = map->max_primitives / workgroup_size;

   /* Zero first 4 dwords of MUE Primitive Header:
    * Reserved, RTAIndex, ViewportIndex, CullPrimitiveMask.
    */

   nir_def *local_invocation_index = nir_load_local_invocation_index(&b);

   /* Zero primitive headers distanced by workgroup_size, starting from
    * invocation index.
    */
   for (unsigned prim_in_inv = 0; prim_in_inv < prims_per_inv; ++prim_in_inv) {
      nir_def *prim = nir_iadd_imm(&b, local_invocation_index,
                                           prim_in_inv * workgroup_size);

      nir_store_per_primitive_output(&b, zerovec, prim, dw_off,
                                     .base = (int)map->per_primitive_start_dw,
                                     .write_mask = WRITEMASK_XYZW,
                                     .component = 0,
                                     .src_type = nir_type_uint32);
   }

   /* How many prims are left? */
   unsigned remaining = map->max_primitives % workgroup_size;

   if (remaining) {
      /* Zero "remaining" primitive headers starting from the last one covered
       * by the loop above + workgroup_size.
       */
      nir_def *cmp = nir_ilt_imm(&b, local_invocation_index, remaining);
      nir_if *if_stmt = nir_push_if(&b, cmp);
      {
         nir_def *prim = nir_iadd_imm(&b, local_invocation_index,
                                               prims_per_inv * workgroup_size);

         nir_store_per_primitive_output(&b, zerovec, prim, dw_off,
                                        .base = (int)map->per_primitive_start_dw,
                                        .write_mask = WRITEMASK_XYZW,
                                        .component = 0,
                                        .src_type = nir_type_uint32);
      }
      nir_pop_if(&b, if_stmt);
   }

   /* If there's more than one subgroup, then we need to wait for all of them
    * to finish initialization before we can proceed. Otherwise some subgroups
    * may start filling MUE before other finished initializing.
    */
   if (workgroup_size > dispatch_width) {
      nir_barrier(&b, SCOPE_WORKGROUP, SCOPE_WORKGROUP,
                         NIR_MEMORY_ACQ_REL, nir_var_shader_out);
   }

   if (remaining) {
      nir_metadata_preserve(entrypoint, nir_metadata_none);
   } else {
      nir_metadata_preserve(entrypoint, nir_metadata_block_index |
                                        nir_metadata_dominance);
   }
}

static void
brw_nir_adjust_offset(nir_builder *b, nir_intrinsic_instr *intrin, uint32_t pitch)
{
   nir_src *index_src = nir_get_io_arrayed_index_src(intrin);
   nir_src *offset_src = nir_get_io_offset_src(intrin);

   b->cursor = nir_before_instr(&intrin->instr);
   nir_def *offset =
      nir_iadd(b,
               offset_src->ssa,
               nir_imul_imm(b, index_src->ssa, pitch));
   nir_src_rewrite(offset_src, offset);
}

static bool
brw_nir_adjust_offset_for_arrayed_indices_instr(nir_builder *b,
                                                nir_intrinsic_instr *intrin,
                                                void *data)
{
   const struct brw_mue_map *map = (const struct brw_mue_map *) data;

   /* Remap per_vertex and per_primitive offsets using the extra source and
    * the pitch.
    */
   switch (intrin->intrinsic) {
   case nir_intrinsic_load_per_vertex_output:
   case nir_intrinsic_store_per_vertex_output:
      brw_nir_adjust_offset(b, intrin, map->per_vertex_pitch_dw);

      return true;

   case nir_intrinsic_load_per_primitive_output:
   case nir_intrinsic_store_per_primitive_output: {
      struct nir_io_semantics sem = nir_intrinsic_io_semantics(intrin);
      uint32_t pitch;
      if (sem.location == VARYING_SLOT_PRIMITIVE_INDICES)
         pitch = map->per_primitive_indices_dw;
      else
         pitch = map->per_primitive_pitch_dw;

      brw_nir_adjust_offset(b, intrin, pitch);

      return true;
   }

   default:
      return false;
   }
}

static bool
brw_nir_adjust_offset_for_arrayed_indices(nir_shader *nir, const struct brw_mue_map *map)
{
   return nir_shader_intrinsics_pass(nir,
                                       brw_nir_adjust_offset_for_arrayed_indices_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       (void *)map);
}

struct index_packing_state {
   unsigned vertices_per_primitive;
   nir_variable *original_prim_indices;
   nir_variable *packed_prim_indices;
};

static bool
brw_can_pack_primitive_indices(nir_shader *nir, struct index_packing_state *state)
{
   /* can single index fit into one byte of U888X format? */
   if (nir->info.mesh.max_vertices_out > 255)
      return false;

   state->vertices_per_primitive =
         mesa_vertices_per_prim(nir->info.mesh.primitive_type);
   /* packing point indices doesn't help */
   if (state->vertices_per_primitive == 1)
      return false;

   state->original_prim_indices =
      nir_find_variable_with_location(nir,
                                      nir_var_shader_out,
                                      VARYING_SLOT_PRIMITIVE_INDICES);
   /* no indices = no changes to the shader, but it's still worth it,
    * because less URB space will be used
    */
   if (!state->original_prim_indices)
      return true;

   ASSERTED const struct glsl_type *type = state->original_prim_indices->type;
   assert(glsl_type_is_array(type));
   assert(glsl_type_is_vector(glsl_without_array(type)));
   assert(glsl_without_array(type)->vector_elements == state->vertices_per_primitive);

   nir_foreach_function_impl(impl, nir) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

            if (intrin->intrinsic != nir_intrinsic_store_deref) {
               /* any unknown deref operation on primitive indices -> don't pack */
               unsigned num_srcs = nir_intrinsic_infos[intrin->intrinsic].num_srcs;
               for (unsigned i = 0; i < num_srcs; i++) {
                  nir_deref_instr *deref = nir_src_as_deref(intrin->src[i]);
                  if (!deref)
                     continue;
                  nir_variable *var = nir_deref_instr_get_variable(deref);

                  if (var == state->original_prim_indices)
                     return false;
               }

               continue;
            }

            nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
            if (!deref)
               continue;

            nir_variable *var = nir_deref_instr_get_variable(deref);
            if (var != state->original_prim_indices)
               continue;

            if (deref->deref_type != nir_deref_type_array)
               return false; /* unknown chain of derefs */

            nir_deref_instr *var_deref = nir_src_as_deref(deref->parent);
            if (!var_deref || var_deref->deref_type != nir_deref_type_var)
               return false; /* unknown chain of derefs */

            assert (var_deref->var == state->original_prim_indices);

            unsigned write_mask = nir_intrinsic_write_mask(intrin);

            /* If only some components are written, then we can't easily pack.
             * In theory we could, by loading current dword value, bitmasking
             * one byte and storing back the whole dword, but it would be slow
             * and could actually decrease performance. TODO: reevaluate this
             * once there will be something hitting this.
             */
            if (write_mask != BITFIELD_MASK(state->vertices_per_primitive))
               return false;
         }
      }
   }

   return true;
}

static bool
brw_pack_primitive_indices_instr(nir_builder *b, nir_intrinsic_instr *intrin,
                                 void *data)
{
   if (intrin->intrinsic != nir_intrinsic_store_deref)
      return false;

   nir_deref_instr *array_deref = nir_src_as_deref(intrin->src[0]);
   if (!array_deref || array_deref->deref_type != nir_deref_type_array)
      return false;

   nir_deref_instr *var_deref = nir_src_as_deref(array_deref->parent);
   if (!var_deref || var_deref->deref_type != nir_deref_type_var)
      return false;

   struct index_packing_state *state =
         (struct index_packing_state *)data;

   nir_variable *var = var_deref->var;

   if (var != state->original_prim_indices)
      return false;

   unsigned vertices_per_primitive = state->vertices_per_primitive;

   b->cursor = nir_before_instr(&intrin->instr);

   nir_deref_instr *new_var_deref =
         nir_build_deref_var(b, state->packed_prim_indices);
   nir_deref_instr *new_array_deref =
         nir_build_deref_array(b, new_var_deref, array_deref->arr.index.ssa);

   nir_src *data_src = &intrin->src[1];
   nir_def *data_def =
         data_src->ssa;

   nir_def *new_data =
         nir_ior(b, nir_ishl_imm(b, nir_channel(b, data_def, 0), 0),
                    nir_ishl_imm(b, nir_channel(b, data_def, 1), 8));

   if (vertices_per_primitive >= 3) {
      new_data =
            nir_ior(b, new_data,
                       nir_ishl_imm(b, nir_channel(b, data_def, 2), 16));
   }

   nir_build_store_deref(b, &new_array_deref->def, new_data);

   nir_instr_remove(&intrin->instr);

   return true;
}

static bool
brw_pack_primitive_indices(nir_shader *nir, void *data)
{
   struct index_packing_state *state = (struct index_packing_state *)data;

   const struct glsl_type *new_type =
         glsl_array_type(glsl_uint_type(),
                         nir->info.mesh.max_primitives_out,
                         0);

   state->packed_prim_indices =
         nir_variable_create(nir, nir_var_shader_out,
                             new_type, "gl_PrimitiveIndicesPacked");
   state->packed_prim_indices->data.location = VARYING_SLOT_PRIMITIVE_INDICES;
   state->packed_prim_indices->data.interpolation = INTERP_MODE_NONE;
   state->packed_prim_indices->data.per_primitive = 1;

   return nir_shader_intrinsics_pass(nir, brw_pack_primitive_indices_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       data);
}

const unsigned *
brw_compile_mesh(const struct brw_compiler *compiler,
                 struct brw_compile_mesh_params *params)
{
   struct nir_shader *nir = params->base.nir;
   const struct brw_mesh_prog_key *key = params->key;
   struct brw_mesh_prog_data *prog_data = params->prog_data;
   const bool debug_enabled = brw_should_print_shader(nir, DEBUG_MESH);

   prog_data->base.base.stage = MESA_SHADER_MESH;
   prog_data->base.base.total_shared = nir->info.shared_size;
   prog_data->base.base.total_scratch = 0;

   prog_data->base.local_size[0] = nir->info.workgroup_size[0];
   prog_data->base.local_size[1] = nir->info.workgroup_size[1];
   prog_data->base.local_size[2] = nir->info.workgroup_size[2];

   prog_data->clip_distance_mask = (1 << nir->info.clip_distance_array_size) - 1;
   prog_data->cull_distance_mask =
         ((1 << nir->info.cull_distance_array_size) - 1) <<
          nir->info.clip_distance_array_size;
   prog_data->primitive_type = nir->info.mesh.primitive_type;

   struct index_packing_state index_packing_state = {};
   if (brw_can_pack_primitive_indices(nir, &index_packing_state)) {
      if (index_packing_state.original_prim_indices)
         NIR_PASS(_, nir, brw_pack_primitive_indices, &index_packing_state);
      prog_data->index_format = BRW_INDEX_FORMAT_U888X;
   } else {
      prog_data->index_format = BRW_INDEX_FORMAT_U32;
   }

   prog_data->uses_drawid =
      BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_DRAW_ID);

   brw_nir_lower_tue_inputs(nir, params->tue_map);

   brw_compute_mue_map(compiler, nir, &prog_data->map,
                       prog_data->index_format, key->compact_mue);
   brw_nir_lower_mue_outputs(nir, &prog_data->map);

   brw_simd_selection_state simd_state{
      .devinfo = compiler->devinfo,
      .prog_data = &prog_data->base,
      .required_width = brw_required_dispatch_width(&nir->info),
   };

   std::unique_ptr<fs_visitor> v[3];

   for (int simd = 0; simd < 3; simd++) {
      if (!brw_simd_should_compile(simd_state, simd))
         continue;

      const unsigned dispatch_width = 8 << simd;

      nir_shader *shader = nir_shader_clone(params->base.mem_ctx, nir);

      /*
       * When Primitive Header is enabled, we may not generates writes to all
       * fields, so let's initialize everything.
       */
      if (prog_data->map.per_primitive_header_size_dw > 0)
         NIR_PASS_V(shader, brw_nir_initialize_mue, &prog_data->map, dispatch_width);

      brw_nir_apply_key(shader, compiler, &key->base, dispatch_width);

      NIR_PASS(_, shader, brw_nir_adjust_offset_for_arrayed_indices, &prog_data->map);
      /* Load uniforms can do a better job for constants, so fold before it. */
      NIR_PASS(_, shader, nir_opt_constant_folding);
      NIR_PASS(_, shader, brw_nir_lower_load_uniforms);

      NIR_PASS(_, shader, brw_nir_lower_simd, dispatch_width);

      brw_postprocess_nir(shader, compiler, debug_enabled,
                          key->base.robust_flags);

      v[simd] = std::make_unique<fs_visitor>(compiler, &params->base,
                                             &key->base,
                                             &prog_data->base.base,
                                             shader, dispatch_width,
                                             params->base.stats != NULL,
                                             debug_enabled);

      if (prog_data->base.prog_mask) {
         unsigned first = ffs(prog_data->base.prog_mask) - 1;
         v[simd]->import_uniforms(v[first].get());
      }

      const bool allow_spilling = !brw_simd_any_compiled(simd_state);
      if (v[simd]->run_mesh(allow_spilling))
         brw_simd_mark_compiled(simd_state, simd, v[simd]->spilled_any_registers);
      else
         simd_state.error[simd] = ralloc_strdup(params->base.mem_ctx, v[simd]->fail_msg);
   }

   int selected_simd = brw_simd_select(simd_state);
   if (selected_simd < 0) {
      params->base.error_str =
         ralloc_asprintf(params->base.mem_ctx,
                         "Can't compile shader: "
                         "SIMD8 '%s', SIMD16 '%s' and SIMD32 '%s'.\n",
                         simd_state.error[0], simd_state.error[1],
                         simd_state.error[2]);
      return NULL;
   }

   fs_visitor *selected = v[selected_simd].get();
   prog_data->base.prog_mask = 1 << selected_simd;

   if (unlikely(debug_enabled)) {
      if (params->tue_map) {
         fprintf(stderr, "Mesh Input ");
         brw_print_tue_map(stderr, params->tue_map);
      }
      fprintf(stderr, "Mesh Output ");
      brw_print_mue_map(stderr, &prog_data->map, nir);
   }

   fs_generator g(compiler, &params->base, &prog_data->base.base,
                  false, MESA_SHADER_MESH);
   if (unlikely(debug_enabled)) {
      g.enable_debug(ralloc_asprintf(params->base.mem_ctx,
                                     "%s mesh shader %s",
                                     nir->info.label ? nir->info.label
                                                     : "unnamed",
                                     nir->info.name));
   }

   g.generate_code(selected->cfg, selected->dispatch_width, selected->shader_stats,
                   selected->performance_analysis.require(), params->base.stats);
   g.add_const_data(nir->constant_data, nir->constant_data_size);
   return g.get_assembly();
}
