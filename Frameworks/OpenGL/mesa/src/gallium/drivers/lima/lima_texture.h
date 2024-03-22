/*
 * Copyright (c) 2018-2019 Lima Project
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

#ifndef H_LIMA_TEXTURE
#define H_LIMA_TEXTURE

#define lima_min_tex_desc_size 64

#define LIMA_SAMPLER_DIM_1D   0
#define LIMA_SAMPLER_DIM_2D   1
#define LIMA_SAMPLER_DIM_3D   2

#define LIMA_TEX_WRAP_REPEAT                 0
#define LIMA_TEX_WRAP_CLAMP_TO_EDGE          1
#define LIMA_TEX_WRAP_CLAMP                  2
#define LIMA_TEX_WRAP_CLAMP_TO_BORDER        3
#define LIMA_TEX_WRAP_MIRROR_REPEAT          4
#define LIMA_TEX_WRAP_MIRROR_CLAMP_TO_EDGE   5
#define LIMA_TEX_WRAP_MIRROR_CLAMP           6
#define LIMA_TEX_WRAP_MIRROR_CLAMP_TO_BORDER 7

typedef struct __attribute__((__packed__)) {
   /* Word 0 */
   uint32_t format : 6;
   uint32_t flag1: 1;
   uint32_t swap_r_b: 1;
   uint32_t unknown_0_1: 8;
   uint32_t stride: 15;
   uint32_t unknown_0_2: 1;

   /* Word 1-3 */
   uint32_t unknown_1_1: 7;
   uint32_t unnorm_coords: 1;
   uint32_t unknown_1_2: 1;
   uint32_t cube_map: 1;
   uint32_t sampler_dim: 2;
   uint32_t min_lod: 8; /* Fixed point, 4.4, unsigned */
   uint32_t max_lod: 8; /* Fixed point, 4.4, unsigned */
   uint32_t lod_bias: 9; /* Fixed point, signed, 1.4.4 */
   uint32_t unknown_2_1: 3;
   uint32_t has_stride: 1;
   uint32_t min_mipfilter_2: 2; /* 0x3 for linear, 0x0 for nearest */
   uint32_t min_img_filter_nearest: 1;
   uint32_t mag_img_filter_nearest: 1;
   uint32_t wrap_s: 3;
   uint32_t wrap_t: 3;
   uint32_t wrap_r: 3;
   uint32_t width: 13;
   uint32_t height: 13;
   uint32_t depth: 13;

   uint32_t border_red: 16;
   uint32_t border_green: 16;
   uint32_t border_blue: 16;
   uint32_t border_alpha: 16;

   /* Word 5 (last 3 bits) */
   uint32_t unknown_5_1: 3;

   /* Word 6-15 */
   /* layout is in va[0] bit 13-14 */
   /* VAs start in va[0] at bit 30, each VA is 26 bits (only MSBs are stored), stored
    * linearly in memory */
   union {
      uint32_t va[0];
      struct __attribute__((__packed__)) {
         uint32_t unknown_6_1: 13;
         uint32_t layout: 2;
         uint32_t unknown_6_2: 9;
         uint32_t unknown_6_3: 6;
#define VA_BIT_OFFSET 30
#define VA_BIT_SIZE 26
         uint32_t va_0: VA_BIT_SIZE;
         uint32_t va_0_1: 8;
         uint32_t va_1_x[0];
      } va_s;
   };
} lima_tex_desc;

void lima_texture_desc_set_res(struct lima_context *ctx, lima_tex_desc *desc,
                               struct pipe_resource *prsc,
                               unsigned first_level, unsigned last_level,
                               unsigned first_layer, unsigned mrt_idx);
void lima_update_textures(struct lima_context *ctx);


static inline int16_t lima_float_to_fixed8(float f)
{
   return (int)(f * 16.0);
}

static inline float lima_fixed8_to_float(int16_t i)
{
   float sign = 1.0;

   if (i > 0xff) {
      i = 0x200 - i;
      sign = -1;
   }

   return sign * (float)(i / 16.0);
}

#endif
