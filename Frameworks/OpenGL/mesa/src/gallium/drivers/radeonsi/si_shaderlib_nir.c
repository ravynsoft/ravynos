/*
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "gallium/auxiliary/nir/pipe_nir.h"
#define AC_SURFACE_INCLUDE_NIR
#include "ac_surface.h"
#include "si_pipe.h"
#include "si_query.h"

#include "nir_format_convert.h"

static void *create_shader_state(struct si_context *sctx, nir_shader *nir)
{
   sctx->b.screen->finalize_nir(sctx->b.screen, (void*)nir);
   return pipe_shader_from_nir(&sctx->b, nir);
}

static nir_def *get_global_ids(nir_builder *b, unsigned num_components)
{
   unsigned mask = BITFIELD_MASK(num_components);

   nir_def *local_ids = nir_channels(b, nir_load_local_invocation_id(b), mask);
   nir_def *block_ids = nir_channels(b, nir_load_workgroup_id(b), mask);
   nir_def *block_size = nir_channels(b, nir_load_workgroup_size(b), mask);
   return nir_iadd(b, nir_imul(b, block_ids, block_size), local_ids);
}

/* unpack_2x16(src, x, y): x = src & 0xffff; y = src >> 16; */
static void unpack_2x16(nir_builder *b, nir_def *src, nir_def **x, nir_def **y)
{
   *x = nir_iand_imm(b, src, 0xffff);
   *y = nir_ushr_imm(b, src, 16);
}

/* unpack_2x16_signed(src, x, y): x = (int32_t)((uint16_t)src); y = src >> 16; */
static void unpack_2x16_signed(nir_builder *b, nir_def *src, nir_def **x, nir_def **y)
{
   *x = nir_i2i32(b, nir_u2u16(b, src));
   *y = nir_ishr_imm(b, src, 16);
}

static nir_def *
deref_ssa(nir_builder *b, nir_variable *var)
{
   return &nir_build_deref_var(b, var)->def;
}

/* Create a NIR compute shader implementing copy_image.
 *
 * This shader can handle 1D and 2D, linear and non-linear images.
 * It expects the source and destination (x,y,z) coords as user_data_amd,
 * packed into 3 SGPRs as 2x16bits per component.
 */
void *si_create_copy_image_cs(struct si_context *sctx, unsigned wg_dim,
                              bool src_is_1d_array, bool dst_is_1d_array)
{
   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "copy_image_cs");
   b.shader->info.num_images = 2;

   /* The workgroup size is either 8x8 for normal (non-linear) 2D images,
    * or 64x1 for 1D and linear-2D images.
    */
   b.shader->info.workgroup_size_variable = true;

   b.shader->info.cs.user_data_components_amd = 3;
   nir_def *ids = nir_pad_vector_imm_int(&b, get_global_ids(&b, wg_dim), 0, 3);

   nir_def *coord_src = NULL, *coord_dst = NULL;
   unpack_2x16(&b, nir_trim_vector(&b, nir_load_user_data_amd(&b), 3),
               &coord_src, &coord_dst);

   coord_src = nir_iadd(&b, coord_src, ids);
   coord_dst = nir_iadd(&b, coord_dst, ids);

   /* Coordinates must have 4 channels in NIR. */
   coord_src = nir_pad_vector(&b, coord_src, 4);
   coord_dst = nir_pad_vector(&b, coord_dst, 4);

   static unsigned swizzle_xz[] = {0, 2, 0, 0};

   if (src_is_1d_array)
      coord_src = nir_swizzle(&b, coord_src, swizzle_xz, 4);
   if (dst_is_1d_array)
      coord_dst = nir_swizzle(&b, coord_dst, swizzle_xz, 4);

   const struct glsl_type *src_img_type = glsl_image_type(src_is_1d_array ? GLSL_SAMPLER_DIM_1D
                                                                          : GLSL_SAMPLER_DIM_2D,
                                                          /*is_array*/ true, GLSL_TYPE_FLOAT);
   const struct glsl_type *dst_img_type = glsl_image_type(dst_is_1d_array ? GLSL_SAMPLER_DIM_1D
                                                                          : GLSL_SAMPLER_DIM_2D,
                                                          /*is_array*/ true, GLSL_TYPE_FLOAT);

   nir_variable *img_src = nir_variable_create(b.shader, nir_var_image, src_img_type, "img_src");
   img_src->data.binding = 0;

   nir_variable *img_dst = nir_variable_create(b.shader, nir_var_image, dst_img_type, "img_dst");
   img_dst->data.binding = 1;

   nir_def *undef32 = nir_undef(&b, 1, 32);
   nir_def *zero = nir_imm_int(&b, 0);

   nir_def *data = nir_image_deref_load(&b, /*num_components*/ 4, /*bit_size*/ 32,
      deref_ssa(&b, img_src), coord_src, undef32, zero);

   nir_image_deref_store(&b, deref_ssa(&b, img_dst), coord_dst, undef32, data, zero);

   return create_shader_state(sctx, b.shader);
}

void *si_create_dcc_retile_cs(struct si_context *sctx, struct radeon_surf *surf)
{
   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "dcc_retile");
   b.shader->info.workgroup_size[0] = 8;
   b.shader->info.workgroup_size[1] = 8;
   b.shader->info.workgroup_size[2] = 1;
   b.shader->info.cs.user_data_components_amd = 3;
   b.shader->info.num_ssbos = 1;

   /* Get user data SGPRs. */
   nir_def *user_sgprs = nir_load_user_data_amd(&b);

   /* Relative offset from the displayable DCC to the non-displayable DCC in the same buffer. */
   nir_def *src_dcc_offset = nir_channel(&b, user_sgprs, 0);

   nir_def *src_dcc_pitch, *dst_dcc_pitch, *src_dcc_height, *dst_dcc_height;
   unpack_2x16(&b, nir_channel(&b, user_sgprs, 1), &src_dcc_pitch, &src_dcc_height);
   unpack_2x16(&b, nir_channel(&b, user_sgprs, 2), &dst_dcc_pitch, &dst_dcc_height);

   /* Get the 2D coordinates. */
   nir_def *coord = get_global_ids(&b, 2);
   nir_def *zero = nir_imm_int(&b, 0);

   /* Multiply the coordinates by the DCC block size (they are DCC block coordinates). */
   coord = nir_imul(&b, coord, nir_imm_ivec2(&b, surf->u.gfx9.color.dcc_block_width,
                                             surf->u.gfx9.color.dcc_block_height));

   nir_def *src_offset =
      ac_nir_dcc_addr_from_coord(&b, &sctx->screen->info, surf->bpe, &surf->u.gfx9.color.dcc_equation,
                                 src_dcc_pitch, src_dcc_height, zero, /* DCC slice size */
                                 nir_channel(&b, coord, 0), nir_channel(&b, coord, 1), /* x, y */
                                 zero, zero, zero); /* z, sample, pipe_xor */
   src_offset = nir_iadd(&b, src_offset, src_dcc_offset);
   nir_def *value = nir_load_ssbo(&b, 1, 8, zero, src_offset, .align_mul=1);

   nir_def *dst_offset =
      ac_nir_dcc_addr_from_coord(&b, &sctx->screen->info, surf->bpe, &surf->u.gfx9.color.display_dcc_equation,
                                 dst_dcc_pitch, dst_dcc_height, zero, /* DCC slice size */
                                 nir_channel(&b, coord, 0), nir_channel(&b, coord, 1), /* x, y */
                                 zero, zero, zero); /* z, sample, pipe_xor */
   nir_store_ssbo(&b, value, zero, dst_offset, .write_mask=0x1, .align_mul=1);

   return create_shader_state(sctx, b.shader);
}

