/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
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

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  */


#include "pipe/p_context.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_inlines.h"
#include "util/u_helpers.h"
#include "util/u_prim.h"
#include "util/format/u_format.h"
#include "draw_context.h"
#include "draw_pipe.h"
#include "draw_prim_assembler.h"
#include "draw_vs.h"
#include "draw_gs.h"
#include "draw_tess.h"
#include "draw_mesh.h"

#if DRAW_LLVM_AVAILABLE
#include "gallivm/lp_bld_init.h"
#include "gallivm/lp_bld_limits.h"
#include "draw_llvm.h"
#endif


bool
draw_get_option_use_llvm(void)
{
#if DRAW_LLVM_AVAILABLE
   return debug_get_bool_option("DRAW_USE_LLVM", true);
#else
   return false;
#endif
}


/**
 * Create new draw module context with gallivm state for LLVM JIT.
 */
static struct draw_context *
draw_create_context(struct pipe_context *pipe, void *context,
                    bool try_llvm)
{
   struct draw_context *draw = CALLOC_STRUCT(draw_context);
   if (!draw)
      goto err_out;

#if DRAW_LLVM_AVAILABLE
   if (try_llvm && draw_get_option_use_llvm()) {
      draw->llvm = draw_llvm_create(draw, (LLVMContextRef)context);
   }
#endif

   draw->pipe = pipe;
   draw->constant_buffer_stride = (sizeof(float) * 4);

   if (!draw_init(draw))
      goto err_destroy;

   draw->ia = draw_prim_assembler_create(draw);
   if (!draw->ia)
      goto err_destroy;

   return draw;

err_destroy:
   draw_destroy(draw);
err_out:
   return NULL;
}


/**
 * Create new draw module context, with LLVM JIT.
 */
struct draw_context *
draw_create(struct pipe_context *pipe)
{
   return draw_create_context(pipe, NULL, true);
}


#if DRAW_LLVM_AVAILABLE
struct draw_context *
draw_create_with_llvm_context(struct pipe_context *pipe,
                              void *context)
{
   return draw_create_context(pipe, context, true);
}
#endif


/**
 * Create a new draw context, without LLVM JIT.
 */
struct draw_context *
draw_create_no_llvm(struct pipe_context *pipe)
{
   return draw_create_context(pipe, NULL, false);
}


bool
draw_init(struct draw_context *draw)
{
   /*
    * Note that several functions compute the clipmask of the predefined
    * formats with hardcoded formulas instead of using these. So modifications
    * here must be reflected there too.
    */

   ASSIGN_4V(draw->plane[0], -1,  0,  0, 1);
   ASSIGN_4V(draw->plane[1],  1,  0,  0, 1);
   ASSIGN_4V(draw->plane[2],  0, -1,  0, 1);
   ASSIGN_4V(draw->plane[3],  0,  1,  0, 1);
   ASSIGN_4V(draw->plane[4],  0,  0,  1, 1); /* yes these are correct */
   ASSIGN_4V(draw->plane[5],  0,  0, -1, 1); /* mesa's a bit wonky */
   draw->clip_xy = true;
   draw->clip_z = true;

   draw->pt.user.planes = (float (*) [DRAW_TOTAL_CLIP_PLANES][4]) &(draw->plane[0]);
   draw->pt.user.eltMax = ~0;

   if (!draw_pipeline_init(draw))
      return false;

   if (!draw_pt_init(draw))
      return false;

   if (!draw_vs_init(draw))
      return false;

   if (!draw_gs_init(draw))
      return false;

   draw->quads_always_flatshade_last = !draw->pipe->screen->get_param(
      draw->pipe->screen, PIPE_CAP_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION);

   draw->floating_point_depth = false;

   return true;
}


/*
 * Called whenever we're starting to draw a new instance.
 * Some internal structures don't want to have to reset internal
 * members on each invocation (because their state might have to persist
 * between multiple primitive restart rendering call) but might have to
 * for each new instance.
 * This is particularly the case for primitive id's in geometry shader.
 */
void
draw_new_instance(struct draw_context *draw)
{
   draw_geometry_shader_new_instance(draw->gs.geometry_shader);
   draw_prim_assembler_new_instance(draw->ia);
}


void
draw_destroy(struct draw_context *draw)
{
   if (!draw)
      return;

   struct pipe_context *pipe = draw->pipe;

   /* free any rasterizer CSOs that we may have created.
    */
   for (unsigned i = 0; i < 2; i++) {
      for (unsigned j = 0; j < 2; j++) {
         for (unsigned k = 0; k < 2; k++) {
            if (draw->rasterizer_no_cull[i][j][k]) {
               pipe->delete_rasterizer_state(pipe, draw->rasterizer_no_cull[i][j][k]);
            }
         }
      }
   }

   for (unsigned i = 0; i < draw->pt.nr_vertex_buffers; i++)
      pipe_vertex_buffer_unreference(&draw->pt.vertex_buffer[i]);

   /* Not so fast -- we're just borrowing this at the moment.
    *
   if (draw->render)
      draw->render->destroy(draw->render);
   */

   draw_prim_assembler_destroy(draw->ia);
   draw_pipeline_destroy(draw);
   draw_pt_destroy(draw);
   draw_vs_destroy(draw);
   draw_gs_destroy(draw);
#if DRAW_LLVM_AVAILABLE
   if (draw->llvm)
      draw_llvm_destroy(draw->llvm);
#endif

   FREE(draw);
}


