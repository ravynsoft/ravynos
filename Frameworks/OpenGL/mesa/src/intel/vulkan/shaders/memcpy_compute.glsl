/*
 * Copyright © 2023 Intel Corporation
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

#version 450
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_GOOGLE_include_directive : enable

layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;

/* These 2 bindings will be accessed through A64 messages */
layout(set = 0, binding = 0, std430) buffer Storage0 {
   uint src[];
};

layout(set = 0, binding = 1, std430) buffer Storage1 {
   uint dst[];
};

/* This data will be provided through push constants. */
layout(set = 0, binding = 2) uniform block {
   uint num_dwords;
   uint pad;
};

void main()
{
   uint idx = gl_GlobalInvocationID.x * 4;
   /* Try to do copies in single message as much as possible. */
   if (idx + 4 <= num_dwords) {
      dst[idx + 0] = src[idx + 0];
      dst[idx + 1] = src[idx + 1];
      dst[idx + 2] = src[idx + 2];
      dst[idx + 3] = src[idx + 3];
   } else if (idx + 3 <= num_dwords) {
      dst[idx + 0] = src[idx + 0];
      dst[idx + 1] = src[idx + 1];
      dst[idx + 2] = src[idx + 2];
   } else if (idx + 2 <= num_dwords) {
      dst[idx + 0] = src[idx + 0];
      dst[idx + 1] = src[idx + 1];
   } else if (idx + 1 <= num_dwords) {
      dst[idx + 0] = src[idx + 0];
   }
}
