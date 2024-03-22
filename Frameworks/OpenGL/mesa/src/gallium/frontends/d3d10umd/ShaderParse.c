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
 * ShaderParse.c --
 *    Functions for parsing shader tokens.
 */

#include "Debug.h"
#include "ShaderParse.h"

#include "util/u_memory.h"


void
Shader_parse_init(struct Shader_parser *parser,
                       const unsigned *code)
{
   parser->curr = parser->code = code;

   parser->header.type = DECODE_D3D10_SB_TOKENIZED_PROGRAM_TYPE(*parser->curr);
   parser->header.major_version = DECODE_D3D10_SB_TOKENIZED_PROGRAM_MAJOR_VERSION(*parser->curr);
   parser->header.minor_version = DECODE_D3D10_SB_TOKENIZED_PROGRAM_MINOR_VERSION(*parser->curr);
   parser->curr++;

   parser->header.size = DECODE_D3D10_SB_TOKENIZED_PROGRAM_LENGTH(*parser->curr);
   parser->curr++;
}

#define OP_NOT_DONE (1 << 0) /* not implemented yet */
#define OP_SATURATE (1 << 1) /* saturate in opcode specific control */
#define OP_TEST_BOOLEAN (1 << 2) /* test boolean in opcode specific control */
#define OP_DCL (1 << 3) /* custom opcode specific control */
#define OP_RESINFO_RET_TYPE (1 << 4) /* return type for resinfo */

struct dx10_opcode_info {
   D3D10_SB_OPCODE_TYPE type;
   const char *name;
   unsigned num_dst;
   unsigned num_src;
   unsigned flags;
};

#define _(_opcode) _opcode, #_opcode

