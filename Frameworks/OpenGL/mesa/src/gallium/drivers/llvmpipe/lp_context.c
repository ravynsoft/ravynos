/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * Copyright 2008 VMware, Inc.  All rights reserved.
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

/* Author:
 *    Keith Whitwell <keithw@vmware.com>
 */

#include "draw/draw_context.h"
#include "draw/draw_vbuf.h"
#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/list.h"
#include "util/u_upload_mgr.h"
#include "lp_clear.h"
#include "lp_context.h"
#include "lp_flush.h"
#include "lp_perf.h"
#include "lp_state.h"
#include "lp_surface.h"
#include "lp_query.h"
#include "lp_setup.h"
#include "lp_screen.h"
#include "lp_fence.h"

static void
llvmpipe_destroy(struct pipe_context *pipe)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);
   struct llvmpipe_screen *lp_screen = llvmpipe_screen(pipe->screen);
   uint i;

   mtx_lock(&lp_screen->ctx_mutex);
   list_del(&llvmpipe->list);
   mtx_unlock(&lp_screen->ctx_mutex);
   lp_print_counters();

   if (llvmpipe->csctx) {
      lp_csctx_destroy(llvmpipe->csctx);
   }
   if (llvmpipe->task_ctx) {
      lp_csctx_destroy(llvmpipe->task_ctx);
   }
   if (llvmpipe->mesh_ctx) {
      lp_csctx_destroy(llvmpipe->mesh_ctx);
   }
   if (llvmpipe->blitter) {
      util_blitter_destroy(llvmpipe->blitter);
   }

   if (llvmpipe->pipe.stream_uploader)
      u_upload_destroy(llvmpipe->pipe.stream_uploader);

   /* This will also destroy llvmpipe->setup:
    */
   if (llvmpipe->draw)
      draw_destroy(llvmpipe->draw);

   util_unreference_framebuffer_state(&llvmpipe->framebuffer);

   for (enum pipe_shader_type s = PIPE_SHADER_VERTEX; s < PIPE_SHADER_MESH_TYPES; s++) {
      for (i = 0; i < ARRAY_SIZE(llvmpipe->sampler_views[0]); i++) {
         pipe_sampler_view_reference(&llvmpipe->sampler_views[s][i], NULL);
      }
      for (i = 0; i < LP_MAX_TGSI_SHADER_IMAGES; i++) {
         pipe_resource_reference(&llvmpipe->images[s][i].resource, NULL);
      }
      for (i = 0; i < LP_MAX_TGSI_SHADER_BUFFERS; i++) {
         pipe_resource_reference(&llvmpipe->ssbos[s][i].buffer, NULL);
      }
      for (i = 0; i < ARRAY_SIZE(llvmpipe->constants[s]); i++) {
         pipe_resource_reference(&llvmpipe->constants[s][i].buffer, NULL);
      }
   }

   for (i = 0; i < llvmpipe->num_vertex_buffers; i++) {
      pipe_vertex_buffer_unreference(&llvmpipe->vertex_buffer[i]);
   }

   lp_delete_setup_variants(llvmpipe);

   llvmpipe_sampler_matrix_destroy(llvmpipe);

#ifndef USE_GLOBAL_LLVM_CONTEXT
   LLVMContextDispose(llvmpipe->context);
#endif
   llvmpipe->context = NULL;

   align_free(llvmpipe);
}


static void
do_flush(struct pipe_context *pipe,
         struct pipe_fence_handle **fence,
         unsigned flags)
{
   llvmpipe_flush(pipe, fence, __func__);
}


static void
llvmpipe_fence_server_sync(struct pipe_context *pipe,
                           struct pipe_fence_handle *fence)
{
   struct lp_fence *f = (struct lp_fence *)fence;

   if (!f->issued)
      return;
   lp_fence_wait(f);
}


