/*
 * Copyright © 2020 Intel Corporation
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

#include "brw_nir_rt.h"
#include "brw_nir_rt_builder.h"

static bool
resize_deref(nir_builder *b, nir_deref_instr *deref,
             unsigned num_components, unsigned bit_size)
{
   if (deref->def.num_components == num_components &&
       deref->def.bit_size == bit_size)
      return false;

   /* NIR requires array indices have to match the deref bit size */
   if (deref->def.bit_size != bit_size &&
       (deref->deref_type == nir_deref_type_array ||
        deref->deref_type == nir_deref_type_ptr_as_array)) {
      b->cursor = nir_before_instr(&deref->instr);
      nir_def *idx;
      if (nir_src_is_const(deref->arr.index)) {
         idx = nir_imm_intN_t(b, nir_src_as_int(deref->arr.index), bit_size);
      } else {
         idx = nir_i2iN(b, deref->arr.index.ssa, bit_size);
      }
      nir_src_rewrite(&deref->arr.index, idx);
   }

   deref->def.num_components = num_components;
   deref->def.bit_size = bit_size;

   return true;
}

static bool
lower_rt_io_derefs(nir_shader *shader)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);

   bool progress = false;

   unsigned num_shader_call_vars = 0;
   nir_foreach_variable_with_modes(var, shader, nir_var_shader_call_data)
      num_shader_call_vars++;

   unsigned num_ray_hit_attrib_vars = 0;
   nir_foreach_variable_with_modes(var, shader, nir_var_ray_hit_attrib)
      num_ray_hit_attrib_vars++;

   /* At most one payload is allowed because it's an input.  Technically, this
    * is also true for hit attribute variables.  However, after we inline an
    * any-hit shader into an intersection shader, we can end up with multiple
    * hit attribute variables.  They'll end up mapping to a cast from the same
    * base pointer so this is fine.
    */
   assert(num_shader_call_vars <= 1);

   nir_builder b = nir_builder_at(nir_before_impl(impl));

   nir_def *call_data_addr = NULL;
   if (num_shader_call_vars > 0) {
      assert(shader->scratch_size >= BRW_BTD_STACK_CALLEE_DATA_SIZE);
      call_data_addr =
         brw_nir_rt_load_scratch(&b, BRW_BTD_STACK_CALL_DATA_PTR_OFFSET, 8,
                                 1, 64);
      progress = true;
   }

   gl_shader_stage stage = shader->info.stage;
   nir_def *hit_attrib_addr = NULL;
   if (num_ray_hit_attrib_vars > 0) {
      assert(stage == MESA_SHADER_ANY_HIT ||
             stage == MESA_SHADER_CLOSEST_HIT ||
             stage == MESA_SHADER_INTERSECTION);
      nir_def *hit_addr =
         brw_nir_rt_mem_hit_addr(&b, stage == MESA_SHADER_CLOSEST_HIT);
      /* The vec2 barycentrics are in 2nd and 3rd dwords of MemHit */
      nir_def *bary_addr = nir_iadd_imm(&b, hit_addr, 4);
      hit_attrib_addr = nir_bcsel(&b, nir_load_leaf_procedural_intel(&b),
                                      brw_nir_rt_hit_attrib_data_addr(&b),
                                      bary_addr);
      progress = true;
   }

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_deref)
            continue;

         nir_deref_instr *deref = nir_instr_as_deref(instr);
         if (nir_deref_mode_is(deref, nir_var_shader_call_data)) {
            deref->modes = nir_var_function_temp;
            if (deref->deref_type == nir_deref_type_var) {
               b.cursor = nir_before_instr(&deref->instr);
               nir_deref_instr *cast =
                  nir_build_deref_cast(&b, call_data_addr,
                                       nir_var_function_temp,
                                       deref->var->type, 0);
               nir_def_rewrite_uses(&deref->def,
                                        &cast->def);
               nir_instr_remove(&deref->instr);
               progress = true;
            }
         } else if (nir_deref_mode_is(deref, nir_var_ray_hit_attrib)) {
            deref->modes = nir_var_function_temp;
            if (deref->deref_type == nir_deref_type_var) {
               b.cursor = nir_before_instr(&deref->instr);
               nir_deref_instr *cast =
                  nir_build_deref_cast(&b, hit_attrib_addr,
                                       nir_var_function_temp,
                                       deref->type, 0);
               nir_def_rewrite_uses(&deref->def,
                                        &cast->def);
               nir_instr_remove(&deref->instr);
               progress = true;
            }
         }

         /* We're going to lower all function_temp memory to scratch using
          * 64-bit addresses.  We need to resize all our derefs first or else
          * nir_lower_explicit_io will have a fit.
          */
         if (nir_deref_mode_is(deref, nir_var_function_temp) &&
             resize_deref(&b, deref, 1, 64))
            progress = true;
      }
   }

   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }

   return progress;
}

