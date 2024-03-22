/*
 * Copyright 2012 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/* The compiler middle-end architecture: Explaining (non-)monolithic shaders
 * -------------------------------------------------------------------------
 *
 * Typically, there is one-to-one correspondence between API and HW shaders,
 * that is, for every API shader, there is exactly one shader binary in
 * the driver.
 *
 * The problem with that is that we also have to emulate some API states
 * (e.g. alpha-test, and many others) in shaders too. The two obvious ways
 * to deal with it are:
 * - each shader has multiple variants for each combination of emulated states,
 *   and the variants are compiled on demand, possibly relying on a shader
 *   cache for good performance
 * - patch shaders at the binary level
 *
 * This driver uses something completely different. The emulated states are
 * usually implemented at the beginning or end of shaders. Therefore, we can
 * split the shader into 3 parts:
 * - prolog part (shader code dependent on states)
 * - main part (the API shader)
 * - epilog part (shader code dependent on states)
 *
 * Each part is compiled as a separate shader and the final binaries are
 * concatenated. This type of shader is called non-monolithic, because it
 * consists of multiple independent binaries. Creating a new shader variant
 * is therefore only a concatenation of shader parts (binaries) and doesn't
 * involve any compilation. The main shader parts are the only parts that are
 * compiled when applications create shader objects. The prolog and epilog
 * parts are compiled on the first use and saved, so that their binaries can
 * be reused by many other shaders.
 *
 * One of the roles of the prolog part is to compute vertex buffer addresses
 * for vertex shaders. A few of the roles of the epilog part are color buffer
 * format conversions in pixel shaders that we have to do manually, and write
 * tessellation factors in tessellation control shaders. The prolog and epilog
 * have many other important responsibilities in various shader stages.
 * They don't just "emulate legacy stuff".
 *
 * Monolithic shaders are shaders where the parts are combined before LLVM
 * compilation, and the whole thing is compiled and optimized as one unit with
 * one binary on the output. The result is the same as the non-monolithic
 * shader, but the final code can be better, because LLVM can optimize across
 * all shader parts. Monolithic shaders aren't usually used except for these
 * special cases:
 *
 * 1) Some rarely-used states require modification of the main shader part
 *    itself, and in such cases, only the monolithic shader variant is
 *    compiled, and that's always done on the first use.
 *
 * 2) When we do cross-stage optimizations for separate shader objects and
 *    e.g. eliminate unused shader varyings, the resulting optimized shader
 *    variants are always compiled as monolithic shaders, and always
 *    asynchronously (i.e. not stalling ongoing rendering). We call them
 *    "optimized monolithic" shaders. The important property here is that
 *    the non-monolithic unoptimized shader variant is always available for use
 *    when the asynchronous compilation of the optimized shader is not done
 *    yet.
 *
 * Starting with GFX9 chips, some shader stages are merged, and the number of
 * shader parts per shader increased. The complete new list of shader parts is:
 * - 1st shader: prolog part
 * - 1st shader: main part
 * - 2nd shader: prolog part
 * - 2nd shader: main part
 * - 2nd shader: epilog part
 */

/* How linking shader inputs and outputs between vertex, tessellation, and
 * geometry shaders works.
 *
 * Inputs and outputs between shaders are stored in a buffer. This buffer
 * lives in LDS (typical case for tessellation), but it can also live
 * in memory (ESGS). Each input or output has a fixed location within a vertex.
 * The highest used input or output determines the stride between vertices.
 *
 * Since GS and tessellation are only possible in the OpenGL core profile,
 * only these semantics are valid for per-vertex data:
 *
 *   Name             Location
 *
 *   POSITION         0
 *   VAR0..31         1..32
 *   CLIP_DIST0..1    49..50
 *   PSIZ             51
 *
 * For example, a shader only writing GENERIC0 has the output stride of 5.
 *
 * Only these semantics are valid for per-patch data:
 *
 *   Name             Location
 *
 *   TESSOUTER        0
 *   TESSINNER        1
 *   PATCH0..29       2..31
 *
 * That's how independent shaders agree on input and output locations.
 * The si_shader_io_get_unique_index function assigns the locations.
 *
 * For tessellation, other required information for calculating the input and
 * output addresses like the vertex stride, the patch stride, and the offsets
 * where per-vertex and per-patch data start, is passed to the shader via
 * user data SGPRs. The offsets and strides are calculated at draw time and
 * aren't available at compile time.
 */

#ifndef SI_SHADER_H
#define SI_SHADER_H

#include "shader_info.h"
#include "ac_binary.h"
#include "ac_gpu_info.h"
#include "util/u_live_shader_cache.h"
#include "util/u_queue.h"
#include "si_pm4.h"

