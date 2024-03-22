/*
 * Copyright © 2010 - 2015 Intel Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef BRW_COMPILER_H
#define BRW_COMPILER_H

#include <stdio.h>
#include "c11/threads.h"
#include "dev/intel_device_info.h"
#include "util/macros.h"
#include "util/enum_operators.h"
#include "util/ralloc.h"
#include "util/u_math.h"
#include "brw_isa_info.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ra_regs;
struct nir_shader;
struct shader_info;

struct nir_shader_compiler_options;
typedef struct nir_shader nir_shader;

struct brw_compiler {
   const struct intel_device_info *devinfo;

   /* This lock must be taken if the compiler is to be modified in any way,
    * including adding something to the ralloc child list.
    */
   mtx_t mutex;

   struct brw_isa_info isa;

   struct {
      struct ra_regs *regs;

      /**
       * Array of the ra classes for the unaligned contiguous register
       * block sizes used.
       */
      struct ra_class **classes;
   } vec4_reg_set;

   struct {
      struct ra_regs *regs;

      /**
       * Array of the ra classes for the unaligned contiguous register
       * block sizes used, indexed by register size.
       */
      struct ra_class *classes[16];

      /**
       * ra class for the aligned barycentrics we use for PLN, which doesn't
       * appear in *classes.
       */
      struct ra_class *aligned_bary_class;
   } fs_reg_sets[3];

   void (*shader_debug_log)(void *, unsigned *id, const char *str, ...) PRINTFLIKE(3, 4);
   void (*shader_perf_log)(void *, unsigned *id, const char *str, ...) PRINTFLIKE(3, 4);

   bool scalar_stage[MESA_ALL_SHADER_STAGES];
   bool use_tcs_multi_patch;
   struct nir_shader_compiler_options *nir_options[MESA_ALL_SHADER_STAGES];

   /**
    * Apply workarounds for SIN and COS output range problems.
    * This can negatively impact performance.
    */
   bool precise_trig;

   /**
    * Is 3DSTATE_CONSTANT_*'s Constant Buffer 0 relative to Dynamic State
    * Base Address?  (If not, it's a normal GPU address.)
    */
   bool constant_buffer_0_is_relative;

   /**
    * Whether or not the driver supports NIR shader constants.  This controls
    * whether nir_opt_large_constants will be run.
    */
   bool supports_shader_constants;

   /**
    * Whether indirect UBO loads should use the sampler or go through the
    * data/constant cache.  For the sampler, UBO surface states have to be set
    * up with VK_FORMAT_R32G32B32A32_FLOAT whereas if it's going through the
    * constant or data cache, UBOs must use VK_FORMAT_RAW.
    */
   bool indirect_ubos_use_sampler;

   /**
    * Gfx12.5+ has a bit in the SEND instruction extending the bindless
    * surface offset range from 20 to 26 bits, effectively giving us 4Gb of
    * bindless surface descriptors instead of 64Mb previously.
    */
   bool extended_bindless_surface_offset;

   /**
    * Gfx11+ has a bit in the dword 3 of the sampler message header that
    * indicates whether the sampler handle is relative to the dynamic state
    * base address (0) or the bindless sampler base address (1). The driver
    * can select this.
    */
   bool use_bindless_sampler_offset;

   /**
    * Should DPAS instructions be lowered?
    *
    * This will be set for all platforms before Gfx12.5. It may also be set
    * platforms that support DPAS for testing purposes.
    */
   bool lower_dpas;

   /**
    * Calling the ra_allocate function after each register spill can take
    * several minutes. This option speeds up shader compilation by spilling
    * more registers after the ra_allocate failure. Required for
    * Cyberpunk 2077, which uses a watchdog thread to terminate the process
    * in case the render thread hasn't responded within 2 minutes.
    */
   int spilling_rate;

   struct nir_shader *clc_shader;

   struct {
      unsigned mue_header_packing;
      bool mue_compaction;
   } mesh;
};

#define brw_shader_debug_log(compiler, data, fmt, ... ) do {    \
   static unsigned id = 0;                                      \
   compiler->shader_debug_log(data, &id, fmt, ##__VA_ARGS__);   \
} while (0)

#define brw_shader_perf_log(compiler, data, fmt, ... ) do {     \
   static unsigned id = 0;                                      \
   compiler->shader_perf_log(data, &id, fmt, ##__VA_ARGS__);    \
} while (0)

/**
 * We use a constant subgroup size of 32.  It really only needs to be a
 * maximum and, since we do SIMD32 for compute shaders in some cases, it
 * needs to be at least 32.  SIMD8 and SIMD16 shaders will still claim a
 * subgroup size of 32 but will act as if 16 or 24 of those channels are
 * disabled.
 */
#define BRW_SUBGROUP_SIZE 32

static inline bool
brw_shader_stage_is_bindless(gl_shader_stage stage)
{
   return stage >= MESA_SHADER_RAYGEN &&
          stage <= MESA_SHADER_CALLABLE;
}

static inline bool
brw_shader_stage_requires_bindless_resources(gl_shader_stage stage)
{
   return brw_shader_stage_is_bindless(stage) || gl_shader_stage_is_mesh(stage);
}

/**
 * Program key structures.
 *
 * When drawing, we look for the currently bound shaders in the program
 * cache.  This is essentially a hash table lookup, and these are the keys.
 *
 * Sometimes OpenGL features specified as state need to be simulated via
 * shader code, due to a mismatch between the API and the hardware.  This
 * is often referred to as "non-orthagonal state" or "NOS".  We store NOS
 * in the program key so it's considered when searching for a program.  If
 * we haven't seen a particular combination before, we have to recompile a
 * new specialized version.
 *
 * Shader compilation should not look up state in gl_context directly, but
 * instead use the copy in the program key.  This guarantees recompiles will
 * happen correctly.
 *
 *  @{
 */

enum PACKED gfx6_gather_sampler_wa {
   WA_SIGN = 1,      /* whether we need to sign extend */
   WA_8BIT = 2,      /* if we have an 8bit format needing wa */
   WA_16BIT = 4,     /* if we have a 16bit format needing wa */
};

#define BRW_MAX_SAMPLERS 32

/* Provide explicit padding for each member, to ensure that the compiler
 * initializes every bit in the shader cache keys.  The keys will be compared
 * with memcmp.
 */
PRAGMA_DIAGNOSTIC_PUSH
PRAGMA_DIAGNOSTIC_ERROR(-Wpadded)

/**
 * Sampler information needed by VS, WM, and GS program cache keys.
 */
struct brw_sampler_prog_key_data {
   /**
    * EXT_texture_swizzle and DEPTH_TEXTURE_MODE swizzles.
    *
    * This field is not consumed by the back-end compiler and is only relevant
    * for the crocus OpenGL driver for Broadwell and earlier hardware.
    */
   uint16_t swizzles[BRW_MAX_SAMPLERS];

   uint32_t gl_clamp_mask[3];

   /**
    * For RG32F, gather4's channel select is broken.
    */
   uint32_t gather_channel_quirk_mask;

   /**
    * For Sandybridge, which shader w/a we need for gather quirks.
    */
   enum gfx6_gather_sampler_wa gfx6_gather_wa[BRW_MAX_SAMPLERS];
};

enum brw_robustness_flags {
   BRW_ROBUSTNESS_UBO  = BITFIELD_BIT(0),
   BRW_ROBUSTNESS_SSBO = BITFIELD_BIT(1),
};

struct brw_base_prog_key {
   unsigned program_string_id;

   enum brw_robustness_flags robust_flags:2;

   unsigned padding:22;

   /**
    * Apply workarounds for SIN and COS input range problems.
    * This limits input range for SIN and COS to [-2p : 2p] to
    * avoid precision issues.
    */
   bool limit_trig_input_range;

   struct brw_sampler_prog_key_data tex;
};

/**
 * The VF can't natively handle certain types of attributes, such as GL_FIXED
 * or most 10_10_10_2 types.  These flags enable various VS workarounds to
 * "fix" attributes at the beginning of shaders.
 */
#define BRW_ATTRIB_WA_COMPONENT_MASK    7  /* mask for GL_FIXED scale channel count */
#define BRW_ATTRIB_WA_NORMALIZE     8   /* normalize in shader */
#define BRW_ATTRIB_WA_BGRA          16  /* swap r/b channels in shader */
#define BRW_ATTRIB_WA_SIGN          32  /* interpret as signed in shader */
#define BRW_ATTRIB_WA_SCALE         64  /* interpret as scaled in shader */

/**
 * OpenGL attribute slots fall in [0, VERT_ATTRIB_MAX - 1] with the range
 * [VERT_ATTRIB_GENERIC0, VERT_ATTRIB_MAX - 1] reserved for up to 16 user
 * input vertex attributes. In Vulkan, we expose up to 28 user vertex input
 * attributes that are mapped to slots also starting at VERT_ATTRIB_GENERIC0.
 */
#define MAX_GL_VERT_ATTRIB     VERT_ATTRIB_MAX
#define MAX_VK_VERT_ATTRIB     (VERT_ATTRIB_GENERIC0 + 28)

/**
 * Max number of binding table entries used for stream output.
 *
 * From the OpenGL 3.0 spec, table 6.44 (Transform Feedback State), the
 * minimum value of MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS is 64.
 *
 * On Gfx6, the size of transform feedback data is limited not by the number
 * of components but by the number of binding table entries we set aside.  We
 * use one binding table entry for a float, one entry for a vector, and one
 * entry per matrix column.  Since the only way we can communicate our
 * transform feedback capabilities to the client is via
 * MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, we need to plan for the
 * worst case, in which all the varyings are floats, so we use up one binding
 * table entry per component.  Therefore we need to set aside at least 64
 * binding table entries for use by transform feedback.
 *
 * Note: since we don't currently pack varyings, it is currently impossible
 * for the client to actually use up all of these binding table entries--if
 * all of their varyings were floats, they would run out of varying slots and
 * fail to link.  But that's a bug, so it seems prudent to go ahead and
 * allocate the number of binding table entries we will need once the bug is
 * fixed.
 */
#define BRW_MAX_SOL_BINDINGS 64

/** The program key for Vertex Shaders. */
struct brw_vs_prog_key {
   struct brw_base_prog_key base;

