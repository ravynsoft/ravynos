/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
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

/**
 * @file
 * C - JIT interfaces
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#ifndef LP_JIT_H
#define LP_JIT_H


#include "gallivm/lp_bld_struct.h"
#include "gallivm/lp_bld_limits.h"
#include "gallivm/lp_bld_jit_types.h"

#include "pipe/p_state.h"
#include "lp_texture.h"


struct lp_build_format_cache;
struct lp_fragment_shader_variant;
struct lp_compute_shader_variant;
struct lp_rast_state;
struct llvmpipe_screen;
struct vertex_header;

struct lp_jit_viewport
{
   float min_depth;
   float max_depth;
};

enum {
   LP_JIT_VIEWPORT_MIN_DEPTH,
   LP_JIT_VIEWPORT_MAX_DEPTH,
   LP_JIT_VIEWPORT_NUM_FIELDS /* number of fields above */
};

/**
 * This structure is passed directly to the generated fragment shader.
 *
 * It contains the derived state.
 *
 * Changes here must be reflected in the lp_jit_context_* macros and
 * lp_jit_init_types function. Changes to the ordering should be avoided.
 *
 * Only use types with a clear size and padding here, in particular prefer the
 * stdint.h types to the basic integer types.
 */
struct lp_jit_context
{
   float alpha_ref_value;

   uint32_t stencil_ref_front, stencil_ref_back;

   uint8_t *u8_blend_color;
   float *f_blend_color;

   struct lp_jit_viewport *viewports;

   uint32_t sample_mask;
};


/**
 * These enum values must match the position of the fields in the
 * lp_jit_context struct above.
 */
enum {
   LP_JIT_CTX_ALPHA_REF,
   LP_JIT_CTX_STENCIL_REF_FRONT,
   LP_JIT_CTX_STENCIL_REF_BACK,
   LP_JIT_CTX_U8_BLEND_COLOR,
   LP_JIT_CTX_F_BLEND_COLOR,
   LP_JIT_CTX_VIEWPORTS,
   LP_JIT_CTX_SAMPLE_MASK,
   LP_JIT_CTX_COUNT
};

#define lp_jit_context_alpha_ref_value(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CTX_ALPHA_REF, "alpha_ref_value")

#define lp_jit_context_stencil_ref_front_value(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CTX_STENCIL_REF_FRONT, "stencil_ref_front")

#define lp_jit_context_stencil_ref_back_value(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CTX_STENCIL_REF_BACK, "stencil_ref_back")

#define lp_jit_context_u8_blend_color(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CTX_U8_BLEND_COLOR, "u8_blend_color")

#define lp_jit_context_f_blend_color(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CTX_F_BLEND_COLOR, "f_blend_color")

#define lp_jit_context_viewports(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CTX_VIEWPORTS, "viewports")

#define lp_jit_context_sample_mask(_gallivm, _type, _ptr)                \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_CTX_SAMPLE_MASK, "sample_mask")


struct lp_jit_thread_data
{
   struct lp_build_format_cache *cache;
   uint64_t vis_counter;    // for occlusion query
   uint64_t ps_invocations; // pixel shader invocations

   /*
    * Non-interpolated rasterizer state passed through to the fragment shader.
    */
   struct {
      uint32_t viewport_index;
      uint32_t view_index;
   } raster_state;
};


enum {
   LP_JIT_THREAD_DATA_CACHE = 0,
   LP_JIT_THREAD_DATA_VIS_COUNTER,
   LP_JIT_THREAD_DATA_PS_INVOCATIONS,
   LP_JIT_THREAD_DATA_RASTER_STATE_VIEWPORT_INDEX,
   LP_JIT_THREAD_DATA_RASTER_STATE_VIEW_INDEX,
   LP_JIT_THREAD_DATA_COUNT
};


#define lp_jit_thread_data_cache(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_THREAD_DATA_CACHE, "cache")

#define lp_jit_thread_data_vis_counter(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_THREAD_DATA_VIS_COUNTER, "viscounter")

#define lp_jit_thread_data_ps_invocations(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_THREAD_DATA_PS_INVOCATIONS, "psinvocs")

