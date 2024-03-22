/*
 * Copyright (C) 2020-2021 Collabora, Ltd.
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
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 *   Boris Brezillon <boris.brezillon@collabora.com>
 */

#include "pan_blitter.h"
#include <math.h>
#include <stdio.h>
#include "compiler/nir/nir_builder.h"
#include "util/u_math.h"
#include "pan_blend.h"
#include "pan_desc.h"
#include "pan_encoder.h"
#include "pan_jc.h"
#include "pan_pool.h"
#include "pan_shader.h"
#include "pan_texture.h"

#if PAN_ARCH >= 6
/* On Midgard, the native blit infrastructure (via MFBD preloads) is broken or
 * missing in many cases. We instead use software paths as fallbacks to
 * implement blits, which are done as TILER jobs. No vertex shader is
 * necessary since we can supply screen-space coordinates directly.
 *
 * This is primarily designed as a fallback for preloads but could be extended
 * for other clears/blits if needed in the future. */

static enum mali_register_file_format
blit_type_to_reg_fmt(nir_alu_type in)
{
   switch (in) {
   case nir_type_float32:
      return MALI_REGISTER_FILE_FORMAT_F32;
   case nir_type_int32:
      return MALI_REGISTER_FILE_FORMAT_I32;
   case nir_type_uint32:
      return MALI_REGISTER_FILE_FORMAT_U32;
   default:
      unreachable("Invalid blit type");
   }
}
#endif

struct pan_blit_surface {
   gl_frag_result loc              : 4;
   nir_alu_type type               : 8;
   enum mali_texture_dimension dim : 2;
   bool array                      : 1;
   unsigned src_samples            : 5;
   unsigned dst_samples            : 5;
};

struct pan_blit_shader_key {
   struct pan_blit_surface surfaces[8];
};

struct pan_blit_shader_data {
   struct pan_blit_shader_key key;
   struct pan_shader_info info;
   mali_ptr address;
   unsigned blend_ret_offsets[8];
   nir_alu_type blend_types[8];
};

struct pan_blit_blend_shader_key {
   enum pipe_format format;
   nir_alu_type type;
   unsigned rt         : 3;
   unsigned nr_samples : 5;
   unsigned pad        : 24;
};

struct pan_blit_blend_shader_data {
   struct pan_blit_blend_shader_key key;
   mali_ptr address;
};

struct pan_blit_rsd_key {
   struct {
      enum pipe_format format;
      nir_alu_type type               : 8;
      unsigned src_samples            : 5;
      unsigned dst_samples            : 5;
      enum mali_texture_dimension dim : 2;
      bool array                      : 1;
   } rts[8], z, s;
};

struct pan_blit_rsd_data {
   struct pan_blit_rsd_key key;
   mali_ptr address;
};

#if PAN_ARCH >= 5
static void
pan_blitter_emit_blend(const struct panfrost_device *dev, unsigned rt,
                       const struct pan_image_view *iview,
                       const struct pan_blit_shader_data *blit_shader,
                       mali_ptr blend_shader, void *out)
{
   assert(blend_shader == 0 || PAN_ARCH <= 5);

   pan_pack(out, BLEND, cfg) {
      if (!iview) {
         cfg.enable = false;
#if PAN_ARCH >= 6
         cfg.internal.mode = MALI_BLEND_MODE_OFF;
#endif
         continue;
      }

      cfg.round_to_fb_precision = true;
      cfg.srgb = util_format_is_srgb(iview->format);

#if PAN_ARCH >= 6
      cfg.internal.mode = MALI_BLEND_MODE_OPAQUE;
#endif

      if (!blend_shader) {
         cfg.equation.rgb.a = MALI_BLEND_OPERAND_A_SRC;
         cfg.equation.rgb.b = MALI_BLEND_OPERAND_B_SRC;
         cfg.equation.rgb.c = MALI_BLEND_OPERAND_C_ZERO;
         cfg.equation.alpha.a = MALI_BLEND_OPERAND_A_SRC;
         cfg.equation.alpha.b = MALI_BLEND_OPERAND_B_SRC;
         cfg.equation.alpha.c = MALI_BLEND_OPERAND_C_ZERO;
         cfg.equation.color_mask = 0xf;

#if PAN_ARCH >= 6
         nir_alu_type type = blit_shader->key.surfaces[rt].type;

         cfg.internal.fixed_function.num_comps = 4;
         cfg.internal.fixed_function.conversion.memory_format =
            panfrost_format_to_bifrost_blend(dev, iview->format, false);
         cfg.internal.fixed_function.conversion.register_format =
            blit_type_to_reg_fmt(type);

         cfg.internal.fixed_function.rt = rt;
#endif
      } else {
#if PAN_ARCH <= 5
         cfg.blend_shader = true;
         cfg.shader_pc = blend_shader;
#endif
      }
   }
}
#endif

struct pan_blitter_views {
   unsigned rt_count;
   const struct pan_image_view *src_rts[8];
   const struct pan_image_view *dst_rts[8];
   const struct pan_image_view *src_z;
   const struct pan_image_view *dst_z;
   const struct pan_image_view *src_s;
   const struct pan_image_view *dst_s;
};

static bool
pan_blitter_is_ms(struct pan_blitter_views *views)
{
   for (unsigned i = 0; i < views->rt_count; i++) {
      if (views->dst_rts[i]) {
         if (pan_image_view_get_nr_samples(views->dst_rts[i]) > 1)
            return true;
      }
   }

   if (views->dst_z && pan_image_view_get_nr_samples(views->dst_z) > 1)
      return true;

   if (views->dst_s && pan_image_view_get_nr_samples(views->dst_s) > 1)
      return true;

   return false;
}

#if PAN_ARCH >= 5
static void
pan_blitter_emit_blends(const struct panfrost_device *dev,
                        const struct pan_blit_shader_data *blit_shader,
                        struct pan_blitter_views *views,
                        mali_ptr *blend_shaders, void *out)
{
   for (unsigned i = 0; i < MAX2(views->rt_count, 1); ++i) {
      void *dest = out + pan_size(BLEND) * i;
      const struct pan_image_view *rt_view = views->dst_rts[i];
      mali_ptr blend_shader = blend_shaders ? blend_shaders[i] : 0;

      pan_blitter_emit_blend(dev, i, rt_view, blit_shader, blend_shader, dest);
   }
}
#endif