   /**
    * Per-attribute workaround flags
    *
    * For each attribute, a combination of BRW_ATTRIB_WA_*.
    *
    * For OpenGL, where we expose a maximum of 16 user input attributes
    * we only need up to VERT_ATTRIB_MAX slots, however, in Vulkan
    * slots preceding VERT_ATTRIB_GENERIC0 are unused and we can
    * expose up to 28 user input vertex attributes that are mapped to slots
    * starting at VERT_ATTRIB_GENERIC0, so this array needs to be large
    * enough to hold this many slots.
    */
   uint8_t gl_attrib_wa_flags[MAX2(MAX_GL_VERT_ATTRIB, MAX_VK_VERT_ATTRIB)];

   /**
    * For pre-Gfx6 hardware, a bitfield indicating which texture coordinates
    * are going to be replaced with point coordinates (as a consequence of a
    * call to glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE)).  Because
    * our SF thread requires exact matching between VS outputs and FS inputs,
    * these texture coordinates will need to be unconditionally included in
    * the VUE, even if they aren't written by the vertex shader.
    */
   uint8_t point_coord_replace;
   unsigned clamp_pointsize:1;

   bool copy_edgeflag:1;

   bool clamp_vertex_color:1;

   /**
    * How many user clipping planes are being uploaded to the vertex shader as
    * push constants.
    *
    * These are used for lowering legacy gl_ClipVertex/gl_Position clipping to
    * clip distances.
    */
   unsigned nr_userclip_plane_consts:4;

   uint32_t padding: 25;
};

/** The program key for Tessellation Control Shaders. */
struct brw_tcs_prog_key
{
   struct brw_base_prog_key base;

   /** A bitfield of per-vertex outputs written. */
   uint64_t outputs_written;

   enum tess_primitive_mode _tes_primitive_mode;

   /** Number of input vertices, 0 means dynamic */
   unsigned input_vertices;

   /** A bitfield of per-patch outputs written. */
   uint32_t patch_outputs_written;

   bool quads_workaround;
   uint32_t padding:24;
};

#define BRW_MAX_TCS_INPUT_VERTICES (32)

static inline uint32_t
brw_tcs_prog_key_input_vertices(const struct brw_tcs_prog_key *key)
{
   return key->input_vertices != 0 ?
          key->input_vertices : BRW_MAX_TCS_INPUT_VERTICES;
}

/** The program key for Tessellation Evaluation Shaders. */
struct brw_tes_prog_key
{
   struct brw_base_prog_key base;

   /** A bitfield of per-vertex inputs read. */
   uint64_t inputs_read;

   /** A bitfield of per-patch inputs read. */
   uint32_t patch_inputs_read;

   /**
    * How many user clipping planes are being uploaded to the tessellation
    * evaluation shader as push constants.
    *
    * These are used for lowering legacy gl_ClipVertex/gl_Position clipping to
    * clip distances.
    */
   unsigned nr_userclip_plane_consts:4;
   unsigned clamp_pointsize:1;
   uint32_t padding:27;
};

/** The program key for Geometry Shaders. */
struct brw_gs_prog_key
{
   struct brw_base_prog_key base;

   /**
    * How many user clipping planes are being uploaded to the geometry shader
    * as push constants.
    *
    * These are used for lowering legacy gl_ClipVertex/gl_Position clipping to
    * clip distances.
    */
   unsigned nr_userclip_plane_consts:4;
   unsigned clamp_pointsize:1;
   unsigned padding:27;
};

struct brw_task_prog_key
{
   struct brw_base_prog_key base;
};

struct brw_mesh_prog_key
{
   struct brw_base_prog_key base;

   bool compact_mue:1;
   unsigned padding:31;
};

enum brw_sf_primitive {
   BRW_SF_PRIM_POINTS = 0,
   BRW_SF_PRIM_LINES = 1,
   BRW_SF_PRIM_TRIANGLES = 2,
   BRW_SF_PRIM_UNFILLED_TRIS = 3,
};

struct brw_sf_prog_key {
   uint64_t attrs;
   bool contains_flat_varying;
   unsigned char interp_mode[65]; /* BRW_VARYING_SLOT_COUNT */
   uint8_t point_sprite_coord_replace;
   enum brw_sf_primitive primitive:2;
   bool do_twoside_color:1;
   bool frontface_ccw:1;
   bool do_point_sprite:1;
   bool do_point_coord:1;
   bool sprite_origin_lower_left:1;
   bool userclip_active:1;
   unsigned padding: 32;
};

enum brw_clip_mode {
   BRW_CLIP_MODE_NORMAL             = 0,
   BRW_CLIP_MODE_CLIP_ALL           = 1,
   BRW_CLIP_MODE_CLIP_NON_REJECTED  = 2,
   BRW_CLIP_MODE_REJECT_ALL         = 3,
   BRW_CLIP_MODE_ACCEPT_ALL         = 4,
   BRW_CLIP_MODE_KERNEL_CLIP        = 5,
};

enum brw_clip_fill_mode {
   BRW_CLIP_FILL_MODE_LINE = 0,
   BRW_CLIP_FILL_MODE_POINT = 1,
   BRW_CLIP_FILL_MODE_FILL = 2,
   BRW_CLIP_FILL_MODE_CULL = 3,
};

/* Note that if unfilled primitives are being emitted, we have to fix
 * up polygon offset and flatshading at this point:
 */
struct brw_clip_prog_key {
   uint64_t attrs;
   float offset_factor;
   float offset_units;
   float offset_clamp;
   bool contains_flat_varying;
   bool contains_noperspective_varying;
   unsigned char interp_mode[65]; /* BRW_VARYING_SLOT_COUNT */
   unsigned primitive:4;
   unsigned nr_userclip:4;
   bool pv_first:1;
   bool do_unfilled:1;
   enum brw_clip_fill_mode fill_cw:2;  /* includes cull information */
   enum brw_clip_fill_mode fill_ccw:2; /* includes cull information */
   bool offset_cw:1;
   bool offset_ccw:1;
   bool copy_bfc_cw:1;
   bool copy_bfc_ccw:1;
   enum brw_clip_mode clip_mode:3;
   uint64_t padding:51;
};

/* A big lookup table is used to figure out which and how many
 * additional regs will inserted before the main payload in the WM
 * program execution.  These mainly relate to depth and stencil
 * processing and the early-depth-test optimization.
 */
enum brw_wm_iz_bits {
   BRW_WM_IZ_PS_KILL_ALPHATEST_BIT     = 0x1,
   BRW_WM_IZ_PS_COMPUTES_DEPTH_BIT     = 0x2,
   BRW_WM_IZ_DEPTH_WRITE_ENABLE_BIT    = 0x4,
   BRW_WM_IZ_DEPTH_TEST_ENABLE_BIT     = 0x8,
   BRW_WM_IZ_STENCIL_WRITE_ENABLE_BIT  = 0x10,
   BRW_WM_IZ_STENCIL_TEST_ENABLE_BIT   = 0x20,
   BRW_WM_IZ_BIT_MAX                   = 0x40
};

enum brw_sometimes {
   BRW_NEVER = 0,
   BRW_SOMETIMES,
   BRW_ALWAYS
};

static inline enum brw_sometimes
brw_sometimes_invert(enum brw_sometimes x)
{
   return (enum brw_sometimes)((int)BRW_ALWAYS - (int)x);
}

/** The program key for Fragment/Pixel Shaders. */
struct brw_wm_prog_key {
   struct brw_base_prog_key base;

   uint64_t input_slots_valid;
   float alpha_test_ref;
   uint8_t color_outputs_valid;

   /* Some collection of BRW_WM_IZ_* */
   uint8_t iz_lookup;
   bool stats_wm:1;
   bool flat_shade:1;
   unsigned nr_color_regions:5;
   bool emit_alpha_test:1;
   enum compare_func alpha_test_func:3; /* < For Gfx4/5 MRT alpha test */
   bool alpha_test_replicate_alpha:1;
   enum brw_sometimes alpha_to_coverage:2;
   bool clamp_fragment_color:1;

   bool force_dual_color_blend:1;

   /** Whether or inputs are interpolated at sample rate by default
    *
    * This corresponds to the sample shading API bit in Vulkan or OpenGL which
    * controls how inputs with no interpolation qualifier are interpolated.
    * This is distinct from the way that using gl_SampleID or similar requires
    * us to run per-sample.  Even when running per-sample due to gl_SampleID,
    * we may still interpolate unqualified inputs at the pixel center.
    */
   enum brw_sometimes persample_interp:2;

   /* Whether or not we are running on a multisampled framebuffer */
   enum brw_sometimes multisample_fbo:2;

   enum brw_sometimes line_aa:2;

   /* Whether the preceding shader stage is mesh */
   enum brw_sometimes mesh_input:2;

   bool coherent_fb_fetch:1;
   bool ignore_sample_mask_out:1;
   bool coarse_pixel:1;

   uint64_t padding:53;
};

struct brw_cs_prog_key {
   struct brw_base_prog_key base;
};

struct brw_bs_prog_key {
   struct brw_base_prog_key base;

   /* Represents enum enum brw_rt_ray_flags values given at pipeline creation
    * to be combined with ray_flags handed to the traceRayEXT() calls by the
    * shader.
    */
   uint32_t pipeline_ray_flags;
};

struct brw_ff_gs_prog_key {
   uint64_t attrs;

   /**
    * Map from the index of a transform feedback binding table entry to the
    * gl_varying_slot that should be streamed out through that binding table
    * entry.
    */
   unsigned char transform_feedback_bindings[BRW_MAX_SOL_BINDINGS];

   /**
    * Map from the index of a transform feedback binding table entry to the
    * swizzles that should be used when streaming out data through that
    * binding table entry.
    */
   unsigned char transform_feedback_swizzles[BRW_MAX_SOL_BINDINGS];

   /**
    * Hardware primitive type being drawn, e.g. _3DPRIM_TRILIST.
    */
   unsigned primitive:8;

   unsigned pv_first:1;
   unsigned need_gs_prog:1;

   /**
    * Number of varyings that are output to transform feedback.
    */
   unsigned num_transform_feedback_bindings:7; /* 0-BRW_MAX_SOL_BINDINGS */
   uint64_t padding:47;
};

