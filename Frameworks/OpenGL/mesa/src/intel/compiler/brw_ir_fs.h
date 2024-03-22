/* -*- c++ -*- */
/*
 * Copyright © 2010-2015 Intel Corporation
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

#ifndef BRW_IR_FS_H
#define BRW_IR_FS_H

#include "brw_shader.h"

class fs_inst;

class fs_reg : public backend_reg {
public:
   DECLARE_RALLOC_CXX_OPERATORS(fs_reg)

   void init();

   fs_reg();
   fs_reg(struct ::brw_reg reg);
   fs_reg(enum brw_reg_file file, unsigned nr);
   fs_reg(enum brw_reg_file file, unsigned nr, enum brw_reg_type type);

   bool equals(const fs_reg &r) const;
   bool negative_equals(const fs_reg &r) const;
   bool is_contiguous() const;

   /**
    * Return the size in bytes of a single logical component of the
    * register assuming the given execution width.
    */
   unsigned component_size(unsigned width) const;

   /** Register region horizontal stride */
   uint8_t stride;
};

static inline fs_reg
negate(fs_reg reg)
{
   assert(reg.file != IMM);
   reg.negate = !reg.negate;
   return reg;
}

static inline fs_reg
retype(fs_reg reg, enum brw_reg_type type)
{
   reg.type = type;
   return reg;
}

static inline fs_reg
byte_offset(fs_reg reg, unsigned delta)
{
   switch (reg.file) {
   case BAD_FILE:
      break;
   case VGRF:
   case ATTR:
   case UNIFORM:
      reg.offset += delta;
      break;
   case MRF: {
      const unsigned suboffset = reg.offset + delta;
      reg.nr += suboffset / REG_SIZE;
      reg.offset = suboffset % REG_SIZE;
      break;
   }
   case ARF:
   case FIXED_GRF: {
      const unsigned suboffset = reg.subnr + delta;
      reg.nr += suboffset / REG_SIZE;
      reg.subnr = suboffset % REG_SIZE;
      break;
   }
   case IMM:
   default:
      assert(delta == 0);
   }
   return reg;
}

static inline fs_reg
horiz_offset(const fs_reg &reg, unsigned delta)
{
   switch (reg.file) {
   case BAD_FILE:
   case UNIFORM:
   case IMM:
      /* These only have a single component that is implicitly splatted.  A
       * horizontal offset should be a harmless no-op.
       * XXX - Handle vector immediates correctly.
       */
      return reg;
   case VGRF:
   case MRF:
   case ATTR:
      return byte_offset(reg, delta * reg.stride * type_sz(reg.type));
   case ARF:
   case FIXED_GRF:
      if (reg.is_null()) {
         return reg;
      } else {
         const unsigned hstride = reg.hstride ? 1 << (reg.hstride - 1) : 0;
         const unsigned vstride = reg.vstride ? 1 << (reg.vstride - 1) : 0;
         const unsigned width = 1 << reg.width;

         if (delta % width == 0) {
            return byte_offset(reg, delta / width * vstride * type_sz(reg.type));
         } else {
            assert(vstride == hstride * width);
            return byte_offset(reg, delta * hstride * type_sz(reg.type));
         }
      }
   }
   unreachable("Invalid register file");
}

static inline fs_reg
offset(fs_reg reg, unsigned width, unsigned delta)
{
   switch (reg.file) {
   case BAD_FILE:
      break;
   case ARF:
   case FIXED_GRF:
   case MRF:
   case VGRF:
   case ATTR:
   case UNIFORM:
      return byte_offset(reg, delta * reg.component_size(width));
   case IMM:
      assert(delta == 0);
   }
   return reg;
}

/**
 * Get the scalar channel of \p reg given by \p idx and replicate it to all
 * channels of the result.
 */
