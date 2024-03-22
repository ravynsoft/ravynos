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


#include "main/accum.h"
#include "main/context.h"
#include "main/debug_output.h"
#include "main/framebuffer.h"
#include "main/glthread.h"
#include "main/shaderobj.h"
#include "main/state.h"
#include "main/version.h"
#include "main/hash.h"
#include "program/prog_cache.h"
#include "vbo/vbo.h"
#include "glapi/glapi.h"
#include "st_manager.h"
#include "st_context.h"
#include "st_debug.h"
#include "st_cb_bitmap.h"
#include "st_cb_clear.h"
#include "st_cb_drawpixels.h"
#include "st_cb_drawtex.h"
#include "st_cb_eglimage.h"
#include "st_cb_feedback.h"
#include "st_cb_flush.h"
#include "st_atom.h"
#include "st_draw.h"
#include "st_extensions.h"
#include "st_gen_mipmap.h"
#include "st_pbo.h"
#include "st_program.h"
#include "st_sampler_view.h"
#include "st_shader_cache.h"
#include "st_texcompress_compute.h"
#include "st_texture.h"
#include "st_util.h"
#include "pipe/p_context.h"
#include "util/u_cpu_detect.h"
#include "util/u_inlines.h"
#include "util/u_upload_mgr.h"
#include "util/u_vbuf.h"
#include "util/u_memory.h"
#include "util/hash_table.h"
#include "cso_cache/cso_context.h"
#include "compiler/glsl/glsl_parser_extras.h"

DEBUG_GET_ONCE_BOOL_OPTION(mesa_mvp_dp4, "MESA_MVP_DP4", false)

/* The list of state update functions. */
st_update_func_t st_update_functions[ST_NUM_ATOMS];

static void
init_atoms_once(void)
{
   STATIC_ASSERT(ARRAY_SIZE(st_update_functions) <= 64);

#define ST_STATE(FLAG, st_update) st_update_functions[FLAG##_INDEX] = st_update;
#include "st_atom_list.h"
#undef ST_STATE

   if (util_get_cpu_caps()->has_popcnt)
      st_update_functions[ST_NEW_VERTEX_ARRAYS_INDEX] = st_update_array_with_popcnt;
}

void
st_invalidate_buffers(struct st_context *st)
{
   st->ctx->NewDriverState |= ST_NEW_BLEND |
                              ST_NEW_DSA |
                              ST_NEW_FB_STATE |
                              ST_NEW_SAMPLE_STATE |
                              ST_NEW_SAMPLE_SHADING |
                              ST_NEW_FS_STATE |
                              ST_NEW_POLY_STIPPLE |
                              ST_NEW_VIEWPORT |
                              ST_NEW_RASTERIZER |
                              ST_NEW_SCISSOR |
                              ST_NEW_WINDOW_RECTANGLES;
}


static inline bool
st_vp_uses_current_values(const struct gl_context *ctx)
{
   const uint64_t inputs = ctx->VertexProgram._Current->info.inputs_read;

   return ~_mesa_get_enabled_vertex_arrays(ctx) & inputs;
}


void
st_invalidate_state(struct gl_context *ctx)
{
   GLbitfield new_state = ctx->NewState;
   struct st_context *st = st_context(ctx);

   if (new_state & _NEW_BUFFERS) {
      st_invalidate_buffers(st);
   } else {
      /* These set a subset of flags set by _NEW_BUFFERS, so we only have to
       * check them when _NEW_BUFFERS isn't set.
       */
      if (new_state & _NEW_FOG)
         ctx->NewDriverState |= ST_NEW_FS_STATE;
   }

   if (new_state & (_NEW_LIGHT_STATE |
                    _NEW_POINT))
      ctx->NewDriverState |= ST_NEW_RASTERIZER;

   if ((new_state & _NEW_LIGHT_STATE) &&
       (st->lower_flatshade || st->lower_two_sided_color))
      ctx->NewDriverState |= ST_NEW_FS_STATE;

   if (new_state & _NEW_PROJECTION &&
       st_user_clip_planes_enabled(ctx))
      ctx->NewDriverState |= ST_NEW_CLIP_STATE;

   if (new_state & _NEW_PIXEL)
      ctx->NewDriverState |= ST_NEW_PIXEL_TRANSFER;

   if (new_state & _NEW_CURRENT_ATTRIB && st_vp_uses_current_values(ctx)) {
      ctx->NewDriverState |= ST_NEW_VERTEX_ARRAYS;
      /* glColor3f -> glColor4f changes the vertex format. */
      ctx->Array.NewVertexElements = true;
   }

   /* Update the vertex shader if ctx->Light._ClampVertexColor was changed. */
   if (st->clamp_vert_color_in_shader && (new_state & _NEW_LIGHT_STATE)) {
      ctx->NewDriverState |= ST_NEW_VS_STATE;
      if (_mesa_is_desktop_gl_compat(st->ctx) && ctx->Version >= 32) {
         ctx->NewDriverState |= ST_NEW_GS_STATE | ST_NEW_TES_STATE;
      }
   }

   /* Update the vertex shader if ctx->Point was changed. */
   if (st->lower_point_size && new_state & _NEW_POINT) {
      if (ctx->GeometryProgram._Current)
         ctx->NewDriverState |= ST_NEW_GS_STATE | ST_NEW_GS_CONSTANTS;
      else if (ctx->TessEvalProgram._Current)
         ctx->NewDriverState |= ST_NEW_TES_STATE | ST_NEW_TES_CONSTANTS;
      else
         ctx->NewDriverState |= ST_NEW_VS_STATE | ST_NEW_VS_CONSTANTS;
   }

   if (new_state & _NEW_TEXTURE_OBJECT) {
      ctx->NewDriverState |= st->active_states &
                             (ST_NEW_SAMPLER_VIEWS |
                              ST_NEW_SAMPLERS |
                              ST_NEW_IMAGE_UNITS);
      if (ctx->FragmentProgram._Current) {
         struct gl_program *fp = ctx->FragmentProgram._Current;

         if (fp->ExternalSamplersUsed || fp->ati_fs ||
            (!fp->shader_program && fp->ShadowSamplers))
            ctx->NewDriverState |= ST_NEW_FS_STATE;
      }
   }
}


