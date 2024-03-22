/**************************************************************************
 *
 * Copyright 2011 Christian König
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include <assert.h>

#include "pipe/p_screen.h"
#include "pipe/p_context.h"

#include "util/u_draw.h"
#include "util/u_sampler.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"

#include "tgsi/tgsi_ureg.h"

#include "vl_defines.h"
#include "vl_types.h"

#include "vl_zscan.h"
#include "vl_vertex_buffers.h"

enum VS_OUTPUT
{
   VS_O_VPOS = 0,
   VS_O_VTEX = 0
};

static void *
create_vert_shader(struct vl_zscan *zscan)
{
   struct ureg_program *shader;
   struct ureg_src scale;
   struct ureg_src vrect, vpos, block_num;
   struct ureg_dst tmp;
   struct ureg_dst o_vpos;
   struct ureg_dst *o_vtex;
   unsigned i;

   shader = ureg_create(PIPE_SHADER_VERTEX);
   if (!shader)
      return NULL;

   o_vtex = MALLOC(zscan->num_channels * sizeof(struct ureg_dst));

   scale = ureg_imm2f(shader,
      (float)VL_BLOCK_WIDTH / zscan->buffer_width,
      (float)VL_BLOCK_HEIGHT / zscan->buffer_height);

   vrect = ureg_DECL_vs_input(shader, VS_I_RECT);
   vpos = ureg_DECL_vs_input(shader, VS_I_VPOS);
   block_num = ureg_DECL_vs_input(shader, VS_I_BLOCK_NUM);

   tmp = ureg_DECL_temporary(shader);

   o_vpos = ureg_DECL_output(shader, TGSI_SEMANTIC_POSITION, VS_O_VPOS);

   for (i = 0; i < zscan->num_channels; ++i)
      o_vtex[i] = ureg_DECL_output(shader, TGSI_SEMANTIC_GENERIC, VS_O_VTEX + i);

   /*
    * o_vpos.xy = (vpos + vrect) * scale
    * o_vpos.zw = 1.0f
    *
    * tmp.xy = InstanceID / blocks_per_line
    * tmp.x = frac(tmp.x)
    * tmp.y = floor(tmp.y)
    *
    * o_vtex.x = vrect.x / blocks_per_line + tmp.x
    * o_vtex.y = vrect.y
    * o_vtex.z = tmp.z * blocks_per_line / blocks_total
    */
   ureg_ADD(shader, ureg_writemask(tmp, TGSI_WRITEMASK_XY), vpos, vrect);
   ureg_MUL(shader, ureg_writemask(o_vpos, TGSI_WRITEMASK_XY), ureg_src(tmp), scale);
   ureg_MOV(shader, ureg_writemask(o_vpos, TGSI_WRITEMASK_ZW), ureg_imm1f(shader, 1.0f));

   ureg_MUL(shader, ureg_writemask(tmp, TGSI_WRITEMASK_XW), ureg_scalar(block_num, TGSI_SWIZZLE_X),
            ureg_imm1f(shader, 1.0f / zscan->blocks_per_line));

   ureg_FRC(shader, ureg_writemask(tmp, TGSI_WRITEMASK_Y), ureg_scalar(ureg_src(tmp), TGSI_SWIZZLE_X));
   ureg_FLR(shader, ureg_writemask(tmp, TGSI_WRITEMASK_W), ureg_src(tmp));

   for (i = 0; i < zscan->num_channels; ++i) {
      ureg_ADD(shader, ureg_writemask(tmp, TGSI_WRITEMASK_X), ureg_scalar(ureg_src(tmp), TGSI_SWIZZLE_Y),
               ureg_imm1f(shader, 1.0f / (zscan->blocks_per_line * VL_BLOCK_WIDTH)
                * ((signed)i - (signed)zscan->num_channels / 2)));

      ureg_MAD(shader, ureg_writemask(o_vtex[i], TGSI_WRITEMASK_X), vrect,
               ureg_imm1f(shader, 1.0f / zscan->blocks_per_line), ureg_src(tmp));
      ureg_MOV(shader, ureg_writemask(o_vtex[i], TGSI_WRITEMASK_Y), vrect);
      ureg_MOV(shader, ureg_writemask(o_vtex[i], TGSI_WRITEMASK_Z), vpos);
      ureg_MUL(shader, ureg_writemask(o_vtex[i], TGSI_WRITEMASK_W), ureg_src(tmp),
               ureg_imm1f(shader, (float)zscan->blocks_per_line / zscan->blocks_total));
   }

   ureg_release_temporary(shader, tmp);
   ureg_END(shader);

   FREE(o_vtex);

   return ureg_create_shader_and_destroy(shader, zscan->pipe);
}

