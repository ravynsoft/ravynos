/*
 * Copyright © 2017 Red Hat
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
 */

#ifndef _U_TRANSFER_HELPER_H
#define _U_TRANSFER_HELPER_H

#include "pipe/p_state.h"
#include "pipe/p_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A helper to implement various "lowering" for transfers:
 *
 *  - exposing separate depth and stencil resources as packed depth-stencil
 *  - MSAA resolves
 *
 * To use this, drivers should:
 *
 *  1) populate u_transfer_vtbl and plug that into pipe_screen::transfer_helper
 *  2) plug the transfer helpers into pipe_screen/pipe_context
 *
 * To avoid subclassing pipe_resource (and conflicting with threaded_context)
 * the vtbl contains setter/getter methods used for separate_stencil to access
 * the internal_format and separate stencil buffer.
 */

struct u_transfer_vtbl {
   /* NOTE I am not expecting resource_create_from_handle() or
    * resource_create_with_modifiers() paths to be creating any
    * resources that need special handling.  Otherwise they would
    * need to be wrapped too.
    */
   struct pipe_resource * (*resource_create)(struct pipe_screen *pscreen,
                                             const struct pipe_resource *templ);

   void (*resource_destroy)(struct pipe_screen *pscreen,
                            struct pipe_resource *prsc);

   void *(*transfer_map)(struct pipe_context *pctx,
                         struct pipe_resource *prsc,
                         unsigned level,
                         unsigned usage,
                         const struct pipe_box *box,
                         struct pipe_transfer **pptrans);


   void (*transfer_flush_region)(struct pipe_context *pctx,
                                 struct pipe_transfer *ptrans,
                                 const struct pipe_box *box);

   void (*transfer_unmap)(struct pipe_context *pctx,
                          struct pipe_transfer *ptrans);

   /*
    * auxiliary methods to access internal format, stencil:
    */

   /**
    * Must be implemented if separate stencil is used.  The internal_format
    * is the format the resource was created with.  In the case of separate
    * stencil, prsc->format is set bac to the gallium-frontend-visible format
    * (e.g. Z32_FLOAT_S8X24_UINT) after the resource is created.
    */
   enum pipe_format (*get_internal_format)(struct pipe_resource *prsc);

   /**
    * Must be implemented if separate stencil is lowered.  Used to set/get
    * the separate s8 stencil buffer.
    *
    * These two do not get/put references to the pipe_resource.  The
    * stencil resource will be destroyed by u_transfer_helper_resource_destroy().
    */
   void (*set_stencil)(struct pipe_resource *prsc, struct pipe_resource *stencil);
   struct pipe_resource *(*get_stencil)(struct pipe_resource *prsc);
};

enum u_transfer_helper_flags {
   U_TRANSFER_HELPER_SEPARATE_Z32S8 = (1 << 0),
   U_TRANSFER_HELPER_SEPARATE_STENCIL = (1 << 1),
   U_TRANSFER_HELPER_MSAA_MAP = (1 << 3),
   U_TRANSFER_HELPER_Z24_IN_Z32F = (1 << 4),
   U_TRANSFER_HELPER_INTERLEAVE_IN_PLACE = (1 << 5),
};

struct pipe_resource *u_transfer_helper_resource_create(
      struct pipe_screen *pscreen, const struct pipe_resource *templ);

void u_transfer_helper_resource_destroy(struct pipe_screen *pscreen,
                                        struct pipe_resource *prsc);

void *u_transfer_helper_transfer_map(struct pipe_context *pctx,
                                     struct pipe_resource *prsc,
                                     unsigned level,
                                     unsigned usage,
                                     const struct pipe_box *box,
                                     struct pipe_transfer **pptrans);


void u_transfer_helper_transfer_flush_region(struct pipe_context *pctx,
                                             struct pipe_transfer *ptrans,
                                             const struct pipe_box *box);

void u_transfer_helper_transfer_unmap(struct pipe_context *pctx,
                                      struct pipe_transfer *ptrans);

struct u_transfer_helper;

struct u_transfer_helper *
u_transfer_helper_create(const struct u_transfer_vtbl *vtbl,
                         enum u_transfer_helper_flags flags);

void u_transfer_helper_destroy(struct u_transfer_helper *helper);

#ifdef __cplusplus
} // extern "C" {
#endif

#endif /* _U_TRANSFER_HELPER_H */
