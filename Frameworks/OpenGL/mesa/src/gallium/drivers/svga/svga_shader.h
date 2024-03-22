/**********************************************************
 * Copyright 2008-2023 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#ifndef SVGA_SHADER_H
#define SVGA_SHADER_H

#include "svga3d_reg.h"
#include "svga_context.h"
#include "svga_streamout.h"
#include "compiler/shader_enums.h"
#include "tgsi/tgsi_scan.h"


/**
 * We use a 64-bit mask to keep track of the generic indexes.
 * This is the maximum semantic index for a TGSI GENERIC[i] register.
 */
#define MAX_GENERIC_VARYING 64


struct svga_context;


struct svga_compile_key
{
   /* vertex shader only */
   struct {
      uint64_t fs_generic_inputs;
      unsigned passthrough:1;
      unsigned need_prescale:1;
      unsigned undo_viewport:1;
      unsigned allow_psiz:1;
      unsigned need_vertex_id_bias:1;

      /** The following are all 32-bit bitmasks (per VS input) */
      unsigned adjust_attrib_range;
      unsigned attrib_is_pure_int;
      unsigned adjust_attrib_w_1;
      unsigned adjust_attrib_itof;
      unsigned adjust_attrib_utof;
      unsigned attrib_is_bgra;
      unsigned attrib_puint_to_snorm;
      unsigned attrib_puint_to_uscaled;
      unsigned attrib_puint_to_sscaled;
   } vs;

   /* geometry shader only */
   struct {
      uint64_t vs_generic_outputs;
      unsigned need_prescale:1;
      unsigned writes_psize:1;
      unsigned wide_point:1;
      unsigned writes_viewport_index:1;
      unsigned num_prescale:5;
   } gs;

   /* fragment shader only */
   struct {
      uint64_t vs_generic_outputs;
      uint64_t gs_generic_outputs;
      unsigned light_twoside:1;
      unsigned front_ccw:1;
      unsigned white_fragments:1;
      unsigned alpha_to_one:1;
      unsigned flatshade:1;
      unsigned pstipple:1;
      unsigned alpha_func:4;  /**< SVGA3D_CMP_x */
      unsigned write_color0_to_n_cbufs:4;
      unsigned aa_point:1;
      unsigned layer_to_zero:1;
      int aa_point_coord_index;
      float alpha_ref;
   } fs;

   /* tessellation control shader */
   struct {
      unsigned vertices_per_patch:8;
      unsigned vertices_out:8;
      enum mesa_prim prim_mode:8;
      enum pipe_tess_spacing spacing:3;
      unsigned vertices_order_cw:1;
      unsigned point_mode:1;
      unsigned passthrough:1;
   } tcs;

   /* tessellation evaluation shader */
   struct {
      unsigned vertices_per_patch:8;
      unsigned tessfactor_index:8;
      unsigned need_prescale:1;
      unsigned need_tessouter:1;
      unsigned need_tessinner:1;
   } tes;

   /* compute shader */
   struct {
      unsigned grid_size[3];
      unsigned mem_size;
   } cs;

   /* any shader type */
   int8_t generic_remap_table[MAX_GENERIC_VARYING];
   unsigned num_textures:8;
   unsigned num_samplers:8;
   unsigned num_unnormalized_coords:8;
   unsigned clip_plane_enable:PIPE_MAX_CLIP_PLANES;
   unsigned last_vertex_stage:1;
   unsigned clamp_vertex_color:1;
   unsigned sampler_state_mapping:1;    /* Set if use sampler state mapping */
   unsigned sprite_origin_lower_left:1;
   uint16_t sprite_coord_enable;
   struct {
      unsigned compare_mode:1;
      unsigned compare_func:3;
      unsigned compare_in_shader:1;
      unsigned unnormalized:1;
      unsigned texel_bias:1;
      unsigned width_height_idx:5; /**< texture unit */
      unsigned is_array:1;
      unsigned swizzle_r:3;
      unsigned swizzle_g:3;
      unsigned swizzle_b:3;
      unsigned swizzle_a:3;
      unsigned num_samples:5;   /**< Up to 16 samples */
      unsigned target:4;
      unsigned sampler_return_type:4;
      unsigned sampler_view:1;
      unsigned sampler_index:5;
   } tex[PIPE_MAX_SAMPLERS];

   unsigned uav_splice_index:4;      /* starting uav index */
   unsigned srv_raw_constbuf_index:8;   /* start index for srv raw buffers */
   unsigned srv_raw_shaderbuf_index:8;  /* start index for srv raw shader bufs */
   unsigned image_size_used:1;

