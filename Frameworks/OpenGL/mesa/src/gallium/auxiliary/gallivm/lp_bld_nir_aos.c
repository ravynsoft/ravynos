/**************************************************************************
 *
 * Copyright 2022 Red Hat
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/

#include "lp_bld_nir.h"
#include "lp_bld_init.h"
#include "lp_bld_const.h"
#include "lp_bld_flow.h"
#include "lp_bld_struct.h"
#include "lp_bld_swizzle.h"
#include "lp_bld_debug.h"
#include "util/u_math.h"


struct lp_build_nir_aos_context
{
   struct lp_build_nir_context bld_base;

   /* Builder for integer masks and indices */
   struct lp_build_context int_bld;

   /*
    * AoS swizzle used:
    * - swizzles[0] = red index
    * - swizzles[1] = green index
    * - swizzles[2] = blue index
    * - swizzles[3] = alpha index
    */
   unsigned char swizzles[4];
   unsigned char inv_swizzles[4];

   LLVMValueRef consts_ptr;
   const LLVMValueRef *inputs;
   LLVMValueRef *outputs;

   const struct lp_build_sampler_aos *sampler;
};


static inline struct lp_build_nir_aos_context *
lp_nir_aos_context(struct lp_build_nir_context *bld_base)
{
   return (struct lp_build_nir_aos_context *) bld_base;
}


static LLVMValueRef
swizzle_aos(struct lp_build_nir_context *bld_base,
            LLVMValueRef a,
            unsigned swizzle_x,
            unsigned swizzle_y,
            unsigned swizzle_z,
            unsigned swizzle_w)
{
   unsigned char swizzles[4];
   struct lp_build_nir_aos_context *bld = lp_nir_aos_context(bld_base);

   assert(swizzle_x < 4);
   assert(swizzle_y < 4);
   assert(swizzle_z < 4);
   assert(swizzle_w < 4);

   swizzles[bld->inv_swizzles[0]] = bld->swizzles[swizzle_x];
   swizzles[bld->inv_swizzles[1]] = bld->swizzles[swizzle_y];
   swizzles[bld->inv_swizzles[2]] = bld->swizzles[swizzle_z];
   swizzles[bld->inv_swizzles[3]] = bld->swizzles[swizzle_w];

   return lp_build_swizzle_aos(&bld->bld_base.base, a, swizzles);
}


static void
init_var_slots(struct lp_build_nir_context *bld_base,
               nir_variable *var)
{
   struct lp_build_nir_aos_context *bld = lp_nir_aos_context(bld_base);

   if (!bld->outputs)
      return;
   unsigned this_loc = var->data.driver_location;

   bld->outputs[this_loc] = lp_build_alloca(bld_base->base.gallivm,
                                            bld_base->base.vec_type,
                                            "output");
}


static void
emit_var_decl(struct lp_build_nir_context *bld_base,
              nir_variable *var)
{
   if (var->data.mode == nir_var_shader_out) {
      init_var_slots(bld_base, var);
   }
}


static void
emit_load_var(struct lp_build_nir_context *bld_base,
              nir_variable_mode deref_mode,
              unsigned num_components,
              unsigned bit_size,
              nir_variable *var,
              unsigned vertex_index,
              LLVMValueRef indir_vertex_index,
              unsigned const_index,
              LLVMValueRef indir_index,
              LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   struct lp_build_nir_aos_context *bld = lp_nir_aos_context(bld_base);
   unsigned location = var->data.driver_location;

   if (deref_mode == nir_var_shader_in) {
      result[0] = bld->inputs[location];
   }
}


static void
emit_store_var(struct lp_build_nir_context *bld_base,
               nir_variable_mode deref_mode,
               unsigned num_components,
               unsigned bit_size,
               nir_variable *var,
               unsigned writemask,
               LLVMValueRef indir_vertex_index,
               unsigned const_index,
               LLVMValueRef indir_index,
               LLVMValueRef vals)
{
   struct lp_build_nir_aos_context *bld = lp_nir_aos_context(bld_base);
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   unsigned location = var->data.driver_location;

   if (deref_mode == nir_var_shader_out) {
      LLVMBuildStore(gallivm->builder, vals, bld->outputs[location]);
   }
}


static LLVMValueRef
emit_load_reg(struct lp_build_nir_context *bld_base,
              struct lp_build_context *reg_bld,
              const nir_intrinsic_instr *decl,
              unsigned base,
              LLVMValueRef indir_src,
              LLVMValueRef reg_storage)
{
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   assert(indir_src == NULL && "no indirects with linear path");
   return LLVMBuildLoad2(gallivm->builder, reg_bld->vec_type, reg_storage, "");
}