static void
llvmpipe_render_condition(struct pipe_context *pipe,
                          struct pipe_query *query,
                          bool condition,
                          enum pipe_render_cond_flag mode)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   llvmpipe->render_cond_query = query;
   llvmpipe->render_cond_mode = mode;
   llvmpipe->render_cond_cond = condition;
}


static void
llvmpipe_render_condition_mem(struct pipe_context *pipe,
                              struct pipe_resource *buffer,
                              unsigned offset,
                              bool condition)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   llvmpipe->render_cond_buffer = llvmpipe_resource(buffer);
   llvmpipe->render_cond_offset = offset;
   llvmpipe->render_cond_cond = condition;
}


static void
llvmpipe_texture_barrier(struct pipe_context *pipe, unsigned flags)
{
   llvmpipe_finish(pipe, "barrier");
}


static void
lp_draw_disk_cache_find_shader(void *cookie,
                               struct lp_cached_code *cache,
                               unsigned char ir_sha1_cache_key[20])
{
   struct llvmpipe_screen *screen = cookie;
   lp_disk_cache_find_shader(screen, cache, ir_sha1_cache_key);
}


static void
lp_draw_disk_cache_insert_shader(void *cookie,
                                 struct lp_cached_code *cache,
                                 unsigned char ir_sha1_cache_key[20])
{
   struct llvmpipe_screen *screen = cookie;
   lp_disk_cache_insert_shader(screen, cache, ir_sha1_cache_key);
}


static enum pipe_reset_status
llvmpipe_get_device_reset_status(struct pipe_context *pipe)
{
   return PIPE_NO_RESET;
}


