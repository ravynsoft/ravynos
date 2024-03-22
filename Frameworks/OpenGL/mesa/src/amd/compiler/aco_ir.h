/*
 * Copyright © 2018 Valve Corporation
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

#ifndef ACO_IR_H
#define ACO_IR_H

#include "aco_opcodes.h"
#include "aco_shader_info.h"
#include "aco_util.h"

#include "util/compiler.h"

#include "ac_binary.h"
#include "ac_hw_stage.h"
#include "amd_family.h"
#include <algorithm>
#include <bitset>
#include <memory>
#include <vector>

typedef struct nir_shader nir_shader;

namespace aco {

extern uint64_t debug_flags;

enum {
   DEBUG_VALIDATE_IR = 0x1,
   DEBUG_VALIDATE_RA = 0x2,
   DEBUG_PERFWARN = 0x4,
   DEBUG_FORCE_WAITCNT = 0x8,
   DEBUG_NO_VN = 0x10,
   DEBUG_NO_OPT = 0x20,
   DEBUG_NO_SCHED = 0x40,
   DEBUG_PERF_INFO = 0x80,
   DEBUG_LIVE_INFO = 0x100,
   DEBUG_FORCE_WAITDEPS = 0x200,
   DEBUG_NO_VALIDATE_IR = 0x400,
   DEBUG_NO_SCHED_ILP = 0x800,
};

enum storage_class : uint8_t {
   storage_none = 0x0,   /* no synchronization and can be reordered around aliasing stores */
   storage_buffer = 0x1, /* SSBOs and global memory */
   storage_gds = 0x2,
   storage_image = 0x4,
   storage_shared = 0x8,        /* or TCS output */
   storage_vmem_output = 0x10,  /* GS or TCS output stores using VMEM */
   storage_task_payload = 0x20, /* Task-Mesh payload */
   storage_scratch = 0x40,
   storage_vgpr_spill = 0x80,
   storage_count = 8, /* not counting storage_none */
};

enum memory_semantics : uint8_t {
   semantic_none = 0x0,
   /* for loads: don't move any access after this load to before this load (even other loads)
    * for barriers: don't move any access after the barrier to before any
    * atomics/control_barriers/sendmsg_gs_done/position-primitive-export before the barrier */
   semantic_acquire = 0x1,
   /* for stores: don't move any access before this store to after this store
    * for barriers: don't move any access before the barrier to after any
    * atomics/control_barriers/sendmsg_gs_done/position-primitive-export after the barrier */
   semantic_release = 0x2,

   /* the rest are for load/stores/atomics only */
   /* cannot be DCE'd or CSE'd */
   semantic_volatile = 0x4,
   /* does not interact with barriers and assumes this lane is the only lane
    * accessing this memory */
   semantic_private = 0x8,
   /* this operation can be reordered around operations of the same storage.
    * says nothing about barriers */
   semantic_can_reorder = 0x10,
   /* this is a atomic instruction (may only read or write memory) */
   semantic_atomic = 0x20,
   /* this is instruction both reads and writes memory */
   semantic_rmw = 0x40,

   semantic_acqrel = semantic_acquire | semantic_release,
   semantic_atomicrmw = semantic_volatile | semantic_atomic | semantic_rmw,
};

enum sync_scope : uint8_t {
   scope_invocation = 0,
   scope_subgroup = 1,
   scope_workgroup = 2,
   scope_queuefamily = 3,
   scope_device = 4,
};

struct memory_sync_info {
   memory_sync_info() : storage(storage_none), semantics(semantic_none), scope(scope_invocation) {}
   memory_sync_info(int storage_, int semantics_ = 0, sync_scope scope_ = scope_invocation)
       : storage((storage_class)storage_), semantics((memory_semantics)semantics_), scope(scope_)
   {}

   storage_class storage : 8;
   memory_semantics semantics : 8;
   sync_scope scope : 8;

   bool operator==(const memory_sync_info& rhs) const
   {
      return storage == rhs.storage && semantics == rhs.semantics && scope == rhs.scope;
   }

   bool can_reorder() const
   {
      if (semantics & semantic_acqrel)
         return false;
      /* Also check storage so that zero-initialized memory_sync_info can be
       * reordered. */
      return (!storage || (semantics & semantic_can_reorder)) && !(semantics & semantic_volatile);
   }
};
static_assert(sizeof(memory_sync_info) == 3, "Unexpected padding");

enum fp_round {
   fp_round_ne = 0,
   fp_round_pi = 1,
   fp_round_ni = 2,
   fp_round_tz = 3,
};

enum fp_denorm {
   /* Note that v_rcp_f32, v_exp_f32, v_log_f32, v_sqrt_f32, v_rsq_f32 and
    * v_mad_f32/v_madak_f32/v_madmk_f32/v_mac_f32 always flush denormals. */
   fp_denorm_flush = 0x0,
   fp_denorm_keep_in = 0x1,
   fp_denorm_keep_out = 0x2,
   fp_denorm_keep = 0x3,
};

struct float_mode {
   /* matches encoding of the MODE register */
   union {
      struct {
         fp_round round32 : 2;
         fp_round round16_64 : 2;
         unsigned denorm32 : 2;
         unsigned denorm16_64 : 2;
      };
      struct {
         uint8_t round : 4;
         uint8_t denorm : 4;
      };
      uint8_t val = 0;
   };
   /* if false, optimizations which may remove infs/nan/-0.0 can be done */
   bool preserve_signed_zero_inf_nan32 : 1;
   bool preserve_signed_zero_inf_nan16_64 : 1;
   /* if false, optimizations which may remove denormal flushing can be done */
   bool must_flush_denorms32 : 1;
   bool must_flush_denorms16_64 : 1;
   bool care_about_round32 : 1;
   bool care_about_round16_64 : 1;

   /* Returns true if instructions using the mode "other" can safely use the
    * current one instead. */
   bool canReplace(float_mode other) const noexcept
   {
      return val == other.val &&
             (preserve_signed_zero_inf_nan32 || !other.preserve_signed_zero_inf_nan32) &&
             (preserve_signed_zero_inf_nan16_64 || !other.preserve_signed_zero_inf_nan16_64) &&
             (must_flush_denorms32 || !other.must_flush_denorms32) &&
             (must_flush_denorms16_64 || !other.must_flush_denorms16_64) &&
             (care_about_round32 || !other.care_about_round32) &&
             (care_about_round16_64 || !other.care_about_round16_64);
   }
};

struct wait_imm {
   static const uint8_t unset_counter = 0xff;

   uint8_t vm;
   uint8_t exp;
   uint8_t lgkm;
   uint8_t vs;

   wait_imm();
   wait_imm(uint16_t vm_, uint16_t exp_, uint16_t lgkm_, uint16_t vs_);
   wait_imm(enum amd_gfx_level chip, uint16_t packed);

   uint16_t pack(enum amd_gfx_level chip) const;

   bool combine(const wait_imm& other);

   bool empty() const;

   void print(FILE* output) const;
};

/* s_wait_event immediate bits. */
enum wait_event_imm : uint16_t {
   /* If this bit is 0, await that the export buffer space has been allocated.
    * In Primitive Ordered Pixel Shading, export ready means that the overlapped waves have exited
    * their ordered sections (by performing the `done` export), and that the current wave may enter
    * its ordered section.
    */
   wait_event_imm_dont_wait_export_ready = 0x1,
};

constexpr Format
asVOP3(Format format)
{
   return (Format)((uint32_t)Format::VOP3 | (uint32_t)format);
};

constexpr Format
asSDWA(Format format)
{
   assert(format == Format::VOP1 || format == Format::VOP2 || format == Format::VOPC);
   return (Format)((uint32_t)Format::SDWA | (uint32_t)format);
}

constexpr Format
withoutDPP(Format format)
{
   return (Format)((uint32_t)format & ~((uint32_t)Format::DPP16 | (uint32_t)Format::DPP8));
}

constexpr Format
withoutVOP3(Format format)
{
   return (Format)((uint32_t)format & ~((uint32_t)Format::VOP3));
}

enum class RegType {
   sgpr,
   vgpr,
};

struct RegClass {

   enum RC : uint8_t {
      s1 = 1,
      s2 = 2,
      s3 = 3,
      s4 = 4,
      s6 = 6,
      s8 = 8,
      s16 = 16,
      v1 = s1 | (1 << 5),
      v2 = s2 | (1 << 5),
      v3 = s3 | (1 << 5),
      v4 = s4 | (1 << 5),
      v5 = 5 | (1 << 5),
      v6 = 6 | (1 << 5),
      v7 = 7 | (1 << 5),
      v8 = 8 | (1 << 5),
      /* byte-sized register class */
      v1b = v1 | (1 << 7),
      v2b = v2 | (1 << 7),
      v3b = v3 | (1 << 7),
      v4b = v4 | (1 << 7),
      v6b = v6 | (1 << 7),
      v8b = v8 | (1 << 7),
      /* these are used for WWM and spills to vgpr */
      v1_linear = v1 | (1 << 6),
      v2_linear = v2 | (1 << 6),
   };

   RegClass() = default;
   constexpr RegClass(RC rc_) : rc(rc_) {}
   constexpr RegClass(RegType type, unsigned size)
       : rc((RC)((type == RegType::vgpr ? 1 << 5 : 0) | size))
   {}

   constexpr operator RC() const { return rc; }
   explicit operator bool() = delete;

   constexpr RegType type() const { return rc <= RC::s16 ? RegType::sgpr : RegType::vgpr; }
   constexpr bool is_linear_vgpr() const { return rc & (1 << 6); };
   constexpr bool is_subdword() const { return rc & (1 << 7); }
   constexpr unsigned bytes() const { return ((unsigned)rc & 0x1F) * (is_subdword() ? 1 : 4); }
   // TODO: use size() less in favor of bytes()
   constexpr unsigned size() const { return (bytes() + 3) >> 2; }
   constexpr bool is_linear() const { return rc <= RC::s16 || is_linear_vgpr(); }
   constexpr RegClass as_linear() const { return RegClass((RC)(rc | (1 << 6))); }
   constexpr RegClass as_subdword() const { return RegClass((RC)(rc | 1 << 7)); }

   static constexpr RegClass get(RegType type, unsigned bytes)
   {
      if (type == RegType::sgpr) {
         return RegClass(type, DIV_ROUND_UP(bytes, 4u));
      } else {
         return bytes % 4u ? RegClass(type, bytes).as_subdword() : RegClass(type, bytes / 4u);
      }
   }

   constexpr RegClass resize(unsigned bytes) const
   {
      if (is_linear_vgpr()) {
         assert(bytes % 4u == 0);
         return get(RegType::vgpr, bytes).as_linear();
      }
      return get(type(), bytes);
   }

private:
   RC rc;
};