static inline fs_reg
component(fs_reg reg, unsigned idx)
{
   reg = horiz_offset(reg, idx);
   reg.stride = 0;
   if (reg.file == ARF || reg.file == FIXED_GRF) {
      reg.vstride = BRW_VERTICAL_STRIDE_0;
      reg.width = BRW_WIDTH_1;
      reg.hstride = BRW_HORIZONTAL_STRIDE_0;
   }
   return reg;
}

/**
 * Return an integer identifying the discrete address space a register is
 * contained in.  A register is by definition fully contained in the single
 * reg_space it belongs to, so two registers with different reg_space ids are
 * guaranteed not to overlap.  Most register files are a single reg_space of
 * its own, only the VGRF and ATTR files are composed of multiple discrete
 * address spaces, one for each allocation and input attribute respectively.
 */
static inline uint32_t
reg_space(const fs_reg &r)
{
   return r.file << 16 | (r.file == VGRF || r.file == ATTR ? r.nr : 0);
}

/**
 * Return the base offset in bytes of a register relative to the start of its
 * reg_space().
 */
static inline unsigned
reg_offset(const fs_reg &r)
{
   return (r.file == VGRF || r.file == IMM || r.file == ATTR ? 0 : r.nr) *
          (r.file == UNIFORM ? 4 : REG_SIZE) + r.offset +
          (r.file == ARF || r.file == FIXED_GRF ? r.subnr : 0);
}

/**
 * Return the amount of padding in bytes left unused between individual
 * components of register \p r due to a (horizontal) stride value greater than
 * one, or zero if components are tightly packed in the register file.
 */
static inline unsigned
reg_padding(const fs_reg &r)
{
   const unsigned stride = ((r.file != ARF && r.file != FIXED_GRF) ? r.stride :
                            r.hstride == 0 ? 0 :
                            1 << (r.hstride - 1));
   return (MAX2(1, stride) - 1) * type_sz(r.type);
}

/* Do not call this directly. Call regions_overlap() instead. */
static inline bool
regions_overlap_MRF(const fs_reg &r, unsigned dr, const fs_reg &s, unsigned ds)
{
   if (r.nr & BRW_MRF_COMPR4) {
      fs_reg t = r;
      t.nr &= ~BRW_MRF_COMPR4;
      /* COMPR4 regions are translated by the hardware during decompression
       * into two separate half-regions 4 MRFs apart from each other.
       *
       * Note: swapping s and t in this parameter list eliminates one possible
       * level of recursion (since the s in the called versions of
       * regions_overlap_MRF can't be COMPR4), and that makes the compiled
       * code a lot smaller.
       */
      return regions_overlap_MRF(s, ds, t, dr / 2) ||
             regions_overlap_MRF(s, ds, byte_offset(t, 4 * REG_SIZE), dr / 2);
   } else if (s.nr & BRW_MRF_COMPR4) {
      return regions_overlap_MRF(s, ds, r, dr);
   }

   return !((r.nr * REG_SIZE + r.offset + dr) <= (s.nr * REG_SIZE + s.offset) ||
            (s.nr * REG_SIZE + s.offset + ds) <= (r.nr * REG_SIZE + r.offset));
}

/**
 * Return whether the register region starting at \p r and spanning \p dr
 * bytes could potentially overlap the register region starting at \p s and
 * spanning \p ds bytes.
 */
static inline bool
regions_overlap(const fs_reg &r, unsigned dr, const fs_reg &s, unsigned ds)
{
   if (r.file != s.file)
      return false;

   if (r.file == VGRF) {
      return r.nr == s.nr &&
             !(r.offset + dr <= s.offset || s.offset + ds <= r.offset);
   } else if (r.file != MRF) {
      return !(reg_offset(r) + dr <= reg_offset(s) ||
               reg_offset(s) + ds <= reg_offset(r));
   } else {
      return regions_overlap_MRF(r, dr, s, ds);
   }
}

/**
 * Check that the register region given by r [r.offset, r.offset + dr[
 * is fully contained inside the register region given by s
 * [s.offset, s.offset + ds[.
 */
