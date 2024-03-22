/*
 * Copyright (c) 2016 Intel Corporation
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

#include "brw_nir.h"
#include "compiler/nir/nir_builder.h"

struct lower_intrinsics_state {
   nir_shader *nir;
   nir_function_impl *impl;
   bool progress;
   nir_builder builder;
};

static void
compute_local_index_id(nir_builder *b,
                       nir_shader *nir,
                       nir_def **local_index,
                       nir_def **local_id)
{
   nir_def *subgroup_id = nir_load_subgroup_id(b);

   nir_def *thread_local_id =
      nir_imul(b, subgroup_id, nir_load_simd_width_intel(b));
   nir_def *channel = nir_load_subgroup_invocation(b);
   nir_def *linear = nir_iadd(b, channel, thread_local_id);

   nir_def *size_x;
   nir_def *size_y;
   if (nir->info.workgroup_size_variable) {
      nir_def *size_xyz = nir_load_workgroup_size(b);
      size_x = nir_channel(b, size_xyz, 0);
      size_y = nir_channel(b, size_xyz, 1);
   } else {
      size_x = nir_imm_int(b, nir->info.workgroup_size[0]);
      size_y = nir_imm_int(b, nir->info.workgroup_size[1]);
   }
   nir_def *size_xy = nir_imul(b, size_x, size_y);

   /* The local invocation index and ID must respect the following
    *
    *    gl_LocalInvocationID.x =
    *       gl_LocalInvocationIndex % gl_WorkGroupSize.x;
    *    gl_LocalInvocationID.y =
    *       (gl_LocalInvocationIndex / gl_WorkGroupSize.x) %
    *       gl_WorkGroupSize.y;
    *    gl_LocalInvocationID.z =
    *       (gl_LocalInvocationIndex /
    *        (gl_WorkGroupSize.x * gl_WorkGroupSize.y)) %
    *       gl_WorkGroupSize.z;
    *
    * However, the final % gl_WorkGroupSize.z does nothing unless we
    * accidentally end up with a gl_LocalInvocationIndex that is too
    * large so it can safely be omitted.
    */

   nir_def *id_x, *id_y, *id_z;
   switch (nir->info.cs.derivative_group) {
   case DERIVATIVE_GROUP_NONE:
      if (nir->info.num_images == 0 &&
          nir->info.num_textures == 0) {
         /* X-major lid order. Optimal for linear accesses only,
          * which are usually buffers. X,Y ordering will look like:
          * (0,0) (1,0) (2,0) ... (size_x-1,0) (0,1) (1,1) ...
          */
         id_x = nir_umod(b, linear, size_x);
         id_y = nir_umod(b, nir_udiv(b, linear, size_x), size_y);
         *local_index = linear;
      } else if (!nir->info.workgroup_size_variable &&
                 nir->info.workgroup_size[1] % 4 == 0) {
         /* 1x4 block X-major lid order. Same as X-major except increments in
          * blocks of width=1 height=4. Always optimal for tileY and usually
          * optimal for linear accesses.
          *   x = (linear / 4) % size_x
          *   y = ((linear % 4) + (linear / 4 / size_x) * 4) % size_y
          * X,Y ordering will look like: (0,0) (0,1) (0,2) (0,3) (1,0) (1,1)
          * (1,2) (1,3) (2,0) ... (size_x-1,3) (0,4) (0,5) (0,6) (0,7) (1,4) ...
          */
         const unsigned height = 4;
         nir_def *block = nir_udiv_imm(b, linear, height);
         id_x = nir_umod(b, block, size_x);
         id_y = nir_umod(b,
                         nir_iadd(b,
                                  nir_umod_imm(b, linear, height),
                                  nir_imul_imm(b,
                                               nir_udiv(b, block, size_x),
                                               height)),
                         size_y);
      } else {
         /* Y-major lid order. Optimal for tileY accesses only,
          * which are usually images. X,Y ordering will look like:
          * (0,0) (0,1) (0,2) ... (0,size_y-1) (1,0) (1,1) ...
          */
         id_y = nir_umod(b, linear, size_y);
         id_x = nir_umod(b, nir_udiv(b, linear, size_y), size_x);
      }

      id_z = nir_udiv(b, linear, size_xy);
      *local_id = nir_vec3(b, id_x, id_y, id_z);
      if (!*local_index) {
         *local_index = nir_iadd(b, nir_iadd(b, id_x,
                                                nir_imul(b, id_y, size_x)),
                                                nir_imul(b, id_z, size_xy));
      }
      break;
   case DERIVATIVE_GROUP_LINEAR:
      /* For linear, just set the local invocation index linearly,
       * and calculate local invocation ID from that.
       */
      id_x = nir_umod(b, linear, size_x);
      id_y = nir_umod(b, nir_udiv(b, linear, size_x), size_y);
      id_z = nir_udiv(b, linear, size_xy);
      *local_id = nir_vec3(b, id_x, id_y, id_z);
      *local_index = linear;
      break;
   case DERIVATIVE_GROUP_QUADS: {
      /* For quads, first we figure out the 2x2 grid the invocation
       * belongs to -- treating extra Z layers as just more rows.
       * Then map that into local invocation ID (trivial) and local
       * invocation index.  Skipping Z simplify index calculation.
       */

      nir_def *one = nir_imm_int(b, 1);
      nir_def *double_size_x = nir_ishl(b, size_x, one);

      /* ID within a pair of rows, where each group of 4 is 2x2 quad. */
      nir_def *row_pair_id = nir_umod(b, linear, double_size_x);
      nir_def *y_row_pairs = nir_udiv(b, linear, double_size_x);

      nir_def *x =
         nir_ior(b,
                 nir_iand(b, row_pair_id, one),
                 nir_iand(b, nir_ishr(b, row_pair_id, one),
                          nir_imm_int(b, 0xfffffffe)));
      nir_def *y =
         nir_ior(b,
                 nir_ishl(b, y_row_pairs, one),
                 nir_iand(b, nir_ishr(b, row_pair_id, one), one));

      *local_id = nir_vec3(b, x,
                           nir_umod(b, y, size_y),
                           nir_udiv(b, y, size_y));
      *local_index = nir_iadd(b, x, nir_imul(b, y, size_x));
      break;
   }
   default:
      unreachable("invalid derivative group");
   }
}

