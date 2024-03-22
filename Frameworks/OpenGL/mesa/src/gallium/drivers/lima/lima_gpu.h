/*
 * Copyright (c) 2011-2013 Luc Verhaegen <libv@skynet.be>
 * Copyright (c) 2017-2019 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef H_LIMA_GPU
#define H_LIMA_GPU

#include <stdint.h>
#include <assert.h>

#include <util/u_dynarray.h>

struct lima_gp_frame_reg {
   uint32_t vs_cmd_start;
   uint32_t vs_cmd_end;
   uint32_t plbu_cmd_start;
   uint32_t plbu_cmd_end;
   uint32_t tile_heap_start;
   uint32_t tile_heap_end;
};

struct lima_pp_frame_reg {
   uint32_t plbu_array_address;
   uint32_t render_address;
   uint32_t unused_0;
   uint32_t flags;
   uint32_t clear_value_depth;
   uint32_t clear_value_stencil;
   uint32_t clear_value_color;
   uint32_t clear_value_color_1;
   uint32_t clear_value_color_2;
   uint32_t clear_value_color_3;
   uint32_t width;
   uint32_t height;
   uint32_t fragment_stack_address;
   uint32_t fragment_stack_size;
   uint32_t unused_1;
   uint32_t unused_2;
   uint32_t one;
   uint32_t supersampled_height;
   uint32_t dubya;
   uint32_t onscreen;
   uint32_t blocking;
   uint32_t scale;
   uint32_t channel_layout;
};

struct lima_pp_wb_reg {
   uint32_t type;
   uint32_t address;
   uint32_t pixel_format;
   uint32_t downsample_factor;
   uint32_t pixel_layout;
   uint32_t pitch;
   uint32_t flags;
   uint32_t mrt_bits;
   uint32_t mrt_pitch;
   uint32_t unused0;
   uint32_t unused1;
   uint32_t unused2;
};

struct lima_render_state {
   uint32_t blend_color_bg;
   uint32_t blend_color_ra;
   uint32_t alpha_blend;
   uint32_t depth_test;
   uint32_t depth_range;
   uint32_t stencil_front;
   uint32_t stencil_back;
   uint32_t stencil_test;
   uint32_t multi_sample;
   uint32_t shader_address;
   uint32_t varying_types;
   uint32_t uniforms_address;
   uint32_t textures_address;
   uint32_t aux0;
   uint32_t aux1;
   uint32_t varyings_address;
};

/* plbu commands */
#define PLBU_CMD_BEGIN(array, max) { \
   int i = 0, max_n = max; \
   struct util_dynarray *plbu_cmd_array = array; \
   uint32_t *plbu_cmd = util_dynarray_ensure_cap(plbu_cmd_array, plbu_cmd_array->size + max_n * 4);

#define PLBU_CMD_END() \
   assert(i <= max_n); \
   plbu_cmd_array->size += i * 4; \
}

#define PLBU_CMD_CURRENT_POS() \
   (util_dynarray_num_elements(plbu_cmd_array, uint32_t) + i)

#define PLBU_CMD(v1, v2) \
   do { \
      plbu_cmd[i++] = v1; \
      plbu_cmd[i++] = v2; \
   } while (0)

#define PLBU_BLOCK_W_MASK 0xff
#define PLBU_BLOCK_H_MASK 0xff

#define PLBU_CMD_BLOCK_STEP(shift_min, shift_h, shift_w) \
   PLBU_CMD(((shift_min) << 28) | ((shift_h) << 16) | (shift_w), 0x1000010C)
#define PLBU_CMD_TILED_DIMENSIONS(tiled_w, tiled_h) \
   PLBU_CMD((((tiled_w) - 1) << 24) | (((tiled_h) - 1) << 8), 0x10000109)
#define PLBU_CMD_BLOCK_STRIDE(block_w) \
   PLBU_CMD((block_w) & PLBU_BLOCK_W_MASK, 0x30000000)
#define PLBU_CMD_ARRAY_ADDRESS(gp_stream, block_num) \
   PLBU_CMD(gp_stream, 0x28000000 | ((block_num) - 1) | 1)
#define PLBU_CMD_VIEWPORT_LEFT(v) PLBU_CMD(v, 0x10000107)
#define PLBU_CMD_VIEWPORT_RIGHT(v) PLBU_CMD(v, 0x10000108)
#define PLBU_CMD_VIEWPORT_BOTTOM(v) PLBU_CMD(v, 0x10000105)
#define PLBU_CMD_VIEWPORT_TOP(v) PLBU_CMD(v, 0x10000106)
#define PLBU_CMD_ARRAYS_SEMAPHORE_BEGIN() PLBU_CMD(0x00010002, 0x60000000)
#define PLBU_CMD_ARRAYS_SEMAPHORE_END() PLBU_CMD(0x00010001, 0x60000000)
#define PLBU_CMD_PRIMITIVE_SETUP(force_point_size, cull, index_size) \
   PLBU_CMD(0x2200 | ((force_point_size) ? 0x1000 : 0) | \
            (cull) | ((index_size) << 9), 0x1000010B)
