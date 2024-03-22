/**************************************************************************
 *
 * Copyright 2009, VMware, Inc.
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
 * Author: Keith Whitwell <keithw@vmware.com>
 * Author: Jakob Bornecrantz <wallbraker@gmail.com>
 */

#include "dri_screen.h"
#include "dri_drawable.h"
#include "dri_context.h"
#include "frontend/drm_driver.h"

#include "pipe/p_context.h"
#include "pipe-loader/pipe_loader.h"
#include "state_tracker/st_context.h"

#include "util/u_cpu_detect.h"
#include "util/u_memory.h"
#include "util/u_debug.h"

struct dri_context *
dri_create_context(struct dri_screen *screen,
                   gl_api api, const struct gl_config *visual,
                   const struct __DriverContextConfig *ctx_config,
                   unsigned *error,
                   struct dri_context *sharedContextPrivate,
                   void *loaderPrivate)
{
   struct dri_context *ctx = NULL;
   struct st_context *st_share = NULL;
   struct st_context_attribs attribs;
   enum st_context_error ctx_err = 0;
   unsigned allowed_flags = __DRI_CTX_FLAG_DEBUG |
                            __DRI_CTX_FLAG_FORWARD_COMPATIBLE;
   unsigned allowed_attribs =
      __DRIVER_CONTEXT_ATTRIB_PRIORITY |
      __DRIVER_CONTEXT_ATTRIB_RELEASE_BEHAVIOR |
      __DRIVER_CONTEXT_ATTRIB_NO_ERROR;
   const __DRIbackgroundCallableExtension *backgroundCallable =
      screen->dri2.backgroundCallable;
   const struct driOptionCache *optionCache = &screen->dev->option_cache;

   /* This is effectively doing error checking for GLX context creation (by both
    * Mesa and the X server) when the driver doesn't support the robustness ext.
    * EGL already checks, so it won't send us the flags if the ext isn't
    * available.
    */
   if (screen->has_reset_status_query) {
      allowed_flags |= __DRI_CTX_FLAG_ROBUST_BUFFER_ACCESS;
      allowed_attribs |= __DRIVER_CONTEXT_ATTRIB_RESET_STRATEGY;
   }

   if (screen->has_protected_context)
      allowed_attribs |= __DRIVER_CONTEXT_ATTRIB_PROTECTED;

   if (ctx_config->flags & ~allowed_flags) {
      *error = __DRI_CTX_ERROR_UNKNOWN_FLAG;
      goto fail;
   }

   if (ctx_config->attribute_mask & ~allowed_attribs) {
      *error = __DRI_CTX_ERROR_UNKNOWN_ATTRIBUTE;
      goto fail;
   }

   memset(&attribs, 0, sizeof(attribs));
   switch (api) {
   case API_OPENGLES:
      attribs.profile = API_OPENGLES;
      break;
   case API_OPENGLES2:
      attribs.profile = API_OPENGLES2;
      break;
   case API_OPENGL_COMPAT:
   case API_OPENGL_CORE:
      if (driQueryOptionb(optionCache, "force_compat_profile")) {
         attribs.profile = API_OPENGL_COMPAT;
      } else {
         attribs.profile = api == API_OPENGL_COMPAT ? API_OPENGL_COMPAT
                                                    : API_OPENGL_CORE;
      }

      attribs.major = ctx_config->major_version;
      attribs.minor = ctx_config->minor_version;

      if ((ctx_config->flags & __DRI_CTX_FLAG_FORWARD_COMPATIBLE) != 0)
	 attribs.flags |= ST_CONTEXT_FLAG_FORWARD_COMPATIBLE;
      break;
   default:
      *error = __DRI_CTX_ERROR_BAD_API;
      goto fail;
   }

   if ((ctx_config->flags & __DRI_CTX_FLAG_DEBUG) != 0)
      attribs.flags |= ST_CONTEXT_FLAG_DEBUG;

   if (ctx_config->flags & __DRI_CTX_FLAG_ROBUST_BUFFER_ACCESS)
      attribs.context_flags |= PIPE_CONTEXT_ROBUST_BUFFER_ACCESS;

   if (ctx_config->attribute_mask & __DRIVER_CONTEXT_ATTRIB_RESET_STRATEGY)
      if (ctx_config->reset_strategy != __DRI_CTX_RESET_NO_NOTIFICATION)
         attribs.context_flags |= PIPE_CONTEXT_LOSE_CONTEXT_ON_RESET;

   if (ctx_config->attribute_mask & __DRIVER_CONTEXT_ATTRIB_NO_ERROR)
      attribs.flags |= ctx_config->no_error ? ST_CONTEXT_FLAG_NO_ERROR : 0;

   if (ctx_config->attribute_mask & __DRIVER_CONTEXT_ATTRIB_PRIORITY) {
      switch (ctx_config->priority) {
      case __DRI_CTX_PRIORITY_LOW:
         attribs.context_flags |= PIPE_CONTEXT_LOW_PRIORITY;
         break;
      case __DRI_CTX_PRIORITY_HIGH:
         attribs.context_flags |= PIPE_CONTEXT_HIGH_PRIORITY;
         break;
      default:
         break;
      }
   }

   if ((ctx_config->attribute_mask & __DRIVER_CONTEXT_ATTRIB_RELEASE_BEHAVIOR)
       && (ctx_config->release_behavior == __DRI_CTX_RELEASE_BEHAVIOR_NONE))
      attribs.flags |= ST_CONTEXT_FLAG_RELEASE_NONE;

   if (ctx_config->attribute_mask & __DRIVER_CONTEXT_ATTRIB_PROTECTED)
      attribs.context_flags |= PIPE_CONTEXT_PROTECTED;

   struct dri_context *share_ctx = NULL;
   if (sharedContextPrivate) {
      share_ctx = (struct dri_context *)sharedContextPrivate;
      st_share = share_ctx->st;
   }

   ctx = CALLOC_STRUCT(dri_context);
   if (ctx == NULL) {
      *error = __DRI_CTX_ERROR_NO_MEMORY;
      goto fail;
   }

   ctx->screen = screen;
   ctx->loaderPrivate = loaderPrivate;

   /* KHR_no_error is likely to crash, overflow memory, etc if an application
    * has errors so don't enable it for setuid processes.
    */
   if (debug_get_bool_option("MESA_NO_ERROR", false) ||
       driQueryOptionb(&screen->dev->option_cache, "mesa_no_error"))
#if !defined(_WIN32)
      if (__normal_user())
#endif
         attribs.flags |= ST_CONTEXT_FLAG_NO_ERROR;

   attribs.options = screen->options;
   dri_fill_st_visual(&attribs.visual, screen, visual);
   ctx->st = st_api_create_context(&screen->base, &attribs, &ctx_err,
				   st_share);
   if (ctx->st == NULL) {
      switch (ctx_err) {
      case ST_CONTEXT_SUCCESS:
	 *error = __DRI_CTX_ERROR_SUCCESS;
	 break;
      case ST_CONTEXT_ERROR_NO_MEMORY:
	 *error = __DRI_CTX_ERROR_NO_MEMORY;
	 break;
      case ST_CONTEXT_ERROR_BAD_VERSION:
	 *error = __DRI_CTX_ERROR_BAD_VERSION;
	 break;
      }
      goto fail;
   }
   ctx->st->frontend_context = (void *) ctx;

   if (ctx->st->cso_context) {
      ctx->pp = pp_init(ctx->st->pipe, screen->pp_enabled, ctx->st->cso_context,
                        ctx->st, st_context_invalidate_state);
      ctx->hud = hud_create(ctx->st->cso_context,
                            share_ctx ? share_ctx->hud : NULL,
                            ctx->st, st_context_invalidate_state);
   }

   /* order of precedence (least to most):
    * - driver setting
    * - app setting
    * - user setting
    */
   bool enable_glthread = driQueryOptionb(&screen->dev->option_cache, "mesa_glthread_driver");

   /* always disable glthread by default if fewer than 5 "big" CPUs are active */
   unsigned nr_big_cpus = util_get_cpu_caps()->nr_big_cpus;
   if (util_get_cpu_caps()->nr_cpus < 4 || (nr_big_cpus && nr_big_cpus < 5))
      enable_glthread = false;

   int app_enable_glthread = driQueryOptioni(&screen->dev->option_cache, "mesa_glthread_app_profile");
   if (app_enable_glthread != -1) {
      /* if set (not -1), apply the app setting */
      enable_glthread = app_enable_glthread == 1;
   }
   if (getenv("mesa_glthread")) {
      /* only apply the env var if set */
      bool user_enable_glthread = debug_get_bool_option("mesa_glthread", false);
      if (user_enable_glthread != enable_glthread) {
         /* print warning to mimic old behavior */
         fprintf(stderr, "ATTENTION: default value of option mesa_glthread overridden by environment.\n");
      }
      enable_glthread = user_enable_glthread;
   }
   /* Do this last. */
   if (enable_glthread) {
      bool safe = true;

      /* This is only needed by X11/DRI2, which can be unsafe. */
      if (backgroundCallable &&
          backgroundCallable->base.version >= 2 &&
          backgroundCallable->isThreadSafe &&
          !backgroundCallable->isThreadSafe(loaderPrivate))
         safe = false;

      if (safe)
         _mesa_glthread_init(ctx->st->ctx);
   }

   *error = __DRI_CTX_ERROR_SUCCESS;
   return ctx;

 fail:
   if (ctx && ctx->st)
      st_destroy_context(ctx->st);

   free(ctx);
   return NULL;
}

