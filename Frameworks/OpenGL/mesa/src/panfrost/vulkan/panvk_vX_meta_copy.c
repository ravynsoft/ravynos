/*
 * Copyright © 2021 Collabora Ltd.
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

#include "gen_macros.h"

#include "nir/nir_builder.h"
#include "pan_encoder.h"
#include "pan_shader.h"

#include "panvk_private.h"

static mali_ptr
panvk_meta_copy_img_emit_texture(struct panfrost_device *pdev,
                                 struct pan_pool *desc_pool,
                                 const struct pan_image_view *view)
{
   struct panfrost_ptr texture = pan_pool_alloc_desc(desc_pool, TEXTURE);
   size_t payload_size = GENX(panfrost_estimate_texture_payload_size)(view);
   struct panfrost_ptr surfaces = pan_pool_alloc_aligned(
      desc_pool, payload_size, pan_alignment(SURFACE_WITH_STRIDE));

   GENX(panfrost_new_texture)(pdev, view, texture.cpu, &surfaces);

   return texture.gpu;
}

static mali_ptr
panvk_meta_copy_img_emit_sampler(struct panfrost_device *pdev,
                                 struct pan_pool *desc_pool)
{
   struct panfrost_ptr sampler = pan_pool_alloc_desc(desc_pool, SAMPLER);

   pan_pack(sampler.cpu, SAMPLER, cfg) {
      cfg.seamless_cube_map = false;
      cfg.normalized_coordinates = false;
      cfg.minify_nearest = true;
      cfg.magnify_nearest = true;
   }

   return sampler.gpu;
}

static void
panvk_meta_copy_emit_varying(struct pan_pool *pool, mali_ptr coordinates,
                             mali_ptr *varying_bufs, mali_ptr *varyings)
{
   struct panfrost_ptr varying = pan_pool_alloc_desc(pool, ATTRIBUTE);
   struct panfrost_ptr varying_buffer =
      pan_pool_alloc_desc_array(pool, 2, ATTRIBUTE_BUFFER);

   pan_pack(varying_buffer.cpu, ATTRIBUTE_BUFFER, cfg) {
      cfg.pointer = coordinates;
      cfg.stride = 4 * sizeof(uint32_t);
      cfg.size = cfg.stride * 4;
   }

   /* Bifrost needs an empty desc to mark end of prefetching */
   pan_pack(varying_buffer.cpu + pan_size(ATTRIBUTE_BUFFER), ATTRIBUTE_BUFFER,
            cfg)
      ;

   pan_pack(varying.cpu, ATTRIBUTE, cfg) {
      cfg.buffer_index = 0;
      cfg.format = pool->dev->formats[PIPE_FORMAT_R32G32B32_FLOAT].hw;
   }

   *varyings = varying.gpu;
   *varying_bufs = varying_buffer.gpu;
}

static void
panvk_meta_copy_emit_dcd(struct pan_pool *pool, mali_ptr src_coords,
                         mali_ptr dst_coords, mali_ptr texture,
                         mali_ptr sampler, mali_ptr vpd, mali_ptr tsd,
                         mali_ptr rsd, mali_ptr push_constants, void *out)
{
   pan_pack(out, DRAW, cfg) {
      cfg.thread_storage = tsd;
      cfg.state = rsd;
      cfg.push_uniforms = push_constants;
      cfg.position = dst_coords;
      if (src_coords) {
         panvk_meta_copy_emit_varying(pool, src_coords, &cfg.varying_buffers,
                                      &cfg.varyings);
      }
      cfg.viewport = vpd;
      cfg.textures = texture;
      cfg.samplers = sampler;
   }
}

static struct panfrost_ptr
panvk_meta_copy_emit_tiler_job(struct pan_pool *desc_pool, struct pan_jc *jc,
                               mali_ptr src_coords, mali_ptr dst_coords,
                               mali_ptr texture, mali_ptr sampler,
                               mali_ptr push_constants, mali_ptr vpd,
                               mali_ptr rsd, mali_ptr tsd, mali_ptr tiler)
{
   struct panfrost_ptr job = pan_pool_alloc_desc(desc_pool, TILER_JOB);

   panvk_meta_copy_emit_dcd(desc_pool, src_coords, dst_coords, texture, sampler,
                            vpd, tsd, rsd, push_constants,
                            pan_section_ptr(job.cpu, TILER_JOB, DRAW));

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

   pan_section_pack(job.cpu, TILER_JOB, PADDING, cfg)
      ;
   pan_section_pack(job.cpu, TILER_JOB, TILER, cfg) {
      cfg.address = tiler;
   }

   pan_jc_add_job(desc_pool, jc, MALI_JOB_TYPE_TILER, false, false, 0, 0, &job,
                  false);
   return job;
}

static struct panfrost_ptr
panvk_meta_copy_emit_compute_job(struct pan_pool *desc_pool, struct pan_jc *jc,
                                 const struct pan_compute_dim *num_wg,
                                 const struct pan_compute_dim *wg_sz,
                                 mali_ptr texture, mali_ptr sampler,
                                 mali_ptr push_constants, mali_ptr rsd,
                                 mali_ptr tsd)
{
   struct panfrost_ptr job = pan_pool_alloc_desc(desc_pool, COMPUTE_JOB);

   void *invoc = pan_section_ptr(job.cpu, COMPUTE_JOB, INVOCATION);
   panfrost_pack_work_groups_compute(invoc, num_wg->x, num_wg->y, num_wg->z,
                                     wg_sz->x, wg_sz->y, wg_sz->z, false,
                                     false);

   pan_section_pack(job.cpu, COMPUTE_JOB, PARAMETERS, cfg) {
      cfg.job_task_split = 8;
   }

   panvk_meta_copy_emit_dcd(desc_pool, 0, 0, texture, sampler, 0, tsd, rsd,
                            push_constants,
                            pan_section_ptr(job.cpu, COMPUTE_JOB, DRAW));

   pan_jc_add_job(desc_pool, jc, MALI_JOB_TYPE_COMPUTE, false, false, 0, 0,
                  &job, false);
   return job;
}

static uint32_t
panvk_meta_copy_img_bifrost_raw_format(unsigned texelsize)
{
   switch (texelsize) {
   case 6:
      return MALI_RGB16UI << 12;
   case 8:
      return MALI_RG32UI << 12;
   case 12:
      return MALI_RGB32UI << 12;
   case 16:
      return MALI_RGBA32UI << 12;
   default:
      unreachable("Invalid texel size\n");
   }
}

static mali_ptr
panvk_meta_copy_to_img_emit_rsd(struct panfrost_device *pdev,
                                struct pan_pool *desc_pool, mali_ptr shader,
                                const struct pan_shader_info *shader_info,
                                enum pipe_format fmt, unsigned wrmask,
                                bool from_img)
{
   struct panfrost_ptr rsd_ptr = pan_pool_alloc_desc_aggregate(
      desc_pool, PAN_DESC(RENDERER_STATE), PAN_DESC_ARRAY(1, BLEND));

   bool raw = util_format_get_blocksize(fmt) > 4;
   unsigned fullmask = (1 << util_format_get_nr_components(fmt)) - 1;
   bool partialwrite = fullmask != wrmask && !raw;
   bool readstb = fullmask != wrmask && raw;

   pan_pack(rsd_ptr.cpu, RENDERER_STATE, cfg) {
      pan_shader_prepare_rsd(shader_info, shader, &cfg);
      if (from_img) {
         cfg.shader.varying_count = 1;
         cfg.shader.texture_count = 1;
         cfg.shader.sampler_count = 1;
      }
      cfg.properties.depth_source = MALI_DEPTH_SOURCE_FIXED_FUNCTION;
      cfg.multisample_misc.sample_mask = UINT16_MAX;
      cfg.multisample_misc.depth_function = MALI_FUNC_ALWAYS;
      cfg.stencil_mask_misc.stencil_mask_front = 0xFF;
      cfg.stencil_mask_misc.stencil_mask_back = 0xFF;
      cfg.stencil_front.compare_function = MALI_FUNC_ALWAYS;
      cfg.stencil_front.stencil_fail = MALI_STENCIL_OP_REPLACE;
      cfg.stencil_front.depth_fail = MALI_STENCIL_OP_REPLACE;
      cfg.stencil_front.depth_pass = MALI_STENCIL_OP_REPLACE;
      cfg.stencil_front.mask = 0xFF;
      cfg.stencil_back = cfg.stencil_front;

      cfg.properties.allow_forward_pixel_to_be_killed = true;
      cfg.properties.allow_forward_pixel_to_kill = !partialwrite && !readstb;
      cfg.properties.zs_update_operation = MALI_PIXEL_KILL_STRONG_EARLY;
      cfg.properties.pixel_kill_operation = MALI_PIXEL_KILL_FORCE_EARLY;
   }

   pan_pack(rsd_ptr.cpu + pan_size(RENDERER_STATE), BLEND, cfg) {
      cfg.round_to_fb_precision = true;
      cfg.load_destination = partialwrite;
      cfg.equation.rgb.a = MALI_BLEND_OPERAND_A_SRC;
      cfg.equation.rgb.b = MALI_BLEND_OPERAND_B_SRC;
      cfg.equation.rgb.c = MALI_BLEND_OPERAND_C_ZERO;
      cfg.equation.alpha.a = MALI_BLEND_OPERAND_A_SRC;
      cfg.equation.alpha.b = MALI_BLEND_OPERAND_B_SRC;
      cfg.equation.alpha.c = MALI_BLEND_OPERAND_C_ZERO;
      cfg.internal.mode =
         partialwrite ? MALI_BLEND_MODE_FIXED_FUNCTION : MALI_BLEND_MODE_OPAQUE;
      cfg.equation.color_mask = partialwrite ? wrmask : 0xf;
      cfg.internal.fixed_function.num_comps = 4;
      if (!raw) {
         cfg.internal.fixed_function.conversion.memory_format =
            panfrost_format_to_bifrost_blend(pdev, fmt, false);
         cfg.internal.fixed_function.conversion.register_format =
            MALI_REGISTER_FILE_FORMAT_F32;
      } else {
         unsigned imgtexelsz = util_format_get_blocksize(fmt);

         cfg.internal.fixed_function.conversion.memory_format =
            panvk_meta_copy_img_bifrost_raw_format(imgtexelsz);
         cfg.internal.fixed_function.conversion.register_format =
            (imgtexelsz & 2) ? MALI_REGISTER_FILE_FORMAT_U16
                             : MALI_REGISTER_FILE_FORMAT_U32;
      }
   }

   return rsd_ptr.gpu;
}

