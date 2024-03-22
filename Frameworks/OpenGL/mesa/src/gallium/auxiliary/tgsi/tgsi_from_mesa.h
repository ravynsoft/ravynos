/*
 * Copyright 2017 Advanced Micro Devices, Inc.
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

#ifndef TGSI_FROM_MESA_H
#define TGSI_FROM_MESA_H

#include <stdbool.h>

#include "util/compiler.h"
#include "pipe/p_defines.h"
#include "pipe/p_shader_tokens.h"

#include "compiler/shader_enums.h"

#ifdef __cplusplus
extern "C" {
#endif


void
tgsi_get_gl_varying_semantic(gl_varying_slot attr,
                             bool needs_texcoord_semantic,
                             unsigned *semantic_name,
                             unsigned *semantic_index);

unsigned
tgsi_get_generic_gl_varying_index(gl_varying_slot attr,
                                  bool needs_texcoord_semantic);

void
tgsi_get_gl_frag_result_semantic(gl_frag_result frag_result,
                                 unsigned *semantic_name,
                                 unsigned *semantic_index);

enum tgsi_semantic
tgsi_get_sysval_semantic(unsigned sysval);

enum tgsi_interpolate_mode
tgsi_get_interp_mode(enum glsl_interp_mode mode, bool color);

static inline enum pipe_shader_type
pipe_shader_type_from_mesa(gl_shader_stage stage)
{
   STATIC_ASSERT((enum pipe_shader_type) MESA_SHADER_VERTEX == PIPE_SHADER_VERTEX);
   STATIC_ASSERT((enum pipe_shader_type) MESA_SHADER_FRAGMENT == PIPE_SHADER_FRAGMENT);
   STATIC_ASSERT((enum pipe_shader_type) MESA_SHADER_TESS_CTRL == PIPE_SHADER_TESS_CTRL);
   STATIC_ASSERT((enum pipe_shader_type) MESA_SHADER_TESS_EVAL == PIPE_SHADER_TESS_EVAL);
   STATIC_ASSERT((enum pipe_shader_type) MESA_SHADER_GEOMETRY == PIPE_SHADER_GEOMETRY);
   STATIC_ASSERT((enum pipe_shader_type) MESA_SHADER_COMPUTE == PIPE_SHADER_COMPUTE);
   return (enum pipe_shader_type)stage;
}

static inline gl_shader_stage
tgsi_processor_to_shader_stage(unsigned processor)
{
   return (gl_shader_stage)processor;
}

#ifdef __cplusplus
}
#endif

#endif /* TGSI_FROM_MESA_H */
