/*
 * Copyright © 2013 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)

#include "glxclient.h"
#include "glx_error.h"
#include "GL/internal/dri_interface.h"
#include "dri2_priv.h"
#if defined(HAVE_DRI3)
#include "dri3_priv.h"
#endif
#include "GL/mesa_glinterop.h"

_X_HIDDEN int
dri2_interop_query_device_info(struct glx_context *ctx,
                               struct mesa_glinterop_device_info *out)
{
   struct dri2_screen *psc = (struct dri2_screen*)ctx->psc;

   if (!psc->interop)
      return MESA_GLINTEROP_UNSUPPORTED;

   return psc->interop->query_device_info(ctx->driContext, out);
}

_X_HIDDEN int
dri2_interop_export_object(struct glx_context *ctx,
                           struct mesa_glinterop_export_in *in,
                           struct mesa_glinterop_export_out *out)
{
   struct dri2_screen *psc = (struct dri2_screen*)ctx->psc;

   if (!psc->interop)
      return MESA_GLINTEROP_UNSUPPORTED;

   return psc->interop->export_object(ctx->driContext, in, out);
}

_X_HIDDEN int
dri2_interop_flush_objects(struct glx_context *ctx,
                           unsigned count, struct mesa_glinterop_export_in *objects,
                           struct mesa_glinterop_flush_out *out)
{
   struct dri2_screen *psc = (struct dri2_screen*)ctx->psc;

   if (!psc->interop || psc->interop->base.version < 2)
      return MESA_GLINTEROP_UNSUPPORTED;

   return psc->interop->flush_objects(ctx->driContext, count, objects, out);
}

#if defined(HAVE_DRI3)

_X_HIDDEN int
dri3_interop_query_device_info(struct glx_context *ctx,
                               struct mesa_glinterop_device_info *out)
{
   struct dri3_screen *psc = (struct dri3_screen*)ctx->psc;

   if (!psc->interop)
      return MESA_GLINTEROP_UNSUPPORTED;

   return psc->interop->query_device_info(ctx->driContext, out);
}

_X_HIDDEN int
dri3_interop_export_object(struct glx_context *ctx,
                           struct mesa_glinterop_export_in *in,
                           struct mesa_glinterop_export_out *out)
{
   struct dri3_screen *psc = (struct dri3_screen*)ctx->psc;

   if (!psc->interop)
      return MESA_GLINTEROP_UNSUPPORTED;

   return psc->interop->export_object(ctx->driContext, in, out);
}

_X_HIDDEN int
dri3_interop_flush_objects(struct glx_context *ctx,
                           unsigned count, struct mesa_glinterop_export_in *objects,
                           struct mesa_glinterop_flush_out *out)
{
   struct dri3_screen *psc = (struct dri3_screen*)ctx->psc;

   if (!psc->interop || psc->interop->base.version < 2)
      return MESA_GLINTEROP_UNSUPPORTED;

   return psc->interop->flush_objects(ctx->driContext, count, objects, out);
}

#endif /* HAVE_DRI3 */
#endif /* defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL) */