#if PAN_ARCH <= 7
static void
pan_blitter_emit_rsd(const struct panfrost_device *dev,
                     const struct pan_blit_shader_data *blit_shader,
                     struct pan_blitter_views *views, mali_ptr *blend_shaders,
                     void *out)
{
   UNUSED bool zs = (views->dst_z || views->dst_s);
   bool ms = pan_blitter_is_ms(views);

   pan_pack(out, RENDERER_STATE, cfg) {
      assert(blit_shader->address);
      pan_shader_prepare_rsd(&blit_shader->info, blit_shader->address, &cfg);

      cfg.multisample_misc.sample_mask = 0xFFFF;
      cfg.multisample_misc.multisample_enable = ms;
      cfg.multisample_misc.evaluate_per_sample = ms;
      cfg.multisample_misc.depth_write_mask = views->dst_z != NULL;
      cfg.multisample_misc.depth_function = MALI_FUNC_ALWAYS;

      cfg.stencil_mask_misc.stencil_enable = views->dst_s != NULL;
      cfg.stencil_mask_misc.stencil_mask_front = 0xFF;
      cfg.stencil_mask_misc.stencil_mask_back = 0xFF;
      cfg.stencil_front.compare_function = MALI_FUNC_ALWAYS;
      cfg.stencil_front.stencil_fail = MALI_STENCIL_OP_REPLACE;
      cfg.stencil_front.depth_fail = MALI_STENCIL_OP_REPLACE;
      cfg.stencil_front.depth_pass = MALI_STENCIL_OP_REPLACE;
      cfg.stencil_front.mask = 0xFF;
      cfg.stencil_back = cfg.stencil_front;

#if PAN_ARCH >= 6
      if (zs) {
         /* Writing Z/S requires late updates */
         cfg.properties.zs_update_operation = MALI_PIXEL_KILL_FORCE_LATE;
         cfg.properties.pixel_kill_operation = MALI_PIXEL_KILL_FORCE_LATE;
      } else {
         /* Skipping ATEST requires forcing Z/S */
         cfg.properties.zs_update_operation = MALI_PIXEL_KILL_STRONG_EARLY;
         cfg.properties.pixel_kill_operation = MALI_PIXEL_KILL_FORCE_EARLY;
      }

      /* However, while shaders writing Z/S can normally be killed, on v6
       * for frame shaders it can cause GPU timeouts, so only allow colour
       * blit shaders to be killed. */
      cfg.properties.allow_forward_pixel_to_kill = !zs;

      if (PAN_ARCH == 6)
         cfg.properties.allow_forward_pixel_to_be_killed = !zs;
#else

      mali_ptr blend_shader =
         blend_shaders
            ? panfrost_last_nonnull(blend_shaders, MAX2(views->rt_count, 1))
            : 0;

      cfg.properties.work_register_count = 4;
      cfg.properties.force_early_z = !zs;
      cfg.stencil_mask_misc.alpha_test_compare_function = MALI_FUNC_ALWAYS;

      /* Set even on v5 for erratum workaround */
#if PAN_ARCH == 5
      cfg.legacy_blend_shader = blend_shader;
#else
      cfg.blend_shader = blend_shader;
      cfg.stencil_mask_misc.write_enable = true;
      cfg.stencil_mask_misc.dither_disable = true;
      cfg.multisample_misc.blend_shader = !!blend_shader;
      cfg.blend_shader = blend_shader;
      if (!cfg.multisample_misc.blend_shader) {
         cfg.blend_equation.rgb.a = MALI_BLEND_OPERAND_A_SRC;
         cfg.blend_equation.rgb.b = MALI_BLEND_OPERAND_B_SRC;
         cfg.blend_equation.rgb.c = MALI_BLEND_OPERAND_C_ZERO;
         cfg.blend_equation.alpha.a = MALI_BLEND_OPERAND_A_SRC;
         cfg.blend_equation.alpha.b = MALI_BLEND_OPERAND_B_SRC;
         cfg.blend_equation.alpha.c = MALI_BLEND_OPERAND_C_ZERO;
         cfg.blend_constant = 0;

         if (views->dst_rts[0] != NULL) {
            cfg.stencil_mask_misc.srgb =
               util_format_is_srgb(views->dst_rts[0]->format);
            cfg.blend_equation.color_mask = 0xf;
         }
      }
#endif
#endif
   }

#if PAN_ARCH >= 5
   pan_blitter_emit_blends(dev, blit_shader, views, blend_shaders,
                           out + pan_size(RENDERER_STATE));
#endif
}
#endif

#if PAN_ARCH <= 5
static void
pan_blitter_get_blend_shaders(struct panfrost_device *dev, unsigned rt_count,
                              const struct pan_image_view **rts,
                              const struct pan_blit_shader_data *blit_shader,
                              mali_ptr *blend_shaders)
{
   if (!rt_count)
      return;

   struct pan_blend_state blend_state = {
      .rt_count = rt_count,
   };

   for (unsigned i = 0; i < rt_count; i++) {
      if (!rts[i] || panfrost_blendable_formats_v7[rts[i]->format].internal)
         continue;

      struct pan_blit_blend_shader_key key = {
         .format = rts[i]->format,
         .rt = i,
         .nr_samples = pan_image_view_get_nr_samples(rts[i]),
         .type = blit_shader->blend_types[i],
      };

      pthread_mutex_lock(&dev->blitter.shaders.lock);
      struct hash_entry *he =
         _mesa_hash_table_search(dev->blitter.shaders.blend, &key);
      struct pan_blit_blend_shader_data *blend_shader = he ? he->data : NULL;
      if (blend_shader) {
         blend_shaders[i] = blend_shader->address;
         pthread_mutex_unlock(&dev->blitter.shaders.lock);
         continue;
      }

      blend_shader =
         rzalloc(dev->blitter.shaders.blend, struct pan_blit_blend_shader_data);
      blend_shader->key = key;

      blend_state.rts[i] = (struct pan_blend_rt_state){
         .format = rts[i]->format,
         .nr_samples = pan_image_view_get_nr_samples(rts[i]),
         .equation =
            {
               .blend_enable = false,
               .color_mask = 0xf,
            },
      };

      pthread_mutex_lock(&dev->blend_shaders.lock);
      struct pan_blend_shader_variant *b = GENX(pan_blend_get_shader_locked)(
         dev, &blend_state, blit_shader->blend_types[i],
         nir_type_float32, /* unused */
         i);

      assert(b->work_reg_count <= 4);
      struct panfrost_ptr bin =
         pan_pool_alloc_aligned(dev->blitter.shaders.pool, b->binary.size, 64);
      memcpy(bin.cpu, b->binary.data, b->binary.size);

      blend_shader->address = bin.gpu | b->first_tag;
      pthread_mutex_unlock(&dev->blend_shaders.lock);
      _mesa_hash_table_insert(dev->blitter.shaders.blend, &blend_shader->key,
                              blend_shader);
      pthread_mutex_unlock(&dev->blitter.shaders.lock);
      blend_shaders[i] = blend_shader->address;
   }
}
#endif

/*
 * Early Mali GPUs did not respect sampler LOD clamps or bias, so the Midgard
 * compiler inserts lowering code with a load_sampler_lod_parameters_pan sysval
 * that we need to lower. Our samplers do not use LOD clamps or bias, so we
 * lower to the identity settings and let constant folding get rid of the
 * unnecessary lowering.
 */
static bool
lower_sampler_parameters(nir_builder *b, nir_intrinsic_instr *intr,
                         UNUSED void *data)
{
   if (intr->intrinsic != nir_intrinsic_load_sampler_lod_parameters_pan)
      return false;

   const nir_const_value constants[4] = {
      nir_const_value_for_float(0.0f, 32),     /* min_lod */
      nir_const_value_for_float(INFINITY, 32), /* max_lod */
      nir_const_value_for_float(0.0f, 32),     /* lod_bias */
   };

   b->cursor = nir_after_instr(&intr->instr);
   nir_def_rewrite_uses(&intr->def, nir_build_imm(b, 3, 32, constants));
   return true;
}

