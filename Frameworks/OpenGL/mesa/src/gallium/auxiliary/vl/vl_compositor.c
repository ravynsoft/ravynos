/**************************************************************************
 *
 * Copyright 2009 Younes Manton.
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

#include "util/u_sampler.h"

#include "vl_compositor_gfx.h"
#include "vl_compositor_cs.h"

static bool
init_shaders(struct vl_compositor *c)
{
   assert(c);

   if (c->pipe_cs_composit_supported) {
      if (!vl_compositor_cs_init_shaders(c))
         return false;

   } else if (c->pipe_gfx_supported) {
      c->fs_video_buffer = create_frag_shader_video_buffer(c);
      if (!c->fs_video_buffer) {
         debug_printf("Unable to create YCbCr-to-RGB fragment shader.\n");
         return false;
      }

      c->fs_weave_rgb = create_frag_shader_weave_rgb(c);
      if (!c->fs_weave_rgb) {
         debug_printf("Unable to create YCbCr-to-RGB weave fragment shader.\n");
         return false;
      }

      c->fs_yuv.weave.y = create_frag_shader_deint_yuv(c, true, true);
      c->fs_yuv.weave.uv = create_frag_shader_deint_yuv(c, false, true);
      c->fs_yuv.bob.y = create_frag_shader_deint_yuv(c, true, false);
      c->fs_yuv.bob.uv = create_frag_shader_deint_yuv(c, false, false);
      if (!c->fs_yuv.weave.y || !c->fs_yuv.weave.uv ||
          !c->fs_yuv.bob.y || !c->fs_yuv.bob.uv) {
         debug_printf("Unable to create YCbCr i-to-YCbCr p deint fragment shader.\n");
         return false;
      }

      c->fs_rgb_yuv.y = create_frag_shader_rgb_yuv(c, true);
      c->fs_rgb_yuv.uv = create_frag_shader_rgb_yuv(c, false);
      if (!c->fs_rgb_yuv.y || !c->fs_rgb_yuv.uv) {
         debug_printf("Unable to create RGB-to-YUV fragment shader.\n");
         return false;
      }
   }

   if (c->pipe_gfx_supported) {
      c->vs = create_vert_shader(c);
      if (!c->vs) {
         debug_printf("Unable to create vertex shader.\n");
         return false;
      }

      c->fs_palette.yuv = create_frag_shader_palette(c, true);
      if (!c->fs_palette.yuv) {
         debug_printf("Unable to create YUV-Palette-to-RGB fragment shader.\n");
         return false;
      }

      c->fs_palette.rgb = create_frag_shader_palette(c, false);
      if (!c->fs_palette.rgb) {
         debug_printf("Unable to create RGB-Palette-to-RGB fragment shader.\n");
         return false;
      }

      c->fs_rgba = create_frag_shader_rgba(c);
      if (!c->fs_rgba) {
         debug_printf("Unable to create RGB-to-RGB fragment shader.\n");
         return false;
      }
   }

   return true;
}

static void cleanup_shaders(struct vl_compositor *c)
{
   assert(c);

   if (c->pipe_cs_composit_supported) {
      vl_compositor_cs_cleanup_shaders(c);
   } else if (c->pipe_gfx_supported) {
      c->pipe->delete_fs_state(c->pipe, c->fs_video_buffer);
      c->pipe->delete_fs_state(c->pipe, c->fs_weave_rgb);
      c->pipe->delete_fs_state(c->pipe, c->fs_yuv.weave.y);
      c->pipe->delete_fs_state(c->pipe, c->fs_yuv.weave.uv);
      c->pipe->delete_fs_state(c->pipe, c->fs_yuv.bob.y);
      c->pipe->delete_fs_state(c->pipe, c->fs_yuv.bob.uv);
      c->pipe->delete_fs_state(c->pipe, c->fs_rgb_yuv.y);
      c->pipe->delete_fs_state(c->pipe, c->fs_rgb_yuv.uv);
   }

   if (c->pipe_gfx_supported) {
      c->pipe->delete_vs_state(c->pipe, c->vs);
      c->pipe->delete_fs_state(c->pipe, c->fs_palette.yuv);
      c->pipe->delete_fs_state(c->pipe, c->fs_palette.rgb);
      c->pipe->delete_fs_state(c->pipe, c->fs_rgba);
   }
}

static bool
init_pipe_state(struct vl_compositor *c)
{
   struct pipe_rasterizer_state rast;
   struct pipe_sampler_state sampler;
   struct pipe_blend_state blend;
   struct pipe_depth_stencil_alpha_state dsa;
   unsigned i;

   assert(c);

   c->fb_state.nr_cbufs = 1;
   c->fb_state.zsbuf = NULL;

   memset(&sampler, 0, sizeof(sampler));
   sampler.wrap_s = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler.wrap_t = PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   sampler.wrap_r = PIPE_TEX_WRAP_REPEAT;
   sampler.min_img_filter = PIPE_TEX_FILTER_LINEAR;
   sampler.min_mip_filter = PIPE_TEX_MIPFILTER_NONE;
   sampler.mag_img_filter = PIPE_TEX_FILTER_LINEAR;
   sampler.compare_mode = PIPE_TEX_COMPARE_NONE;
   sampler.compare_func = PIPE_FUNC_ALWAYS;

   c->sampler_linear = c->pipe->create_sampler_state(c->pipe, &sampler);

   sampler.min_img_filter = PIPE_TEX_FILTER_NEAREST;
   sampler.mag_img_filter = PIPE_TEX_FILTER_NEAREST;
   c->sampler_nearest = c->pipe->create_sampler_state(c->pipe, &sampler);

   if (c->pipe_gfx_supported) {
           memset(&blend, 0, sizeof blend);
           blend.independent_blend_enable = 0;
           blend.rt[0].blend_enable = 0;
           blend.logicop_enable = 0;
           blend.logicop_func = PIPE_LOGICOP_CLEAR;
           blend.rt[0].colormask = PIPE_MASK_RGBA;
           blend.dither = 0;
           c->blend_clear = c->pipe->create_blend_state(c->pipe, &blend);

           blend.rt[0].blend_enable = 1;
           blend.rt[0].rgb_func = PIPE_BLEND_ADD;
           blend.rt[0].rgb_src_factor = PIPE_BLENDFACTOR_SRC_ALPHA;
           blend.rt[0].rgb_dst_factor = PIPE_BLENDFACTOR_INV_SRC_ALPHA;
           blend.rt[0].alpha_func = PIPE_BLEND_ADD;
           blend.rt[0].alpha_src_factor = PIPE_BLENDFACTOR_ONE;
           blend.rt[0].alpha_dst_factor = PIPE_BLENDFACTOR_ONE;
           c->blend_add = c->pipe->create_blend_state(c->pipe, &blend);

           memset(&rast, 0, sizeof rast);
           rast.flatshade = 0;
           rast.front_ccw = 1;
           rast.cull_face = PIPE_FACE_NONE;
           rast.fill_back = PIPE_POLYGON_MODE_FILL;
           rast.fill_front = PIPE_POLYGON_MODE_FILL;
           rast.scissor = 1;
           rast.line_width = 1;
           rast.point_size_per_vertex = 1;
           rast.offset_units = 1;
           rast.offset_scale = 1;
           rast.half_pixel_center = 1;
           rast.bottom_edge_rule = 1;
           rast.depth_clip_near = 1;
           rast.depth_clip_far = 1;

           c->rast = c->pipe->create_rasterizer_state(c->pipe, &rast);

           memset(&dsa, 0, sizeof dsa);
           dsa.depth_enabled = 0;
           dsa.depth_writemask = 0;
           dsa.depth_func = PIPE_FUNC_ALWAYS;
           for (i = 0; i < 2; ++i) {
                   dsa.stencil[i].enabled = 0;
                   dsa.stencil[i].func = PIPE_FUNC_ALWAYS;
                   dsa.stencil[i].fail_op = PIPE_STENCIL_OP_KEEP;
                   dsa.stencil[i].zpass_op = PIPE_STENCIL_OP_KEEP;
                   dsa.stencil[i].zfail_op = PIPE_STENCIL_OP_KEEP;
                   dsa.stencil[i].valuemask = 0;
                   dsa.stencil[i].writemask = 0;
           }
           dsa.alpha_enabled = 0;
           dsa.alpha_func = PIPE_FUNC_ALWAYS;
           dsa.alpha_ref_value = 0;
           c->dsa = c->pipe->create_depth_stencil_alpha_state(c->pipe, &dsa);
           c->pipe->bind_depth_stencil_alpha_state(c->pipe, c->dsa);
   }

   return true;
}

static void cleanup_pipe_state(struct vl_compositor *c)
{
   assert(c);

   if (c->pipe_gfx_supported) {
           /* Asserted in softpipe_delete_fs_state() for some reason */
           c->pipe->bind_vs_state(c->pipe, NULL);
           c->pipe->bind_fs_state(c->pipe, NULL);

           c->pipe->delete_depth_stencil_alpha_state(c->pipe, c->dsa);
           c->pipe->delete_blend_state(c->pipe, c->blend_clear);
           c->pipe->delete_blend_state(c->pipe, c->blend_add);
           c->pipe->delete_rasterizer_state(c->pipe, c->rast);
   }
   c->pipe->delete_sampler_state(c->pipe, c->sampler_linear);
   c->pipe->delete_sampler_state(c->pipe, c->sampler_nearest);
}

