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
 * Texture sampling.
 *
 * @author Jose Fonseca <jfonseca@vmware.com>
 */

#ifndef LP_BLD_SAMPLE_H
#define LP_BLD_SAMPLE_H


#include "pipe/p_state.h"
#include "util/format/u_formats.h"
#include "util/u_debug.h"
#include "gallivm/lp_bld.h"
#include "gallivm/lp_bld_type.h"
#include "gallivm/lp_bld_swizzle.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LP_MAX_TEXEL_BUFFER_ELEMENTS 134217728

struct util_format_description;
struct lp_type;
struct lp_build_context;


/**
 * Helper struct holding all derivatives needed for sampling
 */
struct lp_derivatives
{
   LLVMValueRef ddx[3];
   LLVMValueRef ddy[3];
};


enum lp_sampler_lod_property {
   LP_SAMPLER_LOD_SCALAR,
   LP_SAMPLER_LOD_PER_ELEMENT,
   LP_SAMPLER_LOD_PER_QUAD
};


enum lp_sampler_lod_control {
   LP_SAMPLER_LOD_IMPLICIT,
   LP_SAMPLER_LOD_BIAS,
   LP_SAMPLER_LOD_EXPLICIT,
   LP_SAMPLER_LOD_DERIVATIVES,
};


enum lp_sampler_op_type {
   LP_SAMPLER_OP_TEXTURE,
   LP_SAMPLER_OP_FETCH,
   LP_SAMPLER_OP_GATHER,
   LP_SAMPLER_OP_LODQ
};


#define LP_SAMPLER_SHADOW             (1 << 0)
#define LP_SAMPLER_OFFSETS            (1 << 1)
#define LP_SAMPLER_OP_TYPE_SHIFT            2
#define LP_SAMPLER_OP_TYPE_MASK       (3 << 2)
#define LP_SAMPLER_LOD_CONTROL_SHIFT        4
#define LP_SAMPLER_LOD_CONTROL_MASK   (3 << 4)
#define LP_SAMPLER_LOD_PROPERTY_SHIFT       6
#define LP_SAMPLER_LOD_PROPERTY_MASK  (3 << 6)
#define LP_SAMPLER_GATHER_COMP_SHIFT        8
#define LP_SAMPLER_GATHER_COMP_MASK   (3 << 8)
#define LP_SAMPLER_FETCH_MS          (1 << 10)


/* Parameters used to handle TEX instructions */
struct lp_sampler_params
{
   struct lp_type type;
   unsigned texture_index;
   unsigned sampler_index;
   LLVMValueRef texture_index_offset;
   unsigned sample_key;
   LLVMTypeRef resources_type;
   LLVMValueRef resources_ptr;
   LLVMTypeRef thread_data_type;
   LLVMValueRef thread_data_ptr;
   const LLVMValueRef *coords;
   const LLVMValueRef *offsets;
   LLVMValueRef ms_index;
   LLVMValueRef lod;
   LLVMValueRef aniso_filter_table;
   const struct lp_derivatives *derivs;
   LLVMValueRef *texel;

   LLVMValueRef texture_resource;
   LLVMValueRef sampler_resource;
   LLVMValueRef exec_mask;
};

/* Parameters used to handle sampler_size instructions */
struct lp_sampler_size_query_params
{
   struct lp_type int_type;
   unsigned texture_unit;
   LLVMValueRef texture_unit_offset;
   unsigned target;
   LLVMTypeRef resources_type;
   LLVMValueRef resources_ptr;
   bool is_sviewinfo;
   bool samples_only;
   bool ms;
   enum lp_sampler_lod_property lod_property;
   LLVMValueRef explicit_lod;
   LLVMValueRef *sizes_out;

   LLVMValueRef resource;
   LLVMValueRef exec_mask;
   enum pipe_format format;
};

#define LP_IMG_LOAD 0
#define LP_IMG_STORE 1
#define LP_IMG_ATOMIC 2
#define LP_IMG_ATOMIC_CAS 3
#define LP_IMG_OP_COUNT 4

