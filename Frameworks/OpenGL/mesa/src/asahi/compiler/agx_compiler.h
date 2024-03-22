/*
 * Copyright 2021 Alyssa Rosenzweig
 * Copyright 2020 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */

#ifndef __AGX_COMPILER_H
#define __AGX_COMPILER_H

#include "compiler/nir/nir.h"
#include "util/half_float.h"
#include "util/u_dynarray.h"
#include "util/u_math.h"
#include "util/u_worklist.h"
#include "agx_compile.h"
#include "agx_minifloat.h"
#include "agx_opcodes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* r0-r127 inclusive, as pairs of 16-bits, gives 256 registers */
#define AGX_NUM_REGS (256)

/* u0-u255 inclusive, as pairs of 16-bits */
#define AGX_NUM_UNIFORMS (512)

enum agx_index_type {
   AGX_INDEX_NULL = 0,
   AGX_INDEX_NORMAL = 1,
   AGX_INDEX_IMMEDIATE = 2,
   AGX_INDEX_UNIFORM = 3,
   AGX_INDEX_REGISTER = 4,
   AGX_INDEX_UNDEF = 5,
};

enum agx_size { AGX_SIZE_16 = 0, AGX_SIZE_32 = 1, AGX_SIZE_64 = 2 };

static inline unsigned
agx_size_align_16(enum agx_size size)
{
   switch (size) {
   case AGX_SIZE_16:
      return 1;
   case AGX_SIZE_32:
      return 2;
   case AGX_SIZE_64:
      return 4;
   }

   unreachable("Invalid size");
}

/* Keep synced with hash_index */
typedef struct {
   /* Sufficient for as many SSA values, immediates, and uniforms as we need. */
   uint32_t value;

   /* Indicates that this source kills the referenced value (because it is the
    * last use in a block and the source is not live after the block). Set by
    * liveness analysis.
    */
   bool kill : 1;

   /* Cache hints */
   bool cache   : 1;
   bool discard : 1;

   /* src - float modifiers */
   bool abs : 1;
   bool neg : 1;

   unsigned channels_m1     : 3;
   enum agx_size size       : 2;
   enum agx_index_type type : 3;
   unsigned padding         : 18;
} agx_index;

static inline unsigned
agx_channels(agx_index idx)
{
   return idx.channels_m1 + 1;
}

static inline unsigned
agx_index_size_16(agx_index idx)
{
   return agx_size_align_16(idx.size) * agx_channels(idx);
}

static inline agx_index
agx_get_vec_index(unsigned value, enum agx_size size, unsigned channels)
{
   return (agx_index){
      .value = value,
      .channels_m1 = channels - 1,
      .size = size,
      .type = AGX_INDEX_NORMAL,
   };
}

static inline agx_index
agx_get_index(unsigned value, enum agx_size size)
{
   return agx_get_vec_index(value, size, 1);
}

static inline agx_index
agx_immediate(uint32_t imm)
{
   assert(imm < (1 << 16) && "overflowed immediate");

   return (agx_index){
      .value = imm,
      .size = AGX_SIZE_16,
      .type = AGX_INDEX_IMMEDIATE,
   };
}

static inline agx_index
agx_immediate_f(float f)
{
   assert(agx_minifloat_exact(f));
   return agx_immediate(agx_minifloat_encode(f));
}

/* in half-words, specify r0h as 1, r1 as 2... */
static inline agx_index
agx_register(uint32_t imm, enum agx_size size)
{
   assert(imm < AGX_NUM_REGS);

   return (agx_index){
      .value = imm,
      .size = size,
      .type = AGX_INDEX_REGISTER,
   };
}

static inline agx_index
agx_register_like(uint32_t imm, agx_index like)
{
   assert(imm < AGX_NUM_REGS);

   return (agx_index){
      .value = imm,
      .channels_m1 = like.channels_m1,
      .size = like.size,
      .type = AGX_INDEX_REGISTER,
   };
}

static inline agx_index
agx_undef(enum agx_size size)
{
   return (agx_index){
      .size = size,
      .type = AGX_INDEX_UNDEF,
   };
}

/* Also in half-words */
static inline agx_index
agx_uniform(uint32_t imm, enum agx_size size)
{
   assert(imm < AGX_NUM_UNIFORMS);

   return (agx_index){
      .value = imm,
      .size = size,
      .type = AGX_INDEX_UNIFORM,
   };
}