static inline bool
region_contained_in(const fs_reg &r, unsigned dr, const fs_reg &s, unsigned ds)
{
   return reg_space(r) == reg_space(s) &&
          reg_offset(r) >= reg_offset(s) &&
          reg_offset(r) + dr <= reg_offset(s) + ds;
}

/**
 * Return whether the given register region is n-periodic, i.e. whether the
 * original region remains invariant after shifting it by \p n scalar
 * channels.
 */
static inline bool
is_periodic(const fs_reg &reg, unsigned n)
{
   if (reg.file == BAD_FILE || reg.is_null()) {
      return true;

   } else if (reg.file == IMM) {
      const unsigned period = (reg.type == BRW_REGISTER_TYPE_UV ||
                               reg.type == BRW_REGISTER_TYPE_V ? 8 :
                               reg.type == BRW_REGISTER_TYPE_VF ? 4 :
                               1);
      return n % period == 0;

   } else if (reg.file == ARF || reg.file == FIXED_GRF) {
      const unsigned period = (reg.hstride == 0 && reg.vstride == 0 ? 1 :
                               reg.vstride == 0 ? 1 << reg.width :
                               ~0);
      return n % period == 0;

   } else {
      return reg.stride == 0;
   }
}

static inline bool
is_uniform(const fs_reg &reg)
{
   return is_periodic(reg, 1);
}

/**
 * Get the specified 8-component quarter of a register.
 */
static inline fs_reg
quarter(const fs_reg &reg, unsigned idx)
{
   assert(idx < 4);
   return horiz_offset(reg, 8 * idx);
}

/**
 * Reinterpret each channel of register \p reg as a vector of values of the
 * given smaller type and take the i-th subcomponent from each.
 */
static inline fs_reg
subscript(fs_reg reg, brw_reg_type type, unsigned i)
{
   assert((i + 1) * type_sz(type) <= type_sz(reg.type));

   if (reg.file == ARF || reg.file == FIXED_GRF) {
      /* The stride is encoded inconsistently for fixed GRF and ARF registers
       * as the log2 of the actual vertical and horizontal strides.
       */
      const int delta = util_logbase2(type_sz(reg.type)) -
                        util_logbase2(type_sz(type));
      reg.hstride += (reg.hstride ? delta : 0);
      reg.vstride += (reg.vstride ? delta : 0);

   } else if (reg.file == IMM) {
      unsigned bit_size = type_sz(type) * 8;
      reg.u64 >>= i * bit_size;
      reg.u64 &= BITFIELD64_MASK(bit_size);
      if (bit_size <= 16)
         reg.u64 |= reg.u64 << 16;
      return retype(reg, type);
   } else {
      reg.stride *= type_sz(reg.type) / type_sz(type);
   }

   return byte_offset(retype(reg, type), i * type_sz(type));
}

static inline fs_reg
horiz_stride(fs_reg reg, unsigned s)
{
   reg.stride *= s;
   return reg;
}

static const fs_reg reg_undef;

class fs_inst : public backend_instruction {
   fs_inst &operator=(const fs_inst &);

   void init(enum opcode opcode, uint8_t exec_width, const fs_reg &dst,
             const fs_reg *src, unsigned sources);

public:
   DECLARE_RALLOC_CXX_OPERATORS(fs_inst)

   fs_inst();
   fs_inst(enum opcode opcode, uint8_t exec_size);
   fs_inst(enum opcode opcode, uint8_t exec_size, const fs_reg &dst);
   fs_inst(enum opcode opcode, uint8_t exec_size, const fs_reg &dst,
           const fs_reg &src0);
   fs_inst(enum opcode opcode, uint8_t exec_size, const fs_reg &dst,
           const fs_reg &src0, const fs_reg &src1);
   fs_inst(enum opcode opcode, uint8_t exec_size, const fs_reg &dst,
           const fs_reg &src0, const fs_reg &src1, const fs_reg &src2);
   fs_inst(enum opcode opcode, uint8_t exec_size, const fs_reg &dst,
           const fs_reg src[], unsigned sources);
   fs_inst(const fs_inst &that);
   ~fs_inst();