void
draw_flush(struct draw_context *draw)
{
   draw_do_flush(draw, DRAW_FLUSH_BACKEND);
}


/**
 * Specify the depth stencil format for the draw pipeline. This function
 * determines the Minimum Resolvable Depth factor for polygon offset.
 * This factor potentially depends on the number of Z buffer bits,
 * the rasterization algorithm and the arithmetic performed on Z
 * values between vertex shading and rasterization.
 */
void
draw_set_zs_format(struct draw_context *draw, enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);

   draw->floating_point_depth =
      (util_get_depth_format_type(desc) == UTIL_FORMAT_TYPE_FLOAT);

   draw->mrd = util_get_depth_format_mrd(desc);
}


static bool
draw_is_vs_window_space(struct draw_context *draw)
{
   if (draw->vs.vertex_shader) {
      struct tgsi_shader_info *info = &draw->vs.vertex_shader->info;

      return info->properties[TGSI_PROPERTY_VS_WINDOW_SPACE_POSITION] != 0;
   }
   return false;
}


void
draw_update_clip_flags(struct draw_context *draw)
{
   bool window_space = draw_is_vs_window_space(draw);

   draw->clip_xy = !draw->driver.bypass_clip_xy && !window_space;
   draw->guard_band_xy = (!draw->driver.bypass_clip_xy &&
                          draw->driver.guard_band_xy);
   draw->clip_z = (!draw->driver.bypass_clip_z &&
                   draw->rasterizer && draw->rasterizer->depth_clip_near) &&
                  !window_space;
   draw->clip_user = draw->rasterizer &&
                     draw->rasterizer->clip_plane_enable != 0 &&
                     !window_space;
   draw->guard_band_points_lines_xy = draw->guard_band_xy ||
                                (draw->driver.bypass_clip_points_lines &&
                                (draw->rasterizer &&
                                 draw->rasterizer->point_line_tri_clip));
}


void
draw_update_viewport_flags(struct draw_context *draw)
{
   bool window_space = draw_is_vs_window_space(draw);

   draw->bypass_viewport = window_space || draw->identity_viewport;
}


/**
 * Register new primitive rasterization/rendering state.
 * This causes the drawing pipeline to be rebuilt.
 */
void draw_set_rasterizer_state(struct draw_context *draw,
                                const struct pipe_rasterizer_state *raster,
                                void *rast_handle)
{
   if (!draw->suspend_flushing) {
      draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);

      draw->rasterizer = raster;
      draw->rast_handle = rast_handle;
      draw_update_clip_flags(draw);
   }
}


/* With a little more work, llvmpipe will be able to turn this off and
 * do its own x/y clipping.
 *
 * Some hardware can turn off clipping altogether - in particular any
 * hardware with a TNL unit can do its own clipping, even if it is
 * relying on the draw module for some other reason.
 * Setting bypass_clip_points_lines to achieve d3d-style point clipping (the driver
 * will need to do the "vp scissoring") _requires_ the driver to implement
 * wide points / point sprites itself (points will still be clipped if rasterizer
 * point_line_tri_clip isn't set). Only relevant if bypass_clip_xy isn't set.
 */
void
draw_set_driver_clipping(struct draw_context *draw,
                         bool bypass_clip_xy,
                         bool bypass_clip_z,
                         bool guard_band_xy,
                         bool bypass_clip_points_lines)
{
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);

   draw->driver.bypass_clip_xy = bypass_clip_xy;
   draw->driver.bypass_clip_z = bypass_clip_z;
   draw->driver.guard_band_xy = guard_band_xy;
   draw->driver.bypass_clip_points_lines = bypass_clip_points_lines;
   draw_update_clip_flags(draw);
}


/**
 * Plug in the primitive rendering/rasterization stage (which is the last
 * stage in the drawing pipeline).
 * This is provided by the device driver.
 */
void
draw_set_rasterize_stage(struct draw_context *draw,
                         struct draw_stage *stage)
{
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);

   draw->pipeline.rasterize = stage;
}


/**
 * Set the draw module's clipping state.
 */
void
draw_set_clip_state(struct draw_context *draw,
                    const struct pipe_clip_state *clip)
{
   draw_do_flush(draw, DRAW_FLUSH_PARAMETER_CHANGE);

   memcpy(&draw->plane[6], clip->ucp, sizeof(clip->ucp));
}


