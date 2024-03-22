/*
 * Copyright 2018 Collabora Ltd.
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
 */

#ifndef ZINK_RENDERPASS_H
#define ZINK_RENDERPASS_H

#include "zink_types.h"

struct zink_render_pass *
zink_create_render_pass(struct zink_screen *screen,
                        struct zink_render_pass_state *state,
                        struct zink_render_pass_pipeline_state *pstate);

void
zink_destroy_render_pass(struct zink_screen *screen,
                         struct zink_render_pass *rp);


unsigned
zink_begin_render_pass(struct zink_context *ctx);
void
zink_end_render_pass(struct zink_context *ctx);

VkImageLayout
zink_render_pass_attachment_get_barrier_info(const struct zink_rt_attrib *rt, bool color, VkPipelineStageFlags *pipeline, VkAccessFlags *access);
VkImageLayout
zink_tc_renderpass_info_parse(struct zink_context *ctx, const struct tc_renderpass_info *info, unsigned idx, VkPipelineStageFlags *pipeline, VkAccessFlags *access);
bool
zink_init_render_pass(struct zink_context *ctx);
bool
zink_render_update_swapchain(struct zink_context *ctx);
void
zink_render_fixup_swapchain(struct zink_context *ctx);
void
zink_init_zs_attachment(struct zink_context *ctx, struct zink_rt_attrib *rt);
void
zink_init_color_attachment(struct zink_context *ctx, unsigned i, struct zink_rt_attrib *rt);
void
zink_tc_init_zs_attachment(struct zink_context *ctx, const struct tc_renderpass_info *info, struct zink_rt_attrib *rt);
void
zink_tc_init_color_attachment(struct zink_context *ctx, const struct tc_renderpass_info *info, unsigned i, struct zink_rt_attrib *rt);
#endif