unsigned
lp_nir_aos_swizzle(struct lp_build_nir_context *bld_base, unsigned chan)
{
   struct lp_build_nir_aos_context *bld = lp_nir_aos_context(bld_base);
   return bld->swizzles[chan];
}


/*
 * If an instruction has a writemask like r0.x = foo and the
 * AOS/linear context uses swizzle={2,1,0,3} we need to change
 * the writemask to r0.z
 */
static unsigned
swizzle_writemask(struct lp_build_nir_aos_context *bld,
                  unsigned writemask)
{
   assert(writemask != 0x0);
   assert(writemask != 0xf);

   // Ex: swap r/b channels
   unsigned new_writemask = 0;
   for (unsigned chan = 0; chan < 4; chan++) {
      if (writemask & (1 << chan)) {
         new_writemask |= 1 << bld->swizzles[chan];
      }
   }
   return new_writemask;
}


static void
emit_store_reg(struct lp_build_nir_context *bld_base,
               struct lp_build_context *reg_bld,
               const nir_intrinsic_instr *decl,
               unsigned writemask,
               unsigned base,
               LLVMValueRef indir_src,
               LLVMValueRef reg_storage,
               LLVMValueRef vals[NIR_MAX_VEC_COMPONENTS])
{
   struct lp_build_nir_aos_context *bld = lp_nir_aos_context(bld_base);
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   assert(indir_src == NULL && "no indirects with linear path");

   if (writemask == 0xf) {
      LLVMBuildStore(gallivm->builder, vals[0], reg_storage);
      return;
   }

   writemask = swizzle_writemask(bld, writemask);

   LLVMValueRef cur = LLVMBuildLoad2(gallivm->builder, reg_bld->vec_type,
                                     reg_storage, "");
   LLVMTypeRef i32t = LLVMInt32TypeInContext(gallivm->context);
   LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH];
   for (unsigned j = 0; j < 16; j++) {
      unsigned comp = j % 4;
      if (writemask & (1 << comp)) {
         shuffles[j] = LLVMConstInt(i32t, 16 + j, 0); // new val
      } else {
         shuffles[j] = LLVMConstInt(i32t, j, 0);      // cur val
      }
   }
   cur = LLVMBuildShuffleVector(gallivm->builder, cur, vals[0],
                                LLVMConstVector(shuffles, 16), "");

   LLVMBuildStore(gallivm->builder, cur, reg_storage);
}


static void
emit_load_ubo(struct lp_build_nir_context *bld_base,
              unsigned nc,
              unsigned bit_size,
              bool offset_is_uniform,
              LLVMValueRef index,
              LLVMValueRef offset,
              LLVMValueRef result[NIR_MAX_VEC_COMPONENTS])
{
   struct lp_build_nir_aos_context *bld = lp_nir_aos_context(bld_base);
   LLVMBuilderRef builder = bld_base->base.gallivm->builder;
   struct gallivm_state *gallivm = bld_base->base.gallivm;
   struct lp_type type = bld_base->base.type;
   LLVMValueRef res;

   res = bld->bld_base.base.undef;
   offset = LLVMBuildExtractElement(builder, offset,
                                    lp_build_const_int32(gallivm, 0), "");
   assert(LLVMIsConstant(offset));
   unsigned offset_val = LLVMConstIntGetZExtValue(offset) >> 2;
   for (unsigned chan = 0; chan < nc; ++chan) {
      LLVMValueRef this_offset = lp_build_const_int32(gallivm,
                                                      offset_val + chan);

      LLVMTypeRef scalar_type = LLVMInt8TypeInContext(gallivm->context);
      LLVMValueRef scalar_ptr = LLVMBuildGEP2(builder, scalar_type, bld->consts_ptr, &this_offset, 1, "");
      LLVMValueRef scalar = LLVMBuildLoad2(builder, scalar_type, scalar_ptr, "");

      lp_build_name(scalar, "const[%u].%c", offset_val, "xyzw"[chan]);

      LLVMValueRef swizzle = lp_build_const_int32(bld->bld_base.base.gallivm,
                                           nc == 1 ? 0 : bld->swizzles[chan]);

      res = LLVMBuildInsertElement(builder, res, scalar, swizzle, "");
   }

   if (type.length > 4) {
      LLVMValueRef shuffles[LP_MAX_VECTOR_LENGTH];

      for (unsigned chan = 0; chan < nc; ++chan) {
         shuffles[chan] =
            lp_build_const_int32(bld->bld_base.base.gallivm, chan);
      }

      for (unsigned i = nc; i < type.length; ++i) {
         shuffles[i] = shuffles[i % nc];
      }

      res = LLVMBuildShuffleVector(builder, res, bld->bld_base.base.undef,
                                   LLVMConstVector(shuffles, type.length),
                                   "");
   }

   if (nc == 4)
      swizzle_aos(bld_base, res, 0, 1, 2, 3);

   result[0] = res;
}


