/**********************************************************
 * Copyright 2008-2022 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "svga_cmd.h"

#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "pipe/p_screen.h"
#include "util/u_memory.h"
#include "util/u_bitmask.h"
#include "util/u_upload_mgr.h"

#include "svga_context.h"
#include "svga_screen.h"
#include "svga_surface.h"
#include "svga_resource_texture.h"
#include "svga_resource_buffer.h"
#include "svga_resource.h"
#include "svga_winsys.h"
#include "svga_swtnl.h"
#include "svga_draw.h"
#include "svga_debug.h"
#include "svga_state.h"
#include "svga_winsys.h"
#include "svga_streamout.h"

#define CONST0_UPLOAD_DEFAULT_SIZE 65536

DEBUG_GET_ONCE_BOOL_OPTION(no_swtnl, "SVGA_NO_SWTNL", false)
DEBUG_GET_ONCE_BOOL_OPTION(force_swtnl, "SVGA_FORCE_SWTNL", false);
DEBUG_GET_ONCE_BOOL_OPTION(use_min_mipmap, "SVGA_USE_MIN_MIPMAP", false);
DEBUG_GET_ONCE_BOOL_OPTION(no_line_width, "SVGA_NO_LINE_WIDTH", false);
DEBUG_GET_ONCE_BOOL_OPTION(force_hw_line_stipple, "SVGA_FORCE_HW_LINE_STIPPLE", false);


static void
svga_destroy(struct pipe_context *pipe)
{
   struct svga_context *svga = svga_context(pipe);
   unsigned shader, i;

   /* free depthstencil_disable state */
   if (svga->depthstencil_disable) {
      pipe->delete_depth_stencil_alpha_state(pipe, svga->depthstencil_disable);
   }

   /* free HW constant buffers */
   for (shader = 0; shader < ARRAY_SIZE(svga->state.hw_draw.constbuf); shader++) {
      for (i = 0; i < ARRAY_SIZE(svga->state.hw_draw.constbuf[0]); i++) {
         pipe_resource_reference(&svga->state.hw_draw.constbuf[shader][i], NULL);
      }
   }

   pipe->delete_blend_state(pipe, svga->noop_blend);

   /* destroy stream output statistics queries */
   svga_destroy_stream_output_queries(svga);

   /* free query gb object */
   if (svga->gb_query) {
      pipe->destroy_query(pipe, NULL);
      svga->gb_query = NULL;
   }

   util_blitter_destroy(svga->blitter);

   svga_cleanup_sampler_state(svga);
   svga_cleanup_framebuffer(svga);
   svga_cleanup_tss_binding(svga);
   svga_cleanup_vertex_state(svga);
   svga_cleanup_tcs_state(svga);
   svga_cleanup_shader_image_state(svga);

   svga_destroy_swtnl(svga);
   svga_hwtnl_destroy(svga->hwtnl);

   svga->swc->destroy(svga->swc);

   util_bitmask_destroy(svga->blend_object_id_bm);
   util_bitmask_destroy(svga->ds_object_id_bm);
   util_bitmask_destroy(svga->input_element_object_id_bm);
   util_bitmask_destroy(svga->rast_object_id_bm);
   util_bitmask_destroy(svga->sampler_object_id_bm);
   util_bitmask_destroy(svga->sampler_view_id_bm);
   util_bitmask_destroy(svga->shader_id_bm);
   util_bitmask_destroy(svga->surface_view_id_bm);
   util_bitmask_destroy(svga->stream_output_id_bm);
   util_bitmask_destroy(svga->query_id_bm);
   util_bitmask_destroy(svga->uav_id_bm);
   util_bitmask_destroy(svga->uav_to_free_id_bm);

   u_upload_destroy(svga->const0_upload);
   u_upload_destroy(svga->pipe.stream_uploader);
   u_upload_destroy(svga->pipe.const_uploader);
   svga_texture_transfer_map_upload_destroy(svga);

   /* free user's constant buffers */
   for (shader = 0; shader < PIPE_SHADER_TYPES; ++shader) {
      for (i = 0; i < ARRAY_SIZE(svga->curr.constbufs[shader]); ++i) {
         pipe_resource_reference(&svga->curr.constbufs[shader][i].buffer, NULL);
      }
   }

   /* free any pending srvs that were created for rawbuf sr view for
    * constant buf.
    */
   if (svga_have_gl43(svga)) {
      svga_destroy_rawbuf_srv(svga);
      util_bitmask_destroy(svga->sampler_view_to_free_id_bm);
      pipe_resource_reference(&svga->dummy_resource, NULL);
   }

   FREE(svga);
}