static inline agx_index
agx_null()
{
   return (agx_index){.type = AGX_INDEX_NULL};
}

static inline agx_index
agx_zero()
{
   return agx_immediate(0);
}

/* IEEE 754 additive identity -0.0, stored as an 8-bit AGX minifloat: mantissa
 * = exponent = 0, sign bit set */

static inline agx_index
agx_negzero()
{
   return agx_immediate(0x80);
}

static inline agx_index
agx_abs(agx_index idx)
{
   idx.abs = true;
   idx.neg = false;
   return idx;
}

static inline agx_index
agx_neg(agx_index idx)
{
   idx.neg ^= true;
   return idx;
}

/* Replaces an index, preserving any modifiers */

static inline agx_index
agx_replace_index(agx_index old, agx_index replacement)
{
   replacement.abs = old.abs;
   replacement.neg = old.neg;
   return replacement;
}

static inline bool
agx_is_null(agx_index idx)
{
   return idx.type == AGX_INDEX_NULL;
}

/* Compares equivalence as references */

static inline bool
agx_is_equiv(agx_index left, agx_index right)
{
   return (left.type == right.type) && (left.value == right.value);
}

enum agx_icond {
   AGX_ICOND_UEQ = 0,
   AGX_ICOND_ULT = 1,
   AGX_ICOND_UGT = 2,
   /* unknown */
   AGX_ICOND_SEQ = 4,
   AGX_ICOND_SLT = 5,
   AGX_ICOND_SGT = 6,
   /* unknown */
};

enum agx_fcond {
   AGX_FCOND_EQ = 0,
   AGX_FCOND_LT = 1,
   AGX_FCOND_GT = 2,
   AGX_FCOND_LTN = 3,
   /* unknown */
   AGX_FCOND_GE = 5,
   AGX_FCOND_LE = 6,
   AGX_FCOND_GTN = 7,
};

enum agx_round {
   AGX_ROUND_RTZ = 0,
   AGX_ROUND_RTE = 1,
};

enum agx_convert {
   AGX_CONVERT_U8_TO_F = 0,
   AGX_CONVERT_S8_TO_F = 1,
   AGX_CONVERT_F_TO_U16 = 4,
   AGX_CONVERT_F_TO_S16 = 5,
   AGX_CONVERT_U16_TO_F = 6,
   AGX_CONVERT_S16_TO_F = 7,
   AGX_CONVERT_F_TO_U32 = 8,
   AGX_CONVERT_F_TO_S32 = 9,
   AGX_CONVERT_U32_TO_F = 10,
   AGX_CONVERT_S32_TO_F = 11
};

enum agx_lod_mode {
   AGX_LOD_MODE_AUTO_LOD = 0,
   AGX_LOD_MODE_AUTO_LOD_BIAS_UNIFORM = 1,
   AGX_LOD_MODE_LOD_MIN_UNIFORM = 2,
   AGX_LOD_MODE_AUTO_LOD_BIAS = 5,
   AGX_LOD_MODE_LOD_MIN = 6,
   AGX_LOD_MODE_LOD_GRAD = 4,
   AGX_LOD_MODE_LOD_GRAD_MIN = 12
};

/* Forward declare for branch target */
struct agx_block;

/* Keep synced with hash_instr */
typedef struct {
   /* Must be first */
   struct list_head link;

   /* The sources list.
    *
    * As a special case to workaround ordering issues when translating phis, if
    * nr_srcs == 0 and the opcode is PHI, holds a pointer to the NIR phi node.
    */
   union {
      agx_index *src;
      nir_phi_instr *phi;
   };

   /* Data flow */
   agx_index *dest;

   enum agx_opcode op;

   uint8_t nr_dests;
   uint8_t nr_srcs;

   /* TODO: More efficient */
   union {
      enum agx_icond icond;
      enum agx_fcond fcond;
   };

   union {
      uint64_t imm;
      uint32_t writeout;
      uint32_t truth_table;
      uint32_t component;
      uint32_t channels;
      uint32_t bfi_mask;
      uint16_t pixel_offset;
      uint16_t zs;
      int16_t stack_size;
      enum agx_sr sr;
      enum agx_round round;
      enum agx_atomic_opc atomic_opc;
      enum agx_lod_mode lod_mode;
      struct agx_block *target;
   };

   /* For local access */
   enum agx_format format;

   /* Number of nested control flow layers to jump by. TODO: Optimize */
   uint32_t nest;

   /* Invert icond/fcond */
   bool invert_cond : 1;

   /* TODO: Handle tex ops more efficient */
   enum agx_dim dim       : 4;
   bool offset            : 1;
   bool shadow            : 1;
   bool query_lod         : 1;
   enum agx_gather gather : 3;

   /* TODO: Handle iter ops more efficient */
   enum agx_interpolation interpolation : 2;

   /* Final st_vary op */
   bool last : 1;

   /* Shift for a bitwise or memory op (conflicts with format for memory ops) */
   unsigned shift : 4;

   /* Scoreboard index, 0 or 1. Leave as 0 for instructions that do not require
    * scoreboarding (everything but memory load/store and texturing). */
   unsigned scoreboard : 1;

   /* Output modifiers */
   bool saturate : 1;
   unsigned mask : 4;

   unsigned padding : 8;
} agx_instr;