/** Lowers ray-tracing shader I/O and scratch access
 *
 * SPV_KHR_ray_tracing adds three new types of I/O, each of which need their
 * own bit of special care:
 *
 *  - Shader payload data:  This is represented by the IncomingCallableData
 *    and IncomingRayPayload storage classes which are both represented by
 *    nir_var_call_data in NIR.  There is at most one of these per-shader and
 *    they contain payload data passed down the stack from the parent shader
 *    when it calls executeCallable() or traceRay().  In our implementation,
 *    the actual storage lives in the calling shader's scratch space and we're
 *    passed a pointer to it.
 *
 *  - Hit attribute data:  This is represented by the HitAttribute storage
 *    class in SPIR-V and nir_var_ray_hit_attrib in NIR.  For triangle
 *    geometry, it's supposed to contain two floats which are the barycentric
 *    coordinates.  For AABS/procedural geometry, it contains the hit data
 *    written out by the intersection shader.  In our implementation, it's a
 *    64-bit pointer which points either to the u/v area of the relevant
 *    MemHit data structure or the space right after the HW ray stack entry.
 *
 *  - Shader record buffer data:  This allows read-only access to the data
 *    stored in the SBT right after the bindless shader handles.  It's
 *    effectively a UBO with a magic address.  Coming out of spirv_to_nir,
 *    we get a nir_intrinsic_load_shader_record_ptr which is cast to a
 *    nir_var_mem_global deref and all access happens through that.  The
 *    shader_record_ptr system value is handled in brw_nir_lower_rt_intrinsics
 *    and we assume nir_lower_explicit_io is called elsewhere thanks to
 *    VK_KHR_buffer_device_address so there's really nothing to do here.
 *
 * We also handle lowering any remaining function_temp variables to scratch at
 * this point.  This gets rid of any remaining arrays and also takes care of
 * the sending side of ray payloads where we pass pointers to a function_temp
 * variable down the call stack.
 */
static void
lower_rt_io_and_scratch(nir_shader *nir)
{
   /* First, we to ensure all the I/O variables have explicit types.  Because
    * these are shader-internal and don't come in from outside, they don't
    * have an explicit memory layout and we have to assign them one.
    */
   NIR_PASS_V(nir, nir_lower_vars_to_explicit_types,
              nir_var_function_temp |
              nir_var_shader_call_data |
              nir_var_ray_hit_attrib,
              glsl_get_natural_size_align_bytes);

   /* Now patch any derefs to I/O vars */
   NIR_PASS_V(nir, lower_rt_io_derefs);

   /* Finally, lower any remaining function_temp, mem_constant, or
    * ray_hit_attrib access to 64-bit global memory access.
    */
   NIR_PASS_V(nir, nir_lower_explicit_io,
              nir_var_function_temp |
              nir_var_mem_constant |
              nir_var_ray_hit_attrib,
              nir_address_format_64bit_global);
}

