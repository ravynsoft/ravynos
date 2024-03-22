/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * Copyright 2010 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

#ifndef SP_TEX_SAMPLE_H
#define SP_TEX_SAMPLE_H


#include "tgsi/tgsi_exec.h"


struct sp_sampler_view;
struct sp_sampler;

typedef void (*wrap_nearest_func)(float s,
                                  unsigned size,
                                  int offset,
                                  int *icoord);

typedef void (*wrap_linear_func)(float s, 
                                 unsigned size,
                                 int offset,
                                 int *icoord0,
                                 int *icoord1,
                                 float *w);

typedef float (*compute_lambda_func)(const struct sp_sampler_view *sp_sview,
                                     const float s[TGSI_QUAD_SIZE],
                                     const float t[TGSI_QUAD_SIZE],
                                     const float p[TGSI_QUAD_SIZE]);

typedef float (*compute_lambda_from_grad_func)(const struct sp_sampler_view *sp_sview,
                                               const float derivs[3][2][TGSI_QUAD_SIZE],
                                               uint quad);

struct img_filter_args {
   float s;
   float t;
   float p;
   unsigned level;
   unsigned face_id;
   const int8_t *offset;
   bool gather_only;
   int gather_comp;
};

typedef void (*img_filter_func)(const struct sp_sampler_view *sp_sview,
                                const struct sp_sampler *sp_samp,
                                const struct img_filter_args *args,
                                float *rgba);

struct filter_args {
   enum tgsi_sampler_control control;
   const int8_t *offset;
   const uint *faces;
};

typedef void (*mip_filter_func)(const struct sp_sampler_view *sp_sview,
                                const struct sp_sampler *sp_samp,
                                img_filter_func min_filter,
                                img_filter_func mag_filter,
                                const float s[TGSI_QUAD_SIZE],
                                const float t[TGSI_QUAD_SIZE],
                                const float p[TGSI_QUAD_SIZE],
                                int gather_comp,
                                const float lod[TGSI_QUAD_SIZE],
                                const struct filter_args *args,
                                float rgba[TGSI_NUM_CHANNELS][TGSI_QUAD_SIZE]);


typedef void (*mip_level_func)(const struct sp_sampler_view *sp_sview,
                               const struct sp_sampler *sp_samp,
                               const float lod[TGSI_QUAD_SIZE],
                               float level[TGSI_QUAD_SIZE]);

typedef void (*fetch_func)(struct sp_sampler_view *sp_sview,
                           const int i[TGSI_QUAD_SIZE],
                           const int j[TGSI_QUAD_SIZE], const int k[TGSI_QUAD_SIZE],
                           const int lod[TGSI_QUAD_SIZE], const int8_t offset[3],
                           float rgba[TGSI_NUM_CHANNELS][TGSI_QUAD_SIZE]);


struct sp_sampler_view
{
   struct pipe_sampler_view base;

   /* For sp_get_samples_2d_linear_POT:
    */
   unsigned xpot;
   unsigned ypot;

   bool need_swizzle;
   bool pot2d;
   bool need_cube_convert;

   /* these are different per shader type */
   struct softpipe_tex_tile_cache *cache;
   compute_lambda_func compute_lambda;
   compute_lambda_from_grad_func compute_lambda_from_grad;
   union pipe_color_union border_color;
   /* Value to use for PIPE_SWIZZLE_1 (integer vs float) */
   float oneval;
};

struct sp_filter_funcs {
   mip_level_func relative_level;
   mip_filter_func filter;
};

struct sp_sampler {
   struct pipe_sampler_state base;

   bool min_mag_equal_repeat_linear;
   bool min_mag_equal;
   unsigned min_img_filter;

   wrap_nearest_func nearest_texcoord_s;
   wrap_nearest_func nearest_texcoord_t;
   wrap_nearest_func nearest_texcoord_p;

   wrap_linear_func linear_texcoord_s;
   wrap_linear_func linear_texcoord_t;
   wrap_linear_func linear_texcoord_p;

   const struct sp_filter_funcs *filter_funcs;
};


/**
 * Subclass of tgsi_sampler
 */
struct sp_tgsi_sampler
{
   struct tgsi_sampler base;  /**< base class */
   struct sp_sampler *sp_sampler[PIPE_MAX_SAMPLERS];
   struct sp_sampler_view sp_sview[PIPE_MAX_SHADER_SAMPLER_VIEWS];

};

compute_lambda_func
softpipe_get_lambda_func(const struct pipe_sampler_view *view,
                         enum pipe_shader_type shader);

compute_lambda_from_grad_func
softpipe_get_lambda_from_grad_func(const struct pipe_sampler_view *view,
                                   enum pipe_shader_type shader);

void *
softpipe_create_sampler_state(struct pipe_context *pipe,
                              const struct pipe_sampler_state *sampler);


struct pipe_sampler_view *
softpipe_create_sampler_view(struct pipe_context *pipe,
                             struct pipe_resource *resource,
                             const struct pipe_sampler_view *templ);


struct sp_tgsi_sampler *
sp_create_tgsi_sampler(void);


#endif /* SP_TEX_SAMPLE_H */