/* transitional helper expressions */
static constexpr RegClass s1{RegClass::s1};
static constexpr RegClass s2{RegClass::s2};
static constexpr RegClass s3{RegClass::s3};
static constexpr RegClass s4{RegClass::s4};
static constexpr RegClass s8{RegClass::s8};
static constexpr RegClass s16{RegClass::s16};
static constexpr RegClass v1{RegClass::v1};
static constexpr RegClass v2{RegClass::v2};
static constexpr RegClass v3{RegClass::v3};
static constexpr RegClass v4{RegClass::v4};
static constexpr RegClass v5{RegClass::v5};
static constexpr RegClass v6{RegClass::v6};
static constexpr RegClass v7{RegClass::v7};
static constexpr RegClass v8{RegClass::v8};
static constexpr RegClass v1b{RegClass::v1b};
static constexpr RegClass v2b{RegClass::v2b};
static constexpr RegClass v3b{RegClass::v3b};
static constexpr RegClass v4b{RegClass::v4b};
static constexpr RegClass v6b{RegClass::v6b};
static constexpr RegClass v8b{RegClass::v8b};

/**
 * Temp Class
 * Each temporary virtual register has a
 * register class (i.e. size and type)
 * and SSA id.
 */
struct Temp {
   Temp() noexcept : id_(0), reg_class(0) {}
   constexpr Temp(uint32_t id, RegClass cls) noexcept : id_(id), reg_class(uint8_t(cls)) {}

   constexpr uint32_t id() const noexcept { return id_; }
   constexpr RegClass regClass() const noexcept { return (RegClass::RC)reg_class; }

   constexpr unsigned bytes() const noexcept { return regClass().bytes(); }
   constexpr unsigned size() const noexcept { return regClass().size(); }
   constexpr RegType type() const noexcept { return regClass().type(); }
   constexpr bool is_linear() const noexcept { return regClass().is_linear(); }

   constexpr bool operator<(Temp other) const noexcept { return id() < other.id(); }
   constexpr bool operator==(Temp other) const noexcept { return id() == other.id(); }
   constexpr bool operator!=(Temp other) const noexcept { return id() != other.id(); }

private:
   uint32_t id_ : 24;
   uint32_t reg_class : 8;
};

/**
 * PhysReg
 * Represents the physical register for each
 * Operand and Definition.
 */
struct PhysReg {
   constexpr PhysReg() = default;
   explicit constexpr PhysReg(unsigned r) : reg_b(r << 2) {}
   constexpr unsigned reg() const { return reg_b >> 2; }
   constexpr unsigned byte() const { return reg_b & 0x3; }
   constexpr operator unsigned() const { return reg(); }
   constexpr bool operator==(PhysReg other) const { return reg_b == other.reg_b; }
   constexpr bool operator!=(PhysReg other) const { return reg_b != other.reg_b; }
   constexpr bool operator<(PhysReg other) const { return reg_b < other.reg_b; }
   constexpr PhysReg advance(int bytes) const
   {
      PhysReg res = *this;
      res.reg_b += bytes;
      return res;
   }

   uint16_t reg_b = 0;
};

/* helper expressions for special registers */
static constexpr PhysReg m0{124};
static constexpr PhysReg flat_scr_lo{102}; /* GFX8-GFX9, encoded differently on GFX6-7 */
static constexpr PhysReg flat_scr_hi{103}; /* GFX8-GFX9, encoded differently on GFX6-7 */
static constexpr PhysReg vcc{106};
static constexpr PhysReg vcc_hi{107};
static constexpr PhysReg tba{108}; /* GFX6-GFX8 */
static constexpr PhysReg tma{110}; /* GFX6-GFX8 */
static constexpr PhysReg ttmp0{112};
static constexpr PhysReg ttmp1{113};
static constexpr PhysReg ttmp2{114};
static constexpr PhysReg ttmp3{115};
static constexpr PhysReg ttmp4{116};
static constexpr PhysReg ttmp5{117};
static constexpr PhysReg ttmp6{118};
static constexpr PhysReg ttmp7{119};
static constexpr PhysReg ttmp8{120};
static constexpr PhysReg ttmp9{121};
static constexpr PhysReg ttmp10{122};
static constexpr PhysReg ttmp11{123};
static constexpr PhysReg sgpr_null{125}; /* GFX10+ */
static constexpr PhysReg exec{126};
static constexpr PhysReg exec_lo{126};
static constexpr PhysReg exec_hi{127};
static constexpr PhysReg pops_exiting_wave_id{239}; /* GFX9-GFX10.3 */
static constexpr PhysReg scc{253};

/**
 * Operand Class
 * Initially, each Operand refers to either
 * a temporary virtual register
 * or to a constant value
 * Temporary registers get mapped to physical register during RA
 * Constant values are inlined into the instruction sequence.
 */
class Operand final {
public:
   constexpr Operand()
       : reg_(PhysReg{128}), isTemp_(false), isFixed_(true), isConstant_(false), isKill_(false),
         isUndef_(true), isFirstKill_(false), constSize(0), isLateKill_(false), is16bit_(false),
         is24bit_(false), signext(false)
   {}

   explicit Operand(Temp r) noexcept
   {
      data_.temp = r;
      if (r.id()) {
         isTemp_ = true;
      } else {
         isUndef_ = true;
         setFixed(PhysReg{128});
      }
   };
   explicit Operand(Temp r, PhysReg reg) noexcept
   {
      assert(r.id()); /* Don't allow fixing an undef to a register */
      data_.temp = r;
      isTemp_ = true;
      setFixed(reg);
   };

   /* 8-bit constant */
   static Operand c8(uint8_t v) noexcept
   {
      /* 8-bit constants are only used for copies and copies from any 8-bit
       * constant can be implemented with a SDWA v_mul_u32_u24. So consider all
       * to be inline constants. */
      Operand op;
      op.control_ = 0;
      op.data_.i = v;
      op.isConstant_ = true;
      op.constSize = 0;
      op.setFixed(PhysReg{0u});
      return op;
   };

   /* 16-bit constant */
   static Operand c16(uint16_t v) noexcept
   {
      Operand op;
      op.control_ = 0;
      op.data_.i = v;
      op.isConstant_ = true;
      op.constSize = 1;
      if (v <= 64)
         op.setFixed(PhysReg{128u + v});
      else if (v >= 0xFFF0) /* [-16 .. -1] */
         op.setFixed(PhysReg{(unsigned)(192 - (int16_t)v)});
      else if (v == 0x3800) /* 0.5 */
         op.setFixed(PhysReg{240});
      else if (v == 0xB800) /* -0.5 */
         op.setFixed(PhysReg{241});
      else if (v == 0x3C00) /* 1.0 */
         op.setFixed(PhysReg{242});
      else if (v == 0xBC00) /* -1.0 */
         op.setFixed(PhysReg{243});
      else if (v == 0x4000) /* 2.0 */
         op.setFixed(PhysReg{244});
      else if (v == 0xC000) /* -2.0 */
         op.setFixed(PhysReg{245});
      else if (v == 0x4400) /* 4.0 */
         op.setFixed(PhysReg{246});
      else if (v == 0xC400) /* -4.0 */
         op.setFixed(PhysReg{247});
      else if (v == 0x3118) /* 1/2 PI */
         op.setFixed(PhysReg{248});
      else /* Literal Constant */
         op.setFixed(PhysReg{255});
      return op;
   }

   /* 32-bit constant */
   static Operand c32(uint32_t v) noexcept { return c32_or_c64(v, false); }

   /* 64-bit constant */
   static Operand c64(uint64_t v) noexcept
   {
      Operand op;
      op.control_ = 0;
      op.isConstant_ = true;
      op.constSize = 3;
      if (v <= 64) {
         op.data_.i = (uint32_t)v;
         op.setFixed(PhysReg{128 + (uint32_t)v});
      } else if (v >= 0xFFFFFFFFFFFFFFF0) { /* [-16 .. -1] */
         op.data_.i = (uint32_t)v;
         op.setFixed(PhysReg{192 - (uint32_t)v});
      } else if (v == 0x3FE0000000000000) { /* 0.5 */
         op.data_.i = 0x3f000000;
         op.setFixed(PhysReg{240});
      } else if (v == 0xBFE0000000000000) { /* -0.5 */
         op.data_.i = 0xbf000000;
         op.setFixed(PhysReg{241});
      } else if (v == 0x3FF0000000000000) { /* 1.0 */
         op.data_.i = 0x3f800000;
         op.setFixed(PhysReg{242});
      } else if (v == 0xBFF0000000000000) { /* -1.0 */
         op.data_.i = 0xbf800000;
         op.setFixed(PhysReg{243});
      } else if (v == 0x4000000000000000) { /* 2.0 */
         op.data_.i = 0x40000000;
         op.setFixed(PhysReg{244});
      } else if (v == 0xC000000000000000) { /* -2.0 */
         op.data_.i = 0xc0000000;
         op.setFixed(PhysReg{245});
      } else if (v == 0x4010000000000000) { /* 4.0 */
         op.data_.i = 0x40800000;
         op.setFixed(PhysReg{246});
      } else if (v == 0xC010000000000000) { /* -4.0 */
         op.data_.i = 0xc0800000;
         op.setFixed(PhysReg{247});
      } else { /* Literal Constant: we don't know if it is a long or double.*/
         op.signext = v >> 63;
         op.data_.i = v & 0xffffffffu;
         op.setFixed(PhysReg{255});
         assert(op.constantValue64() == v &&
                "attempt to create a unrepresentable 64-bit literal constant");
      }
      return op;
   }

   /* 32-bit constant stored as a 32-bit or 64-bit operand */
   static Operand c32_or_c64(uint32_t v, bool is64bit) noexcept
   {
      Operand op;
      op.control_ = 0;
      op.data_.i = v;
      op.isConstant_ = true;
      op.constSize = is64bit ? 3 : 2;
      if (v <= 64)
         op.setFixed(PhysReg{128 + v});
      else if (v >= 0xFFFFFFF0) /* [-16 .. -1] */
         op.setFixed(PhysReg{192 - v});
      else if (v == 0x3f000000) /* 0.5 */
         op.setFixed(PhysReg{240});
      else if (v == 0xbf000000) /* -0.5 */
         op.setFixed(PhysReg{241});
      else if (v == 0x3f800000) /* 1.0 */
         op.setFixed(PhysReg{242});
      else if (v == 0xbf800000) /* -1.0 */
         op.setFixed(PhysReg{243});
      else if (v == 0x40000000) /* 2.0 */
         op.setFixed(PhysReg{244});
      else if (v == 0xc0000000) /* -2.0 */
         op.setFixed(PhysReg{245});
      else if (v == 0x40800000) /* 4.0 */
         op.setFixed(PhysReg{246});
      else if (v == 0xc0800000) /* -4.0 */
         op.setFixed(PhysReg{247});
      else { /* Literal Constant */
         assert(!is64bit && "attempt to create a 64-bit literal constant");
         op.setFixed(PhysReg{255});
      }
      return op;
   }

