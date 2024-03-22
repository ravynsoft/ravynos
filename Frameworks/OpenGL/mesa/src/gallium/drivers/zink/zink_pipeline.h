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

#ifndef ZINK_PIPELINE_H
#define ZINK_PIPELINE_H


#include "zink_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zink_gfx_output_key *
zink_find_or_create_output(struct zink_context *ctx);
struct zink_gfx_output_key *
zink_find_or_create_output_ds3(struct zink_context *ctx);
struct zink_gfx_input_key *
zink_find_or_create_input(struct zink_context *ctx, VkPrimitiveTopology vkmode);
struct zink_gfx_input_key *
zink_find_or_create_input_dynamic(struct zink_context *ctx, VkPrimitiveTopology vkmode);

VkPipeline
zink_create_gfx_pipeline(struct zink_screen *screen,
                         struct zink_gfx_program *prog,
                         struct zink_shader_object *objs,
                         struct zink_gfx_pipeline_state *state,
                         const uint8_t *binding_map,
                         VkPrimitiveTopology primitive_topology,
                         bool optimize,
                         struct util_dynarray *dgc);

VkPipeline
zink_create_compute_pipeline(struct zink_screen *screen, struct zink_compute_program *comp, struct zink_compute_pipeline_state *state);

VkPipeline
zink_create_gfx_pipeline_input(struct zink_screen *screen,
                               struct zink_gfx_pipeline_state *state,
                               const uint8_t *binding_map,
                               VkPrimitiveTopology primitive_topology);
VkPipeline
zink_create_gfx_pipeline_library(struct zink_screen *screen, struct zink_gfx_program *prog);
VkPipeline
zink_create_gfx_pipeline_output(struct zink_screen *screen, struct zink_gfx_pipeline_state *state);
VkPipeline
zink_create_gfx_pipeline_combined(struct zink_screen *screen, struct zink_gfx_program *prog, VkPipeline input, VkPipeline *library, unsigned libcount, VkPipeline output, bool optimized, bool testonly);
VkPipeline
zink_create_gfx_pipeline_separate(struct zink_screen *screen, struct zink_shader_object *objs, VkPipelineLayout layout, gl_shader_stage stage);
#ifdef __cplusplus
}
#endif
#endif
