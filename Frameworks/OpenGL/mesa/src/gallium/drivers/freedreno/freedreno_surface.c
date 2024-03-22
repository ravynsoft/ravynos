/*
 * Copyright (C) 2012 Rob Clark <robclark@freedesktop.org>
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

#include "freedreno_surface.h"
#include "freedreno_resource.h"
#include "freedreno_util.h"

#include "util/u_inlines.h"
#include "util/u_memory.h"

struct pipe_surface *
fd_create_surface(struct pipe_context *pctx, struct pipe_resource *ptex,
                  const struct pipe_surface *surf_tmpl)
{
   struct fd_surface *surface = CALLOC_STRUCT(fd_surface);

   if (!surface)
      return NULL;

   struct pipe_surface *psurf = &surface->base;
   unsigned level = surf_tmpl->u.tex.level;

   pipe_reference_init(&psurf->reference, 1);
   pipe_resource_reference(&psurf->texture, ptex);

   psurf->context = pctx;
   psurf->format = surf_tmpl->format;
   psurf->width = u_minify(ptex->width0, level);
   psurf->height = u_minify(ptex->height0, level);
   psurf->nr_samples = surf_tmpl->nr_samples;

   if (ptex->target == PIPE_BUFFER) {
      psurf->u.buf.first_element = surf_tmpl->u.buf.first_element;
      psurf->u.buf.last_element = surf_tmpl->u.buf.last_element;
   } else {
      psurf->u.tex.level = level;
      psurf->u.tex.first_layer = surf_tmpl->u.tex.first_layer;
      psurf->u.tex.last_layer = surf_tmpl->u.tex.last_layer;
   }

   return &surface->base;
}

void
fd_surface_destroy(struct pipe_context *pctx, struct pipe_surface *psurf)
{
   pipe_resource_reference(&psurf->texture, NULL);
   FREE(psurf);
}