   static Operand literal32(uint32_t v) noexcept
   {
      Operand op;
      op.control_ = 0;
      op.data_.i = v;
      op.isConstant_ = true;
      op.constSize = 2;
      op.setFixed(PhysReg{255});
      return op;
   }

   explicit Operand(RegClass type) noexcept
   {
      isUndef_ = true;
      data_.temp = Temp(0, type);
      setFixed(PhysReg{128});
   };
   explicit Operand(PhysReg reg, RegClass type) noexcept
   {
      data_.temp = Temp(0, type);
      setFixed(reg);
   }

   static Operand zero(unsigned bytes = 4) noexcept
   {
      if (bytes == 8)
         return Operand::c64(0);
      else if (bytes == 4)
         return Operand::c32(0);
      else if (bytes == 2)
         return Operand::c16(0);
      assert(bytes == 1);
      return Operand::c8(0);
   }

   /* This is useful over the constructors when you want to take a gfx level
    * for 1/2 PI or an unknown operand size.
    */
   static Operand get_const(enum amd_gfx_level chip, uint64_t val, unsigned bytes)
   {
      if (val == 0x3e22f983 && bytes == 4 && chip >= GFX8) {
         /* 1/2 PI can be an inline constant on GFX8+ */
         Operand op = Operand::c32(val);
         op.setFixed(PhysReg{248});
         return op;
      }

      if (bytes == 8)
         return Operand::c64(val);
      else if (bytes == 4)
         return Operand::c32(val);
      else if (bytes == 2)
         return Operand::c16(val);
      assert(bytes == 1);
      return Operand::c8(val);
   }

   static bool is_constant_representable(uint64_t val, unsigned bytes, bool zext = false,
                                         bool sext = false)
   {
      if (bytes <= 4)
         return true;

      if (zext && (val & 0xFFFFFFFF00000000) == 0x0000000000000000)
         return true;
      uint64_t upper33 = val & 0xFFFFFFFF80000000;
      if (sext && (upper33 == 0xFFFFFFFF80000000 || upper33 == 0))
         return true;

      return val >= 0xFFFFFFFFFFFFFFF0 || val <= 64 || /* [-16 .. 64] */
             val == 0x3FE0000000000000 ||              /* 0.5 */
             val == 0xBFE0000000000000 ||              /* -0.5 */
             val == 0x3FF0000000000000 ||              /* 1.0 */
             val == 0xBFF0000000000000 ||              /* -1.0 */
             val == 0x4000000000000000 ||              /* 2.0 */
             val == 0xC000000000000000 ||              /* -2.0 */
             val == 0x4010000000000000 ||              /* 4.0 */
             val == 0xC010000000000000;                /* -4.0 */
   }

   constexpr bool isTemp() const noexcept { return isTemp_; }

   constexpr void setTemp(Temp t) noexcept
   {
      assert(!isConstant_);
      isTemp_ = true;
      data_.temp = t;
   }

   constexpr Temp getTemp() const noexcept { return data_.temp; }

   constexpr uint32_t tempId() const noexcept { return data_.temp.id(); }

   constexpr bool hasRegClass() const noexcept { return !isConstant(); }

   constexpr RegClass regClass() const noexcept { return data_.temp.regClass(); }

   constexpr unsigned bytes() const noexcept
   {
      if (isConstant())
         return 1 << constSize;
      else
         return data_.temp.bytes();
   }

   constexpr unsigned size() const noexcept
   {
      if (isConstant())
         return constSize > 2 ? 2 : 1;
      else
         return data_.temp.size();
   }

   constexpr bool isFixed() const noexcept { return isFixed_; }

   constexpr PhysReg physReg() const noexcept { return reg_; }

   constexpr void setFixed(PhysReg reg) noexcept
   {
      isFixed_ = reg != unsigned(-1);
      reg_ = reg;
   }

   constexpr bool isConstant() const noexcept { return isConstant_; }

   constexpr bool isLiteral() const noexcept { return isConstant() && reg_ == 255; }

   constexpr bool isUndefined() const noexcept { return isUndef_; }

   constexpr uint32_t constantValue() const noexcept { return data_.i; }

   constexpr bool constantEquals(uint32_t cmp) const noexcept
   {
      return isConstant() && constantValue() == cmp;
   }

   constexpr uint64_t constantValue64() const noexcept
   {
      if (constSize == 3) {
         if (reg_ <= 192)
            return reg_ - 128;
         else if (reg_ <= 208)
            return 0xFFFFFFFFFFFFFFFF - (reg_ - 193);

         switch (reg_) {
         case 240: return 0x3FE0000000000000;
         case 241: return 0xBFE0000000000000;
         case 242: return 0x3FF0000000000000;
         case 243: return 0xBFF0000000000000;
         case 244: return 0x4000000000000000;
         case 245: return 0xC000000000000000;
         case 246: return 0x4010000000000000;
         case 247: return 0xC010000000000000;
         case 255:
            return (signext && (data_.i & 0x80000000u) ? 0xffffffff00000000ull : 0ull) | data_.i;
         }
         unreachable("invalid register for 64-bit constant");
      } else {
         return data_.i;
      }
   }

   /* Value if this were used with vop3/opsel or vop3p. */
   constexpr uint16_t constantValue16(bool opsel) const noexcept
   {
      assert(bytes() == 2 || bytes() == 4);
      if (opsel) {
         if (bytes() == 2 && int16_t(data_.i) >= -16 && int16_t(data_.i) <= 64 && !isLiteral())
            return int16_t(data_.i) >>
                   16; /* 16-bit inline integers are sign-extended, even with fp16 instrs */
         else
            return data_.i >> 16;
      }
      return data_.i;
   }

   constexpr bool isOfType(RegType type) const noexcept
   {
      return hasRegClass() && regClass().type() == type;
   }

   /* Indicates that the killed operand's live range intersects with the
    * instruction's definitions. Unlike isKill() and isFirstKill(), this is
    * not set by liveness analysis. */
   constexpr void setLateKill(bool flag) noexcept { isLateKill_ = flag; }

   constexpr bool isLateKill() const noexcept { return isLateKill_; }

   constexpr void setKill(bool flag) noexcept
   {
      isKill_ = flag;
      if (!flag)
         setFirstKill(false);
   }

   constexpr bool isKill() const noexcept { return isKill_ || isFirstKill(); }

   constexpr void setFirstKill(bool flag) noexcept
   {
      isFirstKill_ = flag;
      if (flag)
         setKill(flag);
   }

   /* When there are multiple operands killing the same temporary,
    * isFirstKill() is only returns true for the first one. */
   constexpr bool isFirstKill() const noexcept { return isFirstKill_; }

   constexpr bool isKillBeforeDef() const noexcept { return isKill() && !isLateKill(); }

   constexpr bool isFirstKillBeforeDef() const noexcept { return isFirstKill() && !isLateKill(); }

   constexpr bool operator==(Operand other) const noexcept
   {
      if (other.size() != size())
         return false;
      if (isFixed() != other.isFixed() || isKillBeforeDef() != other.isKillBeforeDef())
         return false;
      if (isFixed() && other.isFixed() && physReg() != other.physReg())
         return false;
      if (isLiteral())
         return other.isLiteral() && other.constantValue() == constantValue();
      else if (isConstant())
         return other.isConstant() && other.physReg() == physReg();
      else if (isUndefined())
         return other.isUndefined() && other.regClass() == regClass();
      else
         return other.isTemp() && other.getTemp() == getTemp();
   }

   constexpr bool operator!=(Operand other) const noexcept { return !operator==(other); }

   constexpr void set16bit(bool flag) noexcept { is16bit_ = flag; }

   constexpr bool is16bit() const noexcept { return is16bit_; }

   constexpr void set24bit(bool flag) noexcept { is24bit_ = flag; }

   constexpr bool is24bit() const noexcept { return is24bit_; }

private:
   union {
      Temp temp;
      uint32_t i;
      float f;
   } data_ = {Temp(0, s1)};
   PhysReg reg_;
   union {
      struct {
         uint8_t isTemp_ : 1;
         uint8_t isFixed_ : 1;
         uint8_t isConstant_ : 1;
         uint8_t isKill_ : 1;
         uint8_t isUndef_ : 1;
         uint8_t isFirstKill_ : 1;
         uint8_t constSize : 2;
         uint8_t isLateKill_ : 1;
         uint8_t is16bit_ : 1;
         uint8_t is24bit_ : 1;
         uint8_t signext : 1;
      };
      /* can't initialize bit-fields in c++11, so work around using a union */
      uint16_t control_ = 0;
   };
};

/**
 * Definition Class
 * Definitions are the results of Instructions
 * and refer to temporary virtual registers
 * which are later mapped to physical registers
 */
class Definition final {
public:
   constexpr Definition()
       : temp(Temp(0, s1)), reg_(0), isFixed_(0), isKill_(0), isPrecise_(0), isNUW_(0), isNoCSE_(0)
   {}
   Definition(uint32_t index, RegClass type) noexcept : temp(index, type) {}
   explicit Definition(Temp tmp) noexcept : temp(tmp) {}
   Definition(PhysReg reg, RegClass type) noexcept : temp(Temp(0, type)) { setFixed(reg); }
   Definition(uint32_t tmpId, PhysReg reg, RegClass type) noexcept : temp(Temp(tmpId, type))
   {
      setFixed(reg);
   }

   constexpr bool isTemp() const noexcept { return tempId() > 0; }

   constexpr Temp getTemp() const noexcept { return temp; }

   constexpr uint32_t tempId() const noexcept { return temp.id(); }

   constexpr void setTemp(Temp t) noexcept { temp = t; }

   void swapTemp(Definition& other) noexcept { std::swap(temp, other.temp); }

   constexpr RegClass regClass() const noexcept { return temp.regClass(); }

   constexpr unsigned bytes() const noexcept { return temp.bytes(); }

   constexpr unsigned size() const noexcept { return temp.size(); }

   constexpr bool isFixed() const noexcept { return isFixed_; }

   constexpr PhysReg physReg() const noexcept { return reg_; }

   constexpr void setFixed(PhysReg reg) noexcept
   {
      isFixed_ = 1;
      reg_ = reg;
   }

   constexpr void setKill(bool flag) noexcept { isKill_ = flag; }

   constexpr bool isKill() const noexcept { return isKill_; }

   constexpr void setPrecise(bool precise) noexcept { isPrecise_ = precise; }

   constexpr bool isPrecise() const noexcept { return isPrecise_; }

   /* No Unsigned Wrap */
   constexpr void setNUW(bool nuw) noexcept { isNUW_ = nuw; }

   constexpr bool isNUW() const noexcept { return isNUW_; }

   constexpr void setNoCSE(bool noCSE) noexcept { isNoCSE_ = noCSE; }