static inline void
agx_replace_src(agx_instr *I, unsigned src_index, agx_index replacement)
{
   I->src[src_index] = agx_replace_index(I->src[src_index], replacement);
}

struct agx_block;

typedef struct agx_block {
   /* Link to next block. Must be first */
   struct list_head link;

   /* List of instructions emitted for the current block */
   struct list_head instructions;

   /* Index of the block in source order */
   unsigned index;

   /* Control flow graph */
   struct agx_block *successors[2];
   struct util_dynarray predecessors;
   bool unconditional_jumps;

   /* Liveness analysis results */
   BITSET_WORD *live_in;
   BITSET_WORD *live_out;

   /* For visited blocks during register assignment and live-out registers, the
    * mapping of SSA names to registers at the end of the block.
    */
   uint8_t *ssa_to_reg_out;

   /* Register allocation */
   BITSET_DECLARE(regs_out, AGX_NUM_REGS);

   /* Is this block a loop header? If not, all of its predecessors precede it in
    * source order.
    */
   bool loop_header;

   /* Offset of the block in the emitted binary */
   off_t offset, last_offset;

   /** Available for passes to use for metadata */
   uint8_t pass_flags;
} agx_block;

typedef struct {
   nir_shader *nir;
   gl_shader_stage stage;
   bool is_preamble;

   struct list_head blocks; /* list of agx_block */
   struct agx_shader_info *out;
   struct agx_shader_key *key;

   /* Maximum block index */
   unsigned num_blocks;

   /* For creating temporaries */
   unsigned alloc;

   /* I don't really understand how writeout ops work yet */
   bool did_writeout;

   /* Has r0l been zeroed yet due to control flow? */
   bool any_cf;

   /* Number of nested control flow structures within the innermost loop. Since
    * NIR is just loop and if-else, this is the number of nested if-else
    * statements in the loop */
   unsigned loop_nesting;

   /* Total nesting across all loops, to determine if we need push_exec */
   unsigned total_nesting;

   /* Whether loop being emitted used any `continue` jumps */
   bool loop_continues;

   /* During instruction selection, for inserting control flow */
   agx_block *current_block;
   agx_block *continue_block;
   agx_block *break_block;
   agx_block *after_block;
   agx_block **indexed_nir_blocks;

   /* During instruction selection, map from vector agx_index to its scalar
    * components, populated by a split. */
   struct hash_table_u64 *allocated_vec;

   /* During instruction selection, preloaded values,
    * or NULL if it hasn't been preloaded
    */
   agx_index vertex_id, instance_id;

   /* Stats for shader-db */
   unsigned loop_count;
   unsigned spills;
   unsigned fills;
   unsigned max_reg;
} agx_context;

static inline void
agx_remove_instruction(agx_instr *ins)
{
   list_del(&ins->link);
}

static inline agx_index
agx_vec_temp(agx_context *ctx, enum agx_size size, unsigned channels)
{
   return agx_get_vec_index(ctx->alloc++, size, channels);
}

static inline agx_index
agx_temp(agx_context *ctx, enum agx_size size)
{
   return agx_get_index(ctx->alloc++, size);
}

static enum agx_size
agx_size_for_bits(unsigned bits)
{
   switch (bits) {
   case 1:
   case 8:
   case 16:
      return AGX_SIZE_16;
   case 32:
      return AGX_SIZE_32;
   case 64:
      return AGX_SIZE_64;
   default:
      unreachable("Invalid bitsize");
   }
}

