/*
 * Copyright © 2015-2019 Intel Corporation
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

/** @file brw_eu_validate.c
 *
 * This file implements a pass that validates shader assembly.
 *
 * The restrictions implemented herein are intended to verify that instructions
 * in shader assembly do not violate restrictions documented in the graphics
 * programming reference manuals.
 *
 * The restrictions are difficult for humans to quickly verify due to their
 * complexity and abundance.
 *
 * It is critical that this code is thoroughly unit tested because false
 * results will lead developers astray, which is worse than having no validator
 * at all. Functional changes to this file without corresponding unit tests (in
 * test_eu_validate.cpp) will be rejected.
 */

#include <stdlib.h>
#include "brw_eu.h"

/* We're going to do lots of string concatenation, so this should help. */
struct string {
   char *str;
   size_t len;
};

static void
cat(struct string *dest, const struct string src)
{
   dest->str = realloc(dest->str, dest->len + src.len + 1);
   memcpy(dest->str + dest->len, src.str, src.len);
   dest->str[dest->len + src.len] = '\0';
   dest->len = dest->len + src.len;
}
#define CAT(dest, src) cat(&dest, (struct string){src, strlen(src)})

static bool
contains(const struct string haystack, const struct string needle)
{
   return haystack.str && memmem(haystack.str, haystack.len,
                                 needle.str, needle.len) != NULL;
}
#define CONTAINS(haystack, needle) \
   contains(haystack, (struct string){needle, strlen(needle)})

#define error(str)   "\tERROR: " str "\n"
#define ERROR_INDENT "\t       "

#define ERROR(msg) ERROR_IF(true, msg)
#define ERROR_IF(cond, msg)                             \
   do {                                                 \
      if ((cond) && !CONTAINS(error_msg, error(msg))) { \
         CAT(error_msg, error(msg));                    \
      }                                                 \
   } while(0)

#define CHECK(func, args...)                             \
   do {                                                  \
      struct string __msg = func(isa, inst, ##args); \
      if (__msg.str) {                                   \
         cat(&error_msg, __msg);                         \
         free(__msg.str);                                \
      }                                                  \
   } while (0)

#define STRIDE(stride) (stride != 0 ? 1 << ((stride) - 1) : 0)
#define WIDTH(width)   (1 << (width))

static bool
inst_is_send(const struct brw_isa_info *isa, const brw_inst *inst)
{
   switch (brw_inst_opcode(isa, inst)) {
   case BRW_OPCODE_SEND:
   case BRW_OPCODE_SENDC:
   case BRW_OPCODE_SENDS:
   case BRW_OPCODE_SENDSC:
      return true;
   default:
      return false;
   }
}

static bool
inst_is_split_send(const struct brw_isa_info *isa, const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   if (devinfo->ver >= 12) {
      return inst_is_send(isa, inst);
   } else {
      switch (brw_inst_opcode(isa, inst)) {
      case BRW_OPCODE_SENDS:
      case BRW_OPCODE_SENDSC:
         return true;
      default:
         return false;
      }
   }
}

static unsigned
signed_type(unsigned type)
{
   switch (type) {
   case BRW_REGISTER_TYPE_UD: return BRW_REGISTER_TYPE_D;
   case BRW_REGISTER_TYPE_UW: return BRW_REGISTER_TYPE_W;
   case BRW_REGISTER_TYPE_UB: return BRW_REGISTER_TYPE_B;
   case BRW_REGISTER_TYPE_UQ: return BRW_REGISTER_TYPE_Q;
   default:                   return type;
   }
}

static enum brw_reg_type
inst_dst_type(const struct brw_isa_info *isa, const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   return (devinfo->ver < 12 || !inst_is_send(isa, inst)) ?
      brw_inst_dst_type(devinfo, inst) : BRW_REGISTER_TYPE_D;
}

static bool
inst_is_raw_move(const struct brw_isa_info *isa, const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   unsigned dst_type = signed_type(inst_dst_type(isa, inst));
   unsigned src_type = signed_type(brw_inst_src0_type(devinfo, inst));

   if (brw_inst_src0_reg_file(devinfo, inst) == BRW_IMMEDIATE_VALUE) {
      /* FIXME: not strictly true */
      if (brw_inst_src0_type(devinfo, inst) == BRW_REGISTER_TYPE_VF ||
          brw_inst_src0_type(devinfo, inst) == BRW_REGISTER_TYPE_UV ||
          brw_inst_src0_type(devinfo, inst) == BRW_REGISTER_TYPE_V) {
         return false;
      }
   } else if (brw_inst_src0_negate(devinfo, inst) ||
              brw_inst_src0_abs(devinfo, inst)) {
      return false;
   }

   return brw_inst_opcode(isa, inst) == BRW_OPCODE_MOV &&
          brw_inst_saturate(devinfo, inst) == 0 &&
          dst_type == src_type;
}

static bool
dst_is_null(const struct intel_device_info *devinfo, const brw_inst *inst)
{
   return brw_inst_dst_reg_file(devinfo, inst) == BRW_ARCHITECTURE_REGISTER_FILE &&
          brw_inst_dst_da_reg_nr(devinfo, inst) == BRW_ARF_NULL;
}

static bool
src0_is_null(const struct intel_device_info *devinfo, const brw_inst *inst)
{
   return brw_inst_src0_address_mode(devinfo, inst) == BRW_ADDRESS_DIRECT &&
          brw_inst_src0_reg_file(devinfo, inst) == BRW_ARCHITECTURE_REGISTER_FILE &&
          brw_inst_src0_da_reg_nr(devinfo, inst) == BRW_ARF_NULL;
}

static bool
src1_is_null(const struct intel_device_info *devinfo, const brw_inst *inst)
{
   return brw_inst_src1_reg_file(devinfo, inst) == BRW_ARCHITECTURE_REGISTER_FILE &&
          brw_inst_src1_da_reg_nr(devinfo, inst) == BRW_ARF_NULL;
}

static bool
src0_is_acc(const struct intel_device_info *devinfo, const brw_inst *inst)
{
   return brw_inst_src0_reg_file(devinfo, inst) == BRW_ARCHITECTURE_REGISTER_FILE &&
          (brw_inst_src0_da_reg_nr(devinfo, inst) & 0xF0) == BRW_ARF_ACCUMULATOR;
}

static bool
src1_is_acc(const struct intel_device_info *devinfo, const brw_inst *inst)
{
   return brw_inst_src1_reg_file(devinfo, inst) == BRW_ARCHITECTURE_REGISTER_FILE &&
          (brw_inst_src1_da_reg_nr(devinfo, inst) & 0xF0) == BRW_ARF_ACCUMULATOR;
}

static bool
src0_has_scalar_region(const struct intel_device_info *devinfo,
                       const brw_inst *inst)
{
   return brw_inst_src0_vstride(devinfo, inst) == BRW_VERTICAL_STRIDE_0 &&
          brw_inst_src0_width(devinfo, inst) == BRW_WIDTH_1 &&
          brw_inst_src0_hstride(devinfo, inst) == BRW_HORIZONTAL_STRIDE_0;
}

static bool
src1_has_scalar_region(const struct intel_device_info *devinfo,
                       const brw_inst *inst)
{
   return brw_inst_src1_vstride(devinfo, inst) == BRW_VERTICAL_STRIDE_0 &&
          brw_inst_src1_width(devinfo, inst) == BRW_WIDTH_1 &&
          brw_inst_src1_hstride(devinfo, inst) == BRW_HORIZONTAL_STRIDE_0;
}

static struct string
invalid_values(const struct brw_isa_info *isa, const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   struct string error_msg = { .str = NULL, .len = 0 };

   switch ((enum brw_execution_size) brw_inst_exec_size(devinfo, inst)) {
   case BRW_EXECUTE_1:
   case BRW_EXECUTE_2:
   case BRW_EXECUTE_4:
   case BRW_EXECUTE_8:
   case BRW_EXECUTE_16:
   case BRW_EXECUTE_32:
      break;
   default:
      ERROR("invalid execution size");
      break;
   }

   if (error_msg.str)
      return error_msg;

   if (devinfo->ver >= 12) {
      unsigned group_size = 1 << brw_inst_exec_size(devinfo, inst);
      unsigned qtr_ctrl = brw_inst_qtr_control(devinfo, inst);
      unsigned nib_ctrl = brw_inst_nib_control(devinfo, inst);

      unsigned chan_off = (qtr_ctrl * 2 + nib_ctrl) << 2;
      ERROR_IF(chan_off % group_size != 0,
               "The execution size must be a factor of the chosen offset");
   }

   if (inst_is_send(isa, inst))
      return error_msg;

   if (num_sources == 3) {
      /* Nothing to test:
       *    No 3-src instructions on Gfx4-5
       *    No reg file bits on Gfx6-10 (align16)
       *    No invalid encodings on Gfx10-12 (align1)
       */
   } else {
      if (devinfo->ver > 6) {
         ERROR_IF(brw_inst_dst_reg_file(devinfo, inst) == MRF ||
                  (num_sources > 0 &&
                   brw_inst_src0_reg_file(devinfo, inst) == MRF) ||
                  (num_sources > 1 &&
                   brw_inst_src1_reg_file(devinfo, inst) == MRF),
                  "invalid register file encoding");
      }
   }

   if (error_msg.str)
      return error_msg;

   if (num_sources == 3) {
      if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1) {
         if (devinfo->ver >= 10) {
            ERROR_IF(brw_inst_3src_a1_dst_type (devinfo, inst) == INVALID_REG_TYPE ||
                     brw_inst_3src_a1_src0_type(devinfo, inst) == INVALID_REG_TYPE ||
                     brw_inst_3src_a1_src1_type(devinfo, inst) == INVALID_REG_TYPE ||
                     brw_inst_3src_a1_src2_type(devinfo, inst) == INVALID_REG_TYPE,
                     "invalid register type encoding");
         } else {
            ERROR("Align1 mode not allowed on Gen < 10");
         }
      } else {
         ERROR_IF(brw_inst_3src_a16_dst_type(devinfo, inst) == INVALID_REG_TYPE ||
                  brw_inst_3src_a16_src_type(devinfo, inst) == INVALID_REG_TYPE,
                  "invalid register type encoding");
      }
   } else {
      ERROR_IF(brw_inst_dst_type (devinfo, inst) == INVALID_REG_TYPE ||
               (num_sources > 0 &&
                brw_inst_src0_type(devinfo, inst) == INVALID_REG_TYPE) ||
               (num_sources > 1 &&
                brw_inst_src1_type(devinfo, inst) == INVALID_REG_TYPE),
               "invalid register type encoding");
   }

   return error_msg;
}

static struct string
sources_not_null(const struct brw_isa_info *isa,
                 const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;
   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   struct string error_msg = { .str = NULL, .len = 0 };

   /* Nothing to test. 3-src instructions can only have GRF sources, and
    * there's no bit to control the file.
    */
   if (num_sources == 3)
      return (struct string){};

   /* Nothing to test.  Split sends can only encode a file in sources that are
    * allowed to be NULL.
    */
   if (inst_is_split_send(isa, inst))
      return (struct string){};

   if (num_sources >= 1 && brw_inst_opcode(isa, inst) != BRW_OPCODE_SYNC)
      ERROR_IF(src0_is_null(devinfo, inst), "src0 is null");

   if (num_sources == 2)
      ERROR_IF(src1_is_null(devinfo, inst), "src1 is null");

   return error_msg;
}

static struct string
alignment_supported(const struct brw_isa_info *isa,
                    const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;
   struct string error_msg = { .str = NULL, .len = 0 };

   ERROR_IF(devinfo->ver >= 11 && brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_16,
            "Align16 not supported");

   return error_msg;
}

static bool
inst_uses_src_acc(const struct brw_isa_info *isa,
                  const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   /* Check instructions that use implicit accumulator sources */
   switch (brw_inst_opcode(isa, inst)) {
   case BRW_OPCODE_MAC:
   case BRW_OPCODE_MACH:
   case BRW_OPCODE_SADA2:
      return true;
   default:
      break;
   }

   /* FIXME: support 3-src instructions */
   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   assert(num_sources < 3);

   return src0_is_acc(devinfo, inst) || (num_sources > 1 && src1_is_acc(devinfo, inst));
}

