/**************************************************************************
 *
 * Copyright 2008-2010 VMware, Inc.
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


#include "util/compiler.h"
#include "util/u_memory.h"
#include "util/u_string.h"
#include "util/format/u_format.h"
#include "tgsi/tgsi_dump.h"

#include <inttypes.h>

#include "u_dump.h"


/*
 * Dump primitives
 */

static inline void
util_stream_writef(FILE *stream, const char *format, ...)
{
   static char buf[1024];
   unsigned len;
   va_list ap;
   va_start(ap, format);
   len = vsnprintf(buf, sizeof(buf), format, ap);
   va_end(ap);
   fwrite(buf, len, 1, stream);
}

static void
util_dump_bool(FILE *stream, int value)
{
   util_stream_writef(stream, "%c", value ? '1' : '0');
}

static void
util_dump_int(FILE *stream, long long int value)
{
   util_stream_writef(stream, "%lli", value);
}

static void
util_dump_uint(FILE *stream, long long unsigned value)
{
   util_stream_writef(stream, "%llu", value);
}

static void
util_dump_float(FILE *stream, double value)
{
   util_stream_writef(stream, "%g", value);
}

void
util_dump_ns(FILE *f, uint64_t time)
{
   uint64_t secs = time / (1000*1000*1000);
   unsigned usecs = (time % (1000*1000*1000)) / 1000;
   fprintf(f, "%"PRIu64".%06us", secs, usecs);
}

static void
util_dump_string(FILE *stream, const char *str)
{
   fputs("\"", stream);
   fputs(str, stream);
   fputs("\"", stream);
}

static void
util_dump_enum(FILE *stream, const char *value)
{
   fputs(value, stream);
}

static void
util_dump_array_begin(FILE *stream)
{
   fputs("{", stream);
}

static void
util_dump_array_end(FILE *stream)
{
   fputs("}", stream);
}

static void
util_dump_elem_begin(UNUSED FILE *stream)
{
}

static void
util_dump_elem_end(FILE *stream)
{
   fputs(", ", stream);
}

static void
util_dump_struct_begin(FILE *stream, UNUSED const char *name)
{
   fputs("{", stream);
}

static void
util_dump_struct_end(FILE *stream)
{
   fputs("}", stream);
}

static void
util_dump_member_begin(FILE *stream, const char *name)
{
   util_stream_writef(stream, "%s = ", name);
}

static void
util_dump_member_end(FILE *stream)
{
   fputs(", ", stream);
}

static void
util_dump_null(FILE *stream)
{
   fputs("NULL", stream);
}

void
util_dump_ptr(FILE *stream, const void *value)
{
   if(value)
      util_stream_writef(stream, "%p", value);
   else
      util_dump_null(stream);
}


/*
 * Code saving macros.
 */

