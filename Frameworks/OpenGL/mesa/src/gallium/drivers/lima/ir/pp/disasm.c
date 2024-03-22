/*
 * Copyright (c) 2018 Lima Project
 *
 * Copyright (c) 2013 Codethink (http://www.codethink.co.uk)
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
 *
 */

#include "util/half_float.h"

#include "ppir.h"
#include "codegen.h"

typedef struct {
   char *name;
   unsigned srcs;
} asm_op;

static void
print_swizzle(uint8_t swizzle, FILE *fp)
{
   if (swizzle == 0xE4)
      return;

   fprintf(fp, ".");
   for (unsigned i = 0; i < 4; i++, swizzle >>= 2)
      fprintf(fp, "%c", "xyzw"[swizzle & 3]);
}

static void
print_mask(uint8_t mask, FILE *fp)
{
   if (mask == 0xF)
      return;

   fprintf(fp, ".");
   if (mask & 1) fprintf(fp, "x");
   if (mask & 2) fprintf(fp, "y");
   if (mask & 4) fprintf(fp, "z");
   if (mask & 8) fprintf(fp, "w");
}

static void
print_reg(ppir_codegen_vec4_reg reg, const char *special, FILE *fp)
{
   if (special) {
      fprintf(fp, "%s", special);
   } else {
      switch (reg)
      {
         case ppir_codegen_vec4_reg_constant0:
            fprintf(fp, "^const0");
            break;
         case ppir_codegen_vec4_reg_constant1:
            fprintf(fp, "^const1");
            break;
         case ppir_codegen_vec4_reg_texture:
            fprintf(fp, "^texture");
            break;
         case ppir_codegen_vec4_reg_uniform:
            fprintf(fp, "^uniform");
            break;
         default:
            fprintf(fp, "$%u", reg);
            break;
      }
   }
}

static void
print_vector_source(ppir_codegen_vec4_reg reg, const char *special,
                    uint8_t swizzle, bool abs, bool neg, FILE *fp)
{
   if (neg)
      fprintf(fp, "-");
   if (abs)
      fprintf(fp, "abs(");

   print_reg(reg, special, fp);
   print_swizzle(swizzle, fp);

   if (abs)
      fprintf(fp, ")");
}

static void
print_source_scalar(unsigned reg, const char *special, bool abs, bool neg, FILE *fp)
{
   if (neg)
      fprintf(fp, "-");
   if (abs)
      fprintf(fp, "abs(");

   print_reg(reg >> 2, special, fp);
   if (!special)
      fprintf(fp, ".%c", "xyzw"[reg & 3]);

   if (abs)
      fprintf(fp, ")");
}

static void
print_varying_source(ppir_codegen_field_varying *varying, FILE *fp)
{
   switch (varying->imm.alignment) {
   case 0:
      fprintf(fp, "%u.%c", varying->imm.index >> 2,
             "xyzw"[varying->imm.index & 3]);
      break;
   case 1: {
      const char *c[2] = {"xy", "zw"};
      fprintf(fp, "%u.%s", varying->imm.index >> 1, c[varying->imm.index & 1]);
      break;
   }
   default:
      fprintf(fp, "%u", varying->imm.index);
      break;
   }

   if (varying->imm.offset_vector != 15) {
      unsigned reg = (varying->imm.offset_vector << 2) +
         varying->imm.offset_scalar;
      fprintf(fp, "+");
      print_source_scalar(reg, NULL, false, false, fp);
   }
}

static void
print_outmod(ppir_codegen_outmod modifier, FILE *fp)
{
   switch (modifier)
   {
      case ppir_codegen_outmod_clamp_fraction:
         fprintf(fp, ".sat");
         break;
      case ppir_codegen_outmod_clamp_positive:
         fprintf(fp, ".pos");
         break;
      case ppir_codegen_outmod_round:
         fprintf(fp, ".int");
         break;
      default:
         break;
   }
}