static struct string
send_restrictions(const struct brw_isa_info *isa,
                  const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   struct string error_msg = { .str = NULL, .len = 0 };

   if (inst_is_split_send(isa, inst)) {
      ERROR_IF(brw_inst_send_src1_reg_file(devinfo, inst) == BRW_ARCHITECTURE_REGISTER_FILE &&
               brw_inst_send_src1_reg_nr(devinfo, inst) != BRW_ARF_NULL,
               "src1 of split send must be a GRF or NULL");

      ERROR_IF(brw_inst_eot(devinfo, inst) &&
               brw_inst_src0_da_reg_nr(devinfo, inst) < 112,
               "send with EOT must use g112-g127");
      ERROR_IF(brw_inst_eot(devinfo, inst) &&
               brw_inst_send_src1_reg_file(devinfo, inst) == BRW_GENERAL_REGISTER_FILE &&
               brw_inst_send_src1_reg_nr(devinfo, inst) < 112,
               "send with EOT must use g112-g127");

      if (brw_inst_send_src0_reg_file(devinfo, inst) == BRW_GENERAL_REGISTER_FILE &&
          brw_inst_send_src1_reg_file(devinfo, inst) == BRW_GENERAL_REGISTER_FILE) {
         /* Assume minimums if we don't know */
         unsigned mlen = 1;
         if (!brw_inst_send_sel_reg32_desc(devinfo, inst)) {
            const uint32_t desc = brw_inst_send_desc(devinfo, inst);
            mlen = brw_message_desc_mlen(devinfo, desc) / reg_unit(devinfo);
         }

         unsigned ex_mlen = 1;
         if (!brw_inst_send_sel_reg32_ex_desc(devinfo, inst)) {
            const uint32_t ex_desc = brw_inst_sends_ex_desc(devinfo, inst);
            ex_mlen = brw_message_ex_desc_ex_mlen(devinfo, ex_desc) /
                      reg_unit(devinfo);
         }
         const unsigned src0_reg_nr = brw_inst_src0_da_reg_nr(devinfo, inst);
         const unsigned src1_reg_nr = brw_inst_send_src1_reg_nr(devinfo, inst);
         ERROR_IF((src0_reg_nr <= src1_reg_nr &&
                   src1_reg_nr < src0_reg_nr + mlen) ||
                  (src1_reg_nr <= src0_reg_nr &&
                   src0_reg_nr < src1_reg_nr + ex_mlen),
                   "split send payloads must not overlap");
      }
   } else if (inst_is_send(isa, inst)) {
      ERROR_IF(brw_inst_src0_address_mode(devinfo, inst) != BRW_ADDRESS_DIRECT,
               "send must use direct addressing");

      if (devinfo->ver >= 7) {
         ERROR_IF(brw_inst_send_src0_reg_file(devinfo, inst) != BRW_GENERAL_REGISTER_FILE,
                  "send from non-GRF");
         ERROR_IF(brw_inst_eot(devinfo, inst) &&
                  brw_inst_src0_da_reg_nr(devinfo, inst) < 112,
                  "send with EOT must use g112-g127");
      }

      if (devinfo->ver >= 8) {
         ERROR_IF(!dst_is_null(devinfo, inst) &&
                  (brw_inst_dst_da_reg_nr(devinfo, inst) +
                   brw_inst_rlen(devinfo, inst) > 127) &&
                  (brw_inst_src0_da_reg_nr(devinfo, inst) +
                   brw_inst_mlen(devinfo, inst) >
                   brw_inst_dst_da_reg_nr(devinfo, inst)),
                  "r127 must not be used for return address when there is "
                  "a src and dest overlap");
      }
   }

   return error_msg;
}

static bool
is_unsupported_inst(const struct brw_isa_info *isa,
                    const brw_inst *inst)
{
   return brw_inst_opcode(isa, inst) == BRW_OPCODE_ILLEGAL;
}

/**
 * Returns whether a combination of two types would qualify as mixed float
 * operation mode
 */
static inline bool
types_are_mixed_float(enum brw_reg_type t0, enum brw_reg_type t1)
{
   return (t0 == BRW_REGISTER_TYPE_F && t1 == BRW_REGISTER_TYPE_HF) ||
          (t1 == BRW_REGISTER_TYPE_F && t0 == BRW_REGISTER_TYPE_HF);
}

static enum brw_reg_type
execution_type_for_type(enum brw_reg_type type)
{
   switch (type) {
   case BRW_REGISTER_TYPE_NF:
   case BRW_REGISTER_TYPE_DF:
   case BRW_REGISTER_TYPE_F:
   case BRW_REGISTER_TYPE_HF:
      return type;

   case BRW_REGISTER_TYPE_VF:
      return BRW_REGISTER_TYPE_F;

   case BRW_REGISTER_TYPE_Q:
   case BRW_REGISTER_TYPE_UQ:
      return BRW_REGISTER_TYPE_Q;

   case BRW_REGISTER_TYPE_D:
   case BRW_REGISTER_TYPE_UD:
      return BRW_REGISTER_TYPE_D;

   case BRW_REGISTER_TYPE_W:
   case BRW_REGISTER_TYPE_UW:
   case BRW_REGISTER_TYPE_B:
   case BRW_REGISTER_TYPE_UB:
   case BRW_REGISTER_TYPE_V:
   case BRW_REGISTER_TYPE_UV:
      return BRW_REGISTER_TYPE_W;
   }
   unreachable("not reached");
}

/**
 * Returns the execution type of an instruction \p inst
 */
static enum brw_reg_type
execution_type(const struct brw_isa_info *isa, const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   enum brw_reg_type src0_exec_type, src1_exec_type;

   /* Execution data type is independent of destination data type, except in
    * mixed F/HF instructions.
    */
   enum brw_reg_type dst_exec_type = inst_dst_type(isa, inst);

   src0_exec_type = execution_type_for_type(brw_inst_src0_type(devinfo, inst));
   if (num_sources == 1) {
      if (src0_exec_type == BRW_REGISTER_TYPE_HF)
         return dst_exec_type;
      return src0_exec_type;
   }

   src1_exec_type = execution_type_for_type(brw_inst_src1_type(devinfo, inst));
   if (types_are_mixed_float(src0_exec_type, src1_exec_type) ||
       types_are_mixed_float(src0_exec_type, dst_exec_type) ||
       types_are_mixed_float(src1_exec_type, dst_exec_type)) {
      return BRW_REGISTER_TYPE_F;
   }

   if (src0_exec_type == src1_exec_type)
      return src0_exec_type;

   if (src0_exec_type == BRW_REGISTER_TYPE_NF ||
       src1_exec_type == BRW_REGISTER_TYPE_NF)
      return BRW_REGISTER_TYPE_NF;

   /* Mixed operand types where one is float is float on Gen < 6
    * (and not allowed on later platforms)
    */
   if (devinfo->ver < 6 &&
       (src0_exec_type == BRW_REGISTER_TYPE_F ||
        src1_exec_type == BRW_REGISTER_TYPE_F))
      return BRW_REGISTER_TYPE_F;

   if (src0_exec_type == BRW_REGISTER_TYPE_Q ||
       src1_exec_type == BRW_REGISTER_TYPE_Q)
      return BRW_REGISTER_TYPE_Q;

   if (src0_exec_type == BRW_REGISTER_TYPE_D ||
       src1_exec_type == BRW_REGISTER_TYPE_D)
      return BRW_REGISTER_TYPE_D;

   if (src0_exec_type == BRW_REGISTER_TYPE_W ||
       src1_exec_type == BRW_REGISTER_TYPE_W)
      return BRW_REGISTER_TYPE_W;

   if (src0_exec_type == BRW_REGISTER_TYPE_DF ||
       src1_exec_type == BRW_REGISTER_TYPE_DF)
      return BRW_REGISTER_TYPE_DF;

   unreachable("not reached");
}

/**
 * Returns whether a region is packed
 *
 * A region is packed if its elements are adjacent in memory, with no
 * intervening space, no overlap, and no replicated values.
 */
static bool
is_packed(unsigned vstride, unsigned width, unsigned hstride)
{
   if (vstride == width) {
      if (vstride == 1) {
         return hstride == 0;
      } else {
         return hstride == 1;
      }
   }

   return false;
}

/**
 * Returns whether a region is linear
 *
 * A region is linear if its elements do not overlap and are not replicated.
 * Unlike a packed region, intervening space (i.e. strided values) is allowed.
 */
static bool
is_linear(unsigned vstride, unsigned width, unsigned hstride)
{
   return vstride == width * hstride ||
          (hstride == 0 && width == 1);
}

/**
 * Returns whether an instruction is an explicit or implicit conversion
 * to/from half-float.
 */
static bool
is_half_float_conversion(const struct brw_isa_info *isa,
                         const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   enum brw_reg_type dst_type = brw_inst_dst_type(devinfo, inst);

   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   enum brw_reg_type src0_type = brw_inst_src0_type(devinfo, inst);

   if (dst_type != src0_type &&
       (dst_type == BRW_REGISTER_TYPE_HF || src0_type == BRW_REGISTER_TYPE_HF)) {
      return true;
   } else if (num_sources > 1) {
      enum brw_reg_type src1_type = brw_inst_src1_type(devinfo, inst);
      return dst_type != src1_type &&
            (dst_type == BRW_REGISTER_TYPE_HF ||
             src1_type == BRW_REGISTER_TYPE_HF);
   }

   return false;
}

/*
 * Returns whether an instruction is using mixed float operation mode
 */
static bool
is_mixed_float(const struct brw_isa_info *isa, const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   if (devinfo->ver < 8)
      return false;

   if (inst_is_send(isa, inst))
      return false;

   unsigned opcode = brw_inst_opcode(isa, inst);
   const struct opcode_desc *desc = brw_opcode_desc(isa, opcode);
   if (desc->ndst == 0)
      return false;

   /* FIXME: support 3-src instructions */
   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   assert(num_sources < 3);

   enum brw_reg_type dst_type = brw_inst_dst_type(devinfo, inst);
   enum brw_reg_type src0_type = brw_inst_src0_type(devinfo, inst);

   if (num_sources == 1)
      return types_are_mixed_float(src0_type, dst_type);

   enum brw_reg_type src1_type = brw_inst_src1_type(devinfo, inst);

   return types_are_mixed_float(src0_type, src1_type) ||
          types_are_mixed_float(src0_type, dst_type) ||
          types_are_mixed_float(src1_type, dst_type);
}

/**
 * Returns whether an instruction is an explicit or implicit conversion
 * to/from byte.
 */
static bool
is_byte_conversion(const struct brw_isa_info *isa,
                   const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   enum brw_reg_type dst_type = brw_inst_dst_type(devinfo, inst);

   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   enum brw_reg_type src0_type = brw_inst_src0_type(devinfo, inst);

   if (dst_type != src0_type &&
       (type_sz(dst_type) == 1 || type_sz(src0_type) == 1)) {
      return true;
   } else if (num_sources > 1) {
      enum brw_reg_type src1_type = brw_inst_src1_type(devinfo, inst);
      return dst_type != src1_type &&
            (type_sz(dst_type) == 1 || type_sz(src1_type) == 1);
   }

   return false;
}

/**
 * Checks restrictions listed in "General Restrictions Based on Operand Types"
 * in the "Register Region Restrictions" section.
 */
