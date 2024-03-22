/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "freedreno_query_hw.h"

#include "fd3_blend.h"
#include "fd3_context.h"
#include "fd3_draw.h"
#include "fd3_emit.h"
#include "fd3_gmem.h"
#include "fd3_program.h"
#include "fd3_query.h"
#include "fd3_rasterizer.h"
#include "fd3_texture.h"
#include "fd3_zsa.h"

static void
fd3_context_destroy(struct pipe_context *pctx) in_dt
{
   struct fd3_context *fd3_ctx = fd3_context(fd_context(pctx));

   u_upload_destroy(fd3_ctx->border_color_uploader);
   pipe_resource_reference(&fd3_ctx->border_color_buf, NULL);

   fd_context_destroy(pctx);

   fd_bo_del(fd3_ctx->vs_pvt_mem);
   fd_bo_del(fd3_ctx->fs_pvt_mem);
   fd_bo_del(fd3_ctx->vsc_size_mem);

   fd_context_cleanup_common_vbos(&fd3_ctx->base);

   fd_hw_query_fini(pctx);

   free(fd3_ctx);
}

struct pipe_context *
fd3_context_create(struct pipe_screen *pscreen, void *priv,
                   unsigned flags) in_dt
{
   struct fd_screen *screen = fd_screen(pscreen);
   struct fd3_context *fd3_ctx = CALLOC_STRUCT(fd3_context);
   struct pipe_context *pctx;

   if (!fd3_ctx)
      return NULL;

   pctx = &fd3_ctx->base.base;
   pctx->screen = pscreen;

   fd3_ctx->base.flags = flags;
   fd3_ctx->base.dev = fd_device_ref(screen->dev);
   fd3_ctx->base.screen = fd_screen(pscreen);
   fd3_ctx->base.last.key = &fd3_ctx->last_key;

   pctx->destroy = fd3_context_destroy;
   pctx->create_blend_state = fd3_blend_state_create;
   pctx->create_rasterizer_state = fd3_rasterizer_state_create;
   pctx->create_depth_stencil_alpha_state = fd3_zsa_state_create;

   fd3_draw_init(pctx);
   fd3_gmem_init(pctx);
   fd3_texture_init(pctx);
   fd3_prog_init(pctx);
   fd3_emit_init(pctx);

   pctx = fd_context_init(&fd3_ctx->base, pscreen, priv, flags);
   if (!pctx)
      return NULL;

   fd_hw_query_init(pctx);

   fd3_ctx->vs_pvt_mem =
      fd_bo_new(screen->dev, 0x2000, 0, "vs_pvt");

   fd3_ctx->fs_pvt_mem =
      fd_bo_new(screen->dev, 0x2000, 0, "fs_pvt");

   fd3_ctx->vsc_size_mem =
      fd_bo_new(screen->dev, 0x1000, 0, "vsc_size");

   fd_context_setup_common_vbos(&fd3_ctx->base);

   fd3_query_context_init(pctx);

   fd3_ctx->border_color_uploader =
      u_upload_create(pctx, 4096, 0, PIPE_USAGE_STREAM, 0);

   return pctx;
}