   void resize_sources(uint8_t num_sources);

   bool is_send_from_grf() const;
   bool is_payload(unsigned arg) const;
   bool is_partial_write() const;
   unsigned components_read(unsigned i) const;
   unsigned size_read(int arg) const;
   bool can_do_source_mods(const struct intel_device_info *devinfo) const;
   bool can_do_cmod();
   bool can_change_types() const;
   bool has_source_and_destination_hazard() const;
   unsigned implied_mrf_writes() const;

   /**
    * Return whether \p arg is a control source of a virtual instruction which
    * shouldn't contribute to the execution type and usual regioning
    * restriction calculations of arithmetic instructions.
    */
   bool is_control_source(unsigned arg) const;

   /**
    * Return the subset of flag registers read by the instruction as a bitset
    * with byte granularity.
    */
   unsigned flags_read(const intel_device_info *devinfo) const;

   /**
    * Return the subset of flag registers updated by the instruction (either
    * partially or fully) as a bitset with byte granularity.
    */
   unsigned flags_written(const intel_device_info *devinfo) const;

   /**
    * Return true if this instruction is a sampler message gathering residency
    * data.
    */
   bool has_sampler_residency() const;

   fs_reg dst;
   fs_reg *src;

   uint8_t sources; /**< Number of fs_reg sources. */

   bool last_rt:1;
   bool pi_noperspective:1;   /**< Pixel interpolator noperspective flag */
   bool keep_payload_trailing_zeros;

   tgl_swsb sched; /**< Scheduling info. */
};

/**
 * Make the execution of \p inst dependent on the evaluation of a possibly
 * inverted predicate.
 */
static inline fs_inst *
set_predicate_inv(enum brw_predicate pred, bool inverse,
                  fs_inst *inst)
{
   inst->predicate = pred;
   inst->predicate_inverse = inverse;
   return inst;
}

/**
 * Make the execution of \p inst dependent on the evaluation of a predicate.
 */
static inline fs_inst *
set_predicate(enum brw_predicate pred, fs_inst *inst)
{
   return set_predicate_inv(pred, false, inst);
}

/**
 * Write the result of evaluating the condition given by \p mod to a flag
 * register.
 */
static inline fs_inst *
set_condmod(enum brw_conditional_mod mod, fs_inst *inst)
{
   inst->conditional_mod = mod;
   return inst;
}

/**
 * Clamp the result of \p inst to the saturation range of its destination
 * datatype.
 */
static inline fs_inst *
set_saturate(bool saturate, fs_inst *inst)
{
   inst->saturate = saturate;
   return inst;
}

/**
 * Return the number of dataflow registers written by the instruction (either
 * fully or partially) counted from 'floor(reg_offset(inst->dst) /
 * register_size)'.  The somewhat arbitrary register size unit is 4B for the
 * UNIFORM and IMM files and 32B for all other files.
 */
inline unsigned
regs_written(const fs_inst *inst)
{
   assert(inst->dst.file != UNIFORM && inst->dst.file != IMM);
   return DIV_ROUND_UP(reg_offset(inst->dst) % REG_SIZE +
                       inst->size_written -
                       MIN2(inst->size_written, reg_padding(inst->dst)),
                       REG_SIZE);
}

/**
 * Return the number of dataflow registers read by the instruction (either
 * fully or partially) counted from 'floor(reg_offset(inst->src[i]) /
 * register_size)'.  The somewhat arbitrary register size unit is 4B for the
 * UNIFORM files and 32B for all other files.
 */
inline unsigned
regs_read(const fs_inst *inst, unsigned i)
{
   if (inst->src[i].file == IMM)
      return 1;

   const unsigned reg_size = inst->src[i].file == UNIFORM ? 4 : REG_SIZE;
   return DIV_ROUND_UP(reg_offset(inst->src[i]) % reg_size +
                       inst->size_read(i) -
                       MIN2(inst->size_read(i), reg_padding(inst->src[i])),
                       reg_size);
}

