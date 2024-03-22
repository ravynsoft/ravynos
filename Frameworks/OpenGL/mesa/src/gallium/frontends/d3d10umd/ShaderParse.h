/**************************************************************************
 *
 * Copyright 2012-2021 VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/

/*
 * ShaderParse.h --
 *    Functions for parsing shader tokens.
 */

#ifndef SHADER_PARSE_H
#define SHADER_PARSE_H

#include "DriverIncludes.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Shader_header {
   D3D10_SB_TOKENIZED_PROGRAM_TYPE type;
   unsigned major_version;
   unsigned minor_version;
   unsigned size;
};

struct dx10_imm_const_buf {
   unsigned count;
   unsigned *data;
};

struct dx10_customdata {
   D3D10_SB_CUSTOMDATA_CLASS _class;
   union {
      struct dx10_imm_const_buf constbuf;
   } u;
};

struct dx10_indexable_temp {
   unsigned index;
   unsigned count;
   unsigned components;
};

struct dx10_global_flags {
   unsigned refactoring_allowed:1;
};

struct Shader_relative_index {
   unsigned imm;
};

struct Shader_relative_operand {
   D3D10_SB_OPERAND_TYPE type;
   struct Shader_relative_index index[2];
   D3D10_SB_4_COMPONENT_NAME comp;
};

struct Shader_index {
   unsigned imm;
   struct Shader_relative_operand rel;
   D3D10_SB_OPERAND_INDEX_REPRESENTATION index_rep;
};

struct Shader_operand {
   D3D10_SB_OPERAND_TYPE type;
   struct Shader_index index[2];
   unsigned index_dim;
};

struct Shader_dst_operand {
   struct Shader_operand base;
   unsigned mask;
};

union Shader_immediate {
   float f32;
   int i32;
   unsigned u32;
};

struct Shader_src_operand {
   struct Shader_operand base;
   union Shader_immediate imm[4];
   D3D10_SB_4_COMPONENT_NAME swizzle[4];
   D3D10_SB_OPERAND_MODIFIER modifier;
};

#define SHADER_MAX_DST_OPERANDS 2
#define SHADER_MAX_SRC_OPERANDS 5

struct Shader_opcode {
   D3D10_SB_OPCODE_TYPE type;
   unsigned num_dst;
   unsigned num_src;
   struct Shader_dst_operand dst[SHADER_MAX_DST_OPERANDS];
   struct Shader_src_operand src[SHADER_MAX_SRC_OPERANDS];

   /* Opcode specific data.
    */
   union {
      D3D10_SB_RESOURCE_DIMENSION dcl_resource_dimension;
      D3D10_SB_SAMPLER_MODE dcl_sampler_mode;
      D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN dcl_cb_access_pattern;
      D3D10_SB_INTERPOLATION_MODE dcl_in_ps_interp;
      D3D10_SB_PRIMITIVE_TOPOLOGY dcl_gs_output_primitive_topology;
      D3D10_SB_PRIMITIVE dcl_gs_input_primitive;
      D3D10_SB_INSTRUCTION_TEST_BOOLEAN test_boolean;
      D3D10_SB_RESINFO_INSTRUCTION_RETURN_TYPE resinfo_ret_type;
      unsigned dcl_max_output_vertex_count;
      unsigned dcl_num_temps;
      struct dx10_indexable_temp dcl_indexable_temp;
      unsigned index_range_count;
      struct dx10_global_flags global_flags;
   } specific;
   D3D10_SB_NAME dcl_siv_name;
   D3D10_SB_RESOURCE_RETURN_TYPE dcl_resource_ret_type[4];

   bool saturate;

   struct {
      int u:4;
      int v:4;
      int w:4;
   } imm_texel_offset;

   struct dx10_customdata customdata;
};

struct Shader_parser {
   const unsigned *code;
   const unsigned *curr;

   struct Shader_header header;
};

void
Shader_parse_init(struct Shader_parser *parser,
                       const unsigned *code);

bool
Shader_parse_opcode(struct Shader_parser *parser,
                         struct Shader_opcode *opcode);

void
Shader_opcode_free(struct Shader_opcode *opcode);


const struct tgsi_token *
Shader_tgsi_translate(const unsigned *code,
                      unsigned *output_mapping);


#ifdef __cplusplus
}
#endif

#endif /* SHADER_PARSE_H */
