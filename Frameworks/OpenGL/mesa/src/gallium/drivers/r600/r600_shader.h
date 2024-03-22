/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
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
#ifndef R600_SHADER_H
#define R600_SHADER_H

#include "r600_pipe.h"
#include "r600_shader_common.h"

#include <assert.h>


#ifdef __cplusplus
extern "C" {
#endif

static_assert(
	R600_SHADER_MAX_INPUTS >= PIPE_MAX_SHADER_INPUTS,
	"Assuming that all Gallium shader inputs can fit into r600_shader inputs");
static_assert(
	R600_SHADER_MAX_OUTPUTS >= PIPE_MAX_SHADER_OUTPUTS,
	"Assuming that all Gallium shader outputs can fit into r600_shader outputs");

struct r600_pipe_shader {
	struct r600_pipe_shader_selector *selector;
	struct r600_pipe_shader	*next_variant;
	/* for GS - corresponding copy shader (installed as VS) */
	struct r600_pipe_shader *gs_copy_shader;
	struct r600_shader	shader;
	struct r600_command_buffer command_buffer; /* register writes */
	struct r600_resource	*bo;
	unsigned		sprite_coord_enable;
	unsigned		flatshade;
	unsigned		msaa;
	unsigned		pa_cl_vs_out_cntl;
	unsigned		nr_ps_color_outputs;
	unsigned                ps_color_export_mask;
	
	union r600_shader_key	key;
	unsigned		db_shader_control;
	unsigned		ps_depth_export;
	unsigned		enabled_stream_buffers_mask;
	unsigned		scratch_space_needed; /* size of scratch space (if > 0) counted in vec4 */
};

void *r600_create_vertex_fetch_shader(struct pipe_context *ctx,
				      unsigned count,
				      const struct pipe_vertex_element *elements);

/* return the table index 0-5 for TGSI_INTERPOLATE_LINEAR/PERSPECTIVE and
 TGSI_INTERPOLATE_LOC_CENTER/SAMPLE/COUNT. Other input values return -1. */
int eg_get_interpolator_index(unsigned interpolate, unsigned location);

int r600_get_lds_unique_index(unsigned semantic_name, unsigned index);

int generate_gs_copy_shader(struct r600_context *rctx,
                            struct r600_pipe_shader *gs,
                            struct pipe_stream_output_info *so);

#ifdef __cplusplus
}  // extern "C"
#endif


#endif