static void
build_terminate_ray(nir_builder *b)
{
   nir_def *skip_closest_hit = nir_test_mask(b, nir_load_ray_flags(b),
      BRW_RT_RAY_FLAG_SKIP_CLOSEST_HIT_SHADER);
   nir_push_if(b, skip_closest_hit);
   {
      /* The shader that calls traceRay() is unable to access any ray hit
       * information except for that which is explicitly written into the ray
       * payload by shaders invoked during the trace.  If there's no closest-
       * hit shader, then accepting the hit has no observable effect; it's
       * just extra memory traffic for no reason.
       */
      brw_nir_btd_return(b);
      nir_jump(b, nir_jump_halt);
   }
   nir_push_else(b, NULL);
   {
      /* The closest hit shader is in the same shader group as the any-hit
       * shader that we're currently in.  We can get the address for its SBT
       * handle by looking at the shader record pointer and subtracting the
       * size of a SBT handle.  The BINDLESS_SHADER_RECORD for a closest hit
       * shader is the first one in the SBT handle.
       */
      nir_def *closest_hit =
         nir_iadd_imm(b, nir_load_shader_record_ptr(b),
                        -BRW_RT_SBT_HANDLE_SIZE);

      brw_nir_rt_commit_hit(b);
      brw_nir_btd_spawn(b, closest_hit);
      nir_jump(b, nir_jump_halt);
   }
   nir_pop_if(b, NULL);
}

/** Lowers away ray walk intrinsics
 *
 * This lowers terminate_ray, ignore_ray_intersection, and the NIR-specific
 * accept_ray_intersection intrinsics to the appropriate Intel-specific
 * intrinsics.
 */
static bool
lower_ray_walk_intrinsics(nir_shader *shader,
                          const struct intel_device_info *devinfo)
{
   assert(shader->info.stage == MESA_SHADER_ANY_HIT ||
          shader->info.stage == MESA_SHADER_INTERSECTION);

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);

   nir_builder b = nir_builder_create(impl);

   bool progress = false;
   nir_foreach_block_safe(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

         switch (intrin->intrinsic) {
         case nir_intrinsic_ignore_ray_intersection: {
            b.cursor = nir_instr_remove(&intrin->instr);

            /* We put the newly emitted code inside a dummy if because it's
             * going to contain a jump instruction and we don't want to deal
             * with that mess here.  It'll get dealt with by our control-flow
             * optimization passes.
             */
            nir_push_if(&b, nir_imm_true(&b));
            nir_trace_ray_intel(&b,
                                nir_load_btd_global_arg_addr_intel(&b),
                                nir_imm_int(&b, BRW_RT_BVH_LEVEL_OBJECT),
                                nir_imm_int(&b, GEN_RT_TRACE_RAY_CONTINUE),
                                .synchronous = false);
            nir_jump(&b, nir_jump_halt);
            nir_pop_if(&b, NULL);
            progress = true;
            break;
         }

         case nir_intrinsic_accept_ray_intersection: {
            b.cursor = nir_instr_remove(&intrin->instr);

            nir_def *terminate = nir_test_mask(&b, nir_load_ray_flags(&b),
               BRW_RT_RAY_FLAG_TERMINATE_ON_FIRST_HIT);
            nir_push_if(&b, terminate);
            {
               build_terminate_ray(&b);
            }
            nir_push_else(&b, NULL);
            {
               nir_trace_ray_intel(&b,
                                   nir_load_btd_global_arg_addr_intel(&b),
                                   nir_imm_int(&b, BRW_RT_BVH_LEVEL_OBJECT),
                                   nir_imm_int(&b, GEN_RT_TRACE_RAY_COMMIT),
                                   .synchronous = false);
               nir_jump(&b, nir_jump_halt);
            }
            nir_pop_if(&b, NULL);
            progress = true;
            break;
         }

         case nir_intrinsic_terminate_ray: {
            b.cursor = nir_instr_remove(&intrin->instr);
            build_terminate_ray(&b);
            progress = true;
            break;
         }

         default:
            break;
         }
      }
   }

   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_none);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }

   return progress;
}