static void *
create_frag_shader(struct vl_zscan *zscan)
{
   struct ureg_program *shader;
   struct ureg_src *vtex;

   struct ureg_src samp_src, samp_scan, samp_quant;

   struct ureg_dst *tmp;
   struct ureg_dst quant, fragment;

   unsigned i;

   shader = ureg_create(PIPE_SHADER_FRAGMENT);
   if (!shader)
      return NULL;

   vtex = MALLOC(zscan->num_channels * sizeof(struct ureg_src));
   tmp = MALLOC(zscan->num_channels * sizeof(struct ureg_dst));

   for (i = 0; i < zscan->num_channels; ++i)
      vtex[i] = ureg_DECL_fs_input(shader, TGSI_SEMANTIC_GENERIC, VS_O_VTEX + i, TGSI_INTERPOLATE_LINEAR);

   samp_src = ureg_DECL_sampler(shader, 0);
   samp_scan = ureg_DECL_sampler(shader, 1);
   samp_quant = ureg_DECL_sampler(shader, 2);

   for (i = 0; i < zscan->num_channels; ++i)
      tmp[i] = ureg_DECL_temporary(shader);
   quant = ureg_DECL_temporary(shader);

   fragment = ureg_DECL_output(shader, TGSI_SEMANTIC_COLOR, 0);

   /*
    * tmp.x = tex(vtex, 1)
    * tmp.y = vtex.z
    * fragment = tex(tmp, 0) * quant
    */
   for (i = 0; i < zscan->num_channels; ++i)
      ureg_TEX(shader, ureg_writemask(tmp[i], TGSI_WRITEMASK_X), TGSI_TEXTURE_2D, vtex[i], samp_scan);

   for (i = 0; i < zscan->num_channels; ++i)
      ureg_MOV(shader, ureg_writemask(tmp[i], TGSI_WRITEMASK_Y), ureg_scalar(vtex[i], TGSI_SWIZZLE_W));

   for (i = 0; i < zscan->num_channels; ++i) {
      ureg_TEX(shader, ureg_writemask(tmp[0], TGSI_WRITEMASK_X << i), TGSI_TEXTURE_2D, ureg_src(tmp[i]), samp_src);
      ureg_TEX(shader, ureg_writemask(quant, TGSI_WRITEMASK_X << i), TGSI_TEXTURE_3D, vtex[i], samp_quant);
   }

   ureg_MUL(shader, quant, ureg_src(quant), ureg_imm1f(shader, 16.0f));
   ureg_MUL(shader, fragment, ureg_src(tmp[0]), ureg_src(quant));

   for (i = 0; i < zscan->num_channels; ++i)
      ureg_release_temporary(shader, tmp[i]);
   ureg_END(shader);

   FREE(vtex);
   FREE(tmp);

   return ureg_create_shader_and_destroy(shader, zscan->pipe);
}

static bool
init_shaders(struct vl_zscan *zscan)
{
   assert(zscan);

   zscan->vs = create_vert_shader(zscan);
   if (!zscan->vs)
      goto error_vs;

   zscan->fs = create_frag_shader(zscan);
   if (!zscan->fs)
      goto error_fs;

   return true;

error_fs:
   zscan->pipe->delete_vs_state(zscan->pipe, zscan->vs);

error_vs:
   return false;
}

static void
cleanup_shaders(struct vl_zscan *zscan)
{
   assert(zscan);

   zscan->pipe->delete_vs_state(zscan->pipe, zscan->vs);
   zscan->pipe->delete_fs_state(zscan->pipe, zscan->fs);
}

