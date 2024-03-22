/* -*- c++ -*- */
/*
 * Copyright © 2010-2016 Intel Corporation
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

#ifndef BRW_IR_H
#define BRW_IR_H

#include <assert.h>
#include "brw_reg.h"
#include "compiler/glsl/list.h"

#define MAX_SAMPLER_MESSAGE_SIZE 11

/* The sampler can return a vec5 when sampling with sparse residency. In
 * SIMD32, each component takes up 4 GRFs, so we need to allow up to size-20
 * VGRFs to hold the result.
 */
#define MAX_VGRF_SIZE(devinfo) ((devinfo)->ver >= 20 ? 40 : 20)

#ifdef __cplusplus
struct backend_reg : private brw_reg
{
   backend_reg() {}
   backend_reg(const struct brw_reg &reg) : brw_reg(reg), offset(0) {}

   const brw_reg &as_brw_reg() const
   {
      assert(file == ARF || file == FIXED_GRF || file == MRF || file == IMM);
      assert(offset == 0);
      return static_cast<const brw_reg &>(*this);
   }

   brw_reg &as_brw_reg()
   {
      assert(file == ARF || file == FIXED_GRF || file == MRF || file == IMM);
      assert(offset == 0);
      return static_cast<brw_reg &>(*this);
   }

   bool equals(const backend_reg &r) const;
   bool negative_equals(const backend_reg &r) const;

   bool is_zero() const;
   bool is_one() const;
   bool is_negative_one() const;
   bool is_null() const;
   bool is_accumulator() const;

   /** Offset from the start of the (virtual) register in bytes. */
   uint16_t offset;

   using brw_reg::type;
   using brw_reg::file;
   using brw_reg::negate;
   using brw_reg::abs;
   using brw_reg::address_mode;
   using brw_reg::subnr;
   using brw_reg::nr;

   using brw_reg::swizzle;
   using brw_reg::writemask;
   using brw_reg::indirect_offset;
   using brw_reg::vstride;
   using brw_reg::width;
   using brw_reg::hstride;

   using brw_reg::df;
   using brw_reg::f;
   using brw_reg::d;
   using brw_reg::ud;
   using brw_reg::d64;
   using brw_reg::u64;
};

struct bblock_t;

struct backend_instruction : public exec_node {
   bool is_3src(const struct brw_compiler *compiler) const;
   bool is_math() const;
   bool is_control_flow_begin() const;
   bool is_control_flow_end() const;
   bool is_control_flow() const;
   bool is_commutative() const;
   bool can_do_source_mods() const;
   bool can_do_saturate() const;
   bool can_do_cmod() const;
   bool reads_accumulator_implicitly() const;
   bool writes_accumulator_implicitly(const struct intel_device_info *devinfo) const;

   /**
    * Instructions that use indirect addressing have additional register
    * regioning restrictions.
    */
   bool uses_indirect_addressing() const;

   void remove(bblock_t *block, bool defer_later_block_ip_updates = false);
   void insert_after(bblock_t *block, backend_instruction *inst);
   void insert_before(bblock_t *block, backend_instruction *inst);

   /**
    * True if the instruction has side effects other than writing to
    * its destination registers.  You are expected not to reorder or
    * optimize these out unless you know what you are doing.
    */
   bool has_side_effects() const;

   /**
    * True if the instruction might be affected by side effects of other
    * instructions.
    */
   bool is_volatile() const;
#else
struct backend_instruction {
   struct exec_node link;
#endif
   /** @{
    * Annotation for the generated IR.  One of the two can be set.
    */
   const void *ir;
   const char *annotation;
   /** @} */

   /**
    * Execution size of the instruction.  This is used by the generator to
    * generate the correct binary for the given instruction.  Current valid
    * values are 1, 4, 8, 16, 32.
    */
   uint8_t exec_size;

   /**
    * Channel group from the hardware execution and predication mask that
    * should be applied to the instruction.  The subset of channel enable
    * signals (calculated from the EU control flow and predication state)
    * given by [group, group + exec_size) will be used to mask GRF writes and
    * any other side effects of the instruction.
    */
   uint8_t group;

   uint32_t offset; /**< spill/unspill offset or texture offset bitfield */
   uint8_t mlen; /**< SEND message length */
   uint8_t ex_mlen; /**< SENDS extended message length */
   int8_t base_mrf; /**< First MRF in the SEND message, if mlen is nonzero. */
   uint8_t target; /**< MRT target. */
   uint8_t sfid; /**< SFID for SEND instructions */
   uint32_t desc; /**< SEND[S] message descriptor immediate */
   uint32_t ex_desc; /**< SEND[S] extended message descriptor immediate */
   unsigned size_written; /**< Data written to the destination register in bytes. */

   enum opcode opcode; /* BRW_OPCODE_* or FS_OPCODE_* */
   enum brw_conditional_mod conditional_mod; /**< BRW_CONDITIONAL_* */
   enum brw_predicate predicate;
   bool predicate_inverse:1;
   bool writes_accumulator:1; /**< instruction implicitly writes accumulator */
   bool force_writemask_all:1;
   bool no_dd_clear:1;
   bool no_dd_check:1;
   bool saturate:1;
   bool shadow_compare:1;
   bool check_tdr:1; /**< Only valid for SEND; turns it into a SENDC */
   bool send_has_side_effects:1; /**< Only valid for SHADER_OPCODE_SEND */
   bool send_is_volatile:1; /**< Only valid for SHADER_OPCODE_SEND */
   bool send_ex_desc_scratch:1; /**< Only valid for SHADER_OPCODE_SEND, use
                                 *   the scratch surface offset to build
                                 *   extended descriptor
                                 */
   bool send_ex_bso:1; /**< Only for SHADER_OPCODE_SEND, use extended bindless
                        *   surface offset (26bits instead of 20bits)
                        */
   bool predicate_trivial:1; /**< The predication mask applied to this
                              *   instruction is guaranteed to be uniform and
                              *   a superset of the execution mask of the
                              *   present block, no currently enabled channels
                              *   will be disabled by the predicate.
                              */
   bool eot:1;

   /* Chooses which flag subregister (f0.0 to f1.1) is used for conditional
    * mod and predication.
    */
   unsigned flag_subreg:2;

   /**
    * Systolic depth used by DPAS instruction.
    */
   unsigned sdepth:4;

   /**
    * Repeat count used by DPAS instruction.
    */
   unsigned rcount:4;

   /** The number of hardware registers used for a message header. */
   uint8_t header_size;
};

#endif