static const struct dx10_opcode_info
opcode_info[D3D10_SB_NUM_OPCODES] = {
   {_(D3D10_SB_OPCODE_ADD),                              1, 2, OP_SATURATE},
   {_(D3D10_SB_OPCODE_AND),                              1, 2, 0},
   {_(D3D10_SB_OPCODE_BREAK),                            0, 0, 0},
   {_(D3D10_SB_OPCODE_BREAKC),                           0, 1, OP_TEST_BOOLEAN},
   {_(D3D10_SB_OPCODE_CALL),                             0, 1, 0},
   {_(D3D10_SB_OPCODE_CALLC),                            0, 2, OP_TEST_BOOLEAN},
   {_(D3D10_SB_OPCODE_CASE),                             0, 1, 0},
   {_(D3D10_SB_OPCODE_CONTINUE),                         0, 0, 0},
   {_(D3D10_SB_OPCODE_CONTINUEC),                        0, 1, OP_TEST_BOOLEAN},
   {_(D3D10_SB_OPCODE_CUT),                              0, 0, 0},
   {_(D3D10_SB_OPCODE_DEFAULT),                          0, 0, 0},
   {_(D3D10_SB_OPCODE_DERIV_RTX),                        1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_DERIV_RTY),                        1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_DISCARD),                          0, 1, OP_TEST_BOOLEAN},
   {_(D3D10_SB_OPCODE_DIV),                              1, 2, OP_SATURATE},
   {_(D3D10_SB_OPCODE_DP2),                              1, 2, OP_SATURATE},
   {_(D3D10_SB_OPCODE_DP3),                              1, 2, OP_SATURATE},
   {_(D3D10_SB_OPCODE_DP4),                              1, 2, OP_SATURATE},
   {_(D3D10_SB_OPCODE_ELSE),                             0, 0, 0},
   {_(D3D10_SB_OPCODE_EMIT),                             0, 0, 0},
   {_(D3D10_SB_OPCODE_EMITTHENCUT),                      0, 0, 0},
   {_(D3D10_SB_OPCODE_ENDIF),                            0, 0, 0},
   {_(D3D10_SB_OPCODE_ENDLOOP),                          0, 0, 0},
   {_(D3D10_SB_OPCODE_ENDSWITCH),                        0, 0, 0},
   {_(D3D10_SB_OPCODE_EQ),                               1, 2, 0},
   {_(D3D10_SB_OPCODE_EXP),                              1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_FRC),                              1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_FTOI),                             1, 1, 0},
   {_(D3D10_SB_OPCODE_FTOU),                             1, 1, 0},
   {_(D3D10_SB_OPCODE_GE),                               1, 2, 0},
   {_(D3D10_SB_OPCODE_IADD),                             1, 2, 0},
   {_(D3D10_SB_OPCODE_IF),                               0, 1, OP_TEST_BOOLEAN},
   {_(D3D10_SB_OPCODE_IEQ),                              1, 2, 0},
   {_(D3D10_SB_OPCODE_IGE),                              1, 2, 0},
   {_(D3D10_SB_OPCODE_ILT),                              1, 2, 0},
   {_(D3D10_SB_OPCODE_IMAD),                             1, 3, 0},
   {_(D3D10_SB_OPCODE_IMAX),                             1, 2, 0},
   {_(D3D10_SB_OPCODE_IMIN),                             1, 2, 0},
   {_(D3D10_SB_OPCODE_IMUL),                             2, 2, 0},
   {_(D3D10_SB_OPCODE_INE),                              1, 2, 0},
   {_(D3D10_SB_OPCODE_INEG),                             1, 1, 0},
   {_(D3D10_SB_OPCODE_ISHL),                             1, 2, 0},
   {_(D3D10_SB_OPCODE_ISHR),                             1, 2, 0},
   {_(D3D10_SB_OPCODE_ITOF),                             1, 1, 0},
   {_(D3D10_SB_OPCODE_LABEL),                            0, 1, 0},
   {_(D3D10_SB_OPCODE_LD),                               1, 2, 0},
   {_(D3D10_SB_OPCODE_LD_MS),                            1, 3, 0},
   {_(D3D10_SB_OPCODE_LOG),                              1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_LOOP),                             0, 0, 0},
   {_(D3D10_SB_OPCODE_LT),                               1, 2, 0},
   {_(D3D10_SB_OPCODE_MAD),                              1, 3, OP_SATURATE},
   {_(D3D10_SB_OPCODE_MIN),                              1, 2, OP_SATURATE},
   {_(D3D10_SB_OPCODE_MAX),                              1, 2, OP_SATURATE},
   {_(D3D10_SB_OPCODE_CUSTOMDATA),                       0, 0, 0},
   {_(D3D10_SB_OPCODE_MOV),                              1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_MOVC),                             1, 3, OP_SATURATE},
   {_(D3D10_SB_OPCODE_MUL),                              1, 2, OP_SATURATE},
   {_(D3D10_SB_OPCODE_NE),                               1, 2, 0},
   {_(D3D10_SB_OPCODE_NOP),                              0, 0, 0},
   {_(D3D10_SB_OPCODE_NOT),                              1, 1, 0},
   {_(D3D10_SB_OPCODE_OR),                               1, 2, 0},
   {_(D3D10_SB_OPCODE_RESINFO),                          1, 2, OP_RESINFO_RET_TYPE},
   {_(D3D10_SB_OPCODE_RET),                              0, 0, 0},
   {_(D3D10_SB_OPCODE_RETC),                             0, 1, OP_TEST_BOOLEAN},
   {_(D3D10_SB_OPCODE_ROUND_NE),                         1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_ROUND_NI),                         1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_ROUND_PI),                         1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_ROUND_Z),                          1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_RSQ),                              1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_SAMPLE),                           1, 3, 0},
   {_(D3D10_SB_OPCODE_SAMPLE_C),                         1, 4, 0},
   {_(D3D10_SB_OPCODE_SAMPLE_C_LZ),                      1, 4, 0},
   {_(D3D10_SB_OPCODE_SAMPLE_L),                         1, 4, 0},
   {_(D3D10_SB_OPCODE_SAMPLE_D),                         1, 5, 0},
   {_(D3D10_SB_OPCODE_SAMPLE_B),                         1, 4, 0},
   {_(D3D10_SB_OPCODE_SQRT),                             1, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_SWITCH),                           0, 1, 0},
   {_(D3D10_SB_OPCODE_SINCOS),                           2, 1, OP_SATURATE},
   {_(D3D10_SB_OPCODE_UDIV),                             2, 2, 0},
   {_(D3D10_SB_OPCODE_ULT),                              1, 2, 0},
   {_(D3D10_SB_OPCODE_UGE),                              1, 2, 0},
   {_(D3D10_SB_OPCODE_UMUL),                             2, 2, 0},
   {_(D3D10_SB_OPCODE_UMAD),                             1, 3, 0},
   {_(D3D10_SB_OPCODE_UMAX),                             1, 2, 0},
   {_(D3D10_SB_OPCODE_UMIN),                             1, 2, 0},
   {_(D3D10_SB_OPCODE_USHR),                             1, 2, 0},
   {_(D3D10_SB_OPCODE_UTOF),                             1, 1, 0},
   {_(D3D10_SB_OPCODE_XOR),                              1, 2, 0},
   {_(D3D10_SB_OPCODE_DCL_RESOURCE),                     1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER),              0, 1, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_SAMPLER),                      1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_INDEX_RANGE),                  1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY), 0, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE),           0, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT),      0, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_INPUT),                        1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_INPUT_SGV),                    1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_INPUT_SIV),                    1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_INPUT_PS),                     1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_INPUT_PS_SGV),                 1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_INPUT_PS_SIV),                 1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_OUTPUT),                       1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_OUTPUT_SGV),                   1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_OUTPUT_SIV),                   1, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_TEMPS),                        0, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP),               0, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS),                 0, 0, OP_DCL},
   {_(D3D10_SB_OPCODE_RESERVED0),                        0, 0, OP_NOT_DONE},
   {_(D3D10_1_SB_OPCODE_LOD),                            0, 0, OP_NOT_DONE},
   {_(D3D10_1_SB_OPCODE_GATHER4),                        0, 0, OP_NOT_DONE},
   {_(D3D10_1_SB_OPCODE_SAMPLE_POS),                     0, 0, OP_NOT_DONE},
   {_(D3D10_1_SB_OPCODE_SAMPLE_INFO),                    0, 0, OP_NOT_DONE}
};

