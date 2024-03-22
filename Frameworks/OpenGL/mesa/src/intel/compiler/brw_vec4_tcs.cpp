/*
 * Copyright © 2013 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file brw_vec4_tcs.cpp
 *
 * Tessellaton control shader specific code derived from the vec4_visitor class.
 */

#include "brw_nir.h"
#include "brw_vec4_tcs.h"
#include "brw_fs.h"
#include "brw_private.h"
#include "dev/intel_debug.h"

namespace brw {

vec4_tcs_visitor::vec4_tcs_visitor(const struct brw_compiler *compiler,
                                   const struct brw_compile_params *params,
                                   const struct brw_tcs_prog_key *key,
                                   struct brw_tcs_prog_data *prog_data,
                                   const nir_shader *nir,
                                   bool debug_enabled)
   : vec4_visitor(compiler, params, &key->base.tex, &prog_data->base,
                  nir, false, debug_enabled),
     key(key)
{
}


void
vec4_tcs_visitor::setup_payload()
{
   int reg = 0;

   /* The payload always contains important data in r0, which contains
    * the URB handles that are passed on to the URB write at the end
    * of the thread.
    */
   reg++;

   /* r1.0 - r4.7 may contain the input control point URB handles,
    * which we use to pull vertex data.
    */
   reg += 4;

   /* Push constants may start at r5.0 */
   reg = setup_uniforms(reg);

   this->first_non_payload_grf = reg;
}


void
vec4_tcs_visitor::emit_prolog()
{
   invocation_id = src_reg(this, glsl_uint_type());
   emit(TCS_OPCODE_GET_INSTANCE_ID, dst_reg(invocation_id));

   /* HS threads are dispatched with the dispatch mask set to 0xFF.
    * If there are an odd number of output vertices, then the final
    * HS instance dispatched will only have its bottom half doing real
    * work, and so we need to disable the upper half:
    */
   if (nir->info.tess.tcs_vertices_out % 2) {
      emit(CMP(dst_null_d(), invocation_id,
               brw_imm_ud(nir->info.tess.tcs_vertices_out),
               BRW_CONDITIONAL_L));

      /* Matching ENDIF is in emit_thread_end() */
      emit(IF(BRW_PREDICATE_NORMAL));
   }
}


void
vec4_tcs_visitor::emit_thread_end()
{
   vec4_instruction *inst;
   current_annotation = "thread end";

   if (nir->info.tess.tcs_vertices_out % 2) {
      emit(BRW_OPCODE_ENDIF);
   }

   if (devinfo->ver == 7) {
      struct brw_tcs_prog_data *tcs_prog_data =
         (struct brw_tcs_prog_data *) prog_data;

      current_annotation = "release input vertices";

      /* Synchronize all threads, so we know that no one is still
       * using the input URB handles.
       */
      if (tcs_prog_data->instances > 1) {
         dst_reg header = dst_reg(this, glsl_uvec4_type());
         emit(TCS_OPCODE_CREATE_BARRIER_HEADER, header);
         emit(SHADER_OPCODE_BARRIER, dst_null_ud(), src_reg(header));
      }

      /* Make thread 0 (invocations <1, 0>) release pairs of ICP handles.
       * We want to compare the bottom half of invocation_id with 0, but
       * use that truth value for the top half as well.  Unfortunately,
       * we don't have stride in the vec4 world, nor UV immediates in
       * align16, so we need an opcode to get invocation_id<0,4,0>.
       */
      set_condmod(BRW_CONDITIONAL_Z,
                  emit(TCS_OPCODE_SRC0_010_IS_ZERO, dst_null_d(),
                       invocation_id));
      emit(IF(BRW_PREDICATE_NORMAL));
      for (unsigned i = 0; i < key->input_vertices; i += 2) {
         /* If we have an odd number of input vertices, the last will be
          * unpaired.  We don't want to use an interleaved URB write in
          * that case.
          */
         const bool is_unpaired = i == key->input_vertices - 1;

         dst_reg header(this, glsl_uvec4_type());
         emit(TCS_OPCODE_RELEASE_INPUT, header, brw_imm_ud(i),
              brw_imm_ud(is_unpaired));
      }
      emit(BRW_OPCODE_ENDIF);
   }

   inst = emit(TCS_OPCODE_THREAD_END);
   inst->base_mrf = 14;
   inst->mlen = 2;
}


void
vec4_tcs_visitor::emit_input_urb_read(const dst_reg &dst,
                                      const src_reg &vertex_index,
                                      unsigned base_offset,
                                      unsigned first_component,
                                      const src_reg &indirect_offset)
{
   vec4_instruction *inst;
   dst_reg temp(this, glsl_ivec4_type());
   temp.type = dst.type;

   /* Set up the message header to reference the proper parts of the URB */
   dst_reg header = dst_reg(this, glsl_uvec4_type());
   inst = emit(VEC4_TCS_OPCODE_SET_INPUT_URB_OFFSETS, header, vertex_index,
               indirect_offset);
   inst->force_writemask_all = true;

   /* Read into a temporary, ignoring writemasking. */
   inst = emit(VEC4_OPCODE_URB_READ, temp, src_reg(header));
   inst->offset = base_offset;
   inst->mlen = 1;
   inst->base_mrf = -1;

   /* Copy the temporary to the destination to deal with writemasking.
    *
    * Also attempt to deal with gl_PointSize being in the .w component.
    */
   if (inst->offset == 0 && indirect_offset.file == BAD_FILE) {
      emit(MOV(dst, swizzle(src_reg(temp), BRW_SWIZZLE_WWWW)));
   } else {
      src_reg src = src_reg(temp);
      src.swizzle = BRW_SWZ_COMP_INPUT(first_component);
      emit(MOV(dst, src));
   }
}

void
vec4_tcs_visitor::emit_output_urb_read(const dst_reg &dst,
                                       unsigned base_offset,
                                       unsigned first_component,
                                       const src_reg &indirect_offset)
{
   vec4_instruction *inst;

   /* Set up the message header to reference the proper parts of the URB */
   dst_reg header = dst_reg(this, glsl_uvec4_type());
   inst = emit(VEC4_TCS_OPCODE_SET_OUTPUT_URB_OFFSETS, header,
               brw_imm_ud(dst.writemask << first_component), indirect_offset);
   inst->force_writemask_all = true;

   vec4_instruction *read = emit(VEC4_OPCODE_URB_READ, dst, src_reg(header));
   read->offset = base_offset;
   read->mlen = 1;
   read->base_mrf = -1;

   if (first_component) {
      /* Read into a temporary and copy with a swizzle and writemask. */
      read->dst = retype(dst_reg(this, glsl_ivec4_type()), dst.type);
      emit(MOV(dst, swizzle(src_reg(read->dst),
                            BRW_SWZ_COMP_INPUT(first_component))));
   }
}

void
vec4_tcs_visitor::emit_urb_write(const src_reg &value,
                                 unsigned writemask,
                                 unsigned base_offset,
                                 const src_reg &indirect_offset)
{
   if (writemask == 0)
      return;

   src_reg message(this, glsl_uvec4_type(), 2);
   vec4_instruction *inst;

   inst = emit(VEC4_TCS_OPCODE_SET_OUTPUT_URB_OFFSETS, dst_reg(message),
               brw_imm_ud(writemask), indirect_offset);
   inst->force_writemask_all = true;
   inst = emit(MOV(byte_offset(dst_reg(retype(message, value.type)), REG_SIZE),
                   value));
   inst->force_writemask_all = true;

   inst = emit(VEC4_TCS_OPCODE_URB_WRITE, dst_null_f(), message);
   inst->offset = base_offset;
   inst->mlen = 2;
   inst->base_mrf = -1;
}

void
vec4_tcs_visitor::nir_emit_intrinsic(nir_intrinsic_instr *instr)
{
   switch (instr->intrinsic) {
   case nir_intrinsic_load_invocation_id:
      emit(MOV(get_nir_def(instr->def, BRW_REGISTER_TYPE_UD),
               invocation_id));
      break;
   case nir_intrinsic_load_primitive_id:
      emit(TCS_OPCODE_GET_PRIMITIVE_ID,
           get_nir_def(instr->def, BRW_REGISTER_TYPE_UD));
      break;
   case nir_intrinsic_load_patch_vertices_in:
      emit(MOV(get_nir_def(instr->def, BRW_REGISTER_TYPE_D),
               brw_imm_d(key->input_vertices)));
      break;
   case nir_intrinsic_load_per_vertex_input: {
      assert(instr->def.bit_size == 32);
      src_reg indirect_offset = get_indirect_offset(instr);
      unsigned imm_offset = nir_intrinsic_base(instr);

      src_reg vertex_index = retype(get_nir_src_imm(instr->src[0]),
                                    BRW_REGISTER_TYPE_UD);

      unsigned first_component = nir_intrinsic_component(instr);
      dst_reg dst = get_nir_def(instr->def, BRW_REGISTER_TYPE_D);
      dst.writemask = brw_writemask_for_size(instr->num_components);
      emit_input_urb_read(dst, vertex_index, imm_offset,
                          first_component, indirect_offset);
      break;
   }
   case nir_intrinsic_load_input:
      unreachable("nir_lower_io should use load_per_vertex_input intrinsics");
      break;
   case nir_intrinsic_load_output:
   case nir_intrinsic_load_per_vertex_output: {
      src_reg indirect_offset = get_indirect_offset(instr);
      unsigned imm_offset = nir_intrinsic_base(instr);

      dst_reg dst = get_nir_def(instr->def, BRW_REGISTER_TYPE_D);
      dst.writemask = brw_writemask_for_size(instr->num_components);

      emit_output_urb_read(dst, imm_offset, nir_intrinsic_component(instr),
                           indirect_offset);
      break;
   }
   case nir_intrinsic_store_output:
   case nir_intrinsic_store_per_vertex_output: {
      assert(nir_src_bit_size(instr->src[0]) == 32);
      src_reg value = get_nir_src(instr->src[0]);
      unsigned mask = nir_intrinsic_write_mask(instr);
      unsigned swiz = BRW_SWIZZLE_XYZW;

      src_reg indirect_offset = get_indirect_offset(instr);
      unsigned imm_offset = nir_intrinsic_base(instr);

      unsigned first_component = nir_intrinsic_component(instr);
      if (first_component) {
         assert(swiz == BRW_SWIZZLE_XYZW);
         swiz = BRW_SWZ_COMP_OUTPUT(first_component);
         mask = mask << first_component;
      }

      emit_urb_write(swizzle(value, swiz), mask,
                     imm_offset, indirect_offset);
      break;
   }

   case nir_intrinsic_barrier:
      if (nir_intrinsic_memory_scope(instr) != SCOPE_NONE)
         vec4_visitor::nir_emit_intrinsic(instr);
      if (nir_intrinsic_execution_scope(instr) == SCOPE_WORKGROUP) {
         dst_reg header = dst_reg(this, glsl_uvec4_type());
         emit(TCS_OPCODE_CREATE_BARRIER_HEADER, header);
         emit(SHADER_OPCODE_BARRIER, dst_null_ud(), src_reg(header));
      }
      break;

   default:
      vec4_visitor::nir_emit_intrinsic(instr);
   }
}

/**
 * Return the number of patches to accumulate before a MULTI_PATCH mode thread is
 * launched.  In cases with a large number of input control points and a large
 * amount of VS outputs, the VS URB space needed to store an entire 8 patches
 * worth of data can be prohibitive, so it can be beneficial to launch threads
 * early.
 *
 * See the 3DSTATE_HS::Patch Count Threshold documentation for the recommended
 * values.  Note that 0 means to "disable" early dispatch, meaning to wait for
 * a full 8 patches as normal.
 */
static int
get_patch_count_threshold(int input_control_points)
{
   if (input_control_points <= 4)
      return 0;
   else if (input_control_points <= 6)
      return 5;
   else if (input_control_points <= 8)
      return 4;
   else if (input_control_points <= 10)
      return 3;
   else if (input_control_points <= 14)
      return 2;

   /* Return patch count 1 for PATCHLIST_15 - PATCHLIST_32 */
   return 1;
}

} /* namespace brw */

