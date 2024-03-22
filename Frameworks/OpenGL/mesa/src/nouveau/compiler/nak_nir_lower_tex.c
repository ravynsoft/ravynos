/*
 * Copyright © 2023 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "nak_private.h"
#include "nir_builder.h"
#include "nir_format_convert.h"

#include "util/u_math.h"

static enum glsl_sampler_dim
remap_sampler_dim(enum glsl_sampler_dim dim)
{
   switch (dim) {
   case GLSL_SAMPLER_DIM_SUBPASS: return GLSL_SAMPLER_DIM_2D;
   case GLSL_SAMPLER_DIM_SUBPASS_MS: return GLSL_SAMPLER_DIM_MS;
   default: return dim;
   }
}

static bool
lower_tex(nir_builder *b, nir_tex_instr *tex, const struct nak_compiler *nak)
{
   b->cursor = nir_before_instr(&tex->instr);

   nir_def *tex_h = NULL, *samp_h = NULL, *coord = NULL, *ms_idx = NULL;
   nir_def *offset = NULL, *lod = NULL, *bias = NULL, *min_lod = NULL;
   nir_def *ddx = NULL, *ddy = NULL, *z_cmpr = NULL;
   for (unsigned i = 0; i < tex->num_srcs; i++) {
      switch (tex->src[i].src_type) {
      case nir_tex_src_texture_handle: tex_h =     tex->src[i].src.ssa; break;
      case nir_tex_src_sampler_handle: samp_h =    tex->src[i].src.ssa; break;
      case nir_tex_src_coord:          coord =     tex->src[i].src.ssa; break;
      case nir_tex_src_ms_index:       ms_idx =    tex->src[i].src.ssa; break;
      case nir_tex_src_comparator:     z_cmpr =    tex->src[i].src.ssa; break;
      case nir_tex_src_offset:         offset =    tex->src[i].src.ssa; break;
      case nir_tex_src_lod:            lod =       tex->src[i].src.ssa; break;
      case nir_tex_src_bias:           bias =      tex->src[i].src.ssa; break;
      case nir_tex_src_min_lod:        min_lod =   tex->src[i].src.ssa; break;
      case nir_tex_src_ddx:            ddx =       tex->src[i].src.ssa; break;
      case nir_tex_src_ddy:            ddy =       tex->src[i].src.ssa; break;
      default:
         unreachable("Unsupported texture source");
      }
   }

   /* Combine sampler and texture into one if needed */
   if (samp_h != NULL && samp_h != tex_h) {
      tex_h = nir_ior(b, nir_iand_imm(b, tex_h,  0x000fffff),
                         nir_iand_imm(b, samp_h, 0xfff00000));
   }
   tex_h = nir_u2u32(b, tex_h);

   /* Array index is treated separately, so pull it off if we have one. */
   nir_def *arr_idx = NULL;
   unsigned coord_components = tex->coord_components;
   if (coord && tex->is_array) {
      if (tex->op == nir_texop_lod) {
         /* The HW wants an array index. Use zero. */
         arr_idx = nir_imm_int(b, 0);
      } else {
         arr_idx = nir_channel(b, coord, --coord_components);

         /* Everything but texelFetch takes a float index
          *
          * TODO: Use F2I.U32.RNE
          */
         if (tex->op != nir_texop_txf && tex->op != nir_texop_txf_ms) {
            arr_idx = nir_fadd_imm(b, arr_idx, 0.5);

            // TODO: Hardware seems to clamp negative values to zero for us
            // in f2u, but we still need this fmax for constant folding.
            arr_idx = nir_fmax(b, arr_idx, nir_imm_float(b, 0.0));

            arr_idx = nir_f2u32(b, arr_idx);
         }

         arr_idx = nir_umin(b, arr_idx, nir_imm_int(b, UINT16_MAX));
      }
   }

   enum nak_nir_lod_mode lod_mode = NAK_NIR_LOD_MODE_AUTO;
   if (tex->op == nir_texop_txf_ms) {
      /* Multisampled textures do not have miplevels */
      lod_mode = NAK_NIR_LOD_MODE_ZERO;
      lod = NULL; /* We don't need this */
   } else if (lod != NULL) {
      nir_scalar lod_s = { .def = lod, .comp = 0 };
      if (nir_scalar_is_const(lod_s) &&
          nir_scalar_as_uint(lod_s) == 0) {
         lod_mode = NAK_NIR_LOD_MODE_ZERO;
         lod = NULL; /* We don't need this */
      } else {
         lod_mode = NAK_NIR_LOD_MODE_LOD;
      }
   } else if (bias != NULL) {
      lod_mode = NAK_NIR_LOD_MODE_BIAS;
      lod = bias;
   }

   if (min_lod != NULL) {
      switch (lod_mode) {
      case NAK_NIR_LOD_MODE_AUTO:
         lod_mode = NAK_NIR_LOD_MODE_CLAMP;
         break;
      case NAK_NIR_LOD_MODE_BIAS:
         lod_mode = NAK_NIR_LOD_MODE_BIAS_CLAMP;
         break;
      default:
         unreachable("Invalid min_lod");
      }
      min_lod = nir_f2u32(b, nir_fmax(b, nir_fmul_imm(b, min_lod, 256),
                                         nir_imm_float(b, 16)));
   }

   enum nak_nir_offset_mode offset_mode = NAK_NIR_OFFSET_MODE_NONE;
   if (offset != NULL) {
      /* For TG4, offsets, are packed into a single 32-bit value with 8 bits
       * per component.  For all other texture instructions, offsets are
       * packed into a single at most 16-bit value with 8 bits per component.
       */
      static const unsigned bits4[] = { 4, 4, 4, 4 };
      static const unsigned bits8[] = { 8, 8, 8, 8 };
      const unsigned *bits = tex->op == nir_texop_tg4 ? bits8 : bits4;

      offset = nir_pad_vector_imm_int(b, offset, 0, 4);
      offset = nir_format_clamp_sint(b, offset, bits);
      offset = nir_format_pack_uint(b, offset, bits, 4);
      offset_mode = NAK_NIR_OFFSET_MODE_AOFFI;
   } else if (nir_tex_instr_has_explicit_tg4_offsets(tex)) {
      uint64_t off_u64 = 0;
      for (uint8_t i = 0; i < 8; ++i) {
         uint64_t off = (uint8_t)tex->tg4_offsets[i / 2][i % 2];
         off_u64 |= off << (i * 8);
      }
      offset = nir_imm_ivec2(b, off_u64, off_u64 >> 32);
      offset_mode = NAK_NIR_OFFSET_MODE_PER_PX;
   }

   nir_def *src0[4] = { NULL, };
   nir_def *src1[4] = { NULL, };
   unsigned src0_comps = 0, src1_comps = 0;

