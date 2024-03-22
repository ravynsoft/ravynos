/*
 * Copyright (c) 2012-2015 Etnaviv Project
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
 */

#ifndef H_ETNA_INTERNAL
#define H_ETNA_INTERNAL

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "hw/common.xml.h"
#include "hw/common_3d.xml.h"
#include "hw/state.xml.h"
#include "hw/state_3d.xml.h"

#include "drm/etnaviv_drmif.h"

#define ETNA_NUM_INPUTS (16)
#define ETNA_NUM_VARYINGS 16
#define ETNA_NUM_LOD (14)
#define ETNA_NUM_LAYERS (6)
#define ETNA_MAX_UNIFORMS (256)
#define ETNA_MAX_CONST_BUF 16
#define ETNA_MAX_PIXELPIPES 2

/* All RS operations must have width%16 = 0 */
#define ETNA_RS_WIDTH_MASK (16 - 1)
/* RS tiled operations must have height%4 = 0 */
#define ETNA_RS_HEIGHT_MASK (3)
/* PE render targets must be aligned to 64 bytes */
#define ETNA_PE_ALIGNMENT (64)

/* These demarcate the margin (fixp16) between the computed sizes and the
  value sent to the chip. These have been set to the numbers used by the
  Vivante driver on gc2000. They used to be -1 for scissor right and bottom. I
  am not sure whether older hardware was relying on these or they were just a
  guess. But if so, these need to be moved to the _specs structure.
*/
#define ETNA_SE_SCISSOR_MARGIN_RIGHT (0x1119)
#define ETNA_SE_SCISSOR_MARGIN_BOTTOM (0x1111)
#define ETNA_SE_CLIP_MARGIN_RIGHT (0xffff)
#define ETNA_SE_CLIP_MARGIN_BOTTOM (0xffff)

/* GPU chip 3D specs */
struct etna_specs {
   /* HALTI (gross architecture) level. -1 for pre-HALTI. */
   int halti : 8;
   /* supports SUPERTILE (64x64) tiling? */
   unsigned can_supertile : 1;
   /* needs z=(z+w)/2, for older GCxxx */
   unsigned vs_need_z_div : 1;
   /* supports trigonometric instructions */
   unsigned has_sin_cos_sqrt : 1;
   /* has SIGN/FLOOR/CEIL instructions */
   unsigned has_sign_floor_ceil : 1;
   /* can use VS_RANGE, PS_RANGE registers*/
   unsigned has_shader_range_registers : 1;
   /* has the new sin/cos/log functions */
   unsigned has_new_transcendentals : 1;
   /* has the new dp2/dpX_norm instructions, among others */
   unsigned has_halti2_instructions : 1;
   /* has no limit on the number of constant sources per instruction */
   unsigned has_no_oneconst_limit : 1;
   /* has V4_COMPRESSION */
   unsigned v4_compression : 1;
   /* supports single-buffer rendering with multiple pixel pipes */
   unsigned single_buffer : 1;
   /* has unified uniforms memory */
   unsigned has_unified_uniforms : 1;
   /* can load shader instructions from memory */
   unsigned has_icache : 1;
   /* ASTC texture support (and has associated states) */
   unsigned tex_astc : 1;
   /* has BLT engine instead of RS */
   unsigned use_blt : 1;
   /* can use any kind of wrapping mode on npot textures */
   unsigned npot_tex_any_wrap : 1;
   /* supports seamless cube map */
   unsigned seamless_cube_map : 1;
   /* number of bits per TS tile */
   unsigned bits_per_tile;
   /* clear value for TS (dependent on bits_per_tile) */
   uint32_t ts_clear_value;
   /* base of vertex texture units */
   unsigned vertex_sampler_offset;
   /* number of fragment sampler units */
   unsigned fragment_sampler_count;
   /* number of vertex sampler units */
   unsigned vertex_sampler_count;
   /* size of vertex shader output buffer */
   unsigned vertex_output_buffer_size;
   /* maximum number of vertex element configurations */
   unsigned vertex_max_elements;
   /* size of a cached vertex (?) */
   unsigned vertex_cache_size;
   /* number of shader cores */
   unsigned shader_core_count;
   /* number of vertex streams */
   unsigned stream_count;
   /* vertex shader memory address*/
   uint32_t vs_offset;
   /* pixel shader memory address*/
   uint32_t ps_offset;
   /* vertex shader uniforms address*/
   uint32_t vs_uniforms_offset;
   /* pixel shader uniforms address*/
   uint32_t ps_uniforms_offset;
   /* vertex/fragment shader max instructions */
   uint32_t max_instructions;
   /* maximum number of varyings */
   unsigned max_varyings;
   /* maximum number of registers */
   unsigned max_registers;
   /* maximum vertex uniforms */
   unsigned max_vs_uniforms;
   /* maximum pixel uniforms */
   unsigned max_ps_uniforms;
   /* maximum texture size */
   unsigned max_texture_size;
   /* maximum texture size */
   unsigned max_rendertarget_size;
   /* available pixel pipes */
   unsigned pixel_pipes;
   /* number of constants */
   unsigned num_constants;
};