void *gfx9_create_clear_dcc_msaa_cs(struct si_context *sctx, struct si_texture *tex)
{
   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "clear_dcc_msaa");
   b.shader->info.workgroup_size[0] = 8;
   b.shader->info.workgroup_size[1] = 8;
   b.shader->info.workgroup_size[2] = 1;
   b.shader->info.cs.user_data_components_amd = 2;
   b.shader->info.num_ssbos = 1;

   /* Get user data SGPRs. */
   nir_def *user_sgprs = nir_load_user_data_amd(&b);
   nir_def *dcc_pitch, *dcc_height, *clear_value, *pipe_xor;
   unpack_2x16(&b, nir_channel(&b, user_sgprs, 0), &dcc_pitch, &dcc_height);
   unpack_2x16(&b, nir_channel(&b, user_sgprs, 1), &clear_value, &pipe_xor);
   clear_value = nir_u2u16(&b, clear_value);

   /* Get the 2D coordinates. */
   nir_def *coord = get_global_ids(&b, 3);
   nir_def *zero = nir_imm_int(&b, 0);

   /* Multiply the coordinates by the DCC block size (they are DCC block coordinates). */
   coord = nir_imul(&b, coord,
                    nir_imm_ivec3(&b, tex->surface.u.gfx9.color.dcc_block_width,
                                      tex->surface.u.gfx9.color.dcc_block_height,
                                      tex->surface.u.gfx9.color.dcc_block_depth));

   nir_def *offset =
      ac_nir_dcc_addr_from_coord(&b, &sctx->screen->info, tex->surface.bpe,
                                 &tex->surface.u.gfx9.color.dcc_equation,
                                 dcc_pitch, dcc_height, zero, /* DCC slice size */
                                 nir_channel(&b, coord, 0), nir_channel(&b, coord, 1), /* x, y */
                                 tex->buffer.b.b.array_size > 1 ? nir_channel(&b, coord, 2) : zero, /* z */
                                 zero, pipe_xor); /* sample, pipe_xor */

   /* The trick here is that DCC elements for an even and the next odd sample are next to each other
    * in memory, so we only need to compute the address for sample 0 and the next DCC byte is always
    * sample 1. That's why the clear value has 2 bytes - we're clearing 2 samples at the same time.
    */
   nir_store_ssbo(&b, clear_value, zero, offset, .write_mask=0x1, .align_mul=2);

   return create_shader_state(sctx, b.shader);
}

/* Create a compute shader implementing clear_buffer or copy_buffer. */
void *si_create_clear_buffer_rmw_cs(struct si_context *sctx)
{
   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b =
      nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "clear_buffer_rmw_cs");
   b.shader->info.workgroup_size[0] = 64;
   b.shader->info.workgroup_size[1] = 1;
   b.shader->info.workgroup_size[2] = 1;
   b.shader->info.cs.user_data_components_amd = 2;
   b.shader->info.num_ssbos = 1;

   /* address = blockID * 64 + threadID; */
   nir_def *address = get_global_ids(&b, 1);

   /* address = address * 16; (byte offset, loading one vec4 per thread) */
   address = nir_ishl_imm(&b, address, 4);
   
   nir_def *zero = nir_imm_int(&b, 0);
   nir_def *data = nir_load_ssbo(&b, 4, 32, zero, address, .align_mul = 4);

   /* Get user data SGPRs. */
   nir_def *user_sgprs = nir_load_user_data_amd(&b);

   /* data &= inverted_writemask; */
   data = nir_iand(&b, data, nir_channel(&b, user_sgprs, 1));
   /* data |= clear_value_masked; */
   data = nir_ior(&b, data, nir_channel(&b, user_sgprs, 0));

   nir_store_ssbo(&b, data, zero, address,
      .access = SI_COMPUTE_DST_CACHE_POLICY != L2_LRU ? ACCESS_NON_TEMPORAL : 0,
      .align_mul = 4);

   return create_shader_state(sctx, b.shader);
}

/* This is used when TCS is NULL in the VS->TCS->TES chain. In this case,
 * VS passes its outputs to TES directly, so the fixed-function shader only
 * has to write TESSOUTER and TESSINNER.
 */
void *si_create_passthrough_tcs(struct si_context *sctx)
{
   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR,
                                           PIPE_SHADER_TESS_CTRL);

   unsigned locations[PIPE_MAX_SHADER_OUTPUTS];

   struct si_shader_info *info = &sctx->shader.vs.cso->info;
   for (unsigned i = 0; i < info->num_outputs; i++) {
      locations[i] = info->output_semantic[i];
   }

   nir_shader *tcs =
         nir_create_passthrough_tcs_impl(options, locations, info->num_outputs,
                                         sctx->patch_vertices);

   return create_shader_state(sctx, tcs);
}

static nir_def *convert_linear_to_srgb(nir_builder *b, nir_def *input)
{
   /* There are small precision differences compared to CB, so the gfx blit will return slightly
    * different results.
    */

   nir_def *comp[4];
   for (unsigned i = 0; i < 3; i++)
      comp[i] = nir_format_linear_to_srgb(b, nir_channel(b, input, i));
   comp[3] = nir_channel(b, input, 3);

   return nir_vec(b, comp, 4);
}

static nir_def *average_samples(nir_builder *b, nir_def **samples, unsigned num_samples)
{
   /* This works like add-reduce by computing the sum of each pair independently, and then
    * computing the sum of each pair of sums, and so on, to get better instruction-level
    * parallelism.
    */
   if (num_samples == 16) {
      for (unsigned i = 0; i < 8; i++)
         samples[i] = nir_fadd(b, samples[i * 2], samples[i * 2 + 1]);
   }
   if (num_samples >= 8) {
      for (unsigned i = 0; i < 4; i++)
         samples[i] = nir_fadd(b, samples[i * 2], samples[i * 2 + 1]);
   }
   if (num_samples >= 4) {
      for (unsigned i = 0; i < 2; i++)
         samples[i] = nir_fadd(b, samples[i * 2], samples[i * 2 + 1]);
   }
   if (num_samples >= 2)
      samples[0] = nir_fadd(b, samples[0], samples[1]);

   return nir_fmul_imm(b, samples[0], 1.0 / num_samples); /* average the sum */
}

static nir_def *image_resolve_msaa(struct si_screen *sscreen, nir_builder *b, nir_variable *img,
                                   unsigned num_samples, nir_def *coord)
{
   nir_def *zero = nir_imm_int(b, 0);
   nir_def *result = NULL;
   nir_variable *var = NULL;

   /* Gfx11 doesn't support samples_identical, so we can't use it. */
   if (sscreen->info.gfx_level < GFX11) {
      /* We need a local variable to get the result out of conditional branches in SSA. */
      var = nir_local_variable_create(b->impl, glsl_vec4_type(), NULL);

      /* If all samples are identical, load only sample 0. */
      nir_push_if(b, nir_image_deref_samples_identical(b, 1, deref_ssa(b, img), coord));
      result = nir_image_deref_load(b, 4, 32, deref_ssa(b, img), coord, zero, zero);
      nir_store_var(b, var, result, 0xf);

      nir_push_else(b, NULL);
   }

   nir_def *sample_index[16];
   for (unsigned i = 0; i < num_samples; i++)
      sample_index[i] = nir_imm_int(b, i);

   /* We need to hide the constant sample indices behind the optimization barrier, otherwise
    * LLVM doesn't put loads into the same clause.
    *
    * TODO: nir_group_loads could do this.
    */
   if (!sscreen->use_aco) {
      for (unsigned i = 0; i < num_samples; i++)
         sample_index[i] = nir_optimization_barrier_vgpr_amd(b, 32, sample_index[i]);
   }

   /* Load all samples. */
   nir_def *samples[16];
   for (unsigned i = 0; i < num_samples; i++) {
      samples[i] = nir_image_deref_load(b, 4, 32, deref_ssa(b, img),
                                        coord, sample_index[i], zero);
   }

   result = average_samples(b, samples, num_samples);

   if (sscreen->info.gfx_level < GFX11) {
      /* Exit the conditional branch and get the result out of the branch. */
      nir_store_var(b, var, result, 0xf);
      nir_pop_if(b, NULL);
      result = nir_load_var(b, var);
   }

   return result;
}

static nir_def *apply_blit_output_modifiers(nir_builder *b, nir_def *color,
                                                const union si_compute_blit_shader_key *options)
{
   if (options->sint_to_uint)
      color = nir_imax(b, color, nir_imm_int(b, 0));

   if (options->uint_to_sint)
      color = nir_umin(b, color, nir_imm_int(b, INT32_MAX));

   if (options->dst_is_srgb)
      color = convert_linear_to_srgb(b, color);

   nir_def *zero = nir_imm_int(b, 0);
   nir_def *one = options->use_integer_one ? nir_imm_int(b, 1) : nir_imm_float(b, 1);

   /* Set channels not present in src to 0 or 1. This will eliminate code loading and resolving
    * those channels.
    */
   for (unsigned chan = options->last_src_channel + 1; chan <= options->last_dst_channel; chan++)
      color = nir_vector_insert_imm(b, color, chan == 3 ? one : zero, chan);

   /* Discard channels not present in dst. The hardware fills unstored channels with 0. */
   if (options->last_dst_channel < 3)
      color = nir_trim_vector(b, color, options->last_dst_channel + 1);

   /* Convert to FP16 with rtz to match the pixel shader. Not necessary, but it helps verify
    * the behavior of the whole shader by comparing it to the gfx blit.
    */
   if (options->fp16_rtz)
      color = nir_f2f16_rtz(b, color);

   return color;
}