struct lp_img_params
{
   struct lp_type type;
   unsigned image_index;
   LLVMValueRef image_index_offset;
   unsigned img_op;
   unsigned target;
   LLVMAtomicRMWBinOp op;
   LLVMValueRef exec_mask;
   LLVMTypeRef resources_type;
   LLVMValueRef resources_ptr;
   LLVMTypeRef thread_data_type;
   LLVMValueRef thread_data_ptr;
   const LLVMValueRef *coords;
   LLVMValueRef ms_index;
   LLVMValueRef indata[4];
   LLVMValueRef indata2[4];
   LLVMValueRef *outdata;

   LLVMValueRef resource;
   enum pipe_format format;
};


/**
 * Texture static state.
 *
 * These are the bits of state from pipe_resource/pipe_sampler_view that
 * are embedded in the generated code.
 */
struct lp_static_texture_state
{
   /* pipe_sampler_view's state */
   enum pipe_format format;
   enum pipe_format res_format;
   unsigned swizzle_r:3;     /**< PIPE_SWIZZLE_* */
   unsigned swizzle_g:3;
   unsigned swizzle_b:3;
   unsigned swizzle_a:3;

   /* pipe_texture's state */
   enum pipe_texture_target target:5;        /**< PIPE_TEXTURE_* */
   unsigned pot_width:1;     /**< is the width a power of two? */
   unsigned pot_height:1;
   unsigned pot_depth:1;
   unsigned level_zero_only:1;
};


/**
 * Sampler static state.
 *
 * These are the bits of state from pipe_sampler_state that
 * are embedded in the generated code.
 */
struct lp_static_sampler_state
{
   /* pipe_sampler_state's state */
   unsigned wrap_s:3;
   unsigned wrap_t:3;
   unsigned wrap_r:3;
   unsigned min_img_filter:2;
   unsigned min_mip_filter:2;
   unsigned mag_img_filter:2;
   unsigned compare_mode:1;
   unsigned compare_func:3;
   unsigned normalized_coords:1;
   unsigned min_max_lod_equal:1;  /**< min_lod == max_lod ? */
   unsigned lod_bias_non_zero:1;
   unsigned max_lod_pos:1;
   unsigned apply_min_lod:1;  /**< min_lod > 0 ? */
   unsigned apply_max_lod:1;  /**< max_lod < last_level ? */
   unsigned seamless_cube_map:1;
   unsigned aniso:1;
   unsigned reduction_mode:2;
};


/**
 * Sampler dynamic state.
 *
 * These are the bits of state from pipe_resource/pipe_sampler_view
 * as well as from sampler state that are computed at runtime.
 *
 * There are obtained through callbacks, as we don't want to tie the texture
 * sampling code generation logic to any particular texture layout or pipe
 * driver.
 */
struct lp_sampler_dynamic_state
{
   /* First callbacks for sampler view state */

   /** Obtain the base texture width (or number of elements) (returns int32) */
   LLVMValueRef
   (*width)(struct gallivm_state *gallivm,
            LLVMTypeRef resources_type,
            LLVMValueRef resources_ptr,
            unsigned texture_unit, LLVMValueRef texture_unit_offset);

   /** Obtain the base texture height (returns int32) */
   LLVMValueRef
   (*height)(struct gallivm_state *gallivm,
             LLVMTypeRef resources_type,
             LLVMValueRef resources_ptr,
             unsigned texture_unit, LLVMValueRef texture_unit_offset);

   /** Obtain the base texture depth (or array size) (returns int32) */
   LLVMValueRef
   (*depth)(struct gallivm_state *gallivm,
            LLVMTypeRef resources_type,
            LLVMValueRef resources_ptr,
            unsigned texture_unit, LLVMValueRef texture_unit_offset);

   /** Obtain the first mipmap level (base level) (returns int32) */
   LLVMValueRef
   (*first_level)(struct gallivm_state *gallivm,
                  LLVMTypeRef resources_type,
                  LLVMValueRef resources_ptr,
                  unsigned texture_unit, LLVMValueRef texture_unit_offset);