/**
 * Set the draw module's viewport state.
 */
void
draw_set_viewport_states(struct draw_context *draw,
                         unsigned start_slot,
                         unsigned num_viewports,
                         const struct pipe_viewport_state *vps)
{
   const struct pipe_viewport_state *viewport = vps;
   draw_do_flush(draw, DRAW_FLUSH_PARAMETER_CHANGE);

   assert(start_slot < PIPE_MAX_VIEWPORTS);
   assert((start_slot + num_viewports) <= PIPE_MAX_VIEWPORTS);

   memcpy(draw->viewports + start_slot, vps,
          sizeof(struct pipe_viewport_state) * num_viewports);

   draw->identity_viewport = (num_viewports == 1) &&
      (viewport->scale[0] == 1.0f &&
       viewport->scale[1] == 1.0f &&
       viewport->scale[2] == 1.0f &&
       viewport->translate[0] == 0.0f &&
       viewport->translate[1] == 0.0f &&
       viewport->translate[2] == 0.0f);
   draw_update_viewport_flags(draw);
}


void
draw_set_vertex_buffers(struct draw_context *draw,
                        unsigned count,
                        unsigned unbind_num_trailing_slots,
                        const struct pipe_vertex_buffer *buffers)
{
   assert(count <= PIPE_MAX_ATTRIBS);

   util_set_vertex_buffers_count(draw->pt.vertex_buffer,
                                 &draw->pt.nr_vertex_buffers,
                                 buffers, count,
                                 unbind_num_trailing_slots, false);
}


void
draw_set_vertex_elements(struct draw_context *draw,
                         unsigned count,
                         const struct pipe_vertex_element *elements)
{
   assert(count <= PIPE_MAX_ATTRIBS);

   /* We could improve this by only flushing the frontend and the fetch part
    * of the middle. This would avoid recalculating the emit keys.*/
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);

   memcpy(draw->pt.vertex_element, elements, count * sizeof(elements[0]));
   draw->pt.nr_vertex_elements = count;
   for (unsigned i = 0; i < count; i++)
      draw->pt.vertex_strides[elements[i].vertex_buffer_index] = elements[i].src_stride;
}


/**
 * Tell drawing context where to find mapped vertex buffers.
 */
void
draw_set_mapped_vertex_buffer(struct draw_context *draw,
                              unsigned attr, const void *buffer,
                              size_t size)
{
   draw->pt.user.vbuffer[attr].map  = buffer;
   draw->pt.user.vbuffer[attr].size = size;
}


void
draw_set_mapped_constant_buffer(struct draw_context *draw,
                                enum pipe_shader_type shader_type,
                                unsigned slot,
                                const void *buffer,
                                unsigned size)
{
   assert(shader_type == PIPE_SHADER_VERTEX ||
                shader_type == PIPE_SHADER_GEOMETRY ||
                shader_type == PIPE_SHADER_TESS_CTRL ||
                shader_type == PIPE_SHADER_TESS_EVAL);
   assert(slot < PIPE_MAX_CONSTANT_BUFFERS);

   draw_do_flush(draw, DRAW_FLUSH_PARAMETER_CHANGE);

   draw->pt.user.constants[shader_type][slot].ptr = buffer;
   draw->pt.user.constants[shader_type][slot].size = size;
}

void
draw_set_mapped_shader_buffer(struct draw_context *draw,
                              enum pipe_shader_type shader_type,
                              unsigned slot,
                              const void *buffer,
                              unsigned size)
{
   assert(shader_type == PIPE_SHADER_VERTEX ||
                shader_type == PIPE_SHADER_GEOMETRY ||
                shader_type == PIPE_SHADER_TESS_CTRL ||
                shader_type == PIPE_SHADER_TESS_EVAL);
   assert(slot < PIPE_MAX_SHADER_BUFFERS);

   draw_do_flush(draw, DRAW_FLUSH_PARAMETER_CHANGE);

   draw->pt.user.ssbos[shader_type][slot].ptr = buffer;
   draw->pt.user.ssbos[shader_type][slot].size = size;
}


/**
 * Tells the draw module to draw points with triangles if their size
 * is greater than this threshold.
 */
void
draw_wide_point_threshold(struct draw_context *draw, float threshold)
{
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);
   draw->pipeline.wide_point_threshold = threshold;
}


/**
 * Should the draw module handle point->quad conversion for drawing sprites?
 */
void
draw_wide_point_sprites(struct draw_context *draw, bool draw_sprite)
{
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);
   draw->pipeline.wide_point_sprites = draw_sprite;
}


/**
 * Tells the draw module to draw lines with triangles if their width
 * is greater than this threshold.
 */
void
draw_wide_line_threshold(struct draw_context *draw, float threshold)
{
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);
   draw->pipeline.wide_line_threshold = roundf(threshold);
}


/**
 * Tells the draw module whether or not to implement line stipple.
 */
