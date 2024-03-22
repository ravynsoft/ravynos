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

#include "interface.h"

/* These 3 bindings will be accessed through A64 messages */
layout(set = 0, binding = 0, std430) buffer Storage0 {
   uint query_data[];
};

layout(set = 0, binding = 1, std430) buffer Storage1 {
   uint destination[];
};

/* This data will be provided through push constants. */
layout(set = 0, binding = 2) uniform block {
   anv_query_copy_shader_params params;
};

void query_copy(uint item_idx)
{
   if (item_idx >= params.num_queries)
      return;

   bool is_result64 = (params.flags & ANV_COPY_QUERY_FLAG_RESULT64) != 0;
   bool write_available = (params.flags & ANV_COPY_QUERY_FLAG_AVAILABLE) != 0;
   bool compute_delta = (params.flags & ANV_COPY_QUERY_FLAG_DELTA) != 0;
   bool partial_result = (params.flags & ANV_COPY_QUERY_FLAG_PARTIAL) != 0;

   uint query_byte = (params.query_base + item_idx) * params.query_stride;
   uint query_data_byte = query_byte + params.query_data_offset;
   uint destination_byte = item_idx * params.destination_stride;

   uint64_t availability = query_data[query_byte / 4];

   uint query_data_dword = query_data_byte / 4;
   uint dest_dword = destination_byte / 4;
   for (uint i = 0; i < params.num_items; i++) {
      uint item_data_dword = query_data_dword + i * 2 * (compute_delta ? 2 : 1);

      uint64_t v;
      if (compute_delta) {
         uint64_t v0 = uint64_t(query_data[item_data_dword + 0]) |
                       (uint64_t(query_data[item_data_dword + 1]) << 32);
         uint64_t v1 = uint64_t(query_data[item_data_dword + 2]) |
                       (uint64_t(query_data[item_data_dword + 3]) << 32);

         v = v1 - v0;
      } else {

         v = uint64_t(query_data[item_data_dword + 0]) |
             (uint64_t(query_data[item_data_dword + 1]) << 32);
      }

      /* vkCmdCopyQueryPoolResults:
       *
       *    "If VK_QUERY_RESULT_PARTIAL_BIT is set, then for any query that is
       *     unavailable, an intermediate result between zero and the final
       *     result value is written for that query."
       *
       * We write 0 as the values not being written yet, we can't really make
       * provide any sensible value.
       */
      if (partial_result && availability == 0)
         v = 0;

      if (is_result64) {
         destination[dest_dword + 0] = uint(v & 0xffffffff);
         destination[dest_dword + 1] = uint(v >> 32);
         dest_dword += 2;
      } else {
         destination[dest_dword + 0] = uint(v & 0xffffffff);
         dest_dword += 1;
      }
   }

   if (write_available) {
      if (is_result64) {
         destination[dest_dword + 0] = uint(availability & 0xffffffff);
         destination[dest_dword + 1] = uint(availability >> 32);
      } else {
         destination[dest_dword + 0] = uint(availability & 0xffffffff);
      }
   }
}
