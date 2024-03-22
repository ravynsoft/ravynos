/*
 * Copyright © 2022 Intel Corporation
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

#ifndef ANV_GENERATED_INDIRECT_DRAWS_H
#define ANV_GENERATED_INDIRECT_DRAWS_H

#include "shaders/interface.h"

struct PACKED anv_generated_indirect_params {
   struct anv_generated_indirect_draw_params draw;

   /* Global address of binding 0 */
   uint64_t indirect_data_addr;

   /* Global address of binding 1 */
   uint64_t generated_cmds_addr;

   /* Global address of binding 2 */
   uint64_t draw_ids_addr;

   /* Global address of binding 3 (points to the draw_count field above) */
   uint64_t draw_count_addr;

   /* Draw count value for non count variants of draw indirect commands */
   uint32_t draw_count;

   /* CPU side pointer to the previous item when number of draws has to be
    * split into smaller chunks, see while loop in
    * genX(cmd_buffer_emit_indirect_generated_draws)
    */
   struct anv_generated_indirect_params *prev;
};

struct PACKED anv_query_copy_params {
   struct anv_query_copy_shader_params copy;

   uint64_t query_data_addr;

   uint64_t destination_addr;
};

/* This needs to match memcpy_compute.glsl :
 *
 *    layout(set = 0, binding = 2) uniform block
 */
struct PACKED anv_memcpy_shader_params {
   uint32_t num_dwords;
   uint32_t pad;
};

struct PACKED anv_memcpy_params {
   struct anv_memcpy_shader_params copy;

   uint64_t src_addr;

   uint64_t dst_addr;
};

#endif /* ANV_GENERATED_INDIRECT_DRAWS_H */
