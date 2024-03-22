/* -*- c++ -*- */
/*
 * Copyright © 2010 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef GLSL_LINKER_H
#define GLSL_LINKER_H

#include "linker_util.h"

struct gl_shader_program;
struct gl_shader;
struct gl_linked_shader;

extern bool
link_function_calls(gl_shader_program *prog, gl_linked_shader *main,
                    gl_shader **shader_list, unsigned num_shaders);

bool
validate_intrastage_arrays(struct gl_shader_program *prog,
                           ir_variable *const var,
                           ir_variable *const existing,
                           bool match_precision = true);

void
validate_intrastage_interface_blocks(struct gl_shader_program *prog,
                                     const gl_shader **shader_list,
                                     unsigned num_shaders);

void
validate_interstage_inout_blocks(struct gl_shader_program *prog,
                                 const gl_linked_shader *producer,
                                 const gl_linked_shader *consumer);

void
validate_interstage_uniform_blocks(struct gl_shader_program *prog,
                                   gl_linked_shader **stages);

extern struct gl_linked_shader *
link_intrastage_shaders(void *mem_ctx,
                        struct gl_context *ctx,
                        struct gl_shader_program *prog,
                        struct gl_shader **shader_list,
                        unsigned num_shaders,
                        bool allow_missing_main);

#endif /* GLSL_LINKER_H */