static inline agx_index
agx_def_index(nir_def *ssa)
{
   return agx_get_vec_index(ssa->index, agx_size_for_bits(ssa->bit_size),
                            ssa->num_components);
}

static inline agx_index
agx_src_index(nir_src *src)
{
   return agx_def_index(src->ssa);
}

static inline agx_index
agx_vec_for_def(agx_context *ctx, nir_def *def)
{
   return agx_vec_temp(ctx, agx_size_for_bits(def->bit_size),
                       def->num_components);
}

static inline agx_index
agx_vec_for_intr(agx_context *ctx, nir_intrinsic_instr *instr)
{
   return agx_vec_for_def(ctx, &instr->def);
}

static inline unsigned
agx_num_predecessors(agx_block *block)
{
   return util_dynarray_num_elements(&block->predecessors, agx_block *);
}

static inline agx_block *
agx_start_block(agx_context *ctx)
{
   agx_block *first = list_first_entry(&ctx->blocks, agx_block, link);
   assert(agx_num_predecessors(first) == 0);
   return first;
}

/* Iterators for AGX IR */

#define agx_foreach_block(ctx, v)                                              \
   list_for_each_entry(agx_block, v, &ctx->blocks, link)

#define agx_foreach_block_rev(ctx, v)                                          \
   list_for_each_entry_rev(agx_block, v, &ctx->blocks, link)

#define agx_foreach_block_from(ctx, from, v)                                   \
   list_for_each_entry_from(agx_block, v, from, &ctx->blocks, link)

#define agx_foreach_block_from_rev(ctx, from, v)                               \
   list_for_each_entry_from_rev(agx_block, v, from, &ctx->blocks, link)

#define agx_foreach_instr_in_block(block, v)                                   \
   list_for_each_entry(agx_instr, v, &(block)->instructions, link)

#define agx_foreach_instr_in_block_rev(block, v)                               \
   list_for_each_entry_rev(agx_instr, v, &(block)->instructions, link)

#define agx_foreach_instr_in_block_safe(block, v)                              \
   list_for_each_entry_safe(agx_instr, v, &(block)->instructions, link)

#define agx_foreach_instr_in_block_safe_rev(block, v)                          \
   list_for_each_entry_safe_rev(agx_instr, v, &(block)->instructions, link)

#define agx_foreach_instr_in_block_from(block, v, from)                        \
   list_for_each_entry_from(agx_instr, v, from, &(block)->instructions, link)

#define agx_foreach_instr_in_block_from_rev(block, v, from)                    \
   list_for_each_entry_from_rev(agx_instr, v, from, &(block)->instructions,    \
                                link)

#define agx_foreach_instr_global(ctx, v)                                       \
   agx_foreach_block(ctx, v_block)                                             \
      agx_foreach_instr_in_block(v_block, v)

#define agx_foreach_instr_global_rev(ctx, v)                                   \
   agx_foreach_block_rev(ctx, v_block)                                         \
      agx_foreach_instr_in_block_rev(v_block, v)

#define agx_foreach_instr_global_safe(ctx, v)                                  \
   agx_foreach_block(ctx, v_block)                                             \
      agx_foreach_instr_in_block_safe(v_block, v)

#define agx_foreach_instr_global_safe_rev(ctx, v)                              \
   agx_foreach_block_rev(ctx, v_block)                                         \
      agx_foreach_instr_in_block_safe_rev(v_block, v)

/* Based on set_foreach, expanded with automatic type casts */

#define agx_foreach_successor(blk, v)                                          \
   agx_block *v;                                                               \
   agx_block **_v;                                                             \
   for (_v = (agx_block **)&blk->successors[0], v = *_v;                       \
        v != NULL && _v < (agx_block **)&blk->successors[2]; _v++, v = *_v)

#define agx_foreach_predecessor(blk, v)                                        \
   util_dynarray_foreach(&blk->predecessors, agx_block *, v)

#define agx_foreach_src(ins, v) for (unsigned v = 0; v < ins->nr_srcs; ++v)

#define agx_foreach_dest(ins, v) for (unsigned v = 0; v < ins->nr_dests; ++v)

#define agx_foreach_ssa_src(ins, v)                                            \
   agx_foreach_src(ins, v)                                                     \
      if (ins->src[v].type == AGX_INDEX_NORMAL)