   /** Obtain the number of mipmap levels minus one (returns int32) */
   LLVMValueRef
   (*last_level)(struct gallivm_state *gallivm,
                 LLVMTypeRef resources_type,
                 LLVMValueRef resources_ptr,
                 unsigned texture_unit, LLVMValueRef texture_unit_offset);

   /** Obtain stride in bytes between image rows/blocks (returns int32) */
   LLVMValueRef
   (*row_stride)(struct gallivm_state *gallivm,
                 LLVMTypeRef resources_type,
                 LLVMValueRef resources_ptr,
                 unsigned texture_unit, LLVMValueRef texture_unit_offset,
                 LLVMTypeRef *out_type);

   /** Obtain stride in bytes between image slices (returns int32) */
   LLVMValueRef
   (*img_stride)(struct gallivm_state *gallivm,
                 LLVMTypeRef resources_type,
                 LLVMValueRef resources_ptr,
                 unsigned texture_unit, LLVMValueRef texture_unit_offset,\
                 LLVMTypeRef *out_type);

   /** Obtain pointer to base of texture */
   LLVMValueRef
   (*base_ptr)(struct gallivm_state *gallivm,
               LLVMTypeRef resources_type,
               LLVMValueRef resources_ptr,
               unsigned texture_unit, LLVMValueRef texture_unit_offset);

   /** Obtain pointer to array of mipmap offsets */
   LLVMValueRef
   (*mip_offsets)(struct gallivm_state *gallivm,
                  LLVMTypeRef resources_type,
                  LLVMValueRef resources_ptr,
                  unsigned texture_unit, LLVMValueRef texture_unit_offset,
                  LLVMTypeRef *out_type);

   /** Obtain number of samples (returns int32) */
   LLVMValueRef
   (*num_samples)(struct gallivm_state *gallivm,
                  LLVMTypeRef resources_type,
                  LLVMValueRef resources_ptr,
                  unsigned texture_unit, LLVMValueRef texture_unit_offset);

   /** Obtain multisample stride (returns int32) */
   LLVMValueRef
   (*sample_stride)(struct gallivm_state *gallivm,
                    LLVMTypeRef resources_type,
                    LLVMValueRef resources_ptr,
                    unsigned texture_unit, LLVMValueRef texture_unit_offset);

   /* These are callbacks for sampler state */

   /** Obtain texture min lod (returns float) */
   LLVMValueRef
   (*min_lod)(struct gallivm_state *gallivm,
              LLVMTypeRef resources_type,
              LLVMValueRef resources_ptr,
              unsigned sampler_unit);

   /** Obtain texture max lod (returns float) */
   LLVMValueRef
   (*max_lod)(struct gallivm_state *gallivm,
              LLVMTypeRef resources_type,
              LLVMValueRef resources_ptr,
              unsigned sampler_unit);

   /** Obtain texture lod bias (returns float) */
   LLVMValueRef
   (*lod_bias)(struct gallivm_state *gallivm,
               LLVMTypeRef resources_type,
               LLVMValueRef resources_ptr,
               unsigned sampler_unit);

   /** Obtain texture border color (returns ptr to float[4]) */
   LLVMValueRef
   (*border_color)(struct gallivm_state *gallivm,
                   LLVMTypeRef resources_type,
                   LLVMValueRef resources_ptr,
                   unsigned sampler_unit);

   /** Obtain maximum anisotropy */
   LLVMValueRef
   (*max_aniso)(struct gallivm_state *gallivm,
                LLVMTypeRef resources_type,
                LLVMValueRef resources_ptr,
                unsigned sampler_unit);

   /**
    * Obtain texture cache (returns ptr to lp_build_format_cache).
    *
    * It's optional: no caching will be done if it's NULL.
    */
   LLVMValueRef
   (*cache_ptr)(struct gallivm_state *gallivm,
                LLVMTypeRef thread_data_type,
                LLVMValueRef thread_data_ptr,
                unsigned unit);
};


struct lp_build_sampler_soa;
struct lp_build_image_soa;