void
draw_enable_line_stipple(struct draw_context *draw, bool enable)
{
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);
   draw->pipeline.line_stipple = enable;
}


/**
 * Tells draw module whether to convert points to quads for sprite mode.
 */
void
draw_enable_point_sprites(struct draw_context *draw, bool enable)
{
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);
   draw->pipeline.point_sprite = enable;
}


/**
 * Allocate an extra vertex/geometry shader vertex attribute, if it doesn't
 * exist already.
 *
 * This is used by some of the optional draw module stages such
 * as wide_point which may need to allocate additional generic/texcoord
 * attributes.
 */
int
draw_alloc_extra_vertex_attrib(struct draw_context *draw,
                               enum tgsi_semantic semantic_name,
                               unsigned semantic_index)
{
   int slot = draw_find_shader_output(draw, semantic_name, semantic_index);
   if (slot >= 0) {
      return slot;
   }

   unsigned num_outputs = draw_current_shader_outputs(draw);
   unsigned n = draw->extra_shader_outputs.num;

   assert(n < ARRAY_SIZE(draw->extra_shader_outputs.semantic_name));

   draw->extra_shader_outputs.semantic_name[n] = semantic_name;
   draw->extra_shader_outputs.semantic_index[n] = semantic_index;
   draw->extra_shader_outputs.slot[n] = num_outputs + n;
   draw->extra_shader_outputs.num++;

   return draw->extra_shader_outputs.slot[n];
}


/**
 * Remove all extra vertex attributes that were allocated with
 * draw_alloc_extra_vertex_attrib().
 */
void
draw_remove_extra_vertex_attribs(struct draw_context *draw)
{
   draw->extra_shader_outputs.num = 0;
}


/**
 * If a geometry shader is present, return its info, else the vertex shader's
 * info.
 */
struct tgsi_shader_info *
draw_get_shader_info(const struct draw_context *draw)
{
   if (draw->ms.mesh_shader) {
      return &draw->ms.mesh_shader->info;
   } else if (draw->gs.geometry_shader) {
      return &draw->gs.geometry_shader->info;
   } else if (draw->tes.tess_eval_shader) {
      return &draw->tes.tess_eval_shader->info;
   } else {
      return &draw->vs.vertex_shader->info;
   }
}


/**
 * Prepare outputs slots from the draw module
 *
 * Certain parts of the draw module can emit additional
 * outputs that can be quite useful to the backends, a good
 * example of it is the process of decomposing primitives
 * into wireframes (aka. lines) which normally would lose
 * the face-side information, but using this method we can
 * inject another shader output which passes the original
 * face side information to the backend.
 */
void
draw_prepare_shader_outputs(struct draw_context *draw)
{
   draw_remove_extra_vertex_attribs(draw);
   draw_prim_assembler_prepare_outputs(draw->ia);
   draw_unfilled_prepare_outputs(draw, draw->pipeline.unfilled);
   if (draw->pipeline.aapoint)
      draw_aapoint_prepare_outputs(draw, draw->pipeline.aapoint);
   if (draw->pipeline.aaline)
      draw_aaline_prepare_outputs(draw, draw->pipeline.aaline);
}


/**
 * Ask the draw module for the location/slot of the given vertex attribute in
 * a post-transformed vertex.
 *
 * With this function, drivers that use the draw module should have no reason
 * to track the current vertex/geometry shader.
 *
 * Note that the draw module may sometimes generate vertices with extra
 * attributes (such as texcoords for AA lines).  The driver can call this
 * function to find those attributes.
 *
 * -1 is returned if the attribute is not found since this is
 * an undefined situation. Note, that zero is valid and can
 * be used by any of the attributes, because position is not
 * required to be attribute 0 or even at all present.
 */
int
draw_find_shader_output(const struct draw_context *draw,
                        enum tgsi_semantic semantic_name,
                        unsigned semantic_index)
{
   const struct tgsi_shader_info *info = draw_get_shader_info(draw);

   for (unsigned i = 0; i < info->num_outputs; i++) {
      if (info->output_semantic_name[i] == semantic_name &&
          info->output_semantic_index[i] == semantic_index)
         return i;
   }

   /* Search the extra vertex attributes */
   for (unsigned i = 0; i < draw->extra_shader_outputs.num; i++) {
      if (draw->extra_shader_outputs.semantic_name[i] == semantic_name &&
          draw->extra_shader_outputs.semantic_index[i] == semantic_index) {
         return draw->extra_shader_outputs.slot[i];
      }
   }

   return -1;
}


/**
 * Return total number of the shader outputs.  This function is similar to
 * draw_current_shader_outputs() but this function also counts any extra
 * vertex/geometry output attributes that may be filled in by some draw
 * stages (such as AA point, AA line).
 *
 * If geometry shader is present, its output will be returned,
 * if not vertex shader is used.
 */
