/*
 * Copyright © 2008 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Soft-
 * ware"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, provided that the above copyright
 * notice(s) and this permission notice appear in all copies of the Soft-
 * ware and that both the above copyright notice(s) and this permission
 * notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
 * ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY
 * RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN
 * THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSE-
 * QUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFOR-
 * MANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization of
 * the copyright holder.
 *
 * Authors:
 *   Kristian Høgsberg (krh@redhat.com)
 */

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)

#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>
#include <xcb/dri2.h>
#include "glxclient.h"
#include <X11/extensions/dri2proto.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "dri2.h"
#include "dri_common.h"
#include "dri2_priv.h"
#include "loader.h"
#include "loader_dri_helper.h"

#undef DRI2_MINOR
#define DRI2_MINOR 1

struct dri2_display
{
   __GLXDRIdisplay base;

   __glxHashTable *dri2Hash;

   const __DRIextension *loader_extensions[5];
};

struct dri2_drawable
{
   __GLXDRIdrawable base;
   __DRIdrawable *driDrawable;
   __DRIbuffer buffers[5];
   int bufferCount;
   int width, height;
   int have_back;
   int have_fake_front;
   int swap_interval;

   uint64_t previous_time;
   unsigned frames;
};

static const struct glx_context_vtable dri2_context_vtable;

/* For XCB's handling of ust/msc/sbc counters, we have to hand it the high and
 * low halves separately.  This helps you split them.
 */
static void
split_counter(uint64_t counter, uint32_t *hi, uint32_t *lo)
{
   *hi = (counter >> 32);
   *lo = counter & 0xffffffff;
}

static uint64_t
merge_counter(uint32_t hi, uint32_t lo)
{
   return ((uint64_t)hi << 32) | lo;
}

static void
dri2_destroy_context(struct glx_context *context)
{
   struct dri2_screen *psc = (struct dri2_screen *) context->psc;

   driReleaseDrawables(context);

   free((char *) context->extensions);

   psc->core->destroyContext(context->driContext);

   free(context);
}

static Bool
dri2_bind_context(struct glx_context *context, GLXDrawable draw, GLXDrawable read)
{
   struct dri2_screen *psc = (struct dri2_screen *) context->psc;
   struct dri2_drawable *pdraw, *pread;
   __DRIdrawable *dri_draw = NULL, *dri_read = NULL;

   pdraw = (struct dri2_drawable *) driFetchDrawable(context, draw);
   pread = (struct dri2_drawable *) driFetchDrawable(context, read);

   driReleaseDrawables(context);

   if (pdraw)
      dri_draw = pdraw->driDrawable;
   else if (draw != None)
      return GLXBadDrawable;

   if (pread)
      dri_read = pread->driDrawable;
   else if (read != None)
      return GLXBadDrawable;

   if (!psc->core->bindContext(context->driContext, dri_draw, dri_read))
      return GLXBadContext;

   return Success;
}

static void
dri2_unbind_context(struct glx_context *context)
{
   struct dri2_screen *psc = (struct dri2_screen *) context->psc;

   psc->core->unbindContext(context->driContext);
}

static struct glx_context *
dri2_create_context_attribs(struct glx_screen *base,
			    struct glx_config *config_base,
			    struct glx_context *shareList,
			    unsigned num_attribs,
			    const uint32_t *attribs,
			    unsigned *error)
{
   struct glx_context *pcp = NULL;
   struct dri2_screen *psc = (struct dri2_screen *) base;
   __GLXDRIconfigPrivate *config = (__GLXDRIconfigPrivate *) config_base;
   __DRIcontext *shared = NULL;

   struct dri_ctx_attribs dca;
   uint32_t ctx_attribs[2 * 6];
   unsigned num_ctx_attribs = 0;

   *error = dri_convert_glx_attribs(num_attribs, attribs, &dca);
   if (*error != __DRI_CTX_ERROR_SUCCESS)
      goto error_exit;

   /* Check the renderType value */
   if (!validate_renderType_against_config(config_base, dca.render_type)) {
      *error = BadValue;
      goto error_exit;
   }

   if (shareList) {
      /* We can't share with an indirect context */
      if (!shareList->isDirect)
         return NULL;

      /* The GLX_ARB_create_context_no_error specs say:
       *
       *    BadMatch is generated if the value of GLX_CONTEXT_OPENGL_NO_ERROR_ARB
       *    used to create <share_context> does not match the value of
       *    GLX_CONTEXT_OPENGL_NO_ERROR_ARB for the context being created.
       */
      if (!!shareList->noError != !!dca.no_error) {
         *error = BadMatch;
         return NULL;
      }

      shared = shareList->driContext;
   }

   pcp = calloc(1, sizeof *pcp);
   if (pcp == NULL) {
      *error = BadAlloc;
      goto error_exit;
   }

   if (!glx_context_init(pcp, &psc->base, config_base))
      goto error_exit;

   ctx_attribs[num_ctx_attribs++] = __DRI_CTX_ATTRIB_MAJOR_VERSION;
   ctx_attribs[num_ctx_attribs++] = dca.major_ver;
   ctx_attribs[num_ctx_attribs++] = __DRI_CTX_ATTRIB_MINOR_VERSION;
   ctx_attribs[num_ctx_attribs++] = dca.minor_ver;

   /* Only send a value when the non-default value is requested.  By doing
    * this we don't have to check the driver's DRI2 version before sending the
    * default value.
    */
   if (dca.reset != __DRI_CTX_RESET_NO_NOTIFICATION) {
      ctx_attribs[num_ctx_attribs++] = __DRI_CTX_ATTRIB_RESET_STRATEGY;
      ctx_attribs[num_ctx_attribs++] = dca.reset;
   }

   if (dca.release != __DRI_CTX_RELEASE_BEHAVIOR_FLUSH) {
      ctx_attribs[num_ctx_attribs++] = __DRI_CTX_ATTRIB_RELEASE_BEHAVIOR;
      ctx_attribs[num_ctx_attribs++] = dca.release;
   }

   if (dca.no_error) {
      ctx_attribs[num_ctx_attribs++] = __DRI_CTX_ATTRIB_NO_ERROR;
      ctx_attribs[num_ctx_attribs++] = dca.no_error;
      pcp->noError = GL_TRUE;
   }

   if (dca.flags != 0) {
      ctx_attribs[num_ctx_attribs++] = __DRI_CTX_ATTRIB_FLAGS;
      ctx_attribs[num_ctx_attribs++] = dca.flags;
   }

   /* The renderType is retrieved from attribs, or set to default
    *  of GLX_RGBA_TYPE.
    */
   pcp->renderType = dca.render_type;

   pcp->driContext =
      psc->dri2->createContextAttribs(psc->driScreen,
					  dca.api,
					  config ? config->driConfig : NULL,
					  shared,
					  num_ctx_attribs / 2,
					  ctx_attribs,
					  error,
					  pcp);

   *error = dri_context_error_to_glx_error(*error);

   if (pcp->driContext == NULL)
      goto error_exit;

   pcp->vtable = base->context_vtable;

   return pcp;

error_exit:
   free(pcp);

   return NULL;
}