struct pipe_context *
llvmpipe_create_context(struct pipe_screen *screen, void *priv,
                        unsigned flags)
{
   struct llvmpipe_context *llvmpipe;
   struct llvmpipe_screen *lp_screen = llvmpipe_screen(screen);

   if (!llvmpipe_screen_late_init(lp_screen))
      return NULL;

   llvmpipe = align_malloc(sizeof(struct llvmpipe_context), 16);
   if (!llvmpipe)
      return NULL;

   memset(llvmpipe, 0, sizeof *llvmpipe);

   list_inithead(&llvmpipe->fs_variants_list.list);

   list_inithead(&llvmpipe->setup_variants_list.list);

   list_inithead(&llvmpipe->cs_variants_list.list);

   llvmpipe->pipe.screen = screen;
   llvmpipe->pipe.priv = priv;

   /* Init the pipe context methods */
   llvmpipe->pipe.destroy = llvmpipe_destroy;
   llvmpipe->pipe.set_framebuffer_state = llvmpipe_set_framebuffer_state;
   llvmpipe->pipe.clear = llvmpipe_clear;
   llvmpipe->pipe.flush = do_flush;
   llvmpipe->pipe.texture_barrier = llvmpipe_texture_barrier;

   llvmpipe->pipe.render_condition = llvmpipe_render_condition;
   llvmpipe->pipe.render_condition_mem = llvmpipe_render_condition_mem;

   llvmpipe->pipe.fence_server_sync = llvmpipe_fence_server_sync;
   llvmpipe->pipe.get_device_reset_status = llvmpipe_get_device_reset_status;
   llvmpipe_init_blend_funcs(llvmpipe);
   llvmpipe_init_clip_funcs(llvmpipe);
   llvmpipe_init_draw_funcs(llvmpipe);
   llvmpipe_init_compute_funcs(llvmpipe);
   llvmpipe_init_sampler_funcs(llvmpipe);
   llvmpipe_init_query_funcs(llvmpipe);
   llvmpipe_init_vertex_funcs(llvmpipe);
   llvmpipe_init_so_funcs(llvmpipe);
   llvmpipe_init_fs_funcs(llvmpipe);
   llvmpipe_init_vs_funcs(llvmpipe);
   llvmpipe_init_gs_funcs(llvmpipe);
   llvmpipe_init_tess_funcs(llvmpipe);
   llvmpipe_init_task_funcs(llvmpipe);
   llvmpipe_init_mesh_funcs(llvmpipe);
   llvmpipe_init_rasterizer_funcs(llvmpipe);
   llvmpipe_init_context_resource_funcs(&llvmpipe->pipe);
   llvmpipe_init_surface_functions(llvmpipe);

   llvmpipe_init_sampler_matrix(llvmpipe);

#ifdef USE_GLOBAL_LLVM_CONTEXT
   llvmpipe->context = LLVMGetGlobalContext();
#else
   llvmpipe->context = LLVMContextCreate();
#endif

   if (!llvmpipe->context)
      goto fail;

#if LLVM_VERSION_MAJOR == 15
   LLVMContextSetOpaquePointers(llvmpipe->context, false);
#endif

   /*
    * Create drawing context and plug our rendering stage into it.
    */
   llvmpipe->draw = draw_create_with_llvm_context(&llvmpipe->pipe,
                                                  llvmpipe->context);
   if (!llvmpipe->draw)
      goto fail;

   draw_set_disk_cache_callbacks(llvmpipe->draw,
                                 lp_screen,
                                 lp_draw_disk_cache_find_shader,
                                 lp_draw_disk_cache_insert_shader);

   draw_set_constant_buffer_stride(llvmpipe->draw,
                                   lp_get_constant_buffer_stride(screen));

   /* FIXME: devise alternative to draw_texture_samplers */

   llvmpipe->setup = lp_setup_create(&llvmpipe->pipe, llvmpipe->draw);
   if (!llvmpipe->setup)
      goto fail;

   llvmpipe->csctx = lp_csctx_create(&llvmpipe->pipe);
   if (!llvmpipe->csctx)
      goto fail;

   llvmpipe->task_ctx = lp_csctx_create(&llvmpipe->pipe);
   if (!llvmpipe->task_ctx)
      goto fail;

   llvmpipe->mesh_ctx = lp_csctx_create(&llvmpipe->pipe);
   if (!llvmpipe->mesh_ctx)
      goto fail;

   llvmpipe->pipe.stream_uploader = u_upload_create_default(&llvmpipe->pipe);
   if (!llvmpipe->pipe.stream_uploader)
      goto fail;

   llvmpipe->pipe.const_uploader = llvmpipe->pipe.stream_uploader;

   llvmpipe->blitter = util_blitter_create(&llvmpipe->pipe);
   if (!llvmpipe->blitter) {
      goto fail;
   }

   /* must be done before installing Draw stages */
   util_blitter_cache_all_shaders(llvmpipe->blitter);

   /* plug in AA line/point stages */
   draw_install_aaline_stage(llvmpipe->draw, &llvmpipe->pipe);
   draw_install_aapoint_stage(llvmpipe->draw, &llvmpipe->pipe, nir_type_bool32);
   draw_install_pstipple_stage(llvmpipe->draw, &llvmpipe->pipe);

   /* convert points and lines into triangles:
    * (otherwise, draw points and lines natively)
    */
   draw_wide_point_sprites(llvmpipe->draw, false);
   draw_enable_point_sprites(llvmpipe->draw, false);
   draw_wide_point_threshold(llvmpipe->draw, 10000.0);
   draw_wide_line_threshold(llvmpipe->draw, 10000.0);

   /* initial state for clipping - enabled, with no guardband */
   draw_set_driver_clipping(llvmpipe->draw, false, false, false, true);

   lp_reset_counters();

   /* If llvmpipe_set_scissor_states() is never called, we still need to
    * make sure that derived scissor state is computed.
    * See https://bugs.freedesktop.org/show_bug.cgi?id=101709
    */
   llvmpipe->dirty |= LP_NEW_SCISSOR;

   mtx_lock(&lp_screen->ctx_mutex);
   list_addtail(&llvmpipe->list, &lp_screen->ctx_list);
   mtx_unlock(&lp_screen->ctx_mutex);
   return &llvmpipe->pipe;

 fail:
   llvmpipe_destroy(&llvmpipe->pipe);
   return NULL;
}