#undef _

static void
parse_operand(const unsigned **curr,
              struct Shader_operand *operand)
{
   operand->type = DECODE_D3D10_SB_OPERAND_TYPE(**curr);

   /* Index dimension. */
   switch (DECODE_D3D10_SB_OPERAND_INDEX_DIMENSION(**curr)) {
   case D3D10_SB_OPERAND_INDEX_0D:
      operand->index_dim = 0;
      break;
   case D3D10_SB_OPERAND_INDEX_1D:
      operand->index_dim = 1;
      break;
   case D3D10_SB_OPERAND_INDEX_2D:
      operand->index_dim = 2;
      break;
   default:
      assert(0);
   }

   if (operand->index_dim >= 1) {
      operand->index[0].index_rep = DECODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(0, **curr);
      if (operand->index_dim >= 2) {
         operand->index[1].index_rep = DECODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(1, **curr);
      }
   }

   (*curr)++;
}

static void
parse_relative_operand(const unsigned **curr,
                       struct Shader_relative_operand *operand)
{
   assert(!DECODE_IS_D3D10_SB_OPERAND_EXTENDED(**curr));
   assert(DECODE_D3D10_SB_OPERAND_NUM_COMPONENTS(**curr) == D3D10_SB_OPERAND_4_COMPONENT);
   assert(DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(**curr) == D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE);

   operand->comp = DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(**curr);

