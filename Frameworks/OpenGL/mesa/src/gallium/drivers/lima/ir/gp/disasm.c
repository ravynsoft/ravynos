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

#include "gpir.h"
#include "codegen.h"

typedef enum {
   unit_acc_0,
   unit_acc_1,
   unit_mul_0,
   unit_mul_1,
   unit_pass,
   unit_complex,
   num_units
} gp_unit;

static const gpir_codegen_store_src gp_unit_to_store_src[num_units] = {
   [unit_acc_0] = gpir_codegen_store_src_acc_0,
   [unit_acc_1] = gpir_codegen_store_src_acc_1,
   [unit_mul_0] = gpir_codegen_store_src_mul_0,
   [unit_mul_1] = gpir_codegen_store_src_mul_1,
   [unit_pass] = gpir_codegen_store_src_pass,
   [unit_complex] = gpir_codegen_store_src_complex,
};

static void
print_dest(gpir_codegen_instr *instr, gp_unit unit, unsigned cur_dest_index, FILE *fp)
{
   fprintf(fp, "^%u", cur_dest_index + unit);

   gpir_codegen_store_src src = gp_unit_to_store_src[unit];

   if (instr->store0_src_x == src ||
       instr->store0_src_y == src) {
      if (instr->store0_temporary) {
         /* Temporary stores ignore the address, and always use whatever's
          * stored in address register 0.
          */
         fprintf(fp, "/t[addr0]");
      } else {
         if (instr->store0_varying)
            fprintf(fp, "/v");
         else
            fprintf(fp, "/$");
         fprintf(fp, "%u", instr->store0_addr);
      }

      fprintf(fp, ".");
      if (instr->store0_src_x == src)
         fprintf(fp, "x");
      if (instr->store0_src_y == src)
         fprintf(fp, "y");
   }

   if (instr->store1_src_z == src ||
       instr->store1_src_w == src) {
      if (instr->store1_temporary) {
         fprintf(fp, "/t[addr0]");
      } else {
         if (instr->store1_varying)
            fprintf(fp, "/v");
         else
            fprintf(fp, "/$");
         fprintf(fp, "%u", instr->store1_addr);
      }

      fprintf(fp, ".");
      if (instr->store1_src_z == src)
         fprintf(fp, "z");
      if (instr->store1_src_w == src)
         fprintf(fp, "w");
   }

   if (unit == unit_complex) {
      switch (instr->complex_op) {
      case gpir_codegen_complex_op_temp_store_addr:
         fprintf(fp, "/addr0");
         break;
      case gpir_codegen_complex_op_temp_load_addr_0:
         fprintf(fp, "/addr1");
         break;
      case gpir_codegen_complex_op_temp_load_addr_1:
         fprintf(fp, "/addr2");
         break;
      case gpir_codegen_complex_op_temp_load_addr_2:
         fprintf(fp, "/addr3");
         break;
      default:
         break;
      }
   }
}