static bool
init_state(struct vl_zscan *zscan)
{
   struct pipe_blend_state blend;
   struct pipe_rasterizer_state rs_state;
   struct pipe_sampler_state sampler;
   unsigned i;

   assert(zscan);

   memset(&rs_state, 0, sizeof(rs_state));
   rs_state.half_pixel_center = true;
   rs_state.bottom_edge_rule = true;
   rs_state.depth_clip_near = 1;
   rs_state.depth_clip_far = 1;

   zscan->rs_state = zscan->pipe->create_rasterizer_state(zscan->pipe, &rs_state);
   if (!zscan->rs_state)
      goto error_rs_state;

   memset(&blend, 0, sizeof blend);

   blend.independent_blend_enable = 0;
   blend.rt[0].blend_enable = 0;
   blend.rt[0].rgb_func = PIPE_BLEND_ADD;
   blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_ONE;
   blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_ONE;
   blend.rt[0].alpha_func = PIPE_BLEND_ADD;
   blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
   blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_ONE;
   blend.logicop_enable = 0;
   blend.logicop_func = PIPE_LOGICOP_CLEAR;
   /* Needed to allow color writes to FB, even if blending disabled */
   blend.rt[0].colormask = PIPE_MASK_RGBA;
   blend.dither = 0;
   zscan->blend = zscan->pipe->create_blend_state(zscan->pipe, &blend);
   if (!zscan->blend)
      goto error_blend;

   for (i = 0; i < 3; ++i) {
      memset(&sampler, 0, sizeof(sampler));
      sampler.wrap_s = PIPE_TEX_WRAP_REPEAT;
      sampler.wrap_t = PIPE_TEX_WRAP_REPEAT;
      sampler.wrap_r = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
      sampler.min_img_filter = PIPE_TEX_FILTER_NEAREST;
      sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
      sampler.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
      sampler.compare_mode = PIPE_TEX_COMPARE_NONE;
      sampler.compare_func = PIPE_FUNC_ALWAYS;
      zscan->samplers[i] = zscan->pipe->create_sampler_state(zscan->pipe, &sampler);
      if (!zscan->samplers[i])
         goto error_samplers;
   }

   return true;

error_samplers:
   for (i = 0; i < 2; ++i)
      if (zscan->samplers[i])
         zscan->pipe->delete_sampler_state(zscan->pipe, zscan->samplers[i]);

   zscan->pipe->delete_rasterizer_state(zscan->pipe, zscan->rs_state);

error_blend:
   zscan->pipe->delete_blend_state(zscan->pipe, zscan->blend);

error_rs_state:
   return false;
}

static void
cleanup_state(struct vl_zscan *zscan)
{
   unsigned i;

   assert(zscan);

   for (i = 0; i < 3; ++i)
      zscan->pipe->delete_sampler_state(zscan->pipe, zscan->samplers[i]);

   zscan->pipe->delete_rasterizer_state(zscan->pipe, zscan->rs_state);
   zscan->pipe->delete_blend_state(zscan->pipe, zscan->blend);
}