#define util_dump_arg(_stream, _type, _arg) \
   do { \
      util_dump_arg_begin(_stream, #_arg); \
      util_dump_##_type(_stream, _arg); \
      util_dump_arg_end(_stream); \
   } while(0)

#define util_dump_ret(_stream, _type, _arg) \
   do { \
      util_dump_ret_begin(_stream); \
      util_dump_##_type(_stream, _arg); \
      util_dump_ret_end(_stream); \
   } while(0)

#define util_dump_array(_stream, _type, _obj, _size) \
   do { \
      size_t idx; \
      util_dump_array_begin(_stream); \
      for(idx = 0; idx < (_size); ++idx) { \
         util_dump_elem_begin(_stream); \
         util_dump_##_type(_stream, (_obj)[idx]); \
         util_dump_elem_end(_stream); \
      } \
      util_dump_array_end(_stream); \
   } while(0)

#define util_dump_struct_array(_stream, _type, _obj, _size) \
   do { \
      size_t idx; \
      util_dump_array_begin(_stream); \
      for(idx = 0; idx < (_size); ++idx) { \
         util_dump_elem_begin(_stream); \
         util_dump_##_type(_stream, &(_obj)[idx]); \
         util_dump_elem_end(_stream); \
      } \
      util_dump_array_end(_stream); \
   } while(0)

#define util_dump_member(_stream, _type, _obj, _member) \
   do { \
      util_dump_member_begin(_stream, #_member); \
      util_dump_##_type(_stream, (_obj)->_member); \
      util_dump_member_end(_stream); \
   } while(0)

#define util_dump_arg_array(_stream, _type, _arg, _size) \
   do { \
      util_dump_arg_begin(_stream, #_arg); \
      util_dump_array(_stream, _type, _arg, _size); \
      util_dump_arg_end(_stream); \
   } while(0)

#define util_dump_member_array(_stream, _type, _obj, _member) \
   do { \
      util_dump_member_begin(_stream, #_member); \
      util_dump_array(_stream, _type, (_obj)->_member, sizeof((_obj)->_member)/sizeof((_obj)->_member[0])); \
      util_dump_member_end(_stream); \
   } while(0)



/*
 * Wrappers for enum -> string dumpers.
 */


static void
util_dump_format(FILE *stream, enum pipe_format format)
{
   util_dump_enum(stream, util_format_name(format));
}


static void
util_dump_enum_blend_factor(FILE *stream, unsigned value)
{
   util_dump_enum(stream, util_str_blend_factor(value, true));
}

static void
util_dump_enum_blend_func(FILE *stream, unsigned value)
{
   util_dump_enum(stream, util_str_blend_func(value, true));
}

static void
util_dump_enum_func(FILE *stream, unsigned value)
{
   util_dump_enum(stream, util_str_func(value, true));
}

static void
util_dump_enum_prim_mode(FILE *stream, unsigned value)
{
   util_dump_enum(stream, util_str_prim_mode(value, true));
}

static void
util_dump_enum_tex_target(FILE *stream, unsigned value)
{
   util_dump_enum(stream, util_str_tex_target(value, true));
}

static void
util_dump_enum_tex_filter(FILE *stream, unsigned value)
{
   util_dump_enum(stream, util_str_tex_filter(value, true));
}

static void
util_dump_enum_tex_mipfilter(FILE *stream, unsigned value)
{
   util_dump_enum(stream, util_str_tex_mipfilter(value, true));
}

static void
util_dump_enum_tex_wrap(FILE *stream, unsigned value)
{
   util_dump_enum(stream, util_str_tex_wrap(value, true));
}

static void
util_dump_enum_stencil_op(FILE *stream, unsigned value)
{
   util_dump_enum(stream, util_str_stencil_op(value, true));
}


/*
 * Public functions
 */


void
util_dump_resource(FILE *stream, const struct pipe_resource *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_resource");

   util_dump_member(stream, enum_tex_target, state, target);
   util_dump_member(stream, format, state, format);

   util_dump_member(stream, uint, state, width0);
   util_dump_member(stream, uint, state, height0);
   util_dump_member(stream, uint, state, depth0);
   util_dump_member(stream, uint, state, array_size);

   util_dump_member(stream, uint, state, last_level);
   util_dump_member(stream, uint, state, nr_samples);
   util_dump_member(stream, uint, state, nr_storage_samples);
   util_dump_member(stream, uint, state, usage);
   util_dump_member(stream, uint, state, bind);
   util_dump_member(stream, uint, state, flags);

   util_dump_struct_end(stream);
}


void
util_dump_rasterizer_state(FILE *stream, const struct pipe_rasterizer_state *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_rasterizer_state");

   util_dump_member(stream, bool, state, flatshade);
   util_dump_member(stream, bool, state, light_twoside);
   util_dump_member(stream, bool, state, clamp_vertex_color);
   util_dump_member(stream, bool, state, clamp_fragment_color);
   util_dump_member(stream, uint, state, front_ccw);
   util_dump_member(stream, uint, state, cull_face);
   util_dump_member(stream, uint, state, fill_front);
   util_dump_member(stream, uint, state, fill_back);
   util_dump_member(stream, bool, state, offset_point);
   util_dump_member(stream, bool, state, offset_line);
   util_dump_member(stream, bool, state, offset_tri);
   util_dump_member(stream, bool, state, scissor);
   util_dump_member(stream, bool, state, poly_smooth);
   util_dump_member(stream, bool, state, poly_stipple_enable);
   util_dump_member(stream, bool, state, point_smooth);
   util_dump_member(stream, uint, state, sprite_coord_enable);
   util_dump_member(stream, bool, state, sprite_coord_mode);
   util_dump_member(stream, bool, state, point_quad_rasterization);
   util_dump_member(stream, bool, state, point_line_tri_clip);
   util_dump_member(stream, bool, state, point_size_per_vertex);
   util_dump_member(stream, bool, state, multisample);
   util_dump_member(stream, bool, state, line_smooth);
   util_dump_member(stream, bool, state, line_stipple_enable);
   util_dump_member(stream, uint, state, line_stipple_factor);
   util_dump_member(stream, uint, state, line_stipple_pattern);
   util_dump_member(stream, bool, state, line_last_pixel);
   util_dump_member(stream, bool, state, flatshade_first);
   util_dump_member(stream, bool, state, half_pixel_center);
   util_dump_member(stream, bool, state, bottom_edge_rule);
   util_dump_member(stream, bool, state, rasterizer_discard);
   util_dump_member(stream, bool, state, depth_clip_near);
   util_dump_member(stream, bool, state, depth_clip_far);
   util_dump_member(stream, bool, state, clip_halfz);
   util_dump_member(stream, uint, state, clip_plane_enable);

   util_dump_member(stream, float, state, line_width);
   util_dump_member(stream, float, state, point_size);
   util_dump_member(stream, float, state, offset_units);
   util_dump_member(stream, float, state, offset_scale);
   util_dump_member(stream, float, state, offset_clamp);

   util_dump_struct_end(stream);
}


void
util_dump_poly_stipple(FILE *stream, const struct pipe_poly_stipple *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_poly_stipple");

   util_dump_member_begin(stream, "stipple");
   util_dump_member_array(stream, uint, state, stipple);
   util_dump_member_end(stream);

   util_dump_struct_end(stream);
}


void
util_dump_viewport_state(FILE *stream, const struct pipe_viewport_state *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_viewport_state");

   util_dump_member_array(stream, float, state, scale);
   util_dump_member_array(stream, float, state, translate);

   util_dump_struct_end(stream);
}


void
util_dump_scissor_state(FILE *stream, const struct pipe_scissor_state *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_scissor_state");

   util_dump_member(stream, uint, state, minx);
   util_dump_member(stream, uint, state, miny);
   util_dump_member(stream, uint, state, maxx);
   util_dump_member(stream, uint, state, maxy);

   util_dump_struct_end(stream);
}


void
util_dump_clip_state(FILE *stream, const struct pipe_clip_state *state)
{
   unsigned i;

   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_clip_state");

   util_dump_member_begin(stream, "ucp");
   util_dump_array_begin(stream);
   for(i = 0; i < PIPE_MAX_CLIP_PLANES; ++i) {
      util_dump_elem_begin(stream);
      util_dump_array(stream, float, state->ucp[i], 4);
      util_dump_elem_end(stream);
   }
   util_dump_array_end(stream);
   util_dump_member_end(stream);

   util_dump_struct_end(stream);
}

void
util_dump_stream_output_info(FILE *stream,
                             const struct pipe_stream_output_info *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_stream_output_info");
   util_dump_member(stream, uint, state, num_outputs);
   util_dump_array(stream, uint, state->stride,
                   ARRAY_SIZE(state->stride));
   util_dump_array_begin(stream);
   for (unsigned i = 0; i < state->num_outputs; ++i) {
      util_dump_elem_begin(stream);
      util_dump_struct_begin(stream, ""); /* anonymous */
      util_dump_member(stream, uint, &state->output[i], register_index);
      util_dump_member(stream, uint, &state->output[i], start_component);
      util_dump_member(stream, uint, &state->output[i], num_components);
      util_dump_member(stream, uint, &state->output[i], output_buffer);
      util_dump_struct_end(stream);
      util_dump_elem_end(stream);
   }
   util_dump_array_end(stream);
   util_dump_struct_end(stream);
}

void
util_dump_shader_state(FILE *stream, const struct pipe_shader_state *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_shader_state");

   if (state->type == PIPE_SHADER_IR_TGSI) {
      util_dump_member_begin(stream, "tokens");
      fprintf(stream, "\"\n");
      tgsi_dump_to_file(state->tokens, 0, stream);
      fprintf(stream, "\"");
      util_dump_member_end(stream);
   }

   if (state->stream_output.num_outputs) {
      util_dump_member_begin(stream, "stream_output");
      util_dump_stream_output_info(stream, &state->stream_output);
      util_dump_member_end(stream);
   }

   util_dump_struct_end(stream);
}


void
util_dump_depth_stencil_alpha_state(FILE *stream, const struct pipe_depth_stencil_alpha_state *state)
{
   unsigned i;

   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_depth_stencil_alpha_state");

   util_dump_member(stream, bool, state, depth_enabled);
   if (state->depth_enabled) {
      util_dump_member(stream, bool, state, depth_writemask);
      util_dump_member(stream, enum_func, state, depth_func);
   }

   util_dump_member_begin(stream, "stencil");
   util_dump_array_begin(stream);
   for(i = 0; i < ARRAY_SIZE(state->stencil); ++i) {
      util_dump_elem_begin(stream);
      util_dump_struct_begin(stream, "pipe_stencil_state");
      util_dump_member(stream, bool, &state->stencil[i], enabled);
      if (state->stencil[i].enabled) {
         util_dump_member(stream, enum_func, &state->stencil[i], func);
         util_dump_member(stream, enum_stencil_op,
                          &state->stencil[i], fail_op);
         util_dump_member(stream, enum_stencil_op,
                          &state->stencil[i], zpass_op);
         util_dump_member(stream, enum_stencil_op,
                          &state->stencil[i], zfail_op);
         util_dump_member(stream, uint, &state->stencil[i], valuemask);
         util_dump_member(stream, uint, &state->stencil[i], writemask);
      }
      util_dump_struct_end(stream);
      util_dump_elem_end(stream);
   }
   util_dump_array_end(stream);
   util_dump_member_end(stream);

   util_dump_member(stream, bool, state, alpha_enabled);
   if (state->alpha_enabled) {
      util_dump_member(stream, enum_func, state, alpha_func);
      util_dump_member(stream, float, state, alpha_ref_value);
   }

   util_dump_struct_end(stream);
}

void
util_dump_rt_blend_state(FILE *stream, const struct pipe_rt_blend_state *state)
{
   util_dump_struct_begin(stream, "pipe_rt_blend_state");

   util_dump_member(stream, uint, state, blend_enable);
   if (state->blend_enable) {
      util_dump_member(stream, enum_blend_func, state, rgb_func);
      util_dump_member(stream, enum_blend_factor, state, rgb_src_factor);
      util_dump_member(stream, enum_blend_factor, state, rgb_dst_factor);

      util_dump_member(stream, enum_blend_func, state, alpha_func);
      util_dump_member(stream, enum_blend_factor, state, alpha_src_factor);
      util_dump_member(stream, enum_blend_factor, state, alpha_dst_factor);
   }

   util_dump_member(stream, uint, state, colormask);

   util_dump_struct_end(stream);
}

void
util_dump_blend_state(FILE *stream, const struct pipe_blend_state *state)
{
   unsigned valid_entries = 1;

   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_blend_state");

   util_dump_member(stream, bool, state, dither);
   util_dump_member(stream, bool, state, alpha_to_coverage);
   util_dump_member(stream, bool, state, alpha_to_one);
   util_dump_member(stream, uint, state, max_rt);

   util_dump_member(stream, bool, state, logicop_enable);
   if (state->logicop_enable) {
      util_dump_member(stream, enum_func, state, logicop_func);
   }
   else {
      util_dump_member(stream, bool, state, independent_blend_enable);

      util_dump_member_begin(stream, "rt");
      if (state->independent_blend_enable)
         valid_entries = state->max_rt + 1;
      util_dump_struct_array(stream, rt_blend_state, state->rt, valid_entries);
      util_dump_member_end(stream);
   }

   util_dump_struct_end(stream);
}


void
util_dump_blend_color(FILE *stream, const struct pipe_blend_color *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_blend_color");

   util_dump_member_array(stream, float, state, color);

   util_dump_struct_end(stream);
}

void
util_dump_stencil_ref(FILE *stream, const struct pipe_stencil_ref *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_stencil_ref");

   util_dump_member_array(stream, uint, state, ref_value);

   util_dump_struct_end(stream);
}

void
util_dump_framebuffer_state(FILE *stream, const struct pipe_framebuffer_state *state)
{
   util_dump_struct_begin(stream, "pipe_framebuffer_state");

   util_dump_member(stream, uint, state, width);
   util_dump_member(stream, uint, state, height);
   util_dump_member(stream, uint, state, samples);
   util_dump_member(stream, uint, state, layers);
   util_dump_member(stream, uint, state, nr_cbufs);
   util_dump_member_array(stream, ptr, state, cbufs);
   util_dump_member(stream, ptr, state, zsbuf);

   util_dump_struct_end(stream);
}


void
util_dump_sampler_state(FILE *stream, const struct pipe_sampler_state *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_sampler_state");

   util_dump_member(stream, enum_tex_wrap, state, wrap_s);
   util_dump_member(stream, enum_tex_wrap, state, wrap_t);
   util_dump_member(stream, enum_tex_wrap, state, wrap_r);
   util_dump_member(stream, enum_tex_filter, state, min_img_filter);
   util_dump_member(stream, enum_tex_mipfilter, state, min_mip_filter);
   util_dump_member(stream, enum_tex_filter, state, mag_img_filter);
   util_dump_member(stream, uint, state, compare_mode);
   util_dump_member(stream, enum_func, state, compare_func);
   util_dump_member(stream, bool, state, unnormalized_coords);
   util_dump_member(stream, uint, state, max_anisotropy);
   util_dump_member(stream, bool, state, seamless_cube_map);
   util_dump_member(stream, float, state, lod_bias);
   util_dump_member(stream, float, state, min_lod);
   util_dump_member(stream, float, state, max_lod);
   util_dump_member_array(stream, float, state, border_color.f);

   util_dump_struct_end(stream);
}


void
util_dump_surface(FILE *stream, const struct pipe_surface *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_surface");

   util_dump_member(stream, format, state, format);
   util_dump_member(stream, uint, state, width);
   util_dump_member(stream, uint, state, height);

   util_dump_member(stream, ptr, state, texture);
   util_dump_member(stream, uint, state, u.tex.level);
   util_dump_member(stream, uint, state, u.tex.first_layer);
   util_dump_member(stream, uint, state, u.tex.last_layer);

   util_dump_struct_end(stream);
}


void
util_dump_image_view(FILE *stream, const struct pipe_image_view *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_image_view");

   util_dump_member(stream, ptr, state, resource);
   util_dump_member(stream, format, state, format);

   if (state->resource->target == PIPE_BUFFER) {
      util_dump_member(stream, uint, state, u.buf.offset);
      util_dump_member(stream, uint, state, u.buf.size);
   }
   else {
      util_dump_member(stream, bool, state, u.tex.single_layer_view);
      util_dump_member(stream, uint, state, u.tex.first_layer);
      util_dump_member(stream, uint, state, u.tex.last_layer);
      util_dump_member(stream, uint, state, u.tex.level);
   }

   util_dump_struct_end(stream);
}


void
util_dump_shader_buffer(FILE *stream, const struct pipe_shader_buffer *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_shader_buffer");

   util_dump_member(stream, ptr, state, buffer);
   util_dump_member(stream, uint, state, buffer_offset);
   util_dump_member(stream, uint, state, buffer_size);

   util_dump_struct_end(stream);

}


void
util_dump_sampler_view(FILE *stream, const struct pipe_sampler_view *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_sampler_view");

   util_dump_member(stream, enum_tex_target, state, target);
   util_dump_member(stream, format, state, format);
   util_dump_member(stream, ptr, state, texture);

   if (state->target == PIPE_BUFFER) {
      util_dump_member(stream, uint, state, u.buf.offset);
      util_dump_member(stream, uint, state, u.buf.size);
   }
   else {
      util_dump_member(stream, uint, state, u.tex.first_layer);
      util_dump_member(stream, uint, state, u.tex.last_layer);
      util_dump_member(stream, uint, state, u.tex.first_level);
      util_dump_member(stream, uint, state, u.tex.last_level);
   }

   util_dump_member(stream, uint, state, swizzle_r);
   util_dump_member(stream, uint, state, swizzle_g);
   util_dump_member(stream, uint, state, swizzle_b);
   util_dump_member(stream, uint, state, swizzle_a);

   util_dump_struct_end(stream);
}


void
util_dump_transfer(FILE *stream, const struct pipe_transfer *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_transfer");

   util_dump_member(stream, ptr, state, resource);
   util_dump_member(stream, uint, state, level);
   util_dump_member(stream, transfer_usage, state, usage);
   util_dump_member_begin(stream, "box");
   util_dump_box(stream, &state->box);
   util_dump_member_end(stream);
   util_dump_member(stream, uint, state, stride);
   util_dump_member(stream, uint, state, layer_stride);

   util_dump_struct_end(stream);
}


void
util_dump_constant_buffer(FILE *stream,
                          const struct pipe_constant_buffer *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_constant_buffer");

   util_dump_member(stream, ptr, state, buffer);
   util_dump_member(stream, uint, state, buffer_offset);
   util_dump_member(stream, uint, state, buffer_size);
   util_dump_member(stream, ptr, state, user_buffer);

   util_dump_struct_end(stream);
}


void
util_dump_vertex_buffer(FILE *stream, const struct pipe_vertex_buffer *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_vertex_buffer");

   util_dump_member(stream, bool, state, is_user_buffer);
   util_dump_member(stream, uint, state, buffer_offset);
   util_dump_member(stream, ptr, state, buffer.resource);

   util_dump_struct_end(stream);
}


void
util_dump_vertex_element(FILE *stream, const struct pipe_vertex_element *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_vertex_element");

   util_dump_member(stream, uint, state, src_offset);
   util_dump_member(stream, uint, state, instance_divisor);
   util_dump_member(stream, uint, state, vertex_buffer_index);
   util_dump_member(stream, format, state, src_format);
   util_dump_member(stream, uint, state, src_stride);

   util_dump_struct_end(stream);
}


void
util_dump_stream_output_target(FILE *stream,
                               const struct pipe_stream_output_target *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_stream_output_target");

   util_dump_member(stream, ptr, state, buffer);
   util_dump_member(stream, uint, state, buffer_offset);
   util_dump_member(stream, uint, state, buffer_size);

   util_dump_struct_end(stream);
}


void
util_dump_draw_info(FILE *stream, const struct pipe_draw_info *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_draw_info");

   util_dump_member(stream, uint, state, index_size);
   util_dump_member(stream, uint, state, has_user_indices);

   util_dump_member(stream, enum_prim_mode, state, mode);

   util_dump_member(stream, uint, state, start_instance);
   util_dump_member(stream, uint, state, instance_count);

   util_dump_member(stream, uint, state, min_index);
   util_dump_member(stream, uint, state, max_index);

   util_dump_member(stream, bool, state, primitive_restart);
   if (state->primitive_restart)
      util_dump_member(stream, uint, state, restart_index);

   if (state->index_size) {
      if (state->has_user_indices)
         util_dump_member(stream, ptr, state, index.user);
      else
         util_dump_member(stream, ptr, state, index.resource);
   }
   util_dump_struct_end(stream);
}

void
util_dump_draw_start_count_bias(FILE *stream, const struct pipe_draw_start_count_bias *state)
{
   util_dump_struct_begin(stream, "pipe_draw_start_count_bias");
   util_dump_member(stream, uint, state, start);
   util_dump_member(stream, uint, state, count);
   util_dump_member(stream, int,  state, index_bias);
   util_dump_struct_end(stream);
}

void
util_dump_draw_indirect_info(FILE *stream,
                             const struct pipe_draw_indirect_info *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_draw_indirect_info");
   util_dump_member(stream, uint, state, offset);
   util_dump_member(stream, uint, state, stride);
   util_dump_member(stream, uint, state, draw_count);
   util_dump_member(stream, uint, state, indirect_draw_count_offset);
   util_dump_member(stream, ptr, state, buffer);
   util_dump_member(stream, ptr, state, indirect_draw_count);
   util_dump_member(stream, ptr, state, count_from_stream_output);
   util_dump_struct_end(stream);
}

void util_dump_grid_info(FILE *stream, const struct pipe_grid_info *state)
{
   if (!state) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_grid_info");

   util_dump_member(stream, uint, state, pc);
   util_dump_member(stream, ptr, state, input);
   util_dump_member(stream, uint, state, work_dim);

   util_dump_member_begin(stream, "block");
   util_dump_array(stream, uint, state->block, ARRAY_SIZE(state->block));
   util_dump_member_end(stream);

   util_dump_member_begin(stream, "grid");
   util_dump_array(stream, uint, state->grid, ARRAY_SIZE(state->grid));
   util_dump_member_end(stream);

   util_dump_member(stream, ptr, state, indirect);
   util_dump_member(stream, uint, state, indirect_offset);

   util_dump_struct_end(stream);
}

void util_dump_box(FILE *stream, const struct pipe_box *box)
{
   if (!box) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_box");

   util_dump_member(stream, int, box, x);
   util_dump_member(stream, int, box, y);
   util_dump_member(stream, int, box, z);
   util_dump_member(stream, int, box, width);
   util_dump_member(stream, int, box, height);
   util_dump_member(stream, int, box, depth);

   util_dump_struct_end(stream);
}

void util_dump_blit_info(FILE *stream, const struct pipe_blit_info *info)
{
   char mask[7];

   if (!info) {
      util_dump_null(stream);
      return;
   }

   util_dump_struct_begin(stream, "pipe_blit_info");

   util_dump_member_begin(stream, "dst");
   util_dump_struct_begin(stream, "dst");
   util_dump_member(stream, ptr, &info->dst, resource);
   util_dump_member(stream, uint, &info->dst, level);
   util_dump_member(stream, format, &info->dst, format);
   util_dump_member_begin(stream, "box");
   util_dump_box(stream, &info->dst.box);
   util_dump_member_end(stream);
   util_dump_struct_end(stream);
   util_dump_member_end(stream);

   util_dump_member_begin(stream, "src");
   util_dump_struct_begin(stream, "src");
   util_dump_member(stream, ptr, &info->src, resource);
   util_dump_member(stream, uint, &info->src, level);
   util_dump_member(stream, format, &info->src, format);
   util_dump_member_begin(stream, "box");
   util_dump_box(stream, &info->src.box);
   util_dump_member_end(stream);
   util_dump_struct_end(stream);
   util_dump_member_end(stream);

   mask[0] = (info->mask & PIPE_MASK_R) ? 'R' : '-';
   mask[1] = (info->mask & PIPE_MASK_G) ? 'G' : '-';
   mask[2] = (info->mask & PIPE_MASK_B) ? 'B' : '-';
   mask[3] = (info->mask & PIPE_MASK_A) ? 'A' : '-';
   mask[4] = (info->mask & PIPE_MASK_Z) ? 'Z' : '-';
   mask[5] = (info->mask & PIPE_MASK_S) ? 'S' : '-';
   mask[6] = 0;

   util_dump_member_begin(stream, "mask");
   util_dump_string(stream, mask);
   util_dump_member_end(stream);
   util_dump_member(stream, enum_tex_filter, info, filter);

   util_dump_member(stream, bool, info, scissor_enable);
   util_dump_member_begin(stream, "scissor");
   util_dump_scissor_state(stream, &info->scissor);
   util_dump_member_end(stream);

   util_dump_member(stream, bool, info, render_condition_enable);

   util_dump_struct_end(stream);
}
