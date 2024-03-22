/*
 * Copyright 2022 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LP_BLD_JIT_TYPES_H
#define LP_BLD_JIT_TYPES_H

#include "gallivm/lp_bld_limits.h"
#include "gallivm/lp_bld_sample.h"
#include "gallivm/lp_bld_struct.h"

struct lp_sampler_dynamic_state;

struct lp_jit_buffer
{
   union {
      const uint32_t *u;
      const float *f;
   };
   uint32_t num_elements;
};

enum {
   LP_JIT_BUFFER_BASE = 0,
   LP_JIT_BUFFER_NUM_ELEMENTS,
   LP_JIT_BUFFER_NUM_FIELDS,
};

LLVMValueRef
lp_llvm_descriptor_base(struct gallivm_state *gallivm,
                        LLVMValueRef buffers_ptr,
                        LLVMValueRef index, unsigned buffers_limit);

LLVMValueRef
lp_llvm_buffer_base(struct gallivm_state *gallivm,
                    LLVMValueRef buffers_ptr,
                    LLVMValueRef buffers_offset, unsigned buffers_limit);

LLVMValueRef
lp_llvm_buffer_num_elements(struct gallivm_state *gallivm,
                            LLVMValueRef buffers_ptr,
                            LLVMValueRef buffers_offset, unsigned buffers_limit);

#define LP_JIT_TEXTURE_SAMPLE_STRIDE 15 /* mip_offsets[15] */

struct lp_jit_texture
{
   const void *base;
   uint32_t width;        /* same as number of elements */
   uint16_t height;
   uint16_t depth;        /* doubles as array size */
   uint8_t first_level;
   uint8_t last_level;    /* contains num_samples for multisample */
   uint32_t row_stride[PIPE_MAX_TEXTURE_LEVELS];
   uint32_t img_stride[PIPE_MAX_TEXTURE_LEVELS];
   uint32_t mip_offsets[PIPE_MAX_TEXTURE_LEVELS]; /* sample stride is in mip_offsets[15] */
   uint32_t sampler_index;
};

enum {
   LP_JIT_TEXTURE_BASE = 0,
   LP_JIT_TEXTURE_WIDTH,
   LP_JIT_TEXTURE_HEIGHT,
   LP_JIT_TEXTURE_DEPTH,
   LP_JIT_TEXTURE_FIRST_LEVEL,
   LP_JIT_TEXTURE_LAST_LEVEL,
   LP_JIT_TEXTURE_ROW_STRIDE,
   LP_JIT_TEXTURE_IMG_STRIDE,
   LP_JIT_TEXTURE_MIP_OFFSETS,
   LP_JIT_SAMPLER_INDEX_DUMMY,
   LP_JIT_TEXTURE_NUM_FIELDS  /* number of fields above */
};

struct lp_jit_sampler
{
   float min_lod;
   float max_lod;
   float lod_bias;
   float border_color[4];
   float max_aniso;
};

enum {
   LP_JIT_SAMPLER_MIN_LOD,
   LP_JIT_SAMPLER_MAX_LOD,
   LP_JIT_SAMPLER_LOD_BIAS,
   LP_JIT_SAMPLER_BORDER_COLOR,
   LP_JIT_SAMPLER_MAX_ANISO,
   LP_JIT_SAMPLER_NUM_FIELDS  /* number of fields above */
};

struct lp_jit_image
{
   const void *base;
   uint32_t width;        /* same as number of elements */
   uint16_t height;
   uint16_t depth;
   uint8_t num_samples;
   uint32_t sample_stride;
   uint32_t row_stride;
   uint32_t img_stride;
};

enum {
   LP_JIT_IMAGE_BASE = 0,
   LP_JIT_IMAGE_WIDTH,
   LP_JIT_IMAGE_HEIGHT,
   LP_JIT_IMAGE_DEPTH,
   LP_JIT_IMAGE_NUM_SAMPLES,
   LP_JIT_IMAGE_SAMPLE_STRIDE,
   LP_JIT_IMAGE_ROW_STRIDE,
   LP_JIT_IMAGE_IMG_STRIDE,
   LP_JIT_IMAGE_NUM_FIELDS  /* number of fields above */
};

