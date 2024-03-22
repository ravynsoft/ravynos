/*
 * Copyright © 2014 Broadcom
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef NIR_TO_RC_H
#define NIR_TO_RC_H

#include <stdbool.h>
#include "pipe/p_defines.h"

struct nir_shader;
struct pipe_screen;
struct pipe_shader_state;

struct nir_to_rc_options {
   bool lower_cmp;
   /* Emit MAX(a,-a) instead of abs src modifier) */
   bool lower_fabs;
   bool unoptimized_ra;
   bool lower_ssbo_bindings;
   uint32_t ubo_vec4_max;
};

const void *nir_to_rc(struct nir_shader *s,
                        struct pipe_screen *screen);

const void *nir_to_rc_options(struct nir_shader *s,
                                struct pipe_screen *screen,
                                const struct nir_to_rc_options *ntr_options);

#endif /* NIR_TO_RC_H */