static mali_ptr
panvk_meta_copy_to_buf_emit_rsd(struct panfrost_device *pdev,
                                struct pan_pool *desc_pool, mali_ptr shader,
                                const struct pan_shader_info *shader_info,
                                bool from_img)
{
   struct panfrost_ptr rsd_ptr =
      pan_pool_alloc_desc_aggregate(desc_pool, PAN_DESC(RENDERER_STATE));

   pan_pack(rsd_ptr.cpu, RENDERER_STATE, cfg) {
      pan_shader_prepare_rsd(shader_info, shader, &cfg);
      if (from_img) {
         cfg.shader.texture_count = 1;
         cfg.shader.sampler_count = 1;
      }
   }

   return rsd_ptr.gpu;
}

static mali_ptr
panvk_meta_copy_img2img_shader(struct panfrost_device *pdev,
                               struct pan_pool *bin_pool,
                               enum pipe_format srcfmt, enum pipe_format dstfmt,
                               unsigned dstmask, unsigned texdim,
                               bool texisarray, bool is_ms,
                               struct pan_shader_info *shader_info)
{
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_FRAGMENT, GENX(pan_shader_get_compiler_options)(),
      "panvk_meta_copy_img2img(srcfmt=%s,dstfmt=%s,%dD%s%s)",
      util_format_name(srcfmt), util_format_name(dstfmt), texdim,
      texisarray ? "[]" : "", is_ms ? ",ms" : "");

   nir_variable *coord_var = nir_variable_create(
      b.shader, nir_var_shader_in,
      glsl_vector_type(GLSL_TYPE_FLOAT, texdim + texisarray), "coord");
   coord_var->data.location = VARYING_SLOT_VAR0;
   nir_def *coord = nir_f2u32(&b, nir_load_var(&b, coord_var));

   nir_tex_instr *tex = nir_tex_instr_create(b.shader, is_ms ? 2 : 1);
   tex->op = is_ms ? nir_texop_txf_ms : nir_texop_txf;
   tex->texture_index = 0;
   tex->is_array = texisarray;
   tex->dest_type =
      util_format_is_unorm(srcfmt) ? nir_type_float32 : nir_type_uint32;

   switch (texdim) {
   case 1:
      assert(!is_ms);
      tex->sampler_dim = GLSL_SAMPLER_DIM_1D;
      break;
   case 2:
      tex->sampler_dim = is_ms ? GLSL_SAMPLER_DIM_MS : GLSL_SAMPLER_DIM_2D;
      break;
   case 3:
      assert(!is_ms);
      tex->sampler_dim = GLSL_SAMPLER_DIM_3D;
      break;
   default:
      unreachable("Invalid texture dimension");
   }

   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord, coord);
   tex->coord_components = texdim + texisarray;

   if (is_ms) {
      tex->src[1] =
         nir_tex_src_for_ssa(nir_tex_src_ms_index, nir_load_sample_id(&b));
   }

   nir_def_init(&tex->instr, &tex->def, 4,
                nir_alu_type_get_type_size(tex->dest_type));
   nir_builder_instr_insert(&b, &tex->instr);

   nir_def *texel = &tex->def;

   unsigned dstcompsz =
      util_format_get_component_bits(dstfmt, UTIL_FORMAT_COLORSPACE_RGB, 0);
   unsigned ndstcomps = util_format_get_nr_components(dstfmt);
   const struct glsl_type *outtype = NULL;

   if (srcfmt == PIPE_FORMAT_R5G6B5_UNORM && dstfmt == PIPE_FORMAT_R8G8_UNORM) {
      nir_def *rgb = nir_f2u32(
         &b, nir_fmul(&b, texel,
                      nir_vec3(&b, nir_imm_float(&b, 31), nir_imm_float(&b, 63),
                               nir_imm_float(&b, 31))));
      nir_def *rg = nir_vec2(
         &b,
         nir_ior(&b, nir_channel(&b, rgb, 0),
                 nir_ishl(&b, nir_channel(&b, rgb, 1), nir_imm_int(&b, 5))),
         nir_ior(&b, nir_ushr_imm(&b, nir_channel(&b, rgb, 1), 3),
                 nir_ishl(&b, nir_channel(&b, rgb, 2), nir_imm_int(&b, 3))));
      rg = nir_iand_imm(&b, rg, 255);
      texel = nir_fmul_imm(&b, nir_u2f32(&b, rg), 1.0 / 255);
      outtype = glsl_vector_type(GLSL_TYPE_FLOAT, 2);
   } else if (srcfmt == PIPE_FORMAT_R8G8_UNORM &&
              dstfmt == PIPE_FORMAT_R5G6B5_UNORM) {
      nir_def *rg = nir_f2u32(&b, nir_fmul_imm(&b, texel, 255));
      nir_def *rgb = nir_vec3(
         &b, nir_channel(&b, rg, 0),
         nir_ior(&b, nir_ushr_imm(&b, nir_channel(&b, rg, 0), 5),
                 nir_ishl(&b, nir_channel(&b, rg, 1), nir_imm_int(&b, 3))),
         nir_ushr_imm(&b, nir_channel(&b, rg, 1), 3));
      rgb = nir_iand(&b, rgb,
                     nir_vec3(&b, nir_imm_int(&b, 31), nir_imm_int(&b, 63),
                              nir_imm_int(&b, 31)));
      texel = nir_fmul(
         &b, nir_u2f32(&b, rgb),
         nir_vec3(&b, nir_imm_float(&b, 1.0 / 31), nir_imm_float(&b, 1.0 / 63),
                  nir_imm_float(&b, 1.0 / 31)));
      outtype = glsl_vector_type(GLSL_TYPE_FLOAT, 3);
   } else {
      assert(srcfmt == dstfmt);
      enum glsl_base_type basetype;
      if (util_format_is_unorm(dstfmt)) {
         basetype = GLSL_TYPE_FLOAT;
      } else if (dstcompsz == 16) {
         basetype = GLSL_TYPE_UINT16;
      } else {
         assert(dstcompsz == 32);
         basetype = GLSL_TYPE_UINT;
      }

      if (dstcompsz == 16)
         texel = nir_u2u16(&b, texel);

      texel = nir_trim_vector(&b, texel, ndstcomps);
      outtype = glsl_vector_type(basetype, ndstcomps);
   }

   nir_variable *out =
      nir_variable_create(b.shader, nir_var_shader_out, outtype, "out");
   out->data.location = FRAG_RESULT_DATA0;

   unsigned fullmask = (1 << ndstcomps) - 1;
   if (dstcompsz > 8 && dstmask != fullmask) {
      nir_def *oldtexel = nir_load_var(&b, out);
      nir_def *dstcomps[4];

      for (unsigned i = 0; i < ndstcomps; i++) {
         if (dstmask & BITFIELD_BIT(i))
            dstcomps[i] = nir_channel(&b, texel, i);
         else
            dstcomps[i] = nir_channel(&b, oldtexel, i);
      }

      texel = nir_vec(&b, dstcomps, ndstcomps);
   }

   nir_store_var(&b, out, texel, 0xff);

   struct panfrost_compile_inputs inputs = {
      .gpu_id = panfrost_device_gpu_id(pdev),
      .is_blit = true,
      .no_ubo_to_push = true,
   };

   struct util_dynarray binary;

   util_dynarray_init(&binary, NULL);
   pan_shader_preprocess(b.shader, inputs.gpu_id);
   NIR_PASS_V(b.shader, GENX(pan_inline_rt_conversion), pdev, &dstfmt);
   GENX(pan_shader_compile)(b.shader, &inputs, &binary, shader_info);

   shader_info->fs.sample_shading = is_ms;

   mali_ptr shader =
      pan_pool_upload_aligned(bin_pool, binary.data, binary.size, 128);

   util_dynarray_fini(&binary);
   ralloc_free(b.shader);

   return shader;
}

static enum pipe_format
panvk_meta_copy_img_format(enum pipe_format fmt)
{
   /* We can't use a non-compressed format when handling a tiled/AFBC
    * compressed format because the tile size differ (4x4 blocks for
    * compressed formats and 16x16 texels for non-compressed ones).
    */
   assert(!util_format_is_compressed(fmt));

   /* Pick blendable formats when we can, otherwise pick the UINT variant
    * matching the texel size.
    */
   switch (util_format_get_blocksize(fmt)) {
   case 16:
      return PIPE_FORMAT_R32G32B32A32_UINT;
   case 12:
      return PIPE_FORMAT_R32G32B32_UINT;
   case 8:
      return PIPE_FORMAT_R32G32_UINT;
   case 6:
      return PIPE_FORMAT_R16G16B16_UINT;
   case 4:
      return PIPE_FORMAT_R8G8B8A8_UNORM;
   case 2:
      return (fmt == PIPE_FORMAT_R5G6B5_UNORM ||
              fmt == PIPE_FORMAT_B5G6R5_UNORM)
                ? PIPE_FORMAT_R5G6B5_UNORM
                : PIPE_FORMAT_R8G8_UNORM;
   case 1:
      return PIPE_FORMAT_R8_UNORM;
   default:
      unreachable("Unsupported format\n");
   }
}

struct panvk_meta_copy_img2img_format_info {
   enum pipe_format srcfmt;
   enum pipe_format dstfmt;
   unsigned dstmask;
} PACKED;

static const struct panvk_meta_copy_img2img_format_info
   panvk_meta_copy_img2img_fmts[] = {
      {PIPE_FORMAT_R8_UNORM, PIPE_FORMAT_R8_UNORM, 0x1},
      {PIPE_FORMAT_R5G6B5_UNORM, PIPE_FORMAT_R5G6B5_UNORM, 0x7},
      {PIPE_FORMAT_R5G6B5_UNORM, PIPE_FORMAT_R8G8_UNORM, 0x3},
      {PIPE_FORMAT_R8G8_UNORM, PIPE_FORMAT_R5G6B5_UNORM, 0x7},
      {PIPE_FORMAT_R8G8_UNORM, PIPE_FORMAT_R8G8_UNORM, 0x3},
      /* Z24S8(depth) */
      {PIPE_FORMAT_R8G8B8A8_UNORM, PIPE_FORMAT_R8G8B8A8_UNORM, 0x7},
      /* Z24S8(stencil) */
      {PIPE_FORMAT_R8G8B8A8_UNORM, PIPE_FORMAT_R8G8B8A8_UNORM, 0x8},
      {PIPE_FORMAT_R8G8B8A8_UNORM, PIPE_FORMAT_R8G8B8A8_UNORM, 0xf},
      {PIPE_FORMAT_R16G16B16_UINT, PIPE_FORMAT_R16G16B16_UINT, 0x7},
      {PIPE_FORMAT_R32G32_UINT, PIPE_FORMAT_R32G32_UINT, 0x3},
      /* Z32S8X24(depth) */
      {PIPE_FORMAT_R32G32_UINT, PIPE_FORMAT_R32G32_UINT, 0x1},
      /* Z32S8X24(stencil) */
      {PIPE_FORMAT_R32G32_UINT, PIPE_FORMAT_R32G32_UINT, 0x2},
      {PIPE_FORMAT_R32G32B32_UINT, PIPE_FORMAT_R32G32B32_UINT, 0x7},
      {PIPE_FORMAT_R32G32B32A32_UINT, PIPE_FORMAT_R32G32B32A32_UINT, 0xf},
};

