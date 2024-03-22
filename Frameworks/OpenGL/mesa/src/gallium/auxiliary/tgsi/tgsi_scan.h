/**************************************************************************
 * 
 * Copyright 2008 VMware, Inc.
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

#ifndef TGSI_SCAN_H
#define TGSI_SCAN_H


#include "util/compiler.h"
#include "pipe/p_state.h"
#include "pipe/p_shader_tokens.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shader summary info
 */
struct tgsi_shader_info
{
   uint8_t num_inputs;
   uint8_t num_outputs;
   uint8_t input_semantic_name[PIPE_MAX_SHADER_INPUTS]; /**< TGSI_SEMANTIC_x */
   uint8_t input_semantic_index[PIPE_MAX_SHADER_INPUTS];
   uint8_t input_interpolate[PIPE_MAX_SHADER_INPUTS];
   uint8_t input_interpolate_loc[PIPE_MAX_SHADER_INPUTS];
   uint8_t input_usage_mask[PIPE_MAX_SHADER_INPUTS];
   uint8_t output_semantic_name[PIPE_MAX_SHADER_OUTPUTS]; /**< TGSI_SEMANTIC_x */
   uint8_t output_semantic_index[PIPE_MAX_SHADER_OUTPUTS];
   uint8_t output_usagemask[PIPE_MAX_SHADER_OUTPUTS];
   uint8_t output_streams[PIPE_MAX_SHADER_OUTPUTS];

   uint8_t num_system_values;
   uint8_t system_value_semantic_name[PIPE_MAX_SHADER_INPUTS];

   uint8_t processor;

   uint32_t file_mask[TGSI_FILE_COUNT];  /**< bitmask of declared registers */
   unsigned file_count[TGSI_FILE_COUNT];  /**< number of declared registers */
   int file_max[TGSI_FILE_COUNT];  /**< highest index of declared registers */
   int const_file_max[PIPE_MAX_CONSTANT_BUFFERS];
   unsigned const_buffers_declared; /**< bitmask of declared const buffers */
   unsigned samplers_declared; /**< bitmask of declared samplers */
   uint8_t sampler_targets[PIPE_MAX_SHADER_SAMPLER_VIEWS];  /**< TGSI_TEXTURE_x values */
   uint8_t sampler_type[PIPE_MAX_SHADER_SAMPLER_VIEWS]; /**< TGSI_RETURN_TYPE_x */
   uint8_t num_stream_output_components[4];

   uint8_t input_array_first[PIPE_MAX_SHADER_INPUTS];
   uint8_t output_array_first[PIPE_MAX_SHADER_OUTPUTS];

   unsigned immediate_count; /**< number of immediates declared */
   unsigned num_instructions;

   unsigned opcode_count[TGSI_OPCODE_LAST];  /**< opcode histogram */

   /**
    * If a tessellation control shader reads outputs, this describes which ones.
    */
   bool reads_pervertex_outputs;
   bool reads_perpatch_outputs;
   bool reads_tessfactor_outputs;

   bool reads_z; /**< does fragment shader read depth? */
   bool writes_z;  /**< does fragment shader write Z value? */
   bool writes_stencil; /**< does fragment shader write stencil value? */
   bool writes_samplemask; /**< does fragment shader write sample mask? */
   bool writes_edgeflag; /**< vertex shader outputs edgeflag */
   bool uses_kill;  /**< KILL or KILL_IF instruction used? */
   bool uses_instanceid;
   bool uses_vertexid;
   bool uses_vertexid_nobase;
   bool uses_basevertex;
   bool uses_primid;
   bool uses_frontface;
   bool uses_invocationid;
   bool uses_grid_size;
   bool writes_position;
   bool writes_psize;
   bool writes_clipvertex;
   bool writes_viewport_index;
   bool writes_layer;
   bool writes_memory; /**< contains stores or atomics to buffers or images */
   bool uses_fbfetch;
   unsigned num_written_culldistance;
   unsigned num_written_clipdistance;

   unsigned images_declared; /**< bitmask of declared images */
   unsigned msaa_images_declared; /**< bitmask of declared MSAA images */

   /**
    * Bitmask indicating which declared image is a buffer.
    */
   unsigned images_buffers;
   unsigned shader_buffers_declared; /**< bitmask of declared shader buffers */
   unsigned shader_buffers_load; /**< bitmask of shader buffers using loads */
   unsigned shader_buffers_store; /**< bitmask of shader buffers using stores */
   unsigned shader_buffers_atomic; /**< bitmask of shader buffers using atomics */

   unsigned hw_atomic_declared; /**< bitmask of declared atomic_counter */
   /**
    * Bitmask indicating which register files are accessed with
    * indirect addressing.  The bits are (1 << TGSI_FILE_x), etc.
    */
   unsigned indirect_files;
   /**
    * Bitmask indicating which register files are read / written with
    * indirect addressing.  The bits are (1 << TGSI_FILE_x).
    */
   unsigned dim_indirect_files; /**< shader resource indexing */

   unsigned properties[TGSI_PROPERTY_COUNT]; /* index with TGSI_PROPERTY_ */
};

extern void
tgsi_scan_shader(const struct tgsi_token *tokens,
                 struct tgsi_shader_info *info);

static inline bool
tgsi_is_bindless_image_file(enum tgsi_file_type file)
{
   return file != TGSI_FILE_IMAGE &&
          file != TGSI_FILE_MEMORY &&
          file != TGSI_FILE_BUFFER &&
          file != TGSI_FILE_CONSTBUF &&
          file != TGSI_FILE_HW_ATOMIC;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* TGSI_SCAN_H */