static void
print_dest_scalar(unsigned reg, FILE *fp)
{
   fprintf(fp, "$%u", reg >> 2);
   fprintf(fp, ".%c ", "xyzw"[reg & 3]);
}

static void
print_const(unsigned const_num, uint16_t *val, FILE *fp)
{
   fprintf(fp, "const%u", const_num);
   for (unsigned i = 0; i < 4; i++)
      fprintf(fp, " %f", _mesa_half_to_float(val[i]));
}

static void
print_const0(void *code, unsigned offset, FILE *fp)
{
   (void) offset;

   print_const(0, code, fp);
}

static void
print_const1(void *code, unsigned offset, FILE *fp)
{
   (void) offset;

   print_const(1, code, fp);
}

static void
print_varying(void *code, unsigned offset, FILE *fp)
{
   (void) offset;
   ppir_codegen_field_varying *varying = code;

   fprintf(fp, "load");

   bool perspective = varying->imm.source_type < 2 && varying->imm.perspective;
   if (perspective)
   {
      fprintf(fp, ".perspective");
      switch (varying->imm.perspective)
      {
      case 2:
         fprintf(fp, ".z");
         break;
      case 3:
         fprintf(fp, ".w");
         break;
      default:
         fprintf(fp, ".unknown");
         break;
      }
   }

   fprintf(fp, ".v ");

   switch (varying->imm.dest)
   {
   case ppir_codegen_vec4_reg_discard:
      fprintf(fp, "^discard");
      break;
   default:
      fprintf(fp, "$%u", varying->imm.dest);
      break;
   }
   print_mask(varying->imm.mask, fp);
   fprintf(fp, " ");

   switch (varying->imm.source_type) {
   case 1:
      print_vector_source(varying->reg.source, NULL, varying->reg.swizzle,
                          varying->reg.absolute, varying->reg.negate, fp);
      break;
   case 2:
      switch (varying->imm.perspective) {
      case 0:
         fprintf(fp, "cube(");
         print_varying_source(varying, fp);
         fprintf(fp, ")");
         break;
      case 1:
         fprintf(fp, "cube(");
         print_vector_source(varying->reg.source, NULL, varying->reg.swizzle,
                             varying->reg.absolute, varying->reg.negate, fp);
         fprintf(fp, ")");
         break;
      case 2:
         fprintf(fp, "normalize(");
         print_vector_source(varying->reg.source, NULL, varying->reg.swizzle,
                             varying->reg.absolute, varying->reg.negate, fp);
         fprintf(fp, ")");
         break;
      default:
         fprintf(fp, "gl_FragCoord");
         break;
      }
      break;
   case 3:
      if (varying->imm.perspective)
         fprintf(fp, "gl_FrontFacing");
      else
         fprintf(fp, "gl_PointCoord");
      break;
   default:
      print_varying_source(varying, fp);
      break;
   }
}

static void
print_sampler(void *code, unsigned offset, FILE *fp)
{
   (void) offset;
   ppir_codegen_field_sampler *sampler = code;

   fprintf(fp, "texld");
   if (sampler->lod_bias_en)
      fprintf(fp, ".b");

   switch (sampler->type) {
   case ppir_codegen_sampler_type_generic:
      break;
   case ppir_codegen_sampler_type_cube:
      fprintf(fp, ".cube");
      break;
   default:
      fprintf(fp, "_t%u", sampler->type);
      break;
   }

   fprintf(fp, " %u", sampler->index);

   if (sampler->offset_en)
   {
      fprintf(fp, "+");
      print_source_scalar(sampler->index_offset, NULL, false, false, fp);
   }

   if (sampler->lod_bias_en)
   {
      fprintf(fp, " ");
      print_source_scalar(sampler->lod_bias, NULL, false, false, fp);
   }
}