void
brw_nir_lower_raygen(nir_shader *nir)
{
   assert(nir->info.stage == MESA_SHADER_RAYGEN);
   NIR_PASS_V(nir, brw_nir_lower_shader_returns);
   lower_rt_io_and_scratch(nir);
}

void
brw_nir_lower_any_hit(nir_shader *nir, const struct intel_device_info *devinfo)
{
   assert(nir->info.stage == MESA_SHADER_ANY_HIT);
   NIR_PASS_V(nir, brw_nir_lower_shader_returns);
   NIR_PASS_V(nir, lower_ray_walk_intrinsics, devinfo);
   lower_rt_io_and_scratch(nir);
}

void
brw_nir_lower_closest_hit(nir_shader *nir)
{
   assert(nir->info.stage == MESA_SHADER_CLOSEST_HIT);
   NIR_PASS_V(nir, brw_nir_lower_shader_returns);
   lower_rt_io_and_scratch(nir);
}

void
brw_nir_lower_miss(nir_shader *nir)
{
   assert(nir->info.stage == MESA_SHADER_MISS);
   NIR_PASS_V(nir, brw_nir_lower_shader_returns);
   lower_rt_io_and_scratch(nir);
}

void
brw_nir_lower_callable(nir_shader *nir)
{
   assert(nir->info.stage == MESA_SHADER_CALLABLE);
   NIR_PASS_V(nir, brw_nir_lower_shader_returns);
   lower_rt_io_and_scratch(nir);
}

void
brw_nir_lower_combined_intersection_any_hit(nir_shader *intersection,
                                            const nir_shader *any_hit,
                                            const struct intel_device_info *devinfo)
{
   assert(intersection->info.stage == MESA_SHADER_INTERSECTION);
   assert(any_hit == NULL || any_hit->info.stage == MESA_SHADER_ANY_HIT);
   NIR_PASS_V(intersection, brw_nir_lower_shader_returns);
   NIR_PASS_V(intersection, brw_nir_lower_intersection_shader,
              any_hit, devinfo);
   NIR_PASS_V(intersection, lower_ray_walk_intrinsics, devinfo);
   lower_rt_io_and_scratch(intersection);
}

static nir_def *
build_load_uniform(nir_builder *b, unsigned offset,
                   unsigned num_components, unsigned bit_size)
{
   return nir_load_uniform(b, num_components, bit_size, nir_imm_int(b, 0),
                           .base = offset,
                           .range = num_components * bit_size / 8);
}

#define load_trampoline_param(b, name, num_components, bit_size) \
   build_load_uniform((b), offsetof(struct brw_rt_raygen_trampoline_params, name), \
                      (num_components), (bit_size))