static unsigned
panvk_meta_copy_img2img_format_idx(
   struct panvk_meta_copy_img2img_format_info key)
{
   STATIC_ASSERT(ARRAY_SIZE(panvk_meta_copy_img2img_fmts) ==
                 PANVK_META_COPY_IMG2IMG_NUM_FORMATS);

   for (unsigned i = 0; i < ARRAY_SIZE(panvk_meta_copy_img2img_fmts); i++) {
      if (!memcmp(&key, &panvk_meta_copy_img2img_fmts[i], sizeof(key)))
         return i;
   }

   unreachable("Invalid image format\n");
}

static unsigned
panvk_meta_copy_img_mask(enum pipe_format imgfmt, VkImageAspectFlags aspectMask)
{
   if (aspectMask != VK_IMAGE_ASPECT_DEPTH_BIT &&
       aspectMask != VK_IMAGE_ASPECT_STENCIL_BIT) {
      enum pipe_format outfmt = panvk_meta_copy_img_format(imgfmt);

      return (1 << util_format_get_nr_components(outfmt)) - 1;
   }

   switch (imgfmt) {
   case PIPE_FORMAT_S8_UINT:
      return 1;
   case PIPE_FORMAT_Z16_UNORM:
      return 3;
   case PIPE_FORMAT_Z16_UNORM_S8_UINT:
      return aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT ? 3 : 8;
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      return aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT ? 7 : 8;
   case PIPE_FORMAT_Z24X8_UNORM:
      assert(aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT);
      return 7;
   case PIPE_FORMAT_Z32_FLOAT:
      return 0xf;
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      return aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT ? 1 : 2;
   default:
      unreachable("Invalid depth format\n");
   }
}

static void
panvk_meta_copy_img2img(struct panvk_cmd_buffer *cmdbuf,
                        const struct panvk_image *src,
                        const struct panvk_image *dst,
                        const VkImageCopy2 *region)
{
   struct panfrost_device *pdev = &cmdbuf->device->physical_device->pdev;
   struct pan_fb_info *fbinfo = &cmdbuf->state.fb.info;
   struct panvk_meta_copy_img2img_format_info key = {
      .srcfmt = panvk_meta_copy_img_format(src->pimage.layout.format),
      .dstfmt = panvk_meta_copy_img_format(dst->pimage.layout.format),
      .dstmask = panvk_meta_copy_img_mask(dst->pimage.layout.format,
                                          region->dstSubresource.aspectMask),
   };

   assert(src->pimage.layout.nr_samples == dst->pimage.layout.nr_samples);

   unsigned texdimidx = panvk_meta_copy_tex_type(
      src->pimage.layout.dim, src->pimage.layout.array_size > 1);
   unsigned fmtidx = panvk_meta_copy_img2img_format_idx(key);
   unsigned ms = dst->pimage.layout.nr_samples > 1 ? 1 : 0;

   mali_ptr rsd =
      cmdbuf->device->physical_device->meta.copy.img2img[ms][texdimidx][fmtidx]
         .rsd;

   struct pan_image_view srcview = {
      .format = key.srcfmt,
      .dim = src->pimage.layout.dim == MALI_TEXTURE_DIMENSION_CUBE
                ? MALI_TEXTURE_DIMENSION_2D
                : src->pimage.layout.dim,
      .planes[0] = &src->pimage,
      .nr_samples = src->pimage.layout.nr_samples,
      .first_level = region->srcSubresource.mipLevel,
      .last_level = region->srcSubresource.mipLevel,
      .first_layer = region->srcSubresource.baseArrayLayer,
      .last_layer = region->srcSubresource.baseArrayLayer +
                    region->srcSubresource.layerCount - 1,
      .swizzle = {PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y, PIPE_SWIZZLE_Z,
                  PIPE_SWIZZLE_W},
   };

   struct pan_image_view dstview = {
      .format = key.dstfmt,
      .dim = MALI_TEXTURE_DIMENSION_2D,
      .planes[0] = &dst->pimage,
      .nr_samples = dst->pimage.layout.nr_samples,
      .first_level = region->dstSubresource.mipLevel,
      .last_level = region->dstSubresource.mipLevel,
      .swizzle = {PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y, PIPE_SWIZZLE_Z,
                  PIPE_SWIZZLE_W},
   };

   unsigned minx = MAX2(region->dstOffset.x, 0);
   unsigned miny = MAX2(region->dstOffset.y, 0);
   unsigned maxx = MAX2(region->dstOffset.x + region->extent.width - 1, 0);
   unsigned maxy = MAX2(region->dstOffset.y + region->extent.height - 1, 0);

   mali_ptr vpd = panvk_per_arch(meta_emit_viewport)(&cmdbuf->desc_pool.base,
                                                     minx, miny, maxx, maxy);

   float dst_rect[] = {
      minx, miny,     0.0, 1.0, maxx + 1, miny,     0.0, 1.0,
      minx, maxy + 1, 0.0, 1.0, maxx + 1, maxy + 1, 0.0, 1.0,
   };

   mali_ptr dst_coords = pan_pool_upload_aligned(
      &cmdbuf->desc_pool.base, dst_rect, sizeof(dst_rect), 64);

   /* TODO: don't force preloads of dst resources if unneeded */

   unsigned width =
      u_minify(dst->pimage.layout.width, region->dstSubresource.mipLevel);
   unsigned height =
      u_minify(dst->pimage.layout.height, region->dstSubresource.mipLevel);
   cmdbuf->state.fb.crc_valid[0] = false;
   *fbinfo = (struct pan_fb_info){
      .width = width,
      .height = height,
      .extent.minx = minx & ~31,
      .extent.miny = miny & ~31,
      .extent.maxx = MIN2(ALIGN_POT(maxx + 1, 32), width) - 1,
      .extent.maxy = MIN2(ALIGN_POT(maxy + 1, 32), height) - 1,
      .nr_samples = dst->pimage.layout.nr_samples,
      .rt_count = 1,
      .rts[0].view = &dstview,
      .rts[0].preload = true,
      .rts[0].crc_valid = &cmdbuf->state.fb.crc_valid[0],
   };

   mali_ptr texture =
      panvk_meta_copy_img_emit_texture(pdev, &cmdbuf->desc_pool.base, &srcview);
   mali_ptr sampler =
      panvk_meta_copy_img_emit_sampler(pdev, &cmdbuf->desc_pool.base);

   panvk_per_arch(cmd_close_batch)(cmdbuf);

   minx = MAX2(region->srcOffset.x, 0);
   miny = MAX2(region->srcOffset.y, 0);
   maxx = MAX2(region->srcOffset.x + region->extent.width - 1, 0);
   maxy = MAX2(region->srcOffset.y + region->extent.height - 1, 0);
   assert(region->dstOffset.z >= 0);

   unsigned first_src_layer = MAX2(0, region->srcOffset.z);
   unsigned first_dst_layer =
      MAX2(region->dstSubresource.baseArrayLayer, region->dstOffset.z);
   unsigned nlayers =
      MAX2(region->dstSubresource.layerCount, region->extent.depth);
   for (unsigned l = 0; l < nlayers; l++) {
      unsigned src_l = l + first_src_layer;
      float src_rect[] = {
         minx, miny,     src_l, 1.0, maxx + 1, miny,     src_l, 1.0,
         minx, maxy + 1, src_l, 1.0, maxx + 1, maxy + 1, src_l, 1.0,
      };

      mali_ptr src_coords = pan_pool_upload_aligned(
         &cmdbuf->desc_pool.base, src_rect, sizeof(src_rect), 64);

      struct panvk_batch *batch = panvk_cmd_open_batch(cmdbuf);

      dstview.first_layer = dstview.last_layer = l + first_dst_layer;
      batch->blit.src = src->pimage.data.bo;
      batch->blit.dst = dst->pimage.data.bo;
      panvk_per_arch(cmd_alloc_tls_desc)(cmdbuf, true);
      panvk_per_arch(cmd_alloc_fb_desc)(cmdbuf);
      panvk_per_arch(cmd_prepare_tiler_context)(cmdbuf);

      mali_ptr tsd, tiler;

      tsd = batch->tls.gpu;
      tiler = batch->tiler.descs.gpu;

      struct panfrost_ptr job;

      job = panvk_meta_copy_emit_tiler_job(&cmdbuf->desc_pool.base, &batch->jc,
                                           src_coords, dst_coords, texture,
                                           sampler, 0, vpd, rsd, tsd, tiler);

      util_dynarray_append(&batch->jobs, void *, job.cpu);
      panvk_per_arch(cmd_close_batch)(cmdbuf);
   }
}

static void
panvk_meta_copy_img2img_init(struct panvk_physical_device *dev, bool is_ms)
{
   STATIC_ASSERT(ARRAY_SIZE(panvk_meta_copy_img2img_fmts) ==
                 PANVK_META_COPY_IMG2IMG_NUM_FORMATS);

   for (unsigned i = 0; i < ARRAY_SIZE(panvk_meta_copy_img2img_fmts); i++) {
      for (unsigned texdim = 1; texdim <= 3; texdim++) {
         unsigned texdimidx = panvk_meta_copy_tex_type(texdim, false);
         assert(texdimidx < ARRAY_SIZE(dev->meta.copy.img2img[0]));

         /* No MSAA on 1D/3D textures */
         if (texdim != 2 && is_ms)
            continue;

         struct pan_shader_info shader_info;
         mali_ptr shader = panvk_meta_copy_img2img_shader(
            &dev->pdev, &dev->meta.bin_pool.base,
            panvk_meta_copy_img2img_fmts[i].srcfmt,
            panvk_meta_copy_img2img_fmts[i].dstfmt,
            panvk_meta_copy_img2img_fmts[i].dstmask, texdim, false, is_ms,
            &shader_info);
         dev->meta.copy.img2img[is_ms][texdimidx][i].rsd =
            panvk_meta_copy_to_img_emit_rsd(
               &dev->pdev, &dev->meta.desc_pool.base, shader, &shader_info,
               panvk_meta_copy_img2img_fmts[i].dstfmt,
               panvk_meta_copy_img2img_fmts[i].dstmask, true);
         if (texdim == 3)
            continue;

         memset(&shader_info, 0, sizeof(shader_info));
         texdimidx = panvk_meta_copy_tex_type(texdim, true);
         assert(texdimidx < ARRAY_SIZE(dev->meta.copy.img2img[0]));
         shader = panvk_meta_copy_img2img_shader(
            &dev->pdev, &dev->meta.bin_pool.base,
            panvk_meta_copy_img2img_fmts[i].srcfmt,
            panvk_meta_copy_img2img_fmts[i].dstfmt,
            panvk_meta_copy_img2img_fmts[i].dstmask, texdim, true, is_ms,
            &shader_info);
         dev->meta.copy.img2img[is_ms][texdimidx][i].rsd =
            panvk_meta_copy_to_img_emit_rsd(
               &dev->pdev, &dev->meta.desc_pool.base, shader, &shader_info,
               panvk_meta_copy_img2img_fmts[i].dstfmt,
               panvk_meta_copy_img2img_fmts[i].dstmask, true);
      }
   }
}