#define PUSH(a, x) do { \
   nir_def *val = (x); \
   assert(a##_comps < ARRAY_SIZE(a)); \
   a[a##_comps++] = val; \
} while(0)

   if (nak->sm >= 50) {
      if (tex->op == nir_texop_txd) {
         PUSH(src0, tex_h);

         for (uint32_t i = 0; i < coord_components; i++)
            PUSH(src0, nir_channel(b, coord, i));

         if (offset != NULL) {
            nir_def *arr_off = nir_ishl_imm(b, offset, 16);
            if (arr_idx)
               arr_off = nir_ior(b, arr_off, arr_idx);
            PUSH(src0, arr_off);
         } else if (arr_idx != NULL) {
            PUSH(src0, arr_idx);
         }

         assert(ddx->num_components == coord_components);
         for (uint32_t i = 0; i < coord_components; i++) {
            PUSH(src1, nir_channel(b, ddx, i));
            PUSH(src1, nir_channel(b, ddy, i));
         }
      } else {
         if (min_lod != NULL) {
            nir_def *arr_ml = nir_ishl_imm(b, min_lod, 16);
            if (arr_idx)
               arr_ml = nir_ior(b, arr_ml, arr_idx);
            PUSH(src0, arr_ml);
         } else if (arr_idx != NULL) {
            PUSH(src0, arr_idx);
         }

         for (uint32_t i = 0; i < coord_components; i++)
            PUSH(src0, nir_channel(b, coord, i));

         PUSH(src1, tex_h);
         if (ms_idx != NULL)
            PUSH(src1, ms_idx);
         if (lod != NULL)
            PUSH(src1, lod);
         if (offset_mode == NAK_NIR_OFFSET_MODE_AOFFI) {
            PUSH(src1, offset);
         } else if (offset_mode == NAK_NIR_OFFSET_MODE_PER_PX) {
            PUSH(src1, nir_channel(b, offset, 0));
            PUSH(src1, nir_channel(b, offset, 1));
         }
         if (z_cmpr != NULL)
            PUSH(src1, z_cmpr);
      }
   } else {
      unreachable("Unsupported shader model");
   }

   nir_def *vec_srcs[2] = {
      nir_vec(b, src0, src0_comps),
      nir_vec(b, src1, src1_comps),
   };

   tex->src[0].src_type = nir_tex_src_backend1;
   nir_src_rewrite(&tex->src[0].src, vec_srcs[0]);

   tex->src[1].src_type = nir_tex_src_backend2;
   nir_src_rewrite(&tex->src[1].src, vec_srcs[1]);

   /* Remove any extras */
   while (tex->num_srcs > 2)
      nir_tex_instr_remove_src(tex, tex->num_srcs - 1);

   tex->sampler_dim = remap_sampler_dim(tex->sampler_dim);

   struct nak_nir_tex_flags flags = {
      .lod_mode = lod_mode,
      .offset_mode = offset_mode,
      .has_z_cmpr = tex->is_shadow,
   };
   STATIC_ASSERT(sizeof(flags) == sizeof(tex->backend_flags));
   memcpy(&tex->backend_flags, &flags, sizeof(flags));

   if (tex->op == nir_texop_lod) {
      b->cursor = nir_after_instr(&tex->instr);

      /* The outputs are flipped compared to what NIR expects */
      nir_def *abs = nir_channel(b, &tex->def, 1);
      nir_def *rel = nir_channel(b, &tex->def, 0);

      /* The returned values are not quite what we want:
       * (a) convert from s16/u16 to f32
       * (b) multiply by 1/256
       *
       * TODO: We can make this cheaper once we have 16-bit in NAK
       */
      abs = nir_u2f32(b, nir_iand_imm(b, abs, 0xffff));
      nir_def *shift = nir_imm_int(b, 16);
      rel = nir_i2f32(b, nir_ishr(b, nir_ishl(b, rel, shift), shift));

      abs = nir_fmul_imm(b, abs, 1.0 / 256.0);
      rel = nir_fmul_imm(b, rel, 1.0 / 256.0);

      nir_def *res = nir_vec2(b, abs, rel);
      nir_def_rewrite_uses_after(&tex->def, res, res->parent_instr);
   }

   return true;
}