   uint16_t raw_constbufs;           /* bitmask of raw constant buffers */
   uint64_t raw_shaderbufs;          /* bitmask of raw shader buffers */

   struct {
      enum tgsi_return_type return_type;
      enum pipe_texture_target resource_target;
      unsigned is_array:1;
      unsigned is_single_layer:1;
      unsigned uav_index;
   } images[PIPE_MAX_SHADER_IMAGES];

   uint32_t shader_buf_uav_index[PIPE_MAX_SHADER_BUFFERS];
   uint32_t atomic_buf_uav_index[PIPE_MAX_HW_ATOMIC_BUFFERS];
};

/* A key for a variant of token string of a shader */
struct svga_token_key {
   struct {
      unsigned sprite_coord_enable:24;
      unsigned sprite_origin_upper_left:1;
      unsigned point_pos_stream_out:1;
      unsigned writes_psize:1;
      unsigned aa_point:1;
   } gs;
   struct {
      unsigned write_position:1;
   } vs;
   unsigned dynamic_indexing:1;
};

/**
 * A single TGSI shader may be compiled into different variants of
 * SVGA3D shaders depending on the compile key.  Each user shader
 * will have a linked list of these variants.
 */
struct svga_shader_variant
{
   const struct svga_shader *shader;

   /** Parameters used to generate this variant */
   struct svga_compile_key key;

   /* svga shader type */
   SVGA3dShaderType type;

   /* Compiled shader tokens:
    */
   const unsigned *tokens;
   unsigned nr_tokens;

   /* shader signature */
   unsigned signatureLen;
   SVGA3dDXShaderSignatureHeader *signature;

   /** Per-context shader identifier used with SVGA_3D_CMD_SHADER_DEFINE,
    * SVGA_3D_CMD_SET_SHADER and SVGA_3D_CMD_SHADER_DESTROY.
    */
   unsigned id;

   /** Start of extra constants (number of float[4] constants) */
   unsigned extra_const_start;

   /* GB object buffer containing the bytecode */
   struct svga_winsys_gb_shader *gb_shader;

   /** Next variant */
   struct svga_shader_variant *next;
};


/**
 * Shader variant for fragment shader
 */
struct svga_fs_variant
{
   struct svga_shader_variant base;

   bool uses_flat_interp;   /** TRUE if flat interpolation qualifier is
                                *  applied to any of the varyings.
                                */

   /** Is the color output just a constant value? (fragment shader only) */
   bool constant_color_output;

   /** Bitmask indicating which texture units are doing the shadow
    * comparison test in the shader rather than the sampler state.
    */
   unsigned fs_shadow_compare_units;

   /** For FS-based polygon stipple */
   unsigned pstipple_sampler_unit:8;
   unsigned pstipple_sampler_state_index:8;
};


/**
 * Shader variant for geometry shader
 */
struct svga_gs_variant
{
   struct svga_shader_variant base;
};


/**
 * Shader variant for vertex shader
 */
struct svga_vs_variant
{
   struct svga_shader_variant base;
};


/**
 * Shader variant for tessellation evaluation shader
 */
struct svga_tes_variant
{
   struct svga_shader_variant base;

   enum mesa_prim prim_mode:8;
   enum pipe_tess_spacing spacing:3;
   unsigned vertices_order_cw:1;
   unsigned point_mode:1;
};


/**
 * Shader variant for tessellation control shader
 */
struct svga_tcs_variant
{
   struct svga_shader_variant base;
};


/**
 * Shader variant for compute shader
 */
struct svga_cs_variant
{
   struct svga_shader_variant base;
};


struct svga_shader_info
{
   uint8_t num_inputs;
   uint8_t num_outputs;

   uint8_t input_semantic_name[PIPE_MAX_SHADER_INPUTS];
   uint8_t input_semantic_index[PIPE_MAX_SHADER_INPUTS];
   uint8_t input_usage_mask[PIPE_MAX_SHADER_INPUTS];
   uint8_t output_semantic_name[PIPE_MAX_SHADER_OUTPUTS];
   uint8_t output_semantic_index[PIPE_MAX_SHADER_OUTPUTS];
   uint8_t output_usage_mask[PIPE_MAX_SHADER_OUTPUTS];

   uint64_t generic_inputs_mask;
   uint64_t generic_outputs_mask;

   bool writes_edgeflag;
   bool writes_layer;
   bool writes_position;
   bool writes_psize;
   bool writes_viewport_index;

