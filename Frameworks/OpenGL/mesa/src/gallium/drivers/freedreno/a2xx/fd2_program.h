/*
 * Copyright (C) 2012-2013 Rob Clark <robclark@freedesktop.org>
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
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FD2_PROGRAM_H_
#define FD2_PROGRAM_H_

#include "pipe/p_context.h"

#include "freedreno_context.h"

#include "disasm.h"
#include "ir2.h"

struct fd2_shader_stateobj {
   nir_shader *nir;
   gl_shader_stage type;
   bool is_a20x;

   /* note: using same set of immediates for all variants
    * it doesn't matter, other than the slightly larger command stream
    */
   unsigned first_immediate; /* const reg # of first immediate */
   unsigned num_immediates;
   struct {
      uint32_t val[4];
      unsigned ncomp;
   } immediates[64];

   bool writes_psize;
   bool need_param;
   bool has_kill;

   /* note:
    * fragment shader only has one variant
    * first vertex shader variant is always binning shader
    * we should use a dynamic array but in normal case there is
    * only 2 variants (and 3 sometimes with GALLIUM_HUD)
    */
   struct ir2_shader_variant variant[8];
};

void fd2_program_emit(struct fd_context *ctx, struct fd_ringbuffer *ring,
                      struct fd_program_stateobj *prog) assert_dt;

void fd2_prog_init(struct pipe_context *pctx);

#endif /* FD2_PROGRAM_H_ */