   constexpr bool isNoCSE() const noexcept { return isNoCSE_; }

private:
   Temp temp = Temp(0, s1);
   PhysReg reg_;
   union {
      struct {
         uint8_t isFixed_ : 1;
         uint8_t isKill_ : 1;
         uint8_t isPrecise_ : 1;
         uint8_t isNUW_ : 1;
         uint8_t isNoCSE_ : 1;
      };
      /* can't initialize bit-fields in c++11, so work around using a union */
      uint8_t control_ = 0;
   };
};

struct Block;
struct Instruction;
struct Pseudo_instruction;
struct SOP1_instruction;
struct SOP2_instruction;
struct SOPK_instruction;
struct SOPP_instruction;
struct SOPC_instruction;
struct SMEM_instruction;
struct DS_instruction;
struct LDSDIR_instruction;
struct MTBUF_instruction;
struct MUBUF_instruction;
struct MIMG_instruction;
struct Export_instruction;
struct FLAT_instruction;
struct Pseudo_branch_instruction;
struct Pseudo_barrier_instruction;
struct Pseudo_reduction_instruction;
struct VALU_instruction;
struct VINTERP_inreg_instruction;
struct VINTRP_instruction;
struct DPP16_instruction;
struct DPP8_instruction;
struct SDWA_instruction;

struct Instruction {
   aco_opcode opcode;
   Format format;
   uint32_t pass_flags;

   aco::span<Operand> operands;
   aco::span<Definition> definitions;

   constexpr bool usesModifiers() const noexcept;

   constexpr bool reads_exec() const noexcept
   {
      for (const Operand& op : operands) {
         if (op.isFixed() && (op.physReg() == exec_lo || op.physReg() == exec_hi))
            return true;
      }
      return false;
   }

   constexpr bool writes_exec() const noexcept
   {
      for (const Definition& def : definitions) {
         if (def.isFixed() && (def.physReg() == exec_lo || def.physReg() == exec_hi))
            return true;
      }
      return false;
   }

   Pseudo_instruction& pseudo() noexcept
   {
      assert(isPseudo());
      return *(Pseudo_instruction*)this;
   }
   const Pseudo_instruction& pseudo() const noexcept
   {
      assert(isPseudo());
      return *(Pseudo_instruction*)this;
   }
   constexpr bool isPseudo() const noexcept { return format == Format::PSEUDO; }
   SOP1_instruction& sop1() noexcept
   {
      assert(isSOP1());
      return *(SOP1_instruction*)this;
   }
   const SOP1_instruction& sop1() const noexcept
   {
      assert(isSOP1());
      return *(SOP1_instruction*)this;
   }
   constexpr bool isSOP1() const noexcept { return format == Format::SOP1; }
   SOP2_instruction& sop2() noexcept
   {
      assert(isSOP2());
      return *(SOP2_instruction*)this;
   }
   const SOP2_instruction& sop2() const noexcept
   {
      assert(isSOP2());
      return *(SOP2_instruction*)this;
   }
   constexpr bool isSOP2() const noexcept { return format == Format::SOP2; }
   SOPK_instruction& sopk() noexcept
   {
      assert(isSOPK());
      return *(SOPK_instruction*)this;
   }
   const SOPK_instruction& sopk() const noexcept
   {
      assert(isSOPK());
      return *(SOPK_instruction*)this;
   }
   constexpr bool isSOPK() const noexcept { return format == Format::SOPK; }
   SOPP_instruction& sopp() noexcept
   {
      assert(isSOPP());
      return *(SOPP_instruction*)this;
   }
   const SOPP_instruction& sopp() const noexcept
   {
      assert(isSOPP());
      return *(SOPP_instruction*)this;
   }
   constexpr bool isSOPP() const noexcept { return format == Format::SOPP; }
   SOPC_instruction& sopc() noexcept
   {
      assert(isSOPC());
      return *(SOPC_instruction*)this;
   }
   const SOPC_instruction& sopc() const noexcept
   {
      assert(isSOPC());
      return *(SOPC_instruction*)this;
   }
   constexpr bool isSOPC() const noexcept { return format == Format::SOPC; }
   SMEM_instruction& smem() noexcept
   {
      assert(isSMEM());
      return *(SMEM_instruction*)this;
   }
   const SMEM_instruction& smem() const noexcept
   {
      assert(isSMEM());
      return *(SMEM_instruction*)this;
   }
   constexpr bool isSMEM() const noexcept { return format == Format::SMEM; }
   DS_instruction& ds() noexcept
   {
      assert(isDS());
      return *(DS_instruction*)this;
   }
   const DS_instruction& ds() const noexcept
   {
      assert(isDS());
      return *(DS_instruction*)this;
   }
   constexpr bool isDS() const noexcept { return format == Format::DS; }
   LDSDIR_instruction& ldsdir() noexcept
   {
      assert(isLDSDIR());
      return *(LDSDIR_instruction*)this;
   }
   const LDSDIR_instruction& ldsdir() const noexcept
   {
      assert(isLDSDIR());
      return *(LDSDIR_instruction*)this;
   }
   constexpr bool isLDSDIR() const noexcept { return format == Format::LDSDIR; }
   MTBUF_instruction& mtbuf() noexcept
   {
      assert(isMTBUF());
      return *(MTBUF_instruction*)this;
   }
   const MTBUF_instruction& mtbuf() const noexcept
   {
      assert(isMTBUF());
      return *(MTBUF_instruction*)this;
   }
   constexpr bool isMTBUF() const noexcept { return format == Format::MTBUF; }
   MUBUF_instruction& mubuf() noexcept
   {
      assert(isMUBUF());
      return *(MUBUF_instruction*)this;
   }
   const MUBUF_instruction& mubuf() const noexcept
   {
      assert(isMUBUF());
      return *(MUBUF_instruction*)this;
   }
   constexpr bool isMUBUF() const noexcept { return format == Format::MUBUF; }
   MIMG_instruction& mimg() noexcept
   {
      assert(isMIMG());
      return *(MIMG_instruction*)this;
   }
   const MIMG_instruction& mimg() const noexcept
   {
      assert(isMIMG());
      return *(MIMG_instruction*)this;
   }
   constexpr bool isMIMG() const noexcept { return format == Format::MIMG; }
   Export_instruction& exp() noexcept
   {
      assert(isEXP());
      return *(Export_instruction*)this;
   }
   const Export_instruction& exp() const noexcept
   {
      assert(isEXP());
      return *(Export_instruction*)this;
   }
   constexpr bool isEXP() const noexcept { return format == Format::EXP; }
   FLAT_instruction& flat() noexcept
   {
      assert(isFlat());
      return *(FLAT_instruction*)this;
   }
   const FLAT_instruction& flat() const noexcept
   {
      assert(isFlat());
      return *(FLAT_instruction*)this;
   }
   constexpr bool isFlat() const noexcept { return format == Format::FLAT; }
   FLAT_instruction& global() noexcept
   {
      assert(isGlobal());
      return *(FLAT_instruction*)this;
   }
   const FLAT_instruction& global() const noexcept
   {
      assert(isGlobal());
      return *(FLAT_instruction*)this;
   }
   constexpr bool isGlobal() const noexcept { return format == Format::GLOBAL; }
   FLAT_instruction& scratch() noexcept
   {
      assert(isScratch());
      return *(FLAT_instruction*)this;
   }
   const FLAT_instruction& scratch() const noexcept
   {
      assert(isScratch());
      return *(FLAT_instruction*)this;
   }
   constexpr bool isScratch() const noexcept { return format == Format::SCRATCH; }
   Pseudo_branch_instruction& branch() noexcept
   {
      assert(isBranch());
      return *(Pseudo_branch_instruction*)this;
   }
   const Pseudo_branch_instruction& branch() const noexcept
   {
      assert(isBranch());
      return *(Pseudo_branch_instruction*)this;
   }
   constexpr bool isBranch() const noexcept { return format == Format::PSEUDO_BRANCH; }
   Pseudo_barrier_instruction& barrier() noexcept
   {
      assert(isBarrier());
      return *(Pseudo_barrier_instruction*)this;
   }
   const Pseudo_barrier_instruction& barrier() const noexcept
   {
      assert(isBarrier());
      return *(Pseudo_barrier_instruction*)this;
   }
   constexpr bool isBarrier() const noexcept { return format == Format::PSEUDO_BARRIER; }
   Pseudo_reduction_instruction& reduction() noexcept
   {
      assert(isReduction());
      return *(Pseudo_reduction_instruction*)this;
   }
   const Pseudo_reduction_instruction& reduction() const noexcept
   {
      assert(isReduction());
      return *(Pseudo_reduction_instruction*)this;
   }
   constexpr bool isReduction() const noexcept { return format == Format::PSEUDO_REDUCTION; }
   constexpr bool isVOP3P() const noexcept { return (uint16_t)format & (uint16_t)Format::VOP3P; }
   VINTERP_inreg_instruction& vinterp_inreg() noexcept
   {
      assert(isVINTERP_INREG());
      return *(VINTERP_inreg_instruction*)this;
   }
   const VINTERP_inreg_instruction& vinterp_inreg() const noexcept
   {
      assert(isVINTERP_INREG());
      return *(VINTERP_inreg_instruction*)this;
   }
   constexpr bool isVINTERP_INREG() const noexcept { return format == Format::VINTERP_INREG; }
   constexpr bool isVOP1() const noexcept { return (uint16_t)format & (uint16_t)Format::VOP1; }
   constexpr bool isVOP2() const noexcept { return (uint16_t)format & (uint16_t)Format::VOP2; }
   constexpr bool isVOPC() const noexcept { return (uint16_t)format & (uint16_t)Format::VOPC; }
   constexpr bool isVOP3() const noexcept { return (uint16_t)format & (uint16_t)Format::VOP3; }
   VINTRP_instruction& vintrp() noexcept
   {
      assert(isVINTRP());
      return *(VINTRP_instruction*)this;
   }
   const VINTRP_instruction& vintrp() const noexcept
   {
      assert(isVINTRP());
      return *(VINTRP_instruction*)this;
   }
   constexpr bool isVINTRP() const noexcept { return format == Format::VINTRP; }
   DPP16_instruction& dpp16() noexcept
   {
      assert(isDPP16());
      return *(DPP16_instruction*)this;
   }
   const DPP16_instruction& dpp16() const noexcept
   {
      assert(isDPP16());
      return *(DPP16_instruction*)this;
   }
   constexpr bool isDPP16() const noexcept { return (uint16_t)format & (uint16_t)Format::DPP16; }
   DPP8_instruction& dpp8() noexcept
   {
      assert(isDPP8());
      return *(DPP8_instruction*)this;
   }
   const DPP8_instruction& dpp8() const noexcept
   {
      assert(isDPP8());
      return *(DPP8_instruction*)this;
   }
   constexpr bool isDPP8() const noexcept { return (uint16_t)format & (uint16_t)Format::DPP8; }
   constexpr bool isDPP() const noexcept { return isDPP16() || isDPP8(); }
   SDWA_instruction& sdwa() noexcept
   {
      assert(isSDWA());
      return *(SDWA_instruction*)this;
   }
   const SDWA_instruction& sdwa() const noexcept
   {
      assert(isSDWA());
      return *(SDWA_instruction*)this;
   }
   constexpr bool isSDWA() const noexcept { return (uint16_t)format & (uint16_t)Format::SDWA; }