/*
 * In some circumstances (such as running google-chrome) the state
 * tracker may try to delete a resource view from a context different
 * than when it was created.  We don't want to do that.
 *
 * In that situation, st_texture_release_all_sampler_views() calls this
 * function to transfer the sampler view reference to this context (expected
 * to be the context which created the view.)
 */
void
st_save_zombie_sampler_view(struct st_context *st,
                            struct pipe_sampler_view *view)
{
   struct st_zombie_sampler_view_node *entry;

   assert(view->context == st->pipe);

   entry = MALLOC_STRUCT(st_zombie_sampler_view_node);
   if (!entry)
      return;

   entry->view = view;

   /* We need a mutex since this function may be called from one thread
    * while free_zombie_resource_views() is called from another.
    */
   simple_mtx_lock(&st->zombie_sampler_views.mutex);
   list_addtail(&entry->node, &st->zombie_sampler_views.list.node);
   simple_mtx_unlock(&st->zombie_sampler_views.mutex);
}


/*
 * Since OpenGL shaders may be shared among contexts, we can wind up
 * with variants of a shader created with different contexts.
 * When we go to destroy a gallium shader, we want to free it with the
 * same context that it was created with, unless the driver reports
 * PIPE_CAP_SHAREABLE_SHADERS = TRUE.
 */
void
st_save_zombie_shader(struct st_context *st,
                      enum pipe_shader_type type,
                      struct pipe_shader_state *shader)
{
   struct st_zombie_shader_node *entry;

   /* we shouldn't be here if the driver supports shareable shaders */
   assert(!st->has_shareable_shaders);

   entry = MALLOC_STRUCT(st_zombie_shader_node);
   if (!entry)
      return;

   entry->shader = shader;
   entry->type = type;

   /* We need a mutex since this function may be called from one thread
    * while free_zombie_shaders() is called from another.
    */
   simple_mtx_lock(&st->zombie_shaders.mutex);
   list_addtail(&entry->node, &st->zombie_shaders.list.node);
   simple_mtx_unlock(&st->zombie_shaders.mutex);
}


/*
 * Free any zombie sampler views that may be attached to this context.
 */
static void
free_zombie_sampler_views(struct st_context *st)
{
   struct st_zombie_sampler_view_node *entry, *next;

   if (list_is_empty(&st->zombie_sampler_views.list.node)) {
      return;
   }

   simple_mtx_lock(&st->zombie_sampler_views.mutex);

   LIST_FOR_EACH_ENTRY_SAFE(entry, next,
                            &st->zombie_sampler_views.list.node, node) {
      list_del(&entry->node);  // remove this entry from the list

      assert(entry->view->context == st->pipe);
      pipe_sampler_view_reference(&entry->view, NULL);

      free(entry);
   }

   assert(list_is_empty(&st->zombie_sampler_views.list.node));

   simple_mtx_unlock(&st->zombie_sampler_views.mutex);
}


/*
 * Free any zombie shaders that may be attached to this context.
 */
static void
free_zombie_shaders(struct st_context *st)
{
   struct st_zombie_shader_node *entry, *next;

   if (list_is_empty(&st->zombie_shaders.list.node)) {
      return;
   }

   simple_mtx_lock(&st->zombie_shaders.mutex);

   LIST_FOR_EACH_ENTRY_SAFE(entry, next,
                            &st->zombie_shaders.list.node, node) {
      list_del(&entry->node);  // remove this entry from the list

      switch (entry->type) {
      case PIPE_SHADER_VERTEX:
         st->pipe->bind_vs_state(st->pipe, NULL);
         st->pipe->delete_vs_state(st->pipe, entry->shader);
         break;
      case PIPE_SHADER_FRAGMENT:
         st->pipe->bind_fs_state(st->pipe, NULL);
         st->pipe->delete_fs_state(st->pipe, entry->shader);
         break;
      case PIPE_SHADER_GEOMETRY:
         st->pipe->bind_gs_state(st->pipe, NULL);
         st->pipe->delete_gs_state(st->pipe, entry->shader);
         break;
      case PIPE_SHADER_TESS_CTRL:
         st->pipe->bind_tcs_state(st->pipe, NULL);
         st->pipe->delete_tcs_state(st->pipe, entry->shader);
         break;
      case PIPE_SHADER_TESS_EVAL:
         st->pipe->bind_tes_state(st->pipe, NULL);
         st->pipe->delete_tes_state(st->pipe, entry->shader);
         break;
      case PIPE_SHADER_COMPUTE:
         st->pipe->bind_compute_state(st->pipe, NULL);
         st->pipe->delete_compute_state(st->pipe, entry->shader);
         break;
      default:
         unreachable("invalid shader type in free_zombie_shaders()");
      }
      free(entry);
   }

   assert(list_is_empty(&st->zombie_shaders.list.node));

   simple_mtx_unlock(&st->zombie_shaders.mutex);
}


