/*
 * Copyright © 2012 Rob Clark <robclark@freedesktop.org>
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

#ifndef DISASM_H_
#define DISASM_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "compiler/shader_enums.h"

/* bitmask of debug flags */
enum debug_t {
   PRINT_RAW = 0x1, /* dump raw hexdump */
   PRINT_VERBOSE = 0x2,
   PRINT_STATS = 0x4,
   EXPAND_REPEAT = 0x8,
};

struct shader_stats {
   /* instructions counts rpnN, and instlen does not */
   int instructions, instlen;
   int nops;
   int ss, sy;
   int constlen;
   int halfreg;
   int fullreg;
   uint16_t sstall;
   uint16_t mov_count;
   uint16_t cov_count;
   uint16_t last_baryf;
   uint16_t instrs_per_cat[8];
};

int disasm_a2xx(uint32_t *dwords, int sizedwords, int level,
                gl_shader_stage type);
int disasm_a3xx(uint32_t *dwords, int sizedwords, int level, FILE *out,
                unsigned gpu_id);
int disasm_a3xx_stat(uint32_t *dwords, int sizedwords, int level, FILE *out,
                     unsigned gpu_id, struct shader_stats *stats);
int try_disasm_a3xx(uint32_t *dwords, int sizedwords, int level, FILE *out,
                    unsigned gpu_id);

void disasm_a2xx_set_debug(enum debug_t debug);
void disasm_a3xx_set_debug(enum debug_t debug);

#endif /* DISASM_H_ */