   FLAT_instruction& flatlike() { return *(FLAT_instruction*)this; }

   const FLAT_instruction& flatlike() const { return *(FLAT_instruction*)this; }

   constexpr bool isFlatLike() const noexcept { return isFlat() || isGlobal() || isScratch(); }

   VALU_instruction& valu() noexcept
   {
      assert(isVALU());
      return *(VALU_instruction*)this;
   }
   const VALU_instruction& valu() const noexcept
   {
      assert(isVALU());
      return *(VALU_instruction*)this;
   }
   constexpr bool isVALU() const noexcept
   {
      return isVOP1() || isVOP2() || isVOPC() || isVOP3() || isVOP3P() || isVINTERP_INREG();
   }

   constexpr bool isSALU() const noexcept
   {
      return isSOP1() || isSOP2() || isSOPC() || isSOPK() || isSOPP();
   }

   constexpr bool isVMEM() const noexcept { return isMTBUF() || isMUBUF() || isMIMG(); }

   bool accessesLDS() const noexcept;
   bool isTrans() const noexcept;
};
static_assert(sizeof(Instruction) == 16, "Unexpected padding");

struct SOPK_instruction : public Instruction {
   uint16_t imm;
   uint16_t padding;
};
static_assert(sizeof(SOPK_instruction) == sizeof(Instruction) + 4, "Unexpected padding");

struct SOPP_instruction : public Instruction {
   uint32_t imm;
   int block;
};
static_assert(sizeof(SOPP_instruction) == sizeof(Instruction) + 8, "Unexpected padding");

struct SOPC_instruction : public Instruction {
   uint32_t padding;
};
static_assert(sizeof(SOPC_instruction) == sizeof(Instruction) + 4, "Unexpected padding");

struct SOP1_instruction : public Instruction {};
static_assert(sizeof(SOP1_instruction) == sizeof(Instruction) + 0, "Unexpected padding");

struct SOP2_instruction : public Instruction {
   uint32_t padding;
};
static_assert(sizeof(SOP2_instruction) == sizeof(Instruction) + 4, "Unexpected padding");

/**
 * Scalar Memory Format:
 * For s_(buffer_)load_dword*:
 * Operand(0): SBASE - SGPR-pair which provides base address
 * Operand(1): Offset - immediate (un)signed offset or SGPR
 * Operand(2) / Definition(0): SDATA - SGPR for read / write result
 * Operand(n-1): SOffset - SGPR offset (Vega only)
 *
 * Having no operands is also valid for instructions such as s_dcache_inv.
 *
 */
struct SMEM_instruction : public Instruction {
   memory_sync_info sync;
   bool glc : 1; /* VI+: globally coherent */
   bool dlc : 1; /* NAVI: device level coherent */
   bool nv : 1;  /* VEGA only: Non-volatile */
   bool disable_wqm : 1;
   uint8_t padding : 4;
};
static_assert(sizeof(SMEM_instruction) == sizeof(Instruction) + 4, "Unexpected padding");

struct VALU_instruction : public Instruction {
   union {
      bitfield_array8<uint32_t, 0, 3> neg;    /* VOP3, SDWA, DPP16, v_fma_mix, VINTERP_inreg */
      bitfield_array8<uint32_t, 0, 3> neg_lo; /* VOP3P */

      bitfield_array8<uint32_t, 3, 3> abs;    /* VOP3, SDWA, DPP16, v_fma_mix */
      bitfield_array8<uint32_t, 3, 3> neg_hi; /* VOP3P */

      bitfield_array8<uint32_t, 6, 4> opsel;     /* VOP3, VOPC12(GFX11+), VINTERP_inreg */
      bitfield_uint8<uint32_t, 10, 2> omod;      /* VOP3, SDWA(GFX9+) */
      bitfield_array8<uint32_t, 12, 3> opsel_lo; /* VOP3P */
      bitfield_array8<uint32_t, 15, 3> opsel_hi; /* VOP3P */
      bitfield_bool<uint32_t, 18> clamp;         /* VOP3, VOP3P, SDWA, VINTERP_inreg */
   };

   void swapOperands(unsigned idx0, unsigned idx1);
};
static_assert(sizeof(VALU_instruction) == sizeof(Instruction) + 4, "Unexpected padding");

struct VINTERP_inreg_instruction : public VALU_instruction {
   uint8_t wait_exp : 3;
   uint8_t padding3 : 5;
   uint8_t padding4;
   uint8_t padding5;
   uint8_t padding6;
};
static_assert(sizeof(VINTERP_inreg_instruction) == sizeof(VALU_instruction) + 4,
              "Unexpected padding");

/**
 * Data Parallel Primitives Format:
 * This format can be used for VOP1, VOP2 or VOPC instructions.
 * The swizzle applies to the src0 operand.
 *
 */
struct DPP16_instruction : public VALU_instruction {
   uint16_t dpp_ctrl;
   uint8_t row_mask : 4;
   uint8_t bank_mask : 4;
   bool bound_ctrl : 1;
   uint8_t fetch_inactive : 1;
   uint8_t padding3 : 6;
};
static_assert(sizeof(DPP16_instruction) == sizeof(VALU_instruction) + 4, "Unexpected padding");

struct DPP8_instruction : public VALU_instruction {
   uint32_t lane_sel : 24;
   uint32_t fetch_inactive : 1;
   uint32_t padding : 7;
};
static_assert(sizeof(DPP8_instruction) == sizeof(VALU_instruction) + 4, "Unexpected padding");

struct SubdwordSel {
   enum sdwa_sel : uint8_t {
      ubyte = 0x4,
      uword = 0x8,
      dword = 0x10,
      sext = 0x20,
      sbyte = ubyte | sext,
      sword = uword | sext,

      ubyte0 = ubyte,
      ubyte1 = ubyte | 1,
      ubyte2 = ubyte | 2,
      ubyte3 = ubyte | 3,
      sbyte0 = sbyte,
      sbyte1 = sbyte | 1,
      sbyte2 = sbyte | 2,
      sbyte3 = sbyte | 3,
      uword0 = uword,
      uword1 = uword | 2,
      sword0 = sword,
      sword1 = sword | 2,
   };

   SubdwordSel() : sel((sdwa_sel)0) {}
   constexpr SubdwordSel(sdwa_sel sel_) : sel(sel_) {}
   constexpr SubdwordSel(unsigned size, unsigned offset, bool sign_extend)
       : sel((sdwa_sel)((sign_extend ? sext : 0) | size << 2 | offset))
   {}
   constexpr operator sdwa_sel() const { return sel; }
   explicit operator bool() const { return sel != 0; }

   constexpr unsigned size() const { return (sel >> 2) & 0x7; }
   constexpr unsigned offset() const { return sel & 0x3; }
   constexpr bool sign_extend() const { return sel & sext; }
   constexpr unsigned to_sdwa_sel(unsigned reg_byte_offset) const
   {
      reg_byte_offset += offset();
      if (size() == 1)
         return reg_byte_offset;
      else if (size() == 2)
         return 4 + (reg_byte_offset >> 1);
      else
         return 6;
   }

private:
   sdwa_sel sel;
};

/**
 * Sub-Dword Addressing Format:
 * This format can be used for VOP1, VOP2 or VOPC instructions.
 *
 * omod and SGPR/constant operands are only available on GFX9+. For VOPC,
 * the definition doesn't have to be VCC on GFX9+.
 *
 */
struct SDWA_instruction : public VALU_instruction {
   /* these destination modifiers aren't available with VOPC except for
    * clamp on GFX8 */
   SubdwordSel sel[2];
   SubdwordSel dst_sel;
   uint8_t padding3;
};
static_assert(sizeof(SDWA_instruction) == sizeof(VALU_instruction) + 4, "Unexpected padding");

struct VINTRP_instruction : public Instruction {
   uint8_t attribute;
   uint8_t component;
   uint16_t padding;
};
static_assert(sizeof(VINTRP_instruction) == sizeof(Instruction) + 4, "Unexpected padding");

/**
 * Local and Global Data Sharing instructions
 * Operand(0): ADDR - VGPR which supplies the address.
 * Operand(1): DATA0 - First data VGPR.
 * Operand(2): DATA1 - Second data VGPR.
 * Operand(n-1): M0 - LDS size.
 * Definition(0): VDST - Destination VGPR when results returned to VGPRs.
 *
 */
struct DS_instruction : public Instruction {
   memory_sync_info sync;
   bool gds;
   uint16_t offset0;
   uint8_t offset1;
   uint8_t padding;
};
static_assert(sizeof(DS_instruction) == sizeof(Instruction) + 8, "Unexpected padding");

/**
 * LDS Direct instructions
 * Operand(0): M0
 * Definition(0): VDST - Destination VGPR
 */
struct LDSDIR_instruction : public Instruction {
   memory_sync_info sync;
   uint8_t attr : 6;
   uint8_t attr_chan : 2;
   uint32_t wait_vdst : 4;
   uint32_t padding : 28;
};
static_assert(sizeof(LDSDIR_instruction) == sizeof(Instruction) + 8, "Unexpected padding");

/**
 * Vector Memory Untyped-buffer Instructions
 * Operand(0): SRSRC - Specifies which SGPR supplies T# (resource constant)
 * Operand(1): VADDR - Address source. Can carry an index and/or offset
 * Operand(2): SOFFSET - SGPR to supply unsigned byte offset. (SGPR, M0, or inline constant)
 * Operand(3) / Definition(0): VDATA - Vector GPR for write result / read data
 *
 */
struct MUBUF_instruction : public Instruction {
   memory_sync_info sync;
   bool offen : 1;           /* Supply an offset from VGPR (VADDR) */
   bool idxen : 1;           /* Supply an index from VGPR (VADDR) */
   bool addr64 : 1;          /* SI, CIK: Address size is 64-bit */
   bool glc : 1;             /* globally coherent */
   bool dlc : 1;             /* NAVI: device level coherent */
   bool slc : 1;             /* system level coherent */
   bool tfe : 1;             /* texture fail enable */
   bool lds : 1;             /* Return read-data to LDS instead of VGPRs */
   uint16_t disable_wqm : 1; /* Require an exec mask without helper invocations */
   uint16_t offset : 12;     /* Unsigned byte offset - 12 bit */
   uint16_t swizzled : 1;
   uint16_t padding0 : 2;
   uint16_t padding1;
};
static_assert(sizeof(MUBUF_instruction) == sizeof(Instruction) + 8, "Unexpected padding");