void
panvk_per_arch(CmdCopyImage2)(VkCommandBuffer commandBuffer,
                              const VkCopyImageInfo2 *pCopyImageInfo)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   VK_FROM_HANDLE(panvk_image, dst, pCopyImageInfo->dstImage);
   VK_FROM_HANDLE(panvk_image, src, pCopyImageInfo->srcImage);

   for (unsigned i = 0; i < pCopyImageInfo->regionCount; i++) {
      panvk_meta_copy_img2img(cmdbuf, src, dst, &pCopyImageInfo->pRegions[i]);
   }
}

static unsigned
panvk_meta_copy_buf_texelsize(enum pipe_format imgfmt, unsigned mask)
{
   unsigned imgtexelsz = util_format_get_blocksize(imgfmt);
   unsigned nbufcomps = util_bitcount(mask);

   if (nbufcomps == util_format_get_nr_components(imgfmt))
      return imgtexelsz;

   /* Special case for Z24 buffers which are not tightly packed */
   if (mask == 7 && imgtexelsz == 4)
      return 4;

   /* Special case for S8 extraction from Z32_S8X24 */
   if (mask == 2 && imgtexelsz == 8)
      return 1;

   unsigned compsz =
      util_format_get_component_bits(imgfmt, UTIL_FORMAT_COLORSPACE_RGB, 0);

   assert(!(compsz % 8));

   return nbufcomps * compsz / 8;
}

static enum pipe_format
panvk_meta_copy_buf2img_format(enum pipe_format imgfmt)
{
   /* Pick blendable formats when we can, and the FLOAT variant matching the
    * texelsize otherwise.
    */
   switch (util_format_get_blocksize(imgfmt)) {
   case 1:
      return PIPE_FORMAT_R8_UNORM;
   /* AFBC stores things differently for RGB565,
    * we can't simply map to R8G8 in that case */
   case 2:
      return (imgfmt == PIPE_FORMAT_R5G6B5_UNORM ||
              imgfmt == PIPE_FORMAT_B5G6R5_UNORM)
                ? PIPE_FORMAT_R5G6B5_UNORM
                : PIPE_FORMAT_R8G8_UNORM;
   case 4:
      return PIPE_FORMAT_R8G8B8A8_UNORM;
   case 6:
      return PIPE_FORMAT_R16G16B16_UINT;
   case 8:
      return PIPE_FORMAT_R32G32_UINT;
   case 12:
      return PIPE_FORMAT_R32G32B32_UINT;
   case 16:
      return PIPE_FORMAT_R32G32B32A32_UINT;
   default:
      unreachable("Invalid format\n");
   }
}

struct panvk_meta_copy_format_info {
   enum pipe_format imgfmt;
   unsigned mask;
} PACKED;

static const struct panvk_meta_copy_format_info panvk_meta_copy_buf2img_fmts[] =
   {
      {PIPE_FORMAT_R8_UNORM, 0x1},
      {PIPE_FORMAT_R8G8_UNORM, 0x3},
      {PIPE_FORMAT_R5G6B5_UNORM, 0x7},
      {PIPE_FORMAT_R8G8B8A8_UNORM, 0xf},
      {PIPE_FORMAT_R16G16B16_UINT, 0x7},
      {PIPE_FORMAT_R32G32_UINT, 0x3},
      {PIPE_FORMAT_R32G32B32_UINT, 0x7},
      {PIPE_FORMAT_R32G32B32A32_UINT, 0xf},
      /* S8 -> Z24S8 */
      {PIPE_FORMAT_R8G8B8A8_UNORM, 0x8},
      /* S8 -> Z32_S8X24 */
      {PIPE_FORMAT_R32G32_UINT, 0x2},
      /* Z24X8 -> Z24S8 */
      {PIPE_FORMAT_R8G8B8A8_UNORM, 0x7},
      /* Z32 -> Z32_S8X24 */
      {PIPE_FORMAT_R32G32_UINT, 0x1},
};

struct panvk_meta_copy_buf2img_info {
   struct {
      mali_ptr ptr;
      struct {
         unsigned line;
         unsigned surf;
      } stride;
   } buf;
} PACKED;

#define panvk_meta_copy_buf2img_get_info_field(b, field)                       \
   nir_load_push_constant(                                                     \
      (b), 1, sizeof(((struct panvk_meta_copy_buf2img_info *)0)->field) * 8,   \
      nir_imm_int(b, 0),                                                       \
      .base = offsetof(struct panvk_meta_copy_buf2img_info, field),            \
      .range = ~0)

static mali_ptr
panvk_meta_copy_buf2img_shader(struct panfrost_device *pdev,
                               struct pan_pool *bin_pool,
                               struct panvk_meta_copy_format_info key,
                               struct pan_shader_info *shader_info)
{
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_FRAGMENT, GENX(pan_shader_get_compiler_options)(),
      "panvk_meta_copy_buf2img(imgfmt=%s,mask=%x)",
      util_format_name(key.imgfmt), key.mask);

   nir_variable *coord_var =
      nir_variable_create(b.shader, nir_var_shader_in,
                          glsl_vector_type(GLSL_TYPE_FLOAT, 3), "coord");
   coord_var->data.location = VARYING_SLOT_VAR0;
   nir_def *coord = nir_load_var(&b, coord_var);

   coord = nir_f2u32(&b, coord);

   nir_def *bufptr = panvk_meta_copy_buf2img_get_info_field(&b, buf.ptr);
   nir_def *buflinestride =
      panvk_meta_copy_buf2img_get_info_field(&b, buf.stride.line);
   nir_def *bufsurfstride =
      panvk_meta_copy_buf2img_get_info_field(&b, buf.stride.surf);

   unsigned imgtexelsz = util_format_get_blocksize(key.imgfmt);
   unsigned buftexelsz = panvk_meta_copy_buf_texelsize(key.imgfmt, key.mask);
   unsigned writemask = key.mask;

   nir_def *offset =
      nir_imul(&b, nir_channel(&b, coord, 0), nir_imm_int(&b, buftexelsz));
   offset = nir_iadd(&b, offset,
                     nir_imul(&b, nir_channel(&b, coord, 1), buflinestride));
   offset = nir_iadd(&b, offset,
                     nir_imul(&b, nir_channel(&b, coord, 2), bufsurfstride));
   bufptr = nir_iadd(&b, bufptr, nir_u2u64(&b, offset));

   unsigned imgcompsz =
      (imgtexelsz <= 4 && key.imgfmt != PIPE_FORMAT_R5G6B5_UNORM)
         ? 1
         : MIN2(1 << (ffs(imgtexelsz) - 1), 4);

   unsigned nimgcomps = imgtexelsz / imgcompsz;
   unsigned bufcompsz = MIN2(buftexelsz, imgcompsz);
   unsigned nbufcomps = buftexelsz / bufcompsz;

   assert(bufcompsz == 1 || bufcompsz == 2 || bufcompsz == 4);
   assert(nbufcomps <= 4 && nimgcomps <= 4);

   nir_def *texel =
      nir_load_global(&b, bufptr, bufcompsz, nbufcomps, bufcompsz * 8);

   enum glsl_base_type basetype;
   if (key.imgfmt == PIPE_FORMAT_R5G6B5_UNORM) {
      texel = nir_vec3(
         &b, nir_iand_imm(&b, texel, BITFIELD_MASK(5)),
         nir_iand_imm(&b, nir_ushr_imm(&b, texel, 5), BITFIELD_MASK(6)),
         nir_iand_imm(&b, nir_ushr_imm(&b, texel, 11), BITFIELD_MASK(5)));
      texel = nir_fmul(
         &b, nir_u2f32(&b, texel),
         nir_vec3(&b, nir_imm_float(&b, 1.0f / 31),
                  nir_imm_float(&b, 1.0f / 63), nir_imm_float(&b, 1.0f / 31)));
      nimgcomps = 3;
      basetype = GLSL_TYPE_FLOAT;
   } else if (imgcompsz == 1) {
      assert(bufcompsz == 1);
      /* Blendable formats are unorm and the fixed-function blend unit
       * takes float values.
       */
      texel = nir_fmul_imm(&b, nir_u2f32(&b, texel), 1.0f / 255);
      basetype = GLSL_TYPE_FLOAT;
   } else {
      texel = nir_u2uN(&b, texel, imgcompsz * 8);
      basetype = imgcompsz == 2 ? GLSL_TYPE_UINT16 : GLSL_TYPE_UINT;
   }

   /* We always pass the texel using 32-bit regs for now */
   nir_variable *out =
      nir_variable_create(b.shader, nir_var_shader_out,
                          glsl_vector_type(basetype, nimgcomps), "out");
   out->data.location = FRAG_RESULT_DATA0;

   uint16_t fullmask = (1 << nimgcomps) - 1;

   assert(fullmask >= writemask);

   if (fullmask != writemask) {
      unsigned first_written_comp = ffs(writemask) - 1;
      nir_def *oldtexel = NULL;
      if (imgcompsz > 1)
         oldtexel = nir_load_var(&b, out);

      nir_def *texel_comps[4];
      for (unsigned i = 0; i < nimgcomps; i++) {
         if (writemask & BITFIELD_BIT(i))
            texel_comps[i] = nir_channel(&b, texel, i - first_written_comp);
         else if (imgcompsz > 1)
            texel_comps[i] = nir_channel(&b, oldtexel, i);
         else
            texel_comps[i] = nir_imm_intN_t(&b, 0, texel->bit_size);
      }

      texel = nir_vec(&b, texel_comps, nimgcomps);
   }

   nir_store_var(&b, out, texel, 0xff);

   struct panfrost_compile_inputs inputs = {
      .gpu_id = panfrost_device_gpu_id(pdev),
      .is_blit = true,
      .no_ubo_to_push = true,
   };

   struct util_dynarray binary;

   util_dynarray_init(&binary, NULL);
   pan_shader_preprocess(b.shader, inputs.gpu_id);

   enum pipe_format rt_formats[8] = {key.imgfmt};
   NIR_PASS_V(b.shader, GENX(pan_inline_rt_conversion), pdev, rt_formats);

   GENX(pan_shader_compile)(b.shader, &inputs, &binary, shader_info);
   shader_info->push.count =
      DIV_ROUND_UP(sizeof(struct panvk_meta_copy_buf2img_info), 4);

   mali_ptr shader =
      pan_pool_upload_aligned(bin_pool, binary.data, binary.size, 128);

   util_dynarray_fini(&binary);
   ralloc_free(b.shader);

   return shader;
}