/*
 * This function is called periodically to free any zombie objects
 * which are attached to this context.
 */
void
st_context_free_zombie_objects(struct st_context *st)
{
   free_zombie_sampler_views(st);
   free_zombie_shaders(st);
}


static void
st_destroy_context_priv(struct st_context *st, bool destroy_pipe)
{
   st_destroy_draw(st);
   st_destroy_clear(st);
   st_destroy_bitmap(st);
   st_destroy_drawpix(st);
   st_destroy_drawtex(st);
   st_destroy_pbo_helpers(st);

   if (_mesa_has_compute_shaders(st->ctx) && st->transcode_astc)
      st_destroy_texcompress_compute(st);

   st_destroy_bound_texture_handles(st);
   st_destroy_bound_image_handles(st);

   /* free glReadPixels cache data */
   st_invalidate_readpix_cache(st);
   util_throttle_deinit(st->screen, &st->throttle);

   cso_destroy_context(st->cso_context);

   if (st->pipe && destroy_pipe)
      st->pipe->destroy(st->pipe);

   st->ctx->st = NULL;
   FREE(st);
}


static void
st_init_driver_flags(struct st_context *st)
{
   struct gl_driver_flags *f = &st->ctx->DriverFlags;

   /* Shader resources */
   if (st->has_hw_atomics)
      f->NewAtomicBuffer = ST_NEW_HW_ATOMICS | ST_NEW_CS_ATOMICS;
   else
      f->NewAtomicBuffer = ST_NEW_ATOMIC_BUFFER;

   f->NewShaderConstants[MESA_SHADER_VERTEX] = ST_NEW_VS_CONSTANTS;
   f->NewShaderConstants[MESA_SHADER_TESS_CTRL] = ST_NEW_TCS_CONSTANTS;
   f->NewShaderConstants[MESA_SHADER_TESS_EVAL] = ST_NEW_TES_CONSTANTS;
   f->NewShaderConstants[MESA_SHADER_GEOMETRY] = ST_NEW_GS_CONSTANTS;
   f->NewShaderConstants[MESA_SHADER_FRAGMENT] = ST_NEW_FS_CONSTANTS;
   f->NewShaderConstants[MESA_SHADER_COMPUTE] = ST_NEW_CS_CONSTANTS;

   if (st->lower_alpha_test)
      f->NewAlphaTest = ST_NEW_FS_STATE | ST_NEW_FS_CONSTANTS;
   else
      f->NewAlphaTest = ST_NEW_DSA;

   f->NewMultisampleEnable = ST_NEW_BLEND | ST_NEW_RASTERIZER |
                             ST_NEW_SAMPLE_STATE | ST_NEW_SAMPLE_SHADING;
   f->NewSampleShading = ST_NEW_SAMPLE_SHADING;

   /* This depends on what the gallium driver wants. */
   if (st->force_persample_in_shader) {
      f->NewMultisampleEnable |= ST_NEW_FS_STATE;
      f->NewSampleShading |= ST_NEW_FS_STATE;
   } else {
      f->NewSampleShading |= ST_NEW_RASTERIZER;
   }

   if (st->clamp_frag_color_in_shader) {
      f->NewFragClamp = ST_NEW_FS_STATE;
   } else {
      f->NewFragClamp = ST_NEW_RASTERIZER;
   }

   f->NewClipPlaneEnable = ST_NEW_RASTERIZER;
   if (st->lower_ucp)
      f->NewClipPlaneEnable |= ST_NEW_VS_STATE | ST_NEW_GS_STATE | ST_NEW_TES_STATE;

   if (st->emulate_gl_clamp)
      f->NewSamplersWithClamp = ST_NEW_SAMPLERS |
                                ST_NEW_VS_STATE | ST_NEW_TCS_STATE |
                                ST_NEW_TES_STATE | ST_NEW_GS_STATE |
                                ST_NEW_FS_STATE | ST_NEW_CS_STATE;

   if (!st->has_hw_atomics && st->ctx->Const.ShaderStorageBufferOffsetAlignment > 4)
      f->NewAtomicBuffer |= ST_NEW_CONSTANTS;
}

static bool
st_have_perfquery(struct st_context *ctx)
{
   struct pipe_context *pipe = ctx->pipe;

   return pipe->init_intel_perf_query_info && pipe->get_intel_perf_query_info &&
          pipe->get_intel_perf_query_counter_info &&
          pipe->new_intel_perf_query_obj && pipe->begin_intel_perf_query &&
          pipe->end_intel_perf_query && pipe->delete_intel_perf_query &&
          pipe->wait_intel_perf_query && pipe->is_intel_perf_query_ready &&
          pipe->get_intel_perf_query_data;
}

