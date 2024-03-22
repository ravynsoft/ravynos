/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
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

/* Authors:  Keith Whitwell <keithw@vmware.com>
 */

#ifndef I915_STATE_H
#define I915_STATE_H

struct i915_context;

struct i915_tracked_state {
   const char *name;
   void (*update)(struct i915_context *);
   unsigned dirty;
};

extern struct i915_tracked_state i915_update_vertex_layout;

extern struct i915_tracked_state i915_hw_samplers;
extern struct i915_tracked_state i915_hw_sampler_views;
extern struct i915_tracked_state i915_hw_immediate;
extern struct i915_tracked_state i915_hw_dynamic;
extern struct i915_tracked_state i915_hw_fs;
extern struct i915_tracked_state i915_hw_framebuffer;
extern struct i915_tracked_state i915_hw_dst_buf_vars;
extern struct i915_tracked_state i915_hw_constants;

void i915_update_derived(struct i915_context *i915);
void i915_emit_hardware_state(struct i915_context *i915);
struct pipe_sampler_view *i915_create_sampler_view_custom(
   struct pipe_context *pipe, struct pipe_resource *texture,
   const struct pipe_sampler_view *templ, unsigned width0, unsigned height0);

#endif
