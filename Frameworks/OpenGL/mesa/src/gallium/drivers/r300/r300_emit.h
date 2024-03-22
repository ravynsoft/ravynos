/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef R300_EMIT_H
#define R300_EMIT_H

#include "r300_context.h"

struct rX00_fragment_program_code;
struct r300_vertex_program_code;

uint32_t pack_float24(float f);

void r300_emit_vertex_arrays(struct r300_context* r300, int offset,
                             bool indexed, int instance_id);

void r300_emit_blend_state(struct r300_context* r300,
                           unsigned size, void* state);

void r300_emit_blend_color_state(struct r300_context* r300,
                                 unsigned size, void* state);

void r300_emit_clip_state(struct r300_context* r300,
                          unsigned size, void* state);

void r300_emit_dsa_state(struct r300_context* r300,
                         unsigned size, void* state);

void r300_emit_hyperz_state(struct r300_context *r300,
                            unsigned size, void *state);

void r300_emit_hyperz_end(struct r300_context *r300);

void r300_emit_fs(struct r300_context* r300, unsigned size, void *state);

void r300_emit_fs_constants(struct r300_context* r300, unsigned size, void *state);

void r300_emit_fs_rc_constant_state(struct r300_context* r300, unsigned size, void *state);

void r500_emit_fs(struct r300_context* r300, unsigned size, void *state);

void r500_emit_fs_constants(struct r300_context* r300, unsigned size, void *state);

void r500_emit_fs_rc_constant_state(struct r300_context* r300, unsigned size, void *state);

void r300_emit_fb_state(struct r300_context* r300, unsigned size, void* state);

void r300_emit_fb_state_pipelined(struct r300_context *r300,
                                  unsigned size, void *state);

void r300_emit_gpu_flush(struct r300_context *r300, unsigned size, void *state);

void r300_emit_aa_state(struct r300_context *r300, unsigned size, void *state);

void r300_emit_query_start(struct r300_context *r300, unsigned size, void *state);

void r300_emit_query_end(struct r300_context* r300);

void r300_emit_rs_state(struct r300_context* r300, unsigned size, void* state);

void r300_emit_rs_block_state(struct r300_context* r300,
                              unsigned size, void* state);

void r300_emit_sample_mask(struct r300_context *r300,
                           unsigned size, void *state);

void r300_emit_scissor_state(struct r300_context* r300,
                             unsigned size, void* state);

void r300_emit_textures_state(struct r300_context *r300,
                              unsigned size, void *state);

void r300_emit_vertex_arrays_swtcl(struct r300_context *r300, bool indexed);

void r300_emit_vap_invariant_state(struct r300_context *r300,
                                   unsigned size, void *state);

void r300_emit_vertex_stream_state(struct r300_context* r300,
                                   unsigned size, void* state);

void r300_emit_vs_constants(struct r300_context* r300,
                            unsigned size, void *state);

void r300_emit_vs_state(struct r300_context* r300, unsigned size, void* state);

void r300_emit_viewport_state(struct r300_context* r300,
                              unsigned size, void* state);

void r300_emit_ztop_state(struct r300_context* r300,
                          unsigned size, void* state);

void r300_emit_pvs_flush(struct r300_context* r300, unsigned size, void* state);

void r300_emit_texture_cache_inval(struct r300_context* r300, unsigned size, void* state);

void r300_emit_invariant_state(struct r300_context *r300,
                               unsigned size, void *state);

void r300_emit_hiz_clear(struct r300_context *r300, unsigned size, void *state);
void r300_emit_zmask_clear(struct r300_context *r300, unsigned size, void *state);
void r300_emit_cmask_clear(struct r300_context *r300, unsigned size, void *state);

unsigned r300_get_num_dirty_dwords(struct r300_context *r300);
unsigned r300_get_num_cs_end_dwords(struct r300_context *r300);

/* Emit all dirty state. */
void r300_emit_dirty_state(struct r300_context* r300);

bool r300_emit_buffer_validate(struct r300_context *r300,
                               bool do_validate_vertex_buffers,
                               struct pipe_resource *index_buffer);

#endif /* R300_EMIT_H */
