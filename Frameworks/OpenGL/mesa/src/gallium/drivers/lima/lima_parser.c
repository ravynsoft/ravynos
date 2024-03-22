/*
 * Copyright (c) 2019 Andreas Baierl <ichgeh@imkreisrum.de>
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

#include "util/u_math.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lima_context.h"
#include "lima_parser.h"
#include "lima_texture.h"

#include "lima/ir/gp/codegen.h"
#include "lima/ir/pp/codegen.h"

typedef struct {
   char *info;
} render_state_info;

static render_state_info render_state_infos[] = {
   { .info = "BLEND_COLOR_BG", },
   { .info = "BLEND_COLOR_RA", },
   { .info = "ALPHA_BLEND", },
   { .info = "DEPTH_TEST", },
   { .info = "DEPTH_RANGE", },
   { .info = "STENCIL_FRONT", },
   { .info = "STENCIL_BACK", },
   { .info = "STENCIL_TEST", },
   { .info = "MULTI_SAMPLE", },
   { .info = "SHADER_ADDRESS (FS)", },
   { .info = "VARYING_TYPES", },
   { .info = "UNIFORMS_ADDRESS (PP)", },
   { .info = "TEXTURES_ADDRESS", },
   { .info = "AUX0", },
   { .info = "AUX1", },
   { .info = "VARYINGS_ADDRESS", },
};

/* VS CMD stream parser functions */

static void
parse_vs_draw(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   if ((*value1 == 0x00000000) && (*value2 == 0x00000000))
      fprintf(fp, "\t/* ---EMPTY CMD */\n");
   else
      fprintf(fp, "\t/* DRAW: num: %d, index_draw: %s */\n",
              (*value1 & 0xff000000) >> 24 | (*value2 & 0x000000ff) << 8,
              (*value1 & 0x00000001) ? "true" : "false");
}

static void
parse_vs_shader_info(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* SHADER_INFO: prefetch: %d, size: %d */\n",
           (*value1 & 0xfff00000) >> 20,
           (((*value1 & 0x000fffff) >> 10) + 1) << 4);
}

static void
parse_vs_unknown1(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* UNKNOWN_1 */\n");
}

static void
parse_vs_varying_attribute_count(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* VARYING_ATTRIBUTE_COUNT: nr_vary: %d, nr_attr: %d */\n",
           ((*value1 & 0x00ffffff) >> 8) + 1, (*value1 >> 24) + 1);
}

static void
parse_vs_attributes_address(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* ATTRIBUTES_ADDRESS: address: 0x%08x, size: %d */\n",
           *value1, (*value2 & 0x0fffffff) >> 17);
}

static void
parse_vs_varyings_address(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* VARYINGS_ADDRESS: varying info @ 0x%08x, size: %d */\n",
           *value1, (*value2 & 0x0fffffff) >> 17);
}

static void
parse_vs_uniforms_address(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* UNIFORMS_ADDRESS (GP): address: 0x%08x, size: %d */\n",
           *value1, (*value2 & 0x0fffffff) >> 12);
}

static void
parse_vs_shader_address(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* SHADER_ADDRESS (VS): address: 0x%08x, size: %d */\n",
           *value1, (*value2 & 0x0fffffff) >> 12);
}

static void
parse_vs_semaphore(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   if (*value1 == 0x00028000)
      fprintf(fp, "\t/* SEMAPHORE_BEGIN_1 */\n");
   else if (*value1 == 0x00000001)
      fprintf(fp, "\t/* SEMAPHORE_BEGIN_2 */\n");
   else if (*value1 == 0x00000000)
      fprintf(fp, "\t/* SEMAPHORE_END: index_draw disabled */\n");
   else if (*value1 == 0x00018000)
      fprintf(fp, "\t/* SEMAPHORE_END: index_draw enabled */\n");
   else
      fprintf(fp, "\t/* SEMAPHORE - cmd unknown! */\n");
}

static void
parse_vs_unknown2(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* UNKNOWN_2 */\n");
}

static void
parse_vs_continue(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* CONTINUE: at 0x%08x */\n", *value1);
}

