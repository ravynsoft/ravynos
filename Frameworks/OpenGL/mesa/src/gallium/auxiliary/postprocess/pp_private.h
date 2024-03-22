/**************************************************************************
 *
 * Copyright 2011 Lauri Kasanen
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
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef PP_PRIVATE_H
#define PP_PRIVATE_H


#include "postprocess.h"
#include "cso_cache/cso_context.h"


/**
 * Internal control details.
 */
struct pp_program
{
   struct pipe_screen *screen;
   struct pipe_context *pipe;
   struct cso_context *cso;

   /* For notifying st_context to rebind states that we clobbered. */
   struct st_context *st;
   pp_st_invalidate_state_func st_invalidate_state;

   struct pipe_blend_state blend;
   struct pipe_depth_stencil_alpha_state depthstencil;
   struct pipe_rasterizer_state rasterizer;
   struct pipe_sampler_state sampler;   /* bilinear */
   struct pipe_sampler_state sampler_point;     /* point */
   struct pipe_viewport_state viewport;
   struct pipe_framebuffer_state framebuffer;
   struct cso_velems_state velem;

   union pipe_color_union clear_color;

   void *passvs;

   struct pipe_resource *vbuf;
   struct pipe_surface surf;
   struct pipe_sampler_view *view;
};



/**
 * The main post-processing queue.
 */
struct pp_queue_t
{
   pp_func *pp_queue;           /* An array of pp_funcs */
   unsigned int n_filters;      /* Number of enabled filters */

   struct pipe_resource *tmp[2];        /* Two temp FBOs for the queue */
   struct pipe_resource *inner_tmp[3];  /* Three for filter use */

   unsigned int n_tmp, n_inner_tmp;

   struct pipe_resource *depth; /* depth of original input */
   struct pipe_resource *stencil;       /* stencil shared by inner_tmps */
   struct pipe_resource *areamaptex;    /* MLAA area map texture */

   struct pipe_surface *tmps[2], *inner_tmps[3], *stencils;

   void ***shaders;             /* Shaders in TGSI form */
   unsigned int *filters;       /* Active filter to filters.h mapping. */
   struct pp_program *p;

   bool fbos_init;
};


void pp_free_fbos(struct pp_queue_t *);

void pp_debug(const char *, ...);

struct pp_program *pp_init_prog(struct pp_queue_t *, struct pipe_context *pipe,
                                struct cso_context *, struct st_context *st,
                                pp_st_invalidate_state_func st_invalidate_state);

void pp_blit(struct pipe_context *pipe,
             struct pipe_resource *src_tex,
             int srcX0, int srcY0,
             int srcX1, int srcY1,
             int srcZ0,
             struct pipe_surface *dst,
             int dstX0, int dstY0,
             int dstX1, int dstY1);


#endif /* PP_PRIVATE_H */