/* brw_any_prog_key is any of the keys that map to an API stage */
union brw_any_prog_key {
   struct brw_base_prog_key base;
   struct brw_vs_prog_key vs;
   struct brw_tcs_prog_key tcs;
   struct brw_tes_prog_key tes;
   struct brw_gs_prog_key gs;
   struct brw_wm_prog_key wm;
   struct brw_cs_prog_key cs;
   struct brw_bs_prog_key bs;
   struct brw_task_prog_key task;
   struct brw_mesh_prog_key mesh;
};

PRAGMA_DIAGNOSTIC_POP

/*
 * Image metadata structure as laid out in the shader parameter
 * buffer.  Entries have to be 16B-aligned for the vec4 back-end to be
 * able to use them.  That's okay because the padding and any unused
 * entries [most of them except when we're doing untyped surface
 * access] will be removed by the uniform packing pass.
 */
#define BRW_IMAGE_PARAM_OFFSET_OFFSET           0
#define BRW_IMAGE_PARAM_SIZE_OFFSET             4
#define BRW_IMAGE_PARAM_STRIDE_OFFSET           8
#define BRW_IMAGE_PARAM_TILING_OFFSET           12
#define BRW_IMAGE_PARAM_SWIZZLING_OFFSET        16
#define BRW_IMAGE_PARAM_SIZE                    20

struct brw_image_param {
   /** Offset applied to the X and Y surface coordinates. */
   uint32_t offset[2];

   /** Surface X, Y and Z dimensions. */
   uint32_t size[3];

   /** X-stride in bytes, Y-stride in pixels, horizontal slice stride in
    * pixels, vertical slice stride in pixels.
    */
   uint32_t stride[4];

   /** Log2 of the tiling modulus in the X, Y and Z dimension. */
   uint32_t tiling[3];

   /**
    * Right shift to apply for bit 6 address swizzling.  Two different
    * swizzles can be specified and will be applied one after the other.  The
    * resulting address will be:
    *
    *  addr' = addr ^ ((1 << 6) & ((addr >> swizzling[0]) ^
    *                              (addr >> swizzling[1])))
    *
    * Use \c 0xff if any of the swizzles is not required.
    */
   uint32_t swizzling[2];
};

/** Max number of render targets in a shader */
#define BRW_MAX_DRAW_BUFFERS 8

/**
 * Binding table index for the first gfx6 SOL binding.
 */
#define BRW_GFX6_SOL_BINDING_START 0

struct brw_ubo_range
{
   uint16_t block;

   /* In units of 32-byte registers */
   uint8_t start;
   uint8_t length;
};

/* We reserve the first 2^16 values for builtins */
#define BRW_PARAM_IS_BUILTIN(param) (((param) & 0xffff0000) == 0)

enum brw_param_builtin {
   BRW_PARAM_BUILTIN_ZERO,

   BRW_PARAM_BUILTIN_CLIP_PLANE_0_X,
   BRW_PARAM_BUILTIN_CLIP_PLANE_0_Y,
   BRW_PARAM_BUILTIN_CLIP_PLANE_0_Z,
   BRW_PARAM_BUILTIN_CLIP_PLANE_0_W,
   BRW_PARAM_BUILTIN_CLIP_PLANE_1_X,
   BRW_PARAM_BUILTIN_CLIP_PLANE_1_Y,
   BRW_PARAM_BUILTIN_CLIP_PLANE_1_Z,
   BRW_PARAM_BUILTIN_CLIP_PLANE_1_W,
   BRW_PARAM_BUILTIN_CLIP_PLANE_2_X,
   BRW_PARAM_BUILTIN_CLIP_PLANE_2_Y,
   BRW_PARAM_BUILTIN_CLIP_PLANE_2_Z,
   BRW_PARAM_BUILTIN_CLIP_PLANE_2_W,
   BRW_PARAM_BUILTIN_CLIP_PLANE_3_X,
   BRW_PARAM_BUILTIN_CLIP_PLANE_3_Y,
   BRW_PARAM_BUILTIN_CLIP_PLANE_3_Z,
   BRW_PARAM_BUILTIN_CLIP_PLANE_3_W,
   BRW_PARAM_BUILTIN_CLIP_PLANE_4_X,
   BRW_PARAM_BUILTIN_CLIP_PLANE_4_Y,
   BRW_PARAM_BUILTIN_CLIP_PLANE_4_Z,
   BRW_PARAM_BUILTIN_CLIP_PLANE_4_W,
   BRW_PARAM_BUILTIN_CLIP_PLANE_5_X,
   BRW_PARAM_BUILTIN_CLIP_PLANE_5_Y,
   BRW_PARAM_BUILTIN_CLIP_PLANE_5_Z,
   BRW_PARAM_BUILTIN_CLIP_PLANE_5_W,
   BRW_PARAM_BUILTIN_CLIP_PLANE_6_X,
   BRW_PARAM_BUILTIN_CLIP_PLANE_6_Y,
   BRW_PARAM_BUILTIN_CLIP_PLANE_6_Z,
   BRW_PARAM_BUILTIN_CLIP_PLANE_6_W,
   BRW_PARAM_BUILTIN_CLIP_PLANE_7_X,
   BRW_PARAM_BUILTIN_CLIP_PLANE_7_Y,
   BRW_PARAM_BUILTIN_CLIP_PLANE_7_Z,
   BRW_PARAM_BUILTIN_CLIP_PLANE_7_W,

   BRW_PARAM_BUILTIN_TESS_LEVEL_OUTER_X,
   BRW_PARAM_BUILTIN_TESS_LEVEL_OUTER_Y,
   BRW_PARAM_BUILTIN_TESS_LEVEL_OUTER_Z,
   BRW_PARAM_BUILTIN_TESS_LEVEL_OUTER_W,
   BRW_PARAM_BUILTIN_TESS_LEVEL_INNER_X,
   BRW_PARAM_BUILTIN_TESS_LEVEL_INNER_Y,

   BRW_PARAM_BUILTIN_PATCH_VERTICES_IN,

   BRW_PARAM_BUILTIN_BASE_WORK_GROUP_ID_X,
   BRW_PARAM_BUILTIN_BASE_WORK_GROUP_ID_Y,
   BRW_PARAM_BUILTIN_BASE_WORK_GROUP_ID_Z,
   BRW_PARAM_BUILTIN_SUBGROUP_ID,
   BRW_PARAM_BUILTIN_WORK_GROUP_SIZE_X,
   BRW_PARAM_BUILTIN_WORK_GROUP_SIZE_Y,
   BRW_PARAM_BUILTIN_WORK_GROUP_SIZE_Z,
   BRW_PARAM_BUILTIN_WORK_DIM,
};

#define BRW_PARAM_BUILTIN_CLIP_PLANE(idx, comp) \
   (BRW_PARAM_BUILTIN_CLIP_PLANE_0_X + ((idx) << 2) + (comp))

#define BRW_PARAM_BUILTIN_IS_CLIP_PLANE(param)  \
   ((param) >= BRW_PARAM_BUILTIN_CLIP_PLANE_0_X && \
    (param) <= BRW_PARAM_BUILTIN_CLIP_PLANE_7_W)

#define BRW_PARAM_BUILTIN_CLIP_PLANE_IDX(param) \
   (((param) - BRW_PARAM_BUILTIN_CLIP_PLANE_0_X) >> 2)

#define BRW_PARAM_BUILTIN_CLIP_PLANE_COMP(param) \
   (((param) - BRW_PARAM_BUILTIN_CLIP_PLANE_0_X) & 0x3)

enum brw_shader_reloc_id {
   BRW_SHADER_RELOC_CONST_DATA_ADDR_LOW,
   BRW_SHADER_RELOC_CONST_DATA_ADDR_HIGH,
   BRW_SHADER_RELOC_SHADER_START_OFFSET,
   BRW_SHADER_RELOC_RESUME_SBT_ADDR_LOW,
   BRW_SHADER_RELOC_RESUME_SBT_ADDR_HIGH,
   BRW_SHADER_RELOC_DESCRIPTORS_ADDR_HIGH,
};

enum brw_shader_reloc_type {
   /** An arbitrary 32-bit value */
   BRW_SHADER_RELOC_TYPE_U32,
   /** A MOV instruction with an immediate source */
   BRW_SHADER_RELOC_TYPE_MOV_IMM,
};

/** Represents a code relocation
 *
 * Relocatable constants are immediates in the code which we want to be able
 * to replace post-compile with the actual value.
 */
struct brw_shader_reloc {
   /** The 32-bit ID of the relocatable constant */
   uint32_t id;

   /** Type of this relocation */
   enum brw_shader_reloc_type type;

   /** The offset in the shader to the relocated value
    *
    * For MOV_IMM relocs, this is an offset to the MOV instruction.  This
    * allows us to do some sanity checking while we update the value.
    */
   uint32_t offset;

   /** Value to be added to the relocated value before it is written */
   uint32_t delta;
};

/** A value to write to a relocation */
struct brw_shader_reloc_value {
   /** The 32-bit ID of the relocatable constant */
   uint32_t id;

   /** The value with which to replace the relocated immediate */
   uint32_t value;
};

struct brw_stage_prog_data {
   struct brw_ubo_range ubo_ranges[4];

   unsigned nr_params;       /**< number of float params/constants */

   gl_shader_stage stage;

   /* zero_push_reg is a bitfield which indicates what push registers (if any)
    * should be zeroed by SW at the start of the shader.  The corresponding
    * push_reg_mask_param specifies the param index (in 32-bit units) where
    * the actual runtime 64-bit mask will be pushed.  The shader will zero
    * push reg i if
    *
    *    reg_used & zero_push_reg & ~*push_reg_mask_param & (1ull << i)
    *
    * If this field is set, brw_compiler::compact_params must be false.
    */
   uint64_t zero_push_reg;
   unsigned push_reg_mask_param;

   unsigned curb_read_length;
   unsigned total_scratch;
   unsigned total_shared;

   unsigned program_size;

   unsigned const_data_size;
   unsigned const_data_offset;

   unsigned num_relocs;
   const struct brw_shader_reloc *relocs;

   /** Does this program pull from any UBO or other constant buffers? */
   bool has_ubo_pull;

   /** How many ray queries objects in this shader. */
   unsigned ray_queries;

   /**
    * Register where the thread expects to find input data from the URB
    * (typically uniforms, followed by vertex or fragment attributes).
    */
   unsigned dispatch_grf_start_reg;