/* The compute blit shader.
 *
 * Differences compared to u_blitter (the gfx blit):
 * - u_blitter doesn't preserve NaNs, but the compute blit does
 * - u_blitter has lower linear->SRGB precision because the CB block doesn't
 *   use FP32, but the compute blit does.
 *
 * Other than that, non-scaled blits are identical to u_blitter.
 *
 * Implementation details:
 * - Out-of-bounds dst coordinates are not clamped at all. The hw drops
 *   out-of-bounds stores for us.
 * - Out-of-bounds src coordinates are clamped by emulating CLAMP_TO_EDGE using
 *   the image_size NIR intrinsic.
 * - X/Y flipping just does this in the shader: -threadIDs - 1
 * - MSAA copies are implemented but disabled because MSAA image stores don't
 *   work.
 */
void *si_create_blit_cs(struct si_context *sctx, const union si_compute_blit_shader_key *options)
{
   const nir_shader_compiler_options *nir_options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, nir_options,
                                                  "blit_non_scaled_cs");
   b.shader->info.num_images = 2;
   if (options->src_is_msaa)
      BITSET_SET(b.shader->info.msaa_images, 0);
   if (options->dst_is_msaa)
      BITSET_SET(b.shader->info.msaa_images, 1);
   /* TODO: 1D blits are 8x slower because the workgroup size is 8x8 */
   b.shader->info.workgroup_size[0] = 8;
   b.shader->info.workgroup_size[1] = 8;
   b.shader->info.workgroup_size[2] = 1;
   b.shader->info.cs.user_data_components_amd = 3;

   const struct glsl_type *img_type[2] = {
      glsl_image_type(options->src_is_1d ? GLSL_SAMPLER_DIM_1D :
                      options->src_is_msaa ? GLSL_SAMPLER_DIM_MS : GLSL_SAMPLER_DIM_2D,
                      /*is_array*/ true, GLSL_TYPE_FLOAT),
      glsl_image_type(options->dst_is_1d ? GLSL_SAMPLER_DIM_1D :
                      options->dst_is_msaa ? GLSL_SAMPLER_DIM_MS : GLSL_SAMPLER_DIM_2D,
                      /*is_array*/ true, GLSL_TYPE_FLOAT),
   };

   nir_variable *img_src = nir_variable_create(b.shader, nir_var_uniform, img_type[0], "img0");
   img_src->data.binding = 0;

   nir_variable *img_dst = nir_variable_create(b.shader, nir_var_uniform, img_type[1], "img1");
   img_dst->data.binding = 1;

   nir_def *zero = nir_imm_int(&b, 0);

   /* Instructions. */
   /* Let's work with 0-based src and dst coordinates (thread IDs) first. */
   nir_def *dst_xyz = nir_pad_vector_imm_int(&b, get_global_ids(&b, options->wg_dim), 0, 3);
   nir_def *src_xyz = dst_xyz;

   /* Flip src coordinates. */
   for (unsigned i = 0; i < 2; i++) {
      if (i ? options->flip_y : options->flip_x) {
         /* x goes from 0 to (dim - 1).
          * The flipped blit should load from -dim to -1.
          * Therefore do: x = -x - 1;
          */
         nir_def *comp = nir_channel(&b, src_xyz, i);
         comp = nir_iadd_imm(&b, nir_ineg(&b, comp), -1);
         src_xyz = nir_vector_insert_imm(&b, src_xyz, comp, i);
      }
   }

   /* Add box.xyz. */
   nir_def *coord_src = NULL, *coord_dst = NULL;
   unpack_2x16_signed(&b, nir_trim_vector(&b, nir_load_user_data_amd(&b), 3),
                      &coord_src, &coord_dst);
   coord_dst = nir_iadd(&b, coord_dst, dst_xyz);
   coord_src = nir_iadd(&b, coord_src, src_xyz);

   /* Clamp to edge for src, only X and Y because Z can't be out of bounds. */
   if (options->xy_clamp_to_edge) {
      unsigned src_clamp_channels = options->src_is_1d ? 0x1 : 0x3;
      nir_def *dim = nir_image_deref_size(&b, 4, 32, deref_ssa(&b, img_src), zero);
      dim = nir_channels(&b, dim, src_clamp_channels);

      nir_def *coord_src_clamped = nir_channels(&b, coord_src, src_clamp_channels);
      coord_src_clamped = nir_imax(&b, coord_src_clamped, nir_imm_int(&b, 0));
      coord_src_clamped = nir_imin(&b, coord_src_clamped, nir_iadd_imm(&b, dim, -1));

      for (unsigned i = 0; i < util_bitcount(src_clamp_channels); i++)
         coord_src = nir_vector_insert_imm(&b, coord_src, nir_channel(&b, coord_src_clamped, i), i);
   }

   /* Swizzle coordinates for 1D_ARRAY. */
   static unsigned swizzle_xz[] = {0, 2, 0, 0};

   if (options->src_is_1d)
      coord_src = nir_swizzle(&b, coord_src, swizzle_xz, 4);
   if (options->dst_is_1d)
      coord_dst = nir_swizzle(&b, coord_dst, swizzle_xz, 4);

   /* Coordinates must have 4 channels in NIR. */
   coord_src = nir_pad_vector(&b, coord_src, 4);
   coord_dst = nir_pad_vector(&b, coord_dst, 4);

   /* TODO: out-of-bounds image stores have no effect, but we could jump over them for better perf */

   /* Execute the image loads and stores. */
   unsigned num_samples = 1 << options->log2_samples;
   nir_def *color;

   if (options->src_is_msaa && !options->dst_is_msaa && !options->sample0_only) {
      /* MSAA resolving (downsampling). */
      assert(num_samples > 1);
      color = image_resolve_msaa(sctx->screen, &b, img_src, num_samples, coord_src);
      color = apply_blit_output_modifiers(&b, color, options);
      nir_image_deref_store(&b, deref_ssa(&b, img_dst), coord_dst, zero, color, zero);

   } else if (options->src_is_msaa && options->dst_is_msaa) {
      /* MSAA copy. */
      nir_def *color[16];
      assert(num_samples > 1);
      /* Group loads together and then stores. */
      for (unsigned i = 0; i < num_samples; i++) {
         color[i] = nir_image_deref_load(&b, 4, 32, deref_ssa(&b, img_src), coord_src,
                                         nir_imm_int(&b, i), zero);
      }
      for (unsigned i = 0; i < num_samples; i++)
         color[i] = apply_blit_output_modifiers(&b, color[i], options);
      for (unsigned i = 0; i < num_samples; i++) {
         nir_image_deref_store(&b, deref_ssa(&b, img_dst), coord_dst,
                               nir_imm_int(&b, i), color[i], zero);
      }
   } else if (!options->src_is_msaa && options->dst_is_msaa) {
      /* MSAA upsampling. */
      assert(num_samples > 1);
      color = nir_image_deref_load(&b, 4, 32, deref_ssa(&b, img_src), coord_src, zero, zero);
      color = apply_blit_output_modifiers(&b, color, options);
      for (unsigned i = 0; i < num_samples; i++) {
         nir_image_deref_store(&b, deref_ssa(&b, img_dst), coord_dst,
                               nir_imm_int(&b, i), color, zero);
      }
   } else {
      /* Non-MSAA copy or read sample 0 only. */
      /* src2 = sample_index (zero), src3 = lod (zero) */
      assert(num_samples == 1);
      color = nir_image_deref_load(&b, 4, 32, deref_ssa(&b, img_src), coord_src, zero, zero);
      color = apply_blit_output_modifiers(&b, color, options);
      nir_image_deref_store(&b, deref_ssa(&b, img_dst), coord_dst, zero, color, zero);
   }

   return create_shader_state(sctx, b.shader);
}