struct pipe_sampler_view *
vl_zscan_layout(struct pipe_context *pipe, const int layout[64], unsigned blocks_per_line)
{
   const unsigned total_size = blocks_per_line * VL_BLOCK_WIDTH * VL_BLOCK_HEIGHT;

   int patched_layout[64];

   struct pipe_resource res_tmpl, *res;
   struct pipe_sampler_view sv_tmpl, *sv;
   struct pipe_transfer *buf_transfer;
   unsigned x, y, i, pitch;
   float *f;

   struct pipe_box rect =
   {
      0, 0, 0,
      VL_BLOCK_WIDTH * blocks_per_line,
      VL_BLOCK_HEIGHT,
      1
   };

   assert(pipe && layout && blocks_per_line);

   for (i = 0; i < 64; ++i)
      patched_layout[layout[i]] = i;

   memset(&res_tmpl, 0, sizeof(res_tmpl));
   res_tmpl.target = PIPE_TEXTURE_2D;
   res_tmpl.format = PIPE_FORMAT_R32_FLOAT;
   res_tmpl.width0 = VL_BLOCK_WIDTH * blocks_per_line;
   res_tmpl.height0 = VL_BLOCK_HEIGHT;
   res_tmpl.depth0 = 1;
   res_tmpl.array_size = 1;
   res_tmpl.usage = PIPE_USAGE_IMMUTABLE;
   res_tmpl.bind = PIPE_BIND_SAMPLER_VIEW;

   res = pipe->screen->resource_create(pipe->screen, &res_tmpl);
   if (!res)
      goto error_resource;

   f = pipe->texture_map(pipe, res,
                          0, PIPE_MAP_WRITE | PIPE_MAP_DISCARD_RANGE,
                          &rect, &buf_transfer);
   if (!f)
      goto error_map;

   pitch = buf_transfer->stride / sizeof(float);

   for (i = 0; i < blocks_per_line; ++i)
      for (y = 0; y < VL_BLOCK_HEIGHT; ++y)
         for (x = 0; x < VL_BLOCK_WIDTH; ++x) {
            float addr = patched_layout[x + y * VL_BLOCK_WIDTH] +
               i * VL_BLOCK_WIDTH * VL_BLOCK_HEIGHT;

            addr /= total_size;

            f[i * VL_BLOCK_WIDTH + y * pitch + x] = addr;
         }

   pipe->texture_unmap(pipe, buf_transfer);

   memset(&sv_tmpl, 0, sizeof(sv_tmpl));
   u_sampler_view_default_template(&sv_tmpl, res, res->format);
   sv = pipe->create_sampler_view(pipe, res, &sv_tmpl);
   pipe_resource_reference(&res, NULL);
   if (!sv)
      goto error_map;

   return sv;

error_map:
   pipe_resource_reference(&res, NULL);

error_resource:
   return NULL;
}

bool
vl_zscan_init(struct vl_zscan *zscan, struct pipe_context *pipe,
              unsigned buffer_width, unsigned buffer_height,
              unsigned blocks_per_line, unsigned blocks_total,
              unsigned num_channels)
{
   assert(zscan && pipe);

   zscan->pipe = pipe;
   zscan->buffer_width = buffer_width;
   zscan->buffer_height = buffer_height;
   zscan->num_channels = num_channels;
   zscan->blocks_per_line = blocks_per_line;
   zscan->blocks_total = blocks_total;

   if(!init_shaders(zscan))
      return false;

   if(!init_state(zscan)) {
      cleanup_shaders(zscan);
      return false;
   }

   return true;
}

void
vl_zscan_cleanup(struct vl_zscan *zscan)
{
   assert(zscan);

   cleanup_shaders(zscan);
   cleanup_state(zscan);
}

bool
vl_zscan_init_buffer(struct vl_zscan *zscan, struct vl_zscan_buffer *buffer,
                     struct pipe_sampler_view *src, struct pipe_surface *dst)
{
   struct pipe_resource res_tmpl, *res;
   struct pipe_sampler_view sv_tmpl;

   assert(zscan && buffer);

   memset(buffer, 0, sizeof(struct vl_zscan_buffer));

   pipe_sampler_view_reference(&buffer->src, src);

   buffer->viewport.scale[0] = dst->width;
   buffer->viewport.scale[1] = dst->height;
   buffer->viewport.scale[2] = 1;
   buffer->viewport.translate[0] = 0;
   buffer->viewport.translate[1] = 0;
   buffer->viewport.translate[2] = 0;
   buffer->viewport.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
   buffer->viewport.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
   buffer->viewport.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
   buffer->viewport.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;

   buffer->fb_state.width = dst->width;
   buffer->fb_state.height = dst->height;
   buffer->fb_state.nr_cbufs = 1;
   pipe_surface_reference(&buffer->fb_state.cbufs[0], dst);

   memset(&res_tmpl, 0, sizeof(res_tmpl));
   res_tmpl.target = PIPE_TEXTURE_3D;
   res_tmpl.format = PIPE_FORMAT_R8_UNORM;
   res_tmpl.width0 = VL_BLOCK_WIDTH * zscan->blocks_per_line;
   res_tmpl.height0 = VL_BLOCK_HEIGHT;
   res_tmpl.depth0 = 2;
   res_tmpl.array_size = 1;
   res_tmpl.usage = PIPE_USAGE_IMMUTABLE;
   res_tmpl.bind = PIPE_BIND_SAMPLER_VIEW;