static bool
init_buffers(struct vl_compositor *c)
{
   struct pipe_vertex_element vertex_elems[3];
   memset(vertex_elems, 0, sizeof(vertex_elems));

   assert(c);

   /*
    * Create our vertex buffer and vertex buffer elements
    */
   c->vertex_buf.buffer_offset = 0;
   c->vertex_buf.buffer.resource = NULL;
   c->vertex_buf.is_user_buffer = false;

   if (c->pipe_gfx_supported) {
           vertex_elems[0].src_offset = 0;
           vertex_elems[0].src_stride = VL_COMPOSITOR_VB_STRIDE;
           vertex_elems[0].instance_divisor = 0;
           vertex_elems[0].vertex_buffer_index = 0;
           vertex_elems[0].src_format = PIPE_FORMAT_R32G32_FLOAT;
           vertex_elems[1].src_offset = sizeof(struct vertex2f);
           vertex_elems[1].src_stride = VL_COMPOSITOR_VB_STRIDE;
           vertex_elems[1].instance_divisor = 0;
           vertex_elems[1].vertex_buffer_index = 0;
           vertex_elems[1].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
           vertex_elems[2].src_offset = sizeof(struct vertex2f) + sizeof(struct vertex4f);
           vertex_elems[1].src_stride = VL_COMPOSITOR_VB_STRIDE;
           vertex_elems[2].instance_divisor = 0;
           vertex_elems[2].vertex_buffer_index = 0;
           vertex_elems[2].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
           c->vertex_elems_state = c->pipe->create_vertex_elements_state(c->pipe, 3, vertex_elems);
   }

   return true;
}

