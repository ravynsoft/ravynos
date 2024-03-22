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

#ifndef DRI2_PRIV_H
#define DRI2_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "GL/internal/mesa_interface.h"

struct dri2_screen {
   struct glx_screen base;

   __DRIscreen *driScreen;
   __GLXDRIscreen vtable;
   const __DRIdri2Extension *dri2;
   const __DRIcoreExtension *core;
   const __DRImesaCoreExtension *mesa;

   const __DRI2flushExtension *f;
   const __DRI2configQueryExtension *config;
   const __DRItexBufferExtension *texBuffer;
   const __DRI2throttleExtension *throttle;
   const __DRI2rendererQueryExtension *rendererQuery;
   const __DRI2interopExtension *interop;
   const __DRIconfig **driver_configs;

   void *driver;
   char *driverName;
   int fd;

   int show_fps_interval;
};

_X_HIDDEN int
dri2_query_renderer_integer(struct glx_screen *base, int attribute,
                            unsigned int *value);

_X_HIDDEN int
dri2_query_renderer_string(struct glx_screen *base, int attribute,
                           const char **value);

_X_HIDDEN int
dri2_interop_query_device_info(struct glx_context *ctx,
                               struct mesa_glinterop_device_info *out);

_X_HIDDEN int
dri2_interop_export_object(struct glx_context *ctx,
                           struct mesa_glinterop_export_in *in,
                           struct mesa_glinterop_export_out *out);

_X_HIDDEN int
dri2_interop_flush_objects(struct glx_context *ctx,
                           unsigned count, struct mesa_glinterop_export_in *objects,
                           struct mesa_glinterop_flush_out *out);

#ifdef __cplusplus
}
#endif

#endif