unsigned
draw_num_shader_outputs(const struct draw_context *draw)
{
   const struct tgsi_shader_info *info = draw_get_shader_info(draw);
   return info->num_outputs + draw->extra_shader_outputs.num;
}


/**
 * Return total number of the vertex shader outputs.  This function
 * also counts any extra vertex output attributes that may
 * be filled in by some draw stages (such as AA point, AA line,
 * front face).
 */
unsigned
draw_total_vs_outputs(const struct draw_context *draw)
{
   const struct tgsi_shader_info *info = &draw->vs.vertex_shader->info;

   return info->num_outputs + draw->extra_shader_outputs.num;
}


/**
 * Return total number of the geometry shader outputs. This function
 * also counts any extra geometry output attributes that may
 * be filled in by some draw stages (such as AA point, AA line, front
 * face).
 */
unsigned
draw_total_gs_outputs(const struct draw_context *draw)
{
   if (!draw->gs.geometry_shader)
      return 0;
   const struct tgsi_shader_info *info = &draw->gs.geometry_shader->info;
   return info->num_outputs + draw->extra_shader_outputs.num;
}


/**
 * Return total number of the tess ctrl shader outputs.
 */
unsigned
draw_total_tcs_outputs(const struct draw_context *draw)
{
   if (!draw->tcs.tess_ctrl_shader)
      return 0;
   const struct tgsi_shader_info *info = &draw->tcs.tess_ctrl_shader->info;
   return info->num_outputs;
}


/**
 * Return total number of the tess eval shader outputs.
 */
unsigned
draw_total_tes_outputs(const struct draw_context *draw)
{
   if (!draw->tes.tess_eval_shader)
      return 0;
   const struct tgsi_shader_info *info = &draw->tes.tess_eval_shader->info;
   return info->num_outputs + draw->extra_shader_outputs.num;
}


/**
 * Provide TGSI sampler objects for vertex/geometry shaders that use
 * texture fetches.  This state only needs to be set once per context.
 * This might only be used by software drivers for the time being.
 */
void
draw_texture_sampler(struct draw_context *draw,
                     enum pipe_shader_type shader,
                     struct tgsi_sampler *sampler)
{
   switch (shader) {
   case PIPE_SHADER_VERTEX:
      draw->vs.tgsi.sampler = sampler;
      break;
   case PIPE_SHADER_GEOMETRY:
      draw->gs.tgsi.sampler = sampler;
      break;
   default:
      assert(0);
      break;
   }
}


/**
 * Provide TGSI image objects for vertex/geometry shaders that use
 * texture fetches.  This state only needs to be set once per context.
 * This might only be used by software drivers for the time being.
 */
void
draw_image(struct draw_context *draw,
           enum pipe_shader_type shader,
           struct tgsi_image *image)
{
   switch (shader) {
   case PIPE_SHADER_VERTEX:
      draw->vs.tgsi.image = image;
      break;
   case PIPE_SHADER_GEOMETRY:
      draw->gs.tgsi.image = image;
      break;
   default:
      assert(0);
      break;
   }
}


/**
 * Provide TGSI buffer objects for vertex/geometry shaders that use
 * load/store/atomic ops.  This state only needs to be set once per context.
 * This might only be used by software drivers for the time being.
 */
void
draw_buffer(struct draw_context *draw,
            enum pipe_shader_type shader,
            struct tgsi_buffer *buffer)
{
   switch (shader) {
   case PIPE_SHADER_VERTEX:
      draw->vs.tgsi.buffer = buffer;
      break;
   case PIPE_SHADER_GEOMETRY:
      draw->gs.tgsi.buffer = buffer;
      break;
   default:
      assert(0);
      break;
   }
}


void
draw_set_render(struct draw_context *draw,
                struct vbuf_render *render)
{
   draw->render = render;
}


/**
 * Tell the draw module where vertex indexes/elements are located, and
 * their size (in bytes).
 */
void
draw_set_indexes(struct draw_context *draw,
                 const void *elements, unsigned elem_size,
                 unsigned elem_buffer_space)
{
   assert(elem_size == 0 ||
          elem_size == 1 ||
          elem_size == 2 ||
          elem_size == 4);
   draw->pt.user.elts = elements;
   draw->pt.user.eltSizeIB = elem_size;
   if (elem_size)
      draw->pt.user.eltMax = elem_buffer_space / elem_size;
   else
      draw->pt.user.eltMax = 0;
}


/* Revamp me please:
 */
void draw_do_flush(struct draw_context *draw, unsigned flags)
{
   if (!draw->suspend_flushing) {
      assert(!draw->flushing); /* catch inadvertant recursion */

      draw->flushing = true;

      draw_pipeline_flush(draw, flags);

      draw_pt_flush(draw, flags);

      draw->flushing = false;
   }
}


/**
 * Return the number of output attributes produced by the geometry
 * shader, if present.  If no geometry shader, return the number of
 * outputs from the vertex shader.
 * \sa draw_num_shader_outputs
 */