static void
cleanup_buffers(struct vl_compositor *c)
{
   assert(c);

   if (c->pipe_gfx_supported) {
           c->pipe->delete_vertex_elements_state(c->pipe, c->vertex_elems_state);
   }
   pipe_resource_reference(&c->vertex_buf.buffer.resource, NULL);
}

static inline struct u_rect
default_rect(struct vl_compositor_layer *layer)
{
   struct pipe_resource *res = layer->sampler_views[0]->texture;
   struct u_rect rect = { 0, res->width0, 0, res->height0 * res->array_size };
   return rect;
}

static inline struct vertex2f
calc_topleft(struct vertex2f size, struct u_rect rect)
{
   struct vertex2f res = { rect.x0 / size.x, rect.y0 / size.y };
   return res;
}

static inline struct vertex2f
calc_bottomright(struct vertex2f size, struct u_rect rect)
{
   struct vertex2f res = { rect.x1 / size.x, rect.y1 / size.y };
   return res;
}

static inline void
calc_src_and_dst(struct vl_compositor_layer *layer, unsigned width, unsigned height,
                 struct u_rect src, struct u_rect dst)
{
   struct vertex2f size =  { width, height };

   layer->src.tl = calc_topleft(size, src);
   layer->src.br = calc_bottomright(size, src);
   layer->dst.tl = calc_topleft(size, dst);
   layer->dst.br = calc_bottomright(size, dst);
   layer->zw.x = 0.0f;
   layer->zw.y = size.y;
}