   operand->type = DECODE_D3D10_SB_OPERAND_TYPE(**curr);
   assert(operand->type != D3D10_SB_OPERAND_TYPE_IMMEDIATE32);

   /* Index dimension. */
   assert(DECODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(0, **curr) == D3D10_SB_OPERAND_INDEX_IMMEDIATE32);

   if (DECODE_D3D10_SB_OPERAND_INDEX_DIMENSION(**curr) == D3D10_SB_OPERAND_INDEX_1D) {
      (*curr)++;
      operand->index[0].imm = **curr;
   } else {
      assert(DECODE_D3D10_SB_OPERAND_INDEX_DIMENSION(**curr) == D3D10_SB_OPERAND_INDEX_2D);
      (*curr)++;
      operand->index[0].imm = **curr;
      (*curr)++;
      operand->index[1].imm = **curr;

   }
   (*curr)++;
}

static void
parse_index(const unsigned **curr,
            struct Shader_index *index)
{
   switch (index->index_rep) {
   case D3D10_SB_OPERAND_INDEX_IMMEDIATE32:
      index->imm = *(*curr)++;
      break;
   case D3D10_SB_OPERAND_INDEX_RELATIVE:
      index->imm = 0;
      parse_relative_operand(curr, &index->rel);
      break;
   case D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE:
      index->imm = *(*curr)++;
      parse_relative_operand(curr, &index->rel);
      break;
   default:
      /* XXX: Support other index representations.
       */
      assert(0);
   }
}

static void
parse_operand_index(const unsigned **curr,
                    struct Shader_operand *operand)
{
   if (operand->index_dim >= 1) {
      parse_index(curr, &operand->index[0]);
      if (operand->index_dim >= 2) {
         parse_index(curr, &operand->index[1]);
      }
   }
}

bool
Shader_parse_opcode(struct Shader_parser *parser,
                         struct Shader_opcode *opcode)
{
   const unsigned *curr = parser->curr;
   const struct dx10_opcode_info *info;
   unsigned length;
   bool opcode_is_extended;
   unsigned i;

   if (curr >= parser->code + parser->header.size) {
      return false;
   }

   memset(opcode, 0, sizeof *opcode);

   /* Opcode type. */
   opcode->type = DECODE_D3D10_SB_OPCODE_TYPE(*curr);

   if (opcode->type == D3D10_SB_OPCODE_CUSTOMDATA) {
      opcode->customdata._class = DECODE_D3D10_SB_CUSTOMDATA_CLASS(*curr);
      curr++;

      assert(opcode->customdata._class == D3D10_SB_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER);

      opcode->customdata.u.constbuf.count = *curr - 2;
      curr++;

      opcode->customdata.u.constbuf.data = MALLOC(opcode->customdata.u.constbuf.count * sizeof(unsigned));
      assert(opcode->customdata.u.constbuf.data);

      memcpy(opcode->customdata.u.constbuf.data,
             curr,
             opcode->customdata.u.constbuf.count * sizeof(unsigned));
      curr += opcode->customdata.u.constbuf.count;

      parser->curr = curr;
      return true;
   }

   opcode->dcl_siv_name = D3D10_SB_NAME_UNDEFINED;

   /* Lookup extra information based on opcode type. */
   assert(opcode->type < D3D10_SB_NUM_OPCODES);
   info = &opcode_info[opcode->type];