/**
 * Vector Memory Typed-buffer Instructions
 * Operand(0): SRSRC - Specifies which SGPR supplies T# (resource constant)
 * Operand(1): VADDR - Address source. Can carry an index and/or offset
 * Operand(2): SOFFSET - SGPR to supply unsigned byte offset. (SGPR, M0, or inline constant)
 * Operand(3) / Definition(0): VDATA - Vector GPR for write result / read data
 *
 */
struct MTBUF_instruction : public Instruction {
   memory_sync_info sync;
   uint8_t dfmt : 4;         /* Data Format of data in memory buffer */
   uint8_t nfmt : 3;         /* Numeric format of data in memory */
   bool offen : 1;           /* Supply an offset from VGPR (VADDR) */
   uint16_t idxen : 1;       /* Supply an index from VGPR (VADDR) */
   uint16_t glc : 1;         /* globally coherent */
   uint16_t dlc : 1;         /* NAVI: device level coherent */
   uint16_t slc : 1;         /* system level coherent */
   uint16_t tfe : 1;         /* texture fail enable */
   uint16_t disable_wqm : 1; /* Require an exec mask without helper invocations */
   uint16_t padding : 10;
   uint16_t offset; /* Unsigned byte offset - 12 bit */
};
static_assert(sizeof(MTBUF_instruction) == sizeof(Instruction) + 8, "Unexpected padding");

/**
 * Vector Memory Image Instructions
 * Operand(0) SRSRC - Scalar GPR that specifies the resource constant.
 * Operand(1): SSAMP - Scalar GPR that specifies sampler constant.
 * Operand(2): VDATA - Vector GPR for write data or zero if TFE/LWE=1.
 * Operand(3): VADDR - Address source. Can carry an offset or an index.
 * Definition(0): VDATA - Vector GPR for read result.
 *
 */
struct MIMG_instruction : public Instruction {
   memory_sync_info sync;
   uint8_t dmask;        /* Data VGPR enable mask */
   uint8_t dim : 3;      /* NAVI: dimensionality */
   bool unrm : 1;        /* Force address to be un-normalized */
   bool dlc : 1;         /* NAVI: device level coherent */
   bool glc : 1;         /* globally coherent */
   bool slc : 1;         /* system level coherent */
   bool tfe : 1;         /* texture fail enable */
   bool da : 1;          /* declare an array */
   bool lwe : 1;         /* LOD warning enable */
   bool r128 : 1;        /* NAVI: Texture resource size */
   bool a16 : 1;         /* VEGA, NAVI: Address components are 16-bits */
   bool d16 : 1;         /* Convert 32-bit data to 16-bit data */
   bool disable_wqm : 1; /* Require an exec mask without helper invocations */
   bool strict_wqm : 1;  /* VADDR is a linear VGPR and additional VGPRs may be copied into it */
   uint8_t padding0 : 1;
   uint8_t padding1;
   uint8_t padding2;
};
static_assert(sizeof(MIMG_instruction) == sizeof(Instruction) + 8, "Unexpected padding");

/**
 * Flat/Scratch/Global Instructions
 * Operand(0): ADDR
 * Operand(1): SADDR
 * Operand(2) / Definition(0): DATA/VDST
 *
 */
struct FLAT_instruction : public Instruction {
   memory_sync_info sync;
   bool slc : 1; /* system level coherent */
   bool glc : 1; /* globally coherent */
   bool dlc : 1; /* NAVI: device level coherent */
   bool lds : 1;
   bool nv : 1;
   bool disable_wqm : 1; /* Require an exec mask without helper invocations */
   uint8_t padding0 : 2;
   int16_t offset; /* Vega/Navi only */
   uint16_t padding1;
};
static_assert(sizeof(FLAT_instruction) == sizeof(Instruction) + 8, "Unexpected padding");

struct Export_instruction : public Instruction {
   uint8_t enabled_mask;
   uint8_t dest;
   bool compressed : 1;
   bool done : 1;
   bool valid_mask : 1;
   bool row_en : 1;
   uint8_t padding0 : 4;
   uint8_t padding1;
};
static_assert(sizeof(Export_instruction) == sizeof(Instruction) + 4, "Unexpected padding");

struct Pseudo_instruction : public Instruction {
   PhysReg scratch_sgpr; /* might not be valid if it's not needed */
   bool tmp_in_scc;
   bool needs_scratch_reg; /* if scratch_sgpr/scc can be written, initialized by RA. */
};
static_assert(sizeof(Pseudo_instruction) == sizeof(Instruction) + 4, "Unexpected padding");

struct Pseudo_branch_instruction : public Instruction {
   /* target[0] is the block index of the branch target.
    * For conditional branches, target[1] contains the fall-through alternative.
    * A value of 0 means the target has not been initialized (BB0 cannot be a branch target).
    */
   uint32_t target[2];

   /* Indicates that selection control prefers to remove this instruction if possible.
    * This is set when the branch is divergent and always taken, or flattened.
    */
   bool selection_control_remove;
};
static_assert(sizeof(Pseudo_branch_instruction) == sizeof(Instruction) + 12, "Unexpected padding");

struct Pseudo_barrier_instruction : public Instruction {
   memory_sync_info sync;
   sync_scope exec_scope;
};
static_assert(sizeof(Pseudo_barrier_instruction) == sizeof(Instruction) + 4, "Unexpected padding");

enum ReduceOp : uint16_t {
   // clang-format off
   iadd8, iadd16, iadd32, iadd64,
   imul8, imul16, imul32, imul64,
          fadd16, fadd32, fadd64,
          fmul16, fmul32, fmul64,
   imin8, imin16, imin32, imin64,
   imax8, imax16, imax32, imax64,
   umin8, umin16, umin32, umin64,
   umax8, umax16, umax32, umax64,
          fmin16, fmin32, fmin64,
          fmax16, fmax32, fmax64,
   iand8, iand16, iand32, iand64,
   ior8, ior16, ior32, ior64,
   ixor8, ixor16, ixor32, ixor64,
   num_reduce_ops,
   // clang-format on
};

/**
 * Subgroup Reduction Instructions, everything except for the data to be
 * reduced and the result as inserted by setup_reduce_temp().
 * Operand(0): data to be reduced
 * Operand(1): reduce temporary
 * Operand(2): vector temporary
 * Definition(0): result
 * Definition(1): scalar temporary
 * Definition(2): scalar identity temporary (not used to store identity on GFX10)
 * Definition(3): scc clobber
 * Definition(4): vcc clobber
 *
 */
struct Pseudo_reduction_instruction : public Instruction {
   ReduceOp reduce_op;
   uint16_t cluster_size; // must be 0 for scans
};
static_assert(sizeof(Pseudo_reduction_instruction) == sizeof(Instruction) + 4,
              "Unexpected padding");

inline bool
Instruction::accessesLDS() const noexcept
{
   return (isDS() && !ds().gds) || isLDSDIR() || isVINTRP();
}

inline void
VALU_instruction::swapOperands(unsigned idx0, unsigned idx1)
{
   if (this->isSDWA() && idx0 != idx1) {
      assert(idx0 < 2 && idx1 < 2);
      std::swap(this->sdwa().sel[0], this->sdwa().sel[1]);
   }
   assert(idx0 < 3 && idx1 < 3);
   std::swap(this->operands[idx0], this->operands[idx1]);
   this->neg[idx0].swap(this->neg[idx1]);
   this->abs[idx0].swap(this->abs[idx1]);
   this->opsel[idx0].swap(this->opsel[idx1]);
   this->opsel_lo[idx0].swap(this->opsel_lo[idx1]);
   this->opsel_hi[idx0].swap(this->opsel_hi[idx1]);
}

extern thread_local aco::monotonic_buffer_resource* instruction_buffer;

struct instr_deleter_functor {
   /* Don't yet free any instructions. They will be de-allocated
    * all at once after compilation finished.
    */
   void operator()(void* p) { return; }
};

template <typename T> using aco_ptr = std::unique_ptr<T, instr_deleter_functor>;

template <typename T>
T*
create_instruction(aco_opcode opcode, Format format, uint32_t num_operands,
                   uint32_t num_definitions)
{
   std::size_t size =
      sizeof(T) + num_operands * sizeof(Operand) + num_definitions * sizeof(Definition);
   void* data = instruction_buffer->allocate(size, alignof(uint32_t));
   memset(data, 0, size);
   T* inst = (T*)data;

   inst->opcode = opcode;
   inst->format = format;

   uint16_t operands_offset = sizeof(T) - offsetof(Instruction, operands);
   inst->operands = aco::span<Operand>(operands_offset, num_operands);
   uint16_t definitions_offset = (char*)inst->operands.end() - (char*)&inst->definitions;
   inst->definitions = aco::span<Definition>(definitions_offset, num_definitions);

   return inst;
}

constexpr bool
Instruction::usesModifiers() const noexcept
{
   if (isDPP() || isSDWA())
      return true;

   if (isVOP3P()) {
      const VALU_instruction& vop3p = this->valu();
      /* opsel_hi must be 1 to not be considered a modifier - even for constants */
      return vop3p.opsel_lo || vop3p.clamp || vop3p.neg_lo || vop3p.neg_hi ||
             (vop3p.opsel_hi & BITFIELD_MASK(operands.size())) != BITFIELD_MASK(operands.size());
   } else if (isVALU()) {
      const VALU_instruction& vop3 = this->valu();
      return vop3.opsel || vop3.clamp || vop3.omod || vop3.abs || vop3.neg;
   }
   return false;
}

constexpr bool
is_phi(Instruction* instr)
{
   return instr->opcode == aco_opcode::p_phi || instr->opcode == aco_opcode::p_linear_phi;
}

static inline bool
is_phi(aco_ptr<Instruction>& instr)
{
   return is_phi(instr.get());
}

memory_sync_info get_sync_info(const Instruction* instr);

inline bool
is_dead(const std::vector<uint16_t>& uses, const Instruction* instr)
{
   if (instr->definitions.empty() || instr->isBranch() || instr->opcode == aco_opcode::p_startpgm ||
       instr->opcode == aco_opcode::p_init_scratch ||
       instr->opcode == aco_opcode::p_dual_src_export_gfx11)
      return false;

   if (std::any_of(instr->definitions.begin(), instr->definitions.end(),
                   [&uses](const Definition& def) { return !def.isTemp() || uses[def.tempId()]; }))
      return false;

   return !(get_sync_info(instr).semantics & (semantic_volatile | semantic_acqrel));
}