   bool use_alt_mode; /**< Use ALT floating point mode?  Otherwise, IEEE. */

   /* 32-bit identifiers for all push/pull parameters.  These can be anything
    * the driver wishes them to be; the core of the back-end compiler simply
    * re-arranges them.  The one restriction is that the bottom 2^16 values
    * are reserved for builtins defined in the brw_param_builtin enum defined
    * above.
    */
   uint32_t *param;

   /* Whether shader uses atomic operations. */
   bool uses_atomic_load_store;
};

static inline uint32_t *
brw_stage_prog_data_add_params(struct brw_stage_prog_data *prog_data,
                               unsigned nr_new_params)
{
   unsigned old_nr_params = prog_data->nr_params;
   prog_data->nr_params += nr_new_params;
   prog_data->param = reralloc(ralloc_parent(prog_data->param),
                               prog_data->param, uint32_t,
                               prog_data->nr_params);
   return prog_data->param + old_nr_params;
}

enum brw_barycentric_mode {
   BRW_BARYCENTRIC_PERSPECTIVE_PIXEL       = 0,
   BRW_BARYCENTRIC_PERSPECTIVE_CENTROID    = 1,
   BRW_BARYCENTRIC_PERSPECTIVE_SAMPLE      = 2,
   BRW_BARYCENTRIC_NONPERSPECTIVE_PIXEL    = 3,
   BRW_BARYCENTRIC_NONPERSPECTIVE_CENTROID = 4,
   BRW_BARYCENTRIC_NONPERSPECTIVE_SAMPLE   = 5,
   BRW_BARYCENTRIC_MODE_COUNT              = 6
};
#define BRW_BARYCENTRIC_PERSPECTIVE_BITS \
   ((1 << BRW_BARYCENTRIC_PERSPECTIVE_PIXEL) | \
    (1 << BRW_BARYCENTRIC_PERSPECTIVE_CENTROID) | \
    (1 << BRW_BARYCENTRIC_PERSPECTIVE_SAMPLE))
#define BRW_BARYCENTRIC_NONPERSPECTIVE_BITS \
   ((1 << BRW_BARYCENTRIC_NONPERSPECTIVE_PIXEL) | \
    (1 << BRW_BARYCENTRIC_NONPERSPECTIVE_CENTROID) | \
    (1 << BRW_BARYCENTRIC_NONPERSPECTIVE_SAMPLE))

enum brw_pixel_shader_computed_depth_mode {
   BRW_PSCDEPTH_OFF   = 0, /* PS does not compute depth */
   BRW_PSCDEPTH_ON    = 1, /* PS computes depth; no guarantee about value */
   BRW_PSCDEPTH_ON_GE = 2, /* PS guarantees output depth >= source depth */
   BRW_PSCDEPTH_ON_LE = 3, /* PS guarantees output depth <= source depth */
};

enum brw_wm_msaa_flags {
   /** Must be set whenever any dynamic MSAA is used
    *
    * This flag mostly exists to let us assert that the driver understands
    * dynamic MSAA so we don't run into trouble with drivers that don't.
    */
   BRW_WM_MSAA_FLAG_ENABLE_DYNAMIC = (1 << 0),

   /** True if the framebuffer is multisampled */
   BRW_WM_MSAA_FLAG_MULTISAMPLE_FBO = (1 << 1),

   /** True if this shader has been dispatched per-sample */
   BRW_WM_MSAA_FLAG_PERSAMPLE_DISPATCH = (1 << 2),

   /** True if inputs should be interpolated per-sample by default */
   BRW_WM_MSAA_FLAG_PERSAMPLE_INTERP = (1 << 3),

   /** True if this shader has been dispatched with alpha-to-coverage */
   BRW_WM_MSAA_FLAG_ALPHA_TO_COVERAGE = (1 << 4),

   /** True if this shader has been dispatched coarse
    *
    * This is intentionally chose to be bit 15 to correspond to the coarse bit
    * in the pixel interpolator messages.
    */
   BRW_WM_MSAA_FLAG_COARSE_PI_MSG = (1 << 15),

   /** True if this shader has been dispatched coarse
    *
    * This is intentionally chose to be bit 18 to correspond to the coarse bit
    * in the render target messages.
    */
   BRW_WM_MSAA_FLAG_COARSE_RT_WRITES = (1 << 18),
};
MESA_DEFINE_CPP_ENUM_BITFIELD_OPERATORS(enum brw_wm_msaa_flags)

/* Data about a particular attempt to compile a program.  Note that
 * there can be many of these, each in a different GL state
 * corresponding to a different brw_wm_prog_key struct, with different
 * compiled programs.
 */
struct brw_wm_prog_data {
   struct brw_stage_prog_data base;

   unsigned num_per_primitive_inputs;
   unsigned num_varying_inputs;

   uint8_t reg_blocks_8;
   uint8_t reg_blocks_16;
   uint8_t reg_blocks_32;

   uint8_t dispatch_grf_start_reg_16;
   uint8_t dispatch_grf_start_reg_32;
   uint32_t prog_offset_16;
   uint32_t prog_offset_32;

   struct {
      /** @{
       * surface indices the WM-specific surfaces
       */
      uint32_t render_target_read_start;
      /** @} */
   } binding_table;

   uint8_t color_outputs_written;
   uint8_t computed_depth_mode;

   /**
    * Number of polygons handled in parallel by the multi-polygon PS
    * kernel.
    */
   uint8_t max_polygons;

   /**
    * Dispatch width of the multi-polygon PS kernel, or 0 if no
    * multi-polygon kernel was built.
    */
   uint8_t dispatch_multi;

   bool computed_stencil;
   bool early_fragment_tests;
   bool post_depth_coverage;
   bool inner_coverage;
   bool dispatch_8;
   bool dispatch_16;
   bool dispatch_32;
   bool dual_src_blend;
   bool uses_pos_offset;
   bool uses_omask;
   bool uses_kill;
   bool uses_src_depth;
   bool uses_src_w;
   bool uses_depth_w_coefficients;
   bool uses_sample_mask;
   bool uses_vmask;
   bool has_render_target_reads;
   bool has_side_effects;
   bool pulls_bary;

   bool contains_flat_varying;
   bool contains_noperspective_varying;

   /** True if the shader wants sample shading
    *
    * This corresponds to whether or not a gl_SampleId, gl_SamplePosition, or
    * a sample-qualified input are used in the shader.  It is independent of
    * GL_MIN_SAMPLE_SHADING_VALUE in GL or minSampleShading in Vulkan.
    */
   bool sample_shading;

   /** Should this shader be dispatched per-sample */
   enum brw_sometimes persample_dispatch;

   /**
    * Shader is ran at the coarse pixel shading dispatch rate (3DSTATE_CPS).
    */
   enum brw_sometimes coarse_pixel_dispatch;

   /**
    * Shader writes the SampleMask and this is AND-ed with the API's
    * SampleMask to generate a new coverage mask.
    */
   enum brw_sometimes alpha_to_coverage;

   unsigned msaa_flags_param;

   /**
    * Mask of which interpolation modes are required by the fragment shader.
    * Those interpolations are delivered as part of the thread payload. Used
    * in hardware setup on gfx6+.
    */
   uint32_t barycentric_interp_modes;

   /**
    * Whether nonperspective interpolation modes are used by the
    * barycentric_interp_modes or fragment shader through interpolator messages.
    */
   bool uses_nonperspective_interp_modes;

   /**
    * Mask of which FS inputs are marked flat by the shader source.  This is
    * needed for setting up 3DSTATE_SF/SBE.
    */
   uint32_t flat_inputs;

   /**
    * The FS inputs
    */
   uint64_t inputs;

   /* Mapping of VUE slots to interpolation modes.
    * Used by the Gfx4-5 clip/sf/wm stages.
    */
   unsigned char interp_mode[65]; /* BRW_VARYING_SLOT_COUNT */

   /**
    * Map from gl_varying_slot to the position within the FS setup data
    * payload where the varying's attribute vertex deltas should be delivered.
    * For varying slots that are not used by the FS, the value is -1.
    */
   int urb_setup[VARYING_SLOT_MAX];
   int urb_setup_channel[VARYING_SLOT_MAX];

   /**
    * Cache structure into the urb_setup array above that contains the
    * attribute numbers of active varyings out of urb_setup.
    * The actual count is stored in urb_setup_attribs_count.
    */
   uint8_t urb_setup_attribs[VARYING_SLOT_MAX];
   uint8_t urb_setup_attribs_count;
};

#ifdef GFX_VERx10

#if GFX_VERx10 >= 200

/** Returns the SIMD width corresponding to a given KSP index
 *
 * The "Variable Pixel Dispatch" table in the PRM (which can be found, for
 * example in Vol. 7 of the SKL PRM) has a mapping from dispatch widths to
 * kernel start pointer (KSP) indices that is based on what dispatch widths
 * are enabled.  This function provides, effectively, the reverse mapping.
 *
 * If the given KSP is enabled, a SIMD width of 8, 16, or 32 is
 * returned.  Note that for a multipolygon dispatch kernel 8 is always
 * returned, since multipolygon kernels use the "_8" fields from
 * brw_wm_prog_data regardless of their SIMD width.  If the KSP is
 * invalid, 0 is returned.
 */
static inline unsigned
brw_fs_simd_width_for_ksp(unsigned ksp_idx, bool enabled, unsigned width_sel)
{
   assert(ksp_idx < 2);
   return !enabled ? 0 :
          width_sel ? 32 :
          16;
}

#define brw_wm_state_simd_width_for_ksp(wm_state, ksp_idx)              \
        (ksp_idx == 0 && (wm_state).Kernel0MaximumPolysperThread ? 8 :  \
         ksp_idx == 0 ? brw_fs_simd_width_for_ksp(ksp_idx, (wm_state).Kernel0Enable, \
                                                  (wm_state).Kernel0SIMDWidth): \
         brw_fs_simd_width_for_ksp(ksp_idx, (wm_state).Kernel1Enable,   \
                                   (wm_state).Kernel1SIMDWidth))

#else