void
dri_destroy_context(struct dri_context *ctx)
{
   /* Wait for glthread to finish because we can't use pipe_context from
    * multiple threads.
    */
   _mesa_glthread_finish(ctx->st->ctx);

   if (ctx->hud) {
      hud_destroy(ctx->hud, ctx->st->cso_context);
   }

   if (ctx->pp)
      pp_free(ctx->pp);

   /* No particular reason to wait for command completion before
    * destroying a context, but we flush the context here
    * to avoid having to add code elsewhere to cope with flushing a
    * partially destroyed context.
    */
   st_context_flush(ctx->st, 0, NULL, NULL, NULL);
   st_destroy_context(ctx->st);
   free(ctx);
}

/* This is called inside MakeCurrent to unbind the context. */
bool
dri_unbind_context(struct dri_context *ctx)
{
   /* dri_util.c ensures cPriv is not null */
   struct st_context *st = ctx->st;

   if (st == st_api_get_current()) {
      _mesa_glthread_finish(st->ctx);

      /* Record HUD queries for the duration the context was "current". */
      if (ctx->hud)
         hud_record_only(ctx->hud, st->pipe);

      st_api_make_current(NULL, NULL, NULL);
   }

   if (ctx->draw || ctx->read) {
      assert(ctx->draw);

      dri_put_drawable(ctx->draw);

      if (ctx->read != ctx->draw)
          dri_put_drawable(ctx->read);

      ctx->draw = NULL;
      ctx->read = NULL;
   }

   return GL_TRUE;
}