struct lp_sampler_dynamic_state *
lp_build_sampler_soa_dynamic_state(struct lp_build_sampler_soa *sampler);

struct lp_sampler_dynamic_state *
lp_build_image_soa_dynamic_state(struct lp_build_image_soa *_image);

/**
 * Keep all information for sampling code generation in a single place.
 */
struct lp_build_sample_context
{
   struct gallivm_state *gallivm;

   const struct lp_static_texture_state *static_texture_state;
   const struct lp_static_sampler_state *static_sampler_state;

   struct lp_sampler_dynamic_state *dynamic_state;

   const struct util_format_description *format_desc;

   /* See texture_dims() */
   unsigned dims;

   /** SIMD vector width */
   unsigned vector_width;

   /** number of mipmaps (valid are 1, length/4, length) */
   unsigned num_mips;

   /** number of lod values (valid are 1, length/4, length) */
   unsigned num_lods;

   unsigned gather_comp;
   bool no_quad_lod;
   bool no_brilinear;
   bool no_rho_approx;
   bool fetch_ms;

   /** regular scalar float type */
   struct lp_type float_type;
   struct lp_build_context float_bld;

   /** float vector type */
   struct lp_build_context float_vec_bld;

   /** regular scalar int type */
   struct lp_type int_type;
   struct lp_build_context int_bld;

   /** Incoming coordinates type and build context */
   struct lp_type coord_type;
   struct lp_build_context coord_bld;

   /** Signed integer coordinates */
   struct lp_type int_coord_type;
   struct lp_build_context int_coord_bld;

   /** Unsigned integer texture size */
   struct lp_type int_size_in_type;
   struct lp_build_context int_size_in_bld;

   /** Float incoming texture size */
   struct lp_type float_size_in_type;
   struct lp_build_context float_size_in_bld;

   /** Unsigned integer texture size (might be per quad) */
   struct lp_type int_size_type;
   struct lp_build_context int_size_bld;

   /** Float texture size (might be per quad) */
   struct lp_type float_size_type;
   struct lp_build_context float_size_bld;

   /** Output texels type and build context */
   struct lp_type texel_type;
   struct lp_build_context texel_bld;

   /** Float level type */
   struct lp_type levelf_type;
   struct lp_build_context levelf_bld;

   /** Int level type */
   struct lp_type leveli_type;
   struct lp_build_context leveli_bld;

   /** Float lod type */
   struct lp_type lodf_type;
   struct lp_build_context lodf_bld;

   /** Int lod type */
   struct lp_type lodi_type;
   struct lp_build_context lodi_bld;

   /* Common dynamic state values */
   LLVMTypeRef row_stride_type;
   LLVMValueRef row_stride_array;
   LLVMTypeRef img_stride_type;
   LLVMValueRef img_stride_array;
   LLVMValueRef base_ptr;
   LLVMTypeRef mip_offsets_type;
   LLVMValueRef mip_offsets;
   LLVMValueRef cache;

   /** Integer vector with texture width, height, depth */
   LLVMValueRef int_size;
   LLVMValueRef int_tex_blocksize;
   LLVMValueRef int_tex_blocksize_log2;
   LLVMValueRef int_view_blocksize;

   LLVMValueRef border_color_clamped;

   LLVMTypeRef resources_type;
   LLVMValueRef resources_ptr;

   LLVMValueRef aniso_filter_table;
};

/*
 * Indirect texture access context
 *
 * This is used to store info across building
 * and indirect texture switch statement.
 */
struct lp_build_sample_array_switch {
   struct gallivm_state *gallivm;
   struct lp_sampler_params params;
   unsigned base, range;
   LLVMValueRef switch_ref;
   LLVMBasicBlockRef merge_ref;
   LLVMValueRef phi;
};

struct lp_build_img_op_array_switch {
   struct gallivm_state *gallivm;
   struct lp_img_params params;
   unsigned base, range;
   LLVMValueRef switch_ref;
   LLVMBasicBlockRef merge_ref;
   LLVMValueRef phi[4];
};


/**
 * We only support a few wrap modes in lp_build_sample_wrap_linear_int() at
 * this time.  Return whether the given mode is supported by that function.
 */