   bool uses_grid_size;
   bool uses_const_buffers;
   bool uses_hw_atomic;
   bool uses_images;
   bool uses_image_size;
   bool uses_shader_buffers;
   bool uses_samplers;

   unsigned const_buffers_declared;  /* bitmask of declared const buffers */
   unsigned constbuf0_num_uniforms;  /* number of uniforms in constbuf0 */

   unsigned shader_buffers_declared;  /* bitmask of declared shader buffers */

   struct {
      bool color0_writes_all_cbufs;
   } fs;

  struct {
      enum mesa_prim in_prim;
      enum mesa_prim out_prim;
   } gs;

   struct {
      unsigned vertices_out;        /* number of vertices in tcs patch */
      bool writes_tess_factor;
   } tcs;

   struct {
      enum mesa_prim prim_mode;
      bool reads_control_point;
   } tes;
};


struct svga_shader
{
   enum pipe_shader_ir type;            /* IR type */
   enum pipe_shader_type stage;         /* shader stage */

   struct svga_shader_info info;        /* shader info */

   /* TGSI */
   const struct tgsi_token *tokens;
   struct svga_token_key token_key;     /* token key for the token string */
   struct tgsi_shader_info tgsi_info;

   /* List of shaders with tokens derived from the same token string */
   struct svga_shader *next;
   struct svga_shader *parent;   /* shader with the original token string */

   struct svga_stream_output *stream_output;

   /** Head of linked list of compiled variants */
   struct svga_shader_variant *variants;

   /* Get dummy shader variant */
   struct svga_shader_variant *(*get_dummy_shader)(struct svga_context *,
                                                   struct svga_shader *,
                                                   const struct svga_compile_key *);

   unsigned id;  /**< for debugging only */
};


struct svga_fragment_shader
{
   struct svga_shader base;

   struct draw_fragment_shader *draw_shader;

   /** Mask of which generic varying variables are read by this shader */
   uint64_t generic_inputs;

   /** Table mapping original TGSI generic indexes to low integers */
   int8_t generic_remap_table[MAX_GENERIC_VARYING];
};


struct svga_vertex_shader
{
   struct svga_shader base;

   struct draw_vertex_shader *draw_shader;

   /** Mask of which generic varying variables are written by this shader */
   uint64_t generic_outputs;

   /** Generated geometry shader that goes with this vertex shader */
   struct svga_geometry_shader *gs;
};


struct svga_geometry_shader
{
   struct svga_shader base;

   struct draw_geometry_shader *draw_shader;

   /** Table mapping original TGSI generic indexes to low integers */
   int8_t generic_remap_table[MAX_GENERIC_VARYING];
   uint64_t generic_outputs;

   unsigned aa_point_coord_index; /* generic index for aa point coord */

   unsigned wide_point:1;      /* set if the shader emulates wide point */
};


struct svga_tcs_shader
{
   struct svga_shader base;

   /** Mask of which generic varying variables are written by this shader */
   uint64_t generic_outputs;
};


struct svga_tes_shader
{
   struct svga_shader base;

   /** Mask of which generic varying variables are written by this shader */
   uint64_t generic_inputs;
};


struct svga_compute_shader
{
   struct svga_shader base;
   unsigned shared_mem_size;
};


static inline bool
svga_compile_keys_equal(const struct svga_compile_key *a,
                        const struct svga_compile_key *b)
{
   unsigned key_size = sizeof(*a);

   return memcmp(a, b, key_size) == 0;
}


uint64_t
svga_get_generic_inputs_mask(const struct tgsi_shader_info *info);

uint64_t
svga_get_generic_outputs_mask(const struct tgsi_shader_info *info);

void
svga_remap_generics(uint64_t generics_mask,
                    int8_t remap_table[MAX_GENERIC_VARYING]);

int
svga_remap_generic_index(int8_t remap_table[MAX_GENERIC_VARYING],
                         int generic_index);

void
svga_init_shader_key_common(const struct svga_context *svga,
                            enum pipe_shader_type shader_type,
                            const struct svga_shader *shader,
                            struct svga_compile_key *key);

struct svga_shader_variant *
svga_search_shader_key(const struct svga_shader *shader,
                       const struct svga_compile_key *key);

struct svga_shader *
svga_search_shader_token_key(struct svga_shader *shader,
                             const struct svga_token_key *key);

struct svga_shader *
svga_create_shader(struct pipe_context *pipe,
                   const struct pipe_shader_state *templ,
                   enum pipe_shader_type stage,
                   unsigned len);