/** Returns the SIMD width corresponding to a given KSP index
 *
 * The "Variable Pixel Dispatch" table in the PRM (which can be found, for
 * example in Vol. 7 of the SKL PRM) has a mapping from dispatch widths to
 * kernel start pointer (KSP) indices that is based on what dispatch widths
 * are enabled.  This function provides, effectively, the reverse mapping.
 *
 * If the given KSP is valid with respect to the SIMD8/16/32 enables, a SIMD
 * width of 8, 16, or 32 is returned.  If the KSP is invalid, 0 is returned.
 */
static inline unsigned
brw_fs_simd_width_for_ksp(unsigned ksp_idx, bool simd8_enabled,
                          bool simd16_enabled, bool simd32_enabled)
{
   /* This function strictly ignores contiguous dispatch */
   switch (ksp_idx) {
   case 0:
      return simd8_enabled ? 8 :
             (simd16_enabled && !simd32_enabled) ? 16 :
             (simd32_enabled && !simd16_enabled) ? 32 : 0;
   case 1:
      return (simd32_enabled && (simd16_enabled || simd8_enabled)) ? 32 : 0;
   case 2:
      return (simd16_enabled && (simd32_enabled || simd8_enabled)) ? 16 : 0;
   default:
      unreachable("Invalid KSP index");
   }
}

#define brw_wm_state_simd_width_for_ksp(wm_state, ksp_idx)              \
   brw_fs_simd_width_for_ksp((ksp_idx), (wm_state)._8PixelDispatchEnable, \
                             (wm_state)._16PixelDispatchEnable, \
                             (wm_state)._32PixelDispatchEnable)

#endif

#endif

#define brw_wm_state_has_ksp(wm_state, ksp_idx) \
   (brw_wm_state_simd_width_for_ksp((wm_state), (ksp_idx)) != 0)

static inline uint32_t
_brw_wm_prog_data_prog_offset(const struct brw_wm_prog_data *prog_data,
                              unsigned simd_width)
{
   switch (simd_width) {
   case 8: return 0;
   case 16: return prog_data->prog_offset_16;
   case 32: return prog_data->prog_offset_32;
   default: return 0;
   }
}

#define brw_wm_prog_data_prog_offset(prog_data, wm_state, ksp_idx) \
   _brw_wm_prog_data_prog_offset(prog_data, \
      brw_wm_state_simd_width_for_ksp(wm_state, ksp_idx))

static inline uint8_t
_brw_wm_prog_data_dispatch_grf_start_reg(const struct brw_wm_prog_data *prog_data,
                                         unsigned simd_width)
{
   switch (simd_width) {
   case 8: return prog_data->base.dispatch_grf_start_reg;
   case 16: return prog_data->dispatch_grf_start_reg_16;
   case 32: return prog_data->dispatch_grf_start_reg_32;
   default: return 0;
   }
}

#define brw_wm_prog_data_dispatch_grf_start_reg(prog_data, wm_state, ksp_idx) \
   _brw_wm_prog_data_dispatch_grf_start_reg(prog_data, \
      brw_wm_state_simd_width_for_ksp(wm_state, ksp_idx))

static inline uint8_t
_brw_wm_prog_data_reg_blocks(const struct brw_wm_prog_data *prog_data,
                             unsigned simd_width)
{
   switch (simd_width) {
   case 8: return prog_data->reg_blocks_8;
   case 16: return prog_data->reg_blocks_16;
   case 32: return prog_data->reg_blocks_32;
   default: return 0;
   }
}

#define brw_wm_prog_data_reg_blocks(prog_data, wm_state, ksp_idx) \
   _brw_wm_prog_data_reg_blocks(prog_data, \
      brw_wm_state_simd_width_for_ksp(wm_state, ksp_idx))

static inline bool
brw_wm_prog_data_is_persample(const struct brw_wm_prog_data *prog_data,
                              enum brw_wm_msaa_flags pushed_msaa_flags)
{
   if (pushed_msaa_flags & BRW_WM_MSAA_FLAG_ENABLE_DYNAMIC) {
      if (!(pushed_msaa_flags & BRW_WM_MSAA_FLAG_MULTISAMPLE_FBO))
         return false;

      if (prog_data->sample_shading)
         assert(pushed_msaa_flags & BRW_WM_MSAA_FLAG_PERSAMPLE_DISPATCH);

      if (pushed_msaa_flags & BRW_WM_MSAA_FLAG_PERSAMPLE_DISPATCH)
         assert(prog_data->persample_dispatch != BRW_NEVER);
      else
         assert(prog_data->persample_dispatch != BRW_ALWAYS);

      return (pushed_msaa_flags & BRW_WM_MSAA_FLAG_PERSAMPLE_DISPATCH) != 0;
   }

   assert(prog_data->persample_dispatch == BRW_ALWAYS ||
          prog_data->persample_dispatch == BRW_NEVER);

   return prog_data->persample_dispatch;
}

static inline uint32_t
wm_prog_data_barycentric_modes(const struct brw_wm_prog_data *prog_data,
                               enum brw_wm_msaa_flags pushed_msaa_flags)
{
   uint32_t modes = prog_data->barycentric_interp_modes;

   /* In the non dynamic case, we can just return the computed modes from
    * compilation time.
    */
   if (!(pushed_msaa_flags & BRW_WM_MSAA_FLAG_ENABLE_DYNAMIC))
      return modes;

   if (pushed_msaa_flags & BRW_WM_MSAA_FLAG_PERSAMPLE_INTERP) {
      assert(prog_data->persample_dispatch == BRW_ALWAYS ||
             (pushed_msaa_flags & BRW_WM_MSAA_FLAG_PERSAMPLE_DISPATCH));

      /* Making dynamic per-sample interpolation work is a bit tricky.  The
       * hardware will hang if SAMPLE is requested but per-sample dispatch is
       * not enabled.  This means we can't preemptively add SAMPLE to the
       * barycentrics bitfield.  Instead, we have to add it late and only
       * on-demand.  Annoyingly, changing the number of barycentrics requested
       * changes the whole PS shader payload so we very much don't want to do
       * that.  Instead, if the dynamic per-sample interpolation flag is set,
       * we check to see if SAMPLE was requested and, if not, replace the
       * highest barycentric bit in the [non]perspective grouping (CENTROID,
       * if it exists, else PIXEL) with SAMPLE.  The shader will stomp all the
       * barycentrics in the shader with SAMPLE so it really doesn't matter
       * which one we replace.  The important thing is that we keep the number
       * of barycentrics in each [non]perspective grouping the same.
       */
      if ((modes & BRW_BARYCENTRIC_PERSPECTIVE_BITS) &&
          !(modes & BITFIELD_BIT(BRW_BARYCENTRIC_PERSPECTIVE_SAMPLE))) {
         int sample_mode =
            util_last_bit(modes & BRW_BARYCENTRIC_PERSPECTIVE_BITS) - 1;
         assert(modes & BITFIELD_BIT(sample_mode));

         modes &= ~BITFIELD_BIT(sample_mode);
         modes |= BITFIELD_BIT(BRW_BARYCENTRIC_PERSPECTIVE_SAMPLE);
      }

      if ((modes & BRW_BARYCENTRIC_NONPERSPECTIVE_BITS) &&
          !(modes & BITFIELD_BIT(BRW_BARYCENTRIC_NONPERSPECTIVE_SAMPLE))) {
         int sample_mode =
            util_last_bit(modes & BRW_BARYCENTRIC_NONPERSPECTIVE_BITS) - 1;
         assert(modes & BITFIELD_BIT(sample_mode));

         modes &= ~BITFIELD_BIT(sample_mode);
         modes |= BITFIELD_BIT(BRW_BARYCENTRIC_NONPERSPECTIVE_SAMPLE);
      }
   } else {
      /* If we're not using per-sample interpolation, we need to disable the
       * per-sample bits.
       *
       * SKL PRMs, Volume 2a: Command Reference: Instructions,
       * 3DSTATE_WM:Barycentric Interpolation Mode:

       *    "MSDISPMODE_PERSAMPLE is required in order to select Perspective
       *     Sample or Non-perspective Sample barycentric coordinates."
       */
      modes &= ~(BITFIELD_BIT(BRW_BARYCENTRIC_PERSPECTIVE_SAMPLE) |
                 BITFIELD_BIT(BRW_BARYCENTRIC_NONPERSPECTIVE_SAMPLE));
   }

   return modes;
}

static inline bool
brw_wm_prog_data_is_coarse(const struct brw_wm_prog_data *prog_data,
                           enum brw_wm_msaa_flags pushed_msaa_flags)
{
   if (pushed_msaa_flags & BRW_WM_MSAA_FLAG_ENABLE_DYNAMIC) {
      if (pushed_msaa_flags & BRW_WM_MSAA_FLAG_COARSE_RT_WRITES)
         assert(prog_data->coarse_pixel_dispatch != BRW_NEVER);
      else
         assert(prog_data->coarse_pixel_dispatch != BRW_ALWAYS);

      return pushed_msaa_flags & BRW_WM_MSAA_FLAG_COARSE_RT_WRITES;
   }

   assert(prog_data->coarse_pixel_dispatch == BRW_ALWAYS ||
          prog_data->coarse_pixel_dispatch == BRW_NEVER);

   return prog_data->coarse_pixel_dispatch;
}

struct brw_push_const_block {
   unsigned dwords;     /* Dword count, not reg aligned */
   unsigned regs;
   unsigned size;       /* Bytes, register aligned */
};

struct brw_cs_prog_data {
   struct brw_stage_prog_data base;

   unsigned local_size[3];

   /* Program offsets for the 8/16/32 SIMD variants.  Multiple variants are
    * kept when using variable group size, and the right one can only be
    * decided at dispatch time.
    */
   unsigned prog_offset[3];

   /* Bitmask indicating which program offsets are valid. */
   unsigned prog_mask;

   /* Bitmask indicating which programs have spilled. */
   unsigned prog_spilled;

   bool uses_barrier;
   bool uses_num_work_groups;
   bool uses_inline_data;
   bool uses_btd_stack_ids;
   bool uses_systolic;

   struct {
      struct brw_push_const_block cross_thread;
      struct brw_push_const_block per_thread;
   } push;

   struct {
      /** @{
       * surface indices the CS-specific surfaces
       */
      uint32_t work_groups_start;
      /** @} */
   } binding_table;
};

