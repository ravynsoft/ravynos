/*
 * Copyright © 2015 Intel Corporation
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
 *
 */

#include "nir.h"
#include "nir_builder.h"

/*
 * lowers:
 *
 * packDouble2x32(foo) -> packDouble2x32Split(foo.x, foo.y)
 * unpackDouble2x32(foo) -> vec2(unpackDouble2x32_x(foo), unpackDouble2x32_y(foo))
 * packInt2x32(foo) -> packInt2x32Split(foo.x, foo.y)
 * unpackInt2x32(foo) -> vec2(unpackInt2x32_x(foo), unpackInt2x32_y(foo))
 */

static nir_def *
lower_pack_64_from_32(nir_builder *b, nir_def *src)
{
   return nir_pack_64_2x32_split(b, nir_channel(b, src, 0),
                                 nir_channel(b, src, 1));
}

static nir_def *
lower_unpack_64_to_32(nir_builder *b, nir_def *src)
{
   return nir_vec2(b, nir_unpack_64_2x32_split_x(b, src),
                   nir_unpack_64_2x32_split_y(b, src));
}

static nir_def *
lower_pack_32_from_16(nir_builder *b, nir_def *src)
{
   return nir_pack_32_2x16_split(b, nir_channel(b, src, 0),
                                 nir_channel(b, src, 1));
}

static nir_def *
lower_unpack_32_to_16(nir_builder *b, nir_def *src)
{
   return nir_vec2(b, nir_unpack_32_2x16_split_x(b, src),
                   nir_unpack_32_2x16_split_y(b, src));
}

static nir_def *
lower_pack_64_from_16(nir_builder *b, nir_def *src)
{
   nir_def *xy = nir_pack_32_2x16_split(b, nir_channel(b, src, 0),
                                        nir_channel(b, src, 1));

   nir_def *zw = nir_pack_32_2x16_split(b, nir_channel(b, src, 2),
                                        nir_channel(b, src, 3));

   return nir_pack_64_2x32_split(b, xy, zw);
}

static nir_def *
lower_unpack_64_to_16(nir_builder *b, nir_def *src)
{
   nir_def *xy = nir_unpack_64_2x32_split_x(b, src);
   nir_def *zw = nir_unpack_64_2x32_split_y(b, src);

   return nir_vec4(b, nir_unpack_32_2x16_split_x(b, xy),
                   nir_unpack_32_2x16_split_y(b, xy),
                   nir_unpack_32_2x16_split_x(b, zw),
                   nir_unpack_32_2x16_split_y(b, zw));
}

static nir_def *
lower_pack_32_from_8(nir_builder *b, nir_def *src)
{
   if (b->shader->options->has_pack_32_4x8) {
      return nir_pack_32_4x8_split(b,
                                   nir_channel(b, src, 0),
                                   nir_channel(b, src, 1),
                                   nir_channel(b, src, 2),
                                   nir_channel(b, src, 3));
   } else {
      nir_def *src32 = nir_u2u32(b, src);

      return nir_ior(b,
                     nir_ior(b,
                                             nir_channel(b, src32, 0)     ,
                             nir_ishl_imm(b, nir_channel(b, src32, 1), 8)),
                     nir_ior(b,
                             nir_ishl_imm(b, nir_channel(b, src32, 2), 16),
                             nir_ishl_imm(b, nir_channel(b, src32, 3), 24)));
   }
}

static nir_def *
lower_unpack_32_to_8(nir_builder *b, nir_def *src)
{
   /* Some drivers call nir_lower_pack after the last time nir_opt_algebraic
    * is called. To prevent issues there, don't generate byte extraction
    * instructions when the lowering flag is set.
    */
   if (b->shader->options->lower_extract_byte) {
      return nir_vec4(b, nir_u2u8(b,                 src     ),
                         nir_u2u8(b, nir_ushr_imm(b, src,  8)),
                         nir_u2u8(b, nir_ushr_imm(b, src, 16)),
                         nir_u2u8(b, nir_ushr_imm(b, src, 24)));
   } else {
      return nir_vec4(b, nir_u2u8(b, nir_extract_u8_imm(b, src, 0)),
                         nir_u2u8(b, nir_extract_u8_imm(b, src, 1)),
                         nir_u2u8(b, nir_extract_u8_imm(b, src, 2)),
                         nir_u2u8(b, nir_extract_u8_imm(b, src, 3)));
   }
}

static bool
lower_pack_instr(nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu_instr = (nir_alu_instr *)instr;

   if (alu_instr->op != nir_op_pack_64_2x32 &&
       alu_instr->op != nir_op_unpack_64_2x32 &&
       alu_instr->op != nir_op_pack_64_4x16 &&
       alu_instr->op != nir_op_unpack_64_4x16 &&
       alu_instr->op != nir_op_pack_32_2x16 &&
       alu_instr->op != nir_op_unpack_32_2x16 &&
       alu_instr->op != nir_op_pack_32_4x8 &&
       alu_instr->op != nir_op_unpack_32_4x8)
      return false;

   b->cursor = nir_before_instr(&alu_instr->instr);

   nir_def *src = nir_ssa_for_alu_src(b, alu_instr, 0);
   nir_def *dest;

   switch (alu_instr->op) {
   case nir_op_pack_64_2x32:
      dest = lower_pack_64_from_32(b, src);
      break;
   case nir_op_unpack_64_2x32:
      dest = lower_unpack_64_to_32(b, src);
      break;
   case nir_op_pack_64_4x16:
      dest = lower_pack_64_from_16(b, src);
      break;
   case nir_op_unpack_64_4x16:
      dest = lower_unpack_64_to_16(b, src);
      break;
   case nir_op_pack_32_2x16:
      dest = lower_pack_32_from_16(b, src);
      break;
   case nir_op_unpack_32_2x16:
      dest = lower_unpack_32_to_16(b, src);
      break;
   case nir_op_pack_32_4x8:
      dest = lower_pack_32_from_8(b, src);
      break;
   case nir_op_unpack_32_4x8:
      dest = lower_unpack_32_to_8(b, src);
      break;
   default:
      unreachable("Impossible opcode");
   }
   nir_def_rewrite_uses(&alu_instr->def, dest);
   nir_instr_remove(&alu_instr->instr);

   return true;
}

bool
nir_lower_pack(nir_shader *shader)
{
   return nir_shader_instructions_pass(shader, lower_pack_instr,
                                       nir_metadata_block_index | nir_metadata_dominance, NULL);
}