unsigned
draw_current_shader_outputs(const struct draw_context *draw)
{
   if (draw->ms.mesh_shader)
      return draw->ms.num_ms_outputs;
   if (draw->gs.geometry_shader)
      return draw->gs.num_gs_outputs;
   if (draw->tes.tess_eval_shader)
      return draw->tes.num_tes_outputs;
   return draw->vs.num_vs_outputs;
}


/**
 * Return the index of the shader output which will contain the
 * vertex position.
 */
unsigned
draw_current_shader_position_output(const struct draw_context *draw)
{
   if (draw->ms.mesh_shader)
      return draw->ms.position_output;
   if (draw->gs.geometry_shader)
      return draw->gs.position_output;
   if (draw->tes.tess_eval_shader)
      return draw->tes.position_output;
   return draw->vs.position_output;
}


/**
 * Return the index of the shader output which will contain the
 * viewport index.
 */
unsigned
draw_current_shader_viewport_index_output(const struct draw_context *draw)
{
   if (draw->ms.mesh_shader)
      return draw->ms.mesh_shader->viewport_index_output;
   if (draw->gs.geometry_shader)
      return draw->gs.geometry_shader->viewport_index_output;
   else if (draw->tes.tess_eval_shader)
      return draw->tes.tess_eval_shader->viewport_index_output;
   return draw->vs.vertex_shader->viewport_index_output;
}


/**
 * Returns true if there's a geometry shader bound and the geometry
 * shader writes out a viewport index.
 */
bool
draw_current_shader_uses_viewport_index(const struct draw_context *draw)
{
   if (draw->ms.mesh_shader)
      return draw->ms.mesh_shader->info.writes_viewport_index;
   if (draw->gs.geometry_shader)
      return draw->gs.geometry_shader->info.writes_viewport_index;
   else if (draw->tes.tess_eval_shader)
      return draw->tes.tess_eval_shader->info.writes_viewport_index;
   return draw->vs.vertex_shader->info.writes_viewport_index;
}


/**
 * Return the index of the shader output which will contain the
 * clip vertex position.
 * Note we don't support clipvertex output in the gs. For clipping
 * to work correctly hence we return ordinary position output instead.
 */
unsigned
draw_current_shader_clipvertex_output(const struct draw_context *draw)
{
   if (draw->ms.mesh_shader)
      return draw->ms.clipvertex_output;
   if (draw->gs.geometry_shader)
      return draw->gs.clipvertex_output;
   if (draw->tes.tess_eval_shader)
      return draw->tes.clipvertex_output;
   return draw->vs.clipvertex_output;
}


unsigned
draw_current_shader_ccdistance_output(const struct draw_context *draw, int index)
{
   assert(index < PIPE_MAX_CLIP_OR_CULL_DISTANCE_ELEMENT_COUNT);
   if (draw->ms.mesh_shader)
      return draw->ms.mesh_shader->ccdistance_output[index];
   if (draw->gs.geometry_shader)
      return draw->gs.geometry_shader->ccdistance_output[index];
   if (draw->tes.tess_eval_shader)
      return draw->tes.tess_eval_shader->ccdistance_output[index];
   return draw->vs.ccdistance_output[index];
}


unsigned
draw_current_shader_num_written_clipdistances(const struct draw_context *draw)
{
   if (draw->ms.mesh_shader)
      return draw->ms.mesh_shader->info.num_written_clipdistance;
   if (draw->gs.geometry_shader)
      return draw->gs.geometry_shader->info.num_written_clipdistance;
   if (draw->tes.tess_eval_shader)
      return draw->tes.tess_eval_shader->info.num_written_clipdistance;
   return draw->vs.vertex_shader->info.num_written_clipdistance;
}

unsigned
draw_current_shader_num_written_culldistances(const struct draw_context *draw)
{
   if (draw->ms.mesh_shader)
      return draw->ms.mesh_shader->info.num_written_culldistance;
   if (draw->gs.geometry_shader)
      return draw->gs.geometry_shader->info.num_written_culldistance;
   if (draw->tes.tess_eval_shader)
      return draw->tes.tess_eval_shader->info.num_written_culldistance;
   return draw->vs.vertex_shader->info.num_written_culldistance;
}


/**
 * Return a pointer/handle for a driver/CSO rasterizer object which
 * disabled culling, stippling, unfilled tris, etc.
 * This is used by some pipeline stages (such as wide_point, aa_line
 * and aa_point) which convert points/lines into triangles.  In those
 * cases we don't want to accidentally cull the triangles.
 *
 * \param scissor  should the rasterizer state enable scissoring?
 * \param flatshade  should the rasterizer state use flat shading?
 * \return  rasterizer CSO handle
 */