bool can_use_input_modifiers(amd_gfx_level gfx_level, aco_opcode op, int idx);
bool can_use_opsel(amd_gfx_level gfx_level, aco_opcode op, int idx);
bool instr_is_16bit(amd_gfx_level gfx_level, aco_opcode op);
uint8_t get_gfx11_true16_mask(aco_opcode op);
bool can_use_SDWA(amd_gfx_level gfx_level, const aco_ptr<Instruction>& instr, bool pre_ra);
bool can_use_DPP(amd_gfx_level gfx_level, const aco_ptr<Instruction>& instr, bool dpp8);
bool can_write_m0(const aco_ptr<Instruction>& instr);
/* updates "instr" and returns the old instruction (or NULL if no update was needed) */
aco_ptr<Instruction> convert_to_SDWA(amd_gfx_level gfx_level, aco_ptr<Instruction>& instr);
aco_ptr<Instruction> convert_to_DPP(amd_gfx_level gfx_level, aco_ptr<Instruction>& instr,
                                    bool dpp8);
bool needs_exec_mask(const Instruction* instr);

aco_opcode get_ordered(aco_opcode op);
aco_opcode get_unordered(aco_opcode op);
aco_opcode get_inverse(aco_opcode op);
aco_opcode get_swapped(aco_opcode op);
aco_opcode get_f32_cmp(aco_opcode op);
aco_opcode get_vcmpx(aco_opcode op);
unsigned get_cmp_bitsize(aco_opcode op);
bool is_fp_cmp(aco_opcode op);
bool is_cmpx(aco_opcode op);

bool can_swap_operands(aco_ptr<Instruction>& instr, aco_opcode* new_op, unsigned idx0 = 0,
                       unsigned idx1 = 1);

uint32_t get_reduction_identity(ReduceOp op, unsigned idx);

unsigned get_mimg_nsa_dwords(const Instruction* instr);

unsigned get_operand_size(aco_ptr<Instruction>& instr, unsigned index);

bool should_form_clause(const Instruction* a, const Instruction* b);

enum block_kind {
   /* uniform indicates that leaving this block,
    * all actives lanes stay active */
   block_kind_uniform = 1 << 0,
   block_kind_top_level = 1 << 1,
   block_kind_loop_preheader = 1 << 2,
   block_kind_loop_header = 1 << 3,
   block_kind_loop_exit = 1 << 4,
   block_kind_continue = 1 << 5,
   block_kind_break = 1 << 6,
   block_kind_continue_or_break = 1 << 7,
   block_kind_branch = 1 << 8,
   block_kind_merge = 1 << 9,
   block_kind_invert = 1 << 10,
   block_kind_discard_early_exit = 1 << 11,
   block_kind_uses_discard = 1 << 12,
   block_kind_resume = 1 << 13,
   block_kind_export_end = 1 << 14,
   block_kind_end_with_regs = 1 << 15,
};

struct RegisterDemand {
   constexpr RegisterDemand() = default;
   constexpr RegisterDemand(const int16_t v, const int16_t s) noexcept : vgpr{v}, sgpr{s} {}
   int16_t vgpr = 0;
   int16_t sgpr = 0;

   constexpr friend bool operator==(const RegisterDemand a, const RegisterDemand b) noexcept
   {
      return a.vgpr == b.vgpr && a.sgpr == b.sgpr;
   }

   constexpr bool exceeds(const RegisterDemand other) const noexcept
   {
      return vgpr > other.vgpr || sgpr > other.sgpr;
   }

   constexpr RegisterDemand operator+(const Temp t) const noexcept
   {
      if (t.type() == RegType::sgpr)
         return RegisterDemand(vgpr, sgpr + t.size());
      else
         return RegisterDemand(vgpr + t.size(), sgpr);
   }

   constexpr RegisterDemand operator+(const RegisterDemand other) const noexcept
   {
      return RegisterDemand(vgpr + other.vgpr, sgpr + other.sgpr);
   }

   constexpr RegisterDemand operator-(const RegisterDemand other) const noexcept
   {
      return RegisterDemand(vgpr - other.vgpr, sgpr - other.sgpr);
   }

   constexpr RegisterDemand& operator+=(const RegisterDemand other) noexcept
   {
      vgpr += other.vgpr;
      sgpr += other.sgpr;
      return *this;
   }

   constexpr RegisterDemand& operator-=(const RegisterDemand other) noexcept
   {
      vgpr -= other.vgpr;
      sgpr -= other.sgpr;
      return *this;
   }

   constexpr RegisterDemand& operator+=(const Temp t) noexcept
   {
      if (t.type() == RegType::sgpr)
         sgpr += t.size();
      else
         vgpr += t.size();
      return *this;
   }

   constexpr RegisterDemand& operator-=(const Temp t) noexcept
   {
      if (t.type() == RegType::sgpr)
         sgpr -= t.size();
      else
         vgpr -= t.size();
      return *this;
   }

   constexpr void update(const RegisterDemand other) noexcept
   {
      vgpr = std::max(vgpr, other.vgpr);
      sgpr = std::max(sgpr, other.sgpr);
   }
};

/* CFG */
struct Block {
   float_mode fp_mode;
   unsigned index;
   unsigned offset = 0;
   std::vector<aco_ptr<Instruction>> instructions;
   std::vector<unsigned> logical_preds;
   std::vector<unsigned> linear_preds;
   std::vector<unsigned> logical_succs;
   std::vector<unsigned> linear_succs;
   RegisterDemand register_demand = RegisterDemand();
   uint32_t kind = 0;
   int32_t logical_idom = -1;
   int32_t linear_idom = -1;
   uint16_t loop_nest_depth = 0;
   uint16_t divergent_if_logical_depth = 0;
   uint16_t uniform_if_depth = 0;

   /* this information is needed for predecessors to blocks with phis when
    * moving out of ssa */
   bool scc_live_out = false;

   Block() : index(0) {}
};

/*
 * Shader stages as provided in Vulkan by the application. Contrast this to HWStage.
 */
enum class SWStage : uint16_t {
   None = 0,
   VS = 1 << 0,  /* Vertex Shader */
   GS = 1 << 1,  /* Geometry Shader */
   TCS = 1 << 2, /* Tessellation Control aka Hull Shader */
   TES = 1 << 3, /* Tessellation Evaluation aka Domain Shader */
   FS = 1 << 4,  /* Fragment aka Pixel Shader */
   CS = 1 << 5,  /* Compute Shader */
   TS = 1 << 6,  /* Task Shader */
   MS = 1 << 7,  /* Mesh Shader */
   RT = 1 << 8,  /* Raytracing Shader */

   /* Stage combinations merged to run on a single HWStage */
   VS_GS = VS | GS,
   VS_TCS = VS | TCS,
   TES_GS = TES | GS,
};