static void
emit_tex(struct lp_build_nir_context *bld_base,
         struct lp_sampler_params *params)
{
   struct lp_build_nir_aos_context *bld = lp_nir_aos_context(bld_base);
   static const struct lp_derivatives derivs = { 0 };
   params->type = bld_base->base.type;
   params->texel[0] = bld->sampler->emit_fetch_texel(bld->sampler,
                                                     &bld->bld_base.base,
                                                     TGSI_TEXTURE_2D,
                                                     params->texture_index,
                                                     params->coords[0],
                                                     params->derivs ? params->derivs[0] : derivs,
                                                     LP_BLD_TEX_MODIFIER_NONE);
}


static void
emit_load_const(struct lp_build_nir_context *bld_base,
                const nir_load_const_instr *instr,
                LLVMValueRef outval[NIR_MAX_VEC_COMPONENTS])
{
   struct lp_build_nir_aos_context *bld = lp_nir_aos_context(bld_base);
   LLVMValueRef elems[16];
   const int nc = instr->def.num_components;
   bool do_swizzle = false;

   if (nc == 4)
      do_swizzle = true;

   /* The constant is something like {float, float, float, float}.
    * We need to convert the float values from [0,1] to ubyte in [0,255].
    * We previously checked for values outside [0,1] in
    * llvmpipe_nir_fn_is_linear_compat().
    * Also, we convert the (typically) 4-element float constant into a
    * swizzled 16-element ubyte constant (z,y,x,w, z,y,x,w, z,y,x,w, z,y,x,w)
    * since that's what 'linear' mode operates on.
    */
   assert(bld_base->base.type.length <= ARRAY_SIZE(elems));
   for (unsigned i = 0; i < bld_base->base.type.length; i++) {
      const unsigned j = do_swizzle ? bld->swizzles[i % nc] : i % nc;
      assert(instr->value[j].f32 >= 0.0f);
      assert(instr->value[j].f32 <= 1.0f);
      const unsigned u8val = float_to_ubyte(instr->value[j].f32);
      elems[i] = LLVMConstInt(bld_base->uint_bld.int_elem_type, u8val, 0);
   }
   outval[0] = LLVMConstVector(elems, bld_base->base.type.length);
   outval[1] = outval[2] = outval[3] = NULL;
}


void
lp_build_nir_aos(struct gallivm_state *gallivm,
                 struct nir_shader *shader,
                 struct lp_type type,
                 const unsigned char swizzles[4],
                 LLVMValueRef consts_ptr,
                 const LLVMValueRef *inputs,
                 LLVMValueRef *outputs,
                 const struct lp_build_sampler_aos *sampler)
{
   struct lp_build_nir_aos_context bld;

   memset(&bld, 0, sizeof bld);
   lp_build_context_init(&bld.bld_base.base, gallivm, type);
   lp_build_context_init(&bld.bld_base.uint_bld, gallivm, lp_uint_type(type));
   lp_build_context_init(&bld.bld_base.int_bld, gallivm, lp_int_type(type));

   for (unsigned chan = 0; chan < 4; ++chan) {
      bld.swizzles[chan] = swizzles[chan];
      bld.inv_swizzles[swizzles[chan]] = chan;
   }
   bld.sampler = sampler;

   bld.bld_base.shader = shader;

   bld.inputs = inputs;
   bld.outputs = outputs;
   bld.consts_ptr = consts_ptr;

   bld.bld_base.load_var = emit_load_var;
   bld.bld_base.store_var = emit_store_var;
   bld.bld_base.load_reg = emit_load_reg;
   bld.bld_base.store_reg = emit_store_reg;
   bld.bld_base.load_ubo = emit_load_ubo;
   bld.bld_base.load_const = emit_load_const;

   bld.bld_base.tex = emit_tex;
   bld.bld_base.emit_var_decl = emit_var_decl;

   lp_build_nir_prepasses(shader);
   NIR_PASS_V(shader, nir_move_vec_src_uses_to_dest, false);
   NIR_PASS_V(shader, nir_lower_vec_to_regs, NULL, NULL);
   lp_build_nir_llvm(&bld.bld_base, shader,
                     nir_shader_get_entrypoint(shader));
}