static const struct pan_blit_shader_data *
pan_blitter_get_blit_shader(struct panfrost_device *dev,
                            const struct pan_blit_shader_key *key)
{
   pthread_mutex_lock(&dev->blitter.shaders.lock);
   struct hash_entry *he =
      _mesa_hash_table_search(dev->blitter.shaders.blit, key);
   struct pan_blit_shader_data *shader = he ? he->data : NULL;

   if (shader)
      goto out;

   unsigned coord_comps = 0;
   unsigned sig_offset = 0;
   char sig[256];
   bool first = true;
   for (unsigned i = 0; i < ARRAY_SIZE(key->surfaces); i++) {
      const char *type_str, *dim_str;
      if (key->surfaces[i].type == nir_type_invalid)
         continue;

      switch (key->surfaces[i].type) {
      case nir_type_float32:
         type_str = "float";
         break;
      case nir_type_uint32:
         type_str = "uint";
         break;
      case nir_type_int32:
         type_str = "int";
         break;
      default:
         unreachable("Invalid type\n");
      }

      switch (key->surfaces[i].dim) {
      case MALI_TEXTURE_DIMENSION_CUBE:
         dim_str = "cube";
         break;
      case MALI_TEXTURE_DIMENSION_1D:
         dim_str = "1D";
         break;
      case MALI_TEXTURE_DIMENSION_2D:
         dim_str = "2D";
         break;
      case MALI_TEXTURE_DIMENSION_3D:
         dim_str = "3D";
         break;
      default:
         unreachable("Invalid dim\n");
      }

      coord_comps = MAX2(coord_comps, (key->surfaces[i].dim ?: 3) +
                                         (key->surfaces[i].array ? 1 : 0));
      first = false;

      if (sig_offset >= sizeof(sig))
         continue;

      sig_offset +=
         snprintf(sig + sig_offset, sizeof(sig) - sig_offset,
                  "%s[%s;%s;%s%s;src_samples=%d,dst_samples=%d]",
                  first ? "" : ",", gl_frag_result_name(key->surfaces[i].loc),
                  type_str, dim_str, key->surfaces[i].array ? "[]" : "",
                  key->surfaces[i].src_samples, key->surfaces[i].dst_samples);
   }

   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_FRAGMENT, GENX(pan_shader_get_compiler_options)(),
      "pan_blit(%s)", sig);
   nir_variable *coord_var = nir_variable_create(
      b.shader, nir_var_shader_in,
      glsl_vector_type(GLSL_TYPE_FLOAT, coord_comps), "coord");
   coord_var->data.location = VARYING_SLOT_VAR0;

   nir_def *coord = nir_load_var(&b, coord_var);

   unsigned active_count = 0;
   for (unsigned i = 0; i < ARRAY_SIZE(key->surfaces); i++) {
      if (key->surfaces[i].type == nir_type_invalid)
         continue;

      /* Resolve operations only work for N -> 1 samples. */
      assert(key->surfaces[i].dst_samples == 1 ||
             key->surfaces[i].src_samples == key->surfaces[i].dst_samples);

      static const char *out_names[] = {
         "out0", "out1", "out2", "out3", "out4", "out5", "out6", "out7",
      };

      unsigned ncomps = key->surfaces[i].loc >= FRAG_RESULT_DATA0 ? 4 : 1;
      enum glsl_base_type type =
         nir_get_glsl_base_type_for_nir_type(key->surfaces[i].type);
      nir_variable *out = nir_variable_create(b.shader, nir_var_shader_out,
                                              glsl_vector_type(type, ncomps),
                                              out_names[active_count]);
      out->data.location = key->surfaces[i].loc;
      out->data.driver_location = active_count;

      bool resolve =
         key->surfaces[i].src_samples > key->surfaces[i].dst_samples;
      bool ms = key->surfaces[i].src_samples > 1;
      enum glsl_sampler_dim sampler_dim;

      switch (key->surfaces[i].dim) {
      case MALI_TEXTURE_DIMENSION_1D:
         sampler_dim = GLSL_SAMPLER_DIM_1D;
         break;
      case MALI_TEXTURE_DIMENSION_2D:
         sampler_dim = ms ? GLSL_SAMPLER_DIM_MS : GLSL_SAMPLER_DIM_2D;
         break;
      case MALI_TEXTURE_DIMENSION_3D:
         sampler_dim = GLSL_SAMPLER_DIM_3D;
         break;
      case MALI_TEXTURE_DIMENSION_CUBE:
         sampler_dim = GLSL_SAMPLER_DIM_CUBE;
         break;
      }

      nir_def *res = NULL;

      if (resolve) {
         /* When resolving a float type, we need to calculate
          * the average of all samples. For integer resolve, GL
          * and Vulkan say that one sample should be chosen
          * without telling which. Let's just pick the first one
          * in that case.
          */
         nir_alu_type base_type =
            nir_alu_type_get_base_type(key->surfaces[i].type);
         unsigned nsamples =
            base_type == nir_type_float ? key->surfaces[i].src_samples : 1;

         for (unsigned s = 0; s < nsamples; s++) {
            nir_tex_instr *tex = nir_tex_instr_create(b.shader, 3);

            tex->op = nir_texop_txf_ms;
            tex->dest_type = key->surfaces[i].type;
            tex->texture_index = active_count;
            tex->is_array = key->surfaces[i].array;
            tex->sampler_dim = sampler_dim;

            tex->src[0] =
               nir_tex_src_for_ssa(nir_tex_src_coord, nir_f2i32(&b, coord));
            tex->coord_components = coord_comps;

            tex->src[1] =
               nir_tex_src_for_ssa(nir_tex_src_ms_index, nir_imm_int(&b, s));

            tex->src[2] =
               nir_tex_src_for_ssa(nir_tex_src_lod, nir_imm_int(&b, 0));
            nir_def_init(&tex->instr, &tex->def, 4, 32);
            nir_builder_instr_insert(&b, &tex->instr);

            res = res ? nir_fadd(&b, res, &tex->def) : &tex->def;
         }

         if (base_type == nir_type_float)
            res = nir_fmul_imm(&b, res, 1.0f / nsamples);
      } else {
         nir_tex_instr *tex = nir_tex_instr_create(b.shader, ms ? 3 : 1);

         tex->dest_type = key->surfaces[i].type;
         tex->texture_index = active_count;
         tex->is_array = key->surfaces[i].array;
         tex->sampler_dim = sampler_dim;

         if (ms) {
            tex->op = nir_texop_txf_ms;

            tex->src[0] =
               nir_tex_src_for_ssa(nir_tex_src_coord, nir_f2i32(&b, coord));
            tex->coord_components = coord_comps;

            tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_ms_index,
                                              nir_load_sample_id(&b));

            tex->src[2] =
               nir_tex_src_for_ssa(nir_tex_src_lod, nir_imm_int(&b, 0));
         } else {
            tex->op = nir_texop_txl;

            tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord, coord);
            tex->coord_components = coord_comps;
         }

         nir_def_init(&tex->instr, &tex->def, 4, 32);
         nir_builder_instr_insert(&b, &tex->instr);
         res = &tex->def;
      }

      assert(res);

      if (key->surfaces[i].loc >= FRAG_RESULT_DATA0) {
         nir_store_var(&b, out, res, 0xFF);
      } else {
         unsigned c = key->surfaces[i].loc == FRAG_RESULT_STENCIL ? 1 : 0;
         nir_store_var(&b, out, nir_channel(&b, res, c), 0xFF);
      }
      active_count++;
   }

   struct panfrost_compile_inputs inputs = {
      .gpu_id = panfrost_device_gpu_id(dev),
      .is_blit = true,
      .no_idvs = true,
   };
   struct util_dynarray binary;

   util_dynarray_init(&binary, NULL);

   shader = rzalloc(dev->blitter.shaders.blit, struct pan_blit_shader_data);

   nir_shader_gather_info(b.shader, nir_shader_get_entrypoint(b.shader));

   for (unsigned i = 0; i < active_count; ++i)
      BITSET_SET(b.shader->info.textures_used, i);

   pan_shader_preprocess(b.shader, inputs.gpu_id);

   if (PAN_ARCH == 4) {
      NIR_PASS_V(b.shader, nir_shader_intrinsics_pass, lower_sampler_parameters,
                 nir_metadata_block_index | nir_metadata_dominance, NULL);
   }

   GENX(pan_shader_compile)(b.shader, &inputs, &binary, &shader->info);

   shader->key = *key;
   shader->address =
      pan_pool_upload_aligned(dev->blitter.shaders.pool, binary.data,
                              binary.size, PAN_ARCH >= 6 ? 128 : 64);

   util_dynarray_fini(&binary);
   ralloc_free(b.shader);

#if PAN_ARCH >= 6
   for (unsigned i = 0; i < ARRAY_SIZE(shader->blend_ret_offsets); i++) {
      shader->blend_ret_offsets[i] =
         shader->info.bifrost.blend[i].return_offset;
      shader->blend_types[i] = shader->info.bifrost.blend[i].type;
   }
#endif

   _mesa_hash_table_insert(dev->blitter.shaders.blit, &shader->key, shader);

out:
   pthread_mutex_unlock(&dev->blitter.shaders.lock);
   return shader;
}