void
lima_parse_vs(FILE *fp, uint32_t *data, int size, uint32_t start)
{
   uint32_t *value1;
   uint32_t *value2;

   fprintf(fp, "\n");
   fprintf(fp, "/* ============ VS CMD STREAM BEGIN ============= */\n");
   for (int i = 0; i * 4 < size; i += 2) {
      value1 = &data[i];
      value2 = &data[i + 1];
      fprintf(fp, "/* 0x%08x (0x%08x) */\t0x%08x 0x%08x",
              start + i * 4, i * 4, *value1, *value2);

      if ((*value2 & 0xffff0000) == 0x00000000)
         parse_vs_draw(fp, value1, value2);
      else if ((*value2 & 0xff0000ff) == 0x10000040)
         parse_vs_shader_info(fp, value1, value2);
      else if ((*value2 & 0xff0000ff) == 0x10000041)
         parse_vs_unknown1(fp, value1, value2);
      else if ((*value2 & 0xff0000ff) == 0x10000042)
         parse_vs_varying_attribute_count(fp, value1, value2);
      else if ((*value2 & 0xff0000ff) == 0x20000000)
         parse_vs_attributes_address(fp, value1, value2);
      else if ((*value2 & 0xff0000ff) == 0x20000008)
         parse_vs_varyings_address(fp, value1, value2);
      else if ((*value2 & 0xff000000) == 0x30000000)
         parse_vs_uniforms_address(fp, value1, value2);
      else if ((*value2 & 0xff000000) == 0x40000000)
         parse_vs_shader_address(fp, value1, value2);
      else if ((*value2  & 0xff000000)== 0x50000000)
         parse_vs_semaphore(fp, value1, value2);
      else if ((*value2 & 0xff000000) == 0x60000000)
         parse_vs_unknown2(fp, value1, value2);
      else if ((*value2 & 0xff000000) == 0xf0000000)
         parse_vs_continue(fp, value1, value2);
      else
         fprintf(fp, "\t/* --- unknown cmd --- */\n");
   }
   fprintf(fp, "/* ============ VS CMD STREAM END =============== */\n");
   fprintf(fp, "\n");
}

/* PLBU CMD stream parser functions */

static void
parse_plbu_block_step(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* BLOCK_STEP: shift_min: %d, shift_h: %d, shift_w: %d */\n",
           (*value1 & 0xf0000000) >> 28,
           (*value1 & 0x0fff0000) >> 16,
           *value1 & 0x0000ffff);
}

static void
parse_plbu_tiled_dimensions(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* TILED_DIMENSIONS: tiled_w: %d, tiled_h: %d */\n",
           ((*value1 & 0xff000000) >> 24) + 1,
           ((*value1 & 0x00ffff00) >> 8) + 1);
}

static void
parse_plbu_block_stride(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* BLOCK_STRIDE: block_w: %d */\n", *value1 & 0x000000ff);
}

static void
parse_plbu_array_address(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* ARRAY_ADDRESS: gp_stream: 0x%08x, block_num (block_w * block_h): %d */\n",
           *value1, (*value2 & 0x00ffffff) + 1);
}

static void
parse_plbu_viewport_left(FILE *fp, float *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* VIEWPORT_LEFT: viewport_left: %f */\n", *value1);
}

static void
parse_plbu_viewport_right(FILE *fp, float *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* VIEWPORT_RIGHT: viewport_right: %f */\n", *value1);
}

static void
parse_plbu_viewport_bottom(FILE *fp, float *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* VIEWPORT_BOTTOM: viewport_bottom: %f */\n", *value1);
}

static void
parse_plbu_viewport_top(FILE *fp, float *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* VIEWPORT_TOP: viewport_top: %f */\n", *value1);
}

static void
parse_plbu_semaphore(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   if (*value1 == 0x00010002)
      fprintf(fp, "\t/* ARRAYS_SEMAPHORE_BEGIN */\n");
   else if (*value1 == 0x00010001)
      fprintf(fp, "\t/* ARRAYS_SEMAPHORE_END */\n");
   else
      fprintf(fp, "\t/* SEMAPHORE - cmd unknown! */\n");
}

static void
parse_plbu_primitive_setup(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   if (*value1 == 0x00000200)
      fprintf(fp, "\t/* UNKNOWN_2 (PRIMITIVE_SETUP INIT?) */\n");
   else
      fprintf(fp, "\t/* PRIMITIVE_SETUP: %scull: %d (0x%x), index_size: %d */\n",
              (*value1 & 0x1000) ? "force point size, " : "",
              (*value1 & 0x000f0000) >> 16, (*value1 & 0x000f0000) >> 16,
              (*value1 & 0x00000e00) >> 9);
}

static void
parse_plbu_rsw_vertex_array(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* RSW_VERTEX_ARRAY: rsw: 0x%08x, gl_pos: 0x%08x */\n",
           *value1,
           (*value2 & 0x0fffffff) << 4);
}

static void
parse_plbu_scissors(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   float minx = (*value1 & 0xc0000000) >> 30 | (*value2 & 0x00001fff) << 2;
   float maxx = ((*value2 & 0x0fffe000) >> 13) + 1;
   float miny = *value1 & 0x00003fff;
   float maxy = ((*value1 & 0x3fff8000) >> 15) + 1;

   fprintf(fp, "\t/* SCISSORS: minx: %f, maxx: %f, miny: %f, maxy: %f */\n",
           minx, maxx, miny, maxy);
}

static void
parse_plbu_unknown_1(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* UNKNOWN_1 */\n");
}

static void
parse_plbu_low_prim_size(FILE *fp, float *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* LOW_PRIM_SIZE: size: %f */\n", *value1);
}