struct pipe_context *
svga_context_create(struct pipe_screen *screen, void *priv, unsigned flags)
{
   struct svga_screen *svgascreen = svga_screen(screen);
   struct svga_context *svga = NULL;
   enum pipe_error ret;

   SVGA_STATS_TIME_PUSH(svgascreen->sws, SVGA_STATS_TIME_CREATECONTEXT);

   svga = CALLOC_STRUCT(svga_context);
   if (!svga)
      goto done;

   list_inithead(&svga->dirty_buffers);

   svga->pipe.screen = screen;
   svga->pipe.priv = priv;
   svga->pipe.destroy = svga_destroy;
   svga->pipe.stream_uploader = u_upload_create(&svga->pipe, 1024 * 1024,
                                                PIPE_BIND_VERTEX_BUFFER |
                                                PIPE_BIND_INDEX_BUFFER,
                                                PIPE_USAGE_STREAM, 0);
   if (!svga->pipe.stream_uploader)
      goto cleanup;

   u_upload_disable_persistent(svga->pipe.stream_uploader);

   svga->pipe.const_uploader = u_upload_create(&svga->pipe, 128 * 1024,
                                               PIPE_BIND_CONSTANT_BUFFER,
                                               PIPE_USAGE_STREAM, 0);
   if (!svga->pipe.const_uploader)
      goto cleanup;

   u_upload_disable_persistent(svga->pipe.const_uploader);

   svga->swc = svgascreen->sws->context_create(svgascreen->sws);
   if (!svga->swc)
      goto cleanup;

   svga_init_resource_functions(svga);
   svga_init_blend_functions(svga);
   svga_init_blit_functions(svga);
   svga_init_depth_stencil_functions(svga);
   svga_init_draw_functions(svga);
   svga_init_flush_functions(svga);
   svga_init_misc_functions(svga);
   svga_init_rasterizer_functions(svga);
   svga_init_sampler_functions(svga);
   svga_init_fs_functions(svga);
   svga_init_vs_functions(svga);
   svga_init_gs_functions(svga);
   svga_init_ts_functions(svga);
   svga_init_vertex_functions(svga);
   svga_init_constbuffer_functions(svga);
   svga_init_query_functions(svga);
   svga_init_surface_functions(svga);
   svga_init_stream_output_functions(svga);
   svga_init_clear_functions(svga);
   svga_init_tracked_state(svga);
   svga_init_shader_image_functions(svga);
   svga_init_shader_buffer_functions(svga);
   svga_init_cs_functions(svga);

   /* init misc state */
   svga->curr.sample_mask = ~0;

   /* debug */
   svga->debug.no_swtnl = debug_get_option_no_swtnl();
   svga->debug.force_swtnl = debug_get_option_force_swtnl();
   svga->debug.use_min_mipmap = debug_get_option_use_min_mipmap();
   svga->debug.no_line_width = debug_get_option_no_line_width();
   svga->debug.force_hw_line_stipple = debug_get_option_force_hw_line_stipple();

   if (!(svga->blend_object_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->ds_object_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->input_element_object_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->rast_object_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->sampler_object_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->sampler_view_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->shader_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->surface_view_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->stream_output_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->query_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->uav_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->uav_to_free_id_bm = util_bitmask_create()))
      goto cleanup;

   if (!(svga->sampler_view_to_free_id_bm = util_bitmask_create()))
      goto cleanup;

   svga->hwtnl = svga_hwtnl_create(svga);
   if (svga->hwtnl == NULL)
      goto cleanup;

   if (!svga_init_swtnl(svga))
      goto cleanup;

   ret = svga_emit_initial_state(svga);
   if (ret != PIPE_OK)
      goto cleanup;

   svga->const0_upload = u_upload_create(&svga->pipe,
                                         CONST0_UPLOAD_DEFAULT_SIZE,
                                         PIPE_BIND_CONSTANT_BUFFER |
                                         PIPE_BIND_CUSTOM,
                                         PIPE_USAGE_STREAM, 0);
   if (!svga->const0_upload)
      goto cleanup;

   u_upload_disable_persistent(svga->const0_upload);

   if (!svga_texture_transfer_map_upload_create(svga))
      goto cleanup;

   /* Avoid shortcircuiting state with initial value of zero.
    */
   memset(&svga->state.hw_clear, 0xcd, sizeof(svga->state.hw_clear));
   memset(&svga->state.hw_clear.framebuffer, 0x0,
          sizeof(svga->state.hw_clear.framebuffer));
   memset(&svga->state.hw_clear.rtv, 0, sizeof(svga->state.hw_clear.rtv));
   svga->state.hw_clear.num_rendertargets = 0;
   svga->state.hw_clear.dsv = NULL;

   memset(&svga->state.hw_draw, 0xcd, sizeof(svga->state.hw_draw));
   memset(&svga->state.hw_draw.views, 0x0, sizeof(svga->state.hw_draw.views));
   memset(&svga->state.hw_draw.num_samplers, 0,
      sizeof(svga->state.hw_draw.num_samplers));
   memset(&svga->state.hw_draw.num_sampler_views, 0,
      sizeof(svga->state.hw_draw.num_sampler_views));
   memset(svga->state.hw_draw.sampler_views, 0,
          sizeof(svga->state.hw_draw.sampler_views));
   svga->state.hw_draw.num_views = 0;
   svga->state.hw_draw.num_backed_views = 0;
   svga->state.hw_draw.rasterizer_discard = false;

   /* Initialize uavs */
   svga->state.hw_draw.uavSpliceIndex = -1;
   svga->state.hw_draw.num_uavs = 0;
   svga->state.hw_draw.num_cs_uavs = 0;

   /* Initialize the shader pointers */
   svga->state.hw_draw.vs = NULL;
   svga->state.hw_draw.gs = NULL;
   svga->state.hw_draw.fs = NULL;
   svga->state.hw_draw.tcs = NULL;
   svga->state.hw_draw.tes = NULL;

   /* Initialize the currently bound buffer resources */
   memset(svga->state.hw_draw.constbuf, 0,
          sizeof(svga->state.hw_draw.constbuf));
   memset(svga->state.hw_draw.default_constbuf_size, 0,
          sizeof(svga->state.hw_draw.default_constbuf_size));
   memset(svga->state.hw_draw.enabled_constbufs, 0,
          sizeof(svga->state.hw_draw.enabled_constbufs));
   memset(svga->state.hw_draw.enabled_rawbufs, 0,
          sizeof(svga->state.hw_draw.enabled_rawbufs));
   memset(svga->state.hw_draw.enabled_raw_shaderbufs, 0,
          sizeof(svga->state.hw_draw.enabled_raw_shaderbufs));
   memset(svga->state.hw_draw.rawbufs, 0,
          sizeof(svga->state.hw_draw.rawbufs));
   svga->state.hw_draw.ib = NULL;
   svga->state.hw_draw.num_vbuffers = 0;
   memset(svga->state.hw_draw.vbuffers, 0,
          sizeof(svga->state.hw_draw.vbuffers));
   svga->state.hw_draw.const0_buffer = NULL;
   svga->state.hw_draw.const0_handle = NULL;

   if (svga_have_gl43(svga)) {
      for (unsigned shader = 0; shader < PIPE_SHADER_TYPES; ++shader) {
         for (unsigned i = 0;
              i < ARRAY_SIZE(svga->state.hw_draw.rawbufs[shader]); i++) {
            svga->state.hw_draw.rawbufs[shader][i].srvid = SVGA3D_INVALID_ID;
         }
      }
      svga_uav_cache_init(svga);
      svga->dummy_resource = NULL;
   }

   /* Create a no-operation blend state which we will bind whenever the
    * requested blend state is impossible (e.g. due to having an integer
    * render target attached).
    *
    * XXX: We will probably actually need 16 of these, one for each possible
    * RGBA color mask (4 bits).  Then, we would bind the one with a color mask
    * matching the blend state it is replacing.
    */
   {
      struct pipe_blend_state noop_tmpl = {0};
      unsigned i;

      for (i = 0; i < PIPE_MAX_COLOR_BUFS; ++i) {
         // Set the color mask to all-ones.  Later this may change.
         noop_tmpl.rt[i].colormask = PIPE_MASK_RGBA;
      }
      svga->noop_blend = svga->pipe.create_blend_state(&svga->pipe, &noop_tmpl);
   }

   svga->dirty = SVGA_NEW_ALL;
   svga->pred.query_id = SVGA3D_INVALID_ID;
   svga->disable_rasterizer = false;

   /**
    * Create stream output statistics queries used in the workaround for auto
    * draw with stream instancing.
    */
   svga_create_stream_output_queries(svga);

   goto done;

cleanup:
   svga_destroy_swtnl(svga);

   if (svga->const0_upload)
      u_upload_destroy(svga->const0_upload);
   if (svga->pipe.const_uploader)
      u_upload_destroy(svga->pipe.const_uploader);
   if (svga->pipe.stream_uploader)
      u_upload_destroy(svga->pipe.stream_uploader);
   svga_texture_transfer_map_upload_destroy(svga);
   if (svga->hwtnl)
      svga_hwtnl_destroy(svga->hwtnl);
   if (svga->swc)
      svga->swc->destroy(svga->swc);
   util_bitmask_destroy(svga->blend_object_id_bm);
   util_bitmask_destroy(svga->ds_object_id_bm);
   util_bitmask_destroy(svga->input_element_object_id_bm);
   util_bitmask_destroy(svga->rast_object_id_bm);
   util_bitmask_destroy(svga->sampler_object_id_bm);
   util_bitmask_destroy(svga->shader_id_bm);
   util_bitmask_destroy(svga->surface_view_id_bm);
   util_bitmask_destroy(svga->stream_output_id_bm);
   util_bitmask_destroy(svga->query_id_bm);

   util_bitmask_destroy(svga->uav_id_bm);
   util_bitmask_destroy(svga->uav_to_free_id_bm);
   util_bitmask_destroy(svga->sampler_view_id_bm);

   FREE(svga);
   svga = NULL;

done:
   SVGA_STATS_TIME_POP(svgascreen->sws);
   return svga ? &svga->pipe:NULL;
}


