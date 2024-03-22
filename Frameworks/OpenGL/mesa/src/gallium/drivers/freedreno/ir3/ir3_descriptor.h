/*
 * Copyright © 2022 Google, Inc.
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

#ifndef IR3_DESCRIPTOR_H_
#define IR3_DESCRIPTOR_H_

#include "ir3/ir3_shader.h"

/*
 * When using bindless descriptor sets for image/SSBO (and fb-read) state,
 * since the descriptor sets are large, layout the descriptor set with the
 * first IR3_BINDLESS_SSBO_COUNT slots for SSBOs followed by
 * IR3_BINDLESS_IMAGE_COUNT slots for images.  (For fragment shaders, the
 * last image slot is reserved for fb-read tex descriptor.)
 *
 * Note that these limits are more or less arbitrary.  But the enable_mask
 * in fd_shaderbuf_stateobj / fd_shaderimg_stateobj would need to be more
 * than uint32_t to support more than 32.
 */

#define IR3_BINDLESS_SSBO_OFFSET  0
#define IR3_BINDLESS_SSBO_COUNT   32
#define IR3_BINDLESS_IMAGE_OFFSET IR3_BINDLESS_SSBO_COUNT
#define IR3_BINDLESS_IMAGE_COUNT  32
#define IR3_BINDLESS_DESC_COUNT   (IR3_BINDLESS_IMAGE_OFFSET + IR3_BINDLESS_IMAGE_COUNT)

/**
 * When using bindless descriptor sets for IBO/etc, each shader stage gets
 * it's own descriptor set, avoiding the need to merge image/ssbo state
 * across shader stages.
 */
static inline unsigned
ir3_shader_descriptor_set(enum pipe_shader_type shader)
{
   switch (shader) {
   case PIPE_SHADER_VERTEX: return 0;
   case PIPE_SHADER_TESS_CTRL: return 1;
   case PIPE_SHADER_TESS_EVAL: return 2;
   case PIPE_SHADER_GEOMETRY:  return 3;
   case PIPE_SHADER_FRAGMENT:  return 4;
   case PIPE_SHADER_COMPUTE:   return 0;
   default:
      unreachable("bad shader stage");
      return ~0;
   }
}

bool ir3_nir_lower_io_to_bindless(nir_shader *shader);

#endif /* IR3_DESCRIPTOR_H_ */
