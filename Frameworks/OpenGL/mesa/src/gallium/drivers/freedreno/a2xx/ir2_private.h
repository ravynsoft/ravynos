/*
 * Copyright (C) 2018 Jonathan Marek <jonathan@marek.ca>
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
 *
 * Authors:
 *    Jonathan Marek <jonathan@marek.ca>
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ir2/instr-a2xx.h"
#include "fd2_program.h"
#include "ir2.h"

enum ir2_src_type {
   IR2_SRC_SSA,
   IR2_SRC_REG,
   IR2_SRC_INPUT,
   IR2_SRC_CONST,
};

struct ir2_src {
   /* num can mean different things
    *   ssa: index of instruction
    *   reg: index in ctx->reg array
    *   input: index in ctx->input array
    *   const: constant index (C0, C1, etc)
    */
   uint16_t num;
   uint8_t swizzle;
   enum ir2_src_type type : 2;
   uint8_t abs : 1;
   uint8_t negate : 1;
   uint8_t : 4;
};

struct ir2_reg_component {
   uint8_t c : 3;     /* assigned x/y/z/w (7=dont write, for fetch instr) */
   bool alloc : 1;    /* is it currently allocated */
   uint8_t ref_count; /* for ra */
};

struct ir2_reg {
   uint8_t idx; /* assigned hardware register */
   uint8_t ncomp;

   uint8_t loop_depth;
   bool initialized;
   /* block_idx to free on (-1 = free on ref_count==0) */
   int block_idx_free;
   struct ir2_reg_component comp[4];
};

struct ir2_instr {
   unsigned idx;

   unsigned block_idx;

   enum {
      IR2_NONE,
      IR2_FETCH,
      IR2_ALU,
      IR2_CF,
   } type : 2;

   /* instruction needs to be emitted (for scheduling) */
   bool need_emit : 1;

   /* predicate value - (usually) same for entire block */
   uint8_t pred : 2;

   /* src */
   uint8_t src_count;
   struct ir2_src src[4];

   /* dst */
   bool is_ssa;
   union {
      struct ir2_reg ssa;
      struct ir2_reg *reg;
   };

   /* type-specific */
   union {
      struct {
         instr_fetch_opc_t opc : 5;
         union {
            struct {
               uint8_t const_idx;
               uint8_t const_idx_sel;
            } vtx;
            struct {
               bool is_cube : 1;
               bool is_rect : 1;
               uint8_t samp_id;
            } tex;
         };
      } fetch;
      struct {
         /* store possible opcs, then we can choose vector/scalar instr */
         instr_scalar_opc_t scalar_opc : 6;
         instr_vector_opc_t vector_opc : 5;
         /* same as nir */
         uint8_t write_mask : 4;
         bool saturate : 1;

         /* export idx (-1 no export) */
         int8_t export;

         /* for scalarized 2 src instruction */
         uint8_t src1_swizzle;
      } alu;
      struct {
         /* jmp dst block_idx */
         uint8_t block_idx;
      } cf;
   };
};

struct ir2_sched_instr {
   uint32_t reg_state[8];
   struct ir2_instr *instr, *instr_s;
};

struct ir2_context {
   struct fd2_shader_stateobj *so;

   unsigned block_idx, pred_idx;
   uint8_t pred;
   bool block_has_jump[64];

   unsigned loop_last_block[64];
   unsigned loop_depth;

   nir_shader *nir;

   /* ssa index of position output */
   struct ir2_src position;

   /* to translate SSA ids to instruction ids */
   int16_t ssa_map[1024];

   struct ir2_shader_info *info;
   struct ir2_frag_linkage *f;

   int prev_export;

   /* RA state */
   struct ir2_reg *live_regs[64];
   uint32_t reg_state[256 / 32]; /* 64*4 bits */

   /* inputs */
   struct ir2_reg input[16 + 1]; /* 16 + param */

   /* non-ssa regs */
   struct ir2_reg reg[1024];
   unsigned reg_count;