static void
set_yuv_layer(struct vl_compositor_state *s, struct vl_compositor *c,
              unsigned layer, struct pipe_video_buffer *buffer,
              struct u_rect *src_rect, struct u_rect *dst_rect,
              bool y, enum vl_compositor_deinterlace deinterlace)
{
   struct pipe_sampler_view **sampler_views;
   float half_a_line;
   unsigned i;

   assert(s && c && buffer);

   assert(layer < VL_COMPOSITOR_MAX_LAYERS);

   s->used_layers |= 1 << layer;
   sampler_views = buffer->get_sampler_view_components(buffer);
   for (i = 0; i < 3; ++i) {
      s->layers[layer].samplers[i] = c->sampler_linear;
      pipe_sampler_view_reference(&s->layers[layer].sampler_views[i], sampler_views[i]);
   }

   calc_src_and_dst(&s->layers[layer], buffer->width, buffer->height,
                    src_rect ? *src_rect : default_rect(&s->layers[layer]),
                    dst_rect ? *dst_rect : default_rect(&s->layers[layer]));

   half_a_line = 0.5f / s->layers[layer].zw.y;

   switch(deinterlace) {
   case VL_COMPOSITOR_BOB_TOP:
      s->layers[layer].zw.x = 0.0f;
      s->layers[layer].src.tl.y += half_a_line;
      s->layers[layer].src.br.y += half_a_line;
      if (c->pipe_gfx_supported)
          s->layers[layer].fs = (y) ? c->fs_yuv.bob.y : c->fs_yuv.bob.uv;
      if (c->pipe_cs_composit_supported)
          s->layers[layer].cs = (y) ? c->cs_yuv.progressive.y : c->cs_yuv.progressive.uv;
      break;

   case VL_COMPOSITOR_BOB_BOTTOM:
      s->layers[layer].zw.x = 1.0f;
      s->layers[layer].src.tl.y -= half_a_line;
      s->layers[layer].src.br.y -= half_a_line;
      if (c->pipe_gfx_supported)
          s->layers[layer].fs = (y) ? c->fs_yuv.bob.y : c->fs_yuv.bob.uv;
      if (c->pipe_cs_composit_supported)
          s->layers[layer].cs = (y) ? c->cs_yuv.progressive.y : c->cs_yuv.progressive.uv;
      break;

   case VL_COMPOSITOR_NONE:
      if (c->pipe_cs_composit_supported) {
          s->layers[layer].cs = (y) ? c->cs_yuv.progressive.y : c->cs_yuv.progressive.uv;
          break;
      }
      FALLTHROUGH;

   default:
      if (c->pipe_gfx_supported)
          s->layers[layer].fs = (y) ? c->fs_yuv.weave.y : c->fs_yuv.weave.uv;
      if (c->pipe_cs_composit_supported)
          s->layers[layer].cs = (y) ? c->cs_yuv.weave.y : c->cs_yuv.weave.uv;
      break;
   }
}

static void
set_rgb_to_yuv_layer(struct vl_compositor_state *s, struct vl_compositor *c,
                     unsigned layer, struct pipe_sampler_view *v,
                     struct u_rect *src_rect, struct u_rect *dst_rect, bool y)
{
   assert(s && c && v);

   assert(layer < VL_COMPOSITOR_MAX_LAYERS);

   s->used_layers |= 1 << layer;

   if (c->pipe_cs_composit_supported)
      s->layers[layer].cs = y ? c->cs_rgb_yuv.y : c->cs_rgb_yuv.uv;
   else if (c->pipe_gfx_supported)
      s->layers[layer].fs = y ? c->fs_rgb_yuv.y : c->fs_rgb_yuv.uv;

