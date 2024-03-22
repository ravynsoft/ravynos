/*
 * Copyright 2010 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Dave Airlie
 */

#include "r300_context.h"
#include "r300_texture.h"
#include "r300_transfer.h"
#include "r300_screen_buffer.h"

static struct pipe_resource *
r300_resource_create(struct pipe_screen *screen,
                    const struct pipe_resource *templ)
{
   if (templ->target == PIPE_BUFFER)
      return r300_buffer_create(screen, templ);
   else
      return r300_texture_create(screen, templ);

}

void r300_init_resource_functions(struct r300_context *r300)
{
   r300->context.buffer_map = r300_buffer_transfer_map;
   r300->context.texture_map = r300_texture_transfer_map;
   r300->context.transfer_flush_region = u_default_transfer_flush_region;
   r300->context.buffer_unmap = r300_buffer_transfer_unmap;
   r300->context.texture_unmap = r300_texture_transfer_unmap;
   r300->context.buffer_subdata = u_default_buffer_subdata;
   r300->context.texture_subdata = u_default_texture_subdata;
   r300->context.create_surface = r300_create_surface;
   r300->context.surface_destroy = r300_surface_destroy;
}

void r300_init_screen_resource_functions(struct r300_screen *r300screen)
{
   r300screen->screen.resource_create = r300_resource_create;
   r300screen->screen.resource_from_handle = r300_texture_from_handle;
   r300screen->screen.resource_get_handle = r300_resource_get_handle;
   r300screen->screen.resource_destroy = r300_resource_destroy;
}