#define lp_jit_thread_data_raster_state_viewport_index(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, \
                        LP_JIT_THREAD_DATA_RASTER_STATE_VIEWPORT_INDEX, \
                        "raster_state.viewport_index")

#define lp_jit_thread_data_raster_state_view_index(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, \
                        LP_JIT_THREAD_DATA_RASTER_STATE_VIEW_INDEX, \
                        "raster_state.view_index")

/**
 * typedef for fragment shader function
 *
 * @param context       jit context
 * @param x             block start x
 * @param y             block start y
 * @param facing        is front facing
 * @param a0            shader input a0
 * @param dadx          shader input dadx
 * @param dady          shader input dady
 * @param color         color buffer
 * @param depth         depth buffer
 * @param mask          mask of visible pixels in block (16-bits per sample)
 * @param thread_data   task thread data
 * @param stride        color buffer row stride in bytes
 * @param depth_stride  depth buffer row stride in bytes
 */
typedef void
(*lp_jit_frag_func)(const struct lp_jit_context *context,
                    const struct lp_jit_resources *res,
                    uint32_t x,
                    uint32_t y,
                    uint32_t facing,
                    const void *a0,
                    const void *dadx,
                    const void *dady,
                    uint8_t **color,
                    uint8_t *depth,
                    uint64_t mask,
                    struct lp_jit_thread_data *thread_data,
                    unsigned *stride,
                    unsigned depth_stride,
                    unsigned *color_sample_stride,
                    unsigned depth_sample_stride);


#define LP_MAX_LINEAR_CONSTANTS 16
#define LP_MAX_LINEAR_TEXTURES 2
#define LP_MAX_LINEAR_INPUTS 8


/**
 * This structure is passed directly to the generated fragment shader.
 *
 * It contains the derived state.
 *
 * Changes here must be reflected in the lp_jit_linear_context_* macros and
 * lp_jit_init_types function. Changes to the ordering should be avoided.
 *
 * Only use types with a clear size and padding here, in particular prefer the
 * stdint.h types to the basic integer types.
 */
struct lp_jit_linear_context
{
   /**
    * Constants in 8bit unorm values.
    */
   const uint8_t (*constants)[4];
   struct lp_linear_elem *tex[LP_MAX_LINEAR_TEXTURES];
   struct lp_linear_elem *inputs[LP_MAX_LINEAR_INPUTS];

   uint8_t *color0;
   uint32_t blend_color;

   uint8_t alpha_ref_value;
};


/**
 * These enum values must match the position of the fields in the
 * lp_jit_linear_context struct above.
 */
enum {
   LP_JIT_LINEAR_CTX_CONSTANTS = 0,
   LP_JIT_LINEAR_CTX_TEX,
   LP_JIT_LINEAR_CTX_INPUTS,
   LP_JIT_LINEAR_CTX_COLOR0,
   LP_JIT_LINEAR_CTX_BLEND_COLOR,
   LP_JIT_LINEAR_CTX_ALPHA_REF,
   LP_JIT_LINEAR_CTX_COUNT
};


#define lp_jit_linear_context_constants(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_LINEAR_CTX_CONSTANTS, "constants")

#define lp_jit_linear_context_tex(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_LINEAR_CTX_TEX, "tex")

#define lp_jit_linear_context_inputs(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_LINEAR_CTX_INPUTS, "inputs")

#define lp_jit_linear_context_color0(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_LINEAR_CTX_COLOR0, "color0")

#define lp_jit_linear_context_blend_color(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_LINEAR_CTX_BLEND_COLOR, "blend_color")

#define lp_jit_linear_context_alpha_ref(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_LINEAR_CTX_ALPHA_REF, "alpha_ref_value")


typedef const uint8_t *
(*lp_jit_linear_llvm_func)(struct lp_jit_linear_context *context,
                           uint32_t x,
                           uint32_t y,
                           uint32_t w);

/* We're not really jitting this, but I need to get into the
 * rast_state struct to call the function we actually are jitting.
 */
typedef bool
(*lp_jit_linear_func)(const struct lp_rast_state *state,
                      uint32_t x,
                      uint32_t y,
                      uint32_t w,
                      uint32_t h,
                      const float (*a0)[4],
                      const float (*dadx)[4],
                      const float (*dady)[4],
                      uint8_t *color,
                      uint32_t color_stride);