extern "C" const unsigned *
brw_compile_tcs(const struct brw_compiler *compiler,
                struct brw_compile_tcs_params *params)
{
   const struct intel_device_info *devinfo = compiler->devinfo;
   nir_shader *nir = params->base.nir;
   const struct brw_tcs_prog_key *key = params->key;
   struct brw_tcs_prog_data *prog_data = params->prog_data;
   struct brw_vue_prog_data *vue_prog_data = &prog_data->base;

   const bool is_scalar = compiler->scalar_stage[MESA_SHADER_TESS_CTRL];
   const bool debug_enabled = brw_should_print_shader(nir, DEBUG_TCS);
   const unsigned *assembly;

   vue_prog_data->base.stage = MESA_SHADER_TESS_CTRL;
   prog_data->base.base.ray_queries = nir->info.ray_queries;
   prog_data->base.base.total_scratch = 0;

   nir->info.outputs_written = key->outputs_written;
   nir->info.patch_outputs_written = key->patch_outputs_written;

   struct brw_vue_map input_vue_map;
   brw_compute_vue_map(devinfo, &input_vue_map, nir->info.inputs_read,
                       nir->info.separate_shader, 1);
   brw_compute_tess_vue_map(&vue_prog_data->vue_map,
                            nir->info.outputs_written,
                            nir->info.patch_outputs_written);