nir_shader *
brw_nir_create_raygen_trampoline(const struct brw_compiler *compiler,
                                 void *mem_ctx)
{
   const struct intel_device_info *devinfo = compiler->devinfo;
   const nir_shader_compiler_options *nir_options =
      compiler->nir_options[MESA_SHADER_COMPUTE];

   STATIC_ASSERT(sizeof(struct brw_rt_raygen_trampoline_params) == 32);

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE,
                                                  nir_options,
                                                  "RT Ray-Gen Trampoline");
   ralloc_steal(mem_ctx, b.shader);

   b.shader->info.workgroup_size_variable = true;

   /* The RT global data and raygen BINDLESS_SHADER_RECORD addresses are
    * passed in as push constants in the first register.  We deal with the
    * raygen BSR address here; the global data we'll deal with later.
    */
   b.shader->num_uniforms = 32;
   nir_def *raygen_param_bsr_addr =
      load_trampoline_param(&b, raygen_bsr_addr, 1, 64);
   nir_def *is_indirect =
      nir_i2b(&b, load_trampoline_param(&b, is_indirect, 1, 8));
   nir_def *local_shift =
      nir_u2u32(&b, load_trampoline_param(&b, local_group_size_log2, 3, 8));

   nir_def *raygen_indirect_bsr_addr;
   nir_push_if(&b, is_indirect);
   {
      raygen_indirect_bsr_addr =
         nir_load_global_constant(&b, raygen_param_bsr_addr,
                                  8 /* align */,
                                  1 /* components */,
                                  64 /* bit_size */);
   }
   nir_pop_if(&b, NULL);

   nir_def *raygen_bsr_addr =
      nir_if_phi(&b, raygen_indirect_bsr_addr, raygen_param_bsr_addr);

   nir_def *global_id = nir_load_workgroup_id_zero_base(&b);
   nir_def *simd_channel = nir_load_subgroup_invocation(&b);
   nir_def *local_x =
      nir_ubfe(&b, simd_channel, nir_imm_int(&b, 0),
                  nir_channel(&b, local_shift, 0));
   nir_def *local_y =
      nir_ubfe(&b, simd_channel, nir_channel(&b, local_shift, 0),
                  nir_channel(&b, local_shift, 1));
   nir_def *local_z =
      nir_ubfe(&b, simd_channel,
                  nir_iadd(&b, nir_channel(&b, local_shift, 0),
                              nir_channel(&b, local_shift, 1)),
                  nir_channel(&b, local_shift, 2));
   nir_def *launch_id =
      nir_iadd(&b, nir_ishl(&b, global_id, local_shift),
                  nir_vec3(&b, local_x, local_y, local_z));

   nir_def *launch_size = nir_load_ray_launch_size(&b);
   nir_push_if(&b, nir_ball(&b, nir_ult(&b, launch_id, launch_size)));
   {
      nir_store_global(&b, brw_nir_rt_sw_hotzone_addr(&b, devinfo), 16,
                       nir_vec4(&b, nir_imm_int(&b, 0), /* Stack ptr */
                                    nir_channel(&b, launch_id, 0),
                                    nir_channel(&b, launch_id, 1),
                                    nir_channel(&b, launch_id, 2)),
                       0xf /* write mask */);

      brw_nir_btd_spawn(&b, raygen_bsr_addr);
   }
   nir_push_else(&b, NULL);
   {
      /* Even though these invocations aren't being used for anything, the
       * hardware allocated stack IDs for them.  They need to retire them.
       */
      brw_nir_btd_retire(&b);
   }
   nir_pop_if(&b, NULL);

   nir_shader *nir = b.shader;
   nir->info.name = ralloc_strdup(nir, "RT: TraceRay trampoline");
   nir_validate_shader(nir, "in brw_nir_create_raygen_trampoline");

   struct brw_nir_compiler_opts opts = {};
   brw_preprocess_nir(compiler, nir, &opts);

   NIR_PASS_V(nir, brw_nir_lower_rt_intrinsics, devinfo);

   b = nir_builder_create(nir_shader_get_entrypoint(b.shader));
   /* brw_nir_lower_rt_intrinsics will leave us with a btd_global_arg_addr
    * intrinsic which doesn't exist in compute shaders.  We also created one
    * above when we generated the BTD spawn intrinsic.  Now we go through and
    * replace them with a uniform load.
    */
   nir_foreach_block(block, b.impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic != nir_intrinsic_load_btd_global_arg_addr_intel)
            continue;

         b.cursor = nir_before_instr(&intrin->instr);
         nir_def *global_arg_addr =
            load_trampoline_param(&b, rt_disp_globals_addr, 1, 64);
         nir_def_rewrite_uses(&intrin->def,
                                  global_arg_addr);
         nir_instr_remove(instr);
      }
   }

   NIR_PASS_V(nir, brw_nir_lower_cs_intrinsics);

   const bool is_scalar = true;
   brw_nir_optimize(nir, is_scalar, devinfo);

   return nir;
}