static inline uint32_t
brw_cs_prog_data_prog_offset(const struct brw_cs_prog_data *prog_data,
                             unsigned dispatch_width)
{
   assert(dispatch_width == 8 ||
          dispatch_width == 16 ||
          dispatch_width == 32);
   const unsigned index = dispatch_width / 16;
   assert(prog_data->prog_mask & (1 << index));
   return prog_data->prog_offset[index];
}

struct brw_bs_prog_data {
   struct brw_stage_prog_data base;

   /** SIMD size of the root shader */
   uint8_t simd_size;

   /** Maximum stack size of all shaders */
   uint32_t max_stack_size;

   /** Offset into the shader where the resume SBT is located */
   uint32_t resume_sbt_offset;

   /** Number of resume shaders */
   uint32_t num_resume_shaders;
};

struct brw_ff_gs_prog_data {
   unsigned urb_read_length;
   unsigned total_grf;

   /**
    * Gfx6 transform feedback: Amount by which the streaming vertex buffer
    * indices should be incremented each time the GS is invoked.
    */
   unsigned svbi_postincrement_value;
};

/**
 * Enum representing the i965-specific vertex results that don't correspond
 * exactly to any element of gl_varying_slot.  The values of this enum are
 * assigned such that they don't conflict with gl_varying_slot.
 */
typedef enum
{
   BRW_VARYING_SLOT_NDC = VARYING_SLOT_MAX,
   BRW_VARYING_SLOT_PAD,
   /**
    * Technically this is not a varying but just a placeholder that
    * compile_sf_prog() inserts into its VUE map to cause the gl_PointCoord
    * builtin variable to be compiled correctly. see compile_sf_prog() for
    * more info.
    */
   BRW_VARYING_SLOT_PNTC,
   BRW_VARYING_SLOT_COUNT
} brw_varying_slot;

/**
 * We always program SF to start reading at an offset of 1 (2 varying slots)
 * from the start of the vertex URB entry.  This causes it to skip:
 * - VARYING_SLOT_PSIZ and BRW_VARYING_SLOT_NDC on gfx4-5
 * - VARYING_SLOT_PSIZ and VARYING_SLOT_POS on gfx6+
 */
#define BRW_SF_URB_ENTRY_READ_OFFSET 1

/**
 * Bitmask indicating which fragment shader inputs represent varyings (and
 * hence have to be delivered to the fragment shader by the SF/SBE stage).
 */
#define BRW_FS_VARYING_INPUT_MASK \
   (BITFIELD64_RANGE(0, VARYING_SLOT_MAX) & \
    ~VARYING_BIT_POS & ~VARYING_BIT_FACE)

/**
 * Data structure recording the relationship between the gl_varying_slot enum
 * and "slots" within the vertex URB entry (VUE).  A "slot" is defined as a
 * single octaword within the VUE (128 bits).
 *
 * Note that each BRW register contains 256 bits (2 octawords), so when
 * accessing the VUE in URB_NOSWIZZLE mode, each register corresponds to two
 * consecutive VUE slots.  When accessing the VUE in URB_INTERLEAVED mode (as
 * in a vertex shader), each register corresponds to a single VUE slot, since
 * it contains data for two separate vertices.
 */
struct brw_vue_map {
   /**
    * Bitfield representing all varying slots that are (a) stored in this VUE
    * map, and (b) actually written by the shader.  Does not include any of
    * the additional varying slots defined in brw_varying_slot.
    */
   uint64_t slots_valid;

   /**
    * Is this VUE map for a separate shader pipeline?
    *
    * Separable programs (GL_ARB_separate_shader_objects) can be mixed and matched
    * without the linker having a chance to dead code eliminate unused varyings.
    *
    * This means that we have to use a fixed slot layout, based on the output's
    * location field, rather than assigning slots in a compact contiguous block.
    */
   bool separate;

   /**
    * Map from gl_varying_slot value to VUE slot.  For gl_varying_slots that are
    * not stored in a slot (because they are not written, or because
    * additional processing is applied before storing them in the VUE), the
    * value is -1.
    */
   signed char varying_to_slot[VARYING_SLOT_TESS_MAX];

   /**
    * Map from VUE slot to gl_varying_slot value.  For slots that do not
    * directly correspond to a gl_varying_slot, the value comes from
    * brw_varying_slot.
    *
    * For slots that are not in use, the value is BRW_VARYING_SLOT_PAD.
    */
   signed char slot_to_varying[VARYING_SLOT_TESS_MAX];

   /**
    * Total number of VUE slots in use
    */
   int num_slots;

   /**
    * Number of position VUE slots.  If num_pos_slots > 1, primitive
    * replication is being used.
    */
   int num_pos_slots;

   /**
    * Number of per-patch VUE slots. Only valid for tessellation control
    * shader outputs and tessellation evaluation shader inputs.
    */
   int num_per_patch_slots;

   /**
    * Number of per-vertex VUE slots. Only valid for tessellation control
    * shader outputs and tessellation evaluation shader inputs.
    */
   int num_per_vertex_slots;
};

void brw_print_vue_map(FILE *fp, const struct brw_vue_map *vue_map,
                       gl_shader_stage stage);

/**
 * Convert a VUE slot number into a byte offset within the VUE.
 */
static inline unsigned brw_vue_slot_to_offset(unsigned slot)
{
   return 16*slot;
}

/**
 * Convert a vertex output (brw_varying_slot) into a byte offset within the
 * VUE.
 */
static inline unsigned
brw_varying_to_offset(const struct brw_vue_map *vue_map, unsigned varying)
{
   return brw_vue_slot_to_offset(vue_map->varying_to_slot[varying]);
}

void brw_compute_vue_map(const struct intel_device_info *devinfo,
                         struct brw_vue_map *vue_map,
                         uint64_t slots_valid,
                         bool separate_shader,
                         uint32_t pos_slots);

void brw_compute_tess_vue_map(struct brw_vue_map *const vue_map,
                              uint64_t slots_valid,
                              uint32_t is_patch);

/* brw_interpolation_map.c */
void brw_setup_vue_interpolation(const struct brw_vue_map *vue_map,
                                 struct nir_shader *nir,
                                 struct brw_wm_prog_data *prog_data);

enum shader_dispatch_mode {
   DISPATCH_MODE_4X1_SINGLE = 0,
   DISPATCH_MODE_4X2_DUAL_INSTANCE = 1,
   DISPATCH_MODE_4X2_DUAL_OBJECT = 2,
   DISPATCH_MODE_SIMD8 = 3,

   DISPATCH_MODE_TCS_SINGLE_PATCH = 0,
   DISPATCH_MODE_TCS_MULTI_PATCH = 2,
};

/**
 * @defgroup Tessellator parameter enumerations.
 *
 * These correspond to the hardware values in 3DSTATE_TE, and are provided
 * as part of the tessellation evaluation shader.
 *
 * @{
 */
enum brw_tess_partitioning {
   BRW_TESS_PARTITIONING_INTEGER         = 0,
   BRW_TESS_PARTITIONING_ODD_FRACTIONAL  = 1,
   BRW_TESS_PARTITIONING_EVEN_FRACTIONAL = 2,
};

enum brw_tess_output_topology {
   BRW_TESS_OUTPUT_TOPOLOGY_POINT   = 0,
   BRW_TESS_OUTPUT_TOPOLOGY_LINE    = 1,
   BRW_TESS_OUTPUT_TOPOLOGY_TRI_CW  = 2,
   BRW_TESS_OUTPUT_TOPOLOGY_TRI_CCW = 3,
};

enum brw_tess_domain {
   BRW_TESS_DOMAIN_QUAD    = 0,
   BRW_TESS_DOMAIN_TRI     = 1,
   BRW_TESS_DOMAIN_ISOLINE = 2,
};
/** @} */

struct brw_vue_prog_data {
   struct brw_stage_prog_data base;
   struct brw_vue_map vue_map;

   /** Should the hardware deliver input VUE handles for URB pull loads? */
   bool include_vue_handles;

   unsigned urb_read_length;
   unsigned total_grf;

   uint32_t clip_distance_mask;
   uint32_t cull_distance_mask;

   /* Used for calculating urb partitions.  In the VS, this is the size of the
    * URB entry used for both input and output to the thread.  In the GS, this
    * is the size of the URB entry used for output.
    */
   unsigned urb_entry_size;

   enum shader_dispatch_mode dispatch_mode;
};

struct brw_vs_prog_data {
   struct brw_vue_prog_data base;

   uint64_t inputs_read;
   uint64_t double_inputs_read;

   unsigned nr_attribute_slots;

   bool uses_vertexid;
   bool uses_instanceid;
   bool uses_is_indexed_draw;
   bool uses_firstvertex;
   bool uses_baseinstance;
   bool uses_drawid;
};

struct brw_tcs_prog_data
{
   struct brw_vue_prog_data base;

   /** Should the non-SINGLE_PATCH payload provide primitive ID? */
   bool include_primitive_id;

   /** Number vertices in output patch */
   int instances;

   /** Track patch count threshold */
   int patch_count_threshold;
};


struct brw_tes_prog_data
{
   struct brw_vue_prog_data base;

   enum brw_tess_partitioning partitioning;
   enum brw_tess_output_topology output_topology;
   enum brw_tess_domain domain;
   bool include_primitive_id;
};

struct brw_gs_prog_data
{
   struct brw_vue_prog_data base;

   unsigned vertices_in;

   /**
    * Size of an output vertex, measured in HWORDS (32 bytes).
    */
   unsigned output_vertex_size_hwords;

   unsigned output_topology;

   /**
    * Size of the control data (cut bits or StreamID bits), in hwords (32
    * bytes).  0 if there is no control data.
    */
   unsigned control_data_header_size_hwords;

   /**
    * Format of the control data (either GFX7_GS_CONTROL_DATA_FORMAT_GSCTL_SID
    * if the control data is StreamID bits, or
    * GFX7_GS_CONTROL_DATA_FORMAT_GSCTL_CUT if the control data is cut bits).
    * Ignored if control_data_header_size is 0.
    */
   unsigned control_data_format;

   bool include_primitive_id;

   /**
    * The number of vertices emitted, if constant - otherwise -1.
    */
   int static_vertex_count;

   int invocations;

   /**
    * Gfx6: Provoking vertex convention for odd-numbered triangles
    * in tristrips.
    */
   unsigned pv_first:1;