static inline bool
lp_is_simple_wrap_mode(unsigned mode)
{
   switch (mode) {
   case PIPE_TEX_WRAP_REPEAT:
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      return true;
   default:
      return false;
   }
}


static inline void
apply_sampler_swizzle(struct lp_build_sample_context *bld,
                      LLVMValueRef *texel)
{
   unsigned char swizzles[4];

   swizzles[0] = bld->static_texture_state->swizzle_r;
   swizzles[1] = bld->static_texture_state->swizzle_g;
   swizzles[2] = bld->static_texture_state->swizzle_b;
   swizzles[3] = bld->static_texture_state->swizzle_a;

   lp_build_swizzle_soa_inplace(&bld->texel_bld, texel, swizzles);
}

/*
 * not really dimension as such, this indicates the amount of
 * "normal" texture coords subject to minification, wrapping etc.
 */
static inline unsigned
texture_dims(enum pipe_texture_target tex)
{
   switch (tex) {
   case PIPE_TEXTURE_1D:
   case PIPE_TEXTURE_1D_ARRAY:
   case PIPE_BUFFER:
      return 1;
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_RECT:
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      return 2;
   case PIPE_TEXTURE_3D:
      return 3;
   default:
      assert(0 && "bad texture target in texture_dims()");
      return 2;
   }
}


static inline bool
has_layer_coord(enum pipe_texture_target tex)
{
   switch (tex) {
   case PIPE_TEXTURE_1D_ARRAY:
   case PIPE_TEXTURE_2D_ARRAY:
   /* cube is not layered but 3rd coord (after cube mapping) behaves the same */
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      return true;
   default:
      return false;
   }
}


bool
lp_sampler_wrap_mode_uses_border_color(enum pipe_tex_wrap mode,
                                       enum pipe_tex_filter min_img_filter,
                                       enum pipe_tex_filter mag_img_filter);

/**
 * Derive the sampler static state.
 */
void
lp_sampler_static_sampler_state(struct lp_static_sampler_state *state,
                                const struct pipe_sampler_state *sampler);


void
lp_sampler_static_texture_state(struct lp_static_texture_state *state,
                                const struct pipe_sampler_view *view);

void
lp_sampler_static_texture_state_image(struct lp_static_texture_state *state,
                                      const struct pipe_image_view *view);

void
lp_build_lod_selector(struct lp_build_sample_context *bld,
                      bool is_lodq,
                      unsigned sampler_index,
                      LLVMValueRef first_level,
                      LLVMValueRef s,
                      LLVMValueRef t,
                      LLVMValueRef r,
                      const struct lp_derivatives *derivs,
                      LLVMValueRef lod_bias, /* optional */
                      LLVMValueRef explicit_lod, /* optional */
                      enum pipe_tex_mipfilter mip_filter,
                      LLVMValueRef max_aniso,
                      LLVMValueRef *out_lod,
                      LLVMValueRef *out_lod_ipart,
                      LLVMValueRef *out_lod_fpart,
                      LLVMValueRef *out_lod_positive);

void
lp_build_nearest_mip_level(struct lp_build_sample_context *bld,
                           LLVMValueRef first_level,
                           LLVMValueRef last_level,
                           LLVMValueRef lod,
                           LLVMValueRef *level_out,
                           LLVMValueRef *out_of_bounds);

void
lp_build_linear_mip_levels(struct lp_build_sample_context *bld,
                           unsigned texture_unit,
                           LLVMValueRef first_level,
                           LLVMValueRef last_level,
                           LLVMValueRef lod_ipart,
                           LLVMValueRef *lod_fpart_inout,
                           LLVMValueRef *level0_out,
                           LLVMValueRef *level1_out);

LLVMValueRef
lp_build_get_mipmap_level(struct lp_build_sample_context *bld,
                          LLVMValueRef level);


LLVMValueRef
lp_build_get_mip_offsets(struct lp_build_sample_context *bld,
                         LLVMValueRef level);


