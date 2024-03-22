/*
 * Copyright (C) 2019 Connor Abbott <cwabbott0@gmail.com>
 * Copyright (C) 2019 Lyude Paul <thatslyude@gmail.com>
 * Copyright (C) 2019 Ryan Houdek <Sonicadvance1@gmail.com>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "util/compiler.h"
#include "util/macros.h"
#include "bi_print_common.h"
#include "bifrost.h"
#include "disassemble.h"

// return bits (high, lo]
static uint64_t
bits(uint32_t word, unsigned lo, unsigned high)
{
   if (high == 32)
      return word >> lo;
   return (word & ((1 << high) - 1)) >> lo;
}

// each of these structs represents an instruction that's dispatched in one
// cycle. Note that these instructions are packed in funny ways within the
// clause, hence the need for a separate struct.
struct bifrost_alu_inst {
   uint32_t fma_bits;
   uint32_t add_bits;
   uint64_t reg_bits;
};

static unsigned
get_reg0(struct bifrost_regs regs)
{
   if (regs.ctrl == 0)
      return regs.reg0 | ((regs.reg1 & 0x1) << 5);

   return regs.reg0 <= regs.reg1 ? regs.reg0 : 63 - regs.reg0;
}

static unsigned
get_reg1(struct bifrost_regs regs)
{
   return regs.reg0 <= regs.reg1 ? regs.reg1 : 63 - regs.reg1;
}

// this represents the decoded version of the ctrl register field.
struct bifrost_reg_ctrl {
   bool read_reg0;
   bool read_reg1;
   struct bifrost_reg_ctrl_23 slot23;
};

static void
dump_header(FILE *fp, struct bifrost_header header, bool verbose)
{
   fprintf(fp, "ds(%u) ", header.dependency_slot);

   if (header.staging_barrier)
      fprintf(fp, "osrb ");

   fprintf(fp, "%s ", bi_flow_control_name(header.flow_control));

   if (header.suppress_inf)
      fprintf(fp, "inf_suppress ");
   if (header.suppress_nan)
      fprintf(fp, "nan_suppress ");

   if (header.flush_to_zero == BIFROST_FTZ_DX11)
      fprintf(fp, "ftz_dx11 ");
   else if (header.flush_to_zero == BIFROST_FTZ_ALWAYS)
      fprintf(fp, "ftz_hsa ");
   if (header.flush_to_zero == BIFROST_FTZ_ABRUPT)
      fprintf(fp, "ftz_au ");

   assert(!header.zero1);
   assert(!header.zero2);

   if (header.float_exceptions == BIFROST_EXCEPTIONS_DISABLED)
      fprintf(fp, "fpe_ts ");
   else if (header.float_exceptions == BIFROST_EXCEPTIONS_PRECISE_DIVISION)
      fprintf(fp, "fpe_pd ");
   else if (header.float_exceptions == BIFROST_EXCEPTIONS_PRECISE_SQRT)
      fprintf(fp, "fpe_psqr ");

   if (header.message_type)
      fprintf(fp, "%s ", bi_message_type_name(header.message_type));

   if (header.terminate_discarded_threads)
      fprintf(fp, "td ");

   if (header.next_clause_prefetch)
      fprintf(fp, "ncph ");

   if (header.next_message_type)
      fprintf(fp, "next_%s ", bi_message_type_name(header.next_message_type));
   if (header.dependency_wait != 0) {
      fprintf(fp, "dwb(");
      bool first = true;
      for (unsigned i = 0; i < 8; i++) {
         if (header.dependency_wait & (1 << i)) {
            if (!first) {
               fprintf(fp, ", ");
            }
            fprintf(fp, "%u", i);
            first = false;
         }
      }
      fprintf(fp, ") ");
   }

   fprintf(fp, "\n");
}

static struct bifrost_reg_ctrl
DecodeRegCtrl(FILE *fp, struct bifrost_regs regs, bool first)
{
   struct bifrost_reg_ctrl decoded = {};
   unsigned ctrl;
   if (regs.ctrl == 0) {
      ctrl = regs.reg1 >> 2;
      decoded.read_reg0 = !(regs.reg1 & 0x2);
      decoded.read_reg1 = false;
   } else {
      ctrl = regs.ctrl;
      decoded.read_reg0 = decoded.read_reg1 = true;
   }

   /* Modify control based on state */
   if (first)
      ctrl = (ctrl & 0x7) | ((ctrl & 0x8) << 1);
   else if (regs.reg2 == regs.reg3)
      ctrl += 16;

   decoded.slot23 = bifrost_reg_ctrl_lut[ctrl];
   ASSERTED struct bifrost_reg_ctrl_23 reserved = {0};
   assert(memcmp(&decoded.slot23, &reserved, sizeof(reserved)));

   return decoded;
}