void *si_clear_render_target_shader(struct si_context *sctx, enum pipe_texture_target type)
{
   nir_def *address;
   enum glsl_sampler_dim sampler_type;

   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b =
   nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "clear_render_target");
   b.shader->info.num_ubos = 1;
   b.shader->info.num_images = 1;
   b.shader->num_uniforms = 2;

   switch (type) {
      case PIPE_TEXTURE_1D_ARRAY:
         b.shader->info.workgroup_size[0] = 64;
         b.shader->info.workgroup_size[1] = 1;
         b.shader->info.workgroup_size[2] = 1;
         sampler_type = GLSL_SAMPLER_DIM_1D;
         address = get_global_ids(&b, 2);
         break;
      case PIPE_TEXTURE_2D_ARRAY:
         b.shader->info.workgroup_size[0] = 8;
         b.shader->info.workgroup_size[1] = 8;
         b.shader->info.workgroup_size[2] = 1;
         sampler_type = GLSL_SAMPLER_DIM_2D;
         address = get_global_ids(&b, 3);
         break;
      default:
         unreachable("unsupported texture target type");
   }

   const struct glsl_type *img_type = glsl_image_type(sampler_type, true, GLSL_TYPE_FLOAT);
   nir_variable *output_img = nir_variable_create(b.shader, nir_var_image, img_type, "image");
   output_img->data.image.format = PIPE_FORMAT_R32G32B32A32_FLOAT;

   nir_def *zero = nir_imm_int(&b, 0);
   nir_def *ubo = nir_load_ubo(&b, 4, 32, zero, zero, .range_base = 0, .range = 16);

   /* TODO: No GL CTS tests for 1D arrays, relying on OpenCL CTS for now.
    * As a sanity check, "OpenCL-CTS/test_conformance/images/clFillImage" tests should pass
    */
   if (type == PIPE_TEXTURE_1D_ARRAY) {
      unsigned swizzle[4] = {0, 2, 0, 0};
      ubo = nir_swizzle(&b, ubo, swizzle, 4);
   }

   address = nir_iadd(&b, address, ubo);
   nir_def *coord = nir_pad_vector(&b, address, 4);

   nir_def *data = nir_load_ubo(&b, 4, 32, zero, nir_imm_int(&b, 16), .range_base = 16, .range = 16);

   nir_image_deref_store(&b, &nir_build_deref_var(&b, output_img)->def, coord, zero, data, zero,
                         .image_dim = sampler_type, .image_array = true);

   return create_shader_state(sctx, b.shader);
}

void *si_clear_12bytes_buffer_shader(struct si_context *sctx)
{
   const nir_shader_compiler_options *options =
   sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b =
   nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "clear_12bytes_buffer");
   b.shader->info.workgroup_size[0] = 64;
   b.shader->info.workgroup_size[1] = 1;
   b.shader->info.workgroup_size[2] = 1;
   b.shader->info.cs.user_data_components_amd = 3;

   nir_def *offset = nir_imul_imm(&b, get_global_ids(&b, 1), 12);
   nir_def *value = nir_trim_vector(&b, nir_load_user_data_amd(&b), 3);

   nir_store_ssbo(&b, value, nir_imm_int(&b, 0), offset,
      .access = SI_COMPUTE_DST_CACHE_POLICY != L2_LRU ? ACCESS_NON_TEMPORAL : 0);

   return create_shader_state(sctx, b.shader);
}

void *si_create_ubyte_to_ushort_compute_shader(struct si_context *sctx)
{
   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   unsigned store_qualifier = ACCESS_COHERENT | ACCESS_RESTRICT;

   /* Don't cache loads, because there is no reuse. */
   unsigned load_qualifier = store_qualifier | ACCESS_NON_TEMPORAL;

   nir_builder b =
      nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "ubyte_to_ushort");

   unsigned default_wave_size = si_determine_wave_size(sctx->screen, NULL);

   b.shader->info.workgroup_size[0] = default_wave_size;
   b.shader->info.workgroup_size[1] = 1;
   b.shader->info.workgroup_size[2] = 1;
   b.shader->info.num_ssbos = 2;

   nir_def *load_address = get_global_ids(&b, 1);
   nir_def *store_address = nir_imul_imm(&b, load_address, 2);

   nir_def *ubyte_value = nir_load_ssbo(&b, 1, 8, nir_imm_int(&b, 1),
                                        load_address, .access = load_qualifier);
   nir_store_ssbo(&b, nir_u2uN(&b, ubyte_value, 16), nir_imm_int(&b, 0),
                  store_address, .access = store_qualifier);

   return create_shader_state(sctx, b.shader);
}

/* Create a compute shader implementing clear_buffer or copy_buffer. */
void *si_create_dma_compute_shader(struct si_context *sctx, unsigned num_dwords_per_thread,
                                   bool dst_stream_cache_policy, bool is_copy)
{
   assert(util_is_power_of_two_nonzero(num_dwords_per_thread));

   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   unsigned store_qualifier = ACCESS_COHERENT | ACCESS_RESTRICT;
   if (dst_stream_cache_policy)
      store_qualifier |= ACCESS_NON_TEMPORAL;

   /* Don't cache loads, because there is no reuse. */
   unsigned load_qualifier = store_qualifier | ACCESS_NON_TEMPORAL;

   nir_builder b =
      nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "create_dma_compute");

   unsigned default_wave_size = si_determine_wave_size(sctx->screen, NULL);

   b.shader->info.workgroup_size[0] = default_wave_size;
   b.shader->info.workgroup_size[1] = 1;
   b.shader->info.workgroup_size[2] = 1;
   b.shader->info.num_ssbos = 1;

   unsigned num_mem_ops = MAX2(1, num_dwords_per_thread / 4);
   unsigned *inst_dwords = alloca(num_mem_ops * sizeof(unsigned));

   for (unsigned i = 0; i < num_mem_ops; i++) {
      if (i * 4 < num_dwords_per_thread)
         inst_dwords[i] = MIN2(4, num_dwords_per_thread - i * 4);
   }

   /* If there are multiple stores,
    * the first store writes into 0 * wavesize + tid,
    * the 2nd store writes into 1 * wavesize + tid,
    * the 3rd store writes into 2 * wavesize + tid, etc.
    */
   nir_def *store_address = get_global_ids(&b, 1);

   /* Convert from a "store size unit" into bytes. */
   store_address = nir_imul_imm(&b, store_address, 4 * inst_dwords[0]);

   nir_def *load_address = store_address, *value, *values[num_mem_ops];
   value = nir_undef(&b, 1, 32);

   if (is_copy) {
      b.shader->info.num_ssbos++;
   } else {
      b.shader->info.cs.user_data_components_amd = inst_dwords[0];
      value = nir_trim_vector(&b, nir_load_user_data_amd(&b), inst_dwords[0]);
   }

   /* Distance between a load and a store for latency hiding. */
   unsigned load_store_distance = is_copy ? 8 : 0;

   for (unsigned i = 0; i < num_mem_ops + load_store_distance; i++) {
      int d = i - load_store_distance;

      if (is_copy && i < num_mem_ops) {
         if (i) {
            load_address = nir_iadd(&b, load_address,
                                    nir_imm_int(&b, 4 * inst_dwords[i] * default_wave_size));
         }
         values[i] = nir_load_ssbo(&b, 4, 32, nir_imm_int(&b, 1),load_address,
                                   .access = load_qualifier);
      }

      if (d >= 0) {
         if (d) {
            store_address = nir_iadd(&b, store_address,
                                     nir_imm_int(&b, 4 * inst_dwords[d] * default_wave_size));
         }
         nir_store_ssbo(&b, is_copy ? values[d] : value, nir_imm_int(&b, 0), store_address,
                        .access = store_qualifier);
      }
   }

   return create_shader_state(sctx, b.shader);
}

/* Load samples from the image, and copy them to the same image. This looks like
 * a no-op, but it's not. Loads use FMASK, while stores don't, so samples are
 * reordered to match expanded FMASK.
 *
 * After the shader finishes, FMASK should be cleared to identity.
 */
