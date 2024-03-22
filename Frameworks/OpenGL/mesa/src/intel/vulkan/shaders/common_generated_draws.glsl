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

#include "interface.h"

/* All storage bindings will be accessed through A64 messages */
layout(set = 0, binding = 0, std430) buffer Storage0 {
   uint indirect_data[];
};

layout(set = 0, binding = 1, std430) buffer Storage1 {
   uint commands[];
};

layout(set = 0, binding = 2, std430) buffer Storage2 {
   uint draw_ids[];
};

/* We're not using a uniform block for this because our compiler
 * infrastructure relies on UBOs to be 32-bytes aligned so that we can push
 * them into registers. This value can come directly from the indirect buffer
 * given to indirect draw commands and the requirement there is 4-bytes
 * alignment.
 *
 * Also use a prefix to the variable to remember to make a copy of it, avoid
 * unnecessary accesses.
 */
layout(set = 0, binding = 3) buffer Storage3 {
   uint _draw_count;
};

/* This data will be provided through push constants. */
layout(set = 0, binding = 4) uniform block {
   anv_generated_indirect_draw_params params;
};

void write_VERTEX_BUFFER_STATE(uint write_offset,
                               uint mocs,
                               uint buffer_idx,
                               uint64_t address,
                               uint size)
{
   commands[write_offset + 0] = (0          << 0  |    /* Buffer Pitch */
                                 0          << 13 |    /* Null Vertex Buffer */
                                 1          << 14 |    /* Address Modify Enable */
                                 mocs       << 16 |    /* MOCS */
                                 buffer_idx << 26);    /* Vertex Buffer Index */
   commands[write_offset + 1]  = uint(address & 0xffffffff);
   commands[write_offset + 2]  = uint(address >> 32);
   commands[write_offset + 3]  = size;
}

void write_3DPRIMITIVE(uint write_offset,
                       bool is_predicated,
                       bool is_indexed,
                       uint vertex_count_per_instance,
                       uint start_vertex_location,
                       uint instance_count,
                       uint start_instance_location,
                       uint base_vertex_location)
{
   commands[write_offset + 0] = (3 << 29 |         /* Command Type */
                                 3 << 27 |         /* Command SubType */
                                 3 << 24 |         /* 3D Command Opcode */
                                 uint(is_predicated) << 8 |
                                 5 << 0);          /* DWord Length */
   commands[write_offset + 1] = uint(is_indexed) << 8;
   commands[write_offset + 2] = vertex_count_per_instance;
   commands[write_offset + 3] = start_vertex_location;
   commands[write_offset + 4] = instance_count;
   commands[write_offset + 5] = start_instance_location;
   commands[write_offset + 6] = base_vertex_location;
}

void write_3DPRIMITIVE_EXTENDED(uint write_offset,
                                bool is_predicated,
                                bool is_indexed,
                                bool use_tbimr,
                                uint vertex_count_per_instance,
                                uint start_vertex_location,
                                uint instance_count,
                                uint start_instance_location,
                                uint base_vertex_location,
                                uint param_base_vertex,
                                uint param_base_instance,
                                uint param_draw_id)
{
   commands[write_offset + 0] = (3 << 29 |         /* Command Type */
                                 3 << 27 |         /* Command SubType */
                                 3 << 24 |         /* 3D Command Opcode */
                                 uint(use_tbimr) << 13 |
                                 1 << 11 |         /* Extended Parameter Enable */
                                 uint(is_predicated) << 8 |
                                 8 << 0);          /* DWord Length */
   commands[write_offset + 1] = uint(is_indexed) << 8;
   commands[write_offset + 2] = vertex_count_per_instance;
   commands[write_offset + 3] = start_vertex_location;
   commands[write_offset + 4] = instance_count;
   commands[write_offset + 5] = start_instance_location;
   commands[write_offset + 6] = base_vertex_location;
   commands[write_offset + 7] = param_base_vertex;
   commands[write_offset + 8] = param_base_instance;
   commands[write_offset + 9] = param_draw_id;
}

void write_MI_BATCH_BUFFER_START(uint write_offset,
                                 uint64_t addr)
{
   commands[write_offset + 0] = (0  << 29 | /* Command Type */
                                 49 << 23 | /* MI Command Opcode */
                                 1  << 8  | /* Address Space Indicator (PPGTT) */
                                 1  << 0);  /* DWord Length */
   commands[write_offset + 1] = uint(addr & 0xffffffff);
   commands[write_offset + 2] = uint(addr >> 32);
}

void end_generated_draws(uint item_idx, uint cmd_idx, uint draw_id, uint draw_count)
{
   uint _3dprim_dw_size = (params.flags >> 16) & 0xff;
   bool indirect_count = (params.flags & ANV_GENERATED_FLAG_COUNT) != 0;
   bool ring_mode = (params.flags & ANV_GENERATED_FLAG_RING_MODE) != 0;
   /* We can have an indirect draw count = 0. */
   uint last_draw_id = draw_count == 0 ? 0 : (min(draw_count, params.max_draw_count) - 1);
   uint jump_offset = draw_count == 0 ? 0 : _3dprim_dw_size;

   if (ring_mode) {
      if (draw_id == last_draw_id) {
         /* Exit the ring buffer to the next user commands */
         write_MI_BATCH_BUFFER_START(cmd_idx + jump_offset, params.end_addr);
      } else if (item_idx == (params.ring_count - 1)) {
         /* Jump back to the generation shader to generate mode draws */
         write_MI_BATCH_BUFFER_START(cmd_idx + jump_offset, params.gen_addr);
      }
   } else {
      if (draw_id == last_draw_id && draw_count < params.max_draw_count) {
         /* Skip forward to the end of the generated draws */
         write_MI_BATCH_BUFFER_START(cmd_idx + jump_offset, params.end_addr);
      }
   }
}