/* Compiled Gallium state. All the different compiled state atoms are woven
 * together and uploaded only when it is necessary to synchronize the state,
 * for example before rendering. */

/* Compiled pipe_blend_color */
struct compiled_blend_color {
   float color[4];
   uint32_t PE_ALPHA_BLEND_COLOR;
   uint32_t PE_ALPHA_COLOR_EXT0;
   uint32_t PE_ALPHA_COLOR_EXT1;
};

/* Compiled pipe_stencil_ref */
struct compiled_stencil_ref {
   uint32_t PE_STENCIL_CONFIG[2];
   uint32_t PE_STENCIL_CONFIG_EXT[2];
};

/* Compiled pipe_viewport_state */
struct compiled_viewport_state {
   uint32_t PA_VIEWPORT_SCALE_X;
   uint32_t PA_VIEWPORT_SCALE_Y;
   uint32_t PA_VIEWPORT_SCALE_Z;
   uint32_t PA_VIEWPORT_OFFSET_X;
   uint32_t PA_VIEWPORT_OFFSET_Y;
   uint32_t PA_VIEWPORT_OFFSET_Z;
   uint32_t SE_SCISSOR_LEFT;
   uint32_t SE_SCISSOR_TOP;
   uint32_t SE_SCISSOR_RIGHT;
   uint32_t SE_SCISSOR_BOTTOM;
   uint32_t PE_DEPTH_NEAR;
   uint32_t PE_DEPTH_FAR;
};

/* Compiled pipe_framebuffer_state */
struct compiled_framebuffer_state {
   uint32_t GL_MULTI_SAMPLE_CONFIG;
   uint32_t PE_COLOR_FORMAT;
   uint32_t PE_DEPTH_CONFIG;
   struct etna_reloc PE_DEPTH_ADDR;
   struct etna_reloc PE_PIPE_DEPTH_ADDR[ETNA_MAX_PIXELPIPES];
   uint32_t PE_DEPTH_STRIDE;
   uint32_t PE_HDEPTH_CONTROL;
   uint32_t PE_DEPTH_NORMALIZE;
   struct etna_reloc PE_COLOR_ADDR;
   struct etna_reloc PE_PIPE_COLOR_ADDR[ETNA_MAX_PIXELPIPES];
   uint32_t PE_COLOR_STRIDE;
   uint32_t PE_MEM_CONFIG;
   uint32_t RA_MULTISAMPLE_UNK00E04;
   uint32_t RA_MULTISAMPLE_UNK00E10[VIVS_RA_MULTISAMPLE_UNK00E10__LEN];
   uint32_t RA_CENTROID_TABLE[VIVS_RA_CENTROID_TABLE__LEN];
   uint32_t TS_MEM_CONFIG;
   uint32_t TS_DEPTH_CLEAR_VALUE;
   struct etna_reloc TS_DEPTH_STATUS_BASE;
   struct etna_reloc TS_DEPTH_SURFACE_BASE;
   uint32_t TS_COLOR_CLEAR_VALUE;
   uint32_t TS_COLOR_CLEAR_VALUE_EXT;
   struct etna_reloc TS_COLOR_STATUS_BASE;
   struct etna_reloc TS_COLOR_SURFACE_BASE;
   uint32_t PE_LOGIC_OP;
   uint32_t PS_CONTROL;
   uint32_t PS_CONTROL_EXT;
   bool msaa_mode; /* adds input (and possible temp) to PS */
};