struct lp_jit_resources {
   struct lp_jit_buffer constants[LP_MAX_TGSI_CONST_BUFFERS];
   struct lp_jit_buffer ssbos[LP_MAX_TGSI_SHADER_BUFFERS];
   struct lp_jit_texture textures[PIPE_MAX_SHADER_SAMPLER_VIEWS];
   struct lp_jit_sampler samplers[PIPE_MAX_SAMPLERS];
   struct lp_jit_image images[PIPE_MAX_SHADER_IMAGES];
   const float *aniso_filter_table;
};

enum {
   LP_JIT_RES_CONSTANTS = 0,
   LP_JIT_RES_SSBOS,
   LP_JIT_RES_TEXTURES,
   LP_JIT_RES_SAMPLERS,
   LP_JIT_RES_IMAGES,
   LP_JIT_RES_ANISO_FILTER_TABLE,
   LP_JIT_RES_COUNT,
};

#define lp_jit_resources_constants(_gallivm, _type, _ptr)                \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_RES_CONSTANTS, "constants")

#define lp_jit_resources_ssbos(_gallivm, _type, _ptr)                    \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_RES_SSBOS, "ssbos")

#define lp_jit_resources_textures(_gallivm, _type, _ptr)                 \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_RES_TEXTURES, "textures")

#define lp_jit_resources_samplers(_gallivm, _type, _ptr)                 \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_RES_SAMPLERS, "samplers")

#define lp_jit_resources_images(_gallivm, _type, _ptr)                   \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_RES_IMAGES, "images")

#define lp_jit_resources_aniso_filter_table(_gallivm, _type, _ptr)       \
   lp_build_struct_get2(_gallivm, _type, _ptr, LP_JIT_RES_ANISO_FILTER_TABLE, "aniso_filter_table")

LLVMTypeRef
lp_build_jit_resources_type(struct gallivm_state *gallivm);

enum {
   LP_JIT_VERTEX_HEADER_VERTEX_ID = 0,
   LP_JIT_VERTEX_HEADER_CLIP_POS,
   LP_JIT_VERTEX_HEADER_DATA
};

#define lp_jit_vertex_header_id(_gallivm, _type, _ptr)              \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_VERTEX_HEADER_VERTEX_ID, "id")

#define lp_jit_vertex_header_clip_pos(_gallivm, _type, _ptr) \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_VERTEX_HEADER_CLIP_POS, "clip_pos")

#define lp_jit_vertex_header_data(_gallivm, _type, _ptr)            \
   lp_build_struct_get_ptr2(_gallivm, _type, _ptr, LP_JIT_VERTEX_HEADER_DATA, "data")

LLVMTypeRef
lp_build_create_jit_vertex_header_type(struct gallivm_state *gallivm, int data_elems);

void
lp_build_jit_fill_sampler_dynamic_state(struct lp_sampler_dynamic_state *state);
void
lp_build_jit_fill_image_dynamic_state(struct lp_sampler_dynamic_state *state);

LLVMTypeRef lp_build_sample_function_type(struct gallivm_state *gallivm, uint32_t sample_key);

LLVMTypeRef lp_build_size_function_type(struct gallivm_state *gallivm,
                                        const struct lp_sampler_size_query_params *params);

LLVMTypeRef lp_build_image_function_type(struct gallivm_state *gallivm,
                                         const struct lp_img_params *params, bool ms);

struct lp_texture_functions {
   void ***sample_functions;
   uint32_t sampler_count;

   void **fetch_functions;

   void *size_function;
   void *samples_function;

   void **image_functions;

   struct lp_static_texture_state state;
   uint32_t ref_count;

   bool sampled;
   bool storage;
};

struct lp_texture_handle {
   void *functions;
   uint32_t sampler_index;
};

struct lp_descriptor {
   union {
      struct {
         struct lp_jit_texture texture;
         struct lp_jit_sampler sampler;
      };
      struct {
         struct lp_jit_image image;
      };
      struct lp_jit_buffer buffer;
   };

   /* Store sample/image functions in the same location since some d3d12 games
    * rely on mismatched descriptor types with null descriptors.
    */
   void *functions;
};

#define LP_MAX_TEX_FUNC_ARGS 32

#endif

