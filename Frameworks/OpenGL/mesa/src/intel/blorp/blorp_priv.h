/*
 * Copyright © 2012 Intel Corporation
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

#ifndef BLORP_PRIV_H
#define BLORP_PRIV_H

#include <stdint.h>

#include "common/intel_measure.h"
#include "compiler/nir/nir.h"
#include "compiler/brw_compiler.h"

#include "blorp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Binding table indices used by BLORP.
 */
enum {
   BLORP_RENDERBUFFER_BT_INDEX,
   BLORP_TEXTURE_BT_INDEX,
   BLORP_NUM_BT_ENTRIES
};

#define BLORP_SAMPLER_INDEX 0

struct brw_blorp_surface_info
{
   bool enabled;

   struct isl_surf surf;
   struct blorp_address addr;

   struct isl_surf aux_surf;
   struct blorp_address aux_addr;
   enum isl_aux_usage aux_usage;

   union isl_color_value clear_color;
   struct blorp_address clear_color_addr;

   struct isl_view view;

   /* Z offset into a 3-D texture or slice of a 2-D array texture. */
   float z_offset;

   uint32_t tile_x_sa, tile_y_sa;
};

void
brw_blorp_surface_info_init(struct blorp_batch *batch,
                            struct brw_blorp_surface_info *info,
                            const struct blorp_surf *surf,
                            unsigned int level, float layer,
                            enum isl_format format, bool is_dest);
void
blorp_surf_convert_to_single_slice(const struct isl_device *isl_dev,
                                   struct brw_blorp_surface_info *info);
void
surf_fake_rgb_with_red(const struct isl_device *isl_dev,
                       struct brw_blorp_surface_info *info);
void
blorp_surf_convert_to_uncompressed(const struct isl_device *isl_dev,
                                   struct brw_blorp_surface_info *info,
                                   uint32_t *x, uint32_t *y,
                                   uint32_t *width, uint32_t *height);
void
blorp_surf_fake_interleaved_msaa(const struct isl_device *isl_dev,
                                 struct brw_blorp_surface_info *info);
void
blorp_surf_retile_w_to_y(const struct isl_device *isl_dev,
                         struct brw_blorp_surface_info *info);


struct brw_blorp_coord_transform
{
   float multiplier;
   float offset;
};

/**
 * Bounding rectangle telling pixel discard which pixels are to be touched.
 * This is needed in when surfaces are configured as something else what they
 * really are:
 *
 *    - writing W-tiled stencil as Y-tiled
 *    - writing interleaved multisampled as single sampled.
 *
 * See blorp_check_in_bounds().
 */
struct brw_blorp_bounds_rect
{
   uint32_t x0;
   uint32_t x1;
   uint32_t y0;
   uint32_t y1;
};

/**
 * Grid needed for blended and scaled blits of integer formats, see
 * blorp_nir_manual_blend_bilinear().
 */
struct brw_blorp_rect_grid
{
   float x1;
   float y1;
   float pad[2];
};

struct blorp_surf_offset {
   uint32_t x;
   uint32_t y;
};

struct brw_blorp_wm_inputs
{
   uint32_t clear_color[4];

   struct brw_blorp_bounds_rect bounds_rect;
   struct brw_blorp_rect_grid rect_grid;
   struct brw_blorp_coord_transform coord_transform[2];

   struct blorp_surf_offset src_offset;
   struct blorp_surf_offset dst_offset;

   /* (1/width, 1/height) for the source surface */
   float src_inv_size[2];

   /* Minimum layer setting works for all the textures types but texture_3d
    * for which the setting has no effect. Use the z-coordinate instead.
    */
   float src_z;

   /* Note: Pad out to an integral number of registers when extending, but
    * make sure subgroup_id is the last 32-bit item.
    */
   /* uint32_t pad[?]; */
   uint32_t subgroup_id;
};

static inline nir_variable *
blorp_create_nir_input(struct nir_shader *nir,
                       const char *name,
                       const struct glsl_type *type,
                       unsigned int offset)
{
   nir_variable *input;
   if (nir->info.stage == MESA_SHADER_COMPUTE) {
      input = nir_variable_create(nir, nir_var_uniform, type, name);
      input->data.driver_location = offset;
      input->data.location = offset;
   } else {
      input = nir_variable_create(nir, nir_var_shader_in, type, name);
      input->data.location = VARYING_SLOT_VAR0 + offset / (4 * sizeof(float));
      input->data.location_frac = (offset / sizeof(float)) % 4;
   }
   if (nir->info.stage == MESA_SHADER_FRAGMENT)
      input->data.interpolation = INTERP_MODE_FLAT;
   return input;
}