/* Compiled context->create_vertex_elements_state */
struct compiled_vertex_elements_state {
   unsigned num_elements;
   uint32_t FE_VERTEX_ELEMENT_CONFIG[VIVS_FE_VERTEX_ELEMENT_CONFIG__LEN];
   uint32_t NFE_GENERIC_ATTRIB_CONFIG0[VIVS_NFE_GENERIC_ATTRIB__LEN];
   uint32_t NFE_GENERIC_ATTRIB_SCALE[VIVS_NFE_GENERIC_ATTRIB__LEN];
   uint32_t NFE_GENERIC_ATTRIB_CONFIG1[VIVS_NFE_GENERIC_ATTRIB__LEN];
   unsigned num_buffers;
   uint32_t NFE_VERTEX_STREAMS_VERTEX_DIVISOR[VIVS_NFE_VERTEX_STREAMS__LEN];
   uint32_t FE_VERTEX_STREAM_CONTROL[VIVS_NFE_VERTEX_STREAMS__LEN];
};

/* Compiled context->set_vertex_buffer result */
struct compiled_set_vertex_buffer {
   struct etna_reloc FE_VERTEX_STREAM_BASE_ADDR;
};

/* Compiled linked VS+PS shader state */
struct compiled_shader_state {
   uint32_t RA_CONTROL;
   uint32_t PA_ATTRIBUTE_ELEMENT_COUNT;
   uint32_t PA_CONFIG;
   uint32_t PA_SHADER_ATTRIBUTES[VIVS_PA_SHADER_ATTRIBUTES__LEN];
   uint32_t VS_END_PC;
   uint32_t VS_OUTPUT_COUNT; /* number of outputs if point size per vertex disabled */
   uint32_t VS_OUTPUT_COUNT_PSIZE; /* number of outputs of point size per vertex enabled */
   uint32_t VS_INPUT_COUNT;
   uint32_t VS_TEMP_REGISTER_CONTROL;
   uint32_t VS_OUTPUT[4];
   uint32_t VS_INPUT[4];
   uint32_t VS_LOAD_BALANCING;
   uint32_t VS_START_PC;
   uint32_t PS_END_PC;
   uint32_t PS_OUTPUT_REG;
   uint32_t PS_INPUT_COUNT;
   uint32_t PS_INPUT_COUNT_MSAA; /* Adds an input */
   uint32_t PS_TEMP_REGISTER_CONTROL;
   uint32_t PS_TEMP_REGISTER_CONTROL_MSAA; /* Adds a temporary if needed to make space for extra input */
   uint32_t PS_START_PC;
   uint32_t GL_VARYING_TOTAL_COMPONENTS;
   uint32_t GL_VARYING_NUM_COMPONENTS[2];
   uint32_t GL_VARYING_COMPONENT_USE[2];
   uint32_t GL_HALTI5_SH_SPECIALS;
   uint32_t FE_HALTI5_ID_CONFIG;
   unsigned vs_inst_mem_size;
   unsigned ps_inst_mem_size;
   uint32_t *VS_INST_MEM;
   uint32_t *PS_INST_MEM;
   struct etna_reloc PS_INST_ADDR;
   struct etna_reloc VS_INST_ADDR;
   unsigned writes_z:1;
   unsigned uses_discard:1;
};

/* Helpers to assist creating and setting bitarrays (eg, for varyings).
 * field_size must be a power of two, and <= 32. */
#define DEFINE_ETNA_BITARRAY(name, num, field_size) \
   uint32_t name[(num) * (field_size) / 32]

static inline void
etna_bitarray_set(uint32_t *array, size_t array_size, size_t field_size,
                  size_t index, uint32_t value)
{
   size_t shift = (index * field_size) % 32;
   size_t offset = (index * field_size) / 32;

   assert(index < array_size * 32 / field_size);
   assert(value < 1 << field_size);

   array[offset] |= value << shift;
}

#define etna_bitarray_set(array, field_size, index, value) \
   etna_bitarray_set((array), ARRAY_SIZE(array), field_size, index, value)

#endif