   s->layers[layer].samplers[0] = c->sampler_linear;
   s->layers[layer].samplers[1] = NULL;
   s->layers[layer].samplers[2] = NULL;

   pipe_sampler_view_reference(&s->layers[layer].sampler_views[0], v);
   pipe_sampler_view_reference(&s->layers[layer].sampler_views[1], NULL);
   pipe_sampler_view_reference(&s->layers[layer].sampler_views[2], NULL);

   calc_src_and_dst(&s->layers[layer], v->texture->width0, v->texture->height0,
                    src_rect ? *src_rect : default_rect(&s->layers[layer]),
                    dst_rect ? *dst_rect : default_rect(&s->layers[layer]));
}

void
vl_compositor_reset_dirty_area(struct u_rect *dirty)
{
   assert(dirty);

   dirty->x0 = dirty->y0 = VL_COMPOSITOR_MIN_DIRTY;
   dirty->x1 = dirty->y1 = VL_COMPOSITOR_MAX_DIRTY;
}

void
vl_compositor_set_clear_color(struct vl_compositor_state *s, union pipe_color_union *color)
{
   assert(s);
   assert(color);

   s->clear_color = *color;
}

void
vl_compositor_get_clear_color(struct vl_compositor_state *s, union pipe_color_union *color)
{
   assert(s);
   assert(color);

   *color = s->clear_color;
}

void
vl_compositor_clear_layers(struct vl_compositor_state *s)
{
   unsigned i, j;

   assert(s);
   s->used_layers = 0;
   for ( i = 0; i < VL_COMPOSITOR_MAX_LAYERS; ++i) {
      struct vertex4f v_one = { 1.0f, 1.0f, 1.0f, 1.0f };
      s->layers[i].clearing = i ? false : true;
      s->layers[i].blend = NULL;
      s->layers[i].fs = NULL;
      s->layers[i].cs = NULL;
      s->layers[i].viewport.scale[2] = 1;
      s->layers[i].viewport.translate[2] = 0;
      s->layers[i].viewport.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
      s->layers[i].viewport.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
      s->layers[i].viewport.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
      s->layers[i].viewport.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;
      s->layers[i].rotate = VL_COMPOSITOR_ROTATE_0;

      for ( j = 0; j < 3; j++)
         pipe_sampler_view_reference(&s->layers[i].sampler_views[j], NULL);
      for ( j = 0; j < 4; ++j)
         s->layers[i].colors[j] = v_one;
   }
}

void
vl_compositor_cleanup(struct vl_compositor *c)
{
   assert(c);

   cleanup_buffers(c);
   cleanup_shaders(c);
   cleanup_pipe_state(c);
}

bool
vl_compositor_set_csc_matrix(struct vl_compositor_state *s,
                             vl_csc_matrix const *matrix,
                             float luma_min, float luma_max)
{
   assert(s);

   memcpy(&s->csc_matrix, matrix, sizeof(vl_csc_matrix));
   s->luma_min = luma_min;
   s->luma_max = luma_max;

   return true;
}

void
vl_compositor_set_dst_clip(struct vl_compositor_state *s, struct u_rect *dst_clip)
{
   assert(s);

   s->scissor_valid = dst_clip != NULL;
   if (dst_clip) {
      s->scissor.minx = dst_clip->x0;
      s->scissor.miny = dst_clip->y0;
      s->scissor.maxx = dst_clip->x1;
      s->scissor.maxy = dst_clip->y1;
   }
}

void
vl_compositor_set_layer_blend(struct vl_compositor_state *s,
                              unsigned layer, void *blend,
                              bool is_clearing)
{
   assert(s && blend);

   assert(layer < VL_COMPOSITOR_MAX_LAYERS);

   s->layers[layer].clearing = is_clearing;
   s->layers[layer].blend = blend;
}