static unsigned
panvk_meta_copy_buf2img_format_idx(struct panvk_meta_copy_format_info key)
{
   for (unsigned i = 0; i < ARRAY_SIZE(panvk_meta_copy_buf2img_fmts); i++) {
      if (!memcmp(&key, &panvk_meta_copy_buf2img_fmts[i], sizeof(key)))
         return i;
   }

   unreachable("Invalid image format\n");
}

static void
panvk_meta_copy_buf2img(struct panvk_cmd_buffer *cmdbuf,
                        const struct panvk_buffer *buf,
                        const struct panvk_image *img,
                        const VkBufferImageCopy2 *region)
{
   struct pan_fb_info *fbinfo = &cmdbuf->state.fb.info;
   unsigned minx = MAX2(region->imageOffset.x, 0);
   unsigned miny = MAX2(region->imageOffset.y, 0);
   unsigned maxx =
      MAX2(region->imageOffset.x + region->imageExtent.width - 1, 0);
   unsigned maxy =
      MAX2(region->imageOffset.y + region->imageExtent.height - 1, 0);

   mali_ptr vpd = panvk_per_arch(meta_emit_viewport)(&cmdbuf->desc_pool.base,
                                                     minx, miny, maxx, maxy);

   float dst_rect[] = {
      minx, miny,     0.0, 1.0, maxx + 1, miny,     0.0, 1.0,
      minx, maxy + 1, 0.0, 1.0, maxx + 1, maxy + 1, 0.0, 1.0,
   };
   mali_ptr dst_coords = pan_pool_upload_aligned(
      &cmdbuf->desc_pool.base, dst_rect, sizeof(dst_rect), 64);

   struct panvk_meta_copy_format_info key = {
      .imgfmt = panvk_meta_copy_buf2img_format(img->pimage.layout.format),
      .mask = panvk_meta_copy_img_mask(img->pimage.layout.format,
                                       region->imageSubresource.aspectMask),
   };

   unsigned fmtidx = panvk_meta_copy_buf2img_format_idx(key);

   mali_ptr rsd =
      cmdbuf->device->physical_device->meta.copy.buf2img[fmtidx].rsd;

   const struct vk_image_buffer_layout buflayout =
      vk_image_buffer_copy_layout(&img->vk, region);
   struct panvk_meta_copy_buf2img_info info = {
      .buf.ptr = panvk_buffer_gpu_ptr(buf, region->bufferOffset),
      .buf.stride.line = buflayout.row_stride_B,
      .buf.stride.surf = buflayout.image_stride_B,
   };

   mali_ptr pushconsts =
      pan_pool_upload_aligned(&cmdbuf->desc_pool.base, &info, sizeof(info), 16);

   struct pan_image_view view = {
      .format = key.imgfmt,
      .dim = MALI_TEXTURE_DIMENSION_2D,
      .planes[0] = &img->pimage,
      .nr_samples = img->pimage.layout.nr_samples,
      .first_level = region->imageSubresource.mipLevel,
      .last_level = region->imageSubresource.mipLevel,
      .swizzle = {PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y, PIPE_SWIZZLE_Z,
                  PIPE_SWIZZLE_W},
   };

   /* TODO: don't force preloads of dst resources if unneeded */
   cmdbuf->state.fb.crc_valid[0] = false;
   *fbinfo = (struct pan_fb_info){
      .width =
         u_minify(img->pimage.layout.width, region->imageSubresource.mipLevel),
      .height =
         u_minify(img->pimage.layout.height, region->imageSubresource.mipLevel),
      .extent.minx = minx,
      .extent.maxx = maxx,
      .extent.miny = miny,
      .extent.maxy = maxy,
      .nr_samples = 1,
      .rt_count = 1,
      .rts[0].view = &view,
      .rts[0].preload = true,
      .rts[0].crc_valid = &cmdbuf->state.fb.crc_valid[0],
   };

   panvk_per_arch(cmd_close_batch)(cmdbuf);

   assert(region->imageSubresource.layerCount == 1 ||
          region->imageExtent.depth == 1);
   assert(region->imageOffset.z >= 0);
   unsigned first_layer =
      MAX2(region->imageSubresource.baseArrayLayer, region->imageOffset.z);
   unsigned nlayers =
      MAX2(region->imageSubresource.layerCount, region->imageExtent.depth);
   for (unsigned l = 0; l < nlayers; l++) {
      float src_rect[] = {
         0,
         0,
         l,
         1.0,
         region->imageExtent.width,
         0,
         l,
         1.0,
         0,
         region->imageExtent.height,
         l,
         1.0,
         region->imageExtent.width,
         region->imageExtent.height,
         l,
         1.0,
      };

      mali_ptr src_coords = pan_pool_upload_aligned(
         &cmdbuf->desc_pool.base, src_rect, sizeof(src_rect), 64);

      struct panvk_batch *batch = panvk_cmd_open_batch(cmdbuf);

      view.first_layer = view.last_layer = l + first_layer;
      batch->blit.src = buf->bo;
      batch->blit.dst = img->pimage.data.bo;
      panvk_per_arch(cmd_alloc_tls_desc)(cmdbuf, true);
      panvk_per_arch(cmd_alloc_fb_desc)(cmdbuf);
      panvk_per_arch(cmd_prepare_tiler_context)(cmdbuf);

      mali_ptr tsd, tiler;

      tsd = batch->tls.gpu;
      tiler = batch->tiler.descs.gpu;

      struct panfrost_ptr job;

      job = panvk_meta_copy_emit_tiler_job(&cmdbuf->desc_pool.base, &batch->jc,
                                           src_coords, dst_coords, 0, 0,
                                           pushconsts, vpd, rsd, tsd, tiler);

      util_dynarray_append(&batch->jobs, void *, job.cpu);
      panvk_per_arch(cmd_close_batch)(cmdbuf);
   }
}

static void
panvk_meta_copy_buf2img_init(struct panvk_physical_device *dev)
{
   STATIC_ASSERT(ARRAY_SIZE(panvk_meta_copy_buf2img_fmts) ==
                 PANVK_META_COPY_BUF2IMG_NUM_FORMATS);

   for (unsigned i = 0; i < ARRAY_SIZE(panvk_meta_copy_buf2img_fmts); i++) {
      struct pan_shader_info shader_info;
      mali_ptr shader = panvk_meta_copy_buf2img_shader(
         &dev->pdev, &dev->meta.bin_pool.base, panvk_meta_copy_buf2img_fmts[i],
         &shader_info);
      dev->meta.copy.buf2img[i].rsd = panvk_meta_copy_to_img_emit_rsd(
         &dev->pdev, &dev->meta.desc_pool.base, shader, &shader_info,
         panvk_meta_copy_buf2img_fmts[i].imgfmt,
         panvk_meta_copy_buf2img_fmts[i].mask, false);
   }
}

void
panvk_per_arch(CmdCopyBufferToImage2)(
   VkCommandBuffer commandBuffer,
   const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   VK_FROM_HANDLE(panvk_buffer, buf, pCopyBufferToImageInfo->srcBuffer);
   VK_FROM_HANDLE(panvk_image, img, pCopyBufferToImageInfo->dstImage);

   for (unsigned i = 0; i < pCopyBufferToImageInfo->regionCount; i++) {
      panvk_meta_copy_buf2img(cmdbuf, buf, img,
                              &pCopyBufferToImageInfo->pRegions[i]);
   }
}

static const struct panvk_meta_copy_format_info panvk_meta_copy_img2buf_fmts[] =
   {
      {PIPE_FORMAT_R8_UINT, 0x1},
      {PIPE_FORMAT_R8G8_UINT, 0x3},
      {PIPE_FORMAT_R5G6B5_UNORM, 0x7},
      {PIPE_FORMAT_R8G8B8A8_UINT, 0xf},
      {PIPE_FORMAT_R16G16B16_UINT, 0x7},
      {PIPE_FORMAT_R32G32_UINT, 0x3},
      {PIPE_FORMAT_R32G32B32_UINT, 0x7},
      {PIPE_FORMAT_R32G32B32A32_UINT, 0xf},
      /* S8 -> Z24S8 */
      {PIPE_FORMAT_R8G8B8A8_UINT, 0x8},
      /* S8 -> Z32_S8X24 */
      {PIPE_FORMAT_R32G32_UINT, 0x2},
      /* Z24X8 -> Z24S8 */
      {PIPE_FORMAT_R8G8B8A8_UINT, 0x7},
      /* Z32 -> Z32_S8X24 */
      {PIPE_FORMAT_R32G32_UINT, 0x1},
};

static enum pipe_format
panvk_meta_copy_img2buf_format(enum pipe_format imgfmt)
{
   /* Pick blendable formats when we can, and the FLOAT variant matching the
    * texelsize otherwise.
    */
   switch (util_format_get_blocksize(imgfmt)) {
   case 1:
      return PIPE_FORMAT_R8_UINT;
   /* AFBC stores things differently for RGB565,
    * we can't simply map to R8G8 in that case */
   case 2:
      return (imgfmt == PIPE_FORMAT_R5G6B5_UNORM ||
              imgfmt == PIPE_FORMAT_B5G6R5_UNORM)
                ? PIPE_FORMAT_R5G6B5_UNORM
                : PIPE_FORMAT_R8G8_UINT;
   case 4:
      return PIPE_FORMAT_R8G8B8A8_UINT;
   case 6:
      return PIPE_FORMAT_R16G16B16_UINT;
   case 8:
      return PIPE_FORMAT_R32G32_UINT;
   case 12:
      return PIPE_FORMAT_R32G32B32_UINT;
   case 16:
      return PIPE_FORMAT_R32G32B32A32_UINT;
   default:
      unreachable("Invalid format\n");
   }
}