#define BLORP_CREATE_NIR_INPUT(shader, name, type) \
   blorp_create_nir_input((shader), #name, (type), \
                          offsetof(struct brw_blorp_wm_inputs, name))

struct blorp_vs_inputs {
   uint32_t base_layer;
   uint32_t _instance_id; /* Set in hardware by SGVS */
   uint32_t pad[2];
};

static inline unsigned
brw_blorp_get_urb_length(const struct brw_wm_prog_data *prog_data)
{
   if (prog_data == NULL)
      return 1;

   /* From the BSpec: 3D Pipeline - Strips and Fans - 3DSTATE_SBE
    *
    * read_length = ceiling((max_source_attr+1)/2)
    */
   return MAX2((prog_data->num_varying_inputs + 1) / 2, 1);
}

enum blorp_shader_type {
   BLORP_SHADER_TYPE_COPY,
   BLORP_SHADER_TYPE_BLIT,
   BLORP_SHADER_TYPE_CLEAR,
   BLORP_SHADER_TYPE_MCS_PARTIAL_RESOLVE,
   BLORP_SHADER_TYPE_LAYER_OFFSET_VS,
   BLORP_SHADER_TYPE_GFX4_SF,
};

enum blorp_shader_pipeline {
   BLORP_SHADER_PIPELINE_RENDER,
   BLORP_SHADER_PIPELINE_COMPUTE,
};

struct blorp_params
{
   enum blorp_op op;
   uint32_t x0;
   uint32_t y0;
   uint32_t x1;
   uint32_t y1;
   float z;
   uint8_t stencil_mask;
   uint8_t stencil_ref;
   struct brw_blorp_surface_info depth;
   struct brw_blorp_surface_info stencil;
   uint32_t depth_format;
   struct brw_blorp_surface_info src;
   struct brw_blorp_surface_info dst;
   enum isl_aux_op hiz_op;
   bool full_surface_hiz_op;
   enum isl_aux_op fast_clear_op;
   uint8_t color_write_disable;
   struct brw_blorp_wm_inputs wm_inputs;
   struct blorp_vs_inputs vs_inputs;
   bool dst_clear_color_as_input;
   unsigned num_samples;
   unsigned num_draw_buffers;
   unsigned num_layers;
   uint32_t vs_prog_kernel;
   struct brw_vs_prog_data *vs_prog_data;
   uint32_t sf_prog_kernel;
   struct brw_sf_prog_data *sf_prog_data;
   uint32_t wm_prog_kernel;
   struct brw_wm_prog_data *wm_prog_data;
   uint32_t cs_prog_kernel;
   struct brw_cs_prog_data *cs_prog_data;

   bool use_pre_baked_binding_table;
   uint32_t pre_baked_binding_table_offset;
   enum blorp_shader_type shader_type;
   enum blorp_shader_pipeline shader_pipeline;
};

enum intel_measure_snapshot_type
blorp_op_to_intel_measure_snapshot(enum blorp_op op);

const char *blorp_op_to_name(enum blorp_op op);

void blorp_params_init(struct blorp_params *params);

struct brw_blorp_base_key
{
   char name[8];
   enum blorp_shader_type shader_type;
   enum blorp_shader_pipeline shader_pipeline;
};

#define BRW_BLORP_BASE_KEY_INIT(_type)                  \
   (struct brw_blorp_base_key) {                        \
      .name = "blorp",                                  \
      .shader_type = _type,                             \
      .shader_pipeline = BLORP_SHADER_PIPELINE_RENDER,  \
   }

struct brw_blorp_blit_prog_key
{
   struct brw_blorp_base_key base;

   /* Number of samples per pixel that have been configured in the surface
    * state for texturing from.
    */
   unsigned tex_samples;

   /* MSAA layout that has been configured in the surface state for texturing
    * from.
    */
   enum isl_msaa_layout tex_layout;

   enum isl_aux_usage tex_aux_usage;

   /* Actual number of samples per pixel in the source image. */
   unsigned src_samples;

   /* Actual MSAA layout used by the source image. */
   enum isl_msaa_layout src_layout;

   /* The swizzle to apply to the source in the shader */
   struct isl_swizzle src_swizzle;

   /* The format of the source if format-specific workarounds are needed
    * and 0 (ISL_FORMAT_R32G32B32A32_FLOAT) if the destination is natively
    * renderable.
    */
   enum isl_format src_format;

   /* True if the source requires normalized coordinates */
   bool src_coords_normalized;

   /* Number of samples per pixel that have been configured in the render
    * target.
    */
   unsigned rt_samples;

   /* MSAA layout that has been configured in the render target. */
   enum isl_msaa_layout rt_layout;

   /* Actual number of samples per pixel in the destination image. */
   unsigned dst_samples;