static void
print_src(gpir_codegen_src src, gp_unit unit, unsigned unit_src_num,
          gpir_codegen_instr *instr, gpir_codegen_instr *prev_instr,
          unsigned cur_dest_index, FILE *fp)
{
   switch (src) {
   case gpir_codegen_src_attrib_x:
   case gpir_codegen_src_attrib_y:
   case gpir_codegen_src_attrib_z:
   case gpir_codegen_src_attrib_w:
      fprintf(fp, "%c%d.%c", instr->register0_attribute ? 'a' : '$',
             instr->register0_addr, "xyzw"[src - gpir_codegen_src_attrib_x]);
      break;

   case gpir_codegen_src_register_x:
   case gpir_codegen_src_register_y:
   case gpir_codegen_src_register_z:
   case gpir_codegen_src_register_w:
      fprintf(fp, "$%d.%c", instr->register1_addr,
             "xyzw"[src - gpir_codegen_src_register_x]);
      break;

   case gpir_codegen_src_unknown_0:
   case gpir_codegen_src_unknown_1:
   case gpir_codegen_src_unknown_2:
   case gpir_codegen_src_unknown_3:
      fprintf(fp, "unknown%d", src - gpir_codegen_src_unknown_0);
      break;

   case gpir_codegen_src_load_x:
   case gpir_codegen_src_load_y:
   case gpir_codegen_src_load_z:
   case gpir_codegen_src_load_w:
      fprintf(fp, "t[%d", instr->load_addr);
      switch (instr->load_offset) {
      case gpir_codegen_load_off_ld_addr_0:
         fprintf(fp, "+addr1");
         break;
      case gpir_codegen_load_off_ld_addr_1:
         fprintf(fp, "+addr2");
         break;
      case gpir_codegen_load_off_ld_addr_2:
         fprintf(fp, "+addr3");
         break;
      case gpir_codegen_load_off_none:
         break;
      default:
         fprintf(fp, "+unk%d", instr->load_offset);
      }
      fprintf(fp, "].%c", "xyzw"[src - gpir_codegen_src_load_x]);
      break;

   case gpir_codegen_src_p1_acc_0:
      fprintf(fp, "^%d", cur_dest_index - 1 * num_units + unit_acc_0);
      break;

   case gpir_codegen_src_p1_acc_1:
      fprintf(fp, "^%d", cur_dest_index - 1 * num_units + unit_acc_1);
      break;

   case gpir_codegen_src_p1_mul_0:
      fprintf(fp, "^%d", cur_dest_index - 1 * num_units + unit_mul_0);
      break;

   case gpir_codegen_src_p1_mul_1:
      fprintf(fp, "^%d", cur_dest_index - 1 * num_units + unit_mul_1);
      break;

   case gpir_codegen_src_p1_pass:
      fprintf(fp, "^%d", cur_dest_index - 1 * num_units + unit_pass);
      break;

   case gpir_codegen_src_unused:
      fprintf(fp, "unused");
      break;

   case gpir_codegen_src_p1_complex: /* Also ident */
      switch (unit) {
      case unit_acc_0:
      case unit_acc_1:
         if (unit_src_num == 1) {
            fprintf(fp, "0");
            return;
         }
         break;
      case unit_mul_0:
      case unit_mul_1:
         if (unit_src_num == 1) {
            fprintf(fp, "1");
            return;
         }
         break;
      default:
         break;
      }
      fprintf(fp, "^%d", cur_dest_index - 1 * num_units + unit_complex);
      break;

   case gpir_codegen_src_p2_pass:
      fprintf(fp, "^%d", cur_dest_index - 2 * num_units + unit_pass);
      break;

   case gpir_codegen_src_p2_acc_0:
      fprintf(fp, "^%d", cur_dest_index - 2 * num_units + unit_acc_0);
      break;

   case gpir_codegen_src_p2_acc_1:
      fprintf(fp, "^%d", cur_dest_index - 2 * num_units + unit_acc_1);
      break;

   case gpir_codegen_src_p2_mul_0:
      fprintf(fp, "^%d", cur_dest_index - 2 * num_units + unit_mul_0);
      break;

   case gpir_codegen_src_p2_mul_1:
      fprintf(fp, "^%d", cur_dest_index - 2 * num_units + unit_mul_1);
      break;

   case gpir_codegen_src_p1_attrib_x:
   case gpir_codegen_src_p1_attrib_y:
   case gpir_codegen_src_p1_attrib_z:
   case gpir_codegen_src_p1_attrib_w:
      fprintf(fp, "%c%d.%c", prev_instr->register0_attribute ? 'a' : '$',
             prev_instr->register0_addr,
             "xyzw"[src - gpir_codegen_src_p1_attrib_x]);
      break;
   }
}