void
svga_context_flush(struct svga_context *svga,
                   struct pipe_fence_handle **pfence)
{
   struct svga_screen *svgascreen = svga_screen(svga->pipe.screen);
   struct pipe_fence_handle *fence = NULL;
   uint64_t t0;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_CONTEXTFLUSH);

   svga->curr.nr_fbs = 0;

   /* Unmap the 0th/default constant buffer.  The u_upload_unmap() function
    * will call pipe_context::transfer_flush_region() to indicate the
    * region of the buffer which was modified (and needs to be uploaded).
    */
   if (svga->state.hw_draw.const0_handle) {
      assert(svga->state.hw_draw.const0_buffer);
      u_upload_unmap(svga->const0_upload);
      pipe_resource_reference(&svga->state.hw_draw.const0_buffer, NULL);
      svga->state.hw_draw.const0_handle = NULL;
   }

   /* Ensure that texture dma uploads are processed
    * before submitting commands.
    */
   svga_context_flush_buffers(svga);

   svga->hud.command_buffer_size +=
      svga->swc->get_command_buffer_size(svga->swc);

   /* Flush pending commands to hardware:
    */
   t0 = svga_get_time(svga);
   svga->swc->flush(svga->swc, &fence);
   svga->hud.flush_time += (svga_get_time(svga) - t0);

   svga->hud.num_flushes++;

   svga_screen_cache_flush(svgascreen, svga, fence);

   SVGA3D_ResetLastCommand(svga->swc);

   /* To force the re-emission of rendertargets and texture sampler bindings on
    * the next command buffer.
    */
   svga->rebind.flags.rendertargets = true;
   svga->rebind.flags.texture_samplers = true;

   if (svga_have_gb_objects(svga)) {

      svga->rebind.flags.constbufs = true;
      svga->rebind.flags.vs = true;
      svga->rebind.flags.fs = true;
      svga->rebind.flags.gs = true;

      if (svga_have_sm5(svga)) {
         svga->rebind.flags.tcs = true;
         svga->rebind.flags.tes = true;
      }

      if (svga_need_to_rebind_resources(svga)) {
         svga->rebind.flags.query = true;
      }

      if (svga_sws(svga)->have_index_vertex_buffer_offset_cmd) {
         svga->rebind.flags.vertexbufs = true;
         svga->rebind.flags.indexbuf = true;
      }
   }

   if (SVGA_DEBUG & DEBUG_SYNC) {
      if (fence)
         svga->pipe.screen->fence_finish(svga->pipe.screen, NULL, fence,
                                          OS_TIMEOUT_INFINITE);
   }

   if (pfence)
      svgascreen->sws->fence_reference(svgascreen->sws, pfence, fence);

   svgascreen->sws->fence_reference(svgascreen->sws, &fence, NULL);

   SVGA_STATS_TIME_POP(svga_sws(svga));
}