void
lp_build_mipmap_level_sizes(struct lp_build_sample_context *bld,
                            LLVMValueRef ilevel,
                            LLVMValueRef *out_size_vec,
                            LLVMValueRef *row_stride_vec,
                            LLVMValueRef *img_stride_vec);

LLVMValueRef
lp_build_scale_view_dims(struct lp_build_context *bld, LLVMValueRef size,
                         LLVMValueRef tex_blocksize,
                         LLVMValueRef tex_blocksize_log2,
                         LLVMValueRef view_blocksize);

LLVMValueRef
lp_build_scale_view_dim(struct gallivm_state *gallivm, LLVMValueRef size,
                        unsigned tex_blocksize, unsigned view_blocksize);

void
lp_build_extract_image_sizes(struct lp_build_sample_context *bld,
                             struct lp_build_context *size_bld,
                             struct lp_type coord_type,
                             LLVMValueRef size,
                             LLVMValueRef *out_width,
                             LLVMValueRef *out_height,
                             LLVMValueRef *out_depth);


void
lp_build_unnormalized_coords(struct lp_build_sample_context *bld,
                             LLVMValueRef flt_size,
                             LLVMValueRef *s,
                             LLVMValueRef *t,
                             LLVMValueRef *r);


void
lp_build_cube_lookup(struct lp_build_sample_context *bld,
                     LLVMValueRef *coords,
                     const struct lp_derivatives *derivs_in, /* optional */
                     struct lp_derivatives *derivs_out, /* optional */
                     bool need_derivs);


void
lp_build_cube_new_coords(struct lp_build_context *ivec_bld,
                         LLVMValueRef face,
                         LLVMValueRef x0,
                         LLVMValueRef x1,
                         LLVMValueRef y0,
                         LLVMValueRef y1,
                         LLVMValueRef max_coord,
                         LLVMValueRef new_faces[4],
                         LLVMValueRef new_xcoords[4][2],
                         LLVMValueRef new_ycoords[4][2]);


void
lp_build_sample_partial_offset(struct lp_build_context *bld,
                               unsigned block_length,
                               LLVMValueRef coord,
                               LLVMValueRef stride,
                               LLVMValueRef *out_offset,
                               LLVMValueRef *out_i);


void
lp_build_sample_offset(struct lp_build_context *bld,
                       const struct util_format_description *format_desc,
                       LLVMValueRef x,
                       LLVMValueRef y,
                       LLVMValueRef z,
                       LLVMValueRef y_stride,
                       LLVMValueRef z_stride,
                       LLVMValueRef *out_offset,
                       LLVMValueRef *out_i,
                       LLVMValueRef *out_j);


void
lp_build_sample_soa_code(struct gallivm_state *gallivm,
                         const struct lp_static_texture_state *static_texture_state,
                         const struct lp_static_sampler_state *static_sampler_state,
                         struct lp_sampler_dynamic_state *dynamic_state,
                         struct lp_type type,
                         unsigned sample_key,
                         unsigned texture_index,
                         unsigned sampler_index,
                         LLVMTypeRef resources_type,
                         LLVMValueRef resources_ptr,
                         LLVMTypeRef thread_data_type,
                         LLVMValueRef thread_data_ptr,
                         const LLVMValueRef *coords,
                         const LLVMValueRef *offsets,
                         const struct lp_derivatives *derivs, /* optional */
                         LLVMValueRef lod, /* optional */
                         LLVMValueRef ms_index, /* optional */
                         LLVMValueRef aniso_filter_table,
                         LLVMValueRef texel_out[4]);


void
lp_build_sample_soa(const struct lp_static_texture_state *static_texture_state,
                    const struct lp_static_sampler_state *static_sampler_state,
                    struct lp_sampler_dynamic_state *dynamic_texture_state,
                    struct gallivm_state *gallivm,
                    const struct lp_sampler_params *params);


void
lp_build_coord_repeat_npot_linear(struct lp_build_sample_context *bld,
                                  LLVMValueRef coord_f,
                                  LLVMValueRef length_i,
                                  LLVMValueRef length_f,
                                  LLVMValueRef *coord0_i,
                                  LLVMValueRef *weight_f);