static void
dump_regs(FILE *fp, struct bifrost_regs srcs, bool first)
{
   struct bifrost_reg_ctrl ctrl = DecodeRegCtrl(fp, srcs, first);
   fprintf(fp, "    # ");
   if (ctrl.read_reg0)
      fprintf(fp, "slot 0: r%u ", get_reg0(srcs));
   if (ctrl.read_reg1)
      fprintf(fp, "slot 1: r%u ", get_reg1(srcs));

   const char *slot3_fma = ctrl.slot23.slot3_fma ? "FMA" : "ADD";

   if (ctrl.slot23.slot2 == BIFROST_OP_WRITE)
      fprintf(fp, "slot 2: r%u (write FMA) ", srcs.reg2);
   else if (ctrl.slot23.slot2 == BIFROST_OP_WRITE_LO)
      fprintf(fp, "slot 2: r%u (write lo FMA) ", srcs.reg2);
   else if (ctrl.slot23.slot2 == BIFROST_OP_WRITE_HI)
      fprintf(fp, "slot 2: r%u (write hi FMA) ", srcs.reg2);
   else if (ctrl.slot23.slot2 == BIFROST_OP_READ)
      fprintf(fp, "slot 2: r%u (read) ", srcs.reg2);

   if (ctrl.slot23.slot3 == BIFROST_OP_WRITE)
      fprintf(fp, "slot 3: r%u (write %s) ", srcs.reg3, slot3_fma);
   else if (ctrl.slot23.slot3 == BIFROST_OP_WRITE_LO)
      fprintf(fp, "slot 3: r%u (write lo %s) ", srcs.reg3, slot3_fma);
   else if (ctrl.slot23.slot3 == BIFROST_OP_WRITE_HI)
      fprintf(fp, "slot 3: r%u (write hi %s) ", srcs.reg3, slot3_fma);

   if (srcs.fau_idx)
      fprintf(fp, "fau %X ", srcs.fau_idx);

   fprintf(fp, "\n");
}

static void
bi_disasm_dest_mask(FILE *fp, enum bifrost_reg_op op)
{
   if (op == BIFROST_OP_WRITE_LO)
      fprintf(fp, ".h0");
   else if (op == BIFROST_OP_WRITE_HI)
      fprintf(fp, ".h1");
}

void
bi_disasm_dest_fma(FILE *fp, struct bifrost_regs *next_regs, bool last)
{
   /* If this is the last instruction, next_regs points to the first reg entry. */
   struct bifrost_reg_ctrl ctrl = DecodeRegCtrl(fp, *next_regs, last);
   if (ctrl.slot23.slot2 >= BIFROST_OP_WRITE) {
      fprintf(fp, "r%u:t0", next_regs->reg2);
      bi_disasm_dest_mask(fp, ctrl.slot23.slot2);
   } else if (ctrl.slot23.slot3 >= BIFROST_OP_WRITE && ctrl.slot23.slot3_fma) {
      fprintf(fp, "r%u:t0", next_regs->reg3);
      bi_disasm_dest_mask(fp, ctrl.slot23.slot3);
   } else
      fprintf(fp, "t0");
}

void
bi_disasm_dest_add(FILE *fp, struct bifrost_regs *next_regs, bool last)
{
   /* If this is the last instruction, next_regs points to the first reg entry. */
   struct bifrost_reg_ctrl ctrl = DecodeRegCtrl(fp, *next_regs, last);

   if (ctrl.slot23.slot3 >= BIFROST_OP_WRITE && !ctrl.slot23.slot3_fma) {
      fprintf(fp, "r%u:t1", next_regs->reg3);
      bi_disasm_dest_mask(fp, ctrl.slot23.slot3);
   } else
      fprintf(fp, "t1");
}