static struct string
general_restrictions_based_on_operand_types(const struct brw_isa_info *isa,
                                            const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   const struct opcode_desc *desc =
      brw_opcode_desc(isa, brw_inst_opcode(isa, inst));
   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   unsigned exec_size = 1 << brw_inst_exec_size(devinfo, inst);
   struct string error_msg = { .str = NULL, .len = 0 };

   if (inst_is_send(isa, inst))
      return error_msg;

   if (devinfo->ver >= 11) {
      /* A register type of B or UB for DPAS actually means 4 bytes packed into
       * a D or UD, so it is allowed.
       */
      if (num_sources == 3 && brw_inst_opcode(isa, inst) != BRW_OPCODE_DPAS) {
         ERROR_IF(brw_reg_type_to_size(brw_inst_3src_a1_src1_type(devinfo, inst)) == 1 ||
                  brw_reg_type_to_size(brw_inst_3src_a1_src2_type(devinfo, inst)) == 1,
                  "Byte data type is not supported for src1/2 register regioning. This includes "
                  "byte broadcast as well.");
      }
      if (num_sources == 2) {
         ERROR_IF(brw_reg_type_to_size(brw_inst_src1_type(devinfo, inst)) == 1,
                  "Byte data type is not supported for src1 register regioning. This includes "
                  "byte broadcast as well.");
      }
   }

   enum brw_reg_type dst_type;

   if (num_sources == 3) {
      if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1)
         dst_type = brw_inst_3src_a1_dst_type(devinfo, inst);
      else
         dst_type = brw_inst_3src_a16_dst_type(devinfo, inst);
   } else {
      dst_type = inst_dst_type(isa, inst);
   }

   ERROR_IF(dst_type == BRW_REGISTER_TYPE_DF &&
            !devinfo->has_64bit_float,
            "64-bit float destination, but platform does not support it");

   ERROR_IF((dst_type == BRW_REGISTER_TYPE_Q ||
             dst_type == BRW_REGISTER_TYPE_UQ) &&
            !devinfo->has_64bit_int,
            "64-bit int destination, but platform does not support it");

   for (unsigned s = 0; s < num_sources; s++) {
      enum brw_reg_type src_type;
      if (num_sources == 3) {
         if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1) {
            switch (s) {
            case 0: src_type = brw_inst_3src_a1_src0_type(devinfo, inst); break;
            case 1: src_type = brw_inst_3src_a1_src1_type(devinfo, inst); break;
            case 2: src_type = brw_inst_3src_a1_src2_type(devinfo, inst); break;
            default: unreachable("invalid src");
            }
         } else {
            src_type = brw_inst_3src_a16_src_type(devinfo, inst);
         }
      } else {
         switch (s) {
         case 0: src_type = brw_inst_src0_type(devinfo, inst); break;
         case 1: src_type = brw_inst_src1_type(devinfo, inst); break;
         default: unreachable("invalid src");
         }
      }

      ERROR_IF(src_type == BRW_REGISTER_TYPE_DF &&
               !devinfo->has_64bit_float,
               "64-bit float source, but platform does not support it");

      ERROR_IF((src_type == BRW_REGISTER_TYPE_Q ||
                src_type == BRW_REGISTER_TYPE_UQ) &&
               !devinfo->has_64bit_int,
               "64-bit int source, but platform does not support it");
   }

   if (num_sources == 3)
      return error_msg;

   if (exec_size == 1)
      return error_msg;

   if (desc->ndst == 0)
      return error_msg;

   /* The PRMs say:
    *
    *    Where n is the largest element size in bytes for any source or
    *    destination operand type, ExecSize * n must be <= 64.
    *
    * But we do not attempt to enforce it, because it is implied by other
    * rules:
    *
    *    - that the destination stride must match the execution data type
    *    - sources may not span more than two adjacent GRF registers
    *    - destination may not span more than two adjacent GRF registers
    *
    * In fact, checking it would weaken testing of the other rules.
    */

   unsigned dst_stride = STRIDE(brw_inst_dst_hstride(devinfo, inst));
   bool dst_type_is_byte =
      inst_dst_type(isa, inst) == BRW_REGISTER_TYPE_B ||
      inst_dst_type(isa, inst) == BRW_REGISTER_TYPE_UB;

   if (dst_type_is_byte) {
      if (is_packed(exec_size * dst_stride, exec_size, dst_stride)) {
         if (!inst_is_raw_move(isa, inst))
            ERROR("Only raw MOV supports a packed-byte destination");
         return error_msg;
      }
   }

   unsigned exec_type = execution_type(isa, inst);
   unsigned exec_type_size = brw_reg_type_to_size(exec_type);
   unsigned dst_type_size = brw_reg_type_to_size(dst_type);

   /* On IVB/BYT, region parameters and execution size for DF are in terms of
    * 32-bit elements, so they are doubled. For evaluating the validity of an
    * instruction, we halve them.
    */
   if (devinfo->verx10 == 70 &&
       exec_type_size == 8 && dst_type_size == 4)
      dst_type_size = 8;

   if (is_byte_conversion(isa, inst)) {
      /* From the BDW+ PRM, Volume 2a, Command Reference, Instructions - MOV:
       *
       *    "There is no direct conversion from B/UB to DF or DF to B/UB.
       *     There is no direct conversion from B/UB to Q/UQ or Q/UQ to B/UB."
       *
       * Even if these restrictions are listed for the MOV instruction, we
       * validate this more generally, since there is the possibility
       * of implicit conversions from other instructions.
       */
      enum brw_reg_type src0_type = brw_inst_src0_type(devinfo, inst);
      enum brw_reg_type src1_type = num_sources > 1 ?
                                    brw_inst_src1_type(devinfo, inst) : 0;

      ERROR_IF(type_sz(dst_type) == 1 &&
               (type_sz(src0_type) == 8 ||
                (num_sources > 1 && type_sz(src1_type) == 8)),
               "There are no direct conversions between 64-bit types and B/UB");

      ERROR_IF(type_sz(dst_type) == 8 &&
               (type_sz(src0_type) == 1 ||
                (num_sources > 1 && type_sz(src1_type) == 1)),
               "There are no direct conversions between 64-bit types and B/UB");
   }

   if (is_half_float_conversion(isa, inst)) {
      /**
       * A helper to validate used in the validation of the following restriction
       * from the BDW+ PRM, Volume 2a, Command Reference, Instructions - MOV:
       *
       *    "There is no direct conversion from HF to DF or DF to HF.
       *     There is no direct conversion from HF to Q/UQ or Q/UQ to HF."
       *
       * Even if these restrictions are listed for the MOV instruction, we
       * validate this more generally, since there is the possibility
       * of implicit conversions from other instructions, such us implicit
       * conversion from integer to HF with the ADD instruction in SKL+.
       */
      enum brw_reg_type src0_type = brw_inst_src0_type(devinfo, inst);
      enum brw_reg_type src1_type = num_sources > 1 ?
                                    brw_inst_src1_type(devinfo, inst) : 0;
      ERROR_IF(dst_type == BRW_REGISTER_TYPE_HF &&
               (type_sz(src0_type) == 8 ||
                (num_sources > 1 && type_sz(src1_type) == 8)),
               "There are no direct conversions between 64-bit types and HF");

      ERROR_IF(type_sz(dst_type) == 8 &&
               (src0_type == BRW_REGISTER_TYPE_HF ||
                (num_sources > 1 && src1_type == BRW_REGISTER_TYPE_HF)),
               "There are no direct conversions between 64-bit types and HF");

      /* From the BDW+ PRM:
       *
       *   "Conversion between Integer and HF (Half Float) must be
       *    DWord-aligned and strided by a DWord on the destination."
       *
       * Also, the above restrictions seems to be expanded on CHV and SKL+ by:
       *
       *   "There is a relaxed alignment rule for word destinations. When
       *    the destination type is word (UW, W, HF), destination data types
       *    can be aligned to either the lowest word or the second lowest
       *    word of the execution channel. This means the destination data
       *    words can be either all in the even word locations or all in the
       *    odd word locations."
       *
       * We do not implement the second rule as is though, since empirical
       * testing shows inconsistencies:
       *   - It suggests that packed 16-bit is not allowed, which is not true.
       *   - It suggests that conversions from Q/DF to W (which need to be
       *     64-bit aligned on the destination) are not possible, which is
       *     not true.
       *
       * So from this rule we only validate the implication that conversions
       * from F to HF need to be DWord strided (except in Align1 mixed
       * float mode where packed fp16 destination is allowed so long as the
       * destination is oword-aligned).
       *
       * Finally, we only validate this for Align1 because Align16 always
       * requires packed destinations, so these restrictions can't possibly
       * apply to Align16 mode.
       */
      if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1) {
         if ((dst_type == BRW_REGISTER_TYPE_HF &&
              (brw_reg_type_is_integer(src0_type) ||
               (num_sources > 1 && brw_reg_type_is_integer(src1_type)))) ||
             (brw_reg_type_is_integer(dst_type) &&
              (src0_type == BRW_REGISTER_TYPE_HF ||
               (num_sources > 1 && src1_type == BRW_REGISTER_TYPE_HF)))) {
            ERROR_IF(dst_stride * dst_type_size != 4,
                     "Conversions between integer and half-float must be "
                     "strided by a DWord on the destination");

            unsigned subreg = brw_inst_dst_da1_subreg_nr(devinfo, inst);
            ERROR_IF(subreg % 4 != 0,
                     "Conversions between integer and half-float must be "
                     "aligned to a DWord on the destination");
         } else if ((devinfo->platform == INTEL_PLATFORM_CHV ||
                     devinfo->ver >= 9) &&
                    dst_type == BRW_REGISTER_TYPE_HF) {
            unsigned subreg = brw_inst_dst_da1_subreg_nr(devinfo, inst);
            ERROR_IF(dst_stride != 2 &&
                     !(is_mixed_float(isa, inst) &&
                       dst_stride == 1 && subreg % 16 == 0),
                     "Conversions to HF must have either all words in even "
                     "word locations or all words in odd word locations or "
                     "be mixed-float with Oword-aligned packed destination");
         }
      }
   }

   /* There are special regioning rules for mixed-float mode in CHV and SKL that
    * override the general rule for the ratio of sizes of the destination type
    * and the execution type. We will add validation for those in a later patch.
    */
   bool validate_dst_size_and_exec_size_ratio =
      !is_mixed_float(isa, inst) ||
      !(devinfo->platform == INTEL_PLATFORM_CHV || devinfo->ver >= 9);

   if (validate_dst_size_and_exec_size_ratio &&
       exec_type_size > dst_type_size) {
      if (!(dst_type_is_byte && inst_is_raw_move(isa, inst))) {
         ERROR_IF(dst_stride * dst_type_size != exec_type_size,
                  "Destination stride must be equal to the ratio of the sizes "
                  "of the execution data type to the destination type");
      }

      unsigned subreg = brw_inst_dst_da1_subreg_nr(devinfo, inst);

      if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1 &&
          brw_inst_dst_address_mode(devinfo, inst) == BRW_ADDRESS_DIRECT) {
         /* The i965 PRM says:
          *
          *    Implementation Restriction: The relaxed alignment rule for byte
          *    destination (#10.5) is not supported.
          */
         if (devinfo->verx10 >= 45 && dst_type_is_byte) {
            ERROR_IF(subreg % exec_type_size != 0 &&
                     subreg % exec_type_size != 1,
                     "Destination subreg must be aligned to the size of the "
                     "execution data type (or to the next lowest byte for byte "
                     "destinations)");
         } else {
            ERROR_IF(subreg % exec_type_size != 0,
                     "Destination subreg must be aligned to the size of the "
                     "execution data type");
         }
      }
   }

   return error_msg;
}

/**
 * Checks restrictions listed in "General Restrictions on Regioning Parameters"
 * in the "Register Region Restrictions" section.
 */