static bool
lower_txq(nir_builder *b, nir_tex_instr *tex, const struct nak_compiler *nak)
{
   b->cursor = nir_before_instr(&tex->instr);

   nir_def *tex_h = NULL, *lod = NULL;
   for (unsigned i = 0; i < tex->num_srcs; i++) {
      switch (tex->src[i].src_type) {
      case nir_tex_src_texture_handle: tex_h = tex->src[i].src.ssa; break;
      case nir_tex_src_sampler_handle: break; /* Ignored */
      case nir_tex_src_lod:            lod = tex->src[i].src.ssa; break;
      default:
         unreachable("Unsupported texture source");
      }
   }

   /* TODO: We should only support 32-bit handles */
   tex_h = nir_u2u32(b, tex_h);

   nir_def *txq_src;
   nir_component_mask_t mask;
   switch (tex->op) {
   case nir_texop_txs:
      tex->op = nir_texop_hdr_dim_nv;
      if (lod == NULL)
         lod = nir_imm_int(b, 0);
      txq_src = nir_vec2(b, tex_h, lod);
      mask = BITSET_MASK(tex->def.num_components);
      break;
   case nir_texop_query_levels:
      tex->op = nir_texop_hdr_dim_nv;
      txq_src = nir_vec2(b, tex_h, nir_imm_int(b, 0));
      mask = BITSET_BIT(3);
      break;
   case nir_texop_texture_samples:
      tex->op = nir_texop_tex_type_nv;
      txq_src = tex_h;
      mask = BITSET_BIT(2);
      break;
   default:
      unreachable("Invalid texture query op");
   }

   tex->src[0].src_type = nir_tex_src_backend1;
   nir_src_rewrite(&tex->src[0].src, txq_src);

   /* Remove any extras */
   while (tex->num_srcs > 1)
      nir_tex_instr_remove_src(tex, tex->num_srcs - 1);

   tex->sampler_dim = remap_sampler_dim(tex->sampler_dim);

   b->cursor = nir_after_instr(&tex->instr);

   /* Only pick off slected components */
   tex->def.num_components = 4;
   nir_def *res = nir_channels(b, &tex->def, mask);
   nir_def_rewrite_uses_after(&tex->def, res, res->parent_instr);

   return true;
}