static void
parse_plbu_depth_range_near(FILE *fp, float *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* DEPTH_RANG_NEAR: depth_range: %f */\n", *value1);
}

static void
parse_plbu_depth_range_far(FILE *fp, float *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* DEPTH_RANGE_FAR: depth_range: %f */\n", *value1);
}

static void
parse_plbu_indexed_dest(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* INDEXED_DEST: gl_pos: 0x%08x */\n", *value1);
}

static void
parse_plbu_indexed_pt_size(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* INDEXED_PT_SIZE: pt_size: 0x%08x */\n", *value1);
}

static void
parse_plbu_indices(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* INDICES: indices: 0x%08x */\n", *value1);
}

static void
parse_plbu_draw_arrays(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   if ((*value1 == 0x00000000) && (*value2 == 0x00000000)) {
      fprintf(fp, "\t/* ---EMPTY CMD */\n");
      return;
   }

   uint32_t count = (*value1 & 0xff000000) >> 24 | (*value2 & 0x000000ff) << 8;
   uint32_t start = *value1 & 0x00ffffff;
   uint32_t mode = (*value2 & 0x001f0000) >> 16;

   fprintf(fp, "\t/* DRAW_ARRAYS: count: %d, start: %d, mode: %d (0x%x) */\n",
           count, start, mode, mode);
}

static void
parse_plbu_draw_elements(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   uint32_t count = (*value1 & 0xff000000) >> 24 | (*value2 & 0x000000ff) << 8;
   uint32_t start = *value1 & 0x00ffffff;
   uint32_t mode = (*value2 & 0x001f0000) >> 16;

   fprintf(fp, "\t/* DRAW_ELEMENTS: count: %d, start: %d, mode: %d (0x%x) */\n",
           count, start, mode, mode);
}

static void
parse_plbu_continue(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* CONTINUE: continue at 0x%08x */\n", *value1);
}

static void
parse_plbu_end(FILE *fp, uint32_t *value1, uint32_t *value2)
{
   fprintf(fp, "\t/* END (FINISH/FLUSH) */\n");
}

void
lima_parse_plbu(FILE *fp, uint32_t *data, int size, uint32_t start)
{
   uint32_t *value1;
   uint32_t *value2;

   fprintf(fp, "/* ============ PLBU CMD STREAM BEGIN ============= */\n");
   for (int i = 0; i * 4 < size; i += 2) {
      value1 = &data[i];
      value2 = &data[i + 1];
      fprintf(fp, "/* 0x%08x (0x%08x) */\t0x%08x 0x%08x",
              start + i * 4, i * 4, *value1, *value2);

      if ((*value2 & 0xffe00000) == 0x00000000)
         parse_plbu_draw_arrays(fp, value1, value2);
      else if ((*value2 & 0xffe00000) == 0x00200000)
         parse_plbu_draw_elements(fp, value1, value2);
      else if ((*value2 & 0xff000fff) == 0x10000100)
         parse_plbu_indexed_dest(fp, value1, value2);
      else if ((*value2 & 0xff000fff) == 0x10000101)
         parse_plbu_indices(fp, value1, value2);
      else if ((*value2 & 0xff000fff) == 0x10000102)
         parse_plbu_indexed_pt_size(fp, value1, value2);
      else if ((*value2 & 0xff000fff) == 0x10000105)
         parse_plbu_viewport_bottom(fp, (float *)value1, value2);
      else if ((*value2 & 0xff000fff) == 0x10000106)
         parse_plbu_viewport_top(fp, (float *)value1, value2);
      else if ((*value2 & 0xff000fff) == 0x10000107)
         parse_plbu_viewport_left(fp, (float *)value1, value2);
      else if ((*value2 & 0xff000fff) == 0x10000108)
         parse_plbu_viewport_right(fp, (float *)value1, value2);
      else if ((*value2 & 0xff000fff) == 0x10000109)
         parse_plbu_tiled_dimensions(fp, value1, value2);
      else if ((*value2 & 0xff000fff) == 0x1000010a)
         parse_plbu_unknown_1(fp, value1, value2);
      else if ((*value2 & 0xff000fff) == 0x1000010b) /* also unknown_2 */
         parse_plbu_primitive_setup(fp, value1, value2);
      else if ((*value2 & 0xff000fff) == 0x1000010c)
         parse_plbu_block_step(fp, value1, value2);
      else if ((*value2 & 0xff000fff) == 0x1000010d)
         parse_plbu_low_prim_size(fp, (float *)value1, value2);
      else if ((*value2 & 0xff000fff) == 0x1000010e)
         parse_plbu_depth_range_near(fp, (float *)value1, value2);
      else if ((*value2 & 0xff000fff) == 0x1000010f)
         parse_plbu_depth_range_far(fp, (float *)value1, value2);
      else if ((*value2 & 0xff000000) == 0x28000000)
         parse_plbu_array_address(fp, value1, value2);
      else if ((*value2 & 0xf0000000) == 0x30000000)
         parse_plbu_block_stride(fp, value1, value2);
      else if (*value2 == 0x50000000)
         parse_plbu_end(fp, value1, value2);
      else if ((*value2  & 0xf0000000)== 0x60000000)
         parse_plbu_semaphore(fp, value1, value2);
      else if ((*value2  & 0xf0000000)== 0x70000000)
         parse_plbu_scissors(fp, value1, value2);
      else if ((*value2  & 0xf0000000)== 0x80000000)
         parse_plbu_rsw_vertex_array(fp, value1, value2);
      else if ((*value2  & 0xf0000000)== 0xf0000000)
         parse_plbu_continue(fp, value1, value2);
      else
         fprintf(fp, "\t/* --- unknown cmd --- */\n");
   }
   fprintf(fp, "/* ============ PLBU CMD STREAM END =============== */\n");
   fprintf(fp, "\n");
}