static struct pan_blit_shader_key
pan_blitter_get_key(struct pan_blitter_views *views)
{
   struct pan_blit_shader_key key = {0};

   if (views->src_z) {
      assert(views->dst_z);
      key.surfaces[0].loc = FRAG_RESULT_DEPTH;
      key.surfaces[0].type = nir_type_float32;
      key.surfaces[0].src_samples = pan_image_view_get_nr_samples(views->src_z);
      key.surfaces[0].dst_samples = pan_image_view_get_nr_samples(views->dst_z);
      key.surfaces[0].dim = views->src_z->dim;
      key.surfaces[0].array =
         views->src_z->first_layer != views->src_z->last_layer;
   }

   if (views->src_s) {
      assert(views->dst_s);
      key.surfaces[1].loc = FRAG_RESULT_STENCIL;
      key.surfaces[1].type = nir_type_uint32;
      key.surfaces[1].src_samples = pan_image_view_get_nr_samples(views->src_s);
      key.surfaces[1].dst_samples = pan_image_view_get_nr_samples(views->dst_s);
      key.surfaces[1].dim = views->src_s->dim;
      key.surfaces[1].array =
         views->src_s->first_layer != views->src_s->last_layer;
   }

   for (unsigned i = 0; i < views->rt_count; i++) {
      if (!views->src_rts[i])
         continue;

      assert(views->dst_rts[i]);
      key.surfaces[i].loc = FRAG_RESULT_DATA0 + i;
      key.surfaces[i].type =
         util_format_is_pure_uint(views->src_rts[i]->format) ? nir_type_uint32
         : util_format_is_pure_sint(views->src_rts[i]->format)
            ? nir_type_int32
            : nir_type_float32;
      key.surfaces[i].src_samples =
         pan_image_view_get_nr_samples(views->src_rts[i]);
      key.surfaces[i].dst_samples =
         pan_image_view_get_nr_samples(views->dst_rts[i]);
      key.surfaces[i].dim = views->src_rts[i]->dim;
      key.surfaces[i].array =
         views->src_rts[i]->first_layer != views->src_rts[i]->last_layer;
   }

   return key;
}

#if PAN_ARCH <= 7
static mali_ptr
pan_blitter_get_rsd(struct panfrost_device *dev,
                    struct pan_blitter_views *views)
{
   struct pan_blit_rsd_key rsd_key = {0};

   assert(!views->rt_count || (!views->src_z && !views->src_s));

   struct pan_blit_shader_key blit_key = pan_blitter_get_key(views);

   if (views->src_z) {
      assert(views->dst_z);
      rsd_key.z.format = views->dst_z->format;
      rsd_key.z.type = blit_key.surfaces[0].type;
      rsd_key.z.src_samples = blit_key.surfaces[0].src_samples;
      rsd_key.z.dst_samples = blit_key.surfaces[0].dst_samples;
      rsd_key.z.dim = blit_key.surfaces[0].dim;
      rsd_key.z.array = blit_key.surfaces[0].array;
   }

   if (views->src_s) {
      assert(views->dst_s);
      rsd_key.s.format = views->dst_s->format;
      rsd_key.s.type = blit_key.surfaces[1].type;
      rsd_key.s.src_samples = blit_key.surfaces[1].src_samples;
      rsd_key.s.dst_samples = blit_key.surfaces[1].dst_samples;
      rsd_key.s.dim = blit_key.surfaces[1].dim;
      rsd_key.s.array = blit_key.surfaces[1].array;
   }

   for (unsigned i = 0; i < views->rt_count; i++) {
      if (!views->src_rts[i])
         continue;

      assert(views->dst_rts[i]);
      rsd_key.rts[i].format = views->dst_rts[i]->format;
      rsd_key.rts[i].type = blit_key.surfaces[i].type;
      rsd_key.rts[i].src_samples = blit_key.surfaces[i].src_samples;
      rsd_key.rts[i].dst_samples = blit_key.surfaces[i].dst_samples;
      rsd_key.rts[i].dim = blit_key.surfaces[i].dim;
      rsd_key.rts[i].array = blit_key.surfaces[i].array;
   }

   pthread_mutex_lock(&dev->blitter.rsds.lock);
   struct hash_entry *he =
      _mesa_hash_table_search(dev->blitter.rsds.rsds, &rsd_key);
   struct pan_blit_rsd_data *rsd = he ? he->data : NULL;
   if (rsd)
      goto out;

   rsd = rzalloc(dev->blitter.rsds.rsds, struct pan_blit_rsd_data);
   rsd->key = rsd_key;

   unsigned bd_count = PAN_ARCH >= 5 ? MAX2(views->rt_count, 1) : 0;
   struct panfrost_ptr rsd_ptr = pan_pool_alloc_desc_aggregate(
      dev->blitter.rsds.pool, PAN_DESC(RENDERER_STATE),
      PAN_DESC_ARRAY(bd_count, BLEND));

   mali_ptr blend_shaders[8] = {0};

   const struct pan_blit_shader_data *blit_shader =
      pan_blitter_get_blit_shader(dev, &blit_key);

#if PAN_ARCH <= 5
   pan_blitter_get_blend_shaders(dev, views->rt_count, views->dst_rts,
                                 blit_shader, blend_shaders);
#endif

   pan_blitter_emit_rsd(dev, blit_shader, views, blend_shaders, rsd_ptr.cpu);
   rsd->address = rsd_ptr.gpu;
   _mesa_hash_table_insert(dev->blitter.rsds.rsds, &rsd->key, rsd);

out:
   pthread_mutex_unlock(&dev->blitter.rsds.lock);
   return rsd->address;
}

static mali_ptr
pan_blit_get_rsd(struct panfrost_device *dev,
                 const struct pan_image_view *src_views,
                 const struct pan_image_view *dst_view)
{
   const struct util_format_description *desc =
      util_format_description(src_views[0].format);

   struct pan_blitter_views views = {};

   if (util_format_has_depth(desc)) {
      views.src_z = &src_views[0];
      views.dst_z = dst_view;
   }

   if (src_views[1].format) {
      views.src_s = &src_views[1];
      views.dst_s = dst_view;
   } else if (util_format_has_stencil(desc)) {
      views.src_s = &src_views[0];
      views.dst_s = dst_view;
   }

   if (!views.src_z && !views.src_s) {
      views.rt_count = 1;
      views.src_rts[0] = src_views;
      views.dst_rts[0] = dst_view;
   }

   return pan_blitter_get_rsd(dev, &views);
}
#endif

static struct pan_blitter_views
pan_preload_get_views(const struct pan_fb_info *fb, bool zs,
                      struct pan_image_view *patched_s)
{
   struct pan_blitter_views views = {0};

   if (zs) {
      if (fb->zs.preload.z)
         views.src_z = views.dst_z = fb->zs.view.zs;

      if (fb->zs.preload.s) {
         const struct pan_image_view *view = fb->zs.view.s ?: fb->zs.view.zs;
         enum pipe_format fmt = util_format_get_depth_only(view->format);

         switch (view->format) {
         case PIPE_FORMAT_Z24_UNORM_S8_UINT:
            fmt = PIPE_FORMAT_X24S8_UINT;
            break;
         case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
            fmt = PIPE_FORMAT_X32_S8X24_UINT;
            break;
         default:
            fmt = view->format;
            break;
         }

         if (fmt != view->format) {
            *patched_s = *view;
            patched_s->format = fmt;
            views.src_s = views.dst_s = patched_s;
         } else {
            views.src_s = views.dst_s = view;
         }
      }
   } else {
      for (unsigned i = 0; i < fb->rt_count; i++) {
         if (fb->rts[i].preload) {
            views.src_rts[i] = fb->rts[i].view;
            views.dst_rts[i] = fb->rts[i].view;
         }
      }

      views.rt_count = fb->rt_count;
   }

   return views;
}

static bool
pan_preload_needed(const struct pan_fb_info *fb, bool zs)
{
   if (zs) {
      if (fb->zs.preload.z || fb->zs.preload.s)
         return true;
   } else {
      for (unsigned i = 0; i < fb->rt_count; i++) {
         if (fb->rts[i].preload)
            return true;
      }
   }

   return false;
}

static mali_ptr
pan_blitter_emit_varying(struct pan_pool *pool)
{
   struct panfrost_ptr varying = pan_pool_alloc_desc(pool, ATTRIBUTE);

   pan_pack(varying.cpu, ATTRIBUTE, cfg) {
      cfg.buffer_index = 0;
      cfg.offset_enable = PAN_ARCH <= 5;
      cfg.format = pool->dev->formats[PIPE_FORMAT_R32G32B32_FLOAT].hw;

#if PAN_ARCH >= 9
      cfg.attribute_type = MALI_ATTRIBUTE_TYPE_1D;
      cfg.table = PAN_TABLE_ATTRIBUTE_BUFFER;
      cfg.frequency = MALI_ATTRIBUTE_FREQUENCY_VERTEX;
      cfg.stride = 4 * sizeof(float);
#endif
   }

   return varying.gpu;
}