   res = zscan->pipe->screen->resource_create(zscan->pipe->screen, &res_tmpl);
   if (!res)
      return false;

   memset(&sv_tmpl, 0, sizeof(sv_tmpl));
   u_sampler_view_default_template(&sv_tmpl, res, res->format);
   sv_tmpl.swizzle_r = sv_tmpl.swizzle_g = sv_tmpl.swizzle_b = sv_tmpl.swizzle_a = TGSI_SWIZZLE_X;
   buffer->quant = zscan->pipe->create_sampler_view(zscan->pipe, res, &sv_tmpl);
   pipe_resource_reference(&res, NULL);
   if (!buffer->quant)
      return false;

   return true;
}

void
vl_zscan_cleanup_buffer(struct vl_zscan_buffer *buffer)
{
   assert(buffer);

   pipe_sampler_view_reference(&buffer->src, NULL);
   pipe_sampler_view_reference(&buffer->layout, NULL);
   pipe_sampler_view_reference(&buffer->quant, NULL);
   pipe_surface_reference(&buffer->fb_state.cbufs[0], NULL);
}

void
vl_zscan_set_layout(struct vl_zscan_buffer *buffer, struct pipe_sampler_view *layout)
{
   assert(buffer);
   assert(layout);

   pipe_sampler_view_reference(&buffer->layout, layout);
}

void
vl_zscan_upload_quant(struct vl_zscan *zscan, struct vl_zscan_buffer *buffer,
                      const uint8_t matrix[64], bool intra)
{
   struct pipe_context *pipe;
   struct pipe_transfer *buf_transfer;
   unsigned x, y, i, pitch;
   uint8_t *data;

   struct pipe_box rect =
   {
      0, 0, intra ? 1 : 0,
      VL_BLOCK_WIDTH,
      VL_BLOCK_HEIGHT,
      1
   };

   assert(buffer);
   assert(matrix);

   pipe = zscan->pipe;

   rect.width *= zscan->blocks_per_line;

   data = pipe->texture_map(pipe, buffer->quant->texture,
                             0, PIPE_MAP_WRITE |
                             PIPE_MAP_DISCARD_RANGE,
                             &rect, &buf_transfer);
   if (!data)
      return;

   pitch = buf_transfer->stride;

   for (i = 0; i < zscan->blocks_per_line; ++i)
      for (y = 0; y < VL_BLOCK_HEIGHT; ++y)
         for (x = 0; x < VL_BLOCK_WIDTH; ++x)
            data[i * VL_BLOCK_WIDTH + y * pitch + x] = matrix[x + y * VL_BLOCK_WIDTH];

   pipe->texture_unmap(pipe, buf_transfer);
}

void
vl_zscan_render(struct vl_zscan *zscan, struct vl_zscan_buffer *buffer, unsigned num_instances)
{
   assert(buffer);

   zscan->pipe->bind_rasterizer_state(zscan->pipe, zscan->rs_state);
   zscan->pipe->bind_blend_state(zscan->pipe, zscan->blend);
   zscan->pipe->bind_sampler_states(zscan->pipe, PIPE_SHADER_FRAGMENT,
                                    0, 3, zscan->samplers);
   zscan->pipe->set_framebuffer_state(zscan->pipe, &buffer->fb_state);
   zscan->pipe->set_viewport_states(zscan->pipe, 0, 1, &buffer->viewport);
   zscan->pipe->set_sampler_views(zscan->pipe, PIPE_SHADER_FRAGMENT,
                                  0, 3, 0, false, &buffer->src);
   zscan->pipe->bind_vs_state(zscan->pipe, zscan->vs);
   zscan->pipe->bind_fs_state(zscan->pipe, zscan->fs);
   util_draw_arrays_instanced(zscan->pipe, MESA_PRIM_QUADS, 0, 4, 0, num_instances);
}