static void
dump_const_imm(FILE *fp, uint32_t imm)
{
   union {
      float f;
      uint32_t i;
   } fi;
   fi.i = imm;
   fprintf(fp, "0x%08x /* %f */", imm, fi.f);
}

static void
dump_pc_imm(FILE *fp, uint64_t imm, unsigned branch_offset,
            enum bi_constmod mod, bool high32)
{
   if (mod == BI_CONSTMOD_PC_HI && !high32) {
      dump_const_imm(fp, imm);
      return;
   }

   /* 60-bit sign-extend */
   uint64_t zx64 = (imm << 4);
   int64_t sx64 = zx64;
   sx64 >>= 4;

   /* 28-bit sign extend x 2 */
   uint32_t imm32[2] = {(uint32_t)imm, (uint32_t)(imm >> 32)};
   uint32_t zx32[2] = {imm32[0] << 4, imm32[1] << 4};
   int32_t sx32[2] = {zx32[0], zx32[1]};
   sx32[0] >>= 4;
   sx32[1] >>= 4;

   int64_t offs = 0;

   switch (mod) {
   case BI_CONSTMOD_PC_LO:
      offs = sx64;
      break;
   case BI_CONSTMOD_PC_HI:
      offs = sx32[1];
      break;
   case BI_CONSTMOD_PC_LO_HI:
      offs = sx32[high32];
      break;
   default:
      unreachable("Invalid PC modifier");
   }

   assert((offs & 15) == 0);
   fprintf(fp, "clause_%" PRId64, branch_offset + (offs / 16));

   if (mod == BI_CONSTMOD_PC_LO && high32)
      fprintf(fp, " >> 32");

   /* While technically in spec, referencing the current clause as (pc +
    * 0) likely indicates an unintended infinite loop  */
   if (offs == 0)
      fprintf(fp, " /* XXX: likely an infinite loop */");
}

/* Convert an index to an embedded constant in FAU-RAM to the index of the
 * embedded constant. No, it's not in order. Yes, really. */

static unsigned
const_fau_to_idx(unsigned fau_value)
{
   unsigned map[8] = {~0, ~0, 4, 5, 0, 1, 2, 3};

   assert(map[fau_value] < 6);
   return map[fau_value];
}

static void
dump_fau_src(FILE *fp, struct bifrost_regs srcs, unsigned branch_offset,
             struct bi_constants *consts, bool high32)
{
   if (srcs.fau_idx & 0x80) {
      unsigned uniform = (srcs.fau_idx & 0x7f);
      fprintf(fp, "u%u.w%u", uniform, high32);
   } else if (srcs.fau_idx >= 0x20) {
      unsigned idx = const_fau_to_idx(srcs.fau_idx >> 4);
      uint64_t imm = consts->raw[idx];
      imm |= (srcs.fau_idx & 0xf);
      if (consts->mods[idx] != BI_CONSTMOD_NONE)
         dump_pc_imm(fp, imm, branch_offset, consts->mods[idx], high32);
      else if (high32)
         dump_const_imm(fp, imm >> 32);
      else
         dump_const_imm(fp, imm);
   } else {
      switch (srcs.fau_idx) {
      case 0:
         fprintf(fp, "#0");
         break;
      case 1:
         fprintf(fp, "lane_id");
         break;
      case 2:
         fprintf(fp, "warp_id");
         break;
      case 3:
         fprintf(fp, "core_id");
         break;
      case 4:
         fprintf(fp, "framebuffer_size");
         break;
      case 5:
         fprintf(fp, "atest_datum");
         break;
      case 6:
         fprintf(fp, "sample");
         break;
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
         fprintf(fp, "blend_descriptor_%u", (unsigned)srcs.fau_idx - 8);
         break;
      default:
         fprintf(fp, "XXX - reserved%u", (unsigned)srcs.fau_idx);
         break;
      }

      if (high32)
         fprintf(fp, ".y");
      else
         fprintf(fp, ".x");
   }
}