void *si_create_fmask_expand_cs(struct si_context *sctx, unsigned num_samples, bool is_array)
{
   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b =
      nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "create_fmask_expand_cs");
   b.shader->info.workgroup_size[0] = 8;
   b.shader->info.workgroup_size[1] = 8;
   b.shader->info.workgroup_size[2] = 1;

   /* Return an empty compute shader */
   if (num_samples == 0)
      return create_shader_state(sctx, b.shader);

   b.shader->info.num_images = 1;

   const struct glsl_type *img_type = glsl_image_type(GLSL_SAMPLER_DIM_MS, is_array, GLSL_TYPE_FLOAT);
   nir_variable *img = nir_variable_create(b.shader, nir_var_image, img_type, "image");
   img->data.access = ACCESS_RESTRICT;

   nir_def *z = nir_undef(&b, 1, 32);
   if (is_array) {
      z = nir_channel(&b, nir_load_workgroup_id(&b), 2);
   }

   nir_def *zero = nir_imm_int(&b, 0);
   nir_def *address = get_global_ids(&b, 2);

   nir_def *sample[8], *addresses[8];
   assert(num_samples <= ARRAY_SIZE(sample));

   nir_def *img_def = &nir_build_deref_var(&b, img)->def;

   /* Load samples, resolving FMASK. */
   for (unsigned i = 0; i < num_samples; i++) {
      nir_def *it = nir_imm_int(&b, i);
      sample[i] = nir_vec4(&b, nir_channel(&b, address, 0), nir_channel(&b, address, 1), z, it);
      addresses[i] = nir_image_deref_load(&b, 4, 32, img_def, sample[i], it, zero,
                                          .access = ACCESS_RESTRICT,
                                          .image_dim = GLSL_SAMPLER_DIM_2D,
                                          .image_array = is_array);
   }

   /* Store samples, ignoring FMASK. */
   for (unsigned i = 0; i < num_samples; i++) {
      nir_image_deref_store(&b, img_def, sample[i], nir_imm_int(&b, i), addresses[i], zero,
                            .access = ACCESS_RESTRICT,
                            .image_dim = GLSL_SAMPLER_DIM_2D,
                            .image_array = is_array);
   }

   return create_shader_state(sctx, b.shader);
}

/* This is just a pass-through shader with 1-3 MOV instructions. */
void *si_get_blitter_vs(struct si_context *sctx, enum blitter_attrib_type type, unsigned num_layers)
{
   unsigned vs_blit_property;
   void **vs;

   switch (type) {
   case UTIL_BLITTER_ATTRIB_NONE:
      vs = num_layers > 1 ? &sctx->vs_blit_pos_layered : &sctx->vs_blit_pos;
      vs_blit_property = SI_VS_BLIT_SGPRS_POS;
      break;
   case UTIL_BLITTER_ATTRIB_COLOR:
      vs = num_layers > 1 ? &sctx->vs_blit_color_layered : &sctx->vs_blit_color;
      vs_blit_property = SI_VS_BLIT_SGPRS_POS_COLOR;
      break;
   case UTIL_BLITTER_ATTRIB_TEXCOORD_XY:
   case UTIL_BLITTER_ATTRIB_TEXCOORD_XYZW:
      assert(num_layers == 1);
      vs = &sctx->vs_blit_texcoord;
      vs_blit_property = SI_VS_BLIT_SGPRS_POS_TEXCOORD;
      break;
   default:
      assert(0);
      return NULL;
   }

   if (*vs)
      return *vs;

   /* Add 1 for the attribute ring address. */
   if (sctx->gfx_level >= GFX11 && type != UTIL_BLITTER_ATTRIB_NONE)
      vs_blit_property++;

   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_VERTEX);

   nir_builder b =
      nir_builder_init_simple_shader(MESA_SHADER_VERTEX, options, "get_blitter_vs");

   /* Tell the shader to load VS inputs from SGPRs: */
   b.shader->info.vs.blit_sgprs_amd = vs_blit_property;
   b.shader->info.vs.window_space_position = true;

   const struct glsl_type *vec4 = glsl_vec4_type();

   nir_copy_var(&b,
                nir_create_variable_with_location(b.shader, nir_var_shader_out,
                                                  VARYING_SLOT_POS, vec4),
                nir_create_variable_with_location(b.shader, nir_var_shader_in,
                                                  VERT_ATTRIB_GENERIC0, vec4));

   if (type != UTIL_BLITTER_ATTRIB_NONE) {
      nir_copy_var(&b,
                   nir_create_variable_with_location(b.shader, nir_var_shader_out,
                                                     VARYING_SLOT_VAR0, vec4),
                   nir_create_variable_with_location(b.shader, nir_var_shader_in,
                                                     VERT_ATTRIB_GENERIC1, vec4));
   }

   if (num_layers > 1) {
      nir_variable *out_layer =
         nir_create_variable_with_location(b.shader, nir_var_shader_out,
                                           VARYING_SLOT_LAYER, glsl_int_type());
      out_layer->data.interpolation = INTERP_MODE_NONE;

      nir_copy_var(&b, out_layer,
                   nir_create_variable_with_location(b.shader, nir_var_system_value,
                                                     SYSTEM_VALUE_INSTANCE_ID, glsl_int_type()));
   }

   *vs = create_shader_state(sctx, b.shader);
   return *vs;
}

/* Create the compute shader that is used to collect the results.
 *
 * One compute grid with a single thread is launched for every query result
 * buffer. The thread (optionally) reads a previous summary buffer, then
 * accumulates data from the query result buffer, and writes the result either
 * to a summary buffer to be consumed by the next grid invocation or to the
 * user-supplied buffer.
 *
 * Data layout:
 *
 * CONST
 *  0.x = end_offset
 *  0.y = result_stride
 *  0.z = result_count
 *  0.w = bit field:
 *          1: read previously accumulated values
 *          2: write accumulated values for chaining
 *          4: write result available
 *          8: convert result to boolean (0/1)
 *         16: only read one dword and use that as result
 *         32: apply timestamp conversion
 *         64: store full 64 bits result
 *        128: store signed 32 bits result
 *        256: SO_OVERFLOW mode: take the difference of two successive half-pairs
 *  1.x = fence_offset
 *  1.y = pair_stride
 *  1.z = pair_count
 *
 */