static struct st_context *
st_create_context_priv(struct gl_context *ctx, struct pipe_context *pipe,
                       const struct st_config_options *options)
{
   struct pipe_screen *screen = pipe->screen;
   struct st_context *st = CALLOC_STRUCT( st_context);

   st->options = *options;

   ctx->st_opts = &st->options;
   ctx->st = st;

   st->ctx = ctx;
   st->screen = screen;
   st->pipe = pipe;

   st->can_bind_const_buffer_as_vertex =
      screen->get_param(screen, PIPE_CAP_CAN_BIND_CONST_BUFFER_AS_VERTEX);

   /* st/mesa always uploads zero-stride vertex attribs, and other user
    * vertex buffers are only possible with a compatibility profile.
    * So tell the u_vbuf module that user VBOs are not possible with the Core
    * profile, so that u_vbuf is bypassed completely if there is nothing else
    * to do.
    */
   unsigned cso_flags;
   switch (ctx->API) {
   case API_OPENGL_CORE:
      cso_flags = CSO_NO_USER_VERTEX_BUFFERS;
      break;
   case API_OPENGLES:
   case API_OPENGLES2:
      cso_flags = CSO_NO_64B_VERTEX_BUFFERS;
      break;
   default:
      cso_flags = 0;
      break;
   }

   st->cso_context = cso_create_context(pipe, cso_flags);
   ctx->cso_context = st->cso_context;

   static once_flag flag = ONCE_FLAG_INIT;
   call_once(&flag, init_atoms_once);

   st_init_clear(st);
   {
      enum pipe_texture_transfer_mode val = screen->get_param(screen, PIPE_CAP_TEXTURE_TRANSFER_MODES);
      st->prefer_blit_based_texture_transfer = (val & PIPE_TEXTURE_TRANSFER_BLIT) != 0;
      st->allow_compute_based_texture_transfer = (val & PIPE_TEXTURE_TRANSFER_COMPUTE) != 0;
   }
   st_init_pbo_helpers(st);

   /* Choose texture target for glDrawPixels, glBitmap, renderbuffers */
   if (screen->get_param(screen, PIPE_CAP_NPOT_TEXTURES))
      st->internal_target = PIPE_TEXTURE_2D;
   else
      st->internal_target = PIPE_TEXTURE_RECT;

   /* Setup vertex element info for 'struct st_util_vertex'.
    */
   {
      STATIC_ASSERT(sizeof(struct st_util_vertex) == 9 * sizeof(float));

      memset(&st->util_velems, 0, sizeof(st->util_velems));
      st->util_velems.velems[0].src_offset = 0;
      st->util_velems.velems[0].vertex_buffer_index = 0;
      st->util_velems.velems[0].src_format = PIPE_FORMAT_R32G32B32_FLOAT;
      st->util_velems.velems[0].src_stride = sizeof(struct st_util_vertex);
      st->util_velems.velems[1].src_offset = 3 * sizeof(float);
      st->util_velems.velems[1].vertex_buffer_index = 0;
      st->util_velems.velems[1].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
      st->util_velems.velems[1].src_stride = sizeof(struct st_util_vertex);
      st->util_velems.velems[2].src_offset = 7 * sizeof(float);
      st->util_velems.velems[2].vertex_buffer_index = 0;
      st->util_velems.velems[2].src_format = PIPE_FORMAT_R32G32_FLOAT;
      st->util_velems.velems[2].src_stride = sizeof(struct st_util_vertex);
   }

   ctx->Const.PackedDriverUniformStorage =
      screen->get_param(screen, PIPE_CAP_PACKED_UNIFORMS);

   ctx->Const.BitmapUsesRed =
      screen->is_format_supported(screen, PIPE_FORMAT_R8_UNORM,
                                  PIPE_TEXTURE_2D, 0, 0,
                                  PIPE_BIND_SAMPLER_VIEW);

   ctx->Const.QueryCounterBits.Timestamp =
      screen->get_param(screen, PIPE_CAP_QUERY_TIMESTAMP_BITS);

   st->has_stencil_export =
      screen->get_param(screen, PIPE_CAP_SHADER_STENCIL_EXPORT);
   st->has_etc1 = screen->is_format_supported(screen, PIPE_FORMAT_ETC1_RGB8,
                                              PIPE_TEXTURE_2D, 0, 0,
                                              PIPE_BIND_SAMPLER_VIEW);
   st->has_etc2 = screen->is_format_supported(screen, PIPE_FORMAT_ETC2_RGB8,
                                              PIPE_TEXTURE_2D, 0, 0,
                                              PIPE_BIND_SAMPLER_VIEW);
   st->transcode_etc = options->transcode_etc &&
                       screen->is_format_supported(screen, PIPE_FORMAT_DXT1_SRGBA,
                                                   PIPE_TEXTURE_2D, 0, 0,
                                                   PIPE_BIND_SAMPLER_VIEW);
   st->transcode_astc = options->transcode_astc &&
                        screen->is_format_supported(screen, PIPE_FORMAT_DXT5_SRGBA,
                                                    PIPE_TEXTURE_2D, 0, 0,
                                                    PIPE_BIND_SAMPLER_VIEW) &&
                        screen->is_format_supported(screen, PIPE_FORMAT_DXT5_RGBA,
                                                    PIPE_TEXTURE_2D, 0, 0,
                                                    PIPE_BIND_SAMPLER_VIEW);
   st->has_astc_2d_ldr =
      screen->is_format_supported(screen, PIPE_FORMAT_ASTC_4x4_SRGB,
                                  PIPE_TEXTURE_2D, 0, 0, PIPE_BIND_SAMPLER_VIEW);
   st->has_astc_5x5_ldr =
      screen->is_format_supported(screen, PIPE_FORMAT_ASTC_5x5_SRGB,
                                  PIPE_TEXTURE_2D, 0, 0, PIPE_BIND_SAMPLER_VIEW);
   st->astc_void_extents_need_denorm_flush =
      screen->get_param(screen, PIPE_CAP_ASTC_VOID_EXTENTS_NEED_DENORM_FLUSH);

   st->has_s3tc = screen->is_format_supported(screen, PIPE_FORMAT_DXT5_RGBA,
                                              PIPE_TEXTURE_2D, 0, 0,
                                              PIPE_BIND_SAMPLER_VIEW);
   st->has_rgtc = screen->is_format_supported(screen, PIPE_FORMAT_RGTC2_UNORM,
                                              PIPE_TEXTURE_2D, 0, 0,
                                              PIPE_BIND_SAMPLER_VIEW);
   st->has_latc = screen->is_format_supported(screen, PIPE_FORMAT_LATC2_UNORM,
                                              PIPE_TEXTURE_2D, 0, 0,
                                              PIPE_BIND_SAMPLER_VIEW);
   st->has_bptc = screen->is_format_supported(screen, PIPE_FORMAT_BPTC_SRGBA,
                                              PIPE_TEXTURE_2D, 0, 0,
                                              PIPE_BIND_SAMPLER_VIEW);
   st->force_persample_in_shader =
      screen->get_param(screen, PIPE_CAP_SAMPLE_SHADING) &&
      !screen->get_param(screen, PIPE_CAP_FORCE_PERSAMPLE_INTERP);
   st->has_shareable_shaders = screen->get_param(screen,
                                                 PIPE_CAP_SHAREABLE_SHADERS);
   st->needs_texcoord_semantic =
      screen->get_param(screen, PIPE_CAP_TGSI_TEXCOORD);
   st->apply_texture_swizzle_to_border_color =
      !!(screen->get_param(screen, PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK) &
         (PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_NV50 |
          PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_R600));
   st->use_format_with_border_color =
      !!(screen->get_param(screen, PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK) &
         PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_FREEDRENO);
   st->alpha_border_color_is_not_w =
      !!(screen->get_param(screen, PIPE_CAP_TEXTURE_BORDER_COLOR_QUIRK) &
         PIPE_QUIRK_TEXTURE_BORDER_COLOR_SWIZZLE_ALPHA_NOT_W);
   st->emulate_gl_clamp =
      !screen->get_param(screen, PIPE_CAP_GL_CLAMP);
   st->has_time_elapsed =
      screen->get_param(screen, PIPE_CAP_QUERY_TIME_ELAPSED);
   ctx->Const.GLSLHasHalfFloatPacking =
      screen->get_param(screen, PIPE_CAP_SHADER_PACK_HALF_FLOAT);
   st->has_multi_draw_indirect =
      screen->get_param(screen, PIPE_CAP_MULTI_DRAW_INDIRECT);
   st->has_indirect_partial_stride =
      screen->get_param(screen, PIPE_CAP_MULTI_DRAW_INDIRECT_PARTIAL_STRIDE);
   st->has_occlusion_query =
      screen->get_param(screen, PIPE_CAP_OCCLUSION_QUERY);
   st->has_single_pipe_stat =
      screen->get_param(screen, PIPE_CAP_QUERY_PIPELINE_STATISTICS_SINGLE);
   st->has_pipeline_stat =
      screen->get_param(screen, PIPE_CAP_QUERY_PIPELINE_STATISTICS);
   st->has_indep_blend_enable =
      screen->get_param(screen, PIPE_CAP_INDEP_BLEND_ENABLE);
   st->has_indep_blend_func =
      screen->get_param(screen, PIPE_CAP_INDEP_BLEND_FUNC);
   st->can_dither =
      screen->get_param(screen, PIPE_CAP_DITHERING);
   st->lower_flatshade =
      !screen->get_param(screen, PIPE_CAP_FLATSHADE);
   st->lower_alpha_test =
      !screen->get_param(screen, PIPE_CAP_ALPHA_TEST);
   st->lower_point_size =
      !screen->get_param(screen, PIPE_CAP_POINT_SIZE_FIXED);
   st->lower_two_sided_color =
      !screen->get_param(screen, PIPE_CAP_TWO_SIDED_COLOR);
   st->lower_ucp =
      !screen->get_param(screen, PIPE_CAP_CLIP_PLANES);
   st->prefer_real_buffer_in_constbuf0 =
      screen->get_param(screen, PIPE_CAP_PREFER_REAL_BUFFER_IN_CONSTBUF0);
   st->has_conditional_render =
      screen->get_param(screen, PIPE_CAP_CONDITIONAL_RENDER);
   st->lower_rect_tex =
      !screen->get_param(screen, PIPE_CAP_TEXRECT);
   st->allow_st_finalize_nir_twice = screen->finalize_nir != NULL;

   st->has_hw_atomics =
      screen->get_shader_param(screen, PIPE_SHADER_FRAGMENT,
                               PIPE_SHADER_CAP_MAX_HW_ATOMIC_COUNTERS)
      ? true : false;

   st->validate_all_dirty_states =
      screen->get_param(screen, PIPE_CAP_VALIDATE_ALL_DIRTY_STATES)
      ? true : false;
   st->can_null_texture =
      screen->get_param(screen, PIPE_CAP_NULL_TEXTURES)
      ? true : false;

   util_throttle_init(&st->throttle,
                      screen->get_param(screen,
                                        PIPE_CAP_MAX_TEXTURE_UPLOAD_MEMORY_BUDGET));

   /* GL limits and extensions */
   st_init_limits(screen, &ctx->Const, &ctx->Extensions, ctx->API);
   st_init_extensions(screen, &ctx->Const,
                      &ctx->Extensions, &st->options, ctx->API);

   if (st_have_perfquery(st)) {
      ctx->Extensions.INTEL_performance_query = GL_TRUE;
   }

   /* Enable shader-based fallbacks for ARB_color_buffer_float if needed. */
   if (screen->get_param(screen, PIPE_CAP_VERTEX_COLOR_UNCLAMPED)) {
      if (!screen->get_param(screen, PIPE_CAP_VERTEX_COLOR_CLAMPED)) {
         st->clamp_vert_color_in_shader = GL_TRUE;
      }

      if (!screen->get_param(screen, PIPE_CAP_FRAGMENT_COLOR_CLAMPED)) {
         st->clamp_frag_color_in_shader = GL_TRUE;
      }

      /* For drivers which cannot do color clamping, it's better to just
       * disable ARB_color_buffer_float in the core profile, because
       * the clamping is deprecated there anyway. */
      if (_mesa_is_desktop_gl_core(ctx) &&
          (st->clamp_frag_color_in_shader || st->clamp_vert_color_in_shader)) {
         st->clamp_vert_color_in_shader = GL_FALSE;
         st->clamp_frag_color_in_shader = GL_FALSE;
         ctx->Extensions.ARB_color_buffer_float = GL_FALSE;
      }
   }

   /* called after _mesa_create_context/_mesa_init_point, fix default user
    * settable max point size up
    */
   ctx->Point.MaxSize = MAX2(ctx->Const.MaxPointSize,
                             ctx->Const.MaxPointSizeAA);

   ctx->Const.NoClippingOnCopyTex = screen->get_param(screen,
                                                      PIPE_CAP_NO_CLIP_ON_COPY_TEX);

   ctx->Const.ForceFloat32TexNearest =
      !screen->get_param(screen, PIPE_CAP_TEXTURE_FLOAT_LINEAR);

   ctx->Const.ShaderCompilerOptions[MESA_SHADER_VERTEX].PositionAlwaysInvariant = options->vs_position_always_invariant;

   ctx->Const.ShaderCompilerOptions[MESA_SHADER_TESS_EVAL].PositionAlwaysPrecise = options->vs_position_always_precise;

   /* NIR drivers that support tess shaders and compact arrays need to use
    * GLSLTessLevelsAsInputs / PIPE_CAP_GLSL_TESS_LEVELS_AS_INPUTS. The NIR
    * linker doesn't support linking these as compat arrays of sysvals.
    */
   assert(ctx->Const.GLSLTessLevelsAsInputs ||
      !screen->get_param(screen, PIPE_CAP_NIR_COMPACT_ARRAYS) ||
      !ctx->Extensions.ARB_tessellation_shader);

   /* Set which shader types can be compiled at link time. */
   st->shader_has_one_variant[MESA_SHADER_VERTEX] =
         st->has_shareable_shaders &&
         !st->clamp_vert_color_in_shader &&
         !st->lower_point_size &&
         !st->lower_ucp;

   st->shader_has_one_variant[MESA_SHADER_FRAGMENT] =
         st->has_shareable_shaders &&
         !st->lower_flatshade &&
         !st->lower_alpha_test &&
         !st->clamp_frag_color_in_shader &&
         !st->force_persample_in_shader &&
         !st->lower_two_sided_color;

   st->shader_has_one_variant[MESA_SHADER_TESS_CTRL] = st->has_shareable_shaders;
   st->shader_has_one_variant[MESA_SHADER_TESS_EVAL] =
         st->has_shareable_shaders &&
         !st->clamp_vert_color_in_shader &&
         !st->lower_point_size &&
         !st->lower_ucp;

   st->shader_has_one_variant[MESA_SHADER_GEOMETRY] =
         st->has_shareable_shaders &&
         !st->clamp_vert_color_in_shader &&
         !st->lower_point_size &&
         !st->lower_ucp;
   st->shader_has_one_variant[MESA_SHADER_COMPUTE] = st->has_shareable_shaders;

   if (util_get_cpu_caps()->num_L3_caches == 1 ||
       !st->pipe->set_context_param)
      st->pin_thread_counter = ST_L3_PINNING_DISABLED;

   st->bitmap.cache.empty = true;

   if (ctx->Const.ForceGLNamesReuse && ctx->Shared->RefCount == 1) {
      _mesa_HashEnableNameReuse(ctx->Shared->TexObjects);
      _mesa_HashEnableNameReuse(ctx->Shared->ShaderObjects);
      _mesa_HashEnableNameReuse(ctx->Shared->BufferObjects);
      _mesa_HashEnableNameReuse(ctx->Shared->SamplerObjects);
      _mesa_HashEnableNameReuse(ctx->Shared->FrameBuffers);
      _mesa_HashEnableNameReuse(ctx->Shared->RenderBuffers);
      _mesa_HashEnableNameReuse(ctx->Shared->MemoryObjects);
      _mesa_HashEnableNameReuse(ctx->Shared->SemaphoreObjects);
   }
   /* SPECviewperf13/sw-04 crashes since a56849ddda6 if Mesa is build with
    * -O3 on gcc 7.5, which doesn't happen with ForceGLNamesReuse, which is
    * the default setting for SPECviewperf because it simulates glGen behavior
    * of closed source drivers.
    */
   if (ctx->Const.ForceGLNamesReuse)
      _mesa_HashEnableNameReuse(ctx->Query.QueryObjects);

   _mesa_override_extensions(ctx);
   _mesa_compute_version(ctx);

   if (ctx->Version == 0 ||
       !_mesa_initialize_dispatch_tables(ctx)) {
      /* This can happen when a core profile was requested, but the driver
       * does not support some features of GL 3.1 or later.
       */
      st_destroy_context_priv(st, false);
      return NULL;
   }

   if (_mesa_has_compute_shaders(ctx) &&
       st->transcode_astc && !st_init_texcompress_compute(st)) {
      /* Transcoding ASTC to DXT5 using compute shaders can provide a
       * significant performance benefit over the CPU path. It isn't strictly
       * necessary to fail if we can't use the compute shader path, but it's
       * very convenient to do so. This should be rare.
       */
      st_destroy_context_priv(st, false);
      return NULL;
   }

   /* This must be done after extensions are initialized to enable persistent
    * mappings immediately.
    */
   _vbo_CreateContext(ctx);

   st_init_driver_flags(st);

   /* Initialize context's winsys buffers list */
   list_inithead(&st->winsys_buffers);

   list_inithead(&st->zombie_sampler_views.list.node);
   simple_mtx_init(&st->zombie_sampler_views.mutex, mtx_plain);
   list_inithead(&st->zombie_shaders.list.node);
   simple_mtx_init(&st->zombie_shaders.mutex, mtx_plain);

   ctx->Const.DriverSupportedPrimMask = screen->get_param(screen, PIPE_CAP_SUPPORTED_PRIM_MODES) |
                                        /* patches is always supported */
                                        BITFIELD_BIT(MESA_PRIM_PATCHES);
   st->active_states = _mesa_get_active_states(ctx);

   return st;
}