   brw_nir_apply_key(nir, compiler, &key->base, 8);
   brw_nir_lower_vue_inputs(nir, &input_vue_map);
   brw_nir_lower_tcs_outputs(nir, &vue_prog_data->vue_map,
                             key->_tes_primitive_mode);
   if (key->quads_workaround)
      brw_nir_apply_tcs_quads_workaround(nir);
   if (key->input_vertices > 0)
      brw_nir_lower_patch_vertices_in(nir, key->input_vertices);

   brw_postprocess_nir(nir, compiler, debug_enabled,
                       key->base.robust_flags);

   bool has_primitive_id =
      BITSET_TEST(nir->info.system_values_read, SYSTEM_VALUE_PRIMITIVE_ID);

   prog_data->patch_count_threshold = brw::get_patch_count_threshold(key->input_vertices);

   if (compiler->use_tcs_multi_patch) {
      vue_prog_data->dispatch_mode = DISPATCH_MODE_TCS_MULTI_PATCH;
      prog_data->instances = nir->info.tess.tcs_vertices_out;
      prog_data->include_primitive_id = has_primitive_id;
   } else {
      unsigned verts_per_thread = is_scalar ? 8 : 2;
      vue_prog_data->dispatch_mode = DISPATCH_MODE_TCS_SINGLE_PATCH;
      prog_data->instances =
         DIV_ROUND_UP(nir->info.tess.tcs_vertices_out, verts_per_thread);
   }