struct panvk_meta_copy_img2buf_info {
   struct {
      mali_ptr ptr;
      struct {
         unsigned line;
         unsigned surf;
      } stride;
   } buf;
   struct {
      struct {
         unsigned x, y, z;
      } offset;
      struct {
         unsigned minx, miny, maxx, maxy;
      } extent;
   } img;
} PACKED;

#define panvk_meta_copy_img2buf_get_info_field(b, field)                       \
   nir_load_push_constant(                                                     \
      (b), 1, sizeof(((struct panvk_meta_copy_img2buf_info *)0)->field) * 8,   \
      nir_imm_int(b, 0),                                                       \
      .base = offsetof(struct panvk_meta_copy_img2buf_info, field),            \
      .range = ~0)

static mali_ptr
panvk_meta_copy_img2buf_shader(struct panfrost_device *pdev,
                               struct pan_pool *bin_pool,
                               struct panvk_meta_copy_format_info key,
                               unsigned texdim, unsigned texisarray,
                               struct pan_shader_info *shader_info)
{
   unsigned imgtexelsz = util_format_get_blocksize(key.imgfmt);
   unsigned buftexelsz = panvk_meta_copy_buf_texelsize(key.imgfmt, key.mask);

   /* FIXME: Won't work on compute queues, but we can't do that with
    * a compute shader if the destination is an AFBC surface.
    */
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_COMPUTE, GENX(pan_shader_get_compiler_options)(),
      "panvk_meta_copy_img2buf(dim=%dD%s,imgfmt=%s,mask=%x)", texdim,
      texisarray ? "[]" : "", util_format_name(key.imgfmt), key.mask);

   nir_def *coord = nir_load_global_invocation_id(&b, 32);
   nir_def *bufptr = panvk_meta_copy_img2buf_get_info_field(&b, buf.ptr);
   nir_def *buflinestride =
      panvk_meta_copy_img2buf_get_info_field(&b, buf.stride.line);
   nir_def *bufsurfstride =
      panvk_meta_copy_img2buf_get_info_field(&b, buf.stride.surf);

   nir_def *imgminx =
      panvk_meta_copy_img2buf_get_info_field(&b, img.extent.minx);
   nir_def *imgminy =
      panvk_meta_copy_img2buf_get_info_field(&b, img.extent.miny);
   nir_def *imgmaxx =
      panvk_meta_copy_img2buf_get_info_field(&b, img.extent.maxx);
   nir_def *imgmaxy =
      panvk_meta_copy_img2buf_get_info_field(&b, img.extent.maxy);

   nir_def *imgcoords, *inbounds;

   switch (texdim + texisarray) {
   case 1:
      imgcoords =
         nir_iadd(&b, nir_channel(&b, coord, 0),
                  panvk_meta_copy_img2buf_get_info_field(&b, img.offset.x));
      inbounds =
         nir_iand(&b, nir_uge(&b, imgmaxx, nir_channel(&b, imgcoords, 0)),
                  nir_uge(&b, nir_channel(&b, imgcoords, 0), imgminx));
      break;
   case 2:
      imgcoords = nir_vec2(
         &b,
         nir_iadd(&b, nir_channel(&b, coord, 0),
                  panvk_meta_copy_img2buf_get_info_field(&b, img.offset.x)),
         nir_iadd(&b, nir_channel(&b, coord, 1),
                  panvk_meta_copy_img2buf_get_info_field(&b, img.offset.y)));
      inbounds = nir_iand(
         &b,
         nir_iand(&b, nir_uge(&b, imgmaxx, nir_channel(&b, imgcoords, 0)),
                  nir_uge(&b, imgmaxy, nir_channel(&b, imgcoords, 1))),
         nir_iand(&b, nir_uge(&b, nir_channel(&b, imgcoords, 0), imgminx),
                  nir_uge(&b, nir_channel(&b, imgcoords, 1), imgminy)));
      break;
   case 3:
      imgcoords = nir_vec3(
         &b,
         nir_iadd(&b, nir_channel(&b, coord, 0),
                  panvk_meta_copy_img2buf_get_info_field(&b, img.offset.x)),
         nir_iadd(&b, nir_channel(&b, coord, 1),
                  panvk_meta_copy_img2buf_get_info_field(&b, img.offset.y)),
         nir_iadd(&b, nir_channel(&b, coord, 2),
                  panvk_meta_copy_img2buf_get_info_field(&b, img.offset.y)));
      inbounds = nir_iand(
         &b,
         nir_iand(&b, nir_uge(&b, imgmaxx, nir_channel(&b, imgcoords, 0)),
                  nir_uge(&b, imgmaxy, nir_channel(&b, imgcoords, 1))),
         nir_iand(&b, nir_uge(&b, nir_channel(&b, imgcoords, 0), imgminx),
                  nir_uge(&b, nir_channel(&b, imgcoords, 1), imgminy)));
      break;
   default:
      unreachable("Invalid texture dimension\n");
   }

   nir_push_if(&b, inbounds);

   /* FIXME: doesn't work for tiled+compressed formats since blocks are 4x4
    * blocks instead of 16x16 texels in that case, and there's nothing we can
    * do to force the tile size to 4x4 in the render path.
    * This being said, compressed textures are not compatible with AFBC, so we
    * could use a compute shader arranging the blocks properly.
    */
   nir_def *offset =
      nir_imul(&b, nir_channel(&b, coord, 0), nir_imm_int(&b, buftexelsz));
   offset = nir_iadd(&b, offset,
                     nir_imul(&b, nir_channel(&b, coord, 1), buflinestride));
   offset = nir_iadd(&b, offset,
                     nir_imul(&b, nir_channel(&b, coord, 2), bufsurfstride));
   bufptr = nir_iadd(&b, bufptr, nir_u2u64(&b, offset));

   unsigned imgcompsz =
      imgtexelsz <= 4 ? 1 : MIN2(1 << (ffs(imgtexelsz) - 1), 4);
   unsigned nimgcomps = imgtexelsz / imgcompsz;
   assert(nimgcomps <= 4);

   nir_tex_instr *tex = nir_tex_instr_create(b.shader, 1);
   tex->op = nir_texop_txf;
   tex->texture_index = 0;
   tex->is_array = texisarray;
   tex->dest_type =
      util_format_is_unorm(key.imgfmt) ? nir_type_float32 : nir_type_uint32;

   switch (texdim) {
   case 1:
      tex->sampler_dim = GLSL_SAMPLER_DIM_1D;
      break;
   case 2:
      tex->sampler_dim = GLSL_SAMPLER_DIM_2D;
      break;
   case 3:
      tex->sampler_dim = GLSL_SAMPLER_DIM_3D;
      break;
   default:
      unreachable("Invalid texture dimension");
   }

   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_coord, imgcoords);
   tex->coord_components = texdim + texisarray;
   nir_def_init(&tex->instr, &tex->def, 4,
                nir_alu_type_get_type_size(tex->dest_type));
   nir_builder_instr_insert(&b, &tex->instr);

   nir_def *texel = &tex->def;

   unsigned fullmask = (1 << util_format_get_nr_components(key.imgfmt)) - 1;
   unsigned nbufcomps = util_bitcount(fullmask);
   if (key.mask != fullmask) {
      nir_def *bufcomps[4];
      nbufcomps = 0;
      for (unsigned i = 0; i < nimgcomps; i++) {
         if (key.mask & BITFIELD_BIT(i))
            bufcomps[nbufcomps++] = nir_channel(&b, texel, i);
      }

      texel = nir_vec(&b, bufcomps, nbufcomps);
   }

   unsigned bufcompsz = buftexelsz / nbufcomps;

   if (key.imgfmt == PIPE_FORMAT_R5G6B5_UNORM) {
      texel = nir_fmul(&b, texel,
                       nir_vec3(&b, nir_imm_float(&b, 31),
                                nir_imm_float(&b, 63), nir_imm_float(&b, 31)));
      texel = nir_f2u16(&b, texel);
      texel = nir_ior(
         &b, nir_channel(&b, texel, 0),
         nir_ior(&b,
                 nir_ishl(&b, nir_channel(&b, texel, 1), nir_imm_int(&b, 5)),
                 nir_ishl(&b, nir_channel(&b, texel, 2), nir_imm_int(&b, 11))));
      imgcompsz = 2;
      bufcompsz = 2;
      nbufcomps = 1;
      nimgcomps = 1;
   } else if (imgcompsz == 1) {
      nir_def *packed = nir_channel(&b, texel, 0);
      for (unsigned i = 1; i < nbufcomps; i++) {
         packed = nir_ior(
            &b, packed,
            nir_ishl(&b, nir_iand_imm(&b, nir_channel(&b, texel, i), 0xff),
                     nir_imm_int(&b, i * 8)));
      }
      texel = packed;

      bufcompsz = nbufcomps == 3 ? 4 : nbufcomps;
      nbufcomps = 1;
   }

   assert(bufcompsz == 1 || bufcompsz == 2 || bufcompsz == 4);
   assert(nbufcomps <= 4 && nimgcomps <= 4);
   texel = nir_u2uN(&b, texel, bufcompsz * 8);

   nir_store_global(&b, bufptr, bufcompsz, texel, (1 << nbufcomps) - 1);
   nir_pop_if(&b, NULL);

   struct panfrost_compile_inputs inputs = {
      .gpu_id = panfrost_device_gpu_id(pdev),
      .is_blit = true,
      .no_ubo_to_push = true,
   };

   struct util_dynarray binary;

   util_dynarray_init(&binary, NULL);
   pan_shader_preprocess(b.shader, inputs.gpu_id);
   GENX(pan_shader_compile)(b.shader, &inputs, &binary, shader_info);

   shader_info->push.count =
      DIV_ROUND_UP(sizeof(struct panvk_meta_copy_img2buf_info), 4);

   mali_ptr shader =
      pan_pool_upload_aligned(bin_pool, binary.data, binary.size, 128);

   util_dynarray_fini(&binary);
   ralloc_free(b.shader);

   return shader;
}

static unsigned
panvk_meta_copy_img2buf_format_idx(struct panvk_meta_copy_format_info key)
{
   for (unsigned i = 0; i < ARRAY_SIZE(panvk_meta_copy_img2buf_fmts); i++) {
      if (!memcmp(&key, &panvk_meta_copy_img2buf_fmts[i], sizeof(key)))
         return i;
   }

   unreachable("Invalid texel size\n");
}