enum pipe_error
svga_compile_shader(struct svga_context *svga,
                    struct svga_shader *shader,
                    const struct svga_compile_key *key,
                    struct svga_shader_variant **out_variant);

enum pipe_error
svga_define_shader(struct svga_context *svga,
                   struct svga_shader_variant *variant);

enum pipe_error
svga_set_shader(struct svga_context *svga,
                SVGA3dShaderType type,
                struct svga_shader_variant *variant);

struct svga_shader_variant *
svga_new_shader_variant(struct svga_context *svga, enum pipe_shader_type type);

void
svga_destroy_shader_variant(struct svga_context *svga,
                            struct svga_shader_variant *variant);

enum pipe_error
svga_rebind_shaders(struct svga_context *svga);

/**
 * Check if a shader's bytecode exceeds the device limits.
 */
static inline bool
svga_shader_too_large(const struct svga_context *svga,
                      const struct svga_shader_variant *variant)
{
   if (svga_have_gb_objects(svga)) {
      return false;
   }

   if (variant->nr_tokens * sizeof(variant->tokens[0])
       + sizeof(SVGA3dCmdDefineShader) + sizeof(SVGA3dCmdHeader)
       < SVGA_CB_MAX_COMMAND_SIZE) {
      return false;
   }

   return true;
}


/**
 * Convert from PIPE_SHADER_* to SVGA3D_SHADERTYPE_*
 */
static inline SVGA3dShaderType
svga_shader_type(enum pipe_shader_type shader)
{
   switch (shader) {
   case PIPE_SHADER_VERTEX:
      return SVGA3D_SHADERTYPE_VS;
   case PIPE_SHADER_GEOMETRY:
      return SVGA3D_SHADERTYPE_GS;
   case PIPE_SHADER_FRAGMENT:
      return SVGA3D_SHADERTYPE_PS;
   case PIPE_SHADER_TESS_CTRL:
      return SVGA3D_SHADERTYPE_HS;
   case PIPE_SHADER_TESS_EVAL:
      return SVGA3D_SHADERTYPE_DS;
   case PIPE_SHADER_COMPUTE:
      return SVGA3D_SHADERTYPE_CS;
   default:
      assert(!"Invalid shader type");
      return SVGA3D_SHADERTYPE_VS;
   }
}


/** Does the current VS have stream output? */
static inline bool
svga_have_vs_streamout(const struct svga_context *svga)
{
   return svga->curr.vs != NULL && svga->curr.vs->base.stream_output != NULL;
}


/** Does the current GS have stream output? */
static inline bool
svga_have_gs_streamout(const struct svga_context *svga)
{
   return svga->curr.gs != NULL && svga->curr.gs->base.stream_output != NULL;
}


static inline struct svga_fs_variant *
svga_fs_variant(struct svga_shader_variant *variant)
{
   assert(!variant || variant->type == SVGA3D_SHADERTYPE_PS);
   return (struct svga_fs_variant *)variant;
}


static inline struct svga_tes_variant *
svga_tes_variant(struct svga_shader_variant *variant)
{
   assert(!variant || variant->type == SVGA3D_SHADERTYPE_DS);
   return (struct svga_tes_variant *)variant;
}


static inline struct svga_cs_variant *
svga_cs_variant(struct svga_shader_variant *variant)
{
   assert(!variant || variant->type == SVGA3D_SHADERTYPE_CS);
   return (struct svga_cs_variant *)variant;
}


/* Returns TRUE if we are currently using flat shading.
 */
static inline bool
svga_is_using_flat_shading(const struct svga_context *svga)
{
   return
      svga->state.hw_draw.fs ?
         svga_fs_variant(svga->state.hw_draw.fs)->uses_flat_interp : false;
}

struct svga_shader_variant *
svga_get_compiled_dummy_vertex_shader(struct svga_context *svga,
                                      struct svga_shader *shader,
                                      const struct svga_compile_key *key);


struct svga_shader_variant *
svga_get_compiled_dummy_fragment_shader(struct svga_context *svga,
                                        struct svga_shader *shader,
                                        const struct svga_compile_key *key);

struct svga_shader_variant *
svga_get_compiled_dummy_geometry_shader(struct svga_context *svga,
                                        struct svga_shader *shader,
                                        const struct svga_compile_key *key);

static inline bool
svga_shader_use_samplers(struct svga_shader *shader)
{
   return shader ? (shader->info.uses_samplers != 0) : false;
}

#endif /* SVGA_SHADER_H */