void
lima_parse_shader(FILE *fp, uint32_t *data, int size, bool is_frag)
{
   uint32_t *value = &data[0];

   if (is_frag) {
      uint32_t *bin = value;
      uint32_t offt = 0;
      uint32_t next_instr_length = 0;

      fprintf(fp, "/* ============ FS DISASSEMBLY BEGIN ============== */\n");

      do {
         ppir_codegen_ctrl *ctrl = (ppir_codegen_ctrl *)bin;
         fprintf(fp, "@%6d: ", offt);
         ppir_disassemble_instr(bin, offt, fp);
         bin += ctrl->count;
         offt += ctrl->count;
         next_instr_length = ctrl->next_count;
      } while (next_instr_length);

      fprintf(fp, "/* ============ FS DISASSEMBLY END ================= */\n");
   } else {
      fprintf(fp, "/* ============ VS DISASSEMBLY BEGIN ============== */\n");
      gpir_disassemble_program((gpir_codegen_instr *)value, size / sizeof(gpir_codegen_instr), fp);
      fprintf(fp, "/* ============ VS DISASSEMBLY END ================= */\n");
   }
}

static void
parse_rsw(FILE *fp, uint32_t *value, int i, uint32_t *helper)
{
   fprintf(fp, "\t/* %s", render_state_infos[i].info);

   switch (i) {
   case 0: /* BLEND COLOR BG */
      fprintf(fp, ": blend_color.color[1] = %f, blend_color.color[2] = %f */\n",
              (float)(ubyte_to_float((*value & 0xffff0000) >> 16)),
              (float)(ubyte_to_float(*value & 0x0000ffff)));
      break;
   case 1: /* BLEND COLOR RA */
      fprintf(fp, ": blend_color.color[3] = %f, blend_color.color[0] = %f */\n",
              (float)(ubyte_to_float((*value & 0xffff0000) >> 16)),
              (float)(ubyte_to_float(*value & 0x0000ffff)));
      break;
   case 2: /* ALPHA BLEND */
      fprintf(fp, "(1): colormask 0x%02x, rgb_func %d (%s), alpha_func %d (%s) */\n",
              (*value & 0xf0000000) >> 28, /* colormask */
              (*value & 0x00000007),
              lima_get_blend_func_string((*value & 0x00000007)), /* rgb_func */
              (*value & 0x00000038) >> 3,
              lima_get_blend_func_string((*value & 0x00000038) >> 3)); /* alpha_func */
      /* add a few tabs for alignment */
      fprintf(fp, "\t\t\t\t\t\t/* %s(2)", render_state_infos[i].info);
      fprintf(fp, ": rgb_src_factor %d (%s), rbg_dst_factor %d (%s) */\n",
              (*value & 0x000007c0) >> 6,
              lima_get_blendfactor_string((*value & 0x000007c0) >> 6), /* rgb_src_factor */
              (*value & 0x0000f800) >> 11,
              lima_get_blendfactor_string((*value & 0x0000f800) >> 11)); /* rgb_dst_factor */
      fprintf(fp, "\t\t\t\t\t\t/* %s(3)", render_state_infos[i].info);
      fprintf(fp, ": alpha_src_factor %d (%s), alpha_dst_factor %d (%s), bits 24-27 0x%02x */\n",
              (*value & 0x000f0000) >> 16,
              lima_get_blendfactor_string((*value & 0x000f0000) >> 16), /* alpha_src_factor */
              (*value & 0x00f00000) >> 20,
              lima_get_blendfactor_string((*value & 0x00f00000) >> 20), /* alpha_dst_factor */
              (*value & 0x0f000000) >> 24); /* bits 24-27 */
      break;
   case 3: /* DEPTH TEST */
      if ((*value & 0x00000001) == 0x00000001)
         fprintf(fp, "(1): depth test enabled && writes allowed");
      else
         fprintf(fp, "(1): depth test disabled || writes not allowed");

      fprintf(fp, "\n\t\t\t\t\t\t/* %s(2)", render_state_infos[i].info);
      fprintf(fp, ": depth_func %d (%s)", ((*value & 0x0000000e) >> 1),
              lima_get_compare_func_string((*value & 0x0000000e) >> 1));
      fprintf(fp, ", offset_scale: %d", (*value & 0x00ff0000) >> 16);
      fprintf(fp, ", offset_units: %d", (*value & 0xff000000) >> 24);
      if (*value & 0x400)
         fprintf(fp, ", shader writes depth or stencil");
      if (*value & 0x800)
         fprintf(fp, ", shader writes depth");
      if (*value & 0x1000)
         fprintf(fp, ", shader writes stencil");
      fprintf(fp, " */\n\t\t\t\t\t\t/* %s(3)", render_state_infos[i].info);
      if ((*value & 0x00000010) == 0x00000010)
         fprintf(fp, ": ignore depth clip near");
      if ((*value & 0x00000020) == 0x00000020)
         fprintf(fp, ", ignore depth clip far");
      fprintf(fp, ", register for gl_FragDepth: $%d", (*value & 0x000003c0) >> 6);
      fprintf(fp, ", unknown bits 13-15: 0x%08x */\n", *value & 0x00000e000);
      break;
   case 4: /* DEPTH RANGE */
      fprintf(fp, ": viewport.far = %f, viewport.near = %f */\n",
              (float)(ushort_to_float((*value & 0xffff0000) >> 16)),
              (float)(ushort_to_float(*value & 0x0000ffff)));
      break;
   case 5: /* STENCIL FRONT */
      fprintf(fp, "(1): valuemask 0x%02x, ref value %d (0x%02x), stencil_func %d (%s)*/\n",
              (*value & 0xff000000) >> 24, /* valuemask */
              (*value & 0x00ff0000) >> 16, (*value & 0x00ff0000) >> 16, /* ref value */
              (*value & 0x00000007),
              lima_get_compare_func_string((*value & 0x00000007))); /* stencil_func */
      /* add a few tabs for alignment */
      fprintf(fp, "\t\t\t\t\t\t/* %s(2)", render_state_infos[i].info);
      fprintf(fp, ": fail_op %d (%s), zfail_op %d (%s), zpass_op %d (%s), unknown (12-15) 0x%02x */\n",
              (*value & 0x00000038) >> 3,
              lima_get_stencil_op_string((*value & 0x00000038) >> 3), /* fail_op */
              (*value & 0x000001c0) >> 6,
              lima_get_stencil_op_string((*value & 0x000001c0) >> 6), /* zfail_op */
              (*value & 0x00000e00) >> 9,
              lima_get_stencil_op_string((*value & 0x00000e00) >> 9), /* zpass_op */
              (*value & 0x0000f000) >> 12); /* unknown */
      break;
   case 6: /* STENCIL BACK */
      fprintf(fp, "(1): valuemask 0x%02x, ref value %d (0x%02x), stencil_func %d (%s)*/\n",
              (*value & 0xff000000) >> 24, /* valuemask */
              (*value & 0x00ff0000) >> 16, (*value & 0x00ff0000) >> 16, /* ref value */
              (*value & 0x00000007),
              lima_get_compare_func_string((*value & 0x00000007))); /* stencil_func */
      /* add a few tabs for alignment */
      fprintf(fp, "\t\t\t\t\t\t/* %s(2)", render_state_infos[i].info);
      fprintf(fp, ": fail_op %d (%s), zfail_op %d (%s), zpass_op %d (%s), unknown (12-15) 0x%02x */\n",
              (*value & 0x00000038) >> 3,
              lima_get_stencil_op_string((*value & 0x00000038) >> 3), /* fail_op */
              (*value & 0x000001c0) >> 6,
              lima_get_stencil_op_string((*value & 0x000001c0) >> 6), /* zfail_op */
              (*value & 0x00000e00) >> 9,
              lima_get_stencil_op_string((*value & 0x00000e00) >> 9), /* zpass_op */
              (*value & 0x0000f000) >> 12); /* unknown */
      break;
   case 7: /* STENCIL TEST */
      fprintf(fp, "(1): stencil_front writemask 0x%02x, stencil_back writemask 0x%02x */\n",
              (*value & 0x000000ff), /* front writemask */
              (*value & 0x0000ff00) >> 8); /* back writemask */
      /* add a few tabs for alignment */
      fprintf(fp, "\t\t\t\t\t\t/* %s(2)", render_state_infos[i].info);
      fprintf(fp, ": alpha_ref_value: 0x%02x */\n", (*value & 0x00ff0000) >> 16);
      fprintf(fp, "\t\t\t\t\t\t/* %s(3)", render_state_infos[i].info);
      fprintf(fp, ": unknown (bits 24-31) 0x%02x */\n",
              (*value & 0xff000000) >> 24); /* unknown */
      break;
   case 8: /* MULTI SAMPLE */
      if ((*value & 0x00000f00) == 0x00000000)
         fprintf(fp, ": points");
      else if ((*value & 0x00000f00) == 0x00000400)
         fprintf(fp, ": lines");
      else if ((*value & 0x00000f00) == 0x00000800)
         fprintf(fp, ": triangles");
      else
         fprintf(fp, ": unknown");

      if ((*value & 0x00000078) == 0x00000068)
         fprintf(fp, ", msaa */\n");
      else if ((*value & 0x00000078) == 0x00000000)
         fprintf(fp, " */\n");
      else
         fprintf(fp, ", UNKNOWN */\n");

      fprintf(fp, "\t\t\t\t\t\t/* %s(3)", render_state_infos[i].info);
      fprintf(fp, ": sample_mask: 0x%.x", ((*value & 0xf000) >> 12));
      if ((*value & (1 << 7)))
         fprintf(fp, ", alpha_to_coverage");
      if ((*value & (1 << 8)))
         fprintf(fp, ", alpha_to_one");
      fprintf(fp, " */\n");

      fprintf(fp, "\t\t\t\t\t\t/* %s(4)", render_state_infos[i].info);
      fprintf(fp, ", register for gl_FragColor: $%d $%d $%d $%d */\n",
              (*value & 0xf0000000) >> 28,
              (*value & 0x0f000000) >> 24,
              (*value & 0x00f00000) >> 20,
              (*value & 0x000f0000) >> 16);
      fprintf(fp, "\t\t\t\t\t\t/* %s(3)", render_state_infos[i].info);
      fprintf(fp, ": alpha_test_func: %d (%s) */\n",
              (*value & 0x00000007),
              lima_get_compare_func_string((*value & 0x00000007))); /* alpha_test_func */
      break;
   case 9: /* SHADER ADDRESS */
      fprintf(fp, ": fs shader @ 0x%08x, first instr length %d */\n",
              *value & 0xffffffe0, *value & 0x0000001f);
      break;
   case 10: /* VARYING TYPES */
      fprintf(fp, "(1): ");
      int val, j;
      /* 0 - 5 */
      for (j = 0; j < 6; j++) {
         val = (*value >> (j * 3)) & 0x07;
         fprintf(fp, "val %d-%d, ", j, val);
      }
      /* 6 - 9 */
      /* add a few tabs for alignment */
      fprintf(fp, "\n\t\t\t\t\t\t/* %s(2): ", render_state_infos[i].info);
      for (j = 6; j < 10; j++) {
         val = (*value >> (j * 3)) & 0x07;
         fprintf(fp, "val %d-%d, ", j, val);
      }
      /* 10 */
      val = ((*value & 0xc0000000) >> 30) | ((*helper & 0x00000001) << 2);
      fprintf(fp, "val %d-%d, ", j, val);
      j++;
      /* 11 */
      val = (*helper & 0x0000000e) >> 1;
      fprintf(fp, "val %d-%d */\n", j, val);
      break;
   case 11: /* UNIFORMS ADDRESS */
      fprintf(fp, ": pp uniform info @ 0x%08x, bits: 0x%01x */\n",
              *value & 0xfffffff0, *value & 0x0000000f);
      break;
   case 12: /* TEXTURES ADDRESS */
      fprintf(fp, ": address: 0x%08x */\n", *value);
      break;
   case 13: /* AUX0 */
      fprintf(fp, "(1): varying_stride: %d", /* bits 0 - 4 varying stride, 8 aligned */
              (*value & 0x0000001f) << 3);
      if ((*value & 0x00000020) == 0x00000020) /* bit 5 has num_samplers */
         fprintf(fp, ", num_samplers %d",
                 (*value & 0xffffc000) >> 14); /* bits 14 - 31 num_samplers */

      if ((*value & 0x00000080) == 0x00000080) /* bit 7 has_fs_uniforms */
         fprintf(fp, ", has_fs_uniforms */");
      else
         fprintf(fp, " */");

      fprintf(fp, "\n\t\t\t\t\t\t/* %s(2):", render_state_infos[i].info);
      if ((*value & 0x00000200) == 0x00000200) /* bit 9 early-z */
         fprintf(fp, " early-z enabled");
      else
         fprintf(fp, " early-z disabled");

      if ((*value & 0x00001000) == 0x00001000) /* bit 12 pixel-kill */
         fprintf(fp, ", pixel kill enabled");
      else
         fprintf(fp, ", pixel kill disabled");

      if ((*value & 0x00000040) == 0x00000040) /* bit 6 unknown */
         fprintf(fp, ", bit 6 set");

      if ((*value & 0x00000100) == 0x00000100) /* bit 8 unknown */
         fprintf(fp, ", bit 8 set");

      if (((*value & 0x00000c00) >> 10) > 0) /* bit 10 - 11 unknown */
         fprintf(fp, ", bit 10 - 11: %d", ((*value & 0x00000c00) >> 10));

      if ((*value & 0x00002000) == 0x00002000) /* bit 13 unknown */
         fprintf(fp, ", bit 13 set");

      fprintf(fp, " */\n");
      fprintf(fp, "\n\t\t\t\t\t\t/* %s(3):", render_state_infos[i].info);
      fprintf(fp, " register for gl_SecondaryFragColor: $%d",
         (*value & 0xf0000000) >> 28);
      fprintf(fp, " */\n");
      break;
   case 14: /* AUX1 */
      fprintf(fp, ": ");
      if ((*value & 0x00002000) == 0x00002000)
         fprintf(fp, "blend->base.dither true, ");

      if ((*value & 0x00001000) == 0x00001000)
         fprintf(fp, "glFrontFace(GL_CCW), ");
      else
         fprintf(fp, "glFrontFace(GL_CW), ");

      if ((*value & 0x00010000) == 0x00010000)
         fprintf(fp, "ctx->const_buffer[PIPE_SHADER_FRAGMENT].buffer true ");
      fprintf(fp, "*/\n");
      break;
   case 15: /* VARYINGS ADDRESS */
      fprintf(fp, ": varyings @ 0x%08x */\n", *value & 0xfffffff0);
      break;
   default: /* should never be executed! */
      fprintf(fp, ": something went wrong!!! */\n");
      break;
   }
}

