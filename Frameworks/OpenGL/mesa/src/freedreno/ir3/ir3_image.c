/*
 * Copyright (C) 2017-2018 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "ir3_image.h"

/*
 * SSBO/Image to/from IBO/tex hw mapping table:
 */

void
ir3_ibo_mapping_init(struct ir3_ibo_mapping *mapping, unsigned num_textures)
{
   memset(mapping, IBO_INVALID, sizeof(*mapping));
   mapping->num_tex = 0;
   mapping->tex_base = num_textures;
}

struct ir3_instruction *
ir3_ssbo_to_ibo(struct ir3_context *ctx, nir_src src)
{
   if (ir3_bindless_resource(src))
      ctx->so->bindless_ibo = true;
   return ir3_get_src(ctx, &src)[0];
}

unsigned
ir3_ssbo_to_tex(struct ir3_ibo_mapping *mapping, unsigned ssbo)
{
   if (mapping->ssbo_to_tex[ssbo] == IBO_INVALID) {
      unsigned tex = mapping->num_tex++;
      mapping->ssbo_to_tex[ssbo] = tex;
      mapping->tex_to_image[tex] = IBO_SSBO | ssbo;
   }
   return mapping->ssbo_to_tex[ssbo] + mapping->tex_base;
}

struct ir3_instruction *
ir3_image_to_ibo(struct ir3_context *ctx, nir_src src)
{
   if (ir3_bindless_resource(src)) {
      ctx->so->bindless_ibo = true;
      return ir3_get_src(ctx, &src)[0];
   }

   if (nir_src_is_const(src)) {
      int image_idx = nir_src_as_uint(src);
      return create_immed(ctx->block, ctx->s->info.num_ssbos + image_idx);
   } else {
      struct ir3_instruction *image_idx = ir3_get_src(ctx, &src)[0];
      if (ctx->s->info.num_ssbos) {
         return ir3_ADD_U(ctx->block,
            image_idx, 0,
            create_immed(ctx->block, ctx->s->info.num_ssbos), 0);
      } else {
         return image_idx;
      }
   }
}

unsigned
ir3_image_to_tex(struct ir3_ibo_mapping *mapping, unsigned image)
{
   if (mapping->image_to_tex[image] == IBO_INVALID) {
      unsigned tex = mapping->num_tex++;
      mapping->image_to_tex[image] = tex;
      mapping->tex_to_image[tex] = image;
   }
   return mapping->image_to_tex[image] + mapping->tex_base;
}

/* see tex_info() for equiv logic for texture instructions.. it would be
 * nice if this could be better unified..
 */
unsigned
ir3_get_image_coords(const nir_intrinsic_instr *instr, unsigned *flagsp)
{
   enum glsl_sampler_dim dim = nir_intrinsic_image_dim(instr);
   unsigned coords = nir_image_intrinsic_coord_components(instr);
   unsigned flags = 0;

   if (dim == GLSL_SAMPLER_DIM_CUBE || nir_intrinsic_image_array(instr))
      flags |= IR3_INSTR_A;
   else if (dim == GLSL_SAMPLER_DIM_3D)
      flags |= IR3_INSTR_3D;

   if (flagsp)
      *flagsp = flags;

   return coords;
}

type_t
ir3_get_type_for_image_intrinsic(const nir_intrinsic_instr *instr)
{
   const nir_intrinsic_info *info = &nir_intrinsic_infos[instr->intrinsic];
   int bit_size = info->has_dest ? instr->def.bit_size : nir_src_bit_size(instr->src[3]);

   nir_alu_type type = nir_type_uint;
   switch (instr->intrinsic) {
   case nir_intrinsic_image_load:
   case nir_intrinsic_bindless_image_load:
      type = nir_alu_type_get_base_type(nir_intrinsic_dest_type(instr));
      /* SpvOpAtomicLoad doesn't have dest type */
      if (type == nir_type_invalid)
         type = nir_type_uint;
      break;

   case nir_intrinsic_image_store:
   case nir_intrinsic_bindless_image_store:
      type = nir_alu_type_get_base_type(nir_intrinsic_src_type(instr));
      /* SpvOpAtomicStore doesn't have src type */
      if (type == nir_type_invalid)
         type = nir_type_uint;
      break;

   case nir_intrinsic_image_atomic:
   case nir_intrinsic_bindless_image_atomic:
   case nir_intrinsic_image_atomic_swap:
   case nir_intrinsic_bindless_image_atomic_swap:
      type = nir_atomic_op_type(nir_intrinsic_atomic_op(instr));
      break;

   default:
      unreachable("Unhandled NIR image intrinsic");
   }

   switch (type) {
   case nir_type_uint:
      return bit_size == 16 ? TYPE_U16 : TYPE_U32;
   case nir_type_int:
      return bit_size == 16 ? TYPE_S16 : TYPE_S32;
   case nir_type_float:
      return bit_size == 16 ? TYPE_F16 : TYPE_F32;
   default:
      unreachable("bad type");
   }
}

/* Returns the number of components for the different image formats
 * supported by the GLES 3.1 spec, plus those added by the
 * GL_NV_image_formats extension.
 */
unsigned
ir3_get_num_components_for_image_format(enum pipe_format format)
{
   if (format == PIPE_FORMAT_NONE)
      return 4;
   else
      return util_format_get_nr_components(format);
}