#ifdef __cplusplus
extern "C" {
#endif

struct nir_shader;
struct nir_instr;
struct nir_lower_subgroups_options;

#define SI_NUM_INTERP     32
#define SI_MAX_ATTRIBS    16
#define SI_MAX_VS_OUTPUTS 40
#define SI_USER_CLIP_PLANE_MASK  0x3F

#define SI_PS_INPUT_CNTL_0000          (S_028644_OFFSET(0x20) | S_028644_DEFAULT_VAL(0))
#define SI_PS_INPUT_CNTL_0001          (S_028644_OFFSET(0x20) | S_028644_DEFAULT_VAL(3))
#define SI_PS_INPUT_CNTL_UNUSED        SI_PS_INPUT_CNTL_0000
/* D3D9 behaviour for COLOR0 requires 0001. GL is undefined. */
#define SI_PS_INPUT_CNTL_UNUSED_COLOR0 SI_PS_INPUT_CNTL_0001

#define SI_VECTOR_ARG_IS_COLOR               BITFIELD_BIT(0)
#define SI_VECTOR_ARG_COLOR_COMPONENT(x)     (((x) & 0x7) << 1)
#define SI_GET_VECTOR_ARG_COLOR_COMPONENT(x) (((x) >> 1) & 0x7)

/* SGPR user data indices */
enum
{
   SI_SGPR_INTERNAL_BINDINGS,
   SI_SGPR_BINDLESS_SAMPLERS_AND_IMAGES,
   SI_SGPR_CONST_AND_SHADER_BUFFERS, /* or just a constant buffer 0 pointer */
   SI_SGPR_SAMPLERS_AND_IMAGES,
   SI_NUM_RESOURCE_SGPRS,

   /* API VS, TES without GS, GS copy shader */
   SI_SGPR_VS_STATE_BITS = SI_NUM_RESOURCE_SGPRS,
   SI_NUM_VS_STATE_RESOURCE_SGPRS,

   /* all VS variants */
   SI_SGPR_BASE_VERTEX = SI_NUM_VS_STATE_RESOURCE_SGPRS,
   SI_SGPR_DRAWID,
   SI_SGPR_START_INSTANCE,
   SI_VS_NUM_USER_SGPR,

   SI_SGPR_VS_BLIT_DATA = SI_SGPR_CONST_AND_SHADER_BUFFERS,

   /* TES */
   SI_SGPR_TES_OFFCHIP_LAYOUT = SI_NUM_VS_STATE_RESOURCE_SGPRS,
   SI_SGPR_TES_OFFCHIP_ADDR,
   SI_TES_NUM_USER_SGPR,

   /* GFX6-8: TCS only */
   GFX6_SGPR_TCS_OFFCHIP_LAYOUT = SI_NUM_RESOURCE_SGPRS,
   GFX6_SGPR_TCS_OFFCHIP_ADDR,
   GFX6_SGPR_TCS_IN_LAYOUT,
   GFX6_TCS_NUM_USER_SGPR,

   /* GFX9: Merged LS-HS (VS-TCS) only. */
   GFX9_SGPR_TCS_OFFCHIP_LAYOUT = SI_VS_NUM_USER_SGPR,
   GFX9_SGPR_TCS_OFFCHIP_ADDR,
   GFX9_TCS_NUM_USER_SGPR,

   /* GS limits */
   GFX6_GS_NUM_USER_SGPR = SI_NUM_RESOURCE_SGPRS,
   SI_GSCOPY_NUM_USER_SGPR = SI_NUM_VS_STATE_RESOURCE_SGPRS,

   GFX9_SGPR_SMALL_PRIM_CULL_INFO = MAX2(SI_VS_NUM_USER_SGPR, SI_TES_NUM_USER_SGPR),
   GFX9_SGPR_ATTRIBUTE_RING_ADDR,
   GFX9_GS_NUM_USER_SGPR,

   /* PS only */
   SI_SGPR_ALPHA_REF = SI_NUM_RESOURCE_SGPRS,
   SI_PS_NUM_USER_SGPR,

   /* The value has to be 12, because the hw requires that descriptors
    * are aligned to 4 SGPRs.
    */
   SI_SGPR_VS_VB_DESCRIPTOR_FIRST = 12,
};

/* LLVM function parameter indices */
enum
{
   SI_NUM_RESOURCE_PARAMS = 4,

   /* PS only parameters */
   SI_PARAM_ALPHA_REF = SI_NUM_RESOURCE_PARAMS,
   SI_PARAM_PRIM_MASK,
   SI_PARAM_PERSP_SAMPLE,
   SI_PARAM_PERSP_CENTER,
   SI_PARAM_PERSP_CENTROID,
   SI_PARAM_PERSP_PULL_MODEL,
   SI_PARAM_LINEAR_SAMPLE,
   SI_PARAM_LINEAR_CENTER,
   SI_PARAM_LINEAR_CENTROID,
   SI_PARAM_LINE_STIPPLE_TEX,
   SI_PARAM_POS_X_FLOAT,
   SI_PARAM_POS_Y_FLOAT,
   SI_PARAM_POS_Z_FLOAT,
   SI_PARAM_POS_W_FLOAT,
   SI_PARAM_FRONT_FACE,
   SI_PARAM_ANCILLARY,
   SI_PARAM_SAMPLE_COVERAGE,
   SI_PARAM_POS_FIXED_PT,

   SI_NUM_PARAMS = SI_PARAM_POS_FIXED_PT + 9, /* +8 for COLOR[0..1] */
};

/* These fields are only set in current_vs_state (except INDEXED) in si_context, and they are
 * accessible in the shader via vs_state_bits in VS, TES, and GS.
 */
#define VS_STATE_CLAMP_VERTEX_COLOR__SHIFT   0
#define VS_STATE_CLAMP_VERTEX_COLOR__MASK    0x1 /* Shared by VS and GS */
#define VS_STATE_INDEXED__SHIFT              1
#define VS_STATE_INDEXED__MASK               0x1 /* Shared by VS and GS */

/* These fields are only set in current_vs_state in si_context, and they are accessible
 * in the shader via vs_state_bits in LS/HS.
 */
/* bit gap */
/* TCS output patch0 offset for per-patch outputs / 4
 * - 64 outputs are implied by SI_UNIQUE_SLOT_* values.
 * - max = 32(CPs) * 64(outputs) * 16(vec4) * 64(num_patches) * 2(inputs + outputs) / 4
 *       = 1M, clamped to 32K(LDS limit) / 4 = 8K
 * - only used by si_llvm_tcs_build_end, it can be removed after NIR lowering replaces it
 */
#define VS_STATE_TCS_OUT_PATCH0_OFFSET__SHIFT   10
#define VS_STATE_TCS_OUT_PATCH0_OFFSET__MASK    0x3fff
#define VS_STATE_LS_OUT_VERTEX_SIZE__SHIFT      24
#define VS_STATE_LS_OUT_VERTEX_SIZE__MASK       0xff /* max 32 * 4 + 1 (to reduce LDS bank conflicts) */

/* These fields are only set in current_gs_state in si_context, and they are accessible
 * in the shader via vs_state_bits in legacy GS, the GS copy shader, and any NGG shader.
 */
/* bit gap */
#define GS_STATE_ESGS_VERTEX_STRIDE__SHIFT      10
#define GS_STATE_ESGS_VERTEX_STRIDE__MASK       0xff /* max 32 * 4 + 1 */
/* Small prim filter precision = num_samples / quant_mode, which can only be equal to 1/2^n
 * where n is between 4 and 12. Knowing that, we only need to store 4 bits of the FP32 exponent.
 * Set it like this: value = (fui(num_samples / quant_mode) >> 23) & 0xf;
 * Expand to FP32 like this: ((0x70 | value) << 23);
 * With 0x70 = 112, we get 2^(112 + value - 127) = 2^(value - 15), which is always a negative
 * exponent and it's equal to 1/2^(15 - value).
 */
#define GS_STATE_SMALL_PRIM_PRECISION_NO_AA__SHIFT 18
#define GS_STATE_SMALL_PRIM_PRECISION_NO_AA__MASK  0xf
#define GS_STATE_SMALL_PRIM_PRECISION__SHIFT    22
#define GS_STATE_SMALL_PRIM_PRECISION__MASK     0xf
#define GS_STATE_STREAMOUT_QUERY_ENABLED__SHIFT 26
#define GS_STATE_STREAMOUT_QUERY_ENABLED__MASK  0x1
#define GS_STATE_PROVOKING_VTX_INDEX__SHIFT     27
#define GS_STATE_PROVOKING_VTX_INDEX__MASK      0x3
#define GS_STATE_OUTPRIM__SHIFT                 29
#define GS_STATE_OUTPRIM__MASK                  0x3
#define GS_STATE_PIPELINE_STATS_EMU__SHIFT      31
#define GS_STATE_PIPELINE_STATS_EMU__MASK       0x1

#define ENCODE_FIELD(field, value) (((unsigned)(value) & field##__MASK) << field##__SHIFT)
#define CLEAR_FIELD(field) (~((unsigned)field##__MASK << field##__SHIFT))

/* This is called by functions that change states. */
#define SET_FIELD(var, field, value) do { \
   assert((value) == ((unsigned)(value) & field##__MASK)); \
   (var) &= CLEAR_FIELD(field); \
   (var) |= ENCODE_FIELD(field, value); \
} while (0)

/* This is called during shader compilation and returns LLVMValueRef. */
#define GET_FIELD(ctx, field) si_unpack_param((ctx), (ctx)->args->vs_state_bits, field##__SHIFT, \
                                             util_bitcount(field##__MASK))

enum
{
   /* These represent the number of SGPRs the shader uses. */
   SI_VS_BLIT_SGPRS_POS = 3,
   SI_VS_BLIT_SGPRS_POS_COLOR = 7,
   SI_VS_BLIT_SGPRS_POS_TEXCOORD = 9,

   MAX_SI_VS_BLIT_SGPRS = 10, /* +1 for the attribute ring address */
};

#define SI_NGG_CULL_TRIANGLES                (1 << 0)   /* this implies W, view.xy, and small prim culling */
#define SI_NGG_CULL_BACK_FACE                (1 << 1)   /* back faces */
#define SI_NGG_CULL_FRONT_FACE               (1 << 2)   /* front faces */
#define SI_NGG_CULL_LINES                    (1 << 3)   /* the primitive type is lines */
#define SI_NGG_CULL_SMALL_LINES_DIAMOND_EXIT (1 << 4)   /* cull small lines according to the diamond exit rule */
#define SI_NGG_CULL_CLIP_PLANE_ENABLE(enable) (((enable) & 0xff) << 5)
#define SI_NGG_CULL_GET_CLIP_PLANE_ENABLE(x)  (((x) >> 5) & 0xff)

#define SI_PROFILE_WAVE32                    (1 << 0)
#define SI_PROFILE_GFX10_WAVE64              (1 << 1)
/* bit gap */
#define SI_PROFILE_VS_NO_BINNING             (1 << 3)
#define SI_PROFILE_GFX9_GFX10_PS_NO_BINNING  (1 << 4)
#define SI_PROFILE_CLAMP_DIV_BY_ZERO         (1 << 5)

enum si_shader_dump_type {
   SI_DUMP_SHADER_KEY,
   SI_DUMP_INIT_NIR,       /* initial input NIR when shaders are created (before lowering) */
   SI_DUMP_NIR,            /* final NIR after lowering when shader variants are created */
   SI_DUMP_INIT_LLVM_IR,   /* initial LLVM IR before optimizations */
   SI_DUMP_LLVM_IR,        /* final LLVM IR */
   SI_DUMP_INIT_ACO_IR,    /* initial ACO IR before optimizations */
   SI_DUMP_ACO_IR,         /* final ACO IR */
   SI_DUMP_ASM,            /* final asm shaders */
   SI_DUMP_STATS,          /* print statistics as shader-db */
   SI_DUMP_ALWAYS,
};

enum {
   SI_UNIQUE_SLOT_POS = 0,

   /* Since some shader stages use the highest used IO index
    * to determine the size to allocate for inputs/outputs
    * (in LDS, tess and GS rings). VARn should be placed right
    * after POSITION to make that size as small as possible.
    */
   SI_UNIQUE_SLOT_VAR0 = 1, /* 0..31 */

   /* Put 16-bit GLES varyings after 32-bit varyings. They can use the same indices as
    * legacy desktop GL varyings because they are mutually exclusive.
    */
   SI_UNIQUE_SLOT_VAR0_16BIT = 33, /* 0..15 */

   /* Legacy GL-only varyings can alias GLES-only 16-bit varyings. */
   SI_UNIQUE_SLOT_FOGC = 33,
   SI_UNIQUE_SLOT_COL0,
   SI_UNIQUE_SLOT_COL1,
   SI_UNIQUE_SLOT_BFC0,
   SI_UNIQUE_SLOT_BFC1,
   SI_UNIQUE_SLOT_TEX0,
   SI_UNIQUE_SLOT_TEX1,
   SI_UNIQUE_SLOT_TEX2,
   SI_UNIQUE_SLOT_TEX3,
   SI_UNIQUE_SLOT_TEX4,
   SI_UNIQUE_SLOT_TEX5,
   SI_UNIQUE_SLOT_TEX6,
   SI_UNIQUE_SLOT_TEX7,
   SI_UNIQUE_SLOT_CLIP_VERTEX,

   /* Varyings present in both GLES and desktop GL must start at 49 after 16-bit varyings. */
   SI_UNIQUE_SLOT_CLIP_DIST0 = 49,
   SI_UNIQUE_SLOT_CLIP_DIST1,
   SI_UNIQUE_SLOT_PSIZ,
   /* These can't be written by LS, HS, and ES. */
   SI_UNIQUE_SLOT_LAYER,
   SI_UNIQUE_SLOT_VIEWPORT,
   SI_UNIQUE_SLOT_PRIMITIVE_ID,
};

/**
 * For VS shader keys, describe any fixups required for vertex fetch.
 *
 * \ref log_size, \ref format, and the number of channels are interpreted as
 * by \ref ac_build_opencoded_load_format.
 *
 * Note: all bits 0 (size = 1 byte, num channels = 1, format = float) is an
 * impossible format and indicates that no fixup is needed (just use
 * buffer_load_format_xyzw).
 */
union si_vs_fix_fetch {
   struct {
      uint8_t log_size : 2;        /* 1, 2, 4, 8 or bytes per channel */
      uint8_t num_channels_m1 : 2; /* number of channels minus 1 */
      uint8_t format : 3;          /* AC_FETCH_FORMAT_xxx */
      uint8_t reverse : 1;         /* reverse XYZ channels */
   } u;
   uint8_t bits;
};

struct si_shader;

/* State of the context creating the shader object. */
struct si_compiler_ctx_state {
   /* Should only be used by si_init_shader_selector_async and
    * si_build_shader_variant if thread_index == -1 (non-threaded). */
   struct ac_llvm_compiler *compiler;

   /* Used if thread_index == -1 or if debug.async is true. */
   struct util_debug_callback debug;

   /* Used for creating the log string for gallium/ddebug. */
   bool is_debug_context;
};

enum si_color_output_type {
   SI_TYPE_ANY32,
   SI_TYPE_FLOAT16,
   SI_TYPE_INT16,
   SI_TYPE_UINT16,
};

union si_input_info {
   struct {
      uint8_t semantic;
      uint8_t interpolate;
      uint8_t fp16_lo_hi_valid;
      uint8_t usage_mask;
   };
   uint32_t _unused; /* this just forces 4-byte alignment */
};

struct si_shader_info {
   shader_info base;

   uint32_t options; /* bitmask of SI_PROFILE_* */

   uint8_t num_inputs;
   uint8_t num_outputs;
   union si_input_info input[PIPE_MAX_SHADER_INPUTS];
   uint8_t output_semantic[PIPE_MAX_SHADER_OUTPUTS];
   uint8_t output_usagemask[PIPE_MAX_SHADER_OUTPUTS];
   uint8_t output_readmask[PIPE_MAX_SHADER_OUTPUTS];
   uint8_t output_streams[PIPE_MAX_SHADER_OUTPUTS];
   uint8_t output_type[PIPE_MAX_SHADER_OUTPUTS]; /* enum nir_alu_type */

   uint8_t num_vs_inputs;
   uint8_t num_vbos_in_user_sgprs;
   uint8_t num_stream_output_components[4];
   uint16_t enabled_streamout_buffer_mask;

   uint64_t inputs_read; /* "get_unique_index" bits */
   uint64_t tcs_vgpr_only_inputs; /* TCS inputs that are only in VGPRs, not LDS. */

   uint64_t outputs_written_before_tes_gs; /* "get_unique_index" bits */
   uint64_t outputs_written_before_ps; /* "get_unique_index" bits */
   uint32_t patch_outputs_written;     /* "get_unique_index_patch" bits */

   uint8_t clipdist_mask;
   uint8_t culldist_mask;

   uint16_t lshs_vertex_stride;
   uint16_t esgs_vertex_stride;
   uint16_t gsvs_vertex_size;
   uint8_t gs_input_verts_per_prim;
   unsigned max_gsvs_emit_size;

   /* Set 0xf or 0x0 (4 bits) per each written output.
    * ANDed with spi_shader_col_format.
    */
   unsigned colors_written_4bit;

   int constbuf0_num_slots;
   uint num_memory_stores;
   uint8_t color_attr_index[2];
   uint8_t color_interpolate[2];
   uint8_t color_interpolate_loc[2];
   uint8_t colors_read; /**< which color components are read by the FS */
   uint8_t colors_written;
   uint16_t output_color_types; /**< Each bit pair is enum si_color_output_type */
   bool vs_needs_prolog;
   bool color0_writes_all_cbufs; /**< gl_FragColor */
   bool reads_samplemask;   /**< does fragment shader read sample mask? */
   bool reads_tess_factors; /**< If TES reads TESSINNER or TESSOUTER */
   bool writes_z;           /**< does fragment shader write Z value? */
   bool writes_stencil;     /**< does fragment shader write stencil value? */
   bool writes_samplemask;  /**< does fragment shader write sample mask? */
   bool writes_edgeflag;    /**< vertex shader outputs edgeflag */
   bool uses_interp_color;
   bool uses_persp_center_color;
   bool uses_persp_centroid_color;
   bool uses_persp_sample_color;
   bool uses_persp_center;
   bool uses_persp_centroid;
   bool uses_persp_sample;
   bool uses_linear_center;
   bool uses_linear_centroid;
   bool uses_linear_sample;
   bool uses_interp_at_sample;
   bool uses_instanceid;
   bool uses_base_vertex;
   bool uses_base_instance;
   bool uses_drawid;
   bool uses_primid;
   bool uses_frontface;
   bool uses_invocationid;
   bool uses_thread_id[3];
   bool uses_block_id[3];
   bool uses_variable_block_size;
   bool uses_grid_size;
   bool uses_tg_size;
   bool writes_position;
   bool writes_psize;
   bool writes_clipvertex;
   bool writes_primid;
   bool writes_viewport_index;
   bool writes_layer;
   bool uses_bindless_samplers;
   bool uses_bindless_images;
   bool uses_indirect_descriptor;
   bool has_divergent_loop;
   bool uses_sampleid;
   bool uses_layer_id;
   bool has_non_uniform_tex_access;

   bool uses_vmem_sampler_or_bvh;
   bool uses_vmem_load_other; /* all other VMEM loads and atomics with return */

   /** Whether all codepaths write tess factors in all invocations. */
   bool tessfactors_are_def_in_all_invocs;

   /* A flag to check if vrs2x2 can be enabled to reduce number of
    * fragment shader invocations if flat shading.
    */
   bool allow_flat_shading;

   /* Optimization: if the texture bound to this texunit has been cleared to 1,
    * then the draw can be skipped (see si_draw_vbo_skip_noop). Initially the
    * value is 0xff (undetermined) and can be later changed to 0 (= false) or
    * texunit + 1.
    */
   uint8_t writes_1_if_tex_is_1;

   /* frag coord and sample pos per component read mask. */
   uint8_t reads_frag_coord_mask;
   uint8_t reads_sample_pos_mask;
};

/* A shader selector is a gallium CSO and contains shader variants and
 * binaries for one NIR program. This can be shared by multiple contexts.
 */
struct si_shader_selector {
   struct util_live_shader base;
   struct si_screen *screen;
   struct util_queue_fence ready;
   struct si_compiler_ctx_state compiler_ctx_state;
   gl_shader_stage stage;

   simple_mtx_t mutex;
   union si_shader_key *keys;
   unsigned variants_count;
   unsigned variants_max_count;
   struct si_shader **variants;

   /* The compiled NIR shader without a prolog and/or epilog (not
    * uploaded to a buffer object).
    */
   struct si_shader *main_shader_part;
   struct si_shader *main_shader_part_ls;     /* as_ls is set in the key */
   struct si_shader *main_shader_part_es;     /* as_es is set in the key */
   struct si_shader *main_shader_part_ngg;    /* as_ngg is set in the key */
   struct si_shader *main_shader_part_ngg_es; /* for Wave32 TES before legacy GS */

   struct nir_shader *nir;
   void *nir_binary;
   unsigned nir_size;

   struct si_shader_info info;

   enum pipe_shader_type pipe_shader_type;
   uint8_t const_and_shader_buf_descriptors_index;
   uint8_t sampler_and_images_descriptors_index;
   uint8_t cs_shaderbufs_sgpr_index;
   uint8_t cs_num_shaderbufs_in_user_sgprs;
   uint8_t cs_images_sgpr_index;
   uint8_t cs_images_num_sgprs;
   uint8_t cs_num_images_in_user_sgprs;
   unsigned ngg_cull_vert_threshold; /* UINT32_MAX = disabled */
   enum mesa_prim rast_prim;

   /* GS parameters. */
   bool tess_turns_off_ngg;

   /* bitmasks of used descriptor slots */
   uint64_t active_const_and_shader_buffers;
   uint64_t active_samplers_and_images;
};

/* Valid shader configurations:
 *
 * API shaders           VS | TCS | TES | GS |pass| PS
 * are compiled as:         |     |     |    |thru|
 *                          |     |     |    |    |
 * Only VS & PS:         VS |     |     |    |    | PS
 * GFX6     - with GS:   ES |     |     | GS | VS | PS
 *          - with tess: LS | HS  | VS  |    |    | PS
 *          - with both: LS | HS  | ES  | GS | VS | PS
 * GFX9     - with GS:   -> |     |     | GS | VS | PS
 *          - with tess: -> | HS  | VS  |    |    | PS
 *          - with both: -> | HS  | ->  | GS | VS | PS
 *                          |     |     |    |    |
 * NGG      - VS & PS:   GS |     |     |    |    | PS
 * (GFX10+) - with GS:   -> |     |     | GS |    | PS
 *          - with tess: -> | HS  | GS  |    |    | PS
 *          - with both: -> | HS  | ->  | GS |    | PS
 *
 * -> = merged with the next stage
 */

/* Use the byte alignment for all following structure members for optimal
 * shader key memory footprint.
 */
#pragma pack(push, 1)

/* Common VS bits between the shader key and the prolog key. */
struct si_vs_prolog_bits {
   /* - If neither "is_one" nor "is_fetched" has a bit set, the instance
    *   divisor is 0.
    * - If "is_one" has a bit set, the instance divisor is 1.
    * - If "is_fetched" has a bit set, the instance divisor will be loaded
    *   from the constant buffer.
    */
   uint16_t instance_divisor_is_one;     /* bitmask of inputs */
   uint16_t instance_divisor_is_fetched; /* bitmask of inputs */
   unsigned ls_vgpr_fix : 1;
};

/* Common TCS bits between the shader key and the epilog key. */
struct si_tcs_epilog_bits {
   unsigned prim_mode : 3;
   unsigned invoc0_tess_factors_are_def : 1;
   unsigned tes_reads_tess_factors : 1;
};

/* Common PS bits between the shader key and the prolog key. */
struct si_ps_prolog_bits {
   unsigned color_two_side : 1;
   unsigned flatshade_colors : 1;
   unsigned poly_stipple : 1;
   unsigned force_persp_sample_interp : 1;
   unsigned force_linear_sample_interp : 1;
   unsigned force_persp_center_interp : 1;
   unsigned force_linear_center_interp : 1;
   unsigned bc_optimize_for_persp : 1;
   unsigned bc_optimize_for_linear : 1;
   unsigned samplemask_log_ps_iter : 3;
};

/* Common PS bits between the shader key and the epilog key. */
struct si_ps_epilog_bits {
   unsigned spi_shader_col_format;
   unsigned color_is_int8 : 8;
   unsigned color_is_int10 : 8;
   unsigned last_cbuf : 3;
   unsigned alpha_func : 3;
   unsigned alpha_to_one : 1;
   unsigned alpha_to_coverage_via_mrtz : 1;  /* gfx11+ */
   unsigned clamp_color : 1;
   unsigned dual_src_blend_swizzle : 1;      /* gfx11+ */
   unsigned rbplus_depth_only_opt:1;
   unsigned kill_samplemask:1;
};

union si_shader_part_key {
   struct {
      struct si_vs_prolog_bits states;
      unsigned wave32 : 1;
      unsigned num_input_sgprs : 6;
      /* For merged stages such as LS-HS, HS input VGPRs are first. */
      unsigned num_merged_next_stage_vgprs : 3;
      unsigned num_inputs : 5;
      unsigned as_ls : 1;
      unsigned as_es : 1;
      unsigned as_ngg : 1;
   } vs_prolog;
   struct {
      struct si_tcs_epilog_bits states;
      unsigned wave32 : 1;
      unsigned noop_s_barrier : 1;
   } tcs_epilog;
   struct {
      struct si_ps_prolog_bits states;
      unsigned wave32 : 1;
      unsigned num_input_sgprs : 6;
      /* Color interpolation and two-side color selection. */
      unsigned colors_read : 8;       /* color input components read */
      unsigned num_interp_inputs : 5; /* BCOLOR is at this location */
      unsigned num_fragcoord_components : 3;
      unsigned wqm : 1;
      char color_attr_index[2];
      signed char color_interp_vgpr_index[2]; /* -1 == constant */
   } ps_prolog;
   struct {
      struct si_ps_epilog_bits states;
      unsigned wave32 : 1;
      unsigned uses_discard : 1;
      unsigned colors_written : 8;
      unsigned color_types : 16;
      unsigned writes_z : 1;
      unsigned writes_stencil : 1;
      unsigned writes_samplemask : 1;
   } ps_epilog;
};

/* The shader key for geometry stages (VS, TCS, TES, GS) */
struct si_shader_key_ge {
   /* Prolog and epilog flags. */
   union {
      struct {
         struct si_vs_prolog_bits prolog;
      } vs;
      struct {
         struct si_vs_prolog_bits ls_prolog; /* for merged LS-HS */
         struct si_shader_selector *ls;      /* for merged LS-HS */
         struct si_tcs_epilog_bits epilog;
      } tcs; /* tessellation control shader */
      struct {
         struct si_vs_prolog_bits vs_prolog; /* for merged ES-GS */
         struct si_shader_selector *es;      /* for merged ES-GS */
      } gs;
   } part;

   /* These three are initially set according to the NEXT_SHADER property,
    * or guessed if the property doesn't seem correct.
    */
   unsigned as_es : 1;  /* whether it's a shader before GS */
   unsigned as_ls : 1;  /* whether it's VS before TCS */
   unsigned as_ngg : 1; /* whether it's the last GE stage and NGG is enabled,
                           also set for the stage right before GS */

   /* Flags for monolithic compilation only. */
   struct {
      /* Whether fetch should be opencoded according to vs_fix_fetch.
       * Otherwise, if vs_fix_fetch is non-zero, buffer_load_format_xyzw
       * with minimal fixups is used. */
      uint16_t vs_fetch_opencode;
      union si_vs_fix_fetch vs_fix_fetch[SI_MAX_ATTRIBS];

      union {
         /* When PS needs PrimID and GS is disabled. */
         unsigned vs_export_prim_id : 1;    /* VS and TES only */
         unsigned gs_tri_strip_adj_fix : 1; /* GS only */
      } u;
   } mono;

   /* Optimization flags for asynchronous compilation only. */
   struct {
      /* For HW VS (it can be VS, TES, GS) */
      uint64_t kill_outputs; /* "get_unique_index" bits */
      unsigned kill_clip_distances : 8;
      unsigned kill_pointsize : 1;
      unsigned kill_layer : 1;
      unsigned remove_streamout : 1;

      /* For NGG VS and TES. */
      unsigned ngg_culling : 13; /* SI_NGG_CULL_* */

      /* For shaders where monolithic variants have better code.
       *
       * This is a flag that has no effect on code generation,
       * but forces monolithic shaders to be used as soon as
       * possible, because it's in the "opt" group.
       */
      unsigned prefer_mono : 1;

      /* VS and TCS have the same number of patch vertices. */
      unsigned same_patch_vertices:1;

      unsigned inline_uniforms:1;

      /* This must be kept last to limit the number of variants
       * depending only on the uniform values.
       */
      uint32_t inlined_uniform_values[MAX_INLINABLE_UNIFORMS];
   } opt;
};

struct si_shader_key_ps {
   struct {
      /* Prolog and epilog flags. */
      struct si_ps_prolog_bits prolog;
      struct si_ps_epilog_bits epilog;
   } part;

   /* Flags for monolithic compilation only. */
   struct {
      unsigned poly_line_smoothing : 1;
      unsigned point_smoothing : 1;
      unsigned interpolate_at_sample_force_center : 1;
      unsigned fbfetch_msaa : 1;
      unsigned fbfetch_is_1D : 1;
      unsigned fbfetch_layered : 1;
   } mono;

   /* Optimization flags for asynchronous compilation only. */
   struct {
      /* For shaders where monolithic variants have better code.
       *
       * This is a flag that has no effect on code generation,
       * but forces monolithic shaders to be used as soon as
       * possible, because it's in the "opt" group.
       */
      unsigned prefer_mono : 1;
      unsigned inline_uniforms:1;

      /* This eliminates the FRONT_FACE input VGPR as well as shader code using it. */
      int force_front_face_input : 2; /* 0 = gl_FrontFacing, 1 = true, -1 = false */

      /* This must be kept last to limit the number of variants
       * depending only on the uniform values.
       */
      uint32_t inlined_uniform_values[MAX_INLINABLE_UNIFORMS];
   } opt;
};

union si_shader_key {
   struct si_shader_key_ge ge; /* geometry engine shaders */
   struct si_shader_key_ps ps;
};

/* Restore the pack alignment to default. */
#pragma pack(pop)

/* GCN-specific shader info. */
struct si_shader_binary_info {
   uint8_t vs_output_param_offset[NUM_TOTAL_VARYING_SLOTS];
   uint32_t vs_output_ps_input_cntl[NUM_TOTAL_VARYING_SLOTS];
   union si_input_info ps_inputs[SI_NUM_INTERP];
   uint8_t num_ps_inputs;
   uint8_t ps_colors_read;
   uint8_t num_input_sgprs;
   uint8_t num_input_vgprs;
   bool uses_vmem_load_other; /* all other VMEM loads and atomics with return */
   bool uses_vmem_sampler_or_bvh;
   uint8_t num_fragcoord_components;
   bool uses_instanceid;
   uint8_t nr_pos_exports;
   uint8_t nr_param_exports;
   unsigned private_mem_vgprs;
   unsigned max_simd_waves;
};

enum si_shader_binary_type {
   SI_SHADER_BINARY_ELF,
   SI_SHADER_BINARY_RAW,
};

struct si_shader_binary {
   enum si_shader_binary_type type;

   /* Depends on binary type, either ELF or raw buffer. */
   const char *code_buffer;
   size_t code_size;
   uint32_t exec_size;

   char *uploaded_code;
   size_t uploaded_code_size;

   char *llvm_ir_string;

   const char *disasm_string;
   size_t disasm_size;

   const unsigned *symbols;
   unsigned num_symbols;
};

struct gfx9_gs_info {
   unsigned es_verts_per_subgroup;
   unsigned gs_prims_per_subgroup;
   unsigned gs_inst_prims_in_subgroup;
   unsigned max_prims_per_subgroup;
   unsigned esgs_ring_size; /* in bytes */
};

struct si_shader {
   struct si_pm4_state pm4; /* base class */
   struct si_compiler_ctx_state compiler_ctx_state;

   struct si_shader_selector *selector;
   struct si_shader_selector *previous_stage_sel; /* for refcounting */

   struct si_shader_part *prolog;
   struct si_shader *previous_stage; /* for GFX9 */
   struct si_shader_part *epilog;
   struct si_shader *gs_copy_shader;

   struct si_resource *bo;
   /* gpu_address should be bo->gpu_address except if SQTT is
    * in use.
    */
   uint64_t gpu_address;
   struct si_resource *scratch_bo;
   union si_shader_key key;
   struct util_queue_fence ready;
   bool compilation_failed;
   bool is_monolithic;
   bool is_optimized;
   bool is_binary_shared;
   bool is_gs_copy_shader;
   uint8_t wave_size;

   /* The following data is all that's needed for binary shaders. */
   struct si_shader_binary binary;
   struct ac_shader_config config;
   struct si_shader_binary_info info;

   /* SI_SGPR_VS_STATE_BITS */
   bool uses_vs_state_provoking_vertex;
   bool uses_gs_state_outprim;

   bool uses_base_instance;

   /* Shader key + LLVM IR + disassembly + statistics.
    * Generated for debug contexts only.
    */
   char *shader_log;
   size_t shader_log_size;

   struct gfx9_gs_info gs_info;

   /* Precomputed register values. */
   union {
      struct {
         unsigned vgt_gsvs_ring_offset_1;
         unsigned vgt_gsvs_ring_offset_2;
         unsigned vgt_gsvs_ring_offset_3;
         unsigned vgt_gsvs_ring_itemsize;
         unsigned vgt_gs_max_vert_out;
         unsigned vgt_gs_vert_itemsize;
         unsigned vgt_gs_vert_itemsize_1;
         unsigned vgt_gs_vert_itemsize_2;
         unsigned vgt_gs_vert_itemsize_3;
         unsigned vgt_gs_instance_cnt;
         unsigned vgt_gs_onchip_cntl;
         unsigned vgt_gs_max_prims_per_subgroup;
         unsigned vgt_esgs_ring_itemsize;
         unsigned spi_shader_pgm_rsrc3_gs;
         unsigned spi_shader_pgm_rsrc4_gs;
      } gs;

      struct {
         /* Computed by gfx10_ngg_calculate_subgroup_info. */
         uint16_t ngg_emit_size; /* in dwords */
         uint16_t hw_max_esverts;
         uint16_t max_gsprims;
         uint16_t max_out_verts;
         bool max_vert_out_per_gs_instance;
         /* Register values. */
         unsigned ge_max_output_per_subgroup;
         unsigned ge_ngg_subgrp_cntl;
         unsigned vgt_primitiveid_en;
         unsigned vgt_gs_onchip_cntl;
         unsigned vgt_gs_instance_cnt;
         unsigned esgs_vertex_stride;
         unsigned spi_vs_out_config;
         unsigned spi_shader_pos_format;
         unsigned pa_cl_vte_cntl;
         unsigned vgt_gs_max_vert_out; /* for API GS */
         unsigned ge_pc_alloc;         /* uconfig register */
         unsigned spi_shader_pgm_rsrc3_gs;
         unsigned spi_shader_pgm_rsrc4_gs;
         unsigned vgt_shader_stages_en;
      } ngg;

      struct {
         unsigned vgt_gs_mode;
         unsigned vgt_primitiveid_en;
         unsigned vgt_reuse_off;
         unsigned spi_vs_out_config;
         unsigned spi_shader_pos_format;
         unsigned pa_cl_vte_cntl;
         unsigned ge_pc_alloc; /* uconfig register */
      } vs;

      struct {
         unsigned spi_ps_input_ena;
         unsigned spi_ps_input_addr;
         unsigned spi_baryc_cntl;
         unsigned spi_ps_in_control;
         unsigned spi_shader_z_format;
         unsigned spi_shader_col_format;
         unsigned cb_shader_mask;
         unsigned db_shader_control;
         unsigned num_interp;
         bool writes_samplemask;
      } ps;
   };

   /* Precomputed register values. */
   unsigned vgt_tf_param;                /* VGT_TF_PARAM */
   unsigned vgt_vertex_reuse_block_cntl; /* VGT_VERTEX_REUSE_BLOCK_CNTL */
   unsigned pa_cl_vs_out_cntl;
   unsigned ge_cntl;
};

struct si_shader_part {
   struct si_shader_part *next;
   union si_shader_part_key key;
   struct si_shader_binary binary;
   struct ac_shader_config config;
};

/* si_shader.c */
struct ac_rtld_binary;

void si_update_shader_binary_info(struct si_shader *shader, struct nir_shader *nir);
bool si_compile_shader(struct si_screen *sscreen, struct ac_llvm_compiler *compiler,
                       struct si_shader *shader, struct util_debug_callback *debug);
bool si_create_shader_variant(struct si_screen *sscreen, struct ac_llvm_compiler *compiler,
                              struct si_shader *shader, struct util_debug_callback *debug);
void si_shader_destroy(struct si_shader *shader);
unsigned si_shader_io_get_unique_index(unsigned semantic);
bool si_shader_binary_upload(struct si_screen *sscreen, struct si_shader *shader,
                             uint64_t scratch_va);
bool si_can_dump_shader(struct si_screen *sscreen, gl_shader_stage stage,
                        enum si_shader_dump_type dump_type);
void si_shader_dump(struct si_screen *sscreen, struct si_shader *shader,
                    struct util_debug_callback *debug, FILE *f, bool check_debug_option);
void si_shader_dump_stats_for_shader_db(struct si_screen *screen, struct si_shader *shader,
                                        struct util_debug_callback *debug);
void si_multiwave_lds_size_workaround(struct si_screen *sscreen, unsigned *lds_size);
const char *si_get_shader_name(const struct si_shader *shader);
void si_shader_binary_clean(struct si_shader_binary *binary);
struct nir_shader *si_deserialize_shader(struct si_shader_selector *sel);
unsigned si_get_ps_num_interp(struct si_shader *ps);
bool si_shader_binary_open(struct si_screen *screen, struct si_shader *shader,
                           struct ac_rtld_binary *rtld);
bool si_get_external_symbol(enum amd_gfx_level gfx_level, void *data, const char *name,
                            uint64_t *value);

/* si_shader_info.c */
void si_nir_scan_shader(struct si_screen *sscreen,  const struct nir_shader *nir,
                        struct si_shader_info *info);

/* si_shader_nir.c */
extern const struct nir_lower_subgroups_options si_nir_subgroups_options;

bool si_alu_to_scalar_packed_math_filter(const struct nir_instr *instr, const void *data);
void si_nir_opts(struct si_screen *sscreen, struct nir_shader *nir, bool first);
void si_nir_late_opts(struct nir_shader *nir);
char *si_finalize_nir(struct pipe_screen *screen, void *nirptr);

/* si_state_shaders.cpp */
unsigned si_determine_wave_size(struct si_screen *sscreen, struct si_shader *shader);
void gfx9_get_gs_info(struct si_shader_selector *es, struct si_shader_selector *gs,
                      struct gfx9_gs_info *out);
bool gfx10_is_ngg_passthrough(struct si_shader *shader);

bool si_should_clear_lds(struct si_screen *sscreen, const struct nir_shader *shader);

/* Inline helpers. */

/* Return the pointer to the main shader part's pointer. */
static inline struct si_shader **si_get_main_shader_part(struct si_shader_selector *sel,
                                                         const union si_shader_key *key)
{
   if (sel->stage <= MESA_SHADER_GEOMETRY) {
      if (key->ge.as_ls)
         return &sel->main_shader_part_ls;
      if (key->ge.as_es && key->ge.as_ngg)
         return &sel->main_shader_part_ngg_es;
      if (key->ge.as_es)
         return &sel->main_shader_part_es;
      if (key->ge.as_ngg)
         return &sel->main_shader_part_ngg;
   }
   return &sel->main_shader_part;
}

static inline bool si_shader_uses_bindless_samplers(struct si_shader_selector *selector)
{
   return selector ? selector->info.uses_bindless_samplers : false;
}

static inline bool si_shader_uses_bindless_images(struct si_shader_selector *selector)
{
   return selector ? selector->info.uses_bindless_images : false;
}

static inline bool gfx10_edgeflags_have_effect(struct si_shader *shader)
{
   if (shader->selector->stage == MESA_SHADER_VERTEX &&
       !shader->selector->info.base.vs.blit_sgprs_amd &&
       !(shader->key.ge.opt.ngg_culling & SI_NGG_CULL_LINES))
      return true;

   return false;
}

static inline bool gfx10_ngg_writes_user_edgeflags(struct si_shader *shader)
{
   return gfx10_edgeflags_have_effect(shader) &&
          shader->selector->info.writes_edgeflag;
}

static inline bool si_shader_uses_streamout(const struct si_shader *shader)
{
   return shader->selector->stage <= MESA_SHADER_GEOMETRY &&
          shader->selector->info.enabled_streamout_buffer_mask &&
          !shader->key.ge.opt.remove_streamout;
}

static inline bool si_shader_uses_discard(struct si_shader *shader)
{
   /* Changes to this should also update ps_modifies_zs. */
   return shader->selector->info.base.fs.uses_discard ||
          shader->key.ps.part.prolog.poly_stipple ||
          shader->key.ps.mono.point_smoothing ||
          shader->key.ps.part.epilog.alpha_func != PIPE_FUNC_ALWAYS;
}

#ifdef __cplusplus
}
#endif

#endif