static void
dri2DestroyDrawable(__GLXDRIdrawable *base)
{
   struct dri2_screen *psc = (struct dri2_screen *) base->psc;
   struct dri2_drawable *pdraw = (struct dri2_drawable *) base;
   struct glx_display *dpyPriv = psc->base.display;
   struct dri2_display *pdp = (struct dri2_display *)dpyPriv->dri2Display;

   __glxHashDelete(pdp->dri2Hash, pdraw->base.xDrawable);
   psc->core->destroyDrawable(pdraw->driDrawable);

   /* If it's a GLX 1.3 drawables, we can destroy the DRI2 drawable
    * now, as the application explicitly asked to destroy the GLX
    * drawable.  Otherwise, for legacy drawables, we let the DRI2
    * drawable linger on the server, since there's no good way of
    * knowing when the application is done with it.  The server will
    * destroy the DRI2 drawable when it destroys the X drawable or the
    * client exits anyway. */
   if (pdraw->base.xDrawable != pdraw->base.drawable)
      DRI2DestroyDrawable(psc->base.dpy, pdraw->base.xDrawable);

   free(pdraw);
}

static __GLXDRIdrawable *
dri2CreateDrawable(struct glx_screen *base, XID xDrawable,
                   GLXDrawable drawable, int type,
                   struct glx_config *config_base)
{
   struct dri2_drawable *pdraw;
   struct dri2_screen *psc = (struct dri2_screen *) base;
   __GLXDRIconfigPrivate *config = (__GLXDRIconfigPrivate *) config_base;
   struct glx_display *dpyPriv;
   struct dri2_display *pdp;

   dpyPriv = __glXInitialize(psc->base.dpy);
   if (dpyPriv == NULL)
      return NULL;

   pdraw = calloc(1, sizeof(*pdraw));
   if (!pdraw)
      return NULL;

   pdraw->base.destroyDrawable = dri2DestroyDrawable;
   pdraw->base.xDrawable = xDrawable;
   pdraw->base.drawable = drawable;
   pdraw->base.psc = &psc->base;
   pdraw->bufferCount = 0;
   pdraw->swap_interval = dri_get_initial_swap_interval(psc->driScreen, psc->config);
   pdraw->have_back = 0;

   DRI2CreateDrawable(psc->base.dpy, xDrawable);
   pdp = (struct dri2_display *)dpyPriv->dri2Display;
   /* Create a new drawable */
   pdraw->driDrawable =
      psc->dri2->createNewDrawable(psc->driScreen,
                                       config->driConfig, pdraw);

   if (!pdraw->driDrawable) {
      DRI2DestroyDrawable(psc->base.dpy, xDrawable);
      free(pdraw);
      return NULL;
   }

   if (__glxHashInsert(pdp->dri2Hash, xDrawable, pdraw)) {
      psc->core->destroyDrawable(pdraw->driDrawable);
      DRI2DestroyDrawable(psc->base.dpy, xDrawable);
      free(pdraw);
      return None;
   }

   /*
    * Make sure server has the same swap interval we do for the new
    * drawable.
    */
   if (psc->vtable.setSwapInterval)
      psc->vtable.setSwapInterval(&pdraw->base, pdraw->swap_interval);

   return &pdraw->base;
}

static int
dri2DrawableGetMSC(struct glx_screen *psc, __GLXDRIdrawable *pdraw,
		   int64_t *ust, int64_t *msc, int64_t *sbc)
{
   xcb_connection_t *c = XGetXCBConnection(pdraw->psc->dpy);
   xcb_dri2_get_msc_cookie_t get_msc_cookie;
   xcb_dri2_get_msc_reply_t *get_msc_reply;

   get_msc_cookie = xcb_dri2_get_msc_unchecked(c, pdraw->xDrawable);
   get_msc_reply = xcb_dri2_get_msc_reply(c, get_msc_cookie, NULL);

   if (!get_msc_reply)
      return 0;

   *ust = merge_counter(get_msc_reply->ust_hi, get_msc_reply->ust_lo);
   *msc = merge_counter(get_msc_reply->msc_hi, get_msc_reply->msc_lo);
   *sbc = merge_counter(get_msc_reply->sbc_hi, get_msc_reply->sbc_lo);
   free(get_msc_reply);

   return 1;
}