void
vl_compositor_set_layer_dst_area(struct vl_compositor_state *s,
                                 unsigned layer, struct u_rect *dst_area)
{
   assert(s);

   assert(layer < VL_COMPOSITOR_MAX_LAYERS);

   s->layers[layer].viewport_valid = dst_area != NULL;
   if (dst_area) {
      s->layers[layer].viewport.scale[0] = dst_area->x1 - dst_area->x0;
      s->layers[layer].viewport.scale[1] = dst_area->y1 - dst_area->y0;
      s->layers[layer].viewport.translate[0] = dst_area->x0;
      s->layers[layer].viewport.translate[1] = dst_area->y0;
   }
}

void
vl_compositor_set_buffer_layer(struct vl_compositor_state *s,
                               struct vl_compositor *c,
                               unsigned layer,
                               struct pipe_video_buffer *buffer,
                               struct u_rect *src_rect,
                               struct u_rect *dst_rect,
                               enum vl_compositor_deinterlace deinterlace)
{
   struct pipe_sampler_view **sampler_views;
   unsigned i;

   assert(s && c && buffer);

   assert(layer < VL_COMPOSITOR_MAX_LAYERS);

   s->used_layers |= 1 << layer;
   sampler_views = buffer->get_sampler_view_components(buffer);
   for (i = 0; i < 3; ++i) {
      s->layers[layer].samplers[i] = c->sampler_linear;
      pipe_sampler_view_reference(&s->layers[layer].sampler_views[i], sampler_views[i]);
   }

   calc_src_and_dst(&s->layers[layer], buffer->width, buffer->height,
                    src_rect ? *src_rect : default_rect(&s->layers[layer]),
                    dst_rect ? *dst_rect : default_rect(&s->layers[layer]));

   if (buffer->interlaced) {
      float half_a_line = 0.5f / s->layers[layer].zw.y;
      switch(deinterlace) {
      case VL_COMPOSITOR_NONE:
      case VL_COMPOSITOR_MOTION_ADAPTIVE:
      case VL_COMPOSITOR_WEAVE:
         if (c->pipe_cs_composit_supported)
            s->layers[layer].cs = c->cs_weave_rgb;
         else if (c->pipe_gfx_supported)
            s->layers[layer].fs = c->fs_weave_rgb;
         break;

      case VL_COMPOSITOR_BOB_TOP:
         s->layers[layer].zw.x = 0.0f;
         s->layers[layer].src.tl.y += half_a_line;
         s->layers[layer].src.br.y += half_a_line;
         if (c->pipe_cs_composit_supported)
            s->layers[layer].cs = c->cs_video_buffer;
         else if (c->pipe_gfx_supported)
            s->layers[layer].fs = c->fs_video_buffer;
         break;

      case VL_COMPOSITOR_BOB_BOTTOM:
         s->layers[layer].zw.x = 1.0f;
         s->layers[layer].src.tl.y -= half_a_line;
         s->layers[layer].src.br.y -= half_a_line;
         if (c->pipe_cs_composit_supported)
            s->layers[layer].cs = c->cs_video_buffer;
         else if (c->pipe_gfx_supported)
            s->layers[layer].fs = c->fs_video_buffer;
         break;
      }

   } else {
      if (c->pipe_cs_composit_supported)
         s->layers[layer].cs = c->cs_video_buffer;
      else if (c->pipe_gfx_supported)
         s->layers[layer].fs = c->fs_video_buffer;
   }
}

void
vl_compositor_set_palette_layer(struct vl_compositor_state *s,
                                struct vl_compositor *c,
                                unsigned layer,
                                struct pipe_sampler_view *indexes,
                                struct pipe_sampler_view *palette,
                                struct u_rect *src_rect,
                                struct u_rect *dst_rect,
                                bool include_color_conversion)
{
   assert(s && c && indexes && palette);

   assert(layer < VL_COMPOSITOR_MAX_LAYERS);

   s->used_layers |= 1 << layer;

   s->layers[layer].fs = include_color_conversion ?
      c->fs_palette.yuv : c->fs_palette.rgb;

   s->layers[layer].samplers[0] = c->sampler_linear;
   s->layers[layer].samplers[1] = c->sampler_nearest;
   s->layers[layer].samplers[2] = NULL;
   pipe_sampler_view_reference(&s->layers[layer].sampler_views[0], indexes);
   pipe_sampler_view_reference(&s->layers[layer].sampler_views[1], palette);
   pipe_sampler_view_reference(&s->layers[layer].sampler_views[2], NULL);
   calc_src_and_dst(&s->layers[layer], indexes->texture->width0, indexes->texture->height0,
                    src_rect ? *src_rect : default_rect(&s->layers[layer]),
                    dst_rect ? *dst_rect : default_rect(&s->layers[layer]));
}