constexpr SWStage
operator|(SWStage a, SWStage b)
{
   return static_cast<SWStage>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

/*
 * Set of SWStages to be merged into a single shader paired with the
 * HWStage it will run on.
 */
struct Stage {
   constexpr Stage() = default;

   explicit constexpr Stage(ac_hw_stage hw_, SWStage sw_) : sw(sw_), hw(hw_) {}

   /* Check if the given SWStage is included */
   constexpr bool has(SWStage stage) const
   {
      return (static_cast<uint16_t>(sw) & static_cast<uint16_t>(stage));
   }

   unsigned num_sw_stages() const { return util_bitcount(static_cast<uint16_t>(sw)); }

   constexpr bool operator==(const Stage& other) const { return sw == other.sw && hw == other.hw; }

   constexpr bool operator!=(const Stage& other) const { return sw != other.sw || hw != other.hw; }

   /* Mask of merged software stages */
   SWStage sw = SWStage::None;

   /* Active hardware stage */
   ac_hw_stage hw{};
};

/* possible settings of Program::stage */
static constexpr Stage vertex_vs(AC_HW_VERTEX_SHADER, SWStage::VS);
static constexpr Stage fragment_fs(AC_HW_PIXEL_SHADER, SWStage::FS);
static constexpr Stage compute_cs(AC_HW_COMPUTE_SHADER, SWStage::CS);
static constexpr Stage tess_eval_vs(AC_HW_VERTEX_SHADER, SWStage::TES);
/* Mesh shading pipeline */
static constexpr Stage task_cs(AC_HW_COMPUTE_SHADER, SWStage::TS);
static constexpr Stage mesh_ngg(AC_HW_NEXT_GEN_GEOMETRY_SHADER, SWStage::MS);
/* GFX10/NGG */
static constexpr Stage vertex_ngg(AC_HW_NEXT_GEN_GEOMETRY_SHADER, SWStage::VS);
static constexpr Stage vertex_geometry_ngg(AC_HW_NEXT_GEN_GEOMETRY_SHADER, SWStage::VS_GS);
static constexpr Stage tess_eval_ngg(AC_HW_NEXT_GEN_GEOMETRY_SHADER, SWStage::TES);
static constexpr Stage tess_eval_geometry_ngg(AC_HW_NEXT_GEN_GEOMETRY_SHADER, SWStage::TES_GS);
/* GFX9 (and GFX10 if NGG isn't used) */
static constexpr Stage vertex_geometry_gs(AC_HW_LEGACY_GEOMETRY_SHADER, SWStage::VS_GS);
static constexpr Stage vertex_tess_control_hs(AC_HW_HULL_SHADER, SWStage::VS_TCS);
static constexpr Stage tess_eval_geometry_gs(AC_HW_LEGACY_GEOMETRY_SHADER, SWStage::TES_GS);
/* pre-GFX9 */
static constexpr Stage vertex_ls(AC_HW_LOCAL_SHADER,
                                 SWStage::VS); /* vertex before tessellation control */
static constexpr Stage vertex_es(AC_HW_EXPORT_SHADER, SWStage::VS); /* vertex before geometry */
static constexpr Stage tess_control_hs(AC_HW_HULL_SHADER, SWStage::TCS);
static constexpr Stage tess_eval_es(AC_HW_EXPORT_SHADER,
                                    SWStage::TES); /* tessellation evaluation before geometry */
static constexpr Stage geometry_gs(AC_HW_LEGACY_GEOMETRY_SHADER, SWStage::GS);
/* Raytracing */
static constexpr Stage raytracing_cs(AC_HW_COMPUTE_SHADER, SWStage::RT);

struct DeviceInfo {
   uint16_t lds_encoding_granule;
   uint16_t lds_alloc_granule;
   uint32_t lds_limit; /* in bytes */
   bool has_16bank_lds;
   uint16_t physical_sgprs;
   uint16_t physical_vgprs;
   uint16_t vgpr_limit;
   uint16_t sgpr_limit;
   uint16_t sgpr_alloc_granule;
   uint16_t vgpr_alloc_granule;
   unsigned scratch_alloc_granule;
   uint16_t max_waves_per_simd;
   unsigned simd_per_cu;
   bool has_fast_fma32 = false;
   bool has_mac_legacy32 = false;
   bool fused_mad_mix = false;
   bool xnack_enabled = false;
   bool sram_ecc_enabled = false;

   int16_t scratch_global_offset_min;
   int16_t scratch_global_offset_max;
   unsigned max_nsa_vgprs;
};

enum class CompilationProgress {
   after_isel,
   after_spilling,
   after_ra,
};

class Program final {
public:
   aco::monotonic_buffer_resource m{65536};
   std::vector<Block> blocks;
   std::vector<RegClass> temp_rc = {s1};
   RegisterDemand max_reg_demand = RegisterDemand();
   ac_shader_config* config;
   struct aco_shader_info info;
   enum amd_gfx_level gfx_level;
   enum radeon_family family;
   DeviceInfo dev;
   unsigned wave_size;
   RegClass lane_mask;
   Stage stage;
   bool needs_exact = false; /* there exists an instruction with disable_wqm = true */
   bool needs_wqm = false;   /* there exists a p_wqm instruction */
   bool has_smem_buffer_or_global_loads = false;
   bool has_pops_overlapped_waves_wait = false;
   bool has_color_exports = false;
   bool is_prolog = false;

   std::vector<uint8_t> constant_data;
   Temp private_segment_buffer;
   Temp scratch_offset;

   uint16_t num_waves = 0;
   uint16_t min_waves = 0;
   unsigned workgroup_size; /* if known; otherwise UINT_MAX */
   bool wgp_mode;

   bool needs_vcc = false;

   CompilationProgress progress;

   bool collect_statistics = false;
   uint32_t statistics[aco_num_statistics];

   float_mode next_fp_mode;
   unsigned next_loop_depth = 0;
   unsigned next_divergent_if_logical_depth = 0;
   unsigned next_uniform_if_depth = 0;

   std::vector<Definition> args_pending_vmem;

   /* For shader part with previous shader part that has lds access. */
   bool pending_lds_access = false;

   struct {
      FILE* output = stderr;
      bool shorten_messages = false;
      void (*func)(void* private_data, enum aco_compiler_debug_level level, const char* message);
      void* private_data;
   } debug;

   uint32_t allocateId(RegClass rc)
   {
      assert(allocationID <= 16777215);
      temp_rc.push_back(rc);
      return allocationID++;
   }

   void allocateRange(unsigned amount)
   {
      assert(allocationID + amount <= 16777216);
      temp_rc.resize(temp_rc.size() + amount);
      allocationID += amount;
   }

   Temp allocateTmp(RegClass rc) { return Temp(allocateId(rc), rc); }

   uint32_t peekAllocationId() { return allocationID; }

   friend void reindex_ssa(Program* program);
   friend void reindex_ssa(Program* program, std::vector<IDSet>& live_out);

   Block* create_and_insert_block()
   {
      Block block;
      return insert_block(std::move(block));
   }

   Block* insert_block(Block&& block)
   {
      block.index = blocks.size();
      block.fp_mode = next_fp_mode;
      block.loop_nest_depth = next_loop_depth;
      block.divergent_if_logical_depth = next_divergent_if_logical_depth;
      block.uniform_if_depth = next_uniform_if_depth;
      blocks.emplace_back(std::move(block));
      return &blocks.back();
   }

private:
   uint32_t allocationID = 1;
};

struct live {
   /* live temps out per block */
   std::vector<IDSet> live_out;
   /* register demand (sgpr/vgpr) per instruction per block */
   std::vector<std::vector<RegisterDemand>> register_demand;
};

struct ra_test_policy {
   /* Force RA to always use its pessimistic fallback algorithm */
   bool skip_optimistic_path = false;
};

void init();

void init_program(Program* program, Stage stage, const struct aco_shader_info* info,
                  enum amd_gfx_level gfx_level, enum radeon_family family, bool wgp_mode,
                  ac_shader_config* config);

void select_program(Program* program, unsigned shader_count, struct nir_shader* const* shaders,
                    ac_shader_config* config, const struct aco_compiler_options* options,
                    const struct aco_shader_info* info, const struct ac_shader_args* args);
void select_trap_handler_shader(Program* program, struct nir_shader* shader,
                                ac_shader_config* config,
                                const struct aco_compiler_options* options,
                                const struct aco_shader_info* info,
                                const struct ac_shader_args* args);
void select_rt_prolog(Program* program, ac_shader_config* config,
                      const struct aco_compiler_options* options,
                      const struct aco_shader_info* info, const struct ac_shader_args* in_args,
                      const struct ac_shader_args* out_args);
void select_vs_prolog(Program* program, const struct aco_vs_prolog_info* pinfo,
                      ac_shader_config* config, const struct aco_compiler_options* options,
                      const struct aco_shader_info* info, const struct ac_shader_args* args);

void select_ps_epilog(Program* program, void* pinfo, ac_shader_config* config,
                      const struct aco_compiler_options* options,
                      const struct aco_shader_info* info, const struct ac_shader_args* args);

void select_tcs_epilog(Program* program, void* pinfo, ac_shader_config* config,
                       const struct aco_compiler_options* options,
                       const struct aco_shader_info* info, const struct ac_shader_args* args);

void select_gl_vs_prolog(Program* program, void* pinfo, ac_shader_config* config,
                         const struct aco_compiler_options* options,
                         const struct aco_shader_info* info, const struct ac_shader_args* args);

void select_ps_prolog(Program* program, void* pinfo, ac_shader_config* config,
                      const struct aco_compiler_options* options,
                      const struct aco_shader_info* info, const struct ac_shader_args* args);

void lower_phis(Program* program);
void calc_min_waves(Program* program);
void update_vgpr_sgpr_demand(Program* program, const RegisterDemand new_demand);
live live_var_analysis(Program* program);
std::vector<uint16_t> dead_code_analysis(Program* program);
void dominator_tree(Program* program);
void insert_exec_mask(Program* program);
void value_numbering(Program* program);
void optimize(Program* program);
void optimize_postRA(Program* program);
void setup_reduce_temp(Program* program);
void lower_to_cssa(Program* program, live& live_vars);
void register_allocation(Program* program, std::vector<IDSet>& live_out_per_block,
                         ra_test_policy = {});
void ssa_elimination(Program* program);
void lower_to_hw_instr(Program* program);
void schedule_program(Program* program, live& live_vars);
void schedule_ilp(Program* program);
void spill(Program* program, live& live_vars);
void insert_wait_states(Program* program);
bool dealloc_vgprs(Program* program);
void insert_NOPs(Program* program);
void form_hard_clauses(Program* program);
unsigned emit_program(Program* program, std::vector<uint32_t>& code,
                      std::vector<struct aco_symbol>* symbols = NULL, bool append_endpgm = true);
/**
 * Returns true if print_asm can disassemble the given program for the current build/runtime
 * configuration
 */
bool check_print_asm_support(Program* program);
bool print_asm(Program* program, std::vector<uint32_t>& binary, unsigned exec_size, FILE* output);
bool validate_ir(Program* program);
bool validate_cfg(Program* program);
bool validate_ra(Program* program);
#ifndef NDEBUG
void perfwarn(Program* program, bool cond, const char* msg, Instruction* instr = NULL);
#else
#define perfwarn(program, cond, msg, ...)                                                          \
   do {                                                                                            \
   } while (0)
#endif

void collect_presched_stats(Program* program);
void collect_preasm_stats(Program* program);
void collect_postasm_stats(Program* program, const std::vector<uint32_t>& code);

struct Instruction_cycle_info {
   /* Latency until the result is ready (if not needing a waitcnt) */
   unsigned latency;

   /* How many cycles issuing this instruction takes (i.e. cycles till the next instruction can be
    * issued)*/
   unsigned issue_cycles;
};

Instruction_cycle_info get_cycle_info(const Program& program, const Instruction& instr);

enum print_flags {
   print_no_ssa = 0x1,
   print_perf_info = 0x2,
   print_kill = 0x4,
   print_live_vars = 0x8,
};

void aco_print_operand(const Operand* operand, FILE* output, unsigned flags = 0);
void aco_print_instr(enum amd_gfx_level gfx_level, const Instruction* instr, FILE* output,
                     unsigned flags = 0);
void aco_print_program(const Program* program, FILE* output, unsigned flags = 0);
void aco_print_program(const Program* program, FILE* output, const live& live_vars,
                       unsigned flags = 0);

void _aco_perfwarn(Program* program, const char* file, unsigned line, const char* fmt, ...);
void _aco_err(Program* program, const char* file, unsigned line, const char* fmt, ...);

#define aco_perfwarn(program, ...) _aco_perfwarn(program, __FILE__, __LINE__, __VA_ARGS__)
#define aco_err(program, ...)      _aco_err(program, __FILE__, __LINE__, __VA_ARGS__)

int get_op_fixed_to_def(Instruction* instr);

/* utilities for dealing with register demand */
RegisterDemand get_live_changes(aco_ptr<Instruction>& instr);
RegisterDemand get_temp_registers(aco_ptr<Instruction>& instr);
RegisterDemand get_demand_before(RegisterDemand demand, aco_ptr<Instruction>& instr,
                                 aco_ptr<Instruction>& instr_before);

/* number of sgprs that need to be allocated but might notbe addressable as s0-s105 */
uint16_t get_extra_sgprs(Program* program);

/* adjust num_waves for workgroup size and LDS limits */
uint16_t max_suitable_waves(Program* program, uint16_t waves);

/* get number of sgprs/vgprs allocated required to address a number of sgprs/vgprs */
uint16_t get_sgpr_alloc(Program* program, uint16_t addressable_sgprs);
uint16_t get_vgpr_alloc(Program* program, uint16_t addressable_vgprs);

/* return number of addressable sgprs/vgprs for max_waves */
uint16_t get_addr_sgpr_from_waves(Program* program, uint16_t max_waves);
uint16_t get_addr_vgpr_from_waves(Program* program, uint16_t max_waves);

bool uses_scratch(Program* program);

typedef struct {
   const int16_t opcode_gfx7[static_cast<int>(aco_opcode::num_opcodes)];
   const int16_t opcode_gfx9[static_cast<int>(aco_opcode::num_opcodes)];
   const int16_t opcode_gfx10[static_cast<int>(aco_opcode::num_opcodes)];
   const int16_t opcode_gfx11[static_cast<int>(aco_opcode::num_opcodes)];
   const std::bitset<static_cast<int>(aco_opcode::num_opcodes)> can_use_input_modifiers;
   const std::bitset<static_cast<int>(aco_opcode::num_opcodes)> can_use_output_modifiers;
   const std::bitset<static_cast<int>(aco_opcode::num_opcodes)> is_atomic;
   const char* name[static_cast<int>(aco_opcode::num_opcodes)];
   const aco::Format format[static_cast<int>(aco_opcode::num_opcodes)];
   /* sizes used for input/output modifiers and constants */
   const unsigned operand_size[static_cast<int>(aco_opcode::num_opcodes)];
   const instr_class classes[static_cast<int>(aco_opcode::num_opcodes)];
   const uint32_t definitions[static_cast<int>(aco_opcode::num_opcodes)];
   const uint32_t operands[static_cast<int>(aco_opcode::num_opcodes)];
} Info;

extern const Info instr_info;

} // namespace aco

#endif /* ACO_IR_H */
