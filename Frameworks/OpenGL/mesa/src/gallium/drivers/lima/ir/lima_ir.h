/*
 * Copyright (c) 2017 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef LIMA_IR_H
#define LIMA_IR_H

#include <stdio.h>
#include <lima_screen.h>

#include "nir.h"

#define gpir_debug(...)                \
   do {                                \
      if (lima_debug & LIMA_DEBUG_GP)  \
         printf("gpir: " __VA_ARGS__); \
   } while (0)

#define gpir_error(...) \
   fprintf(stderr, "gpir: " __VA_ARGS__)

#define ppir_debug(...)                \
   do {                                \
      if (lima_debug & LIMA_DEBUG_PP)  \
         printf("ppir: " __VA_ARGS__); \
   } while (0)

#define ppir_error(...) \
   fprintf(stderr, "ppir: " __VA_ARGS__)


struct ra_regs;
struct lima_vs_compiled_shader;
struct lima_fs_compiled_shader;

/* gpir interface */
bool gpir_compile_nir(struct lima_vs_compiled_shader *prog, struct nir_shader *nir,
                      struct util_debug_callback *debug);


/* ppir interface */
bool ppir_compile_nir(struct lima_fs_compiled_shader *prog, struct nir_shader *nir,
                      struct ra_regs *ra,
                      struct util_debug_callback *debug);
struct ra_regs *ppir_regalloc_init(void *mem_ctx);

void lima_nir_lower_uniform_to_scalar(nir_shader *shader);
bool lima_nir_scale_trig(nir_shader *shader);
bool lima_nir_lower_ftrunc(nir_shader *shader);
bool lima_nir_split_load_input(nir_shader *shader);
bool lima_nir_split_loads(nir_shader *shader);

void lima_nir_duplicate_load_consts(nir_shader *shader);
void lima_nir_duplicate_load_inputs(nir_shader *shader);
void lima_nir_duplicate_load_uniforms(nir_shader *shader);

bool lima_nir_lower_txp(nir_shader *shader);

#endif