static bool
print_mul(gpir_codegen_instr *instr, gpir_codegen_instr *prev_instr,
          unsigned cur_dest_index, FILE *fp)
{
   bool printed = false;

   switch (instr->mul_op) {
   case gpir_codegen_mul_op_mul:
   case gpir_codegen_mul_op_complex2:
      if (instr->mul0_src0 != gpir_codegen_src_unused &&
          instr->mul0_src1 != gpir_codegen_src_unused) {
         printed = true;
         fprintf(fp, "\t");
         if (instr->mul0_src1 == gpir_codegen_src_ident &&
             !instr->mul0_neg) {
            fprintf(fp, "mov.m0 ");
            print_dest(instr, unit_mul_0, cur_dest_index, fp);
            fprintf(fp, " ");
            print_src(instr->mul0_src0, unit_mul_0, 0, instr, prev_instr,
                      cur_dest_index, fp);
         } else {
            if (instr->mul_op == gpir_codegen_mul_op_complex2)
               fprintf(fp, "complex2.m0 ");
            else
               fprintf(fp, "mul.m0 ");

            print_dest(instr, unit_mul_0, cur_dest_index, fp);
            fprintf(fp, " ");
            print_src(instr->mul0_src0, unit_mul_0, 0, instr, prev_instr,
                      cur_dest_index, fp);
            fprintf(fp, " ");
            if (instr->mul0_neg)
               fprintf(fp, "-");
            print_src(instr->mul0_src1, unit_mul_0, 1, instr, prev_instr,
                      cur_dest_index, fp);
         }

         fprintf(fp, "\n");
      }

      if (instr->mul1_src0 != gpir_codegen_src_unused &&
          instr->mul1_src1 != gpir_codegen_src_unused) {
         printed = true;
         fprintf(fp, "\t");
         if (instr->mul1_src1 == gpir_codegen_src_ident &&
             !instr->mul1_neg) {
            fprintf(fp, "mov.m1 ");
            print_dest(instr, unit_mul_1, cur_dest_index, fp);
            fprintf(fp, " ");
            print_src(instr->mul1_src0, unit_mul_1, 0, instr, prev_instr,
                      cur_dest_index, fp);
         } else {
            fprintf(fp, "mul.m1 ");
            print_dest(instr, unit_mul_1, cur_dest_index, fp);
            fprintf(fp, " ");
            print_src(instr->mul1_src0, unit_mul_1, 0, instr, prev_instr,
                      cur_dest_index, fp);
            fprintf(fp, " ");
            if (instr->mul1_neg)
               fprintf(fp, "-");
            print_src(instr->mul1_src1, unit_mul_0, 1, instr, prev_instr,
                      cur_dest_index, fp);
         }
         fprintf(fp, "\n");
      }

      break;
   case gpir_codegen_mul_op_complex1:
      printed = true;
      fprintf(fp, "\tcomplex1.m01 ");
      print_dest(instr, unit_mul_0, cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul0_src0, unit_mul_0, 0, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul0_src1, unit_mul_0, 1, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul1_src0, unit_mul_1, 0, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul1_src1, unit_mul_1, 1, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, "\n");
      break;

   case gpir_codegen_mul_op_select:
      printed = true;
      fprintf(fp, "\tsel.m01 ");
      print_dest(instr, unit_mul_0, cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul0_src1, unit_mul_0, 1, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul0_src0, unit_mul_0, 0, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul1_src0, unit_mul_1, 0, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, "\n");
      break;

   default:
      printed = true;
      fprintf(fp, "\tunknown%u.m01 ", instr->mul_op);
      print_dest(instr, unit_mul_0, cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul0_src0, unit_mul_0, 0, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul0_src1, unit_mul_0, 1, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul1_src0, unit_mul_1, 0, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(instr->mul1_src1, unit_mul_1, 1, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, "\n");
      break;
   }

   return printed;
}

typedef struct {
   const char *name;
   unsigned srcs;
} acc_op_info;

#define CASE(_name, _srcs) \
   [gpir_codegen_acc_op_##_name] = { \
      .name = #_name, \
      .srcs = _srcs \
   }

static const acc_op_info acc_op_infos[8] = {
   CASE(add, 2),
   CASE(floor, 1),
   CASE(sign, 1),
   CASE(ge, 2),
   CASE(lt, 2),
   CASE(min, 2),
   CASE(max, 2),
};

#undef CASE

static bool
print_acc(gpir_codegen_instr *instr, gpir_codegen_instr *prev_instr,
          unsigned cur_dest_index, FILE *fp)
{
   bool printed = false;
   const acc_op_info op = acc_op_infos[instr->acc_op];

   if (instr->acc0_src0 != gpir_codegen_src_unused) {
      printed = true;
      fprintf(fp, "\t");
      acc_op_info acc0_op = op;
      if (instr->acc0_src1 == gpir_codegen_src_ident &&
          instr->acc0_src1_neg) {
         /* add x, -0 -> mov x */
         acc0_op.name = "mov";
         acc0_op.srcs = 1;
      }

      if (acc0_op.name)
         fprintf(fp, "%s.a0 ", acc0_op.name);
      else
         fprintf(fp, "op%u.a0 ", instr->acc_op);

      print_dest(instr, unit_acc_0, cur_dest_index, fp);
      fprintf(fp, " ");
      if (instr->acc0_src0_neg)
         fprintf(fp, "-");
      print_src(instr->acc0_src0, unit_acc_0, 0, instr, prev_instr,
                cur_dest_index, fp);
      if (acc0_op.srcs > 1) {
         fprintf(fp, " ");
         if (instr->acc0_src1_neg)
            fprintf(fp, "-");
         print_src(instr->acc0_src1, unit_acc_0, 1, instr, prev_instr,
                   cur_dest_index, fp);
      }

      fprintf(fp, "\n");
   }

   if (instr->acc1_src0 != gpir_codegen_src_unused) {
      printed = true;
      fprintf(fp, "\t");
      acc_op_info acc1_op = op;
      if (instr->acc1_src1 == gpir_codegen_src_ident &&
          instr->acc1_src1_neg) {
         /* add x, -0 -> mov x */
         acc1_op.name = "mov";
         acc1_op.srcs = 1;
      }

      if (acc1_op.name)
         fprintf(fp, "%s.a1 ", acc1_op.name);
      else
         fprintf(fp, "op%u.a1 ", instr->acc_op);

      print_dest(instr, unit_acc_1, cur_dest_index, fp);
      fprintf(fp, " ");
      if (instr->acc1_src0_neg)
         fprintf(fp, "-");
      print_src(instr->acc1_src0, unit_acc_1, 0, instr, prev_instr,
                cur_dest_index, fp);
      if (acc1_op.srcs > 1) {
         fprintf(fp, " ");
         if (instr->acc1_src1_neg)
            fprintf(fp, "-");
         print_src(instr->acc1_src1, unit_acc_1, 1, instr, prev_instr,
                   cur_dest_index, fp);
      }

      fprintf(fp, "\n");
   }

   return printed;
}

static bool
print_pass(gpir_codegen_instr *instr, gpir_codegen_instr *prev_instr,
           unsigned cur_dest_index, FILE *fp)
{
   if (instr->pass_src == gpir_codegen_src_unused)
      return false;

   fprintf(fp, "\t");

   switch (instr->pass_op) {
   case gpir_codegen_pass_op_pass:
      fprintf(fp, "mov.p ");
      break;
   case gpir_codegen_pass_op_preexp2:
      fprintf(fp, "preexp2.p ");
      break;
   case gpir_codegen_pass_op_postlog2:
      fprintf(fp, "postlog2.p ");
      break;
   case gpir_codegen_pass_op_clamp:
      fprintf(fp, "clamp.p ");
      break;
   default:
      fprintf(fp, "unk%u.p ", instr->pass_op);
   }

   print_dest(instr, unit_pass, cur_dest_index, fp);
   fprintf(fp, " ");
   print_src(instr->pass_src, unit_pass, 0, instr, prev_instr,
             cur_dest_index, fp);

   if (instr->pass_op == gpir_codegen_pass_op_clamp) {
      fprintf(fp, " ");
      print_src(gpir_codegen_src_load_x, unit_pass, 1, instr, prev_instr,
                cur_dest_index, fp);
      fprintf(fp, " ");
      print_src(gpir_codegen_src_load_y, unit_pass, 2, instr, prev_instr,
                cur_dest_index, fp);
   }

   fprintf(fp, "\n");

   return true;
}

static bool
print_complex(gpir_codegen_instr *instr, gpir_codegen_instr *prev_instr,
              unsigned cur_dest_index, FILE *fp)
{
   if (instr->complex_src == gpir_codegen_src_unused)
      return false;

   fprintf(fp, "\t");

   switch (instr->complex_op) {
   case gpir_codegen_complex_op_nop:
      return false;

   case gpir_codegen_complex_op_exp2:
      fprintf(fp, "exp2.c ");
      break;
   case gpir_codegen_complex_op_log2:
      fprintf(fp, "log2.c ");
      break;
   case gpir_codegen_complex_op_rsqrt:
      fprintf(fp, "rsqrt.c ");
      break;
   case gpir_codegen_complex_op_rcp:
      fprintf(fp, "rcp.c ");
      break;
   case gpir_codegen_complex_op_pass:
   case gpir_codegen_complex_op_temp_store_addr:
   case gpir_codegen_complex_op_temp_load_addr_0:
   case gpir_codegen_complex_op_temp_load_addr_1:
   case gpir_codegen_complex_op_temp_load_addr_2:
      fprintf(fp, "mov.c ");
      break;
   default:
      fprintf(fp, "unk%u.c ", instr->complex_op);
   }

   print_dest(instr, unit_complex, cur_dest_index, fp);
   fprintf(fp, " ");
   print_src(instr->complex_src, unit_complex, 0, instr, prev_instr,
             cur_dest_index, fp);
   fprintf(fp, "\n");

   return true;
}

static void
print_instr(gpir_codegen_instr *instr, gpir_codegen_instr *prev_instr,
            unsigned instr_number, unsigned cur_dest_index, FILE *fp)
{
   bool printed = false;
   fprintf(fp, "%03d:", instr_number);
   printed |= print_acc(instr, prev_instr, cur_dest_index, fp);
   printed |= print_mul(instr, prev_instr, cur_dest_index, fp);
   printed |= print_complex(instr, prev_instr, cur_dest_index, fp);
   printed |= print_pass(instr, prev_instr, cur_dest_index, fp);

   if (instr->branch) {
      printed = true;
      /* The branch condition is taken from the current pass unit result */
      fprintf(fp, "\tbranch ^%d %03d\n", cur_dest_index + unit_pass,
             instr->branch_target + (instr->branch_target_lo ? 0 : 0x100));
   }

   if (instr->unknown_1 != 0) {
      printed = true;
      fprintf(fp, "\tunknown_1 %u\n", instr->unknown_1);
   }

   if (!printed)
      fprintf(fp, "\tnop\n");
}

void
gpir_disassemble_program(gpir_codegen_instr *code, unsigned num_instr, FILE *fp)
{
   unsigned cur_dest_index = 0;
   unsigned cur_instr = 0;
   for (gpir_codegen_instr *instr = code; cur_instr < num_instr;
        instr++, cur_instr++, cur_dest_index += num_units) {
      print_instr(instr, instr - 1, cur_instr, cur_dest_index, fp);
   }
}