/**
 * Flush pending commands and wait for completion with a fence.
 */
void
svga_context_finish(struct svga_context *svga)
{
   struct pipe_screen *screen = svga->pipe.screen;
   struct pipe_fence_handle *fence = NULL;

   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_CONTEXTFINISH);

   svga_context_flush(svga, &fence);
   screen->fence_finish(screen, NULL, fence, OS_TIMEOUT_INFINITE);
   screen->fence_reference(screen, &fence, NULL);

   SVGA_STATS_TIME_POP(svga_sws(svga));
}


/**
 * Emit pending drawing commands to the command buffer.
 * If the command buffer overflows, we flush it and retry.
 * \sa svga_hwtnl_flush()
 */
void
svga_hwtnl_flush_retry(struct svga_context *svga)
{
   enum pipe_error ret = PIPE_OK;

   SVGA_RETRY_OOM(svga, ret, svga_hwtnl_flush(svga->hwtnl));
   assert(ret == PIPE_OK);
}


/**
 * Flush the primitive queue if this buffer is referred.
 *
 * Otherwise DMA commands on the referred buffer will be emitted too late.
 */
void
svga_hwtnl_flush_buffer(struct svga_context *svga,
                        struct pipe_resource *buffer)
{
   if (svga_hwtnl_is_buffer_referred(svga->hwtnl, buffer)) {
      svga_hwtnl_flush_retry(svga);
   }
}


/**
 * Emit all operations pending on host surfaces.
 */
void
svga_surfaces_flush(struct svga_context *svga)
{
   SVGA_STATS_TIME_PUSH(svga_sws(svga), SVGA_STATS_TIME_SURFACEFLUSH);

   /* Emit buffered drawing commands.
    */
   svga_hwtnl_flush_retry(svga);

   /* Emit back-copy from render target views to textures.
    */
   svga_propagate_rendertargets(svga);

   SVGA_STATS_TIME_POP(svga_sws(svga));
}


struct svga_winsys_context *
svga_winsys_context(struct pipe_context *pipe)
{
   return svga_context(pipe)->swc;
}