static mali_ptr
pan_blitter_emit_varying_buffer(struct pan_pool *pool, mali_ptr coordinates)
{
#if PAN_ARCH >= 9
   struct panfrost_ptr varying_buffer = pan_pool_alloc_desc(pool, BUFFER);

   pan_pack(varying_buffer.cpu, BUFFER, cfg) {
      cfg.address = coordinates;
      cfg.size = 4 * sizeof(float) * 4;
   }
#else
   /* Bifrost needs an empty desc to mark end of prefetching */
   bool padding_buffer = PAN_ARCH >= 6;

   struct panfrost_ptr varying_buffer = pan_pool_alloc_desc_array(
      pool, (padding_buffer ? 2 : 1), ATTRIBUTE_BUFFER);

   pan_pack(varying_buffer.cpu, ATTRIBUTE_BUFFER, cfg) {
      cfg.pointer = coordinates;
      cfg.stride = 4 * sizeof(float);
      cfg.size = cfg.stride * 4;
   }

   if (padding_buffer) {
      pan_pack(varying_buffer.cpu + pan_size(ATTRIBUTE_BUFFER),
               ATTRIBUTE_BUFFER, cfg)
         ;
   }
#endif

   return varying_buffer.gpu;
}

static mali_ptr
pan_blitter_emit_sampler(struct pan_pool *pool, bool nearest_filter)
{
   struct panfrost_ptr sampler = pan_pool_alloc_desc(pool, SAMPLER);

   pan_pack(sampler.cpu, SAMPLER, cfg) {
      cfg.seamless_cube_map = false;
      cfg.normalized_coordinates = false;
      cfg.minify_nearest = nearest_filter;
      cfg.magnify_nearest = nearest_filter;
   }

   return sampler.gpu;
}

static mali_ptr
pan_blitter_emit_textures(struct pan_pool *pool, unsigned tex_count,
                          const struct pan_image_view **views)
{
#if PAN_ARCH >= 6
   struct panfrost_ptr textures =
      pan_pool_alloc_desc_array(pool, tex_count, TEXTURE);

   for (unsigned i = 0; i < tex_count; i++) {
      void *texture = textures.cpu + (pan_size(TEXTURE) * i);
      size_t payload_size =
         GENX(panfrost_estimate_texture_payload_size)(views[i]);
      struct panfrost_ptr surfaces =
         pan_pool_alloc_aligned(pool, payload_size, 64);

      GENX(panfrost_new_texture)(pool->dev, views[i], texture, &surfaces);
   }

   return textures.gpu;
#else
   mali_ptr textures[8] = {0};

   for (unsigned i = 0; i < tex_count; i++) {
      size_t sz = pan_size(TEXTURE) +
                  GENX(panfrost_estimate_texture_payload_size)(views[i]);
      struct panfrost_ptr texture =
         pan_pool_alloc_aligned(pool, sz, pan_alignment(TEXTURE));
      struct panfrost_ptr surfaces = {
         .cpu = texture.cpu + pan_size(TEXTURE),
         .gpu = texture.gpu + pan_size(TEXTURE),
      };

      GENX(panfrost_new_texture)(pool->dev, views[i], texture.cpu, &surfaces);
      textures[i] = texture.gpu;
   }

   return pan_pool_upload_aligned(pool, textures, tex_count * sizeof(mali_ptr),
                                  sizeof(mali_ptr));
#endif
}

static mali_ptr
pan_preload_emit_textures(struct pan_pool *pool, const struct pan_fb_info *fb,
                          bool zs, unsigned *tex_count_out)
{
   const struct pan_image_view *views[8];
   struct pan_image_view patched_s_view;
   unsigned tex_count = 0;

   if (zs) {
      if (fb->zs.preload.z)
         views[tex_count++] = fb->zs.view.zs;

      if (fb->zs.preload.s) {
         const struct pan_image_view *view = fb->zs.view.s ?: fb->zs.view.zs;
         enum pipe_format fmt = util_format_get_depth_only(view->format);

         switch (view->format) {
         case PIPE_FORMAT_Z24_UNORM_S8_UINT:
            fmt = PIPE_FORMAT_X24S8_UINT;
            break;
         case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
            fmt = PIPE_FORMAT_X32_S8X24_UINT;
            break;
         default:
            fmt = view->format;
            break;
         }

         if (fmt != view->format) {
            patched_s_view = *view;
            patched_s_view.format = fmt;
            view = &patched_s_view;
         }
         views[tex_count++] = view;
      }
   } else {
      for (unsigned i = 0; i < fb->rt_count; i++) {
         if (fb->rts[i].preload)
            views[tex_count++] = fb->rts[i].view;
      }
   }

   *tex_count_out = tex_count;

   return pan_blitter_emit_textures(pool, tex_count, views);
}

#if PAN_ARCH >= 8
/* TODO: cache */
static mali_ptr
pan_blitter_emit_zs(struct pan_pool *pool, bool z, bool s)
{
   struct panfrost_ptr zsd = pan_pool_alloc_desc(pool, DEPTH_STENCIL);

   pan_pack(zsd.cpu, DEPTH_STENCIL, cfg) {
      cfg.depth_function = MALI_FUNC_ALWAYS;
      cfg.depth_write_enable = z;

      if (z)
         cfg.depth_source = MALI_DEPTH_SOURCE_SHADER;

      cfg.stencil_test_enable = s;
      cfg.stencil_from_shader = s;

      cfg.front_compare_function = MALI_FUNC_ALWAYS;
      cfg.front_stencil_fail = MALI_STENCIL_OP_REPLACE;
      cfg.front_depth_fail = MALI_STENCIL_OP_REPLACE;
      cfg.front_depth_pass = MALI_STENCIL_OP_REPLACE;
      cfg.front_write_mask = 0xFF;
      cfg.front_value_mask = 0xFF;

      cfg.back_compare_function = MALI_FUNC_ALWAYS;
      cfg.back_stencil_fail = MALI_STENCIL_OP_REPLACE;
      cfg.back_depth_fail = MALI_STENCIL_OP_REPLACE;
      cfg.back_depth_pass = MALI_STENCIL_OP_REPLACE;
      cfg.back_write_mask = 0xFF;
      cfg.back_value_mask = 0xFF;

      cfg.depth_cull_enable = false;
   }

   return zsd.gpu;
}
#else
static mali_ptr
pan_blitter_emit_viewport(struct pan_pool *pool, uint16_t minx, uint16_t miny,
                          uint16_t maxx, uint16_t maxy)
{
   struct panfrost_ptr vp = pan_pool_alloc_desc(pool, VIEWPORT);

   pan_pack(vp.cpu, VIEWPORT, cfg) {
      cfg.scissor_minimum_x = minx;
      cfg.scissor_minimum_y = miny;
      cfg.scissor_maximum_x = maxx;
      cfg.scissor_maximum_y = maxy;
   }

   return vp.gpu;
}
#endif

