/**************************************************************************
 *
 * Copyright (C) 2009 VMware, Inc.
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

#ifndef DRI_CONTEXT_H
#define DRI_CONTEXT_H

#include "dri_util.h"
#include "util/compiler.h"
#include "hud/hud_context.h"

struct pipe_context;
struct pipe_fence;
struct st_context;
struct dri_drawable;
struct dri_screen;

struct dri_context
{
   /* dri */
   struct dri_screen *screen;

   /**
    * Pointer to drawable currently bound to this context for drawing.
    */
   struct dri_drawable *draw;

   /**
    * Pointer to drawable currently bound to this context for reading.
    */
   struct dri_drawable *read;

   /**
    * True if the dri_drawable's current __DRIimageBufferMask is
    * __DRI_IMAGE_BUFFER_SHARED.
    */
   bool is_shared_buffer_bound;

   /**
    * The loaders's private context data.  This structure is opaque.
    */
   void *loaderPrivate;

   struct {
       int draw_stamp;
       int read_stamp;
   } dri2;

   /* gallium */
   struct st_context *st;
   struct pp_queue_t *pp;
   struct hud_context *hud;
};

static inline struct dri_context *
dri_context(__DRIcontext *driContextPriv)
{
   return (struct dri_context *)driContextPriv;
}

static inline __DRIcontext *
opaque_dri_context(struct dri_context *ctx)
{
   return (__DRIcontext *)ctx;
}

/***********************************************************************
 * dri_context.c
 */
void dri_destroy_context(struct dri_context *ctx);

bool
dri_unbind_context(struct dri_context *ctx);

bool
dri_make_current(struct dri_context *ctx,
                 struct dri_drawable *draw,
		 struct dri_drawable *read);

struct dri_context *
dri_get_current(void);

struct dri_context *
dri_create_context(struct dri_screen *screen,
                   gl_api api, const struct gl_config *visual,
                   const struct __DriverContextConfig *ctx_config,
                   unsigned *error,
                   struct dri_context *sharedContextPrivate,
                   void *loaderPrivate);

#endif

/* vim: set sw=3 ts=8 sts=3 expandtab: */