void *si_create_query_result_cs(struct si_context *sctx)
{
   const nir_shader_compiler_options *options =
      sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b =
      nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "create_query_result_cs");
   b.shader->info.workgroup_size[0] = 1;
   b.shader->info.workgroup_size[1] = 1;
   b.shader->info.workgroup_size[2] = 1;
   b.shader->info.num_ubos = 1;
   b.shader->info.num_ssbos = 3;
   b.shader->num_uniforms = 2;

   nir_def *var_undef = nir_undef(&b, 1, 32);
   nir_def *zero = nir_imm_int(&b, 0);
   nir_def *one = nir_imm_int(&b, 1);
   nir_def *two = nir_imm_int(&b, 2);
   nir_def *four = nir_imm_int(&b, 4);
   nir_def *eight = nir_imm_int(&b, 8);
   nir_def *sixteen = nir_imm_int(&b, 16);
   nir_def *thirty_one = nir_imm_int(&b, 31);
   nir_def *sixty_four = nir_imm_int(&b, 64);

   /* uint32_t x, y, z = 0; */
   nir_function_impl *e = nir_shader_get_entrypoint(b.shader);
   nir_variable *x = nir_local_variable_create(e, glsl_uint_type(), "x");
   nir_store_var(&b, x, var_undef, 0x1);
   nir_variable *y = nir_local_variable_create(e, glsl_uint_type(), "y");
   nir_store_var(&b, y, var_undef, 0x1);
   nir_variable *z = nir_local_variable_create(e, glsl_uint_type(), "z");
   nir_store_var(&b, z, zero, 0x1);

   /* uint32_t buff_0[4] = load_ubo(0, 0); */
   nir_def *buff_0 = nir_load_ubo(&b, 4, 32, zero, zero, .range_base = 0, .range = 16);
   /* uint32_t buff_1[4] = load_ubo(1, 16); */
   nir_def *buff_1 = nir_load_ubo(&b, 4, 32, zero, sixteen, .range_base = 16, .range = 16);

   /* uint32_t b0_bitfield = buff_0.w; */
   nir_def *b0_bitfield = nir_channel(&b, buff_0, 3);

   /* Check result availability.
    *    if (b0_bitfield & (1u << 4)) {
    *       ...
    */
   nir_def *is_one_dword_result = nir_i2b(&b, nir_iand(&b, b0_bitfield, sixteen));
   nir_if *if_one_dword_result = nir_push_if(&b, is_one_dword_result); {

      /*   int32_t value = load_ssbo(0, fence_offset);
       *   z = ~(value >> 31);
       */
      nir_def *value = nir_load_ssbo(&b, 1, 32, zero, nir_channel(&b, buff_1, 0));
      nir_def *bitmask = nir_inot(&b, nir_ishr(&b, value, thirty_one));
      nir_store_var(&b, z, bitmask, 0x1);

      /* Load result if available.
       *    if (value < 0) {
       *       uint32_t result[2] = load_ssbo(0, 0);
       *       x = result[0];
       *       y = result[1];
       *    }
       */
      nir_if *if_negative = nir_push_if(&b, nir_ilt(&b, value, zero)); {
         nir_def *result = nir_load_ssbo(&b, 2, 32, zero, zero);
         nir_store_var(&b, x, nir_channel(&b, result, 0), 0x1);
         nir_store_var(&b, y, nir_channel(&b, result, 1), 0x1);
      }
      nir_pop_if(&b, if_negative);
   } nir_push_else(&b, if_one_dword_result); {

      /* } else {
       *    x = 0; y = 0;
       */
      nir_store_var(&b, x, zero, 0x1);
      nir_store_var(&b, y, zero, 0x1);

      /* Load previously accumulated result if requested.
       *    if (b0_bitfield & (1u << 0)) {
       *       uint32_t result[3] = load_ssbo(1, 0);
       *       x = result[0];
       *       y = result[1];
       *       z = result[2];
       *    }
       */
      nir_def *is_prev_acc_result = nir_i2b(&b, nir_iand(&b, b0_bitfield, one));
      nir_if *if_prev_acc_result = nir_push_if(&b, is_prev_acc_result); {
         nir_def *result = nir_load_ssbo(&b, 3, 32, one, zero);
         nir_store_var(&b, x, nir_channel(&b, result, 0), 0x1);
         nir_store_var(&b, y, nir_channel(&b, result, 1), 0x1);
         nir_store_var(&b, z, nir_channel(&b, result, 2), 0x1);
      }
      nir_pop_if(&b, if_prev_acc_result);

      /* if (!z) {
       *    uint32_t result_index = 0;
       *    uint32_t pitch = 0;
       *    ...
       */
      nir_def *z_value = nir_load_var(&b, z);
      nir_if *if_not_z = nir_push_if(&b, nir_ieq(&b, z_value, zero)); {
         nir_variable *outer_loop_iter =
            nir_local_variable_create(e, glsl_uint_type(), "outer_loop_iter");
         nir_store_var(&b, outer_loop_iter, zero, 0x1);
         nir_variable *pitch = nir_local_variable_create(e, glsl_uint_type(), "pitch");
         nir_store_var(&b, pitch, zero, 0x1);

         /* Outer loop.
          *   while (result_index <= result_count) {
          *      ...
          */
         nir_loop *loop_outer = nir_push_loop(&b); {
            nir_def *result_index = nir_load_var(&b, outer_loop_iter);
            nir_def *is_result_index_out_of_bound =
               nir_uge(&b, result_index, nir_channel(&b, buff_0, 2));
            nir_if *if_out_of_bound = nir_push_if(&b, is_result_index_out_of_bound); {
               nir_jump(&b, nir_jump_break);
            }
            nir_pop_if(&b, if_out_of_bound);

            /* Load fence and check result availability.
             *    pitch = i * result_stride;
             *    uint32_t address = fence_offset + pitch;
             *    int32_t value = load_ssbo(0, address);
             *    z = ~(value >> 31);
             */
            nir_def *pitch_outer_loop = nir_imul(&b, result_index, nir_channel(&b, buff_0, 1));
            nir_store_var(&b, pitch, pitch_outer_loop, 0x1);
            nir_def *address = nir_iadd(&b, pitch_outer_loop, nir_channel(&b, buff_1, 0));
            nir_def *value = nir_load_ssbo(&b, 1, 32, zero, address);
            nir_def *bitmask = nir_inot(&b, nir_ishr(&b, value, thirty_one));
            nir_store_var(&b, z, bitmask, 0x1);

            /*    if (z) {
             *       break;
             *    }
             */
            nir_if *if_result_available = nir_push_if(&b, nir_i2b(&b, bitmask)); {
               nir_jump(&b, nir_jump_break);
            }
            nir_pop_if(&b, if_result_available);

            /* Inner loop iterator.
             *    uint32_t i = 0;
             */
            nir_variable *inner_loop_iter =
               nir_local_variable_create(e, glsl_uint_type(), "inner_loop_iter");
            nir_store_var(&b, inner_loop_iter, zero, 0x1);

            /* Inner loop.
             *    do {
             *       ...
             */
            nir_loop *loop_inner = nir_push_loop(&b); {
               nir_def *pitch_inner_loop = nir_load_var(&b, pitch);
               nir_def *i = nir_load_var(&b, inner_loop_iter);

               /* Load start and end.
                *    uint64_t first = load_ssbo(0, pitch);
                *    uint64_t second = load_ssbo(0, pitch + end_offset);
                *    uint64_t start_half_pair = second - first;
                */
               nir_def *first = nir_load_ssbo(&b, 1, 64, zero, pitch_inner_loop);
               nir_def *new_pitch = nir_iadd(&b, pitch_inner_loop, nir_channel(&b, buff_0, 0));
               nir_def *second = nir_load_ssbo(&b, 1, 64, zero, new_pitch);
               nir_def *start_half_pair = nir_isub(&b, second, first);

               /* Load second start/end half-pair and take the difference.
                *    if (b0_bitfield & (1u << 8)) {
                *       uint64_t first = load_ssbo(0, pitch + 8);
                *       uint64_t second = load_ssbo(0, pitch + end_offset + 8);
                *       uint64_t end_half_pair = second - first;
                *       uint64_t difference = start_half_pair - end_half_pair;
                *    }
                */
               nir_def *difference;
               nir_def *is_so_overflow_mode = nir_i2b(&b, nir_iand_imm(&b, b0_bitfield, 256));
               nir_if *if_so_overflow_mode = nir_push_if(&b, is_so_overflow_mode); {
                  first = nir_load_ssbo(&b, 1, 64, zero, nir_iadd(&b, pitch_inner_loop, eight));
                  second = nir_load_ssbo(&b, 1, 64, zero, nir_iadd(&b, new_pitch, eight));
                  nir_def *end_half_pair = nir_isub(&b, second, first);
                  difference = nir_isub(&b, start_half_pair, end_half_pair);
               }
               nir_pop_if(&b, if_so_overflow_mode);

               /* uint64_t sum = (x | (uint64_t) y << 32) + difference; */
               nir_def *sum = nir_iadd(&b,
                                       nir_pack_64_2x32_split(&b,
                                                              nir_load_var(&b, x),
                                                              nir_load_var(&b, y)),
                                       nir_if_phi(&b, difference, start_half_pair));
               sum = nir_unpack_64_2x32(&b, sum);

               /* Increment inner loop iterator.
                *    i++;
                */
               i = nir_iadd(&b, i, one);
               nir_store_var(&b, inner_loop_iter, i, 0x1);

               /* Update pitch value.
                *    pitch = i * pair_stride + pitch;
                */
               nir_def *incremented_pitch = nir_iadd(&b,
                                             nir_imul(&b, i, nir_channel(&b, buff_1, 1)),
                                             pitch_outer_loop);
               nir_store_var(&b, pitch, incremented_pitch, 0x1);

               /* Update x and y.
                *    x = sum.x;
                *    y = sum.x >> 32;
                */
               nir_store_var(&b, x, nir_channel(&b, sum, 0), 0x1);
               nir_store_var(&b, y, nir_channel(&b, sum, 1), 0x1);

               /* } while (i < pair_count);
               */
               nir_def *is_pair_count_exceeded = nir_uge(&b, i, nir_channel(&b, buff_1, 2));
               nir_if *if_pair_count_exceeded = nir_push_if(&b, is_pair_count_exceeded); {
                  nir_jump(&b, nir_jump_break);
               }
               nir_pop_if(&b, if_pair_count_exceeded);
            }
            nir_pop_loop(&b, loop_inner);

            /* Increment pair iterator.
             *    result_index++;
             */
            nir_store_var(&b, outer_loop_iter, nir_iadd(&b, result_index, one), 0x1);
         }
         nir_pop_loop(&b, loop_outer);
      }
      nir_pop_if(&b, if_not_z);
   }
   nir_pop_if(&b, if_one_dword_result);

   nir_def *x_value = nir_load_var(&b, x);
   nir_def *y_value = nir_load_var(&b, y);
   nir_def *z_value = nir_load_var(&b, z);

   /* Store accumulated data for chaining.
    *    if (b0_bitfield & (1u << 1)) {
    *       store_ssbo(<x, y, z>, 2, 0);
    */
   nir_def *is_acc_chaining = nir_i2b(&b, nir_iand(&b, b0_bitfield, two));
   nir_if *if_acc_chaining = nir_push_if(&b, is_acc_chaining); {
      nir_store_ssbo(&b, nir_vec3(&b, x_value, y_value, z_value), two, zero);
   } nir_push_else(&b, if_acc_chaining); {

      /* Store result availability.
       *    } else {
       *       if (b0_bitfield & (1u << 2)) {
       *          store_ssbo((~z & 1), 2, 0);
       *          ...
       */
      nir_def *is_result_available = nir_i2b(&b, nir_iand(&b, b0_bitfield, four));
      nir_if *if_result_available = nir_push_if(&b, is_result_available); {
         nir_store_ssbo(&b, nir_iand(&b, nir_inot(&b, z_value), one), two, zero);

         /* Store full 64 bits result.
          *    if (b0_bitfield & (1u << 6)) {
          *       store_ssbo(<0, 0>, 2, 0);
          *    }
          */
         nir_def *is_result_64_bits = nir_i2b(&b, nir_iand(&b, b0_bitfield, sixty_four));
         nir_if *if_result_64_bits = nir_push_if(&b, is_result_64_bits); {
            nir_store_ssbo(&b, nir_imm_ivec2(&b, 0, 0), two, zero,
                           .write_mask = (1u << 1));
         }
         nir_pop_if(&b, if_result_64_bits);
      } nir_push_else(&b, if_result_available); {

         /* } else {
          *    if (~z) {
          *       ...
          */
         nir_def *is_bitwise_not_z = nir_i2b(&b, nir_inot(&b, z_value));
         nir_if *if_bitwise_not_z = nir_push_if(&b, is_bitwise_not_z); {
            nir_def *ts_x, *ts_y;

            /* Apply timestamp conversion.
             *    if (b0_bitfield & (1u << 5)) {
             *       uint64_t xy_million = (x | (uint64_t) y << 32) * (uint64_t) 1000000;
             *       uint64_t ts_converted = xy_million / (uint64_t) clock_crystal_frequency;
             *       x = ts_converted.x;
             *       y = ts_converted.x >> 32;
             *    }
             */
            nir_def *is_apply_timestamp = nir_i2b(&b, nir_iand_imm(&b, b0_bitfield, 32));
            nir_if *if_apply_timestamp = nir_push_if(&b, is_apply_timestamp); {
               /* Add the frequency into the shader for timestamp conversion
                * so that the backend can use the full range of optimizations
                * for divide-by-constant.
                */
               nir_def *clock_crystal_frequency =
                  nir_imm_int64(&b, sctx->screen->info.clock_crystal_freq);

               nir_def *xy_million = nir_imul(&b,
                                           nir_pack_64_2x32_split(&b, x_value, y_value),
                                           nir_imm_int64(&b, 1000000));
               nir_def *ts_converted = nir_udiv(&b, xy_million, clock_crystal_frequency);
               ts_converted = nir_unpack_64_2x32(&b, ts_converted);
               ts_x = nir_channel(&b, ts_converted, 0);
               ts_y = nir_channel(&b, ts_converted, 1);
            }
            nir_pop_if(&b, if_apply_timestamp);

            nir_def *nx = nir_if_phi(&b, ts_x, x_value);
            nir_def *ny = nir_if_phi(&b, ts_y, y_value);

            /* x = b0_bitfield & (1u << 3) ? ((x | (uint64_t) y << 32) != 0) : x;
             * y = b0_bitfield & (1u << 3) ? 0 : y;
             */
            nir_def *is_convert_to_bool = nir_i2b(&b, nir_iand(&b, b0_bitfield, eight));
            nir_def *xy = nir_pack_64_2x32_split(&b, nx, ny);
            nir_def *is_xy = nir_b2i32(&b, nir_ine(&b, xy, nir_imm_int64(&b, 0)));
            nx = nir_bcsel(&b, is_convert_to_bool, is_xy, nx);
            ny = nir_bcsel(&b, is_convert_to_bool, zero, ny);

            /* if (b0_bitfield & (1u << 6)) {
             *    store_ssbo(<x, y>, 2, 0);
             * }
             */
            nir_def *is_result_64_bits = nir_i2b(&b, nir_iand(&b, b0_bitfield, sixty_four));
            nir_if *if_result_64_bits = nir_push_if(&b, is_result_64_bits); {
               nir_store_ssbo(&b, nir_vec2(&b, nx, ny), two, zero);
            } nir_push_else(&b, if_result_64_bits); {

               /* Clamping.
                *    } else {
                *       x = y ? UINT32_MAX : x;
                *       x = b0_bitfield & (1u << 7) ? min(x, INT_MAX) : x;
                *       store_ssbo(x, 2, 0);
                *    }
                */
               nir_def *is_y = nir_ine(&b, ny, zero);
               nx = nir_bcsel(&b, is_y, nir_imm_int(&b, UINT32_MAX), nx);
               nir_def *is_signed_32bit_result = nir_i2b(&b, nir_iand_imm(&b, b0_bitfield, 128));
               nir_def *min = nir_umin(&b, nx, nir_imm_int(&b, INT_MAX));
               nx = nir_bcsel(&b, is_signed_32bit_result, min, nx);
               nir_store_ssbo(&b, nx, two, zero);
            }
            nir_pop_if(&b, if_result_64_bits);
         }
         nir_pop_if(&b, if_bitwise_not_z);
      }
      nir_pop_if(&b, if_result_available);
   }
   nir_pop_if(&b, if_acc_chaining);

   return create_shader_state(sctx, b.shader);
}