static int
dri2WaitForMSC(__GLXDRIdrawable *pdraw, int64_t target_msc, int64_t divisor,
	       int64_t remainder, int64_t *ust, int64_t *msc, int64_t *sbc)
{
   xcb_connection_t *c = XGetXCBConnection(pdraw->psc->dpy);
   xcb_dri2_wait_msc_cookie_t wait_msc_cookie;
   xcb_dri2_wait_msc_reply_t *wait_msc_reply;
   uint32_t target_msc_hi, target_msc_lo;
   uint32_t divisor_hi, divisor_lo;
   uint32_t remainder_hi, remainder_lo;

   split_counter(target_msc, &target_msc_hi, &target_msc_lo);
   split_counter(divisor, &divisor_hi, &divisor_lo);
   split_counter(remainder, &remainder_hi, &remainder_lo);

   wait_msc_cookie = xcb_dri2_wait_msc_unchecked(c, pdraw->xDrawable,
                                                 target_msc_hi, target_msc_lo,
                                                 divisor_hi, divisor_lo,
                                                 remainder_hi, remainder_lo);
   wait_msc_reply = xcb_dri2_wait_msc_reply(c, wait_msc_cookie, NULL);

   if (!wait_msc_reply)
      return 0;

   *ust = merge_counter(wait_msc_reply->ust_hi, wait_msc_reply->ust_lo);
   *msc = merge_counter(wait_msc_reply->msc_hi, wait_msc_reply->msc_lo);
   *sbc = merge_counter(wait_msc_reply->sbc_hi, wait_msc_reply->sbc_lo);
   free(wait_msc_reply);

   return 1;
}

static int
dri2WaitForSBC(__GLXDRIdrawable *pdraw, int64_t target_sbc, int64_t *ust,
	       int64_t *msc, int64_t *sbc)
{
   xcb_connection_t *c = XGetXCBConnection(pdraw->psc->dpy);
   xcb_dri2_wait_sbc_cookie_t wait_sbc_cookie;
   xcb_dri2_wait_sbc_reply_t *wait_sbc_reply;
   uint32_t target_sbc_hi, target_sbc_lo;

   split_counter(target_sbc, &target_sbc_hi, &target_sbc_lo);

   wait_sbc_cookie = xcb_dri2_wait_sbc_unchecked(c, pdraw->xDrawable,
                                                 target_sbc_hi, target_sbc_lo);
   wait_sbc_reply = xcb_dri2_wait_sbc_reply(c, wait_sbc_cookie, NULL);

   if (!wait_sbc_reply)
      return 0;

   *ust = merge_counter(wait_sbc_reply->ust_hi, wait_sbc_reply->ust_lo);
   *msc = merge_counter(wait_sbc_reply->msc_hi, wait_sbc_reply->msc_lo);
   *sbc = merge_counter(wait_sbc_reply->sbc_hi, wait_sbc_reply->sbc_lo);
   free(wait_sbc_reply);

   return 1;
}

static __DRIcontext *
dri2GetCurrentContext()
{
   struct glx_context *gc = __glXGetCurrentContext();

   return (gc != &dummyContext) ? gc->driContext : NULL;
}

/**
 * dri2Throttle - Request driver throttling
 *
 * This function uses the DRI2 throttle extension to give the
 * driver the opportunity to throttle on flush front, copysubbuffer
 * and swapbuffers.
 */
static void
dri2Throttle(struct dri2_screen *psc,
	     struct dri2_drawable *draw,
	     enum __DRI2throttleReason reason)
{
   if (psc->throttle) {
      __DRIcontext *ctx = dri2GetCurrentContext();

      psc->throttle->throttle(ctx, draw->driDrawable, reason);
   }
}

/**
 * Asks the driver to flush any queued work necessary for serializing with the
 * X command stream, and optionally the slightly more strict requirement of
 * glFlush() equivalence (which would require flushing even if nothing had
 * been drawn to a window system framebuffer, for example).
 */
static void
dri2Flush(struct dri2_screen *psc,
          __DRIcontext *ctx,
          struct dri2_drawable *draw,
          unsigned flags,
          enum __DRI2throttleReason throttle_reason)
{
   if (ctx && psc->f && psc->f->base.version >= 4) {
      psc->f->flush_with_flags(ctx, draw->driDrawable, flags, throttle_reason);
   } else {
      if (flags & __DRI2_FLUSH_CONTEXT)
         glFlush();

      if (psc->f)
         psc->f->flush(draw->driDrawable);

      dri2Throttle(psc, draw, throttle_reason);
   }
}

static void
__dri2CopySubBuffer(__GLXDRIdrawable *pdraw, int x, int y,
		    int width, int height,
		    enum __DRI2throttleReason reason, Bool flush)
{
   struct dri2_drawable *priv = (struct dri2_drawable *) pdraw;
   struct dri2_screen *psc = (struct dri2_screen *) pdraw->psc;
   XRectangle xrect;
   XserverRegion region;
   __DRIcontext *ctx = dri2GetCurrentContext();
   unsigned flags;

   /* Check we have the right attachments */
   if (!priv->have_back)
      return;

   xrect.x = x;
   xrect.y = priv->height - y - height;
   xrect.width = width;
   xrect.height = height;