static bool
shrink_image_load(nir_builder *b, nir_intrinsic_instr *intrin,
                  const struct nak_compiler *nak)
{
   enum pipe_format format = nir_intrinsic_format(intrin);
   nir_component_mask_t comps_read = nir_def_components_read(&intrin->def);

   if (intrin->def.bit_size == 64) {
      assert(format == PIPE_FORMAT_NONE ||
             format == PIPE_FORMAT_R64_UINT ||
             format == PIPE_FORMAT_R64_SINT);

      b->cursor = nir_after_instr(&intrin->instr);

      nir_def *data_xy, *data_w;
      if (comps_read & BITFIELD_BIT(3)) {
         /* Thanks to descriptor indexing, we need to ensure that null
          * descriptor behavior works properly.  In particular, normal zero
          * reads will return (0, 0, 0, 1) whereas null descriptor reads need
          * to return (0, 0, 0, 0).  This means we can't blindly extend with
          * an alpha component of 1.  Instead, we need to trust the hardware
          * to extend the original RG32 with z = 0 and w = 1 and copy the w
          * value all the way out to 64-bit w value.
          */
         assert(intrin->num_components == 4);
         assert(intrin->def.num_components == 4);
         intrin->def.bit_size = 32;

         data_xy = nir_channels(b, &intrin->def, 0x3);
         data_w = nir_channels(b, &intrin->def, 0x8);
      } else {
         intrin->num_components = 2;
         intrin->def.num_components = 2;
         intrin->def.bit_size = 32;

         data_xy = nir_channels(b, &intrin->def, 0x3);
         data_w = nir_imm_int(b, 0);
      }

      nir_def *data = nir_vec4(b, nir_pack_64_2x32(b, data_xy),
                               nir_imm_zero(b, 1, 64),
                               nir_imm_zero(b, 1, 64),
                               nir_u2u64(b, data_w));

      nir_def_rewrite_uses_after(&intrin->def, data, data->parent_instr);
      return true;
   }

   if (format == PIPE_FORMAT_NONE)
      return false;

   /* In order for null descriptors to work properly, we don't want to shrink
    * loads when the alpha channel is read even if we know the format has
    * fewer channels.
    */
   if (comps_read & BITFIELD_BIT(3))
      return false;

   const unsigned old_comps = intrin->def.num_components;

   unsigned new_comps = util_format_get_nr_components(format);
   new_comps = util_next_power_of_two(new_comps);
   if (comps_read <= BITFIELD_MASK(2))
      new_comps = 2;
   if (comps_read <= BITFIELD_MASK(1))
      new_comps = 1;

   if (new_comps >= intrin->num_components)
      return false;

   b->cursor = nir_after_instr(&intrin->instr);

   intrin->num_components = new_comps;
   intrin->def.num_components = new_comps;

   assert(new_comps <= 4);
   nir_def *comps[4];
   for (unsigned c = 0; c < new_comps; c++)
      comps[c] = nir_channel(b, &intrin->def, c);
   for (unsigned c = new_comps; c < 3; c++)
      comps[c] = nir_imm_intN_t(b, 0, intrin->def.bit_size);
   if (new_comps < 4)
      comps[3] = nir_imm_intN_t(b, 1, intrin->def.bit_size);

   nir_def *data = nir_vec(b, comps, old_comps);
   nir_def_rewrite_uses_after(&intrin->def, data, data->parent_instr);
   return true;
}

