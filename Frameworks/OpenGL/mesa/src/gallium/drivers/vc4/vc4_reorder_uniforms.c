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

/**
 * @file vc4_reorder_uniforms.c
 *
 * After optimization has occurred, rewrites the shader to have uniform reads
 * reading from the c->uniform_contents[] in order, exactly once each.
 *
 * This allows optimization and instruction scheduling to move things around
 * without worrying about how the hardware has the "each uniform read bumps
 * the uniform read address" property.
 */

#include "util/ralloc.h"
#include "util/u_math.h"
#include "vc4_qir.h"

void
qir_reorder_uniforms(struct vc4_compile *c)
{
        uint32_t *uniform_index = NULL;
        uint32_t uniform_index_size = 0;
        uint32_t next_uniform = 0;

        qir_for_each_inst_inorder(inst, c) {
                uint32_t new = ~0;

                for (int i = 0; i < qir_get_nsrc(inst); i++) {
                        if (inst->src[i].file != QFILE_UNIF)
                                continue;

                        if (new == ~0) {
                                new = next_uniform++;
                                if (uniform_index_size <= new) {
                                        uniform_index_size =
                                                MAX2(uniform_index_size * 2, 16);
                                        uniform_index =
                                                realloc(uniform_index,
                                                        uniform_index_size *
                                                        sizeof(uint32_t));
                                }
                        } else {
                                /* If we've got two uniform references in this
                                 * instruction, they need to be the same
                                 * uniform value.
                                 */
                                assert(inst->src[i].index == uniform_index[new]);
                        }

                        uniform_index[new] = inst->src[i].index;
                        inst->src[i].index = new;
                }
        }

        uint32_t *uniform_data = ralloc_array(c, uint32_t, next_uniform);
        enum quniform_contents *uniform_contents =
                ralloc_array(c, enum quniform_contents, next_uniform);

        for (uint32_t i = 0; i < next_uniform; i++) {
                uniform_data[i] = c->uniform_data[uniform_index[i]];
                uniform_contents[i] = c->uniform_contents[uniform_index[i]];
        }

        ralloc_free(c->uniform_data);
        c->uniform_data = uniform_data;
        ralloc_free(c->uniform_contents);
        c->uniform_contents = uniform_contents;
        c->num_uniforms = next_uniform;

        free(uniform_index);
}