static inline enum brw_reg_type
get_exec_type(const fs_inst *inst)
{
   brw_reg_type exec_type = BRW_REGISTER_TYPE_B;

   for (int i = 0; i < inst->sources; i++) {
      if (inst->src[i].file != BAD_FILE &&
          !inst->is_control_source(i)) {
         const brw_reg_type t = get_exec_type(inst->src[i].type);
         if (type_sz(t) > type_sz(exec_type))
            exec_type = t;
         else if (type_sz(t) == type_sz(exec_type) &&
                  brw_reg_type_is_floating_point(t))
            exec_type = t;
      }
   }

   if (exec_type == BRW_REGISTER_TYPE_B)
      exec_type = inst->dst.type;

   assert(exec_type != BRW_REGISTER_TYPE_B);

   /* Promotion of the execution type to 32-bit for conversions from or to
    * half-float seems to be consistent with the following text from the
    * Cherryview PRM Vol. 7, "Execution Data Type":
    *
    * "When single precision and half precision floats are mixed between
    *  source operands or between source and destination operand [..] single
    *  precision float is the execution datatype."
    *
    * and from "Register Region Restrictions":
    *
    * "Conversion between Integer and HF (Half Float) must be DWord aligned
    *  and strided by a DWord on the destination."
    */
   if (type_sz(exec_type) == 2 &&
       inst->dst.type != exec_type) {
      if (exec_type == BRW_REGISTER_TYPE_HF)
         exec_type = BRW_REGISTER_TYPE_F;
      else if (inst->dst.type == BRW_REGISTER_TYPE_HF)
         exec_type = BRW_REGISTER_TYPE_D;
   }

   return exec_type;
}

static inline unsigned
get_exec_type_size(const fs_inst *inst)
{
   return type_sz(get_exec_type(inst));
}

static inline bool
is_send(const fs_inst *inst)
{
   return inst->mlen || inst->is_send_from_grf();
}

/**
 * Return whether the instruction isn't an ALU instruction and cannot be
 * assumed to complete in-order.
 */
static inline bool
is_unordered(const intel_device_info *devinfo, const fs_inst *inst)
{
   return is_send(inst) || (devinfo->ver < 20 && inst->is_math()) ||
          inst->opcode == BRW_OPCODE_DPAS ||
          (devinfo->has_64bit_float_via_math_pipe &&
           (get_exec_type(inst) == BRW_REGISTER_TYPE_DF ||
            inst->dst.type == BRW_REGISTER_TYPE_DF));
}

/**
 * Return whether the following regioning restriction applies to the specified
 * instruction.  From the Cherryview PRM Vol 7. "Register Region
 * Restrictions":
 *
 * "When source or destination datatype is 64b or operation is integer DWord
 *  multiply, regioning in Align1 must follow these rules:
 *
 *  1. Source and Destination horizontal stride must be aligned to the same qword.
 *  2. Regioning must ensure Src.Vstride = Src.Width * Src.Hstride.
 *  3. Source and Destination offset must be the same, except the case of
 *     scalar source."
 */
static inline bool
has_dst_aligned_region_restriction(const intel_device_info *devinfo,
                                   const fs_inst *inst,
                                   brw_reg_type dst_type)
{
   const brw_reg_type exec_type = get_exec_type(inst);
   /* Even though the hardware spec claims that "integer DWord multiply"
    * operations are restricted, empirical evidence and the behavior of the
    * simulator suggest that only 32x32-bit integer multiplication is
    * restricted.
    */
   const bool is_dword_multiply = !brw_reg_type_is_floating_point(exec_type) &&
      ((inst->opcode == BRW_OPCODE_MUL &&
        MIN2(type_sz(inst->src[0].type), type_sz(inst->src[1].type)) >= 4) ||
       (inst->opcode == BRW_OPCODE_MAD &&
        MIN2(type_sz(inst->src[1].type), type_sz(inst->src[2].type)) >= 4));

   if (type_sz(dst_type) > 4 || type_sz(exec_type) > 4 ||
       (type_sz(exec_type) == 4 && is_dword_multiply))
      return devinfo->platform == INTEL_PLATFORM_CHV ||
             intel_device_info_is_9lp(devinfo) ||
             devinfo->verx10 >= 125;

   else if (brw_reg_type_is_floating_point(dst_type))
      return devinfo->verx10 >= 125;

   else
      return false;
}