static bool
lower_cs_intrinsics_convert_block(struct lower_intrinsics_state *state,
                                  nir_block *block)
{
   bool progress = false;
   nir_builder *b = &state->builder;
   nir_shader *nir = state->nir;

   /* Reuse calculated values inside the block. */
   nir_def *local_index = NULL;
   nir_def *local_id = NULL;

   nir_foreach_instr_safe(instr, block) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *intrinsic = nir_instr_as_intrinsic(instr);

      b->cursor = nir_after_instr(&intrinsic->instr);

      nir_def *sysval;
      switch (intrinsic->intrinsic) {
      case nir_intrinsic_load_local_invocation_index:
      case nir_intrinsic_load_local_invocation_id: {
         if (!local_index && !nir->info.workgroup_size_variable) {
            const uint16_t *ws = nir->info.workgroup_size;
            if (ws[0] * ws[1] * ws[2] == 1) {
               nir_def *zero = nir_imm_int(b, 0);
               local_index = zero;
               local_id = nir_replicate(b, zero, 3);
            }
         }

         if (!local_index) {
            if (nir->info.stage == MESA_SHADER_TASK ||
                nir->info.stage == MESA_SHADER_MESH) {
               /* Will be lowered by nir_emit_task_mesh_intrinsic() using
                * information from the payload.
                */
               continue;
            }

            /* First time we are using those, so let's calculate them. */
            assert(!local_id);
            compute_local_index_id(b, nir, &local_index, &local_id);
         }

         assert(local_id);
         assert(local_index);
         if (intrinsic->intrinsic == nir_intrinsic_load_local_invocation_id)
            sysval = local_id;
         else
            sysval = local_index;
         break;
      }

      case nir_intrinsic_load_num_subgroups: {
         nir_def *size;
         if (state->nir->info.workgroup_size_variable) {
            nir_def *size_xyz = nir_load_workgroup_size(b);
            nir_def *size_x = nir_channel(b, size_xyz, 0);
            nir_def *size_y = nir_channel(b, size_xyz, 1);
            nir_def *size_z = nir_channel(b, size_xyz, 2);
            size = nir_imul(b, nir_imul(b, size_x, size_y), size_z);
         } else {
            size = nir_imm_int(b, nir->info.workgroup_size[0] *
                                  nir->info.workgroup_size[1] *
                                  nir->info.workgroup_size[2]);
         }

         /* Calculate the equivalent of DIV_ROUND_UP. */
         nir_def *simd_width = nir_load_simd_width_intel(b);
         sysval =
            nir_udiv(b, nir_iadd_imm(b, nir_iadd(b, size, simd_width), -1),
                        simd_width);
         break;
      }

      default:
         continue;
      }

      if (intrinsic->def.bit_size == 64)
         sysval = nir_u2u64(b, sysval);

      nir_def_rewrite_uses(&intrinsic->def, sysval);
      nir_instr_remove(&intrinsic->instr);

      state->progress = true;
   }

   return progress;
}

static void
lower_cs_intrinsics_convert_impl(struct lower_intrinsics_state *state)
{
   state->builder = nir_builder_create(state->impl);

   nir_foreach_block(block, state->impl) {
      lower_cs_intrinsics_convert_block(state, block);
   }

   nir_metadata_preserve(state->impl,
                         nir_metadata_block_index | nir_metadata_dominance);
}

bool
brw_nir_lower_cs_intrinsics(nir_shader *nir)
{
   assert(gl_shader_stage_uses_workgroup(nir->info.stage));

   struct lower_intrinsics_state state = {
      .nir = nir,
   };

   /* Constraints from NV_compute_shader_derivatives. */
   if (gl_shader_stage_is_compute(nir->info.stage) &&
       !nir->info.workgroup_size_variable) {
      if (nir->info.cs.derivative_group == DERIVATIVE_GROUP_QUADS) {
         assert(nir->info.workgroup_size[0] % 2 == 0);
         assert(nir->info.workgroup_size[1] % 2 == 0);
      } else if (nir->info.cs.derivative_group == DERIVATIVE_GROUP_LINEAR) {
         ASSERTED unsigned workgroup_size =
            nir->info.workgroup_size[0] *
            nir->info.workgroup_size[1] *
            nir->info.workgroup_size[2];
         assert(workgroup_size % 4 == 0);
      }
   }

   nir_foreach_function_impl(impl, nir) {
      state.impl = impl;
      lower_cs_intrinsics_convert_impl(&state);
   }

   return state.progress;
}