   struct ir2_instr instr[0x300];
   unsigned instr_count;

   struct ir2_sched_instr instr_sched[0x180];
   unsigned instr_sched_count;
};

void assemble(struct ir2_context *ctx, bool binning);

void ir2_nir_compile(struct ir2_context *ctx, bool binning);
bool ir2_nir_lower_scalar(nir_shader *shader);

void ra_count_refs(struct ir2_context *ctx);
void ra_reg(struct ir2_context *ctx, struct ir2_reg *reg, int force_idx,
            bool export, uint8_t export_writemask);
void ra_src_free(struct ir2_context *ctx, struct ir2_instr *instr);
void ra_block_free(struct ir2_context *ctx, unsigned block);

void cp_src(struct ir2_context *ctx);
void cp_export(struct ir2_context *ctx);

/* utils */
enum {
   IR2_SWIZZLE_Y = 1 << 0,
   IR2_SWIZZLE_Z = 2 << 0,
   IR2_SWIZZLE_W = 3 << 0,

   IR2_SWIZZLE_ZW = 2 << 0 | 2 << 2,

   IR2_SWIZZLE_YXW = 1 << 0 | 3 << 2 | 1 << 4,

   IR2_SWIZZLE_XXXX = 0 << 0 | 3 << 2 | 2 << 4 | 1 << 6,
   IR2_SWIZZLE_YYYY = 1 << 0 | 0 << 2 | 3 << 4 | 2 << 6,
   IR2_SWIZZLE_ZZZZ = 2 << 0 | 1 << 2 | 0 << 4 | 3 << 6,
   IR2_SWIZZLE_WWWW = 3 << 0 | 2 << 2 | 1 << 4 | 0 << 6,
   IR2_SWIZZLE_WYWW = 3 << 0 | 0 << 2 | 1 << 4 | 0 << 6,
   IR2_SWIZZLE_XYXY = 0 << 0 | 0 << 2 | 2 << 4 | 2 << 6,
   IR2_SWIZZLE_ZZXY = 2 << 0 | 1 << 2 | 2 << 4 | 2 << 6,
   IR2_SWIZZLE_YXZZ = 1 << 0 | 3 << 2 | 0 << 4 | 3 << 6,
};

#define compile_error(ctx, args...)                                            \
   ({                                                                          \
      printf(args);                                                            \
      assert(0);                                                               \
   })

static inline struct ir2_src
ir2_src(uint16_t num, uint8_t swizzle, enum ir2_src_type type)
{
   return (struct ir2_src){.num = num, .swizzle = swizzle, .type = type};
}

/* ir2_assemble uses it .. */
struct ir2_src ir2_zero(struct ir2_context *ctx);

#define ir2_foreach_instr(it, ctx)                                             \
   for (struct ir2_instr *it = (ctx)->instr; ({                                \
           while (it != &(ctx)->instr[(ctx)->instr_count] &&                   \
                  it->type == IR2_NONE)                                        \
              it++;                                                            \
           it != &(ctx)->instr[(ctx)->instr_count];                            \
        });                                                                    \
        it++)

#define ir2_foreach_live_reg(it, ctx)                                          \
   for (struct ir2_reg **__ptr = (ctx)->live_regs, *it; ({                     \
           while (__ptr != &(ctx)->live_regs[64] && *__ptr == NULL)            \
              __ptr++;                                                         \
           __ptr != &(ctx)->live_regs[64] ? (it = *__ptr) : NULL;              \
        });                                                                    \
        it++)

#define ir2_foreach_avail(it)                                                  \
   for (struct ir2_instr **__instrp = avail, *it;                              \
        it = *__instrp, __instrp != &avail[avail_count]; __instrp++)

#define ir2_foreach_src(it, instr)                                             \
   for (struct ir2_src *it = instr->src; it != &instr->src[instr->src_count];  \
        it++)