void *
draw_get_rasterizer_no_cull(struct draw_context *draw,
                             const struct pipe_rasterizer_state *base_rast)
{
   if (!draw->rasterizer_no_cull[base_rast->scissor][base_rast->flatshade][base_rast->rasterizer_discard]) {
      /* create now */
      struct pipe_context *pipe = draw->pipe;
      struct pipe_rasterizer_state rast;

      memset(&rast, 0, sizeof(rast));
      rast.scissor = base_rast->scissor;
      rast.flatshade = base_rast->flatshade;
      rast.rasterizer_discard = base_rast->rasterizer_discard;
      rast.front_ccw = 1;
      rast.half_pixel_center = draw->rasterizer->half_pixel_center;
      rast.bottom_edge_rule = draw->rasterizer->bottom_edge_rule;
      rast.clip_halfz = draw->rasterizer->clip_halfz;

      draw->rasterizer_no_cull[base_rast->scissor][base_rast->flatshade][base_rast->rasterizer_discard] =
         pipe->create_rasterizer_state(pipe, &rast);
   }
   return draw->rasterizer_no_cull[base_rast->scissor][base_rast->flatshade][base_rast->rasterizer_discard];
}


void
draw_set_mapped_so_targets(struct draw_context *draw,
                           unsigned num_targets,
                           struct draw_so_target *targets[PIPE_MAX_SO_BUFFERS])
{
   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);

   for (unsigned i = 0; i < num_targets; i++)
      draw->so.targets[i] = targets[i];
   for (unsigned i = num_targets; i < PIPE_MAX_SO_BUFFERS; i++)
      draw->so.targets[i] = NULL;

   draw->so.num_targets = num_targets;
}


void
draw_set_sampler_views(struct draw_context *draw,
                       enum pipe_shader_type shader_stage,
                       struct pipe_sampler_view **views,
                       unsigned num)
{
   assert(shader_stage < DRAW_MAX_SHADER_STAGE);
   assert(num <= PIPE_MAX_SHADER_SAMPLER_VIEWS);

   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);

   for (unsigned i = 0; i < num; ++i)
      draw->sampler_views[shader_stage][i] = views[i];
   for (unsigned i = num; i < draw->num_sampler_views[shader_stage]; ++i)
      draw->sampler_views[shader_stage][i] = NULL;

   draw->num_sampler_views[shader_stage] = num;
}


void
draw_set_samplers(struct draw_context *draw,
                  enum pipe_shader_type shader_stage,
                  struct pipe_sampler_state **samplers,
                  unsigned num)
{
   assert(shader_stage < DRAW_MAX_SHADER_STAGE);
   assert(num <= PIPE_MAX_SAMPLERS);

   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);

   for (unsigned i = 0; i < num; ++i)
      draw->samplers[shader_stage][i] = samplers[i];
   for (unsigned i = num; i < PIPE_MAX_SAMPLERS; ++i)
      draw->samplers[shader_stage][i] = NULL;

   draw->num_samplers[shader_stage] = num;

#if DRAW_LLVM_AVAILABLE
   if (draw->llvm)
      draw_llvm_set_sampler_state(draw, shader_stage);
#endif
}


void
draw_set_images(struct draw_context *draw,
                enum pipe_shader_type shader_stage,
                struct pipe_image_view *views,
                unsigned num)
{
   assert(shader_stage < DRAW_MAX_SHADER_STAGE);
   assert(num <= PIPE_MAX_SHADER_IMAGES);

   draw_do_flush(draw, DRAW_FLUSH_STATE_CHANGE);

   for (unsigned i = 0; i < num; ++i)
      draw->images[shader_stage][i] = &views[i];
   for (unsigned i = num; i < draw->num_sampler_views[shader_stage]; ++i)
      draw->images[shader_stage][i] = NULL;

   draw->num_images[shader_stage] = num;
}


void
draw_set_mapped_texture(struct draw_context *draw,
                        enum pipe_shader_type shader_stage,
                        unsigned sview_idx,
                        uint32_t width, uint32_t height, uint32_t depth,
                        uint32_t first_level, uint32_t last_level,
                        uint32_t num_samples,
                        uint32_t sample_stride,
                        const void *base_ptr,
                        uint32_t row_stride[PIPE_MAX_TEXTURE_LEVELS],
                        uint32_t img_stride[PIPE_MAX_TEXTURE_LEVELS],
                        uint32_t mip_offsets[PIPE_MAX_TEXTURE_LEVELS])
{
#if DRAW_LLVM_AVAILABLE
   if (draw->llvm)
      draw_llvm_set_mapped_texture(draw,
                                   shader_stage,
                                   sview_idx,
                                   width, height, depth, first_level,
                                   last_level, num_samples, sample_stride, base_ptr,
                                   row_stride, img_stride, mip_offsets);
#endif
}