   flags = __DRI2_FLUSH_DRAWABLE;
   if (flush)
      flags |= __DRI2_FLUSH_CONTEXT;
   dri2Flush(psc, ctx, priv, flags, __DRI2_THROTTLE_COPYSUBBUFFER);

   region = XFixesCreateRegion(psc->base.dpy, &xrect, 1);
   DRI2CopyRegion(psc->base.dpy, pdraw->xDrawable, region,
                  DRI2BufferFrontLeft, DRI2BufferBackLeft);

   /* Refresh the fake front (if present) after we just damaged the real
    * front.
    */
   if (priv->have_fake_front)
      DRI2CopyRegion(psc->base.dpy, pdraw->xDrawable, region,
		     DRI2BufferFakeFrontLeft, DRI2BufferFrontLeft);

   XFixesDestroyRegion(psc->base.dpy, region);
}

static void
dri2CopySubBuffer(__GLXDRIdrawable *pdraw, int x, int y,
		  int width, int height, Bool flush)
{
   __dri2CopySubBuffer(pdraw, x, y, width, height,
		       __DRI2_THROTTLE_COPYSUBBUFFER, flush);
}


static void
dri2_copy_drawable(struct dri2_drawable *priv, int dest, int src)
{
   XRectangle xrect;
   XserverRegion region;
   struct dri2_screen *psc = (struct dri2_screen *) priv->base.psc;

   xrect.x = 0;
   xrect.y = 0;
   xrect.width = priv->width;
   xrect.height = priv->height;

   if (psc->f)
      psc->f->flush(priv->driDrawable);

   region = XFixesCreateRegion(psc->base.dpy, &xrect, 1);
   DRI2CopyRegion(psc->base.dpy, priv->base.xDrawable, region, dest, src);
   XFixesDestroyRegion(psc->base.dpy, region);

}

static void
dri2_wait_x(struct glx_context *gc)
{
   struct dri2_drawable *priv = (struct dri2_drawable *)
      GetGLXDRIDrawable(gc->currentDpy, gc->currentDrawable);

   if (priv == NULL || !priv->have_fake_front)
      return;

   dri2_copy_drawable(priv, DRI2BufferFakeFrontLeft, DRI2BufferFrontLeft);
}

static void
dri2_wait_gl(struct glx_context *gc)
{
   struct dri2_drawable *priv = (struct dri2_drawable *)
      GetGLXDRIDrawable(gc->currentDpy, gc->currentDrawable);

   if (priv == NULL || !priv->have_fake_front)
      return;

   dri2_copy_drawable(priv, DRI2BufferFrontLeft, DRI2BufferFakeFrontLeft);
}

/**
 * Called by the driver when it needs to update the real front buffer with the
 * contents of its fake front buffer.
 */
static void
dri2FlushFrontBuffer(__DRIdrawable *driDrawable, void *loaderPrivate)
{
   struct glx_display *priv;
   struct glx_context *gc;
   struct dri2_drawable *pdraw = loaderPrivate;
   struct dri2_screen *psc;

   if (!pdraw)
      return;

   if (!pdraw->base.psc)
      return;

   psc = (struct dri2_screen *) pdraw->base.psc;

   priv = __glXInitialize(psc->base.dpy);

   if (priv == NULL)
       return;

   gc = __glXGetCurrentContext();

   dri2Throttle(psc, pdraw, __DRI2_THROTTLE_FLUSHFRONT);

   dri2_wait_gl(gc);
}


static void
dri2DestroyScreen(struct glx_screen *base)
{
   struct dri2_screen *psc = (struct dri2_screen *) base;

   /* Free the direct rendering per screen data */
   psc->core->destroyScreen(psc->driScreen);
   driDestroyConfigs(psc->driver_configs);
   free(psc->driverName);
   close(psc->fd);
   free(psc);
}

/**
 * Process list of buffer received from the server
 *
 * Processes the list of buffers received in a reply from the server to either
 * \c DRI2GetBuffers or \c DRI2GetBuffersWithFormat.
 */
static void
process_buffers(struct dri2_drawable * pdraw, DRI2Buffer * buffers,
                unsigned count)
{
   int i;

   pdraw->bufferCount = count;
   pdraw->have_fake_front = 0;
   pdraw->have_back = 0;

   /* This assumes the DRI2 buffer attachment tokens matches the
    * __DRIbuffer tokens. */
   for (i = 0; i < count; i++) {
      pdraw->buffers[i].attachment = buffers[i].attachment;
      pdraw->buffers[i].name = buffers[i].name;
      pdraw->buffers[i].pitch = buffers[i].pitch;
      pdraw->buffers[i].cpp = buffers[i].cpp;
      pdraw->buffers[i].flags = buffers[i].flags;
      if (pdraw->buffers[i].attachment == __DRI_BUFFER_FAKE_FRONT_LEFT)
         pdraw->have_fake_front = 1;
      if (pdraw->buffers[i].attachment == __DRI_BUFFER_BACK_LEFT)
         pdraw->have_back = 1;
   }

}

unsigned dri2GetSwapEventType(Display* dpy, XID drawable)
{
      struct glx_display *glx_dpy = __glXInitialize(dpy);
      __GLXDRIdrawable *pdraw;
      pdraw = dri2GetGlxDrawableFromXDrawableId(dpy, drawable);
      if (!pdraw || !(pdraw->eventMask & GLX_BUFFER_SWAP_COMPLETE_INTEL_MASK))
         return 0;
      return glx_dpy->codes.first_event + GLX_BufferSwapComplete;
}