static void
print_uniform(void *code, unsigned offset, FILE *fp)
{
   (void) offset;
   ppir_codegen_field_uniform *uniform = code;

   fprintf(fp, "load.");

   switch (uniform->source) {
   case ppir_codegen_uniform_src_uniform:
      fprintf(fp, "u");
      break;
   case ppir_codegen_uniform_src_temporary:
      fprintf(fp, "t");
      break;
   default:
      fprintf(fp, ".u%u", uniform->source);
      break;
   }

   int16_t index = uniform->index;
   switch (uniform->alignment) {
   case 2:
      fprintf(fp, " %d", index);
      break;
   case 1:
      fprintf(fp, " %d.%s", index / 2, (index & 1) ? "zw" : "xy");
      break;
   default:
      fprintf(fp, " %d.%c", index / 4, "xyzw"[index & 3]);
      break;
   }

   if (uniform->offset_en) {
      fprintf(fp, "+");
      print_source_scalar(uniform->offset_reg, NULL, false, false, fp);
   }
}

#define CASE(_name, _srcs) \
[ppir_codegen_vec4_mul_op_##_name] = { \
   .name = #_name, \
   .srcs = _srcs \
}

static const asm_op vec4_mul_ops[] = {
   [0 ... 7] = {
      .name = "mul",
      .srcs = 2
   },
   CASE(not, 1),
   CASE(and, 2),
   CASE(or, 2),
   CASE(xor, 2),
   CASE(ne, 2),
   CASE(gt, 2),
   CASE(ge, 2),
   CASE(eq, 2),
   CASE(min, 2),
   CASE(max, 2),
   CASE(mov, 1),
};

#undef CASE

static void
print_vec4_mul(void *code, unsigned offset, FILE *fp)
{
   (void) offset;
   ppir_codegen_field_vec4_mul *vec4_mul = code;

   asm_op op = vec4_mul_ops[vec4_mul->op];

   if (op.name)
      fprintf(fp, "%s", op.name);
   else
      fprintf(fp, "op%u", vec4_mul->op);
   print_outmod(vec4_mul->dest_modifier, fp);
   fprintf(fp, ".v0 ");

   if (vec4_mul->mask) {
      fprintf(fp, "$%u", vec4_mul->dest);
      print_mask(vec4_mul->mask, fp);
      fprintf(fp, " ");
   }

   print_vector_source(vec4_mul->arg0_source, NULL,
                       vec4_mul->arg0_swizzle,
                       vec4_mul->arg0_absolute,
                       vec4_mul->arg0_negate, fp);

   if (vec4_mul->op < 8 && vec4_mul->op != 0) {
      fprintf(fp, "<<%u", vec4_mul->op);
   }

   fprintf(fp, " ");

   if (op.srcs > 1) {
      print_vector_source(vec4_mul->arg1_source, NULL,
                          vec4_mul->arg1_swizzle,
                          vec4_mul->arg1_absolute,
                          vec4_mul->arg1_negate, fp);
   }
}

#define CASE(_name, _srcs) \
[ppir_codegen_vec4_acc_op_##_name] = { \
   .name = #_name, \
   .srcs = _srcs \
}

static const asm_op vec4_acc_ops[] = {
   CASE(add, 2),
   CASE(fract, 1),
   CASE(ne, 2),
   CASE(gt, 2),
   CASE(ge, 2),
   CASE(eq, 2),
   CASE(floor, 1),
   CASE(ceil, 1),
   CASE(min, 2),
   CASE(max, 2),
   CASE(sum3, 1),
   CASE(sum4, 1),
   CASE(dFdx, 2),
   CASE(dFdy, 2),
   CASE(sel, 2),
   CASE(mov, 1),
};

#undef CASE

