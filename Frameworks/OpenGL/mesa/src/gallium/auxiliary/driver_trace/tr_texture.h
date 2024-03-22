/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
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

#ifndef TR_TEXTURE_H_
#define TR_TEXTURE_H_


#include "util/compiler.h"
#include "pipe/p_state.h"

#include "tr_screen.h"
#include "util/u_threaded_context.h"

struct trace_context;


struct tr_list
{
   struct tr_list *next;
   struct tr_list *prev;
};

struct trace_surface
{
   struct pipe_surface base;

   struct pipe_surface *surface;

   struct tr_list list;
};


struct trace_sampler_view
{
   struct pipe_sampler_view base;
   unsigned refcount;

   struct pipe_sampler_view *sampler_view;
};


struct trace_transfer
{
   struct threaded_transfer base;

   struct pipe_transfer *transfer;
   struct pipe_context *pipe;

   struct tr_list list;

   void *map;
};


static inline struct trace_surface *
trace_surface(struct pipe_surface *surface)
{
   if (!surface)
      return NULL;
   return (struct trace_surface *)surface;
}


static inline struct trace_sampler_view *
trace_sampler_view(struct pipe_sampler_view *sampler_view)
{
   if (!sampler_view)
      return NULL;
   return (struct trace_sampler_view *)sampler_view;
}


static inline struct trace_transfer *
trace_transfer(struct pipe_transfer *transfer)
{
   if (!transfer)
      return NULL;
   return (struct trace_transfer *)transfer;
}


struct pipe_surface *
trace_surf_create(struct trace_context *tr_ctx,
                  struct pipe_resource *tr_res,
                  struct pipe_surface *surface);

void
trace_surf_destroy(struct trace_surface *tr_surf);

struct pipe_transfer *
trace_transfer_create(struct trace_context *tr_ctx,
		      struct pipe_resource *tr_res,
		      struct pipe_transfer *transfer);

void
trace_transfer_destroy(struct trace_context *tr_ctx,
                       struct trace_transfer *tr_trans);

struct pipe_sampler_view *
trace_sampler_view_create(struct trace_context *tr_ctx,
                  struct pipe_resource *tr_res,
                  struct pipe_sampler_view *sampler_view);

void
trace_sampler_view_destroy(struct trace_sampler_view *tr_view);

struct pipe_sampler_view *
trace_sampler_view_unwrap(struct trace_sampler_view *tr_view);

#endif /* TR_TEXTURE_H_ */