static void
pan_preload_emit_dcd(struct pan_pool *pool, struct pan_fb_info *fb, bool zs,
                     mali_ptr coordinates, mali_ptr tsd, void *out,
                     bool always_write)
{
   unsigned tex_count = 0;
   mali_ptr textures = pan_preload_emit_textures(pool, fb, zs, &tex_count);
   mali_ptr samplers = pan_blitter_emit_sampler(pool, true);
   mali_ptr varyings = pan_blitter_emit_varying(pool);
   mali_ptr varying_buffers =
      pan_blitter_emit_varying_buffer(pool, coordinates);

   /* Tiles updated by blit shaders are still considered clean (separate
    * for colour and Z/S), allowing us to suppress unnecessary writeback
    */
   UNUSED bool clean_fragment_write = !always_write;

   /* Image view used when patching stencil formats for combined
    * depth/stencil preloads.
    */
   struct pan_image_view patched_s;

   struct pan_blitter_views views = pan_preload_get_views(fb, zs, &patched_s);

#if PAN_ARCH <= 7
   pan_pack(out, DRAW, cfg) {
      uint16_t minx = 0, miny = 0, maxx, maxy;

      if (PAN_ARCH == 4) {
         maxx = fb->width - 1;
         maxy = fb->height - 1;
      } else {
         /* Align on 32x32 tiles */
         minx = fb->extent.minx & ~31;
         miny = fb->extent.miny & ~31;
         maxx = MIN2(ALIGN_POT(fb->extent.maxx + 1, 32), fb->width) - 1;
         maxy = MIN2(ALIGN_POT(fb->extent.maxy + 1, 32), fb->height) - 1;
      }

      cfg.thread_storage = tsd;
      cfg.state = pan_blitter_get_rsd(pool->dev, &views);

      cfg.position = coordinates;
      cfg.viewport = pan_blitter_emit_viewport(pool, minx, miny, maxx, maxy);

      cfg.varyings = varyings;
      cfg.varying_buffers = varying_buffers;
      cfg.textures = textures;
      cfg.samplers = samplers;

#if PAN_ARCH >= 6
      cfg.clean_fragment_write = clean_fragment_write;
#endif
   }
#else
   struct panfrost_ptr T;
   unsigned nr_tables = 12;

   /* Although individual resources need only 16 byte alignment, the
    * resource table as a whole must be 64-byte aligned.
    */
   T = pan_pool_alloc_aligned(pool, nr_tables * pan_size(RESOURCE), 64);
   memset(T.cpu, 0, nr_tables * pan_size(RESOURCE));

   panfrost_make_resource_table(T, PAN_TABLE_TEXTURE, textures, tex_count);
   panfrost_make_resource_table(T, PAN_TABLE_SAMPLER, samplers, 1);
   panfrost_make_resource_table(T, PAN_TABLE_ATTRIBUTE, varyings, 1);
   panfrost_make_resource_table(T, PAN_TABLE_ATTRIBUTE_BUFFER, varying_buffers,
                                1);

   struct pan_blit_shader_key key = pan_blitter_get_key(&views);
   const struct pan_blit_shader_data *blit_shader =
      pan_blitter_get_blit_shader(pool->dev, &key);

   bool z = fb->zs.preload.z;
   bool s = fb->zs.preload.s;
   bool ms = pan_blitter_is_ms(&views);

   struct panfrost_ptr spd = pan_pool_alloc_desc(pool, SHADER_PROGRAM);
   pan_pack(spd.cpu, SHADER_PROGRAM, cfg) {
      cfg.stage = MALI_SHADER_STAGE_FRAGMENT;
      cfg.fragment_coverage_bitmask_type = MALI_COVERAGE_BITMASK_TYPE_GL;
      cfg.register_allocation = MALI_SHADER_REGISTER_ALLOCATION_32_PER_THREAD;
      cfg.binary = blit_shader->address;
      cfg.preload.r48_r63 = blit_shader->info.preload >> 48;
   }

   unsigned bd_count = views.rt_count;
   struct panfrost_ptr blend = pan_pool_alloc_desc_array(pool, bd_count, BLEND);

   if (!zs) {
      pan_blitter_emit_blends(pool->dev, blit_shader, &views, NULL, blend.cpu);
   }

   pan_pack(out, DRAW, cfg) {
      if (zs) {
         /* ZS_EMIT requires late update/kill */
         cfg.zs_update_operation = MALI_PIXEL_KILL_FORCE_LATE;
         cfg.pixel_kill_operation = MALI_PIXEL_KILL_FORCE_LATE;
         cfg.blend_count = 0;
      } else {
         /* Skipping ATEST requires forcing Z/S */
         cfg.zs_update_operation = MALI_PIXEL_KILL_STRONG_EARLY;
         cfg.pixel_kill_operation = MALI_PIXEL_KILL_FORCE_EARLY;

         cfg.blend = blend.gpu;
         cfg.blend_count = bd_count;
         cfg.render_target_mask = 0x1;
      }

      cfg.allow_forward_pixel_to_kill = !zs;
      cfg.allow_forward_pixel_to_be_killed = true;
      cfg.depth_stencil = pan_blitter_emit_zs(pool, z, s);
      cfg.sample_mask = 0xFFFF;
      cfg.multisample_enable = ms;
      cfg.evaluate_per_sample = ms;
      cfg.maximum_z = 1.0;
      cfg.clean_fragment_write = clean_fragment_write;
      cfg.shader.resources = T.gpu | nr_tables;
      cfg.shader.shader = spd.gpu;
      cfg.shader.thread_storage = tsd;
   }
#endif
}

#if PAN_ARCH <= 7
static void *
pan_blit_emit_tiler_job(struct pan_pool *pool, struct pan_jc *jc,
                        mali_ptr tiler, struct panfrost_ptr *job)
{
   *job = pan_pool_alloc_desc(pool, TILER_JOB);

   pan_section_pack(job->cpu, TILER_JOB, PRIMITIVE, cfg) {
      cfg.draw_mode = MALI_DRAW_MODE_TRIANGLE_STRIP;
      cfg.index_count = 4;
      cfg.job_task_split = 6;
   }

   pan_section_pack(job->cpu, TILER_JOB, PRIMITIVE_SIZE, cfg) {
      cfg.constant = 1.0f;
   }

   void *invoc = pan_section_ptr(job->cpu, TILER_JOB, INVOCATION);
   panfrost_pack_work_groups_compute(invoc, 1, 4, 1, 1, 1, 1, true, false);

#if PAN_ARCH >= 6
   pan_section_pack(job->cpu, TILER_JOB, PADDING, cfg)
      ;
   pan_section_pack(job->cpu, TILER_JOB, TILER, cfg) {
      cfg.address = tiler;
   }
#endif

   pan_jc_add_job(pool, jc, MALI_JOB_TYPE_TILER, false, false, 0, 0, job,
                  false);
   return pan_section_ptr(job->cpu, TILER_JOB, DRAW);
}
#endif

#if PAN_ARCH >= 6
static void
pan_preload_fb_alloc_pre_post_dcds(struct pan_pool *desc_pool,
                                   struct pan_fb_info *fb)
{
   if (fb->bifrost.pre_post.dcds.gpu)
      return;

   fb->bifrost.pre_post.dcds = pan_pool_alloc_desc_array(desc_pool, 3, DRAW);
}