   /* Actual MSAA layout used by the destination image. */
   enum isl_msaa_layout dst_layout;

   /* The swizzle to apply to the destination in the shader */
   struct isl_swizzle dst_swizzle;

   /* The format of the destination if format-specific workarounds are needed
    * and 0 (ISL_FORMAT_R32G32B32A32_FLOAT) if the destination is natively
    * renderable.
    */
   enum isl_format dst_format;

   /* Whether or not the format workarounds are a bitcast operation */
   bool format_bit_cast;

   /** True if we need to perform SINT -> UINT clamping. */
   bool sint32_to_uint;

   /** True if we need to perform UINT -> SINT clamping. */
   bool uint32_to_sint;

   /* Type of the data to be read from the texture (one of
    * nir_type_(int|uint|float)).
    */
   nir_alu_type texture_data_type;

   /* True if the source image is W tiled.  If true, the surface state for the
    * source image must be configured as Y tiled, and tex_samples must be 0.
    */
   bool src_tiled_w;

   /* True if the destination image is W tiled.  If true, the surface state
    * for the render target must be configured as Y tiled, and rt_samples must
    * be 0.
    */
   bool dst_tiled_w;

   /* True if the destination is an RGB format.  If true, the surface state
    * for the render target must be configured as red with three times the
    * normal width.  We need to do this because you cannot render to
    * non-power-of-two formats.
    */
   bool dst_rgb;

   isl_surf_usage_flags_t dst_usage;

   enum blorp_filter filter;

   /* True if the rectangle being sent through the rendering pipeline might be
    * larger than the destination rectangle, so the WM program should kill any
    * pixels that are outside the destination rectangle.
    */
   bool use_kill;

   /**
    * True if the WM program should be run in MSDISPMODE_PERSAMPLE with more
    * than one sample per pixel.
    */
   bool persample_msaa_dispatch;

   /* True if this blit operation may involve intratile offsets on the source.
    * In this case, we need to add the offset before texturing.
    */
   bool need_src_offset;

   /* True if this blit operation may involve intratile offsets on the
    * destination.  In this case, we need to add the offset to gl_FragCoord.
    */
   bool need_dst_offset;

   /* Scale factors between the pixel grid and the grid of samples. We're
    * using grid of samples for bilinear filetring in multisample scaled blits.
    */
   float x_scale;
   float y_scale;

   /* If a compute shader is used, this is the local size y dimension.
    */
   uint8_t local_y;
};

/**
 * \name BLORP internals
 * \{
 *
 * Used internally by gfx6_blorp_exec() and gfx7_blorp_exec().
 */

bool blorp_blitter_supports_aux(const struct intel_device_info *devinfo,
                                enum isl_aux_usage aux_usage);

void brw_blorp_init_wm_prog_key(struct brw_wm_prog_key *wm_key);
void brw_blorp_init_cs_prog_key(struct brw_cs_prog_key *cs_key);

const char *blorp_shader_type_to_name(enum blorp_shader_type type);
const char *blorp_shader_pipeline_to_name(enum blorp_shader_pipeline pipe);

const unsigned *
blorp_compile_fs(struct blorp_context *blorp, void *mem_ctx,
                 struct nir_shader *nir,
                 struct brw_wm_prog_key *wm_key,
                 bool use_repclear,
                 struct brw_wm_prog_data *wm_prog_data);

const unsigned *
blorp_compile_vs(struct blorp_context *blorp, void *mem_ctx,
                 struct nir_shader *nir,
                 struct brw_vs_prog_data *vs_prog_data);

bool
blorp_ensure_sf_program(struct blorp_batch *batch,
                        struct blorp_params *params);

static inline uint8_t
blorp_get_cs_local_y(struct blorp_params *params)
{
   uint32_t height = params->y1 - params->y0;
   uint32_t or_ys = params->y0 | params->y1;
   if (height > 32 || (or_ys & 3) == 0) {
      return 4;
   } else if ((or_ys & 1) == 0) {
      return 2;
   } else {
      return 1;
   }
}

static inline void
blorp_set_cs_dims(struct nir_shader *nir, uint8_t local_y)
{
   assert(local_y != 0 && (16 % local_y == 0));
   nir->info.workgroup_size[0] = 16 / local_y;
   nir->info.workgroup_size[1] = local_y;
   nir->info.workgroup_size[2] = 1;
}

const unsigned *
blorp_compile_cs(struct blorp_context *blorp, void *mem_ctx,
                 struct nir_shader *nir,
                 struct brw_cs_prog_key *cs_key,
                 struct brw_cs_prog_data *cs_prog_data);

/** \} */

#ifdef __cplusplus
} /* end extern "C" */
#endif /* __cplusplus */

#endif /* BLORP_PRIV_H */
