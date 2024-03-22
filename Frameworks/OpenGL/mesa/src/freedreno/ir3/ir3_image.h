/*
 * Copyright (C) 2017-2018 Rob Clark <robclark@freedesktop.org>
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

#ifndef IR3_IMAGE_H_
#define IR3_IMAGE_H_

#include "ir3_context.h"

void ir3_ibo_mapping_init(struct ir3_ibo_mapping *mapping,
                          unsigned num_textures);
struct ir3_instruction *ir3_ssbo_to_ibo(struct ir3_context *ctx, nir_src src);
unsigned ir3_ssbo_to_tex(struct ir3_ibo_mapping *mapping, unsigned ssbo);
struct ir3_instruction *ir3_image_to_ibo(struct ir3_context *ctx, nir_src src);
unsigned ir3_image_to_tex(struct ir3_ibo_mapping *mapping, unsigned image);

unsigned ir3_get_image_coords(const nir_intrinsic_instr *instr,
                              unsigned *flagsp);
type_t ir3_get_type_for_image_intrinsic(const nir_intrinsic_instr *instr);
unsigned ir3_get_num_components_for_image_format(enum pipe_format);

#endif /* IR3_IMAGE_H_ */