   /**
    * Gfx6: Number of varyings that are output to transform feedback.
    */
   unsigned num_transform_feedback_bindings:7; /* 0-BRW_MAX_SOL_BINDINGS */

   /**
    * Gfx6: Map from the index of a transform feedback binding table entry to the
    * gl_varying_slot that should be streamed out through that binding table
    * entry.
    */
   unsigned char transform_feedback_bindings[64 /* BRW_MAX_SOL_BINDINGS */];

   /**
    * Gfx6: Map from the index of a transform feedback binding table entry to the
    * swizzles that should be used when streaming out data through that
    * binding table entry.
    */
   unsigned char transform_feedback_swizzles[64 /* BRW_MAX_SOL_BINDINGS */];
};

struct brw_sf_prog_data {
   uint32_t urb_read_length;
   uint32_t total_grf;

   /* Each vertex may have up to 12 attributes, 4 components each,
    * except WPOS which requires only 2.  (11*4 + 2) == 44 ==> 11
    * rows.
    *
    * Actually we use 4 for each, so call it 12 rows.
    */
   unsigned urb_entry_size;
};

struct brw_clip_prog_data {
   uint32_t curb_read_length;	/* user planes? */
   uint32_t clip_mode;
   uint32_t urb_read_length;
   uint32_t total_grf;
};

struct brw_tue_map {
   uint32_t size_dw;

   uint32_t per_task_data_start_dw;
};

struct brw_mue_map {
   int32_t start_dw[VARYING_SLOT_MAX];
   uint32_t len_dw[VARYING_SLOT_MAX];
   uint32_t per_primitive_indices_dw;

   uint32_t size_dw;

   uint32_t max_primitives;
   uint32_t per_primitive_start_dw;
   uint32_t per_primitive_header_size_dw;
   uint32_t per_primitive_data_size_dw;
   uint32_t per_primitive_pitch_dw;
   bool user_data_in_primitive_header;

   uint32_t max_vertices;
   uint32_t per_vertex_start_dw;
   uint32_t per_vertex_header_size_dw;
   uint32_t per_vertex_data_size_dw;
   uint32_t per_vertex_pitch_dw;
   bool user_data_in_vertex_header;
};

struct brw_task_prog_data {
   struct brw_cs_prog_data base;
   struct brw_tue_map map;
   bool uses_drawid;
};

enum brw_mesh_index_format {
   BRW_INDEX_FORMAT_U32,
   BRW_INDEX_FORMAT_U888X,
};

struct brw_mesh_prog_data {
   struct brw_cs_prog_data base;
   struct brw_mue_map map;

   uint32_t clip_distance_mask;
   uint32_t cull_distance_mask;
   uint16_t primitive_type;

   enum brw_mesh_index_format index_format;

   bool uses_drawid;
};

/* brw_any_prog_data is prog_data for any stage that maps to an API stage */
union brw_any_prog_data {
   struct brw_stage_prog_data base;
   struct brw_vue_prog_data vue;
   struct brw_vs_prog_data vs;
   struct brw_tcs_prog_data tcs;
   struct brw_tes_prog_data tes;
   struct brw_gs_prog_data gs;
   struct brw_wm_prog_data wm;
   struct brw_cs_prog_data cs;
   struct brw_bs_prog_data bs;
   struct brw_task_prog_data task;
   struct brw_mesh_prog_data mesh;
};