static bool
shrink_image_store(nir_builder *b, nir_intrinsic_instr *intrin,
                  const struct nak_compiler *nak)
{
   enum pipe_format format = nir_intrinsic_format(intrin);
   nir_def *data = intrin->src[3].ssa;

   if (data->bit_size == 64) {
      assert(format == PIPE_FORMAT_NONE ||
             format == PIPE_FORMAT_R64_UINT ||
             format == PIPE_FORMAT_R64_SINT);

      b->cursor = nir_before_instr(&intrin->instr);

      /* For 64-bit image ops, we actually want a vec2 */
      nir_def *data_vec2 = nir_unpack_64_2x32(b, nir_channel(b, data, 0));
      nir_src_rewrite(&intrin->src[3], data_vec2);
      intrin->num_components = 2;
      return true;
   }

   if (format == PIPE_FORMAT_NONE)
      return false;

   unsigned new_comps = util_format_get_nr_components(format);
   new_comps = util_next_power_of_two(new_comps);
   if (new_comps >= intrin->num_components)
      return false;

   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *trimmed = nir_trim_vector(b, data, new_comps);
   nir_src_rewrite(&intrin->src[3], trimmed);
   intrin->num_components = new_comps;
   return true;
}

static bool
lower_image_txq(nir_builder *b, nir_intrinsic_instr *intrin,
                const struct nak_compiler *nak)
{
   b->cursor = nir_instr_remove(&intrin->instr);

   /* TODO: We should only support 32-bit handles */
   nir_def *img_h = nir_u2u32(b, intrin->src[0].ssa);

   nir_tex_instr *txq = nir_tex_instr_create(b->shader, 1);
   txq->sampler_dim = remap_sampler_dim(nir_intrinsic_image_dim(intrin));
   txq->is_array = nir_intrinsic_image_array(intrin);
   txq->dest_type = nir_type_int32;

   nir_component_mask_t mask;
   switch (intrin->intrinsic) {
   case nir_intrinsic_bindless_image_size: {
      nir_def *lod = intrin->src[1].ssa;

      txq->op = nir_texop_hdr_dim_nv;
      txq->src[0] = (nir_tex_src) {
         .src_type = nir_tex_src_backend1,
         .src = nir_src_for_ssa(nir_vec2(b, img_h, lod)),
      };
      mask = BITSET_MASK(intrin->def.num_components);
      break;
   }

   case nir_intrinsic_bindless_image_samples:
      txq->op = nir_texop_tex_type_nv;
      txq->src[0] = (nir_tex_src) {
         .src_type = nir_tex_src_backend1,
         .src = nir_src_for_ssa(img_h),
      };
      mask = BITSET_BIT(2);
      break;

   default:
      unreachable("Invalid image query op");
   }

   nir_def_init(&txq->instr, &txq->def, 4, 32);
   nir_builder_instr_insert(b, &txq->instr);

   /* Only pick off slected components */
   nir_def *res = nir_channels(b, &txq->def, mask);

   nir_def_rewrite_uses(&intrin->def, res);

   return true;
}

static bool
lower_tex_instr(nir_builder *b, nir_instr *instr, void *_data)
{
   const struct nak_compiler *nak = _data;

   switch (instr->type) {
   case nir_instr_type_tex: {
      nir_tex_instr *tex = nir_instr_as_tex(instr);
      switch (tex->op) {
      case nir_texop_tex:
      case nir_texop_txb:
      case nir_texop_txl:
      case nir_texop_txd:
      case nir_texop_txf:
      case nir_texop_txf_ms:
      case nir_texop_tg4:
      case nir_texop_lod:
         return lower_tex(b, tex, nak);
      case nir_texop_txs:
      case nir_texop_query_levels:
      case nir_texop_texture_samples:
         return lower_txq(b, tex, nak);
      default:
         unreachable("Unsupported texture instruction");
      }
   }
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_bindless_image_load:
         return shrink_image_load(b, intrin, nak);
      case nir_intrinsic_bindless_image_store:
         return shrink_image_store(b, intrin, nak);
      case nir_intrinsic_bindless_image_size:
      case nir_intrinsic_bindless_image_samples:
         return lower_image_txq(b, intrin, nak);
      default:
         return false;
      }
   }
   default:
      return false;
   }
}

bool
nak_nir_lower_tex(nir_shader *nir, const struct nak_compiler *nak)
{
   return nir_shader_instructions_pass(nir, lower_tex_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       (void *)nak);
}