bool
dri_make_current(struct dri_context *ctx,
		 struct dri_drawable *draw,
		 struct dri_drawable *read)
{
   /* dri_unbind_context() is always called before this, so drawables are
    * always NULL here.
    */
   assert(!ctx->draw);
   assert(!ctx->read);

   if ((draw && !read) || (!draw && read))
      return GL_FALSE; /* only both non-NULL or both NULL are allowed */

   /* Wait for glthread to finish because we can't use st_context from
    * multiple threads.
    */
   _mesa_glthread_finish(ctx->st->ctx);

   /* There are 2 cases that can occur here. Either we bind drawables, or we
    * bind NULL for configless and surfaceless contexts.
    */
   if (!draw && !read)
      return st_api_make_current(ctx->st, NULL, NULL);

   /* Bind drawables to the context */
   ctx->draw = draw;
   ctx->read = read;

   dri_get_drawable(draw);
   draw->texture_stamp = draw->lastStamp - 1;

   if (draw != read) {
      dri_get_drawable(read);
      read->texture_stamp = read->lastStamp - 1;
   }

   st_api_make_current(ctx->st, &draw->base, &read->base);

   /* This is ok to call here. If they are already init, it's a no-op. */
   if (ctx->pp && draw->textures[ST_ATTACHMENT_BACK_LEFT])
      pp_init_fbos(ctx->pp, draw->textures[ST_ATTACHMENT_BACK_LEFT]->width0,
                   draw->textures[ST_ATTACHMENT_BACK_LEFT]->height0);

   return GL_TRUE;
}

struct dri_context *
dri_get_current(void)
{
   struct st_context *st = st_api_get_current();

   return (struct dri_context *) st ? st->frontend_context : NULL;
}

/* vim: set sw=3 ts=8 sts=3 expandtab: */