static void
pan_preload_emit_pre_frame_dcd(struct pan_pool *desc_pool,
                               struct pan_fb_info *fb, bool zs, mali_ptr coords,
                               mali_ptr tsd)
{
   unsigned dcd_idx = zs ? 1 : 0;
   pan_preload_fb_alloc_pre_post_dcds(desc_pool, fb);
   assert(fb->bifrost.pre_post.dcds.cpu);
   void *dcd = fb->bifrost.pre_post.dcds.cpu + (dcd_idx * pan_size(DRAW));

   /* We only use crc_rt to determine whether to force writes for updating
    * the CRCs, so use a conservative tile size (16x16).
    */
   int crc_rt = GENX(pan_select_crc_rt)(fb, 16 * 16);

   bool always_write = false;

   /* If CRC data is currently invalid and this batch will make it valid,
    * write even clean tiles to make sure CRC data is updated. */
   if (crc_rt >= 0) {
      bool *valid = fb->rts[crc_rt].crc_valid;
      bool full = !fb->extent.minx && !fb->extent.miny &&
                  fb->extent.maxx == (fb->width - 1) &&
                  fb->extent.maxy == (fb->height - 1);

      if (full && !(*valid))
         always_write = true;
   }

   pan_preload_emit_dcd(desc_pool, fb, zs, coords, tsd, dcd, always_write);
   if (zs) {
      enum pipe_format fmt = fb->zs.view.zs
                                ? fb->zs.view.zs->planes[0]->layout.format
                                : fb->zs.view.s->planes[0]->layout.format;
      bool always = false;

      /* If we're dealing with a combined ZS resource and only one
       * component is cleared, we need to reload the whole surface
       * because the zs_clean_pixel_write_enable flag is set in that
       * case.
       */
      if (util_format_is_depth_and_stencil(fmt) &&
          fb->zs.clear.z != fb->zs.clear.s)
         always = true;

      /* We could use INTERSECT on Bifrost v7 too, but
       * EARLY_ZS_ALWAYS has the advantage of reloading the ZS tile
       * buffer one or more tiles ahead, making ZS data immediately
       * available for any ZS tests taking place in other shaders.
       * Thing's haven't been benchmarked to determine what's
       * preferable (saving bandwidth vs having ZS preloaded
       * earlier), so let's leave it like that for now.
       */
      fb->bifrost.pre_post.modes[dcd_idx] =
         desc_pool->dev->arch > 6
            ? MALI_PRE_POST_FRAME_SHADER_MODE_EARLY_ZS_ALWAYS
         : always ? MALI_PRE_POST_FRAME_SHADER_MODE_ALWAYS
                  : MALI_PRE_POST_FRAME_SHADER_MODE_INTERSECT;
   } else {
      fb->bifrost.pre_post.modes[dcd_idx] =
         always_write ? MALI_PRE_POST_FRAME_SHADER_MODE_ALWAYS
                      : MALI_PRE_POST_FRAME_SHADER_MODE_INTERSECT;
   }
}
#else
static struct panfrost_ptr
pan_preload_emit_tiler_job(struct pan_pool *desc_pool, struct pan_jc *jc,
                           struct pan_fb_info *fb, bool zs, mali_ptr coords,
                           mali_ptr tsd)
{
   struct panfrost_ptr job = pan_pool_alloc_desc(desc_pool, TILER_JOB);

   pan_preload_emit_dcd(desc_pool, fb, zs, coords, tsd,
                        pan_section_ptr(job.cpu, TILER_JOB, DRAW), false);

   pan_section_pack(job.cpu, TILER_JOB, PRIMITIVE, cfg) {
      cfg.draw_mode = MALI_DRAW_MODE_TRIANGLE_STRIP;
      cfg.index_count = 4;
      cfg.job_task_split = 6;
   }

   pan_section_pack(job.cpu, TILER_JOB, PRIMITIVE_SIZE, cfg) {
      cfg.constant = 1.0f;
   }

   void *invoc = pan_section_ptr(job.cpu, TILER_JOB, INVOCATION);
   panfrost_pack_work_groups_compute(invoc, 1, 4, 1, 1, 1, 1, true, false);

   pan_jc_add_job(desc_pool, jc, MALI_JOB_TYPE_TILER, false, false, 0, 0, &job,
                  true);
   return job;
}
#endif

static struct panfrost_ptr
pan_preload_fb_part(struct pan_pool *pool, struct pan_jc *jc,
                    struct pan_fb_info *fb, bool zs, mali_ptr coords,
                    mali_ptr tsd, mali_ptr tiler)
{
   struct panfrost_ptr job = {0};

#if PAN_ARCH >= 6
   pan_preload_emit_pre_frame_dcd(pool, fb, zs, coords, tsd);
#else
   job = pan_preload_emit_tiler_job(pool, jc, fb, zs, coords, tsd);
#endif
   return job;
}

unsigned
GENX(pan_preload_fb)(struct pan_pool *pool, struct pan_jc *jc,
                     struct pan_fb_info *fb, mali_ptr tsd, mali_ptr tiler,
                     struct panfrost_ptr *jobs)
{
   bool preload_zs = pan_preload_needed(fb, true);
   bool preload_rts = pan_preload_needed(fb, false);
   mali_ptr coords;

   if (!preload_zs && !preload_rts)
      return 0;

   float rect[] = {
      0.0, 0.0,        0.0, 1.0, fb->width, 0.0,        0.0, 1.0,
      0.0, fb->height, 0.0, 1.0, fb->width, fb->height, 0.0, 1.0,
   };

   coords = pan_pool_upload_aligned(pool, rect, sizeof(rect), 64);

   unsigned njobs = 0;
   if (preload_zs) {
      struct panfrost_ptr job =
         pan_preload_fb_part(pool, jc, fb, true, coords, tsd, tiler);
      if (jobs && job.cpu)
         jobs[njobs++] = job;
   }

   if (preload_rts) {
      struct panfrost_ptr job =
         pan_preload_fb_part(pool, jc, fb, false, coords, tsd, tiler);
      if (jobs && job.cpu)
         jobs[njobs++] = job;
   }

   return njobs;
}

#if PAN_ARCH <= 7
void
GENX(pan_blit_ctx_init)(struct panfrost_device *dev,
                        const struct pan_blit_info *info,
                        struct pan_pool *blit_pool,
                        struct pan_blit_context *ctx)
{
   memset(ctx, 0, sizeof(*ctx));

   struct pan_image_view sviews[2] = {
      {
         .format = info->src.planes[0].format,
         .planes =
            {
               info->src.planes[0].image,
               info->src.planes[1].image,
               info->src.planes[2].image,
            },
         .dim =
            info->src.planes[0].image->layout.dim == MALI_TEXTURE_DIMENSION_CUBE
               ? MALI_TEXTURE_DIMENSION_2D
               : info->src.planes[0].image->layout.dim,
         .first_level = info->src.level,
         .last_level = info->src.level,
         .first_layer = info->src.start.layer,
         .last_layer = info->src.end.layer,
         .swizzle =
            {
               PIPE_SWIZZLE_X,
               PIPE_SWIZZLE_Y,
               PIPE_SWIZZLE_Z,
               PIPE_SWIZZLE_W,
            },
      },
   };

   struct pan_image_view dview = {
      .format = info->dst.planes[0].format,
      .planes =
         {
            info->dst.planes[0].image,
            info->dst.planes[1].image,
            info->dst.planes[2].image,
         },
      .dim = info->dst.planes[0].image->layout.dim == MALI_TEXTURE_DIMENSION_1D
                ? MALI_TEXTURE_DIMENSION_1D
                : MALI_TEXTURE_DIMENSION_2D,
      .first_level = info->dst.level,
      .last_level = info->dst.level,
      .first_layer = info->dst.start.layer,
      .last_layer = info->dst.start.layer,
      .swizzle =
         {
            PIPE_SWIZZLE_X,
            PIPE_SWIZZLE_Y,
            PIPE_SWIZZLE_Z,
            PIPE_SWIZZLE_W,
         },
   };

   ctx->src.start.x = info->src.start.x;
   ctx->src.start.y = info->src.start.y;
   ctx->src.end.x = info->src.end.x;
   ctx->src.end.y = info->src.end.y;
   ctx->src.dim = sviews[0].dim;

   if (info->dst.planes[0].image->layout.dim == MALI_TEXTURE_DIMENSION_3D) {
      unsigned max_z =
         u_minify(info->dst.planes[0].image->layout.depth, info->dst.level) - 1;

      ctx->z_scale = (float)(info->src.end.z - info->src.start.z) /
                     (info->dst.end.z - info->dst.start.z);
      assert(info->dst.start.z != info->dst.end.z);
      if (info->dst.start.z > info->dst.end.z) {
         ctx->dst.cur_layer = info->dst.start.z - 1;
         ctx->dst.last_layer = info->dst.end.z;
      } else {
         ctx->dst.cur_layer = info->dst.start.z;
         ctx->dst.last_layer = info->dst.end.z - 1;
      }
      ctx->dst.cur_layer = MIN2(MAX2(ctx->dst.cur_layer, 0), max_z);
      ctx->dst.last_layer = MIN2(MAX2(ctx->dst.last_layer, 0), max_z);
      ctx->dst.layer_offset = ctx->dst.cur_layer;
   } else {
      unsigned max_layer = info->dst.planes[0].image->layout.array_size - 1;
      ctx->dst.layer_offset = info->dst.start.layer;
      ctx->dst.cur_layer = info->dst.start.layer;
      ctx->dst.last_layer = MIN2(info->dst.end.layer, max_layer);
      ctx->z_scale = 1;
   }

   if (sviews[0].dim == MALI_TEXTURE_DIMENSION_3D) {
      if (info->src.start.z < info->src.end.z)
         ctx->src.z_offset = info->src.start.z + fabs(ctx->z_scale * 0.5f);
      else
         ctx->src.z_offset = info->src.start.z - fabs(ctx->z_scale * 0.5f);
   } else {
      ctx->src.layer_offset = info->src.start.layer;
   }

