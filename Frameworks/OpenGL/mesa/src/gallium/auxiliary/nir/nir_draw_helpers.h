/**************************************************************************
 *
 * Copyright 2019 Red Hat.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **************************************************************************/

#ifndef NIR_DRAW_HELPERS_H
#define NIR_DRAW_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif

struct nir_shader;
void
nir_lower_pstipple_fs(struct nir_shader *shader,
                      unsigned *samplerUnitOut,
                      unsigned fixedUnit,
                      bool fs_pos_is_sysval,
                      nir_alu_type bool_type);

void
nir_lower_aaline_fs(struct nir_shader *shader, int *varying,
                    nir_variable *stipple_counter,
                    nir_variable *stipple_pattern);

void
nir_lower_aapoint_fs(struct nir_shader *shader, int *varying,
                     nir_alu_type bool_type);

#ifdef __cplusplus
}
#endif

#endif