   /* Compute URB entry size.  The maximum allowed URB entry size is 32k.
    * That divides up as follows:
    *
    *     32 bytes for the patch header (tessellation factors)
    *    480 bytes for per-patch varyings (a varying component is 4 bytes and
    *              gl_MaxTessPatchComponents = 120)
    *  16384 bytes for per-vertex varyings (a varying component is 4 bytes,
    *              gl_MaxPatchVertices = 32 and
    *              gl_MaxTessControlOutputComponents = 128)
    *
    *  15808 bytes left for varying packing overhead
    */
   const int num_per_patch_slots = vue_prog_data->vue_map.num_per_patch_slots;
   const int num_per_vertex_slots = vue_prog_data->vue_map.num_per_vertex_slots;
   unsigned output_size_bytes = 0;
   /* Note that the patch header is counted in num_per_patch_slots. */
   output_size_bytes += num_per_patch_slots * 16;
   output_size_bytes += nir->info.tess.tcs_vertices_out *
                        num_per_vertex_slots * 16;

   assert(output_size_bytes >= 1);
   if (output_size_bytes > GFX7_MAX_HS_URB_ENTRY_SIZE_BYTES)
      return NULL;

   /* URB entry sizes are stored as a multiple of 64 bytes. */
   vue_prog_data->urb_entry_size = ALIGN(output_size_bytes, 64) / 64;

   /* HS does not use the usual payload pushing from URB to GRFs,
    * because we don't have enough registers for a full-size payload, and
    * the hardware is broken on Haswell anyway.
    */
   vue_prog_data->urb_read_length = 0;

   if (unlikely(debug_enabled)) {
      fprintf(stderr, "TCS Input ");
      brw_print_vue_map(stderr, &input_vue_map, MESA_SHADER_TESS_CTRL);
      fprintf(stderr, "TCS Output ");
      brw_print_vue_map(stderr, &vue_prog_data->vue_map, MESA_SHADER_TESS_CTRL);
   }

   if (is_scalar) {
      const unsigned dispatch_width = devinfo->ver >= 20 ? 16 : 8;
      fs_visitor v(compiler, &params->base, &key->base,
                   &prog_data->base.base, nir, dispatch_width,
                   params->base.stats != NULL, debug_enabled);
      if (!v.run_tcs()) {
         params->base.error_str =
            ralloc_strdup(params->base.mem_ctx, v.fail_msg);
         return NULL;
      }

      assert(v.payload().num_regs % reg_unit(devinfo) == 0);
      prog_data->base.base.dispatch_grf_start_reg = v.payload().num_regs / reg_unit(devinfo);

      fs_generator g(compiler, &params->base,
                     &prog_data->base.base, false, MESA_SHADER_TESS_CTRL);
      if (unlikely(debug_enabled)) {
         g.enable_debug(ralloc_asprintf(params->base.mem_ctx,
                                        "%s tessellation control shader %s",
                                        nir->info.label ? nir->info.label
                                                        : "unnamed",
                                        nir->info.name));
      }

      g.generate_code(v.cfg, dispatch_width, v.shader_stats,
                      v.performance_analysis.require(), params->base.stats);

      g.add_const_data(nir->constant_data, nir->constant_data_size);

      assembly = g.get_assembly();
   } else {
      brw::vec4_tcs_visitor v(compiler, &params->base, key, prog_data,
                              nir, debug_enabled);
      if (!v.run()) {
         params->base.error_str =
            ralloc_strdup(params->base.mem_ctx, v.fail_msg);
         return NULL;
      }

      if (INTEL_DEBUG(DEBUG_TCS))
         v.dump_instructions();


      assembly = brw_vec4_generate_assembly(compiler, &params->base, nir,
                                            &prog_data->base, v.cfg,
                                            v.performance_analysis.require(),
                                            debug_enabled);
   }

   return assembly;
}