#define PLBU_CMD_RSW_VERTEX_ARRAY(rsw, gl_pos) \
   PLBU_CMD(rsw, 0x80000000 | ((gl_pos) >> 4))
#define PLBU_CMD_SCISSORS(minx, maxx, miny, maxy) \
   PLBU_CMD(((minx) << 30) | ((maxy) - 1) << 15 | (miny), \
            0x70000000 | ((maxx) - 1) << 13 | ((minx) >> 2))
#define PLBU_CMD_UNKNOWN1() PLBU_CMD(0x00000000, 0x1000010A)
#define PLBU_CMD_UNKNOWN2() PLBU_CMD(0x00000200, 0x1000010B)
#define PLBU_CMD_LOW_PRIM_SIZE(v) PLBU_CMD(v, 0x1000010D)
#define PLBU_CMD_DEPTH_RANGE_NEAR(v) PLBU_CMD(v, 0x1000010E)
#define PLBU_CMD_DEPTH_RANGE_FAR(v) PLBU_CMD(v, 0x1000010F)
#define PLBU_CMD_INDEXED_DEST(gl_pos) PLBU_CMD(gl_pos, 0x10000100)
#define PLBU_CMD_INDEXED_PT_SIZE(pt_size) PLBU_CMD(pt_size, 0x10000102)
#define PLBU_CMD_INDICES(va) PLBU_CMD(va, 0x10000101)
#define PLBU_CMD_DRAW_ARRAYS(mode, start, count) \
   PLBU_CMD(((count) << 24) | (start), (((mode) & 0x1F) << 16) | ((count) >> 8))
#define PLBU_CMD_DRAW_ELEMENTS(mode, start, count) \
   PLBU_CMD(((count) << 24) | (start), \
            0x00200000 | (((mode) & 0x1F) << 16) | ((count) >> 8))

/* vs commands */
#define VS_CMD_BEGIN(array, max) { \
   int i = 0, max_n = max; \
   struct util_dynarray *vs_cmd_array = array; \
   uint32_t *vs_cmd = util_dynarray_ensure_cap(vs_cmd_array, vs_cmd_array->size + max_n * 4);

#define VS_CMD_END() \
   assert(i <= max_n); \
   vs_cmd_array->size += i * 4; \
}

#define VS_CMD(v1, v2) \
   do { \
      vs_cmd[i++] = v1; \
      vs_cmd[i++] = v2; \
   } while (0)

#define VS_CMD_ARRAYS_SEMAPHORE_BEGIN_1() VS_CMD(0x00028000, 0x50000000)
#define VS_CMD_ARRAYS_SEMAPHORE_BEGIN_2() VS_CMD(0x00000001, 0x50000000)
#define VS_CMD_ARRAYS_SEMAPHORE_END(index_draw) \
   VS_CMD((index_draw) ? 0x00018000 : 0x00000000, 0x50000000)
#define VS_CMD_UNIFORMS_ADDRESS(addr, size) \
   VS_CMD(addr, 0x30000000 | ((size) << 12))
#define VS_CMD_SHADER_ADDRESS(addr, size) \
   VS_CMD(addr, 0x40000000 | ((size) << 12))
#define VS_CMD_SHADER_INFO(prefetch, size) \
   VS_CMD(((prefetch) << 20) | ((((size) >> 4) - 1) << 10), 0x10000040)
#define VS_CMD_VARYING_ATTRIBUTE_COUNT(nv, na) \
   VS_CMD((((nv) - 1) << 8) | (((na) - 1) << 24), 0x10000042)
#define VS_CMD_UNKNOWN1() VS_CMD(0x00000003, 0x10000041)
#define VS_CMD_UNKNOWN2() VS_CMD(0x00000000, 0x60000000)
#define VS_CMD_ATTRIBUTES_ADDRESS(addr, na) \
   VS_CMD(addr, 0x20000000 | ((na) << 17))
#define VS_CMD_VARYINGS_ADDRESS(addr, nv) \
   VS_CMD(addr, 0x20000008 | ((nv) << 17))
#define VS_CMD_DRAW(num, index_draw) \
   VS_CMD(((num) << 24) | ((index_draw) ? 1 : 0), ((num) >> 8))

#endif