static int64_t
dri2XcbSwapBuffers(Display *dpy,
                  __GLXDRIdrawable *pdraw,
                  int64_t target_msc,
                  int64_t divisor,
                  int64_t remainder)
{
   xcb_dri2_swap_buffers_cookie_t swap_buffers_cookie;
   xcb_dri2_swap_buffers_reply_t *swap_buffers_reply;
   uint32_t target_msc_hi, target_msc_lo;
   uint32_t divisor_hi, divisor_lo;
   uint32_t remainder_hi, remainder_lo;
   int64_t ret = 0;
   xcb_connection_t *c = XGetXCBConnection(dpy);

   split_counter(target_msc, &target_msc_hi, &target_msc_lo);
   split_counter(divisor, &divisor_hi, &divisor_lo);
   split_counter(remainder, &remainder_hi, &remainder_lo);

   swap_buffers_cookie =
      xcb_dri2_swap_buffers_unchecked(c, pdraw->xDrawable,
                                      target_msc_hi, target_msc_lo,
                                      divisor_hi, divisor_lo,
                                      remainder_hi, remainder_lo);

   /* Immediately wait on the swapbuffers reply.  If we didn't, we'd have
    * to do so some time before reusing a (non-pageflipped) backbuffer.
    * Otherwise, the new rendering could get ahead of the X Server's
    * dispatch of the swapbuffer and you'd display garbage.
    *
    * We use XSync() first to reap the invalidate events through the event
    * filter, to ensure that the next drawing doesn't use an invalidated
    * buffer.
    */
   XSync(dpy, False);

   swap_buffers_reply =
      xcb_dri2_swap_buffers_reply(c, swap_buffers_cookie, NULL);
   if (swap_buffers_reply) {
      ret = merge_counter(swap_buffers_reply->swap_hi,
                          swap_buffers_reply->swap_lo);
      free(swap_buffers_reply);
   }
   return ret;
}

static int64_t
dri2SwapBuffers(__GLXDRIdrawable *pdraw, int64_t target_msc, int64_t divisor,
		int64_t remainder, Bool flush)
{
    struct dri2_drawable *priv = (struct dri2_drawable *) pdraw;
    struct dri2_screen *psc = (struct dri2_screen *) priv->base.psc;
    int64_t ret = 0;

    /* Check we have the right attachments */
    if (!priv->have_back)
	return ret;

    __DRIcontext *ctx = dri2GetCurrentContext();
    unsigned flags = __DRI2_FLUSH_DRAWABLE;
    if (flush)
       flags |= __DRI2_FLUSH_CONTEXT;
    dri2Flush(psc, ctx, priv, flags, __DRI2_THROTTLE_SWAPBUFFER);

    ret = dri2XcbSwapBuffers(pdraw->psc->dpy, pdraw,
                             target_msc, divisor, remainder);

    return ret;
}

static __DRIbuffer *
dri2GetBuffers(__DRIdrawable * driDrawable,
               int *width, int *height,
               unsigned int *attachments, int count,
               int *out_count, void *loaderPrivate)
{
   struct dri2_drawable *pdraw = loaderPrivate;
   DRI2Buffer *buffers;

   buffers = DRI2GetBuffers(pdraw->base.psc->dpy, pdraw->base.xDrawable,
                            width, height, attachments, count, out_count);
   if (buffers == NULL)
      return NULL;

   pdraw->width = *width;
   pdraw->height = *height;
   process_buffers(pdraw, buffers, *out_count);

   free(buffers);

   return pdraw->buffers;
}

static __DRIbuffer *
dri2GetBuffersWithFormat(__DRIdrawable * driDrawable,
                         int *width, int *height,
                         unsigned int *attachments, int count,
                         int *out_count, void *loaderPrivate)
{
   struct dri2_drawable *pdraw = loaderPrivate;
   DRI2Buffer *buffers;

   buffers = DRI2GetBuffersWithFormat(pdraw->base.psc->dpy,
                                      pdraw->base.xDrawable,
                                      width, height, attachments,
                                      count, out_count);
   if (buffers == NULL)
      return NULL;

   pdraw->width = *width;
   pdraw->height = *height;
   process_buffers(pdraw, buffers, *out_count);

   free(buffers);

   return pdraw->buffers;
}

static int
dri2SetSwapInterval(__GLXDRIdrawable *pdraw, int interval)
{
   xcb_connection_t *c = XGetXCBConnection(pdraw->psc->dpy);
   struct dri2_drawable *priv =  (struct dri2_drawable *) pdraw;
   struct dri2_screen *psc = (struct dri2_screen *) priv->base.psc;

   if (!dri_valid_swap_interval(psc->driScreen, psc->config, interval))
      return GLX_BAD_VALUE;

   xcb_dri2_swap_interval(c, priv->base.xDrawable, interval);
   priv->swap_interval = interval;

   return 0;
}

static int
dri2GetSwapInterval(__GLXDRIdrawable *pdraw)
{
   struct dri2_drawable *priv =  (struct dri2_drawable *) pdraw;

  return priv->swap_interval;
}

static void
driSetBackgroundContext(void *loaderPrivate)
{
   __glXSetCurrentContext(loaderPrivate);
}

static GLboolean
driIsThreadSafe(void *loaderPrivate)
{
   struct glx_context *pcp = (struct glx_context *) loaderPrivate;
   /* Check Xlib is running in thread safe mode
    *
    * 'lock_fns' is the XLockDisplay function pointer of the X11 display 'dpy'.
    * It will be NULL if XInitThreads wasn't called.
    */
   return pcp->psc->dpy->lock_fns != NULL;
}