void
lp_build_size_query_soa(struct gallivm_state *gallivm,
                        const struct lp_static_texture_state *static_state,
                        struct lp_sampler_dynamic_state *dynamic_state,
                        const struct lp_sampler_size_query_params *params);

void
lp_build_sample_nop(struct gallivm_state *gallivm,
                    struct lp_type type,
                    const LLVMValueRef *coords,
                    LLVMValueRef texel_out[4]);


LLVMValueRef
lp_build_minify(struct lp_build_context *bld,
                LLVMValueRef base_size,
                LLVMValueRef level,
                bool lod_scalar);

void
lp_build_img_op_soa(const struct lp_static_texture_state *static_texture_state,
                    struct lp_sampler_dynamic_state *dynamic_state,
                    struct gallivm_state *gallivm,
                    const struct lp_img_params *params,
                    LLVMValueRef outdata[4]);

void
lp_build_sample_array_init_soa(struct lp_build_sample_array_switch *switch_info,
                           struct gallivm_state *gallivm,
                           const struct lp_sampler_params *params,
                           LLVMValueRef idx,
                           unsigned base, unsigned range);

void
lp_build_sample_array_case_soa(struct lp_build_sample_array_switch *switch_info,
                           int idx,
                           const struct lp_static_texture_state *static_texture_state,
                           const struct lp_static_sampler_state *static_sampler_state,
                           struct lp_sampler_dynamic_state *dynamic_texture_state);

void
lp_build_sample_array_fini_soa(struct lp_build_sample_array_switch *switch_info);

void
lp_build_image_op_switch_soa(struct lp_build_img_op_array_switch *switch_info,
                             struct gallivm_state *gallivm,
                             const struct lp_img_params *params,
                             LLVMValueRef idx,
                             unsigned base, unsigned range);

void
lp_build_image_op_array_case(struct lp_build_img_op_array_switch *switch_info,
                             int idx,
                             const struct lp_static_texture_state *static_texture_state,
                             struct lp_sampler_dynamic_state *dynamic_state);

void
lp_build_image_op_array_fini_soa(struct lp_build_img_op_array_switch *switch_info);

void
lp_build_reduce_filter(struct lp_build_context *bld,
                       enum pipe_tex_reduction_mode mode,
                       unsigned flags,
                       unsigned num_chan,
                       LLVMValueRef x,
                       LLVMValueRef *v00,
                       LLVMValueRef *v01,
                       LLVMValueRef *out);
void
lp_build_reduce_filter_2d(struct lp_build_context *bld,
                          enum pipe_tex_reduction_mode mode,
                          unsigned flags,
                          unsigned num_chan,
                          LLVMValueRef x,
                          LLVMValueRef y,
                          LLVMValueRef *v00,
                          LLVMValueRef *v01,
                          LLVMValueRef *v10,
                          LLVMValueRef *v11,
                          LLVMValueRef *out);

void
lp_build_reduce_filter_3d(struct lp_build_context *bld,
                          enum pipe_tex_reduction_mode mode,
                          unsigned flags,
                          unsigned num_chan,
                          LLVMValueRef x,
                          LLVMValueRef y,
                          LLVMValueRef z,
                          LLVMValueRef *v000,
                          LLVMValueRef *v001,
                          LLVMValueRef *v010,
                          LLVMValueRef *v011,
                          LLVMValueRef *v100,
                          LLVMValueRef *v101,
                          LLVMValueRef *v110,
                          LLVMValueRef *v111,
                          LLVMValueRef *out);

struct lp_type
lp_build_texel_type(struct lp_type texel_type,
                    const struct util_format_description *format_desc);

LLVMValueRef lp_sample_load_mip_value(struct gallivm_state *gallivm,
                                      LLVMTypeRef ptr_type,
                                      LLVMValueRef offsets,
                                      LLVMValueRef index1);

const float *lp_build_sample_aniso_filter_table(void);
#ifdef __cplusplus
}
#endif

#endif /* LP_BLD_SAMPLE_H */