static void
panvk_meta_copy_img2buf(struct panvk_cmd_buffer *cmdbuf,
                        const struct panvk_buffer *buf,
                        const struct panvk_image *img,
                        const VkBufferImageCopy2 *region)
{
   struct panfrost_device *pdev = &cmdbuf->device->physical_device->pdev;
   struct panvk_meta_copy_format_info key = {
      .imgfmt = panvk_meta_copy_img2buf_format(img->pimage.layout.format),
      .mask = panvk_meta_copy_img_mask(img->pimage.layout.format,
                                       region->imageSubresource.aspectMask),
   };
   unsigned buftexelsz = panvk_meta_copy_buf_texelsize(key.imgfmt, key.mask);
   unsigned texdimidx = panvk_meta_copy_tex_type(
      img->pimage.layout.dim, img->pimage.layout.array_size > 1);
   unsigned fmtidx = panvk_meta_copy_img2buf_format_idx(key);

   mali_ptr rsd =
      cmdbuf->device->physical_device->meta.copy.img2buf[texdimidx][fmtidx].rsd;

   struct panvk_meta_copy_img2buf_info info = {
      .buf.ptr = panvk_buffer_gpu_ptr(buf, region->bufferOffset),
      .buf.stride.line =
         (region->bufferRowLength ?: region->imageExtent.width) * buftexelsz,
      .img.offset.x = MAX2(region->imageOffset.x & ~15, 0),
      .img.extent.minx = MAX2(region->imageOffset.x, 0),
      .img.extent.maxx =
         MAX2(region->imageOffset.x + region->imageExtent.width - 1, 0),
   };

   if (img->pimage.layout.dim == MALI_TEXTURE_DIMENSION_1D) {
      info.img.extent.maxy = region->imageSubresource.layerCount - 1;
   } else {
      info.img.offset.y = MAX2(region->imageOffset.y & ~15, 0);
      info.img.offset.z = MAX2(region->imageOffset.z, 0);
      info.img.extent.miny = MAX2(region->imageOffset.y, 0);
      info.img.extent.maxy =
         MAX2(region->imageOffset.y + region->imageExtent.height - 1, 0);
   }

   info.buf.stride.surf =
      (region->bufferImageHeight ?: region->imageExtent.height) *
      info.buf.stride.line;

   mali_ptr pushconsts =
      pan_pool_upload_aligned(&cmdbuf->desc_pool.base, &info, sizeof(info), 16);

   struct pan_image_view view = {
      .format = key.imgfmt,
      .dim = img->pimage.layout.dim == MALI_TEXTURE_DIMENSION_CUBE
                ? MALI_TEXTURE_DIMENSION_2D
                : img->pimage.layout.dim,
      .planes[0] = &img->pimage,
      .nr_samples = img->pimage.layout.nr_samples,
      .first_level = region->imageSubresource.mipLevel,
      .last_level = region->imageSubresource.mipLevel,
      .first_layer = region->imageSubresource.baseArrayLayer,
      .last_layer = region->imageSubresource.baseArrayLayer +
                    region->imageSubresource.layerCount - 1,
      .swizzle = {PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y, PIPE_SWIZZLE_Z,
                  PIPE_SWIZZLE_W},
   };

   mali_ptr texture =
      panvk_meta_copy_img_emit_texture(pdev, &cmdbuf->desc_pool.base, &view);
   mali_ptr sampler =
      panvk_meta_copy_img_emit_sampler(pdev, &cmdbuf->desc_pool.base);

   panvk_per_arch(cmd_close_batch)(cmdbuf);

   struct panvk_batch *batch = panvk_cmd_open_batch(cmdbuf);

   struct pan_tls_info tlsinfo = {0};

   batch->blit.src = img->pimage.data.bo;
   batch->blit.dst = buf->bo;
   batch->tls = pan_pool_alloc_desc(&cmdbuf->desc_pool.base, LOCAL_STORAGE);
   GENX(pan_emit_tls)(&tlsinfo, batch->tls.cpu);

   mali_ptr tsd = batch->tls.gpu;

   struct pan_compute_dim wg_sz = {
      16,
      img->pimage.layout.dim == MALI_TEXTURE_DIMENSION_1D ? 1 : 16,
      1,
   };

   struct pan_compute_dim num_wg = {
      (ALIGN_POT(info.img.extent.maxx + 1, 16) - info.img.offset.x) / 16,
      img->pimage.layout.dim == MALI_TEXTURE_DIMENSION_1D
         ? region->imageSubresource.layerCount
         : (ALIGN_POT(info.img.extent.maxy + 1, 16) - info.img.offset.y) / 16,
      img->pimage.layout.dim != MALI_TEXTURE_DIMENSION_1D
         ? MAX2(region->imageSubresource.layerCount, region->imageExtent.depth)
         : 1,
   };

   struct panfrost_ptr job = panvk_meta_copy_emit_compute_job(
      &cmdbuf->desc_pool.base, &batch->jc, &num_wg, &wg_sz, texture, sampler,
      pushconsts, rsd, tsd);

   util_dynarray_append(&batch->jobs, void *, job.cpu);

   panvk_per_arch(cmd_close_batch)(cmdbuf);
}

static void
panvk_meta_copy_img2buf_init(struct panvk_physical_device *dev)
{
   STATIC_ASSERT(ARRAY_SIZE(panvk_meta_copy_img2buf_fmts) ==
                 PANVK_META_COPY_IMG2BUF_NUM_FORMATS);

   for (unsigned i = 0; i < ARRAY_SIZE(panvk_meta_copy_img2buf_fmts); i++) {
      for (unsigned texdim = 1; texdim <= 3; texdim++) {
         unsigned texdimidx = panvk_meta_copy_tex_type(texdim, false);
         assert(texdimidx < ARRAY_SIZE(dev->meta.copy.img2buf));

         struct pan_shader_info shader_info;
         mali_ptr shader = panvk_meta_copy_img2buf_shader(
            &dev->pdev, &dev->meta.bin_pool.base,
            panvk_meta_copy_img2buf_fmts[i], texdim, false, &shader_info);
         dev->meta.copy.img2buf[texdimidx][i].rsd =
            panvk_meta_copy_to_buf_emit_rsd(&dev->pdev,
                                            &dev->meta.desc_pool.base, shader,
                                            &shader_info, true);

         if (texdim == 3)
            continue;

         memset(&shader_info, 0, sizeof(shader_info));
         texdimidx = panvk_meta_copy_tex_type(texdim, true);
         assert(texdimidx < ARRAY_SIZE(dev->meta.copy.img2buf));
         shader = panvk_meta_copy_img2buf_shader(
            &dev->pdev, &dev->meta.bin_pool.base,
            panvk_meta_copy_img2buf_fmts[i], texdim, true, &shader_info);
         dev->meta.copy.img2buf[texdimidx][i].rsd =
            panvk_meta_copy_to_buf_emit_rsd(&dev->pdev,
                                            &dev->meta.desc_pool.base, shader,
                                            &shader_info, true);
      }
   }
}

void
panvk_per_arch(CmdCopyImageToBuffer2)(
   VkCommandBuffer commandBuffer,
   const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   VK_FROM_HANDLE(panvk_buffer, buf, pCopyImageToBufferInfo->dstBuffer);
   VK_FROM_HANDLE(panvk_image, img, pCopyImageToBufferInfo->srcImage);

   for (unsigned i = 0; i < pCopyImageToBufferInfo->regionCount; i++) {
      panvk_meta_copy_img2buf(cmdbuf, buf, img,
                              &pCopyImageToBufferInfo->pRegions[i]);
   }
}

struct panvk_meta_copy_buf2buf_info {
   mali_ptr src;
   mali_ptr dst;
} PACKED;

#define panvk_meta_copy_buf2buf_get_info_field(b, field)                       \
   nir_load_push_constant(                                                     \
      (b), 1, sizeof(((struct panvk_meta_copy_buf2buf_info *)0)->field) * 8,   \
      nir_imm_int(b, 0),                                                       \
      .base = offsetof(struct panvk_meta_copy_buf2buf_info, field),            \
      .range = ~0)

static mali_ptr
panvk_meta_copy_buf2buf_shader(struct panfrost_device *pdev,
                               struct pan_pool *bin_pool, unsigned blksz,
                               struct pan_shader_info *shader_info)
{
   /* FIXME: Won't work on compute queues, but we can't do that with
    * a compute shader if the destination is an AFBC surface.
    */
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_COMPUTE, GENX(pan_shader_get_compiler_options)(),
      "panvk_meta_copy_buf2buf(blksz=%d)", blksz);

   nir_def *coord = nir_load_global_invocation_id(&b, 32);

   nir_def *offset = nir_u2u64(
      &b, nir_imul(&b, nir_channel(&b, coord, 0), nir_imm_int(&b, blksz)));
   nir_def *srcptr =
      nir_iadd(&b, panvk_meta_copy_buf2buf_get_info_field(&b, src), offset);
   nir_def *dstptr =
      nir_iadd(&b, panvk_meta_copy_buf2buf_get_info_field(&b, dst), offset);

   unsigned compsz = blksz < 4 ? blksz : 4;
   unsigned ncomps = blksz / compsz;
   nir_store_global(&b, dstptr, blksz,
                    nir_load_global(&b, srcptr, blksz, ncomps, compsz * 8),
                    (1 << ncomps) - 1);

   struct panfrost_compile_inputs inputs = {
      .gpu_id = panfrost_device_gpu_id(pdev),
      .is_blit = true,
      .no_ubo_to_push = true,
   };

   struct util_dynarray binary;

   util_dynarray_init(&binary, NULL);
   pan_shader_preprocess(b.shader, inputs.gpu_id);
   GENX(pan_shader_compile)(b.shader, &inputs, &binary, shader_info);

   shader_info->push.count =
      DIV_ROUND_UP(sizeof(struct panvk_meta_copy_buf2buf_info), 4);

   mali_ptr shader =
      pan_pool_upload_aligned(bin_pool, binary.data, binary.size, 128);

   util_dynarray_fini(&binary);
   ralloc_free(b.shader);

   return shader;
}

static void
panvk_meta_copy_buf2buf_init(struct panvk_physical_device *dev)
{
   for (unsigned i = 0; i < ARRAY_SIZE(dev->meta.copy.buf2buf); i++) {
      struct pan_shader_info shader_info;
      mali_ptr shader = panvk_meta_copy_buf2buf_shader(
         &dev->pdev, &dev->meta.bin_pool.base, 1 << i, &shader_info);
      dev->meta.copy.buf2buf[i].rsd = panvk_meta_copy_to_buf_emit_rsd(
         &dev->pdev, &dev->meta.desc_pool.base, shader, &shader_info, false);
   }
}