static const __DRIdri2LoaderExtension dri2LoaderExtension = {
   .base = { __DRI_DRI2_LOADER, 3 },

   .getBuffers              = dri2GetBuffers,
   .flushFrontBuffer        = dri2FlushFrontBuffer,
   .getBuffersWithFormat    = dri2GetBuffersWithFormat,
};

const __DRIuseInvalidateExtension dri2UseInvalidate = {
   .base = { __DRI_USE_INVALIDATE, 1 }
};

const __DRIbackgroundCallableExtension driBackgroundCallable = {
   .base = { __DRI_BACKGROUND_CALLABLE, 2 },

   .setBackgroundContext    = driSetBackgroundContext,
   .isThreadSafe            = driIsThreadSafe,
};

_X_HIDDEN void
dri2InvalidateBuffers(Display *dpy, XID drawable)
{
   __GLXDRIdrawable *pdraw =
      dri2GetGlxDrawableFromXDrawableId(dpy, drawable);
   struct dri2_screen *psc;
   struct dri2_drawable *pdp = (struct dri2_drawable *) pdraw;

   if (!pdraw)
      return;

   psc = (struct dri2_screen *) pdraw->psc;

   if (psc->f && psc->f->base.version >= 3 && psc->f->invalidate)
       psc->f->invalidate(pdp->driDrawable);
}

static void
dri2_bind_tex_image(__GLXDRIdrawable *base,
		    int buffer, const int *attrib_list)
{
   struct glx_context *gc = __glXGetCurrentContext();
   struct dri2_drawable *pdraw = (struct dri2_drawable *) base;
   struct dri2_screen *psc;

   if (pdraw != NULL) {
      psc = (struct dri2_screen *) base->psc;

      if (psc->texBuffer->base.version >= 2 &&
	  psc->texBuffer->setTexBuffer2 != NULL) {
	 psc->texBuffer->setTexBuffer2(gc->driContext,
					   pdraw->base.textureTarget,
					   pdraw->base.textureFormat,
					   pdraw->driDrawable);
      }
      else {
	 psc->texBuffer->setTexBuffer(gc->driContext,
					  pdraw->base.textureTarget,
					  pdraw->driDrawable);
      }
   }
}

static void
dri2_release_tex_image(__GLXDRIdrawable *base, int buffer)
{
   struct glx_context *gc = __glXGetCurrentContext();
   struct dri2_drawable *pdraw = (struct dri2_drawable *) base;
   struct dri2_screen *psc;

   if (pdraw != NULL) {
      psc = (struct dri2_screen *) base->psc;

      if (psc->texBuffer->base.version >= 3 &&
          psc->texBuffer->releaseTexBuffer != NULL) {
         psc->texBuffer->releaseTexBuffer(gc->driContext,
                                           pdraw->base.textureTarget,
                                           pdraw->driDrawable);
      }
   }
}

static const struct glx_context_vtable dri2_context_vtable = {
   .destroy             = dri2_destroy_context,
   .bind                = dri2_bind_context,
   .unbind              = dri2_unbind_context,
   .wait_gl             = dri2_wait_gl,
   .wait_x              = dri2_wait_x,
   .interop_query_device_info = dri2_interop_query_device_info,
   .interop_export_object = dri2_interop_export_object,
   .interop_flush_objects = dri2_interop_flush_objects
};

static void
dri2BindExtensions(struct dri2_screen *psc, struct glx_display * priv,
                   const char *driverName)
{
   const unsigned mask = psc->dri2->getAPIMask(psc->driScreen);
   const __DRIextension **extensions;
   int i;

   extensions = psc->core->getExtensions(psc->driScreen);

   __glXEnableDirectExtension(&psc->base, "GLX_EXT_swap_control");
   __glXEnableDirectExtension(&psc->base, "GLX_SGI_swap_control");
   __glXEnableDirectExtension(&psc->base, "GLX_MESA_swap_control");
   __glXEnableDirectExtension(&psc->base, "GLX_SGI_make_current_read");

   /*
    * GLX_INTEL_swap_event is broken on the server side, where it's
    * currently unconditionally enabled. This completely breaks
    * systems running on drivers which don't support that extension.
    * There's no way to test for its presence on this side, so instead
    * of disabling it unconditionally, just disable it for drivers
    * which are known to not support it.
    *
    * This was fixed in xserver 1.15.0 (190b03215), so now we only
    * disable the broken driver.
    */
   if (strcmp(driverName, "vmwgfx") != 0) {
      __glXEnableDirectExtension(&psc->base, "GLX_INTEL_swap_event");
   }

   __glXEnableDirectExtension(&psc->base, "GLX_ARB_create_context");
   __glXEnableDirectExtension(&psc->base, "GLX_ARB_create_context_profile");
   __glXEnableDirectExtension(&psc->base, "GLX_ARB_create_context_no_error");
   __glXEnableDirectExtension(&psc->base, "GLX_EXT_no_config_context");

   if ((mask & ((1 << __DRI_API_GLES) |
                (1 << __DRI_API_GLES2) |
                (1 << __DRI_API_GLES3))) != 0) {
      __glXEnableDirectExtension(&psc->base,
                                 "GLX_EXT_create_context_es_profile");
      __glXEnableDirectExtension(&psc->base,
                                 "GLX_EXT_create_context_es2_profile");
   }

   static const struct dri_extension_match exts[] = {
       { __DRI_TEX_BUFFER, 1, offsetof(struct dri2_screen, texBuffer), true },
       { __DRI2_FLUSH, 1, offsetof(struct dri2_screen, f), true },
       { __DRI2_CONFIG_QUERY, 1, offsetof(struct dri2_screen, config), true },
       { __DRI2_THROTTLE, 1, offsetof(struct dri2_screen, throttle), true },
       { __DRI2_RENDERER_QUERY, 1, offsetof(struct dri2_screen, rendererQuery), true },
       { __DRI2_INTEROP, 1, offsetof(struct dri2_screen, interop), true },
   };
   loader_bind_extensions(psc, exts, ARRAY_SIZE(exts), extensions);

   /* Extensions where we don't care about the extension struct */
   for (i = 0; extensions[i]; i++) {
      if (strcmp(extensions[i]->name, __DRI2_ROBUSTNESS) == 0)
         __glXEnableDirectExtension(&psc->base,
                                    "GLX_ARB_create_context_robustness");

      if (strcmp(extensions[i]->name, __DRI2_FLUSH_CONTROL) == 0)
         __glXEnableDirectExtension(&psc->base,
                                    "GLX_ARB_context_flush_control");
   }

   if (psc->texBuffer)
      __glXEnableDirectExtension(&psc->base, "GLX_EXT_texture_from_pixmap");

   if (psc->rendererQuery)
      __glXEnableDirectExtension(&psc->base, "GLX_MESA_query_renderer");

   if (psc->interop)
      __glXEnableDirectExtension(&psc->base, "GLX_MESA_gl_interop");
}