void
lima_parse_render_state(FILE *fp, uint32_t *data, int size, uint32_t start)
{
   uint32_t *value;

   fprintf(fp, "/* ============ RSW BEGIN ========================= */\n");
   for (int i = 0; i * 4 < size; i++) {
      value = &data[i];
      fprintf(fp, "/* 0x%08x (0x%08x) */\t0x%08x",
              start + i * 4, i * 4, *value);
      if (i == 10)
         parse_rsw(fp, value, i, &data[15]);
      else
         parse_rsw(fp, value, i, NULL);
   }
   fprintf(fp, "/* ============ RSW END =========================== */\n");
}

static void
parse_texture(FILE *fp, uint32_t *data, uint32_t start, uint32_t offset)
{
   uint32_t i = 0;
   offset /= 4;
   lima_tex_desc *desc = (lima_tex_desc *)&data[offset];

   /* Word 0 */
   fprintf(fp, "/* 0x%08x (0x%08x) */\t0x%08x\n",
           start + i * 4, i * 4, *(&data[i + offset]));
   i++;
   fprintf(fp, "\t format: 0x%x (%d)\n", desc->format, desc->format);
   fprintf(fp, "\t flag1: 0x%x (%d)\n", desc->flag1, desc->flag1);
   fprintf(fp, "\t swap_r_b: 0x%x (%d)\n", desc->swap_r_b, desc->swap_r_b);
   fprintf(fp, "\t unknown_0_1: 0x%x (%d)\n", desc->unknown_0_1, desc->unknown_0_1);
   fprintf(fp, "\t stride: 0x%x (%d)\n", desc->stride, desc->stride);
   fprintf(fp, "\t unknown_0_2: 0x%x (%d)\n", desc->unknown_0_2, desc->unknown_0_2);

   /* Word 1 - 5 */
   fprintf(fp, "/* 0x%08x (0x%08x) */\t0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
           start + i * 4, i * 4, *(&data[i + offset]), *(&data[i + 1 + offset]),
           *(&data[i + 2 + offset]), *(&data[i + 3 + offset]), *(&data[i + 4 + offset]));
   i += 5;
   fprintf(fp, "\t unknown_1_1: 0x%x (%d)\n", desc->unknown_1_1, desc->unknown_1_1);
   fprintf(fp, "\t unnorm_coords: 0x%x (%d)\n", desc->unnorm_coords, desc->unnorm_coords);
   fprintf(fp, "\t unknown_1_2: 0x%x (%d)\n", desc->unknown_1_2, desc->unknown_1_2);
   fprintf(fp, "\t cube_map: 0x%x (%d)\n", desc->cube_map, desc->cube_map);
   fprintf(fp, "\t sampler_dim: 0x%x (%d)\n", desc->sampler_dim, desc->sampler_dim);
   fprintf(fp, "\t min_lod: 0x%x (%d) (%f)\n", desc->min_lod, desc->min_lod, lima_fixed8_to_float(desc->min_lod));
   fprintf(fp, "\t max_lod: 0x%x (%d) (%f)\n", desc->max_lod, desc->max_lod, lima_fixed8_to_float(desc->max_lod));
   fprintf(fp, "\t lod_bias: 0x%x (%d) (%f)\n", desc->lod_bias, desc->lod_bias, lima_fixed8_to_float(desc->lod_bias));
   fprintf(fp, "\t unknown_2_1: 0x%x (%d)\n", desc->unknown_2_1, desc->unknown_2_1);
   fprintf(fp, "\t has_stride: 0x%x (%d)\n", desc->has_stride, desc->has_stride);
   fprintf(fp, "\t min_mipfilter_2: 0x%x (%d)\n", desc->min_mipfilter_2, desc->min_mipfilter_2);
   fprintf(fp, "\t min_img_filter_nearest: 0x%x (%d)\n", desc->min_img_filter_nearest, desc->min_img_filter_nearest);
   fprintf(fp, "\t mag_img_filter_nearest: 0x%x (%d)\n", desc->mag_img_filter_nearest, desc->mag_img_filter_nearest);
   fprintf(fp, "\t wrap_s: %d (%s)\n", desc->wrap_s,
           lima_get_wrap_mode_string(desc->wrap_s));
   fprintf(fp, "\t wrap_t: %d (%s)\n", desc->wrap_t,
           lima_get_wrap_mode_string(desc->wrap_t));
   fprintf(fp, "\t wrap_r: %d (%s)\n", desc->wrap_r,
           lima_get_wrap_mode_string(desc->wrap_r));
   fprintf(fp, "\t width: 0x%x (%d)\n", desc->width, desc->width);
   fprintf(fp, "\t height: 0x%x (%d)\n", desc->height, desc->height);
   fprintf(fp, "\t depth: 0x%x (%d)\n", desc->depth, desc->depth);
   fprintf(fp, "\t border_red: 0x%x (%d)\n", desc->border_red, desc->border_red);
   fprintf(fp, "\t border_green: 0x%x (%d)\n", desc->border_green, desc->border_green);
   fprintf(fp, "\t border_blue: 0x%x (%d)\n", desc->border_blue, desc->border_blue);
   fprintf(fp, "\t border_alpha: 0x%x (%d)\n", desc->border_alpha, desc->border_alpha);
   fprintf(fp, "\t unknown_5_1: 0x%x (%d)\n", desc->unknown_5_1, desc->unknown_5_1);

   /* Word 6 - */
   fprintf(fp, "/* 0x%08x (0x%08x) */",
           start + i * 4, i * 4);
   fprintf(fp, "\t");

   int miplevels = (int)lima_fixed8_to_float(desc->max_lod);
   for (int k = 0; k < ((((miplevels + 1) * 26) + 64) / 32); k++)
      fprintf(fp, "0x%08x ", *(&data[i + offset + k]));
   fprintf(fp, "\n");

   i++;
   fprintf(fp, "\t unknown_6_1: 0x%x (%d)\n", desc->va_s.unknown_6_1, desc->va_s.unknown_6_1);
   fprintf(fp, "\t layout: 0x%x (%d)\n", desc->va_s.layout, desc->va_s.layout);
   fprintf(fp, "\t unknown_6_2: 0x%x (%d)\n", desc->va_s.unknown_6_2, desc->va_s.unknown_6_2);
   fprintf(fp, "\t unknown_6_3: 0x%x (%d)\n", desc->va_s.unknown_6_3, desc->va_s.unknown_6_3);

   /* first level */
   fprintf(fp, "\t va_0: 0x%x \n", desc->va_s.va_0 << 6);

   /* second level up to desc->miplevels */
   int j;
   unsigned va_bit_idx;
   unsigned va_idx;
   uint32_t va;
   uint32_t va_1;
   uint32_t va_2;
   for (j = 1; j <= miplevels; j++) {
      va = 0;
      va_1 = 0;
      va_2 = 0;

      va_bit_idx = VA_BIT_OFFSET + (VA_BIT_SIZE * j);
      va_idx = va_bit_idx / 32;
      va_bit_idx %= 32;

      /* the first (32 - va_bit_idx) bits */
      va_1 |= (*(&data[i + offset + va_idx - 1]) >> va_bit_idx);

      /* do we need some bits from the following word? */
      if (va_bit_idx > 6) {
         /* shift left and right again to erase the unneeded bits, keep space for va1 */
         va_2 |= (*(&data[i + offset + va_idx]) << (2 * 32 - VA_BIT_SIZE - va_bit_idx));
         va_2 >>= ((2 * 32 - VA_BIT_SIZE - va_bit_idx) - (32 - va_bit_idx));
         va |= va_2;
      }
      va |= va_1;
      va <<= 6;
      fprintf(fp, "\t va_%d: 0x%x \n", j, va);
   }
}

void
lima_parse_texture_descriptor(FILE *fp, uint32_t *data, int size, uint32_t start, uint32_t offset)
{
   fprintf(fp, "/* ============ TEXTURE BEGIN ===================== */\n");
   parse_texture(fp, data, start, offset);
   fprintf(fp, "/* ============ TEXTURE END ======================= */\n");
}