void
vl_compositor_set_rgba_layer(struct vl_compositor_state *s,
                             struct vl_compositor *c,
                             unsigned layer,
                             struct pipe_sampler_view *rgba,
                             struct u_rect *src_rect,
                             struct u_rect *dst_rect,
                             struct vertex4f *colors)
{
   unsigned i;

   assert(s && c && rgba);

   assert(layer < VL_COMPOSITOR_MAX_LAYERS);

   s->used_layers |= 1 << layer;
   s->layers[layer].fs = c->fs_rgba;
   s->layers[layer].samplers[0] = c->sampler_linear;
   s->layers[layer].samplers[1] = NULL;
   s->layers[layer].samplers[2] = NULL;
   pipe_sampler_view_reference(&s->layers[layer].sampler_views[0], rgba);
   pipe_sampler_view_reference(&s->layers[layer].sampler_views[1], NULL);
   pipe_sampler_view_reference(&s->layers[layer].sampler_views[2], NULL);
   calc_src_and_dst(&s->layers[layer], rgba->texture->width0, rgba->texture->height0,
                    src_rect ? *src_rect : default_rect(&s->layers[layer]),
                    dst_rect ? *dst_rect : default_rect(&s->layers[layer]));

   if (colors)
      for (i = 0; i < 4; ++i)
         s->layers[layer].colors[i] = colors[i];
}

void
vl_compositor_set_layer_rotation(struct vl_compositor_state *s,
                                 unsigned layer,
                                 enum vl_compositor_rotation rotate)
{
   assert(s);
   assert(layer < VL_COMPOSITOR_MAX_LAYERS);
   s->layers[layer].rotate = rotate;
}

void
vl_compositor_yuv_deint_full(struct vl_compositor_state *s,
                             struct vl_compositor *c,
                             struct pipe_video_buffer *src,
                             struct pipe_video_buffer *dst,
                             struct u_rect *src_rect,
                             struct u_rect *dst_rect,
                             enum vl_compositor_deinterlace deinterlace)
{
   struct pipe_surface **dst_surfaces;

   dst_surfaces = dst->get_surfaces(dst);
   vl_compositor_clear_layers(s);

   set_yuv_layer(s, c, 0, src, src_rect, NULL, true, deinterlace);
   vl_compositor_set_layer_dst_area(s, 0, dst_rect);
   vl_compositor_render(s, c, dst_surfaces[0], NULL, false);

   if (dst_rect) {
      dst_rect->x0 /= 2;
      dst_rect->y0 /= 2;
      dst_rect->x1 /= 2;
      dst_rect->y1 /= 2;
   }

   set_yuv_layer(s, c, 0, src, src_rect, NULL, false, deinterlace);
   vl_compositor_set_layer_dst_area(s, 0, dst_rect);
   vl_compositor_render(s, c, dst_surfaces[1], NULL, false);

   s->pipe->flush(s->pipe, NULL, 0);
}

void
vl_compositor_convert_rgb_to_yuv(struct vl_compositor_state *s,
                                 struct vl_compositor *c,
                                 unsigned layer,
                                 struct pipe_resource *src_res,
                                 struct pipe_video_buffer *dst,
                                 struct u_rect *src_rect,
                                 struct u_rect *dst_rect)
{
   struct pipe_sampler_view *sv, sv_templ;
   struct pipe_surface **dst_surfaces;

   dst_surfaces = dst->get_surfaces(dst);

   memset(&sv_templ, 0, sizeof(sv_templ));
   u_sampler_view_default_template(&sv_templ, src_res, src_res->format);
   sv = s->pipe->create_sampler_view(s->pipe, src_res, &sv_templ);