static char *
dri2_get_driver_name(struct glx_screen *glx_screen)
{
    struct dri2_screen *psc = (struct dri2_screen *)glx_screen;

    return psc->driverName;
}

static const struct glx_screen_vtable dri2_screen_vtable = {
   .create_context         = dri_common_create_context,
   .create_context_attribs = dri2_create_context_attribs,
   .query_renderer_integer = dri2_query_renderer_integer,
   .query_renderer_string  = dri2_query_renderer_string,
   .get_driver_name        = dri2_get_driver_name,
};

static struct glx_screen *
dri2CreateScreen(int screen, struct glx_display * priv)
{
   const __DRIconfig **driver_configs;
   const __DRIextension **extensions;
   const struct dri2_display *const pdp = (struct dri2_display *)
      priv->dri2Display;
   struct dri2_screen *psc;
   __GLXDRIscreen *psp;
   struct glx_config *configs = NULL, *visuals = NULL;
   char *driverName = NULL, *loader_driverName, *deviceName, *tmp;
   drm_magic_t magic;

   psc = calloc(1, sizeof *psc);
   if (psc == NULL)
      return NULL;

   psc->fd = -1;

   if (!glx_screen_init(&psc->base, screen, priv)) {
      free(psc);
      return NULL;
   }

   if (!DRI2Connect(priv->dpy, RootWindow(priv->dpy, screen),
		    &driverName, &deviceName)) {
      glx_screen_cleanup(&psc->base);
      free(psc);
      InfoMessageF("screen %d does not appear to be DRI2 capable\n", screen);
      return NULL;
   }

   psc->fd = loader_open_device(deviceName);
   if (psc->fd < 0) {
      ErrorMessageF("failed to open %s: %s\n", deviceName, strerror(errno));
      goto handle_error;
   }

   if (drmGetMagic(psc->fd, &magic)) {
      ErrorMessageF("failed to get magic\n");
      goto handle_error;
   }

   if (!DRI2Authenticate(priv->dpy, RootWindow(priv->dpy, screen), magic)) {
      ErrorMessageF("failed to authenticate magic %d\n", magic);
      goto handle_error;
   }

   /* If Mesa knows about the appropriate driver for this fd, then trust it.
    * Otherwise, default to the server's value.
    */
   loader_driverName = loader_get_driver_for_fd(psc->fd);
   if (loader_driverName) {
      free(driverName);
      driverName = loader_driverName;
   }
   psc->driverName = driverName;

   extensions = driOpenDriver(driverName, &psc->driver);
   if (extensions == NULL)
      goto handle_error;

   static const struct dri_extension_match exts[] = {
       { __DRI_CORE, 1, offsetof(struct dri2_screen, core), false },
       { __DRI_DRI2, 4, offsetof(struct dri2_screen, dri2), false },
       { __DRI_MESA, 1, offsetof(struct dri2_screen, mesa), false },
   };
   if (!loader_bind_extensions(psc, exts, ARRAY_SIZE(exts), extensions))
      goto handle_error;

   psc->driScreen =
       psc->dri2->createNewScreen2(screen, psc->fd,
                                   (const __DRIextension **)&pdp->loader_extensions[0],
                                   extensions,
                                   &driver_configs, psc);

   if (psc->driScreen == NULL) {
      ErrorMessageF("glx: failed to create dri2 screen\n");
      goto handle_error;
   }

   dri2BindExtensions(psc, priv, driverName);

   configs = driConvertConfigs(psc->core, psc->base.configs, driver_configs);
   visuals = driConvertConfigs(psc->core, psc->base.visuals, driver_configs);

   if (!configs || !visuals) {
       ErrorMessageF("No matching fbConfigs or visuals found\n");
       goto handle_error;
   }

   glx_config_destroy_list(psc->base.configs);
   psc->base.configs = configs;
   glx_config_destroy_list(psc->base.visuals);
   psc->base.visuals = visuals;

   psc->driver_configs = driver_configs;

   psc->base.vtable = &dri2_screen_vtable;
   psc->base.context_vtable = &dri2_context_vtable;
   psp = &psc->vtable;
   psc->base.driScreen = psp;
   psp->destroyScreen = dri2DestroyScreen;
   psp->createDrawable = dri2CreateDrawable;
   psp->swapBuffers = dri2SwapBuffers;
   psp->getDrawableMSC = NULL;
   psp->waitForMSC = NULL;
   psp->waitForSBC = NULL;
   psp->setSwapInterval = NULL;
   psp->getSwapInterval = NULL;
   psp->getBufferAge = NULL;
   psp->bindTexImage = dri2_bind_tex_image;
   psp->releaseTexImage = dri2_release_tex_image;

   psp->getDrawableMSC = dri2DrawableGetMSC;
   psp->waitForMSC = dri2WaitForMSC;
   psp->waitForSBC = dri2WaitForSBC;
   psp->setSwapInterval = dri2SetSwapInterval;
   psp->getSwapInterval = dri2GetSwapInterval;
   psp->maxSwapInterval = INT_MAX;

   __glXEnableDirectExtension(&psc->base, "GLX_OML_sync_control");
   __glXEnableDirectExtension(&psc->base, "GLX_SGI_video_sync");

   if (psc->config->base.version > 1 &&
          psc->config->configQuerys(psc->driScreen, "glx_extension_override",
                                    &tmp) == 0)
      __glXParseExtensionOverride(&psc->base, tmp);

   if (psc->config->base.version > 1 &&
          psc->config->configQuerys(psc->driScreen,
                                    "indirect_gl_extension_override",
                                    &tmp) == 0)
      __IndirectGlParseExtensionOverride(&psc->base, tmp);

   if (psc->config->base.version > 1) {
      uint8_t force = false;
      if (psc->config->configQueryb(psc->driScreen, "force_direct_glx_context",
                                    &force) == 0) {
         psc->base.force_direct_context = force;
      }

      uint8_t invalid_glx_destroy_window = false;
      if (psc->config->configQueryb(psc->driScreen,
                                    "allow_invalid_glx_destroy_window",
                                    &invalid_glx_destroy_window) == 0) {
         psc->base.allow_invalid_glx_destroy_window = invalid_glx_destroy_window;
      }
   }

   /* DRI2 supports SubBuffer through DRI2CopyRegion, so it's always
    * available.*/
   psp->copySubBuffer = dri2CopySubBuffer;
   __glXEnableDirectExtension(&psc->base, "GLX_MESA_copy_sub_buffer");

   free(deviceName);

   tmp = getenv("LIBGL_SHOW_FPS");
   psc->show_fps_interval = (tmp) ? atoi(tmp) : 0;
   if (psc->show_fps_interval < 0)
      psc->show_fps_interval = 0;

   InfoMessageF("Using DRI2 for screen %d\n", screen);

   return &psc->base;

handle_error:
   CriticalErrorMessageF("failed to load driver: %s\n", driverName);

   if (configs)
       glx_config_destroy_list(configs);
   if (visuals)
       glx_config_destroy_list(visuals);
   if (psc->driScreen)
       psc->core->destroyScreen(psc->driScreen);
   psc->driScreen = NULL;
   if (psc->fd >= 0)
      close(psc->fd);
   if (psc->driver)
      dlclose(psc->driver);

   free(deviceName);
   glx_screen_cleanup(&psc->base);
   free(psc);

   return NULL;
}