void
dump_src(FILE *fp, unsigned src, struct bifrost_regs srcs,
         unsigned branch_offset, struct bi_constants *consts, bool isFMA)
{
   switch (src) {
   case 0:
      fprintf(fp, "r%u", get_reg0(srcs));
      break;
   case 1:
      fprintf(fp, "r%u", get_reg1(srcs));
      break;
   case 2:
      fprintf(fp, "r%u", srcs.reg2);
      break;
   case 3:
      if (isFMA)
         fprintf(fp, "#0");
      else
         fprintf(fp, "t"); // i.e. the output of FMA this cycle
      break;
   case 4:
      dump_fau_src(fp, srcs, branch_offset, consts, false);
      break;
   case 5:
      dump_fau_src(fp, srcs, branch_offset, consts, true);
      break;
   case 6:
      fprintf(fp, "t0");
      break;
   case 7:
      fprintf(fp, "t1");
      break;
   }
}

/* Tables for decoding M0, or if M0 == 7, M1 respectively.
 *
 * XXX: It's not clear if the third entry of M1_table corresponding to (7, 2)
 * should have PC_LO_HI in the EC1 slot, or it's a weird hybrid mode? I would
 * say this needs testing but no code should ever actually use this mode.
 */

static const enum bi_constmod M1_table[7][2] = {
   {BI_CONSTMOD_NONE, BI_CONSTMOD_NONE},
   {BI_CONSTMOD_PC_LO, BI_CONSTMOD_NONE},
   {BI_CONSTMOD_PC_LO, BI_CONSTMOD_PC_LO},
   {~0, ~0},
   {BI_CONSTMOD_PC_HI, BI_CONSTMOD_NONE},
   {BI_CONSTMOD_PC_HI, BI_CONSTMOD_PC_HI},
   {BI_CONSTMOD_PC_LO, BI_CONSTMOD_NONE},
};

static const enum bi_constmod M2_table[4][2] = {
   {BI_CONSTMOD_PC_LO_HI, BI_CONSTMOD_NONE},
   {BI_CONSTMOD_PC_LO_HI, BI_CONSTMOD_PC_HI},
   {BI_CONSTMOD_PC_LO_HI, BI_CONSTMOD_PC_LO_HI},
   {BI_CONSTMOD_PC_LO_HI, BI_CONSTMOD_PC_HI},
};

static void
decode_M(enum bi_constmod *mod, unsigned M1, unsigned M2, bool single)
{
   if (M1 >= 8) {
      mod[0] = BI_CONSTMOD_NONE;

      if (!single)
         mod[1] = BI_CONSTMOD_NONE;

      return;
   } else if (M1 == 7) {
      assert(M2 < 4);
      memcpy(mod, M2_table[M2], sizeof(*mod) * (single ? 1 : 2));
   } else {
      assert(M1 != 3);
      memcpy(mod, M1_table[M1], sizeof(*mod) * (single ? 1 : 2));
   }
}