   vl_compositor_clear_layers(s);

   set_rgb_to_yuv_layer(s, c, 0, sv, src_rect, NULL, true);
   vl_compositor_set_layer_dst_area(s, 0, dst_rect);
   vl_compositor_render(s, c, dst_surfaces[0], NULL, false);

   if (dst_rect) {
      dst_rect->x0 /= 2;
      dst_rect->y0 /= 2;
      dst_rect->x1 /= 2;
      dst_rect->y1 /= 2;
   }

   set_rgb_to_yuv_layer(s, c, 0, sv, src_rect, NULL, false);
   vl_compositor_set_layer_dst_area(s, 0, dst_rect);
   vl_compositor_render(s, c, dst_surfaces[1], NULL, false);
   pipe_sampler_view_reference(&sv, NULL);

   s->pipe->flush(s->pipe, NULL, 0);
}

void
vl_compositor_render(struct vl_compositor_state *s,
                     struct vl_compositor       *c,
                     struct pipe_surface        *dst_surface,
                     struct u_rect              *dirty_area,
                     bool                        clear_dirty)
{
   assert(s);

   if (s->layers->cs)
      vl_compositor_cs_render(s, c, dst_surface, dirty_area, clear_dirty);
   else if (s->layers->fs)
      vl_compositor_gfx_render(s, c, dst_surface, dirty_area, clear_dirty);
   else
      debug_warning("Hardware don't support.\n");;
}

bool
vl_compositor_init(struct vl_compositor *c, struct pipe_context *pipe)
{
   assert(c);

   memset(c, 0, sizeof(*c));

   c->pipe_cs_composit_supported = pipe->screen->get_param(pipe->screen, PIPE_CAP_PREFER_COMPUTE_FOR_MULTIMEDIA) &&
            pipe->screen->get_param(pipe->screen, PIPE_CAP_TGSI_TEX_TXF_LZ) &&
            pipe->screen->get_param(pipe->screen, PIPE_CAP_TGSI_DIV);

   c->pipe_gfx_supported = pipe->screen->get_param(pipe->screen, PIPE_CAP_GRAPHICS);
   c->pipe = pipe;

   c->deinterlace = VL_COMPOSITOR_NONE;

   if (!init_pipe_state(c)) {
      return false;
   }

   if (!init_shaders(c)) {
      cleanup_pipe_state(c);
      return false;
   }

   if (!init_buffers(c)) {
      cleanup_shaders(c);
      cleanup_pipe_state(c);
      return false;
   }

   return true;
}

bool
vl_compositor_init_state(struct vl_compositor_state *s, struct pipe_context *pipe)
{
   vl_csc_matrix csc_matrix;

   assert(s);

   memset(s, 0, sizeof(*s));

   s->pipe = pipe;

   s->clear_color.f[0] = s->clear_color.f[1] = 0.0f;
   s->clear_color.f[2] = s->clear_color.f[3] = 0.0f;

   /*
    * Create our fragment shader's constant buffer
    * Const buffer contains the color conversion matrix and bias vectors
    */
   /* XXX: Create with IMMUTABLE/STATIC... although it does change every once in a long while... */
   s->shader_params = pipe_buffer_create_const0
   (
      pipe->screen,
      PIPE_BIND_CONSTANT_BUFFER,
      PIPE_USAGE_DEFAULT,
      sizeof(csc_matrix) + 16*sizeof(float) + 2*sizeof(int)
   );

   if (!s->shader_params)
      return false;

   vl_compositor_clear_layers(s);

   vl_csc_get_matrix(VL_CSC_COLOR_STANDARD_IDENTITY, NULL, true, &csc_matrix);
   if (!vl_compositor_set_csc_matrix(s, (const vl_csc_matrix *)&csc_matrix, 1.0f, 0.0f))
      return false;

   return true;
}

void
vl_compositor_cleanup_state(struct vl_compositor_state *s)
{
   assert(s);

   vl_compositor_clear_layers(s);
   pipe_resource_reference(&s->shader_params, NULL);
}