/* Called from __glXFreeDisplayPrivate.
 */
static void
dri2DestroyDisplay(__GLXDRIdisplay * dpy)
{
   struct dri2_display *pdp = (struct dri2_display *) dpy;

   __glxHashDestroy(pdp->dri2Hash);
   free(dpy);
}

_X_HIDDEN __GLXDRIdrawable *
dri2GetGlxDrawableFromXDrawableId(Display *dpy, XID id)
{
   struct glx_display *d = __glXInitialize(dpy);
   struct dri2_display *pdp = (struct dri2_display *) d->dri2Display;
   __GLXDRIdrawable *pdraw;

   if (__glxHashLookup(pdp->dri2Hash, id, (void *) &pdraw) == 0)
      return pdraw;

   return NULL;
}

/*
 * Allocate, initialize and return a __DRIdisplayPrivate object.
 * This is called from __glXInitialize() when we are given a new
 * display pointer.
 */
_X_HIDDEN __GLXDRIdisplay *
dri2CreateDisplay(Display * dpy)
{
   struct dri2_display *pdp;
   int eventBase, errorBase, i;
   int driMajor, driMinor;

   if (!DRI2QueryExtension(dpy, &eventBase, &errorBase))
      return NULL;

   pdp = malloc(sizeof *pdp);
   if (pdp == NULL)
      return NULL;

   if (!DRI2QueryVersion(dpy, &driMajor, &driMinor) ||
       driMinor < 3) {
      free(pdp);
      return NULL;
   }

   pdp->base.destroyDisplay = dri2DestroyDisplay;
   pdp->base.createScreen = dri2CreateScreen;

   i = 0;
   pdp->loader_extensions[i++] = &dri2LoaderExtension.base;
   pdp->loader_extensions[i++] = &dri2UseInvalidate.base;
   pdp->loader_extensions[i++] = &driBackgroundCallable.base;
   pdp->loader_extensions[i++] = NULL;

   pdp->dri2Hash = __glxHashCreate();
   if (pdp->dri2Hash == NULL) {
      free(pdp);
      return NULL;
   }

   return &pdp->base;
}

#endif /* GLX_DIRECT_RENDERING */