void
draw_set_mapped_image(struct draw_context *draw,
                      enum pipe_shader_type shader_stage,
                      unsigned idx,
                      uint32_t width, uint32_t height, uint32_t depth,
                      const void *base_ptr,
                      uint32_t row_stride,
                      uint32_t img_stride,
                      uint32_t num_samples,
                      uint32_t sample_stride)
{
#if DRAW_LLVM_AVAILABLE
   if (draw->llvm)
      draw_llvm_set_mapped_image(draw,
                                 shader_stage,
                                 idx,
                                 width, height, depth,
                                 base_ptr,
                                 row_stride, img_stride,
                                 num_samples, sample_stride);
#endif
}


/**
 * XXX: Results for PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS because there are two
 * different ways of setting textures, and drivers typically only support one.
 */
int
draw_get_shader_param_no_llvm(enum pipe_shader_type shader,
                              enum pipe_shader_cap param)
{
   switch(shader) {
   case PIPE_SHADER_VERTEX:
   case PIPE_SHADER_GEOMETRY:
      return tgsi_exec_get_shader_param(param);
   default:
      return 0;
   }
}


/**
 * XXX: Results for PIPE_SHADER_CAP_MAX_TEXTURE_SAMPLERS because there are two
 * different ways of setting textures, and drivers typically only support one.
 * Drivers requesting a draw context explicitly without llvm must call
 * draw_get_shader_param_no_llvm instead.
 */
int
draw_get_shader_param(enum pipe_shader_type shader, enum pipe_shader_cap param)
{
#if DRAW_LLVM_AVAILABLE
   if (draw_get_option_use_llvm()) {
      switch(shader) {
      case PIPE_SHADER_VERTEX:
      case PIPE_SHADER_GEOMETRY:
      case PIPE_SHADER_TESS_CTRL:
      case PIPE_SHADER_TESS_EVAL:
         return gallivm_get_shader_param(param);
      default:
         return 0;
      }
   }
#endif

   return draw_get_shader_param_no_llvm(shader, param);
}


/**
 * Enables or disables collection of statistics.
 *
 * Draw module is capable of generating statistics for the vertex
 * processing pipeline. Collection of that data isn't free and so
 * it's disabled by default. The users of the module can enable
 * (or disable) this functionality through this function.
 * The actual data will be emitted through the VBUF interface,
 * the 'pipeline_statistics' callback to be exact.
 */
void
draw_collect_pipeline_statistics(struct draw_context *draw,
                                 bool enable)
{
   draw->collect_statistics = enable;
}


/**
 * Enable/disable primitives generated gathering.
 */
void draw_collect_primitives_generated(struct draw_context *draw,
                                       bool enable)
{
   draw->collect_primgen = enable;
}


/**
 * Computes clipper invocation statistics.
 *
 * Figures out how many primitives would have been
 * sent to the clipper given the specified
 * prim info data.
 */
void
draw_stats_clipper_primitives(struct draw_context *draw,
                              const struct draw_prim_info *prim_info)
{
   if (draw->collect_statistics) {
      unsigned i;
      for (i = 0; i < prim_info->primitive_count; i++) {
         draw->statistics.c_invocations +=
            u_decomposed_prims_for_vertices(prim_info->prim,
                                            prim_info->primitive_lengths[i]);
      }
   }
}


/**
 * Returns true if the draw module will inject the frontface
 * info into the outputs.
 *
 * Given the specified primitive and rasterizer state
 * the function will figure out if the draw module
 * will inject the front-face information into shader
 * outputs. This is done to preserve the front-facing
 * info when decomposing primitives into wireframes.
 */
bool
draw_will_inject_frontface(const struct draw_context *draw)
{
   unsigned reduced_prim = u_reduced_prim(draw->pt.prim);
   const struct pipe_rasterizer_state *rast = draw->rasterizer;

   if (reduced_prim != MESA_PRIM_TRIANGLES) {
      return false;
   }

   return (rast &&
           (rast->fill_front != PIPE_POLYGON_MODE_FILL ||
            rast->fill_back != PIPE_POLYGON_MODE_FILL));
}


void
draw_set_tess_state(struct draw_context *draw,
                    const float default_outer_level[4],
                    const float default_inner_level[2])
{
   for (unsigned i = 0; i < 4; i++)
      draw->default_outer_tess_level[i] = default_outer_level[i];
   for (unsigned i = 0; i < 2; i++)
      draw->default_inner_tess_level[i] = default_inner_level[i];
}


void
draw_set_disk_cache_callbacks(struct draw_context *draw,
                              void *data_cookie,
                              void (*find_shader)(void *cookie,
                                                  struct lp_cached_code *cache,
                                                  unsigned char ir_sha1_cache_key[20]),
                              void (*insert_shader)(void *cookie,
                                                    struct lp_cached_code *cache,
                                                    unsigned char ir_sha1_cache_key[20]))
{
   draw->disk_cache_find_shader = find_shader;
   draw->disk_cache_insert_shader = insert_shader;
   draw->disk_cache_cookie = data_cookie;
}


void
draw_set_constant_buffer_stride(struct draw_context *draw, unsigned num_bytes)
{
   draw->constant_buffer_stride = num_bytes;
}
