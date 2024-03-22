/*
 * Copyright © 2016 Red Hat
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
 */

#ifndef ST_NIR_H
#define ST_NIR_H

#include "st_context.h"
#include "compiler/shader_enums.h"
#include "util/perf/cpu_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

struct nir_shader;
struct nir_variable;

void st_nir_lower_builtin(struct nir_shader *shader);
void st_nir_lower_tex_src_plane(struct nir_shader *shader, unsigned free_slots,
                                unsigned lower_2plane, unsigned lower_3plane);

void st_nir_lower_wpos_ytransform(struct nir_shader *nir,
                                  struct gl_program *prog,
                                  struct pipe_screen *pscreen);

char *st_finalize_nir(struct st_context *st, struct gl_program *prog,
                      struct gl_shader_program *shader_program,
                      struct nir_shader *nir, bool finalize_by_driver,
                      bool is_before_variants);

void st_nir_assign_vs_in_locations(struct nir_shader *nir);
void st_nir_assign_varying_locations(struct st_context *st,
                                     struct nir_shader *nir);

void st_nir_lower_samplers(struct pipe_screen *screen, struct nir_shader *nir,
                           struct gl_shader_program *shader_program,
                           struct gl_program *prog);
void st_nir_lower_uniforms(struct st_context *st, struct nir_shader *nir);

void
st_nir_finish_builtin_nir(struct st_context *st, struct nir_shader *nir);
struct pipe_shader_state *
st_nir_finish_builtin_shader(struct st_context *st,
                             struct nir_shader *nir);

struct pipe_shader_state *
st_nir_make_passthrough_shader(struct st_context *st,
                               const char *shader_name,
                               gl_shader_stage stage,
                               unsigned num_vars,
                               const unsigned *input_locations,
                               const gl_varying_slot *output_locations,
                               unsigned *interpolation_modes,
                               unsigned sysval_mask);

struct pipe_shader_state *
st_nir_make_clearcolor_shader(struct st_context *st);

struct nir_variable *
st_nir_state_variable_create(struct nir_shader *shader,
                             const struct glsl_type *type,
                             const gl_state_index16 state[STATE_LENGTH]);

bool st_nir_lower_fog(struct nir_shader *s, enum gl_fog_mode fog_mode,
                      struct gl_program_parameter_list *paramList);
bool st_nir_lower_position_invariant(struct nir_shader *s, bool aos,
                                     struct gl_program_parameter_list *paramList);

#ifdef __cplusplus
}
#endif

#endif /* ST_NIR_H */