static void
print_vec4_acc(void *code, unsigned offset, FILE *fp)
{
   (void) offset;
   ppir_codegen_field_vec4_acc *vec4_acc = code;

   asm_op op = vec4_acc_ops[vec4_acc->op];

   if (op.name)
      fprintf(fp, "%s", op.name);
   else
      fprintf(fp, "op%u", vec4_acc->op);
   print_outmod(vec4_acc->dest_modifier, fp);
   fprintf(fp, ".v1 ");

   if (vec4_acc->mask) {
      fprintf(fp, "$%u", vec4_acc->dest);
      print_mask(vec4_acc->mask, fp);
      fprintf(fp, " ");
   }

   print_vector_source(vec4_acc->arg0_source, vec4_acc->mul_in ? "^v0" : NULL,
                       vec4_acc->arg0_swizzle,
                       vec4_acc->arg0_absolute,
                       vec4_acc->arg0_negate, fp);

   if (op.srcs > 1) {
      fprintf(fp, " ");
      print_vector_source(vec4_acc->arg1_source, NULL,
                          vec4_acc->arg1_swizzle,
                          vec4_acc->arg1_absolute,
                          vec4_acc->arg1_negate, fp);
   }
}

#define CASE(_name, _srcs) \
[ppir_codegen_float_mul_op_##_name] = { \
   .name = #_name, \
   .srcs = _srcs \
}

static const asm_op float_mul_ops[] = {
   [0 ... 7] = {
      .name = "mul",
      .srcs = 2
   },
   CASE(not, 1),
   CASE(and, 2),
   CASE(or, 2),
   CASE(xor, 2),
   CASE(ne, 2),
   CASE(gt, 2),
   CASE(ge, 2),
   CASE(eq, 2),
   CASE(min, 2),
   CASE(max, 2),
   CASE(mov, 1),
};

#undef CASE

static void
print_float_mul(void *code, unsigned offset, FILE *fp)
{
   (void) offset;
   ppir_codegen_field_float_mul *float_mul = code;

   asm_op op = float_mul_ops[float_mul->op];

   if (op.name)
      fprintf(fp, "%s", op.name);
   else
      fprintf(fp, "op%u", float_mul->op);
   print_outmod(float_mul->dest_modifier, fp);
   fprintf(fp, ".s0 ");

   if (float_mul->output_en)
      print_dest_scalar(float_mul->dest, fp);

   print_source_scalar(float_mul->arg0_source, NULL,
                       float_mul->arg0_absolute,
                       float_mul->arg0_negate, fp);

   if (float_mul->op < 8 && float_mul->op != 0) {
      fprintf(fp, "<<%u", float_mul->op);
   }

   if (op.srcs > 1) {
      fprintf(fp, " ");

      print_source_scalar(float_mul->arg1_source, NULL,
                          float_mul->arg1_absolute,
                          float_mul->arg1_negate, fp);
   }
}

#define CASE(_name, _srcs) \
[ppir_codegen_float_acc_op_##_name] = { \
   .name = #_name, \
   .srcs = _srcs \
}

static const asm_op float_acc_ops[] = {
   CASE(add, 2),
   CASE(fract, 1),
   CASE(ne, 2),
   CASE(gt, 2),
   CASE(ge, 2),
   CASE(eq, 2),
   CASE(floor, 1),
   CASE(ceil, 1),
   CASE(min, 2),
   CASE(max, 2),
   CASE(dFdx, 2),
   CASE(dFdy, 2),
   CASE(sel, 2),
   CASE(mov, 1),
};

#undef CASE

static void
print_float_acc(void *code, unsigned offset, FILE *fp)
{
   (void) offset;
   ppir_codegen_field_float_acc *float_acc = code;

   asm_op op = float_acc_ops[float_acc->op];

   if (op.name)
      fprintf(fp, "%s", op.name);
   else
      fprintf(fp, "op%u", float_acc->op);
   print_outmod(float_acc->dest_modifier, fp);
   fprintf(fp, ".s1 ");

   if (float_acc->output_en)
      print_dest_scalar(float_acc->dest, fp);

   print_source_scalar(float_acc->arg0_source, float_acc->mul_in ? "^s0" : NULL,
                       float_acc->arg0_absolute,
                       float_acc->arg0_negate, fp);

   if (op.srcs > 1) {
      fprintf(fp, " ");
      print_source_scalar(float_acc->arg1_source, NULL,
                          float_acc->arg1_absolute,
                          float_acc->arg1_negate, fp);
   }
}