   /* Opcode specific. */
   switch (opcode->type) {
   case D3D10_SB_OPCODE_DCL_RESOURCE:
      opcode->specific.dcl_resource_dimension = DECODE_D3D10_SB_RESOURCE_DIMENSION(*curr);
      break;
   case D3D10_SB_OPCODE_DCL_SAMPLER:
      opcode->specific.dcl_sampler_mode = DECODE_D3D10_SB_SAMPLER_MODE(*curr);
      break;
   case D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY:
      opcode->specific.dcl_gs_output_primitive_topology = DECODE_D3D10_SB_GS_OUTPUT_PRIMITIVE_TOPOLOGY(*curr);
      break;
   case D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE:
      opcode->specific.dcl_gs_input_primitive = DECODE_D3D10_SB_GS_INPUT_PRIMITIVE(*curr);
      break;
   case D3D10_SB_OPCODE_DCL_INPUT_PS:
   case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
      opcode->specific.dcl_in_ps_interp = DECODE_D3D10_SB_INPUT_INTERPOLATION_MODE(*curr);
      break;
   case D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS:
      opcode->specific.global_flags.refactoring_allowed = DECODE_D3D10_SB_GLOBAL_FLAGS(*curr) ? 1 : 0;
      break;
   default:
      /* Parse opcode-specific control bits */
      if (info->flags & OP_DCL) {
         /* no-op */
      } else if (info->flags & OP_SATURATE) {
         opcode->saturate =
            !!DECODE_IS_D3D10_SB_INSTRUCTION_SATURATE_ENABLED(*curr);
      } else if (info->flags & OP_TEST_BOOLEAN) {
         opcode->specific.test_boolean =
            DECODE_D3D10_SB_INSTRUCTION_TEST_BOOLEAN(*curr);
      } else if (info->flags & OP_RESINFO_RET_TYPE) {
         opcode->specific.resinfo_ret_type =
            DECODE_D3D10_SB_RESINFO_INSTRUCTION_RETURN_TYPE(*curr);
      } else {
         /* Warn if there are bits set in the opcode-specific controls (bits 23:11 inclusive)*/
         if (*curr & ((1 << 24) - (1 << 11))) {
            debug_printf("warning: unexpected opcode-specific control in opcode %s\n",
                         info->name);
         }
      }
      break;
   }

   /* Opcode length in DWORDs. */
   length = DECODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(*curr);
   assert(curr + length <= parser->code + parser->header.size);

   /* Opcode specific fields in token0. */
   switch (opcode->type) {
   case D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER:
      opcode->specific.dcl_cb_access_pattern =
         DECODE_D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN(*curr);
      break;
   default:
      break;
   }

   opcode_is_extended = DECODE_IS_D3D10_SB_OPCODE_EXTENDED(*curr);

   curr++;