void
st_set_background_context(struct gl_context *ctx,
                          struct util_queue_monitoring *queue_info)
{
   struct st_context *st = ctx->st;
   struct pipe_frontend_screen *fscreen = st->frontend_screen;

   assert(fscreen->set_background_context);
   fscreen->set_background_context(st, queue_info);
}

static void
st_init_driver_functions(struct pipe_screen *screen,
                         struct dd_function_table *functions,
                         bool has_egl_image_validate)
{
   st_init_draw_functions(screen, functions);

   st_init_eglimage_functions(functions, has_egl_image_validate);

   functions->NewProgram = _mesa_new_program;
   st_init_flush_functions(screen, functions);

   /* GL_ARB_get_program_binary */
   functions->ShaderCacheSerializeDriverBlob =  st_serialise_nir_program;
   functions->ProgramBinarySerializeDriverBlob =
      st_serialise_nir_program_binary;
   functions->ProgramBinaryDeserializeDriverBlob =
      st_deserialise_nir_program;
}


struct st_context *
st_create_context(gl_api api, struct pipe_context *pipe,
                  const struct gl_config *visual,
                  struct st_context *share,
                  const struct st_config_options *options,
                  bool no_error, bool has_egl_image_validate)
{
   struct gl_context *ctx;
   struct gl_context *shareCtx = share ? share->ctx : NULL;
   struct dd_function_table funcs;
   struct st_context *st;

   memset(&funcs, 0, sizeof(funcs));
   st_init_driver_functions(pipe->screen, &funcs, has_egl_image_validate);

   /* gl_context must be 16-byte aligned due to the alignment on GLmatrix. */
   ctx = align_malloc(sizeof(struct gl_context), 16);
   if (!ctx)
      return NULL;
   memset(ctx, 0, sizeof(*ctx));

   ctx->pipe = pipe;
   ctx->screen = pipe->screen;

   if (!_mesa_initialize_context(ctx, api, no_error, visual, shareCtx, &funcs)) {
      align_free(ctx);
      return NULL;
   }

   st_debug_init();

   if (pipe->screen->get_disk_shader_cache)
      ctx->Cache = pipe->screen->get_disk_shader_cache(pipe->screen);

   /* XXX: need a capability bit in gallium to query if the pipe
    * driver prefers DP4 or MUL/MAD for vertex transformation.
    */
   if (debug_get_option_mesa_mvp_dp4())
      ctx->Const.ShaderCompilerOptions[MESA_SHADER_VERTEX].OptimizeForAOS = GL_TRUE;

   if (pipe->screen->get_param(pipe->screen, PIPE_CAP_INVALIDATE_BUFFER))
      ctx->has_invalidate_buffer = true;

   if (pipe->screen->get_param(pipe->screen, PIPE_CAP_STRING_MARKER))
      ctx->has_string_marker = true;

   st = st_create_context_priv(ctx, pipe, options);
   if (!st) {
      _mesa_free_context_data(ctx, true);
      align_free(ctx);
   }

   return st;
}