static void
panvk_meta_copy_buf2buf(struct panvk_cmd_buffer *cmdbuf,
                        const struct panvk_buffer *src,
                        const struct panvk_buffer *dst,
                        const VkBufferCopy2 *region)
{
   struct panvk_meta_copy_buf2buf_info info = {
      .src = panvk_buffer_gpu_ptr(src, region->srcOffset),
      .dst = panvk_buffer_gpu_ptr(dst, region->dstOffset),
   };

   unsigned alignment = ffs((info.src | info.dst | region->size) & 15);
   unsigned log2blksz = alignment ? alignment - 1 : 4;

   assert(log2blksz <
          ARRAY_SIZE(cmdbuf->device->physical_device->meta.copy.buf2buf));
   mali_ptr rsd =
      cmdbuf->device->physical_device->meta.copy.buf2buf[log2blksz].rsd;

   mali_ptr pushconsts =
      pan_pool_upload_aligned(&cmdbuf->desc_pool.base, &info, sizeof(info), 16);

   panvk_per_arch(cmd_close_batch)(cmdbuf);

   struct panvk_batch *batch = panvk_cmd_open_batch(cmdbuf);

   panvk_per_arch(cmd_alloc_tls_desc)(cmdbuf, false);

   mali_ptr tsd = batch->tls.gpu;

   unsigned nblocks = region->size >> log2blksz;
   struct pan_compute_dim num_wg = {nblocks, 1, 1};
   struct pan_compute_dim wg_sz = {1, 1, 1};
   struct panfrost_ptr job = panvk_meta_copy_emit_compute_job(
      &cmdbuf->desc_pool.base, &batch->jc, &num_wg, &wg_sz, 0, 0, pushconsts,
      rsd, tsd);

   util_dynarray_append(&batch->jobs, void *, job.cpu);

   batch->blit.src = src->bo;
   batch->blit.dst = dst->bo;
   panvk_per_arch(cmd_close_batch)(cmdbuf);
}

void
panvk_per_arch(CmdCopyBuffer2)(VkCommandBuffer commandBuffer,
                               const VkCopyBufferInfo2 *pCopyBufferInfo)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   VK_FROM_HANDLE(panvk_buffer, src, pCopyBufferInfo->srcBuffer);
   VK_FROM_HANDLE(panvk_buffer, dst, pCopyBufferInfo->dstBuffer);

   for (unsigned i = 0; i < pCopyBufferInfo->regionCount; i++) {
      panvk_meta_copy_buf2buf(cmdbuf, src, dst, &pCopyBufferInfo->pRegions[i]);
   }
}

struct panvk_meta_fill_buf_info {
   mali_ptr start;
   uint32_t val;
} PACKED;

#define panvk_meta_fill_buf_get_info_field(b, field)                           \
   nir_load_push_constant(                                                     \
      (b), 1, sizeof(((struct panvk_meta_fill_buf_info *)0)->field) * 8,       \
      nir_imm_int(b, 0),                                                       \
      .base = offsetof(struct panvk_meta_fill_buf_info, field), .range = ~0)

static mali_ptr
panvk_meta_fill_buf_shader(struct panfrost_device *pdev,
                           struct pan_pool *bin_pool,
                           struct pan_shader_info *shader_info)
{
   /* FIXME: Won't work on compute queues, but we can't do that with
    * a compute shader if the destination is an AFBC surface.
    */
   nir_builder b = nir_builder_init_simple_shader(
      MESA_SHADER_COMPUTE, GENX(pan_shader_get_compiler_options)(),
      "panvk_meta_fill_buf()");

   nir_def *coord = nir_load_global_invocation_id(&b, 32);

   nir_def *offset = nir_u2u64(&b, nir_imul(&b, nir_channel(&b, coord, 0),
                                            nir_imm_int(&b, sizeof(uint32_t))));
   nir_def *ptr =
      nir_iadd(&b, panvk_meta_fill_buf_get_info_field(&b, start), offset);
   nir_def *val = panvk_meta_fill_buf_get_info_field(&b, val);

   nir_store_global(&b, ptr, sizeof(uint32_t), val, 1);

   struct panfrost_compile_inputs inputs = {
      .gpu_id = panfrost_device_gpu_id(pdev),
      .is_blit = true,
      .no_ubo_to_push = true,
   };

   struct util_dynarray binary;

   util_dynarray_init(&binary, NULL);
   pan_shader_preprocess(b.shader, inputs.gpu_id);
   GENX(pan_shader_compile)(b.shader, &inputs, &binary, shader_info);

   shader_info->push.count =
      DIV_ROUND_UP(sizeof(struct panvk_meta_fill_buf_info), 4);

   mali_ptr shader =
      pan_pool_upload_aligned(bin_pool, binary.data, binary.size, 128);

   util_dynarray_fini(&binary);
   ralloc_free(b.shader);

   return shader;
}

static mali_ptr
panvk_meta_fill_buf_emit_rsd(struct panfrost_device *pdev,
                             struct pan_pool *bin_pool,
                             struct pan_pool *desc_pool)
{
   struct pan_shader_info shader_info;

   mali_ptr shader = panvk_meta_fill_buf_shader(pdev, bin_pool, &shader_info);

   struct panfrost_ptr rsd_ptr =
      pan_pool_alloc_desc_aggregate(desc_pool, PAN_DESC(RENDERER_STATE));

   pan_pack(rsd_ptr.cpu, RENDERER_STATE, cfg) {
      pan_shader_prepare_rsd(&shader_info, shader, &cfg);
   }

   return rsd_ptr.gpu;
}

static void
panvk_meta_fill_buf_init(struct panvk_physical_device *dev)
{
   dev->meta.copy.fillbuf.rsd = panvk_meta_fill_buf_emit_rsd(
      &dev->pdev, &dev->meta.bin_pool.base, &dev->meta.desc_pool.base);
}

static void
panvk_meta_fill_buf(struct panvk_cmd_buffer *cmdbuf,
                    const struct panvk_buffer *dst, VkDeviceSize size,
                    VkDeviceSize offset, uint32_t val)
{
   struct panvk_meta_fill_buf_info info = {
      .start = panvk_buffer_gpu_ptr(dst, offset),
      .val = val,
   };
   size = panvk_buffer_range(dst, offset, size);

   /* From the Vulkan spec:
    *
    *    "size is the number of bytes to fill, and must be either a multiple
    *    of 4, or VK_WHOLE_SIZE to fill the range from offset to the end of
    *    the buffer. If VK_WHOLE_SIZE is used and the remaining size of the
    *    buffer is not a multiple of 4, then the nearest smaller multiple is
    *    used."
    */
   size &= ~3ull;

   assert(!(offset & 3) && !(size & 3));

   unsigned nwords = size / sizeof(uint32_t);
   mali_ptr rsd = cmdbuf->device->physical_device->meta.copy.fillbuf.rsd;

   mali_ptr pushconsts =
      pan_pool_upload_aligned(&cmdbuf->desc_pool.base, &info, sizeof(info), 16);

   panvk_per_arch(cmd_close_batch)(cmdbuf);

   struct panvk_batch *batch = panvk_cmd_open_batch(cmdbuf);

   panvk_per_arch(cmd_alloc_tls_desc)(cmdbuf, false);

   mali_ptr tsd = batch->tls.gpu;

   struct pan_compute_dim num_wg = {nwords, 1, 1};
   struct pan_compute_dim wg_sz = {1, 1, 1};
   struct panfrost_ptr job = panvk_meta_copy_emit_compute_job(
      &cmdbuf->desc_pool.base, &batch->jc, &num_wg, &wg_sz, 0, 0, pushconsts,
      rsd, tsd);

   util_dynarray_append(&batch->jobs, void *, job.cpu);

   batch->blit.dst = dst->bo;
   panvk_per_arch(cmd_close_batch)(cmdbuf);
}

void
panvk_per_arch(CmdFillBuffer)(VkCommandBuffer commandBuffer, VkBuffer dstBuffer,
                              VkDeviceSize dstOffset, VkDeviceSize fillSize,
                              uint32_t data)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   VK_FROM_HANDLE(panvk_buffer, dst, dstBuffer);

   panvk_meta_fill_buf(cmdbuf, dst, fillSize, dstOffset, data);
}

static void
panvk_meta_update_buf(struct panvk_cmd_buffer *cmdbuf,
                      const struct panvk_buffer *dst, VkDeviceSize offset,
                      VkDeviceSize size, const void *data)
{
   struct panvk_meta_copy_buf2buf_info info = {
      .src = pan_pool_upload_aligned(&cmdbuf->desc_pool.base, data, size, 4),
      .dst = panvk_buffer_gpu_ptr(dst, offset),
   };

   unsigned log2blksz = ffs(sizeof(uint32_t)) - 1;

   mali_ptr rsd =
      cmdbuf->device->physical_device->meta.copy.buf2buf[log2blksz].rsd;

   mali_ptr pushconsts =
      pan_pool_upload_aligned(&cmdbuf->desc_pool.base, &info, sizeof(info), 16);

   panvk_per_arch(cmd_close_batch)(cmdbuf);

   struct panvk_batch *batch = panvk_cmd_open_batch(cmdbuf);

   panvk_per_arch(cmd_alloc_tls_desc)(cmdbuf, false);

   mali_ptr tsd = batch->tls.gpu;

   unsigned nblocks = size >> log2blksz;
   struct pan_compute_dim num_wg = {nblocks, 1, 1};
   struct pan_compute_dim wg_sz = {1, 1, 1};
   struct panfrost_ptr job = panvk_meta_copy_emit_compute_job(
      &cmdbuf->desc_pool.base, &batch->jc, &num_wg, &wg_sz, 0, 0, pushconsts,
      rsd, tsd);

   util_dynarray_append(&batch->jobs, void *, job.cpu);

   batch->blit.dst = dst->bo;
   panvk_per_arch(cmd_close_batch)(cmdbuf);
}

void
panvk_per_arch(CmdUpdateBuffer)(VkCommandBuffer commandBuffer,
                                VkBuffer dstBuffer, VkDeviceSize dstOffset,
                                VkDeviceSize dataSize, const void *pData)
{
   VK_FROM_HANDLE(panvk_cmd_buffer, cmdbuf, commandBuffer);
   VK_FROM_HANDLE(panvk_buffer, dst, dstBuffer);

   panvk_meta_update_buf(cmdbuf, dst, dstOffset, dataSize, pData);
}

void
panvk_per_arch(meta_copy_init)(struct panvk_physical_device *dev)
{
   panvk_meta_copy_img2img_init(dev, false);
   panvk_meta_copy_img2img_init(dev, true);
   panvk_meta_copy_buf2img_init(dev);
   panvk_meta_copy_img2buf_init(dev);
   panvk_meta_copy_buf2buf_init(dev);
   panvk_meta_fill_buf_init(dev);
}