static inline bool
has_dst_aligned_region_restriction(const intel_device_info *devinfo,
                                   const fs_inst *inst)
{
   return has_dst_aligned_region_restriction(devinfo, inst, inst->dst.type);
}

/**
 * Return whether the LOAD_PAYLOAD instruction is a plain copy of bits from
 * the specified register file into a VGRF.
 *
 * This implies identity register regions without any source-destination
 * overlap, but otherwise has no implications on the location of sources and
 * destination in the register file: Gathering any number of portions from
 * multiple virtual registers in any order is allowed.
 */
inline bool
is_copy_payload(brw_reg_file file, const fs_inst *inst)
{
   if (inst->opcode != SHADER_OPCODE_LOAD_PAYLOAD ||
       inst->is_partial_write() || inst->saturate ||
       inst->dst.file != VGRF)
      return false;

   for (unsigned i = 0; i < inst->sources; i++) {
      if (inst->src[i].file != file ||
          inst->src[i].abs || inst->src[i].negate)
         return false;

      if (!inst->src[i].is_contiguous())
         return false;

      if (regions_overlap(inst->dst, inst->size_written,
                          inst->src[i], inst->size_read(i)))
         return false;
   }

   return true;
}

/**
 * Like is_copy_payload(), but the instruction is required to copy a single
 * contiguous block of registers from the given register file into the
 * destination without any reordering.
 */
inline bool
is_identity_payload(brw_reg_file file, const fs_inst *inst) {
   if (is_copy_payload(file, inst)) {
      fs_reg reg = inst->src[0];

      for (unsigned i = 0; i < inst->sources; i++) {
         reg.type = inst->src[i].type;
         if (!inst->src[i].equals(reg))
            return false;

         reg = byte_offset(reg, inst->size_read(i));
      }

      return true;
   } else {
      return false;
   }
}

/**
 * Like is_copy_payload(), but the instruction is required to source data from
 * at least two disjoint VGRFs.
 *
 * This doesn't necessarily rule out the elimination of this instruction
 * through register coalescing, but due to limitations of the register
 * coalesce pass it might be impossible to do so directly until a later stage,
 * when the LOAD_PAYLOAD instruction is unrolled into a sequence of MOV
 * instructions.
 */
inline bool
is_multi_copy_payload(const fs_inst *inst) {
   if (is_copy_payload(VGRF, inst)) {
      for (unsigned i = 0; i < inst->sources; i++) {
            if (inst->src[i].nr != inst->src[0].nr)
               return true;
      }
   }

   return false;
}

/**
 * Like is_identity_payload(), but the instruction is required to copy the
 * whole contents of a single VGRF into the destination.
 *
 * This means that there is a good chance that the instruction will be
 * eliminated through register coalescing, but it's neither a necessary nor a
 * sufficient condition for that to happen -- E.g. consider the case where
 * source and destination registers diverge due to other instructions in the
 * program overwriting part of their contents, which isn't something we can
 * predict up front based on a cheap strictly local test of the copy
 * instruction.
 */
inline bool
is_coalescing_payload(const brw::simple_allocator &alloc, const fs_inst *inst)
{
   return is_identity_payload(VGRF, inst) &&
          inst->src[0].offset == 0 &&
          alloc.sizes[inst->src[0].nr] * REG_SIZE == inst->size_written;
}

bool
has_bank_conflict(const struct brw_isa_info *isa, const fs_inst *inst);

#endif