   /* Split depth and stencil */
   if (util_format_is_depth_and_stencil(sviews[0].format)) {
      sviews[1] = sviews[0];
      sviews[0].format = util_format_get_depth_only(sviews[0].format);
      sviews[1].format = util_format_stencil_only(sviews[1].format);
   } else if (info->src.planes[1].format) {
      sviews[1] = sviews[0];
      sviews[1].format = info->src.planes[1].format;
      sviews[1].planes[0] = info->src.planes[1].image;
   }

   ctx->rsd = pan_blit_get_rsd(dev, sviews, &dview);

   ASSERTED unsigned nlayers = info->src.end.layer - info->src.start.layer + 1;

   assert(nlayers == (info->dst.end.layer - info->dst.start.layer + 1));

   unsigned dst_w =
      u_minify(info->dst.planes[0].image->layout.width, info->dst.level);
   unsigned dst_h =
      u_minify(info->dst.planes[0].image->layout.height, info->dst.level);
   unsigned maxx = MIN2(MAX2(info->dst.start.x, info->dst.end.x), dst_w - 1);
   unsigned maxy = MIN2(MAX2(info->dst.start.y, info->dst.end.y), dst_h - 1);
   unsigned minx = MAX2(MIN3(info->dst.start.x, info->dst.end.x, maxx), 0);
   unsigned miny = MAX2(MIN3(info->dst.start.y, info->dst.end.y, maxy), 0);

   if (info->scissor.enable) {
      minx = MAX2(minx, info->scissor.minx);
      miny = MAX2(miny, info->scissor.miny);
      maxx = MIN2(maxx, info->scissor.maxx);
      maxy = MIN2(maxy, info->scissor.maxy);
   }

   const struct pan_image_view *sview_ptrs[] = {&sviews[0], &sviews[1]};
   unsigned nviews = sviews[1].format ? 2 : 1;

   ctx->textures = pan_blitter_emit_textures(blit_pool, nviews, sview_ptrs);
   ctx->samplers = pan_blitter_emit_sampler(blit_pool, info->nearest);

   ctx->vpd = pan_blitter_emit_viewport(blit_pool, minx, miny, maxx, maxy);

   float dst_rect[] = {
      info->dst.start.x, info->dst.start.y, 0.0, 1.0,
      info->dst.end.x,   info->dst.start.y, 0.0, 1.0,
      info->dst.start.x, info->dst.end.y,   0.0, 1.0,
      info->dst.end.x,   info->dst.end.y,   0.0, 1.0,
   };

   ctx->position =
      pan_pool_upload_aligned(blit_pool, dst_rect, sizeof(dst_rect), 64);
}

struct panfrost_ptr
GENX(pan_blit)(struct pan_blit_context *ctx, struct pan_pool *pool,
               struct pan_jc *jc, mali_ptr tsd, mali_ptr tiler)
{
   if (ctx->dst.cur_layer < 0 ||
       (ctx->dst.last_layer >= ctx->dst.layer_offset &&
        ctx->dst.cur_layer > ctx->dst.last_layer) ||
       (ctx->dst.last_layer < ctx->dst.layer_offset &&
        ctx->dst.cur_layer < ctx->dst.last_layer))
      return (struct panfrost_ptr){0};

   int32_t layer = ctx->dst.cur_layer - ctx->dst.layer_offset;
   float src_z;
   if (ctx->src.dim == MALI_TEXTURE_DIMENSION_3D)
      src_z = (ctx->z_scale * layer) + ctx->src.z_offset;
   else
      src_z = ctx->src.layer_offset + layer;

   float src_rect[] = {
      ctx->src.start.x, ctx->src.start.y, src_z, 1.0,
      ctx->src.end.x,   ctx->src.start.y, src_z, 1.0,
      ctx->src.start.x, ctx->src.end.y,   src_z, 1.0,
      ctx->src.end.x,   ctx->src.end.y,   src_z, 1.0,
   };

   mali_ptr src_coords =
      pan_pool_upload_aligned(pool, src_rect, sizeof(src_rect), 64);

   struct panfrost_ptr job = {0};
   void *dcd = pan_blit_emit_tiler_job(pool, jc, tiler, &job);

   pan_pack(dcd, DRAW, cfg) {
      cfg.thread_storage = tsd;
      cfg.state = ctx->rsd;

      cfg.position = ctx->position;
      cfg.varyings = pan_blitter_emit_varying(pool);
      cfg.varying_buffers = pan_blitter_emit_varying_buffer(pool, src_coords);
      cfg.viewport = ctx->vpd;
      cfg.textures = ctx->textures;
      cfg.samplers = ctx->samplers;
   }

   return job;
}
#endif

static uint32_t
pan_blit_shader_key_hash(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct pan_blit_shader_key));
}

static bool
pan_blit_shader_key_equal(const void *a, const void *b)
{
   return !memcmp(a, b, sizeof(struct pan_blit_shader_key));
}

static uint32_t
pan_blit_blend_shader_key_hash(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct pan_blit_blend_shader_key));
}

static bool
pan_blit_blend_shader_key_equal(const void *a, const void *b)
{
   return !memcmp(a, b, sizeof(struct pan_blit_blend_shader_key));
}

static uint32_t
pan_blit_rsd_key_hash(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct pan_blit_rsd_key));
}

static bool
pan_blit_rsd_key_equal(const void *a, const void *b)
{
   return !memcmp(a, b, sizeof(struct pan_blit_rsd_key));
}

static void
pan_blitter_prefill_blit_shader_cache(struct panfrost_device *dev)
{
   static const struct pan_blit_shader_key prefill[] = {
      {
         .surfaces[0] =
            {
               .loc = FRAG_RESULT_DEPTH,
               .type = nir_type_float32,
               .dim = MALI_TEXTURE_DIMENSION_2D,
               .src_samples = 1,
               .dst_samples = 1,
            },
      },
      {
         .surfaces[1] =
            {
               .loc = FRAG_RESULT_STENCIL,
               .type = nir_type_uint32,
               .dim = MALI_TEXTURE_DIMENSION_2D,
               .src_samples = 1,
               .dst_samples = 1,
            },
      },
      {
         .surfaces[0] =
            {
               .loc = FRAG_RESULT_DATA0,
               .type = nir_type_float32,
               .dim = MALI_TEXTURE_DIMENSION_2D,
               .src_samples = 1,
               .dst_samples = 1,
            },
      },
   };

   for (unsigned i = 0; i < ARRAY_SIZE(prefill); i++)
      pan_blitter_get_blit_shader(dev, &prefill[i]);
}

void
GENX(pan_blitter_init)(struct panfrost_device *dev, struct pan_pool *bin_pool,
                       struct pan_pool *desc_pool)
{
   dev->blitter.shaders.blit = _mesa_hash_table_create(
      NULL, pan_blit_shader_key_hash, pan_blit_shader_key_equal);
   dev->blitter.shaders.blend = _mesa_hash_table_create(
      NULL, pan_blit_blend_shader_key_hash, pan_blit_blend_shader_key_equal);
   dev->blitter.shaders.pool = bin_pool;
   pthread_mutex_init(&dev->blitter.shaders.lock, NULL);
   pan_blitter_prefill_blit_shader_cache(dev);

   dev->blitter.rsds.pool = desc_pool;
   dev->blitter.rsds.rsds = _mesa_hash_table_create(NULL, pan_blit_rsd_key_hash,
                                                    pan_blit_rsd_key_equal);
   pthread_mutex_init(&dev->blitter.rsds.lock, NULL);
}

void
GENX(pan_blitter_cleanup)(struct panfrost_device *dev)
{
   _mesa_hash_table_destroy(dev->blitter.shaders.blit, NULL);
   _mesa_hash_table_destroy(dev->blitter.shaders.blend, NULL);
   pthread_mutex_destroy(&dev->blitter.shaders.lock);
   _mesa_hash_table_destroy(dev->blitter.rsds.rsds, NULL);
   pthread_mutex_destroy(&dev->blitter.rsds.lock);
}