   if (opcode_is_extended) {
      /* NOTE: DECODE_IS_D3D10_SB_OPCODE_DOUBLE_EXTENDED is broken.
       */
      assert(!((*curr & D3D10_SB_OPERAND_DOUBLE_EXTENDED_MASK) >> D3D10_SB_OPERAND_DOUBLE_EXTENDED_SHIFT));

      switch (DECODE_D3D10_SB_EXTENDED_OPCODE_TYPE(*curr)) {
      case D3D10_SB_EXTENDED_OPCODE_EMPTY:
         break;
      case D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS:
         opcode->imm_texel_offset.u = DECODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_U, *curr);
         opcode->imm_texel_offset.v = DECODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_V, *curr);
         opcode->imm_texel_offset.w = DECODE_IMMEDIATE_D3D10_SB_ADDRESS_OFFSET(D3D10_SB_IMMEDIATE_ADDRESS_OFFSET_W, *curr);
         break;
      default:
         assert(0);
      }

      curr++;
   }

   if (info->flags & OP_NOT_DONE) {
      /* XXX: Need to figure out the number of operands for this opcode.
       *      Should be okay to continue execution -- we have enough info
       *      to skip to the next instruction.
       */
      LOG_UNSUPPORTED(true);
      opcode->num_dst = 0;
      opcode->num_src = 0;
      goto skip;
   }

   opcode->num_dst = info->num_dst;
   opcode->num_src = info->num_src;

   /* Destination operands. */
   for (i = 0; i < info->num_dst; i++) {
      D3D10_SB_OPERAND_NUM_COMPONENTS num_components;

      assert(!DECODE_IS_D3D10_SB_OPERAND_EXTENDED(*curr));

      num_components = DECODE_D3D10_SB_OPERAND_NUM_COMPONENTS(*curr);
      if (num_components == D3D10_SB_OPERAND_4_COMPONENT) {
         D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE selection_mode;

         selection_mode = DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(*curr);
         assert(selection_mode == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);

         opcode->dst[i].mask = DECODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(*curr);
      } else {
         assert(num_components == D3D10_SB_OPERAND_0_COMPONENT ||
                num_components == D3D10_SB_OPERAND_1_COMPONENT);

         opcode->dst[i].mask = D3D10_SB_OPERAND_4_COMPONENT_MASK_X;
      }

      parse_operand(&curr, &opcode->dst[i].base);
      parse_operand_index(&curr, &opcode->dst[i].base);
   }

   /* Source operands. */
   for (i = 0; i < info->num_src; i++) {
      bool extended;
      D3D10_SB_OPERAND_NUM_COMPONENTS num_components;

      extended = DECODE_IS_D3D10_SB_OPERAND_EXTENDED(*curr);

      num_components = DECODE_D3D10_SB_OPERAND_NUM_COMPONENTS(*curr);
      if (num_components == D3D10_SB_OPERAND_4_COMPONENT) {
         D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE selection_mode;

         selection_mode = DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(*curr);

         if (selection_mode == D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE) {
            opcode->src[i].swizzle[0] = DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(*curr, 0);
            opcode->src[i].swizzle[1] = DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(*curr, 1);
            opcode->src[i].swizzle[2] = DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(*curr, 2);
            opcode->src[i].swizzle[3] = DECODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SOURCE(*curr, 3);
         } else if (selection_mode == D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE) {
            opcode->src[i].swizzle[0] =
               opcode->src[i].swizzle[1] =
               opcode->src[i].swizzle[2] =
               opcode->src[i].swizzle[3] = DECODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(*curr);
         } else {
            /* This case apparently happens only for 4-component 32-bit
             * immediate operands.
             */
            assert(selection_mode == D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
            assert(DECODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(*curr) == 0);
            assert(DECODE_D3D10_SB_OPERAND_TYPE(*curr) == D3D10_SB_OPERAND_TYPE_IMMEDIATE32);


            opcode->src[i].swizzle[0] = D3D10_SB_4_COMPONENT_X;
            opcode->src[i].swizzle[1] = D3D10_SB_4_COMPONENT_Y;
            opcode->src[i].swizzle[2] = D3D10_SB_4_COMPONENT_Z;
            opcode->src[i].swizzle[3] = D3D10_SB_4_COMPONENT_W;
         }
      } else if (num_components == D3D10_SB_OPERAND_1_COMPONENT) {
         opcode->src[i].swizzle[0] =
            opcode->src[i].swizzle[1] =
            opcode->src[i].swizzle[2] =
            opcode->src[i].swizzle[3] = D3D10_SB_4_COMPONENT_X;
      } else {
         /* Samplers only?
          */
         assert(num_components == D3D10_SB_OPERAND_0_COMPONENT);
         assert(DECODE_D3D10_SB_OPERAND_TYPE(*curr) == D3D10_SB_OPERAND_TYPE_SAMPLER ||
                DECODE_D3D10_SB_OPERAND_TYPE(*curr) == D3D10_SB_OPERAND_TYPE_LABEL);

         opcode->src[i].swizzle[0] = D3D10_SB_4_COMPONENT_X;
         opcode->src[i].swizzle[1] = D3D10_SB_4_COMPONENT_Y;
         opcode->src[i].swizzle[2] = D3D10_SB_4_COMPONENT_Z;
         opcode->src[i].swizzle[3] = D3D10_SB_4_COMPONENT_W;
      }

      parse_operand(&curr, &opcode->src[i].base);

      opcode->src[i].modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
      if (extended) {
         /* NOTE: DECODE_IS_D3D10_SB_OPERAND_DOUBLE_EXTENDED is broken.
          */
         assert(!((*curr & D3D10_SB_OPERAND_DOUBLE_EXTENDED_MASK) >> D3D10_SB_OPERAND_DOUBLE_EXTENDED_SHIFT));

         switch (DECODE_D3D10_SB_EXTENDED_OPERAND_TYPE(*curr)) {
         case D3D10_SB_EXTENDED_OPERAND_EMPTY:
            break;

         case D3D10_SB_EXTENDED_OPERAND_MODIFIER:
            opcode->src[i].modifier = DECODE_D3D10_SB_OPERAND_MODIFIER(*curr);
            break;

         default:
            assert(0);
         }

         curr++;
      }

      parse_operand_index(&curr, &opcode->src[i].base);

      if (opcode->src[i].base.type == D3D10_SB_OPERAND_TYPE_IMMEDIATE32) {
         switch (num_components) {
         case D3D10_SB_OPERAND_1_COMPONENT:
            opcode->src[i].imm[0].u32 =
               opcode->src[i].imm[1].u32 =
               opcode->src[i].imm[2].u32 =
               opcode->src[i].imm[3].u32 = *curr++;
            break;

         case D3D10_SB_OPERAND_4_COMPONENT:
            opcode->src[i].imm[0].u32 = *curr++;
            opcode->src[i].imm[1].u32 = *curr++;
            opcode->src[i].imm[2].u32 = *curr++;
            opcode->src[i].imm[3].u32 = *curr++;
            break;

         default:
            /* XXX: Support other component sizes.
             */
            assert(0);
         }
      }
   }

   /* Opcode specific trailing operands. */
   switch (opcode->type) {
   case D3D10_SB_OPCODE_DCL_RESOURCE:
      opcode->dcl_resource_ret_type[0] = DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*curr, 0);
      opcode->dcl_resource_ret_type[1] = DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*curr, 1);
      opcode->dcl_resource_ret_type[2] = DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*curr, 2);
      opcode->dcl_resource_ret_type[3] = DECODE_D3D10_SB_RESOURCE_RETURN_TYPE(*curr, 3);
      curr++;
      break;
   case D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT:
      opcode->specific.dcl_max_output_vertex_count = *curr;
      curr++;
      break;
   case D3D10_SB_OPCODE_DCL_INPUT_SGV:
   case D3D10_SB_OPCODE_DCL_INPUT_SIV:
   case D3D10_SB_OPCODE_DCL_INPUT_PS_SGV:
   case D3D10_SB_OPCODE_DCL_INPUT_PS_SIV:
   case D3D10_SB_OPCODE_DCL_OUTPUT_SIV:
   case D3D10_SB_OPCODE_DCL_OUTPUT_SGV:
      opcode->dcl_siv_name = DECODE_D3D10_SB_NAME(*curr);
      curr++;
      break;
   case D3D10_SB_OPCODE_DCL_TEMPS:
      opcode->specific.dcl_num_temps = *curr;
      curr++;
      break;
   case D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP:
      opcode->specific.dcl_indexable_temp.index = *curr++;
      opcode->specific.dcl_indexable_temp.count = *curr++;
      opcode->specific.dcl_indexable_temp.components = *curr++;
      break;
   case D3D10_SB_OPCODE_DCL_INDEX_RANGE:
      opcode->specific.index_range_count = *curr++;
      break;
   default:
      break;
   }

   assert(curr == parser->curr + length);

skip:
   /* Advance to the next opcode. */
   parser->curr += length;

   return true;
}

void
Shader_opcode_free(struct Shader_opcode *opcode)
{
   if (opcode->type == D3D10_SB_OPCODE_CUSTOMDATA) {
      if (opcode->customdata._class == D3D10_SB_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER) {
         FREE(opcode->customdata.u.constbuf.data);
      }
   }
}