#define CASE(_name, _srcs) \
[ppir_codegen_combine_scalar_op_##_name] = { \
   .name = #_name, \
   .srcs = _srcs \
}

static const asm_op combine_ops[] = {
   CASE(rcp, 1),
   CASE(mov, 1),
   CASE(sqrt, 1),
   CASE(rsqrt, 1),
   CASE(exp2, 1),
   CASE(log2, 1),
   CASE(sin, 1),
   CASE(cos, 1),
   CASE(atan, 1),
   CASE(atan2, 1),
};

#undef CASE

static void
print_combine(void *code, unsigned offset, FILE *fp)
{
   (void) offset;
   ppir_codegen_field_combine *combine = code;

   if (combine->scalar.dest_vec &&
       combine->scalar.arg1_en) {
      /* This particular combination can only be valid for scalar * vector
       * multiplies, and the opcode field is reused for something else.
       */
      fprintf(fp, "mul");
   } else {
      asm_op op = combine_ops[combine->scalar.op];

      if (op.name)
         fprintf(fp, "%s", op.name);
      else
         fprintf(fp, "op%u", combine->scalar.op);
   }

   if (!combine->scalar.dest_vec)
      print_outmod(combine->scalar.dest_modifier, fp);
   fprintf(fp, ".s2 ");

   if (combine->scalar.dest_vec) {
      fprintf(fp, "$%u", combine->vector.dest);
      print_mask(combine->vector.mask, fp);
   } else {
      print_dest_scalar(combine->scalar.dest, fp);
   }
   fprintf(fp, " ");

   print_source_scalar(combine->scalar.arg0_src, NULL,
                       combine->scalar.arg0_absolute,
                       combine->scalar.arg0_negate, fp);
   fprintf(fp, " ");

   if (combine->scalar.arg1_en) {
      if (combine->scalar.dest_vec) {
         print_vector_source(combine->vector.arg1_source, NULL,
                             combine->vector.arg1_swizzle,
                             false, false, fp);
      } else {
         print_source_scalar(combine->scalar.arg1_src, NULL,
                             combine->scalar.arg1_absolute,
                             combine->scalar.arg1_negate, fp);
      }
   }
}

static void
print_temp_write(void *code, unsigned offset, FILE *fp)
{
   (void) offset;
   ppir_codegen_field_temp_write *temp_write = code;

   if (temp_write->fb_read.unknown_0 == 0x7) {
      if (temp_write->fb_read.source)
         fprintf(fp, "fb_color");
      else
         fprintf(fp, "fb_depth");
      fprintf(fp, " $%u", temp_write->fb_read.dest);

      return;
   }

   fprintf(fp, "store.t");

   int16_t index = temp_write->temp_write.index;
   switch (temp_write->temp_write.alignment) {
   case 2:
      fprintf(fp, " %d", index);
      break;
   case 1:
      fprintf(fp, " %d.%s", index / 2, (index & 1) ? "zw" : "xy");
      break;
   default:
      fprintf(fp, " %d.%c", index / 4, "xyzw"[index & 3]);
      break;
   }

   if (temp_write->temp_write.offset_en) {
      fprintf(fp, "+");
      print_source_scalar(temp_write->temp_write.offset_reg,
                          NULL, false, false, fp);
   }

   fprintf(fp, " ");

   if (temp_write->temp_write.alignment) {
      print_reg(temp_write->temp_write.source >> 2, NULL, fp);
   } else {
      print_source_scalar(temp_write->temp_write.source, NULL, false, false, fp);
   }
}