static struct string
general_restrictions_on_region_parameters(const struct brw_isa_info *isa,
                                          const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   const struct opcode_desc *desc =
      brw_opcode_desc(isa, brw_inst_opcode(isa, inst));
   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   unsigned exec_size = 1 << brw_inst_exec_size(devinfo, inst);
   struct string error_msg = { .str = NULL, .len = 0 };

   if (num_sources == 3)
      return (struct string){};

   /* Split sends don't have the bits in the instruction to encode regions so
    * there's nothing to check.
    */
   if (inst_is_split_send(isa, inst))
      return (struct string){};

   if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_16) {
      if (desc->ndst != 0 && !dst_is_null(devinfo, inst))
         ERROR_IF(brw_inst_dst_hstride(devinfo, inst) != BRW_HORIZONTAL_STRIDE_1,
                  "Destination Horizontal Stride must be 1");

      if (num_sources >= 1) {
         if (devinfo->verx10 >= 75) {
            ERROR_IF(brw_inst_src0_reg_file(devinfo, inst) != BRW_IMMEDIATE_VALUE &&
                     brw_inst_src0_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_0 &&
                     brw_inst_src0_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_2 &&
                     brw_inst_src0_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_4,
                     "In Align16 mode, only VertStride of 0, 2, or 4 is allowed");
         } else {
            ERROR_IF(brw_inst_src0_reg_file(devinfo, inst) != BRW_IMMEDIATE_VALUE &&
                     brw_inst_src0_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_0 &&
                     brw_inst_src0_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_4,
                     "In Align16 mode, only VertStride of 0 or 4 is allowed");
         }
      }

      if (num_sources == 2) {
         if (devinfo->verx10 >= 75) {
            ERROR_IF(brw_inst_src1_reg_file(devinfo, inst) != BRW_IMMEDIATE_VALUE &&
                     brw_inst_src1_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_0 &&
                     brw_inst_src1_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_2 &&
                     brw_inst_src1_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_4,
                     "In Align16 mode, only VertStride of 0, 2, or 4 is allowed");
         } else {
            ERROR_IF(brw_inst_src1_reg_file(devinfo, inst) != BRW_IMMEDIATE_VALUE &&
                     brw_inst_src1_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_0 &&
                     brw_inst_src1_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_4,
                     "In Align16 mode, only VertStride of 0 or 4 is allowed");
         }
      }

      return error_msg;
   }

   for (unsigned i = 0; i < num_sources; i++) {
      unsigned vstride, width, hstride, element_size, subreg;
      enum brw_reg_type type;

#define DO_SRC(n)                                                              \
      if (brw_inst_src ## n ## _reg_file(devinfo, inst) ==                     \
          BRW_IMMEDIATE_VALUE)                                                 \
         continue;                                                             \
                                                                               \
      vstride = STRIDE(brw_inst_src ## n ## _vstride(devinfo, inst));          \
      width = WIDTH(brw_inst_src ## n ## _width(devinfo, inst));               \
      hstride = STRIDE(brw_inst_src ## n ## _hstride(devinfo, inst));          \
      type = brw_inst_src ## n ## _type(devinfo, inst);                        \
      element_size = brw_reg_type_to_size(type);                               \
      subreg = brw_inst_src ## n ## _da1_subreg_nr(devinfo, inst)

      if (i == 0) {
         DO_SRC(0);
      } else {
         DO_SRC(1);
      }
#undef DO_SRC

      /* On IVB/BYT, region parameters and execution size for DF are in terms of
       * 32-bit elements, so they are doubled. For evaluating the validity of an
       * instruction, we halve them.
       */
      if (devinfo->verx10 == 70 &&
          element_size == 8)
         element_size = 4;

      /* ExecSize must be greater than or equal to Width. */
      ERROR_IF(exec_size < width, "ExecSize must be greater than or equal "
                                  "to Width");

      /* If ExecSize = Width and HorzStride ≠ 0,
       * VertStride must be set to Width * HorzStride.
       */
      if (exec_size == width && hstride != 0) {
         ERROR_IF(vstride != width * hstride,
                  "If ExecSize = Width and HorzStride ≠ 0, "
                  "VertStride must be set to Width * HorzStride");
      }

      /* If Width = 1, HorzStride must be 0 regardless of the values of
       * ExecSize and VertStride.
       */
      if (width == 1) {
         ERROR_IF(hstride != 0,
                  "If Width = 1, HorzStride must be 0 regardless "
                  "of the values of ExecSize and VertStride");
      }

      /* If ExecSize = Width = 1, both VertStride and HorzStride must be 0. */
      if (exec_size == 1 && width == 1) {
         ERROR_IF(vstride != 0 || hstride != 0,
                  "If ExecSize = Width = 1, both VertStride "
                  "and HorzStride must be 0");
      }

      /* If VertStride = HorzStride = 0, Width must be 1 regardless of the
       * value of ExecSize.
       */
      if (vstride == 0 && hstride == 0) {
         ERROR_IF(width != 1,
                  "If VertStride = HorzStride = 0, Width must be "
                  "1 regardless of the value of ExecSize");
      }

      /* VertStride must be used to cross GRF register boundaries. This rule
       * implies that elements within a 'Width' cannot cross GRF boundaries.
       */
      const uint64_t mask = (1ULL << element_size) - 1;
      unsigned rowbase = subreg;

      for (int y = 0; y < exec_size / width; y++) {
         uint64_t access_mask = 0;
         unsigned offset = rowbase;

         for (int x = 0; x < width; x++) {
            access_mask |= mask << (offset % 64);
            offset += hstride * element_size;
         }

         rowbase += vstride * element_size;

         if ((uint32_t)access_mask != 0 && (access_mask >> 32) != 0) {
            ERROR("VertStride must be used to cross GRF register boundaries");
            break;
         }
      }
   }

   /* Dst.HorzStride must not be 0. */
   if (desc->ndst != 0 && !dst_is_null(devinfo, inst)) {
      ERROR_IF(brw_inst_dst_hstride(devinfo, inst) == BRW_HORIZONTAL_STRIDE_0,
               "Destination Horizontal Stride must not be 0");
   }

   return error_msg;
}

static struct string
special_restrictions_for_mixed_float_mode(const struct brw_isa_info *isa,
                                          const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   struct string error_msg = { .str = NULL, .len = 0 };

   const unsigned opcode = brw_inst_opcode(isa, inst);
   const unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   if (num_sources >= 3)
      return error_msg;

   if (!is_mixed_float(isa, inst))
      return error_msg;

   unsigned exec_size = 1 << brw_inst_exec_size(devinfo, inst);
   bool is_align16 = brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_16;

   enum brw_reg_type src0_type = brw_inst_src0_type(devinfo, inst);
   enum brw_reg_type src1_type = num_sources > 1 ?
                                 brw_inst_src1_type(devinfo, inst) : 0;
   enum brw_reg_type dst_type = brw_inst_dst_type(devinfo, inst);

   unsigned dst_stride = STRIDE(brw_inst_dst_hstride(devinfo, inst));
   bool dst_is_packed = is_packed(exec_size * dst_stride, exec_size, dst_stride);

   /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
    * Float Operations:
    *
    *    "Indirect addressing on source is not supported when source and
    *     destination data types are mixed float."
    */
   ERROR_IF(brw_inst_src0_address_mode(devinfo, inst) != BRW_ADDRESS_DIRECT ||
            (num_sources > 1 &&
             brw_inst_src1_address_mode(devinfo, inst) != BRW_ADDRESS_DIRECT),
            "Indirect addressing on source is not supported when source and "
            "destination data types are mixed float");

   /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
    * Float Operations:
    *
    *    "No SIMD16 in mixed mode when destination is f32. Instruction
    *     execution size must be no more than 8."
    */
   ERROR_IF(exec_size > 8 && dst_type == BRW_REGISTER_TYPE_F,
            "Mixed float mode with 32-bit float destination is limited "
            "to SIMD8");

   if (is_align16) {
      /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
       * Float Operations:
       *
       *   "In Align16 mode, when half float and float data types are mixed
       *    between source operands OR between source and destination operands,
       *    the register content are assumed to be packed."
       *
       * Since Align16 doesn't have a concept of horizontal stride (or width),
       * it means that vertical stride must always be 4, since 0 and 2 would
       * lead to replicated data, and any other value is disallowed in Align16.
       */
      ERROR_IF(brw_inst_src0_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_4,
               "Align16 mixed float mode assumes packed data (vstride must be 4");

      ERROR_IF(num_sources >= 2 &&
               brw_inst_src1_vstride(devinfo, inst) != BRW_VERTICAL_STRIDE_4,
               "Align16 mixed float mode assumes packed data (vstride must be 4");

      /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
       * Float Operations:
       *
       *   "For Align16 mixed mode, both input and output packed f16 data
       *    must be oword aligned, no oword crossing in packed f16."
       *
       * The previous rule requires that Align16 operands are always packed,
       * and since there is only one bit for Align16 subnr, which represents
       * offsets 0B and 16B, this rule is always enforced and we don't need to
       * validate it.
       */

      /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
       * Float Operations:
       *
       *    "No SIMD16 in mixed mode when destination is packed f16 for both
       *     Align1 and Align16."
       *
       * And:
       *
       *   "In Align16 mode, when half float and float data types are mixed
       *    between source operands OR between source and destination operands,
       *    the register content are assumed to be packed."
       *
       * Which implies that SIMD16 is not available in Align16. This is further
       * confirmed by:
       *
       *    "For Align16 mixed mode, both input and output packed f16 data
       *     must be oword aligned, no oword crossing in packed f16"
       *
       * Since oword-aligned packed f16 data would cross oword boundaries when
       * the execution size is larger than 8.
       */
      ERROR_IF(exec_size > 8, "Align16 mixed float mode is limited to SIMD8");

      /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
       * Float Operations:
       *
       *    "No accumulator read access for Align16 mixed float."
       */
      ERROR_IF(inst_uses_src_acc(isa, inst),
               "No accumulator read access for Align16 mixed float");
   } else {
      assert(!is_align16);

      /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
       * Float Operations:
       *
       *    "No SIMD16 in mixed mode when destination is packed f16 for both
       *     Align1 and Align16."
       */
      ERROR_IF(exec_size > 8 && dst_is_packed &&
               dst_type == BRW_REGISTER_TYPE_HF,
               "Align1 mixed float mode is limited to SIMD8 when destination "
               "is packed half-float");

      /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
       * Float Operations:
       *
       *    "Math operations for mixed mode:
       *     - In Align1, f16 inputs need to be strided"
       */
      if (opcode == BRW_OPCODE_MATH) {
         if (src0_type == BRW_REGISTER_TYPE_HF) {
            ERROR_IF(STRIDE(brw_inst_src0_hstride(devinfo, inst)) <= 1,
                     "Align1 mixed mode math needs strided half-float inputs");
         }

         if (num_sources >= 2 && src1_type == BRW_REGISTER_TYPE_HF) {
            ERROR_IF(STRIDE(brw_inst_src1_hstride(devinfo, inst)) <= 1,
                     "Align1 mixed mode math needs strided half-float inputs");
         }
      }

      if (dst_type == BRW_REGISTER_TYPE_HF && dst_stride == 1) {
         /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
          * Float Operations:
          *
          *    "In Align1, destination stride can be smaller than execution
          *     type. When destination is stride of 1, 16 bit packed data is
          *     updated on the destination. However, output packed f16 data
          *     must be oword aligned, no oword crossing in packed f16."
          *
          * The requirement of not crossing oword boundaries for 16-bit oword
          * aligned data means that execution size is limited to 8.
          */
         unsigned subreg;
         if (brw_inst_dst_address_mode(devinfo, inst) == BRW_ADDRESS_DIRECT)
            subreg = brw_inst_dst_da1_subreg_nr(devinfo, inst);
         else
            subreg = brw_inst_dst_ia_subreg_nr(devinfo, inst);
         ERROR_IF(subreg % 16 != 0,
                  "Align1 mixed mode packed half-float output must be "
                  "oword aligned");
         ERROR_IF(exec_size > 8,
                  "Align1 mixed mode packed half-float output must not "
                  "cross oword boundaries (max exec size is 8)");

         /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
          * Float Operations:
          *
          *    "When source is float or half float from accumulator register and
          *     destination is half float with a stride of 1, the source must
          *     register aligned. i.e., source must have offset zero."
          *
          * Align16 mixed float mode doesn't allow accumulator access on sources,
          * so we only need to check this for Align1.
          */
         if (src0_is_acc(devinfo, inst) &&
             (src0_type == BRW_REGISTER_TYPE_F ||
              src0_type == BRW_REGISTER_TYPE_HF)) {
            ERROR_IF(brw_inst_src0_da1_subreg_nr(devinfo, inst) != 0,
                     "Mixed float mode requires register-aligned accumulator "
                     "source reads when destination is packed half-float");

         }

         if (num_sources > 1 &&
             src1_is_acc(devinfo, inst) &&
             (src1_type == BRW_REGISTER_TYPE_F ||
              src1_type == BRW_REGISTER_TYPE_HF)) {
            ERROR_IF(brw_inst_src1_da1_subreg_nr(devinfo, inst) != 0,
                     "Mixed float mode requires register-aligned accumulator "
                     "source reads when destination is packed half-float");
         }
      }

      /* From the SKL PRM, Special Restrictions for Handling Mixed Mode
       * Float Operations:
       *
       *    "No swizzle is allowed when an accumulator is used as an implicit
       *     source or an explicit source in an instruction. i.e. when
       *     destination is half float with an implicit accumulator source,
       *     destination stride needs to be 2."
       *
       * FIXME: it is not quite clear what the first sentence actually means
       *        or its link to the implication described after it, so we only
       *        validate the explicit implication, which is clearly described.
       */
      if (dst_type == BRW_REGISTER_TYPE_HF &&
          inst_uses_src_acc(isa, inst)) {
         ERROR_IF(dst_stride != 2,
                  "Mixed float mode with implicit/explicit accumulator "
                  "source and half-float destination requires a stride "
                  "of 2 on the destination");
      }
   }

   return error_msg;
}

/**
 * Creates an \p access_mask for an \p exec_size, \p element_size, and a region
 *
 * An \p access_mask is a 32-element array of uint64_t, where each uint64_t is
 * a bitmask of bytes accessed by the region.
 *
 * For instance the access mask of the source gX.1<4,2,2>F in an exec_size = 4
 * instruction would be
 *
 *    access_mask[0] = 0x00000000000000F0
 *    access_mask[1] = 0x000000000000F000
 *    access_mask[2] = 0x0000000000F00000
 *    access_mask[3] = 0x00000000F0000000
 *    access_mask[4-31] = 0
 *
 * because the first execution channel accesses bytes 7-4 and the second
 * execution channel accesses bytes 15-12, etc.
 */
static void
align1_access_mask(uint64_t access_mask[static 32],
                   unsigned exec_size, unsigned element_size, unsigned subreg,
                   unsigned vstride, unsigned width, unsigned hstride)
{
   const uint64_t mask = (1ULL << element_size) - 1;
   unsigned rowbase = subreg;
   unsigned element = 0;

   for (int y = 0; y < exec_size / width; y++) {
      unsigned offset = rowbase;

      for (int x = 0; x < width; x++) {
         access_mask[element++] = mask << (offset % 64);
         offset += hstride * element_size;
      }

      rowbase += vstride * element_size;
   }

   assert(element == 0 || element == exec_size);
}

/**
 * Returns the number of registers accessed according to the \p access_mask
 */
static int
registers_read(const uint64_t access_mask[static 32])
{
   int regs_read = 0;

   for (unsigned i = 0; i < 32; i++) {
      if (access_mask[i] > 0xFFFFFFFF) {
         return 2;
      } else if (access_mask[i]) {
         regs_read = 1;
      }
   }

   return regs_read;
}

/**
 * Checks restrictions listed in "Region Alignment Rules" in the "Register
 * Region Restrictions" section.
 */
static struct string
region_alignment_rules(const struct brw_isa_info *isa,
                       const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;
   const struct opcode_desc *desc =
      brw_opcode_desc(isa, brw_inst_opcode(isa, inst));
   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   unsigned exec_size = 1 << brw_inst_exec_size(devinfo, inst);
   uint64_t dst_access_mask[32], src0_access_mask[32], src1_access_mask[32];
   struct string error_msg = { .str = NULL, .len = 0 };

   if (num_sources == 3)
      return (struct string){};

   if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_16)
      return (struct string){};

   if (inst_is_send(isa, inst))
      return (struct string){};

   memset(dst_access_mask, 0, sizeof(dst_access_mask));
   memset(src0_access_mask, 0, sizeof(src0_access_mask));
   memset(src1_access_mask, 0, sizeof(src1_access_mask));

   for (unsigned i = 0; i < num_sources; i++) {
      unsigned vstride, width, hstride, element_size, subreg;
      enum brw_reg_type type;

      /* In Direct Addressing mode, a source cannot span more than 2 adjacent
       * GRF registers.
       */

#define DO_SRC(n)                                                              \
      if (brw_inst_src ## n ## _address_mode(devinfo, inst) !=                 \
          BRW_ADDRESS_DIRECT)                                                  \
         continue;                                                             \
                                                                               \
      if (brw_inst_src ## n ## _reg_file(devinfo, inst) ==                     \
          BRW_IMMEDIATE_VALUE)                                                 \
         continue;                                                             \
                                                                               \
      vstride = STRIDE(brw_inst_src ## n ## _vstride(devinfo, inst));          \
      width = WIDTH(brw_inst_src ## n ## _width(devinfo, inst));               \
      hstride = STRIDE(brw_inst_src ## n ## _hstride(devinfo, inst));          \
      type = brw_inst_src ## n ## _type(devinfo, inst);                        \
      element_size = brw_reg_type_to_size(type);                               \
      subreg = brw_inst_src ## n ## _da1_subreg_nr(devinfo, inst);             \
      align1_access_mask(src ## n ## _access_mask,                             \
                         exec_size, element_size, subreg,                      \
                         vstride, width, hstride)

      if (i == 0) {
         DO_SRC(0);
      } else {
         DO_SRC(1);
      }
#undef DO_SRC

      unsigned num_vstride = exec_size / width;
      unsigned num_hstride = width;
      unsigned vstride_elements = (num_vstride - 1) * vstride;
      unsigned hstride_elements = (num_hstride - 1) * hstride;
      unsigned offset = (vstride_elements + hstride_elements) * element_size +
                        subreg;
      ERROR_IF(offset >= 64 * reg_unit(devinfo),
               "A source cannot span more than 2 adjacent GRF registers");
   }

   if (desc->ndst == 0 || dst_is_null(devinfo, inst))
      return error_msg;

   unsigned stride = STRIDE(brw_inst_dst_hstride(devinfo, inst));
   enum brw_reg_type dst_type = inst_dst_type(isa, inst);
   unsigned element_size = brw_reg_type_to_size(dst_type);
   unsigned subreg = brw_inst_dst_da1_subreg_nr(devinfo, inst);
   unsigned offset = ((exec_size - 1) * stride * element_size) + subreg;
   ERROR_IF(offset >= 64 * reg_unit(devinfo),
            "A destination cannot span more than 2 adjacent GRF registers");

   if (error_msg.str)
      return error_msg;

   /* On IVB/BYT, region parameters and execution size for DF are in terms of
    * 32-bit elements, so they are doubled. For evaluating the validity of an
    * instruction, we halve them.
    */
   if (devinfo->verx10 == 70 &&
       element_size == 8)
      element_size = 4;

   align1_access_mask(dst_access_mask, exec_size, element_size, subreg,
                      exec_size == 1 ? 0 : exec_size * stride,
                      exec_size == 1 ? 1 : exec_size,
                      exec_size == 1 ? 0 : stride);

   unsigned dst_regs = registers_read(dst_access_mask);
   unsigned src0_regs = registers_read(src0_access_mask);
   unsigned src1_regs = registers_read(src1_access_mask);

   /* The SNB, IVB, HSW, BDW, and CHV PRMs say:
    *
    *    When an instruction has a source region spanning two registers and a
    *    destination region contained in one register, the number of elements
    *    must be the same between two sources and one of the following must be
    *    true:
    *
    *       1. The destination region is entirely contained in the lower OWord
    *          of a register.
    *       2. The destination region is entirely contained in the upper OWord
    *          of a register.
    *       3. The destination elements are evenly split between the two OWords
    *          of a register.
    */
   if (devinfo->ver <= 8) {
      if (dst_regs == 1 && (src0_regs == 2 || src1_regs == 2)) {
         unsigned upper_oword_writes = 0, lower_oword_writes = 0;

         for (unsigned i = 0; i < exec_size; i++) {
            if (dst_access_mask[i] > 0x0000FFFF) {
               upper_oword_writes++;
            } else {
               assert(dst_access_mask[i] != 0);
               lower_oword_writes++;
            }
         }

         ERROR_IF(lower_oword_writes != 0 &&
                  upper_oword_writes != 0 &&
                  upper_oword_writes != lower_oword_writes,
                  "Writes must be to only one OWord or "
                  "evenly split between OWords");
      }
   }

   /* The IVB and HSW PRMs say:
    *
    *    When an instruction has a source region that spans two registers and
    *    the destination spans two registers, the destination elements must be
    *    evenly split between the two registers [...]
    *
    * The SNB PRM contains similar wording (but written in a much more
    * confusing manner).
    *
    * The BDW PRM says:
    *
    *    When destination spans two registers, the source may be one or two
    *    registers. The destination elements must be evenly split between the
    *    two registers.
    *
    * The SKL PRM says:
    *
    *    When destination of MATH instruction spans two registers, the
    *    destination elements must be evenly split between the two registers.
    *
    * It is not known whether this restriction applies to KBL other Gens after
    * SKL.
    */
   if (devinfo->ver <= 8 ||
       brw_inst_opcode(isa, inst) == BRW_OPCODE_MATH) {

      /* Nothing explicitly states that on Gen < 8 elements must be evenly
       * split between two destination registers in the two exceptional
       * source-region-spans-one-register cases, but since Broadwell requires
       * evenly split writes regardless of source region, we assume that it was
       * an oversight and require it.
       */
      if (dst_regs == 2) {
         unsigned upper_reg_writes = 0, lower_reg_writes = 0;

         for (unsigned i = 0; i < exec_size; i++) {
            if (dst_access_mask[i] > 0xFFFFFFFF) {
               upper_reg_writes++;
            } else {
               assert(dst_access_mask[i] != 0);
               lower_reg_writes++;
            }
         }

         ERROR_IF(upper_reg_writes != lower_reg_writes,
                  "Writes must be evenly split between the two "
                  "destination registers");
      }
   }

   /* The IVB and HSW PRMs say:
    *
    *    When an instruction has a source region that spans two registers and
    *    the destination spans two registers, the destination elements must be
    *    evenly split between the two registers and each destination register
    *    must be entirely derived from one source register.
    *
    *    Note: In such cases, the regioning parameters must ensure that the
    *    offset from the two source registers is the same.
    *
    * The SNB PRM contains similar wording (but written in a much more
    * confusing manner).
    *
    * There are effectively three rules stated here:
    *
    *    For an instruction with a source and a destination spanning two
    *    registers,
    *
    *       (1) destination elements must be evenly split between the two
    *           registers
    *       (2) all destination elements in a register must be derived
    *           from one source register
    *       (3) the offset (i.e. the starting location in each of the two
    *           registers spanned by a region) must be the same in the two
    *           registers spanned by a region
    *
    * It is impossible to violate rule (1) without violating (2) or (3), so we
    * do not attempt to validate it.
    */
   if (devinfo->ver <= 7 && dst_regs == 2) {
      for (unsigned i = 0; i < num_sources; i++) {
#define DO_SRC(n)                                                             \
         if (src ## n ## _regs <= 1)                                          \
            continue;                                                         \
                                                                              \
         for (unsigned i = 0; i < exec_size; i++) {                           \
            if ((dst_access_mask[i] > 0xFFFFFFFF) !=                          \
                (src ## n ## _access_mask[i] > 0xFFFFFFFF)) {                 \
               ERROR("Each destination register must be entirely derived "    \
                     "from one source register");                             \
               break;                                                         \
            }                                                                 \
         }                                                                    \
                                                                              \
         unsigned offset_0 =                                                  \
            brw_inst_src ## n ## _da1_subreg_nr(devinfo, inst);               \
         unsigned offset_1 = offset_0;                                        \
                                                                              \
         for (unsigned i = 0; i < exec_size; i++) {                           \
            if (src ## n ## _access_mask[i] > 0xFFFFFFFF) {                   \
               offset_1 = __builtin_ctzll(src ## n ## _access_mask[i]) - 32;  \
               break;                                                         \
            }                                                                 \
         }                                                                    \
                                                                              \
         ERROR_IF(num_sources == 2 && offset_0 != offset_1,                   \
                  "The offset from the two source registers "                 \
                  "must be the same")

         if (i == 0) {
            DO_SRC(0);
         } else {
            DO_SRC(1);
         }
#undef DO_SRC
      }
   }

   /* The IVB and HSW PRMs say:
    *
    *    When destination spans two registers, the source MUST span two
    *    registers. The exception to the above rule:
    *        1. When source is scalar, the source registers are not
    *           incremented.
    *        2. When source is packed integer Word and destination is packed
    *           integer DWord, the source register is not incremented by the
    *           source sub register is incremented.
    *
    * The SNB PRM does not contain this rule, but the internal documentation
    * indicates that it applies to SNB as well. We assume that the rule applies
    * to Gen <= 5 although their PRMs do not state it.
    *
    * While the documentation explicitly says in exception (2) that the
    * destination must be an integer DWord, the hardware allows at least a
    * float destination type as well. We emit such instructions from
    *
    *    fs_visitor::emit_interpolation_setup_gfx6
    *    fs_visitor::emit_fragcoord_interpolation
    *
    * and have for years with no ill effects.
    *
    * Additionally the simulator source code indicates that the real condition
    * is that the size of the destination type is 4 bytes.
    *
    * HSW PRMs also add a note to the second exception:
    *  "When lower 8 channels are disabled, the sub register of source1
    *   operand is not incremented. If the lower 8 channels are expected
    *   to be disabled, say by predication, the instruction must be split
    *   into pair of simd8 operations."
    *
    * We can't reliably know if the channels won't be disabled due to,
    * for example, IMASK. So, play it safe and disallow packed-word exception
    * for src1.
    */
   if (devinfo->ver <= 7 && dst_regs == 2) {
      enum brw_reg_type dst_type = inst_dst_type(isa, inst);
      bool dst_is_packed_dword =
         is_packed(exec_size * stride, exec_size, stride) &&
         brw_reg_type_to_size(dst_type) == 4;

      for (unsigned i = 0; i < num_sources; i++) {
#define DO_SRC(n)                                                                  \
         unsigned vstride, width, hstride;                                         \
         vstride = STRIDE(brw_inst_src ## n ## _vstride(devinfo, inst));           \
         width = WIDTH(brw_inst_src ## n ## _width(devinfo, inst));                \
         hstride = STRIDE(brw_inst_src ## n ## _hstride(devinfo, inst));           \
         bool src ## n ## _is_packed_word =                                        \
            n != 1 && is_packed(vstride, width, hstride) &&                        \
            (brw_inst_src ## n ## _type(devinfo, inst) == BRW_REGISTER_TYPE_W ||   \
             brw_inst_src ## n ## _type(devinfo, inst) == BRW_REGISTER_TYPE_UW);   \
                                                                                   \
         ERROR_IF(src ## n ## _regs == 1 &&                                        \
                  !src ## n ## _has_scalar_region(devinfo, inst) &&                \
                  !(dst_is_packed_dword && src ## n ## _is_packed_word),           \
                  "When the destination spans two registers, the source must "     \
                  "span two registers\n" ERROR_INDENT "(exceptions for scalar "    \
                  "sources, and packed-word to packed-dword expansion for src0)")

         if (i == 0) {
            DO_SRC(0);
         } else {
            DO_SRC(1);
         }
#undef DO_SRC
      }
   }

   return error_msg;
}

static struct string
vector_immediate_restrictions(const struct brw_isa_info *isa,
                              const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   struct string error_msg = { .str = NULL, .len = 0 };

   if (num_sources == 3 || num_sources == 0)
      return (struct string){};

   unsigned file = num_sources == 1 ?
                   brw_inst_src0_reg_file(devinfo, inst) :
                   brw_inst_src1_reg_file(devinfo, inst);
   if (file != BRW_IMMEDIATE_VALUE)
      return (struct string){};

   enum brw_reg_type dst_type = inst_dst_type(isa, inst);
   unsigned dst_type_size = brw_reg_type_to_size(dst_type);
   unsigned dst_subreg = brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1 ?
                         brw_inst_dst_da1_subreg_nr(devinfo, inst) : 0;
   unsigned dst_stride = STRIDE(brw_inst_dst_hstride(devinfo, inst));
   enum brw_reg_type type = num_sources == 1 ?
                            brw_inst_src0_type(devinfo, inst) :
                            brw_inst_src1_type(devinfo, inst);

   /* The PRMs say:
    *
    *    When an immediate vector is used in an instruction, the destination
    *    must be 128-bit aligned with destination horizontal stride equivalent
    *    to a word for an immediate integer vector (v) and equivalent to a
    *    DWord for an immediate float vector (vf).
    *
    * The text has not been updated for the addition of the immediate unsigned
    * integer vector type (uv) on SNB, but presumably the same restriction
    * applies.
    */
   switch (type) {
   case BRW_REGISTER_TYPE_V:
   case BRW_REGISTER_TYPE_UV:
   case BRW_REGISTER_TYPE_VF:
      ERROR_IF(dst_subreg % (128 / 8) != 0,
               "Destination must be 128-bit aligned in order to use immediate "
               "vector types");

      if (type == BRW_REGISTER_TYPE_VF) {
         ERROR_IF(dst_type_size * dst_stride != 4,
                  "Destination must have stride equivalent to dword in order "
                  "to use the VF type");
      } else {
         ERROR_IF(dst_type_size * dst_stride != 2,
                  "Destination must have stride equivalent to word in order "
                  "to use the V or UV type");
      }
      break;
   default:
      break;
   }

   return error_msg;
}

static struct string
special_requirements_for_handling_double_precision_data_types(
                                       const struct brw_isa_info *isa,
                                       const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;

   unsigned num_sources = brw_num_sources_from_inst(isa, inst);
   struct string error_msg = { .str = NULL, .len = 0 };

   if (num_sources == 3 || num_sources == 0)
      return (struct string){};

   /* Split sends don't have types so there's no doubles there. */
   if (inst_is_split_send(isa, inst))
      return (struct string){};

   enum brw_reg_type exec_type = execution_type(isa, inst);
   unsigned exec_type_size = brw_reg_type_to_size(exec_type);

   enum brw_reg_file dst_file = brw_inst_dst_reg_file(devinfo, inst);
   enum brw_reg_type dst_type = inst_dst_type(isa, inst);
   unsigned dst_type_size = brw_reg_type_to_size(dst_type);
   unsigned dst_hstride = STRIDE(brw_inst_dst_hstride(devinfo, inst));
   unsigned dst_reg = brw_inst_dst_da_reg_nr(devinfo, inst);
   unsigned dst_subreg = brw_inst_dst_da1_subreg_nr(devinfo, inst);
   unsigned dst_address_mode = brw_inst_dst_address_mode(devinfo, inst);

   bool is_integer_dword_multiply =
      devinfo->ver >= 8 &&
      brw_inst_opcode(isa, inst) == BRW_OPCODE_MUL &&
      (brw_inst_src0_type(devinfo, inst) == BRW_REGISTER_TYPE_D ||
       brw_inst_src0_type(devinfo, inst) == BRW_REGISTER_TYPE_UD) &&
      (brw_inst_src1_type(devinfo, inst) == BRW_REGISTER_TYPE_D ||
       brw_inst_src1_type(devinfo, inst) == BRW_REGISTER_TYPE_UD);

   const bool is_double_precision =
      dst_type_size == 8 || exec_type_size == 8 || is_integer_dword_multiply;

   for (unsigned i = 0; i < num_sources; i++) {
      unsigned vstride, width, hstride, type_size, reg, subreg, address_mode;
      bool is_scalar_region;
      enum brw_reg_file file;
      enum brw_reg_type type;

#define DO_SRC(n)                                                              \
      if (brw_inst_src ## n ## _reg_file(devinfo, inst) ==                     \
          BRW_IMMEDIATE_VALUE)                                                 \
         continue;                                                             \
                                                                               \
      is_scalar_region = src ## n ## _has_scalar_region(devinfo, inst);        \
      vstride = STRIDE(brw_inst_src ## n ## _vstride(devinfo, inst));          \
      width = WIDTH(brw_inst_src ## n ## _width(devinfo, inst));               \
      hstride = STRIDE(brw_inst_src ## n ## _hstride(devinfo, inst));          \
      file = brw_inst_src ## n ## _reg_file(devinfo, inst);                    \
      type = brw_inst_src ## n ## _type(devinfo, inst);                        \
      type_size = brw_reg_type_to_size(type);                                  \
      reg = brw_inst_src ## n ## _da_reg_nr(devinfo, inst);                    \
      subreg = brw_inst_src ## n ## _da1_subreg_nr(devinfo, inst);             \
      address_mode = brw_inst_src ## n ## _address_mode(devinfo, inst)

      if (i == 0) {
         DO_SRC(0);
      } else {
         DO_SRC(1);
      }
#undef DO_SRC

      const unsigned src_stride = (hstride ? hstride : vstride) * type_size;
      const unsigned dst_stride = dst_hstride * dst_type_size;

      /* The PRMs say that for CHV, BXT:
       *
       *    When source or destination datatype is 64b or operation is integer
       *    DWord multiply, regioning in Align1 must follow these rules:
       *
       *    1. Source and Destination horizontal stride must be aligned to the
       *       same qword.
       *    2. Regioning must ensure Src.Vstride = Src.Width * Src.Hstride.
       *    3. Source and Destination offset must be the same, except the case
       *       of scalar source.
       *
       * We assume that the restriction applies to GLK as well.
       */
      if (is_double_precision &&
          brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1 &&
          (devinfo->platform == INTEL_PLATFORM_CHV || intel_device_info_is_9lp(devinfo))) {
         ERROR_IF(!is_scalar_region &&
                  (src_stride % 8 != 0 ||
                   dst_stride % 8 != 0 ||
                   src_stride != dst_stride),
                  "Source and destination horizontal stride must equal and a "
                  "multiple of a qword when the execution type is 64-bit");

         ERROR_IF(vstride != width * hstride,
                  "Vstride must be Width * Hstride when the execution type is "
                  "64-bit");

         ERROR_IF(!is_scalar_region && dst_subreg != subreg,
                  "Source and destination offset must be the same when the "
                  "execution type is 64-bit");
      }

      /* The PRMs say that for CHV, BXT:
       *
       *    When source or destination datatype is 64b or operation is integer
       *    DWord multiply, indirect addressing must not be used.
       *
       * We assume that the restriction applies to GLK as well.
       */
      if (is_double_precision &&
          (devinfo->platform == INTEL_PLATFORM_CHV || intel_device_info_is_9lp(devinfo))) {
         ERROR_IF(BRW_ADDRESS_REGISTER_INDIRECT_REGISTER == address_mode ||
                  BRW_ADDRESS_REGISTER_INDIRECT_REGISTER == dst_address_mode,
                  "Indirect addressing is not allowed when the execution type "
                  "is 64-bit");
      }

      /* The PRMs say that for CHV, BXT:
       *
       *    ARF registers must never be used with 64b datatype or when
       *    operation is integer DWord multiply.
       *
       * We assume that the restriction applies to GLK as well.
       *
       * We assume that the restriction does not apply to the null register.
       */
      if (is_double_precision &&
          (devinfo->platform == INTEL_PLATFORM_CHV ||
           intel_device_info_is_9lp(devinfo))) {
         ERROR_IF(brw_inst_opcode(isa, inst) == BRW_OPCODE_MAC ||
                  brw_inst_acc_wr_control(devinfo, inst) ||
                  (BRW_ARCHITECTURE_REGISTER_FILE == file &&
                   reg != BRW_ARF_NULL) ||
                  (BRW_ARCHITECTURE_REGISTER_FILE == dst_file &&
                   dst_reg != BRW_ARF_NULL),
                  "Architecture registers cannot be used when the execution "
                  "type is 64-bit");
      }

      /* From the hardware spec section "Register Region Restrictions":
       *
       * There are two rules:
       *
       * "In case of all floating point data types used in destination:" and
       *
       * "In case where source or destination datatype is 64b or operation is
       *  integer DWord multiply:"
       *
       * both of which list the same restrictions:
       *
       *  "1. Register Regioning patterns where register data bit location
       *      of the LSB of the channels are changed between source and
       *      destination are not supported on Src0 and Src1 except for
       *      broadcast of a scalar.
       *
       *   2. Explicit ARF registers except null and accumulator must not be
       *      used."
       */
      if (devinfo->verx10 >= 125 &&
          (brw_reg_type_is_floating_point(dst_type) ||
           is_double_precision)) {
         ERROR_IF(!is_scalar_region &&
                  BRW_ADDRESS_REGISTER_INDIRECT_REGISTER != address_mode &&
                  (!is_linear(vstride, width, hstride) ||
                   src_stride != dst_stride ||
                   subreg != dst_subreg),
                  "Register Regioning patterns where register data bit "
                  "location of the LSB of the channels are changed between "
                  "source and destination are not supported except for "
                  "broadcast of a scalar.");

         ERROR_IF((file == BRW_ARCHITECTURE_REGISTER_FILE &&
                   reg != BRW_ARF_NULL && !(reg >= BRW_ARF_ACCUMULATOR && reg < BRW_ARF_FLAG)) ||
                  (dst_file == BRW_ARCHITECTURE_REGISTER_FILE &&
                   dst_reg != BRW_ARF_NULL && dst_reg != BRW_ARF_ACCUMULATOR),
                  "Explicit ARF registers except null and accumulator must not "
                  "be used.");
      }

      /* From the hardware spec section "Register Region Restrictions":
       *
       * "Vx1 and VxH indirect addressing for Float, Half-Float, Double-Float and
       *  Quad-Word data must not be used."
       */
      if (devinfo->verx10 >= 125 &&
          (brw_reg_type_is_floating_point(type) || type_sz(type) == 8)) {
         ERROR_IF(address_mode == BRW_ADDRESS_REGISTER_INDIRECT_REGISTER &&
                  vstride == BRW_VERTICAL_STRIDE_ONE_DIMENSIONAL,
                  "Vx1 and VxH indirect addressing for Float, Half-Float, "
                  "Double-Float and Quad-Word data must not be used");
      }
   }

   /* The PRMs say that for BDW, SKL:
    *
    *    If Align16 is required for an operation with QW destination and non-QW
    *    source datatypes, the execution size cannot exceed 2.
    *
    * We assume that the restriction applies to all Gfx8+ parts.
    */
   if (is_double_precision && devinfo->ver >= 8) {
      enum brw_reg_type src0_type = brw_inst_src0_type(devinfo, inst);
      enum brw_reg_type src1_type =
         num_sources > 1 ? brw_inst_src1_type(devinfo, inst) : src0_type;
      unsigned src0_type_size = brw_reg_type_to_size(src0_type);
      unsigned src1_type_size = brw_reg_type_to_size(src1_type);

      ERROR_IF(brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_16 &&
               dst_type_size == 8 &&
               (src0_type_size != 8 || src1_type_size != 8) &&
               brw_inst_exec_size(devinfo, inst) > BRW_EXECUTE_2,
               "In Align16 exec size cannot exceed 2 with a QWord destination "
               "and a non-QWord source");
   }

   /* The PRMs say that for CHV, BXT:
    *
    *    When source or destination datatype is 64b or operation is integer
    *    DWord multiply, DepCtrl must not be used.
    *
    * We assume that the restriction applies to GLK as well.
    */
   if (is_double_precision &&
       (devinfo->platform == INTEL_PLATFORM_CHV || intel_device_info_is_9lp(devinfo))) {
      ERROR_IF(brw_inst_no_dd_check(devinfo, inst) ||
               brw_inst_no_dd_clear(devinfo, inst),
               "DepCtrl is not allowed when the execution type is 64-bit");
   }

   return error_msg;
}

static struct string
instruction_restrictions(const struct brw_isa_info *isa,
                         const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;
   struct string error_msg = { .str = NULL, .len = 0 };

   /* From Wa_1604601757:
    *
    * "When multiplying a DW and any lower precision integer, source modifier
    *  is not supported."
    */
   if (devinfo->ver >= 12 &&
       brw_inst_opcode(isa, inst) == BRW_OPCODE_MUL) {
      enum brw_reg_type exec_type = execution_type(isa, inst);
      const bool src0_valid = type_sz(brw_inst_src0_type(devinfo, inst)) == 4 ||
         brw_inst_src0_reg_file(devinfo, inst) == BRW_IMMEDIATE_VALUE ||
         !(brw_inst_src0_negate(devinfo, inst) ||
           brw_inst_src0_abs(devinfo, inst));
      const bool src1_valid = type_sz(brw_inst_src1_type(devinfo, inst)) == 4 ||
         brw_inst_src1_reg_file(devinfo, inst) == BRW_IMMEDIATE_VALUE ||
         !(brw_inst_src1_negate(devinfo, inst) ||
           brw_inst_src1_abs(devinfo, inst));

      ERROR_IF(!brw_reg_type_is_floating_point(exec_type) &&
               type_sz(exec_type) == 4 && !(src0_valid && src1_valid),
               "When multiplying a DW and any lower precision integer, source "
               "modifier is not supported.");
   }

   if (brw_inst_opcode(isa, inst) == BRW_OPCODE_CMP ||
       brw_inst_opcode(isa, inst) == BRW_OPCODE_CMPN) {
      if (devinfo->ver <= 7) {
         /* Page 166 of the Ivy Bridge PRM Volume 4 part 3 (Execution Unit
          * ISA) says:
          *
          *    Accumulator cannot be destination, implicit or explicit. The
          *    destination must be a general register or the null register.
          *
          * Page 77 of the Haswell PRM Volume 2b contains the same text.  The
          * 965G PRMs contain similar text.
          *
          * Page 864 (page 880 of the PDF) of the Broadwell PRM Volume 7 says:
          *
          *    For the cmp and cmpn instructions, remove the accumulator
          *    restrictions.
          */
         ERROR_IF(brw_inst_dst_reg_file(devinfo, inst) == BRW_ARCHITECTURE_REGISTER_FILE &&
                  brw_inst_dst_da_reg_nr(devinfo, inst) != BRW_ARF_NULL,
                  "Accumulator cannot be destination, implicit or explicit.");
      }

      /* Page 166 of the Ivy Bridge PRM Volume 4 part 3 (Execution Unit ISA)
       * says:
       *
       *    If the destination is the null register, the {Switch} instruction
       *    option must be used.
       *
       * Page 77 of the Haswell PRM Volume 2b contains the same text.
       */
      if (devinfo->ver == 7) {
         ERROR_IF(dst_is_null(devinfo, inst) &&
                  brw_inst_thread_control(devinfo, inst) != BRW_THREAD_SWITCH,
                  "If the destination is the null register, the {Switch} "
                  "instruction option must be used.");
      }

      ERROR_IF(brw_inst_cond_modifier(devinfo, inst) == BRW_CONDITIONAL_NONE,
               "CMP (or CMPN) must have a condition.");
   }

   if (brw_inst_opcode(isa, inst) == BRW_OPCODE_SEL) {
      if (devinfo->ver < 6) {
         ERROR_IF(brw_inst_cond_modifier(devinfo, inst) != BRW_CONDITIONAL_NONE,
                  "SEL must not have a condition modifier");
         ERROR_IF(brw_inst_pred_control(devinfo, inst) == BRW_PREDICATE_NONE,
                  "SEL must be predicated");
      } else {
         ERROR_IF((brw_inst_cond_modifier(devinfo, inst) != BRW_CONDITIONAL_NONE) ==
                  (brw_inst_pred_control(devinfo, inst) != BRW_PREDICATE_NONE),
                  "SEL must either be predicated or have a condition modifiers");
      }
   }

   if (brw_inst_opcode(isa, inst) == BRW_OPCODE_MUL) {
      const enum brw_reg_type src0_type = brw_inst_src0_type(devinfo, inst);
      const enum brw_reg_type src1_type = brw_inst_src1_type(devinfo, inst);
      const enum brw_reg_type dst_type = inst_dst_type(isa, inst);

      if (devinfo->ver == 6) {
         /* Page 223 of the Sandybridge PRM volume 4 part 2 says:
          *
          *    [DevSNB]: When multiple (sic) a DW and a W, the W has to be on
          *    src0, and the DW has to be on src1.
          *
          * This text appears only in the Sandybridge PRMw.
          */
         ERROR_IF(brw_reg_type_is_integer(src0_type) &&
                  type_sz(src0_type) == 4 && type_sz(src1_type) < 4,
                  "When multiplying a DW and any lower precision integer, the "
                  "DW operand must be src1.");
      } else if (devinfo->ver >= 7) {
         /* Page 966 (page 982 of the PDF) of Broadwell PRM volume 2a says:
          *
          *    When multiplying a DW and any lower precision integer, the DW
          *    operand must on src0.
          *
          * Ivy Bridge, Haswell, Skylake, and Ice Lake PRMs contain the same
          * text.
          */
         ERROR_IF(brw_reg_type_is_integer(src1_type) &&
                  type_sz(src0_type) < 4 && type_sz(src1_type) == 4,
                  "When multiplying a DW and any lower precision integer, the "
                  "DW operand must be src0.");
      }

      if (devinfo->ver <= 7) {
         /* Section 14.2.28 of Intel 965 Express Chipset PRM volume 4 says:
          *
          *    Source operands cannot be an accumulator register.
          *
          * Iron Lake, Sandybridge, and Ivy Bridge PRMs have the same text.
          * Haswell does not.  Given that later PRMs have different
          * restrictions on accumulator sources (see below), it seems most
          * likely that Haswell shares the Ivy Bridge restriction.
          */
         ERROR_IF(src0_is_acc(devinfo, inst) || src1_is_acc(devinfo, inst),
                  "Source operands cannot be an accumulator register.");
      } else {
         /* Page 971 (page 987 of the PDF), section "Accumulator
          * Restrictions," of the Broadwell PRM volume 7 says:
          *
          *    Integer source operands cannot be accumulators.
          *
          * The Skylake and Ice Lake PRMs contain the same text.
          */
         ERROR_IF((src0_is_acc(devinfo, inst) &&
                   brw_reg_type_is_integer(src0_type)) ||
                  (src1_is_acc(devinfo, inst) &&
                   brw_reg_type_is_integer(src1_type)),
                  "Integer source operands cannot be accumulators.");
      }

      if (devinfo->ver <= 6) {
         /* Page 223 of the Sandybridge PRM volume 4 part 2 says:
          *
          *    Dword integer source is not allowed for this instruction in
          *    float execution mode.  In other words, if one source is of type
          *    float (:f, :vf), the other source cannot be of type dword
          *    integer (:ud or :d).
          *
          * G965 and Iron Lake PRMs have similar text.  Later GPUs do not
          * allow mixed source types at all, but that restriction should be
          * handled elsewhere.
          */
         ERROR_IF(execution_type(isa, inst) == BRW_REGISTER_TYPE_F &&
                  (src0_type == BRW_REGISTER_TYPE_UD ||
                   src0_type == BRW_REGISTER_TYPE_D ||
                   src1_type == BRW_REGISTER_TYPE_UD ||
                   src1_type == BRW_REGISTER_TYPE_D),
                  "Dword integer source is not allowed for this instruction in"
                  "float execution mode.");
      }

      if (devinfo->ver <= 7) {
         /* Page 118 of the Haswell PRM volume 2b says:
          *
          *    When operating on integers with at least one of the source
          *    being a DWord type (signed or unsigned), the destination cannot
          *    be floating-point (implementation note: the data converter only
          *    looks at the low 34 bits of the result).
          *
          * G965, Iron Lake, Sandybridge, and Ivy Bridge have similar text.
          * Later GPUs do not allow mixed source and destination types at all,
          * but that restriction should be handled elsewhere.
          */
         ERROR_IF(dst_type == BRW_REGISTER_TYPE_F &&
                  (src0_type == BRW_REGISTER_TYPE_UD ||
                   src0_type == BRW_REGISTER_TYPE_D ||
                   src1_type == BRW_REGISTER_TYPE_UD ||
                   src1_type == BRW_REGISTER_TYPE_D),
                  "Float destination type not allowed with DWord source type.");
      }

      if (devinfo->ver == 8) {
         /* Page 966 (page 982 of the PDF) of the Broadwell PRM volume 2a
          * says:
          *
          *    When multiplying DW x DW, the dst cannot be accumulator.
          *
          * This text also appears in the Cherry Trail / Braswell PRM, but it
          * does not appear in any other PRM.
          */
         ERROR_IF((src0_type == BRW_REGISTER_TYPE_UD ||
                   src0_type == BRW_REGISTER_TYPE_D) &&
                  (src1_type == BRW_REGISTER_TYPE_UD ||
                   src1_type == BRW_REGISTER_TYPE_D) &&
                  brw_inst_dst_reg_file(devinfo, inst) == BRW_ARCHITECTURE_REGISTER_FILE &&
                  brw_inst_dst_da_reg_nr(devinfo, inst) != BRW_ARF_NULL,
                  "When multiplying DW x DW, the dst cannot be accumulator.");
      }

      /* Page 935 (page 951 of the PDF) of the Ice Lake PRM volume 2a says:
       *
       *    When multiplying integer data types, if one of the sources is a
       *    DW, the resulting full precision data is stored in the
       *    accumulator. However, if the destination data type is either W or
       *    DW, the low bits of the result are written to the destination
       *    register and the remaining high bits are discarded. This results
       *    in undefined Overflow and Sign flags. Therefore, conditional
       *    modifiers and saturation (.sat) cannot be used in this case.
       *
       * Similar text appears in every version of the PRM.
       *
       * The wording of the last sentence is not very clear.  It could either
       * be interpreted as "conditional modifiers combined with saturation
       * cannot be used" or "neither conditional modifiers nor saturation can
       * be used."  I have interpreted it as the latter primarily because that
       * is the more restrictive interpretation.
       */
      ERROR_IF((src0_type == BRW_REGISTER_TYPE_UD ||
                src0_type == BRW_REGISTER_TYPE_D ||
                src1_type == BRW_REGISTER_TYPE_UD ||
                src1_type == BRW_REGISTER_TYPE_D) &&
               (dst_type == BRW_REGISTER_TYPE_UD ||
                dst_type == BRW_REGISTER_TYPE_D ||
                dst_type == BRW_REGISTER_TYPE_UW ||
                dst_type == BRW_REGISTER_TYPE_W) &&
               (brw_inst_saturate(devinfo, inst) != 0 ||
                brw_inst_cond_modifier(devinfo, inst) != BRW_CONDITIONAL_NONE),
               "Neither Saturate nor conditional modifier allowed with DW "
               "integer multiply.");
   }

   if (brw_inst_opcode(isa, inst) == BRW_OPCODE_MATH) {
      unsigned math_function = brw_inst_math_function(devinfo, inst);
      switch (math_function) {
      case BRW_MATH_FUNCTION_INT_DIV_QUOTIENT_AND_REMAINDER:
      case BRW_MATH_FUNCTION_INT_DIV_QUOTIENT:
      case BRW_MATH_FUNCTION_INT_DIV_REMAINDER: {
         /* Page 442 of the Broadwell PRM Volume 2a "Extended Math Function" says:
          *    INT DIV function does not support source modifiers.
          * Bspec 6647 extends it back to Ivy Bridge.
          */
         bool src0_valid = !brw_inst_src0_negate(devinfo, inst) &&
                           !brw_inst_src0_abs(devinfo, inst);
         bool src1_valid = !brw_inst_src1_negate(devinfo, inst) &&
                           !brw_inst_src1_abs(devinfo, inst);
         ERROR_IF(!src0_valid || !src1_valid,
                  "INT DIV function does not support source modifiers.");
         break;
      }
      default:
         break;
      }
   }

   if (brw_inst_opcode(isa, inst) == BRW_OPCODE_DP4A) {
      /* Page 396 (page 412 of the PDF) of the DG1 PRM volume 2a says:
       *
       *    Only one of src0 or src1 operand may be an the (sic) accumulator
       *    register (acc#).
       */
      ERROR_IF(src0_is_acc(devinfo, inst) && src1_is_acc(devinfo, inst),
               "Only one of src0 or src1 operand may be an accumulator "
               "register (acc#).");

   }

   if (brw_inst_opcode(isa, inst) == BRW_OPCODE_ADD3) {
      const enum brw_reg_type dst_type = inst_dst_type(isa, inst);

      ERROR_IF(dst_type != BRW_REGISTER_TYPE_D &&
               dst_type != BRW_REGISTER_TYPE_UD &&
               dst_type != BRW_REGISTER_TYPE_W &&
               dst_type != BRW_REGISTER_TYPE_UW,
               "Destination must be integer D, UD, W, or UW type.");

      for (unsigned i = 0; i < 3; i++) {
         enum brw_reg_type src_type;

         switch (i) {
         case 0: src_type = brw_inst_3src_a1_src0_type(devinfo, inst); break;
         case 1: src_type = brw_inst_3src_a1_src1_type(devinfo, inst); break;
         case 2: src_type = brw_inst_3src_a1_src2_type(devinfo, inst); break;
         default: unreachable("invalid src");
         }

         ERROR_IF(src_type != BRW_REGISTER_TYPE_D &&
                  src_type != BRW_REGISTER_TYPE_UD &&
                  src_type != BRW_REGISTER_TYPE_W &&
                  src_type != BRW_REGISTER_TYPE_UW,
                  "Source must be integer D, UD, W, or UW type.");

         if (i == 0) {
            if (brw_inst_3src_a1_src0_is_imm(devinfo, inst)) {
               ERROR_IF(src_type != BRW_REGISTER_TYPE_W &&
                        src_type != BRW_REGISTER_TYPE_UW,
                        "Immediate source must be integer W or UW type.");
            }
         } else if (i == 2) {
            if (brw_inst_3src_a1_src2_is_imm(devinfo, inst)) {
               ERROR_IF(src_type != BRW_REGISTER_TYPE_W &&
                        src_type != BRW_REGISTER_TYPE_UW,
                        "Immediate source must be integer W or UW type.");
            }
         }
      }
   }

   if (brw_inst_opcode(isa, inst) == BRW_OPCODE_OR ||
       brw_inst_opcode(isa, inst) == BRW_OPCODE_AND ||
       brw_inst_opcode(isa, inst) == BRW_OPCODE_XOR ||
       brw_inst_opcode(isa, inst) == BRW_OPCODE_NOT) {
      if (devinfo->ver >= 8) {
         /* While the behavior of the negate source modifier is defined as
          * logical not, the behavior of abs source modifier is not
          * defined. Disallow it to be safe.
          */
         ERROR_IF(brw_inst_src0_abs(devinfo, inst),
                  "Behavior of abs source modifier in logic ops is undefined.");
         ERROR_IF(brw_inst_opcode(isa, inst) != BRW_OPCODE_NOT &&
                  brw_inst_src1_reg_file(devinfo, inst) != BRW_IMMEDIATE_VALUE &&
                  brw_inst_src1_abs(devinfo, inst),
                  "Behavior of abs source modifier in logic ops is undefined.");

         /* Page 479 (page 495 of the PDF) of the Broadwell PRM volume 2a says:
          *
          *    Source modifier is not allowed if source is an accumulator.
          *
          * The same text also appears for OR, NOT, and XOR instructions.
          */
         ERROR_IF((brw_inst_src0_abs(devinfo, inst) ||
                   brw_inst_src0_negate(devinfo, inst)) &&
                  src0_is_acc(devinfo, inst),
                  "Source modifier is not allowed if source is an accumulator.");
         ERROR_IF(brw_num_sources_from_inst(isa, inst) > 1 &&
                  (brw_inst_src1_abs(devinfo, inst) ||
                   brw_inst_src1_negate(devinfo, inst)) &&
                  src1_is_acc(devinfo, inst),
                  "Source modifier is not allowed if source is an accumulator.");
      }

      /* Page 479 (page 495 of the PDF) of the Broadwell PRM volume 2a says:
       *
       *    This operation does not produce sign or overflow conditions. Only
       *    the .e/.z or .ne/.nz conditional modifiers should be used.
       *
       * The same text also appears for OR, NOT, and XOR instructions.
       *
       * Per the comment around nir_op_imod in brw_fs_nir.cpp, we have
       * determined this to not be true. The only conditions that seem
       * absolutely sketchy are O, R, and U.  Some OpenGL shaders from Doom
       * 2016 have been observed to generate and.g and operate correctly.
       */
      const enum brw_conditional_mod cmod =
         brw_inst_cond_modifier(devinfo, inst);
      ERROR_IF(cmod == BRW_CONDITIONAL_O ||
               cmod == BRW_CONDITIONAL_R ||
               cmod == BRW_CONDITIONAL_U,
               "O, R, and U conditional modifiers should not be used.");
   }

   if (brw_inst_opcode(isa, inst) == BRW_OPCODE_BFI2) {
      ERROR_IF(brw_inst_cond_modifier(devinfo, inst) != BRW_CONDITIONAL_NONE,
               "BFI2 cannot have conditional modifier");

      ERROR_IF(brw_inst_saturate(devinfo, inst),
               "BFI2 cannot have saturate modifier");

      enum brw_reg_type dst_type;

      if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1)
         dst_type = brw_inst_3src_a1_dst_type(devinfo, inst);
      else
         dst_type = brw_inst_3src_a16_dst_type(devinfo, inst);

      ERROR_IF(dst_type != BRW_REGISTER_TYPE_D &&
               dst_type != BRW_REGISTER_TYPE_UD,
               "BFI2 destination type must be D or UD");

      for (unsigned s = 0; s < 3; s++) {
         enum brw_reg_type src_type;

         if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1) {
            switch (s) {
            case 0: src_type = brw_inst_3src_a1_src0_type(devinfo, inst); break;
            case 1: src_type = brw_inst_3src_a1_src1_type(devinfo, inst); break;
            case 2: src_type = brw_inst_3src_a1_src2_type(devinfo, inst); break;
            default: unreachable("invalid src");
            }
         } else {
            src_type = brw_inst_3src_a16_src_type(devinfo, inst);
         }

         ERROR_IF(src_type != dst_type,
                  "BFI2 source type must match destination type");
      }
   }

   if (brw_inst_opcode(isa, inst) == BRW_OPCODE_CSEL) {
      ERROR_IF(brw_inst_pred_control(devinfo, inst) != BRW_PREDICATE_NONE,
               "CSEL cannot be predicated");

      /* CSEL is CMP and SEL fused into one. The condition modifier, which
       * does not actually modify the flags, controls the built-in comparison.
       */
      ERROR_IF(brw_inst_cond_modifier(devinfo, inst) == BRW_CONDITIONAL_NONE,
               "CSEL must have a condition.");

      enum brw_reg_type dst_type;

      if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1)
         dst_type = brw_inst_3src_a1_dst_type(devinfo, inst);
      else
         dst_type = brw_inst_3src_a16_dst_type(devinfo, inst);

      if (devinfo->ver < 8) {
         ERROR_IF(devinfo->ver < 8, "CSEL not supported before Gfx8");
      } else if (devinfo->ver <= 9) {
         ERROR_IF(dst_type != BRW_REGISTER_TYPE_F,
                  "CSEL destination type must be F");
      } else {
         ERROR_IF(dst_type != BRW_REGISTER_TYPE_F &&
                  dst_type != BRW_REGISTER_TYPE_HF &&
                  dst_type != BRW_REGISTER_TYPE_D &&
                  dst_type != BRW_REGISTER_TYPE_W,
                  "CSEL destination type must be F, HF, D, or W");
      }

      for (unsigned s = 0; s < 3; s++) {
         enum brw_reg_type src_type;

         if (brw_inst_access_mode(devinfo, inst) == BRW_ALIGN_1) {
            switch (s) {
            case 0: src_type = brw_inst_3src_a1_src0_type(devinfo, inst); break;
            case 1: src_type = brw_inst_3src_a1_src1_type(devinfo, inst); break;
            case 2: src_type = brw_inst_3src_a1_src2_type(devinfo, inst); break;
            default: unreachable("invalid src");
            }
         } else {
            src_type = brw_inst_3src_a16_src_type(devinfo, inst);
         }

         ERROR_IF(src_type != dst_type,
                  "CSEL source type must match destination type");
      }
   }

   if (brw_inst_opcode(isa, inst) == BRW_OPCODE_DPAS) {
      ERROR_IF(brw_inst_dpas_3src_sdepth(devinfo, inst) != BRW_SYSTOLIC_DEPTH_8,
               "Systolic depth must be 8.");

      const unsigned sdepth = 8;

      const enum brw_reg_type dst_type =
         brw_inst_dpas_3src_dst_type(devinfo, inst);
      const enum brw_reg_type src0_type =
         brw_inst_dpas_3src_src0_type(devinfo, inst);
      const enum brw_reg_type src1_type =
         brw_inst_dpas_3src_src1_type(devinfo, inst);
      const enum brw_reg_type src2_type =
         brw_inst_dpas_3src_src2_type(devinfo, inst);

      const enum gfx12_sub_byte_precision src1_sub_byte =
         brw_inst_dpas_3src_src1_subbyte(devinfo, inst);

      if (src1_type != BRW_REGISTER_TYPE_B && src1_type != BRW_REGISTER_TYPE_UB) {
         ERROR_IF(src1_sub_byte != BRW_SUB_BYTE_PRECISION_NONE,
                  "Sub-byte precision must be None for source type larger than Byte.");
      } else {
         ERROR_IF(src1_sub_byte != BRW_SUB_BYTE_PRECISION_NONE &&
                  src1_sub_byte != BRW_SUB_BYTE_PRECISION_4BIT &&
                  src1_sub_byte != BRW_SUB_BYTE_PRECISION_2BIT,
                  "Invalid sub-byte precision.");
      }

      const enum gfx12_sub_byte_precision src2_sub_byte =
         brw_inst_dpas_3src_src2_subbyte(devinfo, inst);

      if (src2_type != BRW_REGISTER_TYPE_B && src2_type != BRW_REGISTER_TYPE_UB) {
         ERROR_IF(src2_sub_byte != BRW_SUB_BYTE_PRECISION_NONE,
                  "Sub-byte precision must be None.");
      } else {
         ERROR_IF(src2_sub_byte != BRW_SUB_BYTE_PRECISION_NONE &&
                  src2_sub_byte != BRW_SUB_BYTE_PRECISION_4BIT &&
                  src2_sub_byte != BRW_SUB_BYTE_PRECISION_2BIT,
                  "Invalid sub-byte precision.");
      }

      const unsigned src1_bits_per_element =
         (8 * brw_reg_type_to_size(src1_type)) >>
         brw_inst_dpas_3src_src1_subbyte(devinfo, inst);

      const unsigned src2_bits_per_element =
         (8 * brw_reg_type_to_size(src2_type)) >>
         brw_inst_dpas_3src_src2_subbyte(devinfo, inst);

      /* The MAX2(1, ...) is just to prevent possible division by 0 later. */
      const unsigned ops_per_chan =
         MAX2(1, 32 / MAX2(src1_bits_per_element, src2_bits_per_element));

      ERROR_IF(brw_inst_exec_size(devinfo, inst) != BRW_EXECUTE_8,
               "DPAS execution size must be 8.");

      const unsigned exec_size = 8;

      const unsigned dst_subnr  = brw_inst_dpas_3src_dst_subreg_nr(devinfo, inst);
      const unsigned src0_subnr = brw_inst_dpas_3src_src0_subreg_nr(devinfo, inst);
      const unsigned src1_subnr = brw_inst_dpas_3src_src1_subreg_nr(devinfo, inst);
      const unsigned src2_subnr = brw_inst_dpas_3src_src2_subreg_nr(devinfo, inst);

      /* Until HF is supported as dst type, this is effectively subnr == 0. */
      ERROR_IF(dst_subnr % exec_size != 0,
               "Destination subregister offset must be a multiple of ExecSize.");

      /* Until HF is supported as src0 type, this is effectively subnr == 0. */
      ERROR_IF(src0_subnr % exec_size != 0,
               "Src0 subregister offset must be a multiple of ExecSize.");

      ERROR_IF(src1_subnr != 0,
               "Src1 subregister offsets must be 0.");

      /* In nearly all cases, this effectively requires that src2.subnr be
       * 0. It is only when src1 is 8 bits and src2 is 2 or 4 bits that the
       * ops_per_chan value can allow non-zero src2.subnr.
       */
      ERROR_IF(src2_subnr % (sdepth * ops_per_chan) != 0,
               "Src2 subregister offset must be a multiple of SystolicDepth "
               "times OPS_PER_CHAN.");

      ERROR_IF(dst_subnr * type_sz(dst_type) >= REG_SIZE,
               "Destination subregister specifies next register.");

      ERROR_IF(src0_subnr * type_sz(src0_type) >= REG_SIZE,
               "Src0 subregister specifies next register.");

      ERROR_IF((src1_subnr * type_sz(src1_type) * src1_bits_per_element) / 8 >= REG_SIZE,
               "Src1 subregister specifies next register.");

      ERROR_IF((src2_subnr * type_sz(src2_type) * src2_bits_per_element) / 8 >= REG_SIZE,
               "Src2 subregister specifies next register.");

      if (brw_inst_3src_atomic_control(devinfo, inst)) {
         /* FINISHME: When we start emitting DPAS with Atomic set, figure out
          * a way to validate it. Also add a test in test_eu_validate.cpp.
          */
         ERROR_IF(true,
                  "When instruction option Atomic is used it must be follwed by a "
                  "DPAS instruction.");
      }

      if (brw_inst_dpas_3src_exec_type(devinfo, inst) ==
          BRW_ALIGN1_3SRC_EXEC_TYPE_FLOAT) {
         ERROR_IF(dst_type != BRW_REGISTER_TYPE_F,
                  "DPAS destination type must be F.");
         ERROR_IF(src0_type != BRW_REGISTER_TYPE_F,
                  "DPAS src0 type must be F.");
         ERROR_IF(src1_type != BRW_REGISTER_TYPE_HF,
                  "DPAS src1 type must be HF.");
         ERROR_IF(src2_type != BRW_REGISTER_TYPE_HF,
                  "DPAS src2 type must be HF.");
      } else {
         ERROR_IF(dst_type != BRW_REGISTER_TYPE_D &&
                  dst_type != BRW_REGISTER_TYPE_UD,
                  "DPAS destination type must be D or UD.");
         ERROR_IF(src0_type != BRW_REGISTER_TYPE_D &&
                  src0_type != BRW_REGISTER_TYPE_UD,
                  "DPAS src0 type must be D or UD.");
         ERROR_IF(src1_type != BRW_REGISTER_TYPE_B &&
                  src1_type != BRW_REGISTER_TYPE_UB,
                  "DPAS src1 base type must be B or UB.");
         ERROR_IF(src2_type != BRW_REGISTER_TYPE_B &&
                  src2_type != BRW_REGISTER_TYPE_UB,
                  "DPAS src2 base type must be B or UB.");

         if (brw_reg_type_is_unsigned_integer(dst_type)) {
            ERROR_IF(!brw_reg_type_is_unsigned_integer(src0_type) ||
                     !brw_reg_type_is_unsigned_integer(src1_type) ||
                     !brw_reg_type_is_unsigned_integer(src2_type),
                     "If any source datatype is signed, destination datatype "
                     "must be signed.");
         }
      }

      /* FINISHME: Additional restrictions mentioned in the Bspec that are not
       * yet enforced here:
       *
       *    - General Accumulator registers access is not supported. This is
       *      currently enforced in brw_dpas_three_src (brw_eu_emit.c).
       *
       *    - Given any combination of datatypes in the sources of a DPAS
       *      instructions, the boundaries of a register should not be crossed.
       */
   }

   return error_msg;
}

static struct string
send_descriptor_restrictions(const struct brw_isa_info *isa,
                             const brw_inst *inst)
{
   const struct intel_device_info *devinfo = isa->devinfo;
   struct string error_msg = { .str = NULL, .len = 0 };

   if (inst_is_split_send(isa, inst)) {
      /* We can only validate immediate descriptors */
      if (brw_inst_send_sel_reg32_desc(devinfo, inst))
         return error_msg;
   } else if (inst_is_send(isa, inst)) {
      /* We can only validate immediate descriptors */
      if (brw_inst_src1_reg_file(devinfo, inst) != BRW_IMMEDIATE_VALUE)
         return error_msg;
   } else {
      return error_msg;
   }

   const uint32_t desc = brw_inst_send_desc(devinfo, inst);

   switch (brw_inst_sfid(devinfo, inst)) {
   case BRW_SFID_URB:
      if (devinfo->ver < 20)
         break;
      FALLTHROUGH;
   case GFX12_SFID_TGM:
   case GFX12_SFID_SLM:
   case GFX12_SFID_UGM:
      ERROR_IF(!devinfo->has_lsc, "Platform does not support LSC");

      ERROR_IF(lsc_opcode_has_transpose(lsc_msg_desc_opcode(devinfo, desc)) &&
               lsc_msg_desc_transpose(devinfo, desc) &&
               brw_inst_exec_size(devinfo, inst) != BRW_EXECUTE_1,
               "Transposed vectors are restricted to Exec_Mask = 1.");
      break;

   default:
      break;
   }

   if (brw_inst_sfid(devinfo, inst) == BRW_SFID_URB && devinfo->ver < 20) {
      /* Gfx4 doesn't have a "header present" bit in the SEND message. */
      ERROR_IF(devinfo->ver > 4 && !brw_inst_header_present(devinfo, inst),
               "Header must be present for all URB messages.");

      switch (brw_inst_urb_opcode(devinfo, inst)) {
      case BRW_URB_OPCODE_WRITE_HWORD:
         break;

      /* case FF_SYNC: */
      case BRW_URB_OPCODE_WRITE_OWORD:
         /* Gfx5 / Gfx6 FF_SYNC message and Gfx7+ URB_WRITE_OWORD have the
          * same opcode value.
          */
         if (devinfo->ver == 5 || devinfo->ver == 6) {
            ERROR_IF(brw_inst_urb_global_offset(devinfo, inst) != 0,
                     "FF_SYNC global offset must be zero.");
            ERROR_IF(brw_inst_urb_swizzle_control(devinfo, inst) != 0,
                     "FF_SYNC swizzle control must be zero.");
            ERROR_IF(brw_inst_urb_used(devinfo, inst) != 0,
                     "FF_SYNC used must be zero.");
            ERROR_IF(brw_inst_urb_complete(devinfo, inst) != 0,
                     "FF_SYNC complete must be zero.");

            /* Volume 4 part 2 of the Sandybridge PRM (page 28) says:
             *
             *    A message response (writeback) length of 1 GRF will be
             *    indicated on the ‘send’ instruction if the thread requires
             *    response data and/or synchronization.
             */
            ERROR_IF((unsigned)brw_inst_rlen(devinfo, inst) > 1,
                     "FF_SYNC read length must be 0 or 1.");
         } else {
            ERROR_IF(devinfo->ver < 7,
                     "URB OWORD write messages only valid on gfx >= 7");
         }
         break;

      case BRW_URB_OPCODE_READ_HWORD:
      case BRW_URB_OPCODE_READ_OWORD:
         ERROR_IF(devinfo->ver < 7,
                  "URB read messages only valid on gfx >= 7");
         break;

      case GFX7_URB_OPCODE_ATOMIC_MOV:
      case GFX7_URB_OPCODE_ATOMIC_INC:
         ERROR_IF(devinfo->ver < 7,
                  "URB atomic move and increment messages only valid on gfx >= 7");
         break;

      case GFX8_URB_OPCODE_ATOMIC_ADD:
         /* The Haswell PRM lists this opcode as valid on page 317. */
         ERROR_IF(devinfo->verx10 < 75,
                  "URB atomic add message only valid on gfx >= 7.5");
         break;

      case GFX8_URB_OPCODE_SIMD8_READ:
         ERROR_IF(brw_inst_rlen(devinfo, inst) == 0,
                  "URB SIMD8 read message must read some data.");
         FALLTHROUGH;

      case GFX8_URB_OPCODE_SIMD8_WRITE:
         ERROR_IF(devinfo->ver < 8,
                  "URB SIMD8 messages only valid on gfx >= 8");
         break;

      case GFX125_URB_OPCODE_FENCE:
         ERROR_IF(devinfo->verx10 < 125,
                  "URB fence message only valid on gfx >= 12.5");
         break;

      default:
         ERROR_IF(true, "Invalid URB message");
         break;
      }
   }

   return error_msg;
}

bool
brw_validate_instruction(const struct brw_isa_info *isa,
                         const brw_inst *inst, int offset,
                         unsigned inst_size,
                         struct disasm_info *disasm)
{
   struct string error_msg = { .str = NULL, .len = 0 };

   if (is_unsupported_inst(isa, inst)) {
      ERROR("Instruction not supported on this Gen");
   } else {
      CHECK(invalid_values);

      if (error_msg.str == NULL) {
         CHECK(sources_not_null);
         CHECK(send_restrictions);
         CHECK(alignment_supported);
         CHECK(general_restrictions_based_on_operand_types);
         CHECK(general_restrictions_on_region_parameters);
         CHECK(special_restrictions_for_mixed_float_mode);
         CHECK(region_alignment_rules);
         CHECK(vector_immediate_restrictions);
         CHECK(special_requirements_for_handling_double_precision_data_types);
         CHECK(instruction_restrictions);
         CHECK(send_descriptor_restrictions);
      }
   }

   if (error_msg.str && disasm) {
      disasm_insert_error(disasm, offset, inst_size, error_msg.str);
   }
   free(error_msg.str);

   return error_msg.len == 0;
}

bool
brw_validate_instructions(const struct brw_isa_info *isa,
                          const void *assembly, int start_offset, int end_offset,
                          struct disasm_info *disasm)
{
   const struct intel_device_info *devinfo = isa->devinfo;
   bool valid = true;

   for (int src_offset = start_offset; src_offset < end_offset;) {
      const brw_inst *inst = assembly + src_offset;
      bool is_compact = brw_inst_cmpt_control(devinfo, inst);
      unsigned inst_size = is_compact ? sizeof(brw_compact_inst)
                                      : sizeof(brw_inst);
      brw_inst uncompacted;

      if (is_compact) {
         brw_compact_inst *compacted = (void *)inst;
         brw_uncompact_instruction(isa, &uncompacted, compacted);
         inst = &uncompacted;
      }

      bool v = brw_validate_instruction(isa, inst, src_offset,
                                        inst_size, disasm);
      valid = valid && v;

      src_offset += inst_size;
   }

   return valid;
}
