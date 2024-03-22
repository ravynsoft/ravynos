/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef D3D12_COMPUTE_TRANSFORMS_H
#define D3D12_COMPUTE_TRANSFORMS_H

#include "d3d12_context.h"
#include "d3d12_compiler.h"

enum class d3d12_compute_transform_type
{
   /* Extract vertex shader draw params (base vertex, instance, draw ID) from
    * a stream of indirect draw (indexed) params
    */
   base_vertex,
   /* Given an SO buffer's declaration in the key, copy filled items from a fake (multiplied)
    * buffer into the original SO buffer, after the original filled size (loaded from the indirect
    * arg buffer double-bound as a UBO), making sure to skip gaps
    */
   fake_so_buffer_copy_back,
   /* Append a fake SO buffer filed size with (vertex count, 1, 1, original filled size)
    * for an indirect dispatch of the fake_so_buffer_copy_back transform, and also update
    * the original filled size with the fake filled size
    */
   fake_so_buffer_vertex_count,
   /* Append a buffer filled size with (vertex count, 1, 0, 0) */
   draw_auto,
   /* Accumulate queries together and write a 32-bit or 64-bit result */
   query_resolve,
   max,
};

struct d3d12_compute_transform_key
{
   d3d12_compute_transform_type type;

   union
   {
      struct {
         unsigned indexed : 1;
         unsigned dynamic_count : 1;
      } base_vertex;

      struct {
         uint16_t stride;
         uint16_t num_ranges;
         struct {
            uint16_t offset;
            uint16_t size;
         } ranges[PIPE_MAX_SO_OUTPUTS];
      } fake_so_buffer_copy_back;

      struct {
         /* true means the accumulation should be done as uint64, else uint32. */
         uint8_t is_64bit : 1;
         /* Indicates how many subqueries to accumulate together into a final result. When
          * set to 1, single_subquery_index determines where the data comes from. */
         uint8_t num_subqueries : 3;
         uint8_t pipe_query_type : 4;
         /* true means output is written where input[0] was, else output is a separate buffer.
          * true also means all fields are accumulated, else single_result_field_offset determines
          * which field is resolved. Implies num_subqueries == 1. */
         uint8_t is_resolve_in_place : 1;
         uint8_t single_subquery_index : 2;
         uint8_t single_result_field_offset : 4;
         uint8_t is_signed : 1;
         float timestamp_multiplier;
      } query_resolve;
   };
};

d3d12_shader_selector *
d3d12_get_compute_transform(struct d3d12_context *ctx, const d3d12_compute_transform_key *key);

void
d3d12_compute_transform_cache_init(struct d3d12_context *ctx);

void
d3d12_compute_transform_cache_destroy(struct d3d12_context *ctx);

struct d3d12_compute_transform_save_restore
{
   struct d3d12_shader_selector *cs;
   struct pipe_constant_buffer cbuf0;
   struct pipe_shader_buffer ssbos[5];
   bool queries_disabled;
};

void
d3d12_save_compute_transform_state(struct d3d12_context *ctx, d3d12_compute_transform_save_restore *save);

void
d3d12_restore_compute_transform_state(struct d3d12_context *ctx, d3d12_compute_transform_save_restore *save);

#endif