#define DEFINE_PROG_DATA_DOWNCAST(STAGE, CHECK)                            \
static inline struct brw_##STAGE##_prog_data *                             \
brw_##STAGE##_prog_data(struct brw_stage_prog_data *prog_data)             \
{                                                                          \
   if (prog_data)                                                          \
      assert(CHECK);                                                       \
   return (struct brw_##STAGE##_prog_data *) prog_data;                    \
}                                                                          \
static inline const struct brw_##STAGE##_prog_data *                       \
brw_##STAGE##_prog_data_const(const struct brw_stage_prog_data *prog_data) \
{                                                                          \
   if (prog_data)                                                          \
      assert(CHECK);                                                       \
   return (const struct brw_##STAGE##_prog_data *) prog_data;              \
}

DEFINE_PROG_DATA_DOWNCAST(vs,  prog_data->stage == MESA_SHADER_VERTEX)
DEFINE_PROG_DATA_DOWNCAST(tcs, prog_data->stage == MESA_SHADER_TESS_CTRL)
DEFINE_PROG_DATA_DOWNCAST(tes, prog_data->stage == MESA_SHADER_TESS_EVAL)
DEFINE_PROG_DATA_DOWNCAST(gs,  prog_data->stage == MESA_SHADER_GEOMETRY)
DEFINE_PROG_DATA_DOWNCAST(wm,  prog_data->stage == MESA_SHADER_FRAGMENT)
DEFINE_PROG_DATA_DOWNCAST(cs,  gl_shader_stage_uses_workgroup(prog_data->stage))
DEFINE_PROG_DATA_DOWNCAST(bs,  brw_shader_stage_is_bindless(prog_data->stage))

DEFINE_PROG_DATA_DOWNCAST(vue, prog_data->stage == MESA_SHADER_VERTEX ||
                               prog_data->stage == MESA_SHADER_TESS_CTRL ||
                               prog_data->stage == MESA_SHADER_TESS_EVAL ||
                               prog_data->stage == MESA_SHADER_GEOMETRY)

DEFINE_PROG_DATA_DOWNCAST(task, prog_data->stage == MESA_SHADER_TASK)
DEFINE_PROG_DATA_DOWNCAST(mesh, prog_data->stage == MESA_SHADER_MESH)

/* These are not really brw_stage_prog_data. */
DEFINE_PROG_DATA_DOWNCAST(ff_gs, true)
DEFINE_PROG_DATA_DOWNCAST(clip,  true)
DEFINE_PROG_DATA_DOWNCAST(sf,    true)
#undef DEFINE_PROG_DATA_DOWNCAST

struct brw_compile_stats {
   uint32_t dispatch_width; /**< 0 for vec4 */
   uint32_t max_polygons;
   uint32_t max_dispatch_width;
   uint32_t instructions;
   uint32_t sends;
   uint32_t loops;
   uint32_t cycles;
   uint32_t spills;
   uint32_t fills;
   uint32_t max_live_registers;
};

/** @} */

struct brw_compiler *
brw_compiler_create(void *mem_ctx, const struct intel_device_info *devinfo);

/**
 * Returns a compiler configuration for use with disk shader cache
 *
 * This value only needs to change for settings that can cause different
 * program generation between two runs on the same hardware.
 *
 * For example, it doesn't need to be different for gen 8 and gen 9 hardware,
 * but it does need to be different if INTEL_DEBUG=nocompact is or isn't used.
 */
uint64_t
brw_get_compiler_config_value(const struct brw_compiler *compiler);

unsigned
brw_prog_data_size(gl_shader_stage stage);

unsigned
brw_prog_key_size(gl_shader_stage stage);

struct brw_compile_params {
   void *mem_ctx;

   nir_shader *nir;

   struct brw_compile_stats *stats;

   void *log_data;

   char *error_str;

   uint64_t debug_flag;

   uint32_t source_hash;
};

/**
 * Parameters for compiling a vertex shader.
 *
 * Some of these will be modified during the shader compilation.
 */
struct brw_compile_vs_params {
   struct brw_compile_params base;

   const struct brw_vs_prog_key *key;
   struct brw_vs_prog_data *prog_data;

   bool edgeflag_is_last; /* true for gallium */
};

/**
 * Compile a vertex shader.
 *
 * Returns the final assembly and updates the parameters structure.
 */
const unsigned *
brw_compile_vs(const struct brw_compiler *compiler,
               struct brw_compile_vs_params *params);

/**
 * Parameters for compiling a tessellation control shader.
 *
 * Some of these will be modified during the shader compilation.
 */
struct brw_compile_tcs_params {
   struct brw_compile_params base;

   const struct brw_tcs_prog_key *key;
   struct brw_tcs_prog_data *prog_data;
};

/**
 * Compile a tessellation control shader.
 *
 * Returns the final assembly and updates the parameters structure.
 */
const unsigned *
brw_compile_tcs(const struct brw_compiler *compiler,
                struct brw_compile_tcs_params *params);

/**
 * Parameters for compiling a tessellation evaluation shader.
 *
 * Some of these will be modified during the shader compilation.
 */
struct brw_compile_tes_params {
   struct brw_compile_params base;

   const struct brw_tes_prog_key *key;
   struct brw_tes_prog_data *prog_data;
   const struct brw_vue_map *input_vue_map;
};

/**
 * Compile a tessellation evaluation shader.
 *
 * Returns the final assembly and updates the parameters structure.
 */
const unsigned *
brw_compile_tes(const struct brw_compiler *compiler,
                struct brw_compile_tes_params *params);

/**
 * Parameters for compiling a geometry shader.
 *
 * Some of these will be modified during the shader compilation.
 */
struct brw_compile_gs_params {
   struct brw_compile_params base;

   const struct brw_gs_prog_key *key;
   struct brw_gs_prog_data *prog_data;
};

/**
 * Compile a geometry shader.
 *
 * Returns the final assembly and updates the parameters structure.
 */
const unsigned *
brw_compile_gs(const struct brw_compiler *compiler,
               struct brw_compile_gs_params *params);

/**
 * Compile a strips and fans shader.
 *
 * This is a fixed-function shader determined entirely by the shader key and
 * a VUE map.
 *
 * Returns the final assembly and the program's size.
 */
const unsigned *
brw_compile_sf(const struct brw_compiler *compiler,
               void *mem_ctx,
               const struct brw_sf_prog_key *key,
               struct brw_sf_prog_data *prog_data,
               struct brw_vue_map *vue_map,
               unsigned *final_assembly_size);

/**
 * Compile a clipper shader.
 *
 * This is a fixed-function shader determined entirely by the shader key and
 * a VUE map.
 *
 * Returns the final assembly and the program's size.
 */
const unsigned *
brw_compile_clip(const struct brw_compiler *compiler,
                 void *mem_ctx,
                 const struct brw_clip_prog_key *key,
                 struct brw_clip_prog_data *prog_data,
                 struct brw_vue_map *vue_map,
                 unsigned *final_assembly_size);

struct brw_compile_task_params {
   struct brw_compile_params base;

   const struct brw_task_prog_key *key;
   struct brw_task_prog_data *prog_data;
};

const unsigned *
brw_compile_task(const struct brw_compiler *compiler,
                 struct brw_compile_task_params *params);

struct brw_compile_mesh_params {
   struct brw_compile_params base;

   const struct brw_mesh_prog_key *key;
   struct brw_mesh_prog_data *prog_data;
   const struct brw_tue_map *tue_map;
};

const unsigned *
brw_compile_mesh(const struct brw_compiler *compiler,
                 struct brw_compile_mesh_params *params);

/**
 * Parameters for compiling a fragment shader.
 *
 * Some of these will be modified during the shader compilation.
 */
struct brw_compile_fs_params {
   struct brw_compile_params base;

   const struct brw_wm_prog_key *key;
   struct brw_wm_prog_data *prog_data;

   const struct brw_vue_map *vue_map;
   const struct brw_mue_map *mue_map;

   bool allow_spilling;
   bool use_rep_send;
   uint8_t max_polygons;
};

/**
 * Compile a fragment shader.
 *
 * Returns the final assembly and updates the parameters structure.
 */
const unsigned *
brw_compile_fs(const struct brw_compiler *compiler,
               struct brw_compile_fs_params *params);

/**
 * Parameters for compiling a compute shader.
 *
 * Some of these will be modified during the shader compilation.
 */
struct brw_compile_cs_params {
   struct brw_compile_params base;

   const struct brw_cs_prog_key *key;
   struct brw_cs_prog_data *prog_data;
};

/**
 * Compile a compute shader.
 *
 * Returns the final assembly and updates the parameters structure.
 */
const unsigned *
brw_compile_cs(const struct brw_compiler *compiler,
               struct brw_compile_cs_params *params);

/**
 * Parameters for compiling a Bindless shader.
 *
 * Some of these will be modified during the shader compilation.
 */
struct brw_compile_bs_params {
   struct brw_compile_params base;

   const struct brw_bs_prog_key *key;
   struct brw_bs_prog_data *prog_data;

   unsigned num_resume_shaders;
   struct nir_shader **resume_shaders;
};

/**
 * Compile a Bindless shader.
 *
 * Returns the final assembly and updates the parameters structure.
 */
const unsigned *
brw_compile_bs(const struct brw_compiler *compiler,
               struct brw_compile_bs_params *params);

/**
 * Compile a fixed function geometry shader.
 *
 * Returns the final assembly and the program's size.
 */
const unsigned *
brw_compile_ff_gs_prog(struct brw_compiler *compiler,
		       void *mem_ctx,
		       const struct brw_ff_gs_prog_key *key,
		       struct brw_ff_gs_prog_data *prog_data,
		       struct brw_vue_map *vue_map,
		       unsigned *final_assembly_size);

void brw_debug_key_recompile(const struct brw_compiler *c, void *log,
                             gl_shader_stage stage,
                             const struct brw_base_prog_key *old_key,
                             const struct brw_base_prog_key *key);

/* Shared Local Memory Size is specified as powers of two,
 * and also have a Gen-dependent minimum value if not zero.
 */
static inline uint32_t
intel_calculate_slm_size(unsigned gen, uint32_t bytes)
{
   assert(bytes <= 64 * 1024);
   if (bytes > 0)
      return MAX2(util_next_power_of_two(bytes), gen >= 9 ? 1024 : 4096);
   else
      return 0;
}

static inline uint32_t
encode_slm_size(unsigned gen, uint32_t bytes)
{
   uint32_t slm_size = 0;

   /* Shared Local Memory is specified as powers of two, and encoded in
    * INTERFACE_DESCRIPTOR_DATA with the following representations:
    *
    * Size   | 0 kB | 1 kB | 2 kB | 4 kB | 8 kB | 16 kB | 32 kB | 64 kB |
    * -------------------------------------------------------------------
    * Gfx7-8 |    0 | none | none |    1 |    2 |     4 |     8 |    16 |
    * -------------------------------------------------------------------
    * Gfx9+  |    0 |    1 |    2 |    3 |    4 |     5 |     6 |     7 |
    */

   if (bytes > 0) {
      slm_size = intel_calculate_slm_size(gen, bytes);
      assert(util_is_power_of_two_nonzero(slm_size));

      if (gen >= 9) {
         /* Turn an exponent of 10 (1024 kB) into 1. */
         assert(slm_size >= 1024);
         slm_size = ffs(slm_size) - 10;
      } else {
         assert(slm_size >= 4096);
         /* Convert to the pre-Gfx9 representation. */
         slm_size = slm_size / 4096;
      }
   }

   return slm_size;
}

unsigned
brw_cs_push_const_total_size(const struct brw_cs_prog_data *cs_prog_data,
                             unsigned threads);

void
brw_write_shader_relocs(const struct brw_isa_info *isa,
                        void *program,
                        const struct brw_stage_prog_data *prog_data,
                        struct brw_shader_reloc_value *values,
                        unsigned num_values);

struct brw_cs_dispatch_info {
   uint32_t group_size;
   uint32_t simd_size;
   uint32_t threads;

   /* RightExecutionMask field used in GPGPU_WALKER. */
   uint32_t right_mask;
};

/**
 * Get the dispatch information for a shader to be used with GPGPU_WALKER and
 * similar instructions.
 *
 * If override_local_size is not NULL, it must to point to a 3-element that
 * will override the value from prog_data->local_size.  This is used by
 * ARB_compute_variable_group_size, where the size is set only at dispatch
 * time (so prog_data is outdated).
 */
struct brw_cs_dispatch_info
brw_cs_get_dispatch_info(const struct intel_device_info *devinfo,
                         const struct brw_cs_prog_data *prog_data,
                         const unsigned *override_local_size);

/**
 * Return true if the given shader stage is dispatched contiguously by the
 * relevant fixed function starting from channel 0 of the SIMD thread, which
 * implies that the dispatch mask of a thread can be assumed to have the form
 * '2^n - 1' for some n.
 */
static inline bool
brw_stage_has_packed_dispatch(ASSERTED const struct intel_device_info *devinfo,
                              gl_shader_stage stage, unsigned max_polygons,
                              const struct brw_stage_prog_data *prog_data)
{
   /* The code below makes assumptions about the hardware's thread dispatch
    * behavior that could be proven wrong in future generations -- Make sure
    * to do a full test run with brw_fs_test_dispatch_packing() hooked up to
    * the NIR front-end before changing this assertion.
    */
   assert(devinfo->ver <= 12);

   switch (stage) {
   case MESA_SHADER_FRAGMENT: {
      /* The PSD discards subspans coming in with no lit samples, which in the
       * per-pixel shading case implies that each subspan will either be fully
       * lit (due to the VMask being used to allow derivative computations),
       * or not dispatched at all.  In per-sample dispatch mode individual
       * samples from the same subspan have a fixed relative location within
       * the SIMD thread, so dispatch of unlit samples cannot be avoided in
       * general and we should return false.
       */
      const struct brw_wm_prog_data *wm_prog_data =
         (const struct brw_wm_prog_data *)prog_data;
      return devinfo->verx10 < 125 &&
             !wm_prog_data->persample_dispatch &&
             wm_prog_data->uses_vmask &&
             max_polygons < 2;
   }
   case MESA_SHADER_COMPUTE:
      /* Compute shaders will be spawned with either a fully enabled dispatch
       * mask or with whatever bottom/right execution mask was given to the
       * GPGPU walker command to be used along the workgroup edges -- In both
       * cases the dispatch mask is required to be tightly packed for our
       * invocation index calculations to work.
       */
      return true;
   default:
      /* Most remaining fixed functions are limited to use a packed dispatch
       * mask due to the hardware representation of the dispatch mask as a
       * single counter representing the number of enabled channels.
       */
      return true;
   }
}

/**
 * Computes the first varying slot in the URB produced by the previous stage
 * that is used in the next stage. We do this by testing the varying slots in
 * the previous stage's vue map against the inputs read in the next stage.
 *
 * Note that:
 *
 * - Each URB offset contains two varying slots and we can only skip a
 *   full offset if both slots are unused, so the value we return here is always
 *   rounded down to the closest multiple of two.
 *
 * - gl_Layer and gl_ViewportIndex don't have their own varying slots, they are
 *   part of the vue header, so if these are read we can't skip anything.
 */
static inline int
brw_compute_first_urb_slot_required(uint64_t inputs_read,
                                    const struct brw_vue_map *prev_stage_vue_map)
{
   if ((inputs_read & (VARYING_BIT_LAYER | VARYING_BIT_VIEWPORT | VARYING_BIT_PRIMITIVE_SHADING_RATE)) == 0) {
      for (int i = 0; i < prev_stage_vue_map->num_slots; i++) {
         int varying = prev_stage_vue_map->slot_to_varying[i];
         if (varying > 0 && (inputs_read & BITFIELD64_BIT(varying)) != 0)
            return ROUND_DOWN_TO(i, 2);
      }
   }

   return 0;
}

/* From InlineData in 3DSTATE_TASK_SHADER_DATA and 3DSTATE_MESH_SHADER_DATA. */
#define BRW_TASK_MESH_INLINE_DATA_SIZE_DW 8

/* InlineData[0-1] is used for Vulkan descriptor. */
#define BRW_TASK_MESH_PUSH_CONSTANTS_START_DW 2

#define BRW_TASK_MESH_PUSH_CONSTANTS_SIZE_DW \
   (BRW_TASK_MESH_INLINE_DATA_SIZE_DW - BRW_TASK_MESH_PUSH_CONSTANTS_START_DW)

/**
 * This enum is used as the base indice of the nir_load_topology_id_intel
 * intrinsic. This is used to return different values based on some aspect of
 * the topology of the device.
 */
enum brw_topology_id
{
   /* A value based of the DSS identifier the shader is currently running on.
    * Be mindful that the DSS ID can be higher than the total number of DSS on
    * the device. This is because of the fusing that can occur on different
    * parts.
    */
   BRW_TOPOLOGY_ID_DSS,

   /* A value composed of EU ID, thread ID & SIMD lane ID. */
   BRW_TOPOLOGY_ID_EU_THREAD_SIMD,
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BRW_COMPILER_H */