/* Create the compute shader that is used to collect the results of gfx10+
 * shader queries.
 *
 * One compute grid with a single thread is launched for every query result
 * buffer. The thread (optionally) reads a previous summary buffer, then
 * accumulates data from the query result buffer, and writes the result either
 * to a summary buffer to be consumed by the next grid invocation or to the
 * user-supplied buffer.
 *
 * Data layout:
 *
 * CONST
 *  0.x = config;
 *          [0:2] the low 3 bits indicate the mode:
 *             0: sum up counts
 *             1: determine result availability and write it as a boolean
 *             2: SO_OVERFLOW
 *          3: SO_ANY_OVERFLOW
 *        the remaining bits form a bitfield:
 *          8: write result as a 64-bit value
 *  0.y = offset in bytes to counts or stream for SO_OVERFLOW mode
 *  0.z = chain bit field:
 *          1: have previous summary buffer
 *          2: write next summary buffer
 *  0.w = result_count
 */
void *gfx11_create_sh_query_result_cs(struct si_context *sctx)
{
   const nir_shader_compiler_options *options =
   sctx->b.screen->get_compiler_options(sctx->b.screen, PIPE_SHADER_IR_NIR, PIPE_SHADER_COMPUTE);

   nir_builder b =
      nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "gfx11_create_sh_query_result_cs");
   b.shader->info.workgroup_size[0] = 1;
   b.shader->info.workgroup_size[1] = 1;
   b.shader->info.workgroup_size[2] = 1;
   b.shader->info.num_ubos = 1;
   b.shader->info.num_ssbos = 3;
   b.shader->num_uniforms = 1;

   nir_def *zero = nir_imm_int(&b, 0);
   nir_def *one = nir_imm_int(&b, 1);
   nir_def *two = nir_imm_int(&b, 2);
   nir_def *four = nir_imm_int(&b, 4);
   nir_def *minus_one = nir_imm_int(&b, 0xffffffff);

   /* uint32_t acc_result = 0, acc_missing = 0; */
   nir_function_impl *e = nir_shader_get_entrypoint(b.shader);
   nir_variable *acc_result = nir_local_variable_create(e, glsl_uint_type(), "acc_result");
   nir_store_var(&b, acc_result, zero, 0x1);
   nir_variable *acc_missing = nir_local_variable_create(e, glsl_uint_type(), "acc_missing");
   nir_store_var(&b, acc_missing, zero, 0x1);

   /* uint32_t buff_0[4] = load_ubo(0, 0); */
   nir_def *buff_0 = nir_load_ubo(&b, 4, 32, zero, zero, .range_base = 0, .range = 16);

   /* if((chain & 1) {
    *    uint32_t result[2] = load_ssbo(1, 0);
    *    acc_result = result[0];
    *    acc_missing = result[1];
    * }
    */
   nir_def *is_prev_summary_buffer = nir_i2b(&b, nir_iand(&b, nir_channel(&b, buff_0, 2), one));
   nir_if *if_prev_summary_buffer = nir_push_if(&b, is_prev_summary_buffer); {
      nir_def *result = nir_load_ssbo(&b, 2, 32, one, zero);
         nir_store_var(&b, acc_result, nir_channel(&b, result, 0), 0x1);
         nir_store_var(&b, acc_missing, nir_channel(&b, result, 1), 0x1);
   }
   nir_pop_if(&b, if_prev_summary_buffer);

   /* uint32_t mode = config & 0b111;
    * bool is_overflow = mode >= 2;
    */
   nir_def *mode = nir_iand_imm(&b, nir_channel(&b, buff_0, 0), 0b111);
   nir_def *is_overflow = nir_uge(&b, mode, two);

   /* uint32_t result_remaining = (is_overflow && acc_result) ? 0 : result_count; */
   nir_variable *result_remaining = nir_local_variable_create(e, glsl_uint_type(), "result_remaining");
   nir_variable *base_offset = nir_local_variable_create(e, glsl_uint_type(), "base_offset");
   nir_def *state = nir_iand(&b,
                             nir_isub(&b, zero, nir_b2i32(&b, is_overflow)),
                             nir_load_var(&b, acc_result));
   nir_def *value = nir_bcsel(&b, nir_i2b(&b, state), zero, nir_channel(&b, buff_0, 3));
   nir_store_var(&b, result_remaining, value, 0x1);

   /* uint32_t base_offset = 0; */
   nir_store_var(&b, base_offset, zero, 0x1);

   /* Outer loop begin.
    *   while (!result_remaining) {
    *      ...
    */
   nir_loop *loop_outer = nir_push_loop(&b); {
      nir_def *condition = nir_load_var(&b, result_remaining);
      nir_if *if_not_condition = nir_push_if(&b, nir_ieq(&b, condition, zero)); {
         nir_jump(&b, nir_jump_break);
      }
      nir_pop_if(&b, if_not_condition);

      /* result_remaining--; */
      condition = nir_iadd(&b, condition, minus_one);
      nir_store_var(&b, result_remaining, condition, 0x1);

      /* uint32_t fence = load_ssbo(0, base_offset + sizeof(gfx11_sh_query_buffer_mem.stream)); */
      nir_def *b_offset = nir_load_var(&b, base_offset);
      uint64_t buffer_mem_stream_size = sizeof(((struct gfx11_sh_query_buffer_mem*)0)->stream);
      nir_def *fence = nir_load_ssbo(&b, 1, 32, zero,
                                    nir_iadd_imm(&b, b_offset, buffer_mem_stream_size));

      /* if (!fence) {
       *    acc_missing = ~0u;
       *    break;
       * }
       */
      nir_def *is_zero = nir_ieq(&b, fence, zero);
      nir_def *y_value = nir_isub(&b, zero, nir_b2i32(&b, is_zero));
      nir_store_var(&b, acc_missing, y_value, 0x1);
      nir_if *if_ssbo_zero = nir_push_if(&b, is_zero); {
         nir_jump(&b, nir_jump_break);
      }
      nir_pop_if(&b, if_ssbo_zero);

      /* stream_offset = base_offset + offset; */
      nir_def *s_offset = nir_iadd(&b, b_offset, nir_channel(&b, buff_0, 1));

      /* if (!(config & 7)) {
       *    acc_result += buffer[0]@stream_offset;
       * }
       */
      nir_if *if_sum_up_counts = nir_push_if(&b, nir_ieq(&b, mode, zero)); {
         nir_def *x_value = nir_load_ssbo(&b, 1, 32, zero, s_offset);
         x_value = nir_iadd(&b, nir_load_var(&b, acc_result), x_value);
         nir_store_var(&b, acc_result, x_value, 0x1);
      }
      nir_pop_if(&b, if_sum_up_counts);

      /* if (is_overflow) {
       *    uint32_t count = (config & 1) ? 4 : 1;
       *    ...
       */
      nir_if *if_overflow = nir_push_if(&b, is_overflow); {
         nir_def *is_result_available = nir_i2b(&b, nir_iand(&b, mode, one));
         nir_def *initial_count = nir_bcsel(&b, is_result_available, four, one);

         nir_variable *count =
            nir_local_variable_create(e, glsl_uint_type(), "count");
         nir_store_var(&b, count, initial_count, 0x1);

         nir_variable *stream_offset =
            nir_local_variable_create(e, glsl_uint_type(), "stream_offset");
         nir_store_var(&b, stream_offset, s_offset, 0x1);

         /* Inner loop begin.
          *    do {
          *       ...
          */
         nir_loop *loop_inner = nir_push_loop(&b); {
            /* uint32_t buffer[4] = load_ssbo(0, stream_offset + 2 * sizeof(uint64_t)); */
            nir_def *stream_offset_value = nir_load_var(&b, stream_offset);
            nir_def *buffer =
               nir_load_ssbo(&b, 4, 32, zero,
                             nir_iadd_imm(&b, stream_offset_value, 2 * sizeof(uint64_t)));

            /* if (generated != emitted) {
             *    acc_result = 1;
             *    base_offset = 0;
             *    break;
             * }
             */
            nir_def *generated = nir_channel(&b, buffer, 0);
            nir_def *emitted = nir_channel(&b, buffer, 2);
            nir_if *if_not_equal = nir_push_if(&b, nir_ine(&b, generated, emitted)); {
               nir_store_var(&b, acc_result, one, 0x1);
               nir_store_var(&b, base_offset, zero, 0x1);
               nir_jump(&b, nir_jump_break);
            }
            nir_pop_if(&b, if_not_equal);

            /* stream_offset += sizeof(gfx11_sh_query_buffer_mem.stream[0]); */
            uint64_t buffer_mem_stream0_size =
               sizeof(((struct gfx11_sh_query_buffer_mem*)0)->stream[0]);
            stream_offset_value = nir_iadd_imm(&b, stream_offset_value, buffer_mem_stream0_size);
            nir_store_var(&b, stream_offset, stream_offset_value, 0x1);

            /* } while(count--); */
            nir_def *loop_count = nir_load_var(&b, count);
            loop_count = nir_iadd(&b, loop_count, minus_one);
            nir_store_var(&b, count, loop_count, 0x1);

            nir_if *if_zero = nir_push_if(&b, nir_ieq(&b, loop_count, zero)); {
               nir_jump(&b, nir_jump_break);
            }
            nir_pop_if(&b, if_zero);
         }
         nir_pop_loop(&b, loop_inner); /* Inner loop end */
      }
      nir_pop_if(&b, if_overflow);

      /* base_offset += sizeof(gfx11_sh_query_buffer_mem); */
      nir_def *buffer_mem_size = nir_imm_int(&b, sizeof(struct gfx11_sh_query_buffer_mem));
      nir_store_var(&b, base_offset, nir_iadd(&b, nir_load_var(&b, base_offset), buffer_mem_size), 0x1);
   }
   nir_pop_loop(&b, loop_outer); /* Outer loop end */

   nir_def *acc_result_value = nir_load_var(&b, acc_result);
   nir_def *y_value = nir_load_var(&b, acc_missing);

   /* if ((chain & 2)) {
    *    store_ssbo(<acc_result, acc_missing>, 2, 0);
    *    ...
    */
   nir_def *is_write_summary_buffer = nir_i2b(&b, nir_iand(&b, nir_channel(&b, buff_0, 2), two));
   nir_if *if_write_summary_buffer = nir_push_if(&b, is_write_summary_buffer); {
      nir_store_ssbo(&b, nir_vec2(&b, acc_result_value, y_value), two, zero);
   } nir_push_else(&b, if_write_summary_buffer); {

      /* } else {
       *    if ((config & 7) == 1) {
       *       acc_result = acc_missing ? 0 : 1;
       *       acc_missing = 0;
       *    }
       *    ...
       */
      nir_def *is_result_available = nir_ieq(&b, mode, one);
      nir_def *is_zero = nir_ieq(&b, y_value, zero);
      acc_result_value = nir_bcsel(&b, is_result_available, nir_b2i32(&b, is_zero), acc_result_value);
      nir_def *ny = nir_bcsel(&b, is_result_available, zero, y_value);

      /* if (!acc_missing) {
       *    store_ssbo(acc_result, 2, 0);
       *    if (config & 8)) {
       *       store_ssbo(0, 2, 4)
       *    }
       * }
       */
      nir_if *if_zero = nir_push_if(&b, nir_ieq(&b, ny, zero)); {
         nir_store_ssbo(&b, acc_result_value, two, zero);

         nir_def *is_so_any_overflow = nir_i2b(&b, nir_iand_imm(&b, nir_channel(&b, buff_0, 0), 8));
         nir_if *if_so_any_overflow = nir_push_if(&b, is_so_any_overflow); {
            nir_store_ssbo(&b, zero, two, four);
         }
         nir_pop_if(&b, if_so_any_overflow);
      }
      nir_pop_if(&b, if_zero);
   }
   nir_pop_if(&b, if_write_summary_buffer);

   return create_shader_state(sctx, b.shader);
}