struct lp_jit_cs_thread_data
{
   struct lp_build_format_cache *cache;
   void *shared;
   void *payload;
};


enum {
   LP_JIT_CS_THREAD_DATA_CACHE = 0,
   LP_JIT_CS_THREAD_DATA_SHARED = 1,
   LP_JIT_CS_THREAD_DATA_PAYLOAD = 2,
   LP_JIT_CS_THREAD_DATA_COUNT
};


#define lp_jit_cs_thread_data_cache(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CS_THREAD_DATA_CACHE, "cache")

#define lp_jit_cs_thread_data_shared(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CS_THREAD_DATA_SHARED, "shared")

#define lp_jit_cs_thread_data_payload(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CS_THREAD_DATA_PAYLOAD, "payload")


struct lp_jit_cs_context
{
   const void *kernel_args;

   uint32_t shared_size;
};

/**
 * These enum values must match the position of the fields in the
 * lp_jit_context struct above.
 */
enum {
   LP_JIT_CS_CTX_KERNEL_ARGS,
   LP_JIT_CS_CTX_SHARED_SIZE,
   LP_JIT_CS_CTX_COUNT
};

#define lp_jit_cs_context_constants(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_CS_CTX_CONSTANTS, "constants")

#define lp_jit_cs_context_ssbos(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_CS_CTX_SSBOS, "ssbos")

#define lp_jit_cs_context_textures(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_CS_CTX_TEXTURES, "textures")

#define lp_jit_cs_context_samplers(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_CS_CTX_SAMPLERS, "samplers")

#define lp_jit_cs_context_images(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_CS_CTX_IMAGES, "images")

#define lp_jit_cs_context_aniso_filter_table(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CS_CTX_ANISO_FILTER_TABLE, "aniso_filter_table")

#define lp_jit_cs_context_kernel_args(_gallivm, _type, _ptr) \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_CS_CTX_KERNEL_ARGS, "kernel_args")

#define lp_jit_cs_context_shared_size(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_CS_CTX_SHARED_SIZE, "shared_size")

typedef void
(*lp_jit_cs_func)(const struct lp_jit_cs_context *context,
                  const struct lp_jit_resources *resources,
                  uint32_t x,
                  uint32_t y,
                  uint32_t z,
                  uint32_t grid_x,
                  uint32_t grid_y,
                  uint32_t grid_z,
                  uint32_t grid_size_x,
                  uint32_t grid_size_y,
                  uint32_t grid_size_z,
                  uint32_t work_dim,
                  uint32_t draw_id,
                  struct vertex_header *io, /* mesh shader only */
                  struct lp_jit_cs_thread_data *thread_data);

void
lp_jit_screen_cleanup(struct llvmpipe_screen *screen);

bool
lp_jit_screen_init(struct llvmpipe_screen *screen);

void
lp_jit_init_types(struct lp_fragment_shader_variant *lp);

void
lp_jit_init_cs_types(struct lp_compute_shader_variant *lp);

/* Helpers for converting pipe_* to lp_jit_* resources. */
void lp_jit_buffer_from_bda(struct lp_jit_buffer *jit, void *mem, size_t size);
void lp_jit_buffer_from_pipe(struct lp_jit_buffer *jit, const struct pipe_shader_buffer *buffer);
void lp_jit_buffer_from_pipe_const(struct lp_jit_buffer *jit, const struct pipe_constant_buffer *buffer, struct pipe_screen *screen);
void lp_jit_texture_from_pipe(struct lp_jit_texture *jit, const struct pipe_sampler_view *view);
void lp_jit_texture_buffer_from_bda(struct lp_jit_texture *jit, void *mem, size_t size, enum pipe_format format);
void lp_jit_sampler_from_pipe(struct lp_jit_sampler *jit, const struct pipe_sampler_state *sampler);
void lp_jit_image_from_pipe(struct lp_jit_image *jit, const struct pipe_image_view *view);
void lp_jit_image_buffer_from_bda(struct lp_jit_image *jit, void *mem, size_t size, enum pipe_format format);

#endif /* LP_JIT_H */