#define agx_foreach_ssa_dest(ins, v)                                           \
   agx_foreach_dest(ins, v)                                                    \
      if (ins->dest[v].type == AGX_INDEX_NORMAL)

/* Phis only come at the start (after else instructions) so we stop as soon as
 * we hit a non-phi
 */
#define agx_foreach_phi_in_block(block, v)                                     \
   agx_foreach_instr_in_block(block, v)                                        \
      if (v->op == AGX_OPCODE_ELSE_ICMP || v->op == AGX_OPCODE_ELSE_FCMP)      \
         continue;                                                             \
      else if (v->op != AGX_OPCODE_PHI)                                        \
         break;                                                                \
      else

/*
 * Find the index of a predecessor, used as the implicit order of phi sources.
 */
static inline unsigned
agx_predecessor_index(agx_block *succ, agx_block *pred)
{
   unsigned index = 0;

   agx_foreach_predecessor(succ, x) {
      if (*x == pred)
         return index;

      index++;
   }

   unreachable("Invalid predecessor");
}

static inline agx_block *
agx_prev_block(agx_block *ins)
{
   return list_last_entry(&(ins->link), agx_block, link);
}

static inline agx_instr *
agx_prev_op(agx_instr *ins)
{
   return list_last_entry(&(ins->link), agx_instr, link);
}

static inline agx_instr *
agx_first_instr(agx_block *block)
{
   if (list_is_empty(&block->instructions))
      return NULL;
   else
      return list_first_entry(&block->instructions, agx_instr, link);
}

static inline agx_instr *
agx_last_instr(agx_block *block)
{
   if (list_is_empty(&block->instructions))
      return NULL;
   else
      return list_last_entry(&block->instructions, agx_instr, link);
}

static inline agx_instr *
agx_next_op(agx_instr *ins)
{
   return list_first_entry(&(ins->link), agx_instr, link);
}

static inline agx_block *
agx_next_block(agx_block *block)
{
   return list_first_entry(&(block->link), agx_block, link);
}

static inline agx_block *
agx_exit_block(agx_context *ctx)
{
   agx_block *last = list_last_entry(&ctx->blocks, agx_block, link);
   assert(!last->successors[0] && !last->successors[1]);
   return last;
}

#define agx_worklist_init(ctx, w)        u_worklist_init(w, ctx->num_blocks, ctx)
#define agx_worklist_push_head(w, block) u_worklist_push_head(w, block, index)
#define agx_worklist_push_tail(w, block) u_worklist_push_tail(w, block, index)
#define agx_worklist_peek_head(w)        u_worklist_peek_head(w, agx_block, index)
#define agx_worklist_pop_head(w)         u_worklist_pop_head(w, agx_block, index)
#define agx_worklist_peek_tail(w)        u_worklist_peek_tail(w, agx_block, index)
#define agx_worklist_pop_tail(w)         u_worklist_pop_tail(w, agx_block, index)

/* Like in NIR, for use with the builder */

enum agx_cursor_option {
   agx_cursor_after_block,
   agx_cursor_before_instr,
   agx_cursor_after_instr
};

typedef struct {
   enum agx_cursor_option option;

   union {
      agx_block *block;
      agx_instr *instr;
   };
} agx_cursor;

static inline agx_cursor
agx_after_block(agx_block *block)
{
   return (agx_cursor){
      .option = agx_cursor_after_block,
      .block = block,
   };
}

static inline agx_cursor
agx_before_instr(agx_instr *instr)
{
   return (agx_cursor){
      .option = agx_cursor_before_instr,
      .instr = instr,
   };
}

static inline agx_cursor
agx_after_instr(agx_instr *instr)
{
   return (agx_cursor){
      .option = agx_cursor_after_instr,
      .instr = instr,
   };
}

static inline agx_cursor
agx_before_nonempty_block(agx_block *block)
{
   agx_instr *I = list_first_entry(&block->instructions, agx_instr, link);
   assert(I != NULL);

   return agx_before_instr(I);
}

static inline agx_cursor
agx_before_block(agx_block *block)
{
   if (list_is_empty(&block->instructions))
      return agx_after_block(block);
   else
      return agx_before_nonempty_block(block);
}