static void
print_branch(void *code, unsigned offset, FILE *fp)
{
   ppir_codegen_field_branch *branch = code;

   if (branch->discard.word0 == PPIR_CODEGEN_DISCARD_WORD0 &&
       branch->discard.word1 == PPIR_CODEGEN_DISCARD_WORD1 &&
       branch->discard.word2 == PPIR_CODEGEN_DISCARD_WORD2) {
      fprintf(fp, "discard");
      return;
   }

   const char* cond[] = {
      "nv", "lt", "eq", "le",
      "gt", "ne", "ge", ""  ,
   };

   unsigned cond_mask = 0;
   cond_mask |= (branch->branch.cond_lt ? 1 : 0);
   cond_mask |= (branch->branch.cond_eq ? 2 : 0);
   cond_mask |= (branch->branch.cond_gt ? 4 : 0);
   fprintf(fp, "branch");
   if (cond_mask != 0x7) {
      fprintf(fp, ".%s ", cond[cond_mask]);
      print_source_scalar(branch->branch.arg0_source, NULL, false, false, fp);
      fprintf(fp, " ");
      print_source_scalar(branch->branch.arg1_source, NULL, false, false, fp);
   }

   fprintf(fp, " %d", branch->branch.target + offset);
}

typedef void (*print_field_func)(void *, unsigned, FILE *);

static const print_field_func print_field[ppir_codegen_field_shift_count] = {
   [ppir_codegen_field_shift_varying] = print_varying,
   [ppir_codegen_field_shift_sampler] = print_sampler,
   [ppir_codegen_field_shift_uniform] = print_uniform,
   [ppir_codegen_field_shift_vec4_mul] = print_vec4_mul,
   [ppir_codegen_field_shift_float_mul] = print_float_mul,
   [ppir_codegen_field_shift_vec4_acc] = print_vec4_acc,
   [ppir_codegen_field_shift_float_acc] = print_float_acc,
   [ppir_codegen_field_shift_combine] = print_combine,
   [ppir_codegen_field_shift_temp_write] = print_temp_write,
   [ppir_codegen_field_shift_branch] = print_branch,
   [ppir_codegen_field_shift_vec4_const_0] = print_const0,
   [ppir_codegen_field_shift_vec4_const_1] = print_const1,
};

static const int ppir_codegen_field_size[] = {
   34, 62, 41, 43, 30, 44, 31, 30, 41, 73, 64, 64
};

static void
bitcopy(unsigned char *src, unsigned char *dst, unsigned bits, unsigned src_offset)
{
   src += src_offset / 8;
   src_offset %= 8;

   for (unsigned b = bits; b > 0; b -= MIN2(b, 8), src++, dst++) {
      unsigned char out = *src >> src_offset;
      if (src_offset > 0 && src_offset + b > 8)
         out |= *(src + 1) << (8 - src_offset);
      *dst = out;
   }
}

void
ppir_disassemble_instr(uint32_t *instr, unsigned offset, FILE *fp)
{
   ppir_codegen_ctrl *ctrl = (ppir_codegen_ctrl *) instr;

   unsigned char *instr_code = (unsigned char *) (instr + 1);
   unsigned bit_offset = 0;
   bool first = true;
   for (unsigned i = 0; i < ppir_codegen_field_shift_count; i++) {
      unsigned char code[12];

      if (!((ctrl->fields >> i) & 1))
         continue;

      unsigned bits = ppir_codegen_field_size[i];
      bitcopy(instr_code, code, bits, bit_offset);

      if (first)
         first = false;
      else
         fprintf(fp, ", ");

      print_field[i](code, offset, fp);

      bit_offset += bits;
   }

   if (ctrl->sync)
      fprintf(fp, ", sync");
   if (ctrl->stop)
      fprintf(fp, ", stop");

   fprintf(fp, "\n");
}