static void
dump_clause(FILE *fp, uint32_t *words, unsigned *size, unsigned offset,
            bool verbose)
{
   // State for a decoded clause
   struct bifrost_alu_inst instrs[8] = {};
   struct bi_constants consts = {};
   unsigned num_instrs = 0;
   unsigned num_consts = 0;
   uint64_t header_bits = 0;

   unsigned i;
   for (i = 0;; i++, words += 4) {
      if (verbose) {
         fprintf(fp, "# ");
         for (int j = 0; j < 4; j++)
            fprintf(fp, "%08x ", words[3 - j]); // low bit on the right
         fprintf(fp, "\n");
      }
      unsigned tag = bits(words[0], 0, 8);

      // speculatively decode some things that are common between many formats,
      // so we can share some code
      struct bifrost_alu_inst main_instr = {};
      // 20 bits
      main_instr.add_bits = bits(words[2], 2, 32 - 13);
      // 23 bits
      main_instr.fma_bits = bits(words[1], 11, 32) | bits(words[2], 0, 2)
                                                        << (32 - 11);
      // 35 bits
      main_instr.reg_bits = ((uint64_t)bits(words[1], 0, 11)) << 24 |
                            (uint64_t)bits(words[0], 8, 32);

      uint64_t const0 = bits(words[0], 8, 32) << 4 | (uint64_t)words[1] << 28 |
                        bits(words[2], 0, 4) << 60;
      uint64_t const1 = bits(words[2], 4, 32) << 4 | (uint64_t)words[3] << 32;

      /* Z-bit */
      bool stop = tag & 0x40;

      if (verbose) {
         fprintf(fp, "# tag: 0x%02x\n", tag);
      }
      if (tag & 0x80) {
         /* Format 5 or 10 */
         unsigned idx = stop ? 5 : 2;
         main_instr.add_bits |= ((tag >> 3) & 0x7) << 17;
         instrs[idx + 1] = main_instr;
         instrs[idx].add_bits = bits(words[3], 0, 17) | ((tag & 0x7) << 17);
         instrs[idx].fma_bits |= bits(words[2], 19, 32) << 10;
         consts.raw[0] = bits(words[3], 17, 32) << 4;
      } else {
         bool done = false;
         switch ((tag >> 3) & 0x7) {
         case 0x0:
            switch (tag & 0x7) {
            case 0x3:
               /* Format 1 */
               main_instr.add_bits |= bits(words[3], 29, 32) << 17;
               instrs[1] = main_instr;
               num_instrs = 2;
               done = stop;
               break;
            case 0x4:
               /* Format 3 */
               instrs[2].add_bits =
                  bits(words[3], 0, 17) | bits(words[3], 29, 32) << 17;
               instrs[2].fma_bits |= bits(words[2], 19, 32) << 10;
               consts.raw[0] = const0;
               decode_M(&consts.mods[0], bits(words[2], 4, 8),
                        bits(words[2], 8, 12), true);
               num_instrs = 3;
               num_consts = 1;
               done = stop;
               break;
            case 0x1:
            case 0x5:
               /* Format 4 */
               instrs[2].add_bits =
                  bits(words[3], 0, 17) | bits(words[3], 29, 32) << 17;
               instrs[2].fma_bits |= bits(words[2], 19, 32) << 10;
               main_instr.add_bits |= bits(words[3], 26, 29) << 17;
               instrs[3] = main_instr;
               if ((tag & 0x7) == 0x5) {
                  num_instrs = 4;
                  done = stop;
               }
               break;
            case 0x6:
               /* Format 8 */
               instrs[5].add_bits =
                  bits(words[3], 0, 17) | bits(words[3], 29, 32) << 17;
               instrs[5].fma_bits |= bits(words[2], 19, 32) << 10;
               consts.raw[0] = const0;
               decode_M(&consts.mods[0], bits(words[2], 4, 8),
                        bits(words[2], 8, 12), true);
               num_instrs = 6;
               num_consts = 1;
               done = stop;
               break;
            case 0x7:
               /* Format 9 */
               instrs[5].add_bits =
                  bits(words[3], 0, 17) | bits(words[3], 29, 32) << 17;
               instrs[5].fma_bits |= bits(words[2], 19, 32) << 10;
               main_instr.add_bits |= bits(words[3], 26, 29) << 17;
               instrs[6] = main_instr;
               num_instrs = 7;
               done = stop;
               break;
            default:
               unreachable("[INSTR_INVALID_ENC] Invalid tag bits");
            }
            break;
         case 0x2:
         case 0x3: {
            /* Format 6 or 11 */
            unsigned idx = ((tag >> 3) & 0x7) == 2 ? 4 : 7;
            main_instr.add_bits |= (tag & 0x7) << 17;
            instrs[idx] = main_instr;
            consts.raw[0] |=
               (bits(words[2], 19, 32) | ((uint64_t)words[3] << 13)) << 19;
            num_consts = 1;
            num_instrs = idx + 1;
            done = stop;
            break;
         }
         case 0x4: {
            /* Format 2 */
            unsigned idx = stop ? 4 : 1;
            main_instr.add_bits |= (tag & 0x7) << 17;
            instrs[idx] = main_instr;
            instrs[idx + 1].fma_bits |= bits(words[3], 22, 32);
            instrs[idx + 1].reg_bits =
               bits(words[2], 19, 32) | (bits(words[3], 0, 22) << (32 - 19));
            break;
         }
         case 0x1:
            /* Format 0 - followed by constants */
            num_instrs = 1;
            done = stop;
            FALLTHROUGH;
         case 0x5:
            /* Format 0 - followed by instructions */
            header_bits =
               bits(words[2], 19, 32) | ((uint64_t)words[3] << (32 - 19));
            main_instr.add_bits |= (tag & 0x7) << 17;
            instrs[0] = main_instr;
            break;
         case 0x6:
         case 0x7: {
            /* Format 12 */
            unsigned pos = tag & 0xf;

            struct {
               unsigned const_idx;
               unsigned nr_tuples;
            } pos_table[0x10] = {{0, 1}, {0, 2}, {0, 4}, {1, 3},
                                 {1, 5}, {2, 4}, {0, 7}, {1, 6},
                                 {3, 5}, {1, 8}, {2, 7}, {3, 6},
                                 {3, 8}, {4, 7}, {5, 6}, {~0, ~0}};

            ASSERTED bool valid_count = pos_table[pos].nr_tuples == num_instrs;
            assert(valid_count && "INSTR_INVALID_ENC");

            unsigned const_idx = pos_table[pos].const_idx;

            if (num_consts < const_idx + 2)
               num_consts = const_idx + 2;

            consts.raw[const_idx] = const0;
            consts.raw[const_idx + 1] = const1;

            /* Calculate M values from A, B and 4-bit
             * unsigned arithmetic. Mathematically it
             * should be (A - B) % 16 but we use this
             * alternate form to avoid sign issues */

            unsigned A1 = bits(words[2], 0, 4);
            unsigned B1 = bits(words[3], 28, 32);
            unsigned A2 = bits(words[1], 0, 4);
            unsigned B2 = bits(words[2], 28, 32);

            unsigned M1 = (16 + A1 - B1) & 0xF;
            unsigned M2 = (16 + A2 - B2) & 0xF;

            decode_M(&consts.mods[const_idx], M1, M2, false);

            done = stop;
            break;
         }
         default:
            break;
         }

         if (done)
            break;
      }
   }

   *size = i + 1;

   if (verbose) {
      fprintf(fp, "# header: %012" PRIx64 "\n", header_bits);
   }

   struct bifrost_header header;
   memcpy((char *)&header, (char *)&header_bits, sizeof(struct bifrost_header));
   dump_header(fp, header, verbose);

   fprintf(fp, "{\n");
   for (i = 0; i < num_instrs; i++) {
      struct bifrost_regs regs, next_regs;
      if (i + 1 == num_instrs) {
         memcpy((char *)&next_regs, (char *)&instrs[0].reg_bits,
                sizeof(next_regs));
      } else {
         memcpy((char *)&next_regs, (char *)&instrs[i + 1].reg_bits,
                sizeof(next_regs));
      }

      memcpy((char *)&regs, (char *)&instrs[i].reg_bits, sizeof(regs));

      if (verbose) {
         fprintf(fp, "    # regs: %016" PRIx64 "\n", instrs[i].reg_bits);
         dump_regs(fp, regs, i == 0);
      }

      bi_disasm_fma(fp, instrs[i].fma_bits, &regs, &next_regs,
                    header.staging_register, offset, &consts,
                    i + 1 == num_instrs);

      bi_disasm_add(fp, instrs[i].add_bits, &regs, &next_regs,
                    header.staging_register, offset, &consts,
                    i + 1 == num_instrs);
   }
   fprintf(fp, "}\n");

   if (verbose) {
      for (unsigned i = 0; i < num_consts; i++) {
         fprintf(fp, "# const%d: %08" PRIx64 "\n", 2 * i,
                 consts.raw[i] & 0xffffffff);
         fprintf(fp, "# const%d: %08" PRIx64 "\n", 2 * i + 1,
                 consts.raw[i] >> 32);
      }
   }

   fprintf(fp, "\n");
   return;
}

void
disassemble_bifrost(FILE *fp, uint8_t *code, size_t size, bool verbose)
{
   uint32_t *words = (uint32_t *)code;
   uint32_t *words_end = words + (size / 4);
   // used for displaying branch targets
   unsigned offset = 0;
   while (words != words_end) {
      /* Shaders have zero bytes at the end for padding; stop
       * disassembling when we hit them. */
      if (*words == 0)
         break;

      fprintf(fp, "clause_%u:\n", offset);

      unsigned size;
      dump_clause(fp, words, &size, offset, verbose);

      words += size * 4;
      offset += size;
   }
}