static inline bool
instr_after_logical_end(const agx_instr *I)
{
   switch (I->op) {
   case AGX_OPCODE_JMP_EXEC_ANY:
   case AGX_OPCODE_JMP_EXEC_NONE:
   case AGX_OPCODE_POP_EXEC:
   case AGX_OPCODE_BREAK:
   case AGX_OPCODE_IF_ICMP:
   case AGX_OPCODE_WHILE_ICMP:
   case AGX_OPCODE_IF_FCMP:
   case AGX_OPCODE_WHILE_FCMP:
   case AGX_OPCODE_STOP:
      return true;
   default:
      return false;
   }
}

/*
 * Get a cursor inserting at the logical end of the block. In particular, this
 * is before branches or control flow instructions, which occur after the
 * logical end but before the physical end.
 */
static inline agx_cursor
agx_after_block_logical(agx_block *block)
{
   /* Search for the first instruction that's not past the logical end */
   agx_foreach_instr_in_block_rev(block, I) {
      if (!instr_after_logical_end(I))
         return agx_after_instr(I);
   }

   /* If we got here, the block is either empty or entirely control flow */
   return agx_before_block(block);
}

/* IR builder in terms of cursor infrastructure */

typedef struct {
   agx_context *shader;
   agx_cursor cursor;
} agx_builder;

static inline agx_builder
agx_init_builder(agx_context *ctx, agx_cursor cursor)
{
   return (agx_builder){
      .shader = ctx,
      .cursor = cursor,
   };
}

/* Insert an instruction at the cursor and move the cursor */

static inline void
agx_builder_insert(agx_cursor *cursor, agx_instr *I)
{
   switch (cursor->option) {
   case agx_cursor_after_instr:
      list_add(&I->link, &cursor->instr->link);
      cursor->instr = I;
      return;

   case agx_cursor_after_block:
      list_addtail(&I->link, &cursor->block->instructions);
      cursor->option = agx_cursor_after_instr;
      cursor->instr = I;
      return;

   case agx_cursor_before_instr:
      list_addtail(&I->link, &cursor->instr->link);
      cursor->option = agx_cursor_after_instr;
      cursor->instr = I;
      return;
   }

   unreachable("Invalid cursor option");
}

/* Routines defined for AIR */

void agx_print_instr(const agx_instr *I, FILE *fp);
void agx_print_block(const agx_block *block, FILE *fp);
void agx_print_shader(const agx_context *ctx, FILE *fp);
void agx_optimizer(agx_context *ctx);
void agx_lower_pseudo(agx_context *ctx);
void agx_lower_uniform_sources(agx_context *ctx);
void agx_opt_cse(agx_context *ctx);
void agx_dce(agx_context *ctx, bool partial);
void agx_pressure_schedule(agx_context *ctx);
void agx_ra(agx_context *ctx);
void agx_lower_64bit_postra(agx_context *ctx);
void agx_insert_waits(agx_context *ctx);
void agx_opt_empty_else(agx_context *ctx);
void agx_opt_break_if(agx_context *ctx);
void agx_opt_jmp_none(agx_context *ctx);
void agx_pack_binary(agx_context *ctx, struct util_dynarray *emission);

#ifndef NDEBUG
void agx_validate(agx_context *ctx, const char *after_str);
#else
static inline void
agx_validate(UNUSED agx_context *ctx, UNUSED const char *after_str)
{
   return;
}
#endif

enum agx_size agx_split_width(const agx_instr *I);
bool agx_allows_16bit_immediate(agx_instr *I);

struct agx_copy {
   /* Base register destination of the copy */
   unsigned dest;

   /* Source of the copy */
   agx_index src;

   /* Whether the copy has been handled. Callers must leave to false. */
   bool done;
};

void agx_emit_parallel_copies(agx_builder *b, struct agx_copy *copies,
                              unsigned n);

void agx_compute_liveness(agx_context *ctx);
void agx_liveness_ins_update(BITSET_WORD *live, agx_instr *I);

bool agx_nir_lower_sample_mask(nir_shader *s, unsigned nr_samples);
bool agx_nir_lower_texture(nir_shader *s);
bool agx_nir_opt_preamble(nir_shader *s, unsigned *preamble_size);
bool agx_nir_lower_load_mask(nir_shader *shader);
bool agx_nir_lower_address(nir_shader *shader);
bool agx_nir_lower_ubo(nir_shader *shader);
bool agx_nir_lower_shared_bitsize(nir_shader *shader);
bool agx_nir_lower_frag_sidefx(nir_shader *s);

extern int agx_compiler_debug;

#ifdef __cplusplus
} /* extern C */
#endif

#endif