/* mask for register allocation
 * 64 registers with 4 components each = 256 bits
 */
/* typedef struct {
        uint64_t data[4];
} regmask_t; */

static inline bool
mask_isset(uint32_t *mask, unsigned num)
{
   return !!(mask[num / 32] & 1 << num % 32);
}

static inline void
mask_set(uint32_t *mask, unsigned num)
{
   mask[num / 32] |= 1 << num % 32;
}

static inline void
mask_unset(uint32_t *mask, unsigned num)
{
   mask[num / 32] &= ~(1 << num % 32);
}

static inline unsigned
mask_reg(uint32_t *mask, unsigned num)
{
   return mask[num / 8] >> num % 8 * 4 & 0xf;
}

static inline bool
is_export(struct ir2_instr *instr)
{
   return instr->type == IR2_ALU && instr->alu.export >= 0;
}

static inline instr_alloc_type_t
export_buf(unsigned num)
{
   return num < 32 ? SQ_PARAMETER_PIXEL : num >= 62 ? SQ_POSITION : SQ_MEMORY;
}

/* component c for channel i */
static inline unsigned
swiz_set(unsigned c, unsigned i)
{
   return ((c - i) & 3) << i * 2;
}

/* get swizzle in channel i */
static inline unsigned
swiz_get(unsigned swiz, unsigned i)
{
   return ((swiz >> i * 2) + i) & 3;
}

static inline unsigned
swiz_merge(unsigned swiz0, unsigned swiz1)
{
   unsigned swiz = 0;
   for (int i = 0; i < 4; i++)
      swiz |= swiz_set(swiz_get(swiz0, swiz_get(swiz1, i)), i);
   return swiz;
}

static inline void
swiz_merge_p(uint8_t *swiz0, unsigned swiz1)
{
   unsigned swiz = 0;
   for (int i = 0; i < 4; i++)
      swiz |= swiz_set(swiz_get(*swiz0, swiz_get(swiz1, i)), i);
   *swiz0 = swiz;
}

static inline struct ir2_reg *
get_reg(struct ir2_instr *instr)
{
   return instr->is_ssa ? &instr->ssa : instr->reg;
}

static inline struct ir2_reg *
get_reg_src(struct ir2_context *ctx, struct ir2_src *src)
{
   switch (src->type) {
   case IR2_SRC_INPUT:
      return &ctx->input[src->num];
   case IR2_SRC_SSA:
      return &ctx->instr[src->num].ssa;
   case IR2_SRC_REG:
      return &ctx->reg[src->num];
   default:
      return NULL;
   }
}

/* gets a ncomp value for the dst */
static inline unsigned
dst_ncomp(struct ir2_instr *instr)
{
   if (instr->is_ssa)
      return instr->ssa.ncomp;

   if (instr->type == IR2_FETCH)
      return instr->reg->ncomp;

   assert(instr->type == IR2_ALU);

   unsigned ncomp = 0;
   for (int i = 0; i < instr->reg->ncomp; i++)
      ncomp += !!(instr->alu.write_mask & 1 << i);
   return ncomp;
}

/* gets a ncomp value for the src registers */
static inline unsigned
src_ncomp(struct ir2_instr *instr)
{
   if (instr->type == IR2_FETCH) {
      switch (instr->fetch.opc) {
      case VTX_FETCH:
         return 1;
      case TEX_FETCH:
         return instr->fetch.tex.is_cube ? 3 : 2;
      case TEX_SET_TEX_LOD:
         return 1;
      default:
         assert(0);
      }
   }

   switch (instr->alu.scalar_opc) {
   case PRED_SETEs ... KILLONEs:
      return 1;
   default:
      break;
   }

   switch (instr->alu.vector_opc) {
   case DOT2ADDv:
      return 2;
   case DOT3v:
      return 3;
   case DOT4v:
   case CUBEv:
   case PRED_SETE_PUSHv:
      return 4;
   default:
      return dst_ncomp(instr);
   }
}