/**
 * When we destroy a context, we must examine all texture objects to
 * find/release any sampler views created by that context.
 *
 * This callback is called per-texture object.  It releases all the
 * texture's sampler views which belong to the context.
 */
static void
destroy_tex_sampler_cb(void *data, void *userData)
{
   struct gl_texture_object *texObj = (struct gl_texture_object *) data;
   struct st_context *st = (struct st_context *) userData;

   st_texture_release_context_sampler_view(st, texObj);
}

static void
destroy_framebuffer_attachment_sampler_cb(void *data, void *userData)
{
   struct gl_framebuffer* glfb = (struct gl_framebuffer*) data;
   struct st_context *st = (struct st_context *) userData;

    for (unsigned i = 0; i < BUFFER_COUNT; i++) {
      struct gl_renderbuffer_attachment *att = &glfb->Attachment[i];
      if (att->Texture) {
        st_texture_release_context_sampler_view(st, att->Texture);
      }
   }
}

void
st_destroy_context(struct st_context *st)
{
   struct gl_context *ctx = st->ctx;
   struct gl_framebuffer *stfb, *next;
   struct gl_framebuffer *save_drawbuffer;
   struct gl_framebuffer *save_readbuffer;

   /* Save the current context and draw/read buffers*/
   GET_CURRENT_CONTEXT(save_ctx);
   if (save_ctx) {
      save_drawbuffer = save_ctx->WinSysDrawBuffer;
      save_readbuffer = save_ctx->WinSysReadBuffer;
   } else {
      save_drawbuffer = save_readbuffer = NULL;
   }

   /*
    * We need to bind the context we're deleting so that
    * _mesa_reference_texobj_() uses this context when deleting textures.
    * Similarly for framebuffer objects, etc.
    */
   _mesa_make_current(ctx, NULL, NULL);

   /* This must be called first so that glthread has a chance to finish */
   _mesa_glthread_destroy(ctx);

   _mesa_HashWalk(ctx->Shared->TexObjects, destroy_tex_sampler_cb, st);

   /* For the fallback textures, free any sampler views belonging to this
    * context.
    */
   for (unsigned i = 0; i < NUM_TEXTURE_TARGETS; i++) {
      for (unsigned j = 0; j < ARRAY_SIZE(ctx->Shared->FallbackTex[0]); j++) {
         struct gl_texture_object *stObj =
            ctx->Shared->FallbackTex[i][j];
         if (stObj) {
            st_texture_release_context_sampler_view(st, stObj);
         }
      }
   }

   st_release_program(st, &st->fp);
   st_release_program(st, &st->gp);
   st_release_program(st, &st->vp);
   st_release_program(st, &st->tcp);
   st_release_program(st, &st->tep);
   st_release_program(st, &st->cp);

   if (st->hw_select_shaders) {
      hash_table_foreach(st->hw_select_shaders, entry)
         st->pipe->delete_gs_state(st->pipe, entry->data);
      _mesa_hash_table_destroy(st->hw_select_shaders, NULL);
   }

   /* release framebuffer in the winsys buffers list */
   LIST_FOR_EACH_ENTRY_SAFE_REV(stfb, next, &st->winsys_buffers, head) {
      _mesa_reference_framebuffer(&stfb, NULL);
   }

   _mesa_HashWalk(ctx->Shared->FrameBuffers, destroy_framebuffer_attachment_sampler_cb, st);

   pipe_sampler_view_reference(&st->pixel_xfer.pixelmap_sampler_view, NULL);
   pipe_resource_reference(&st->pixel_xfer.pixelmap_texture, NULL);

   _vbo_DestroyContext(ctx);

   st_destroy_program_variants(st);

   st_context_free_zombie_objects(st);

   simple_mtx_destroy(&st->zombie_sampler_views.mutex);
   simple_mtx_destroy(&st->zombie_shaders.mutex);

   /* Do not release debug_output yet because it might be in use by other threads.
    * These threads will be terminated by _mesa_free_context_data and
    * st_destroy_context_priv.
    */
   _mesa_free_context_data(ctx, false);

   /* This will free the st_context too, so 'st' must not be accessed
    * afterwards. */
   st_destroy_context_priv(st, true);
   st = NULL;

   _mesa_destroy_debug_output(ctx);

   align_free(ctx);

   if (save_ctx == ctx) {
      /* unbind the context we just deleted */
      _mesa_make_current(NULL, NULL, NULL);
   } else {
      /* Restore the current context and draw/read buffers (may be NULL) */
      _mesa_make_current(save_ctx, save_drawbuffer, save_readbuffer);
   }
}

const struct nir_shader_compiler_options *
st_get_nir_compiler_options(struct st_context *st, gl_shader_stage stage)
{
   return st->ctx->Const.ShaderCompilerOptions[stage].NirOptions;
}
