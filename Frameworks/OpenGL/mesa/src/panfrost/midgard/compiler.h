/*
 * Copyright (C) 2019-2020 Collabora, Ltd.
 * Copyright (C) 2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
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

#ifndef _MDG_COMPILER_H
#define _MDG_COMPILER_H

#include "helpers.h"
#include "midgard.h"
#include "midgard_compile.h"
#include "midgard_ops.h"

#include "util/hash_table.h"
#include "util/list.h"
#include "util/set.h"
#include "util/u_dynarray.h"
#include "util/u_math.h"

#include "compiler/glsl_types.h"
#include "compiler/nir/nir.h"
#include "panfrost/util/lcra.h"
#include "panfrost/util/pan_ir.h"

/* Forward declare */
struct midgard_block;

/* Target types. Defaults to TARGET_GOTO (the type corresponding directly to
 * the hardware), hence why that must be zero. TARGET_DISCARD signals this
 * instruction is actually a discard op. */

#define TARGET_GOTO         0
#define TARGET_BREAK        1
#define TARGET_CONTINUE     2
#define TARGET_DISCARD      3
#define TARGET_TILEBUF_WAIT 4

typedef struct midgard_branch {
   /* If conditional, the condition is specified in r31.w */
   bool conditional;

   /* For conditionals, if this is true, we branch on FALSE. If false, we branch
    * on TRUE. */
   bool invert_conditional;

   /* Branch targets: the start of a block, the start of a loop (continue), the
    * end of a loop (break). Value is one of TARGET_ */
   unsigned target_type;

   /* The actual target */
   union {
      int target_block;
      int target_break;
      int target_continue;
   };
} midgard_branch;

/* Generic in-memory data type repesenting a single logical instruction, rather
 * than a single instruction group. This is the preferred form for code gen.
 * Multiple midgard_insturctions will later be combined during scheduling,
 * though this is not represented in this structure.  Its format bridges
 * the low-level binary representation with the higher level semantic meaning.
 *
 * Notably, it allows registers to be specified as block local SSA, for code
 * emitted before the register allocation pass.
 */

#define MIR_SRC_COUNT      4
#define MIR_VEC_COMPONENTS 16

typedef struct midgard_instruction {
   /* Must be first for casting */
   struct list_head link;

   unsigned type; /* ALU, load/store, texture */

   /* Instruction arguments represented as block-local SSA
    * indices, rather than registers. ~0 means unused. */
   unsigned src[MIR_SRC_COUNT];
   unsigned dest;

   /* vec16 swizzle, unpacked, per source */
   unsigned swizzle[MIR_SRC_COUNT][MIR_VEC_COMPONENTS];

   /* Types! */
   nir_alu_type src_types[MIR_SRC_COUNT];
   nir_alu_type dest_type;

   /* Packing ops have non-32-bit dest types even though they functionally
    * work at the 32-bit level, use this as a signal to disable copyprop.
    * We maybe need synthetic pack ops instead. */
   bool is_pack;

   /* Modifiers, depending on type */
   union {
      struct {
         bool src_abs[MIR_SRC_COUNT];
         bool src_neg[MIR_SRC_COUNT];
      };

      struct {
         bool src_shift[MIR_SRC_COUNT];
      };
   };

   /* Out of the union for csel (could maybe be fixed..) */
   bool src_invert[MIR_SRC_COUNT];

   /* If the op supports it */
   enum midgard_roundmode roundmode;

   /* For textures: should helpers execute this instruction (instead of
    * just helping with derivatives)? Should helpers terminate after? */
   bool helper_terminate;
   bool helper_execute;

   /* I.e. (1 << alu_bit) */
   int unit;

   bool has_constants;
   midgard_constants constants;
   uint16_t inline_constant;
   bool has_inline_constant;

   bool compact_branch;
   uint8_t writeout;
   bool last_writeout;

   /* Masks in a saneish format. One bit per channel, not packed fancy.
    * Use this instead of the op specific ones, and switch over at emit
    * time */

   uint16_t mask;

   /* Hint for the register allocator not to spill the destination written
    * from this instruction (because it is a spill/unspill node itself).
    * Bitmask of spilled classes */

   unsigned no_spill;

   /* Generic hint for intra-pass use */
   bool hint;

   /* During scheduling, the backwards dependency graph
    * (DAG). nr_dependencies is the number of unscheduled
    * instructions that must still be scheduled after
    * (before) this instruction. dependents are which
    * instructions need to be scheduled before (after) this
    * instruction. */

   unsigned nr_dependencies;
   BITSET_WORD *dependents;

   /* Use this in conjunction with `type` */
   unsigned op;

   /* This refers to midgard_outmod_float or midgard_outmod_int.
    * In case of a ALU op, use midgard_is_integer_out_op() to know which
    * one is used.
    * If it's a texture op, it's always midgard_outmod_float. */
   unsigned outmod;

   union {
      midgard_load_store_word load_store;
      midgard_texture_word texture;

      midgard_branch branch;
   };

   unsigned bundle_id;
} midgard_instruction;

typedef struct midgard_block {
   pan_block base;

   bool scheduled;

   /* List of midgard_bundles emitted (after the scheduler has run) */
   struct util_dynarray bundles;

   /* Number of quadwords _actually_ emitted, as determined after scheduling */
   unsigned quadword_count;

   /* Indicates this is a fixed-function fragment epilogue block */
   bool epilogue;

   /* Are helper invocations required by this block? */
   bool helpers_in;
} midgard_block;

typedef struct midgard_bundle {
   /* Tag for the overall bundle */
   int tag;

   /* Instructions contained by the bundle. instruction_count <= 6 (vmul,
    * sadd, vadd, smul, vlut, branch) */
   int instruction_count;
   midgard_instruction *instructions[6];

   /* Bundle-wide ALU configuration */
   int padding;
   int control;
   bool has_embedded_constants;
   midgard_constants constants;
   bool last_writeout;
} midgard_bundle;

enum midgard_rt_id {
   MIDGARD_COLOR_RT0 = 0,
   MIDGARD_COLOR_RT1,
   MIDGARD_COLOR_RT2,
   MIDGARD_COLOR_RT3,
   MIDGARD_COLOR_RT4,
   MIDGARD_COLOR_RT5,
   MIDGARD_COLOR_RT6,
   MIDGARD_COLOR_RT7,
   MIDGARD_ZS_RT,
   MIDGARD_NUM_RTS,
};

#define MIDGARD_MAX_SAMPLE_ITER 16

typedef struct compiler_context {
   const struct panfrost_compile_inputs *inputs;
   nir_shader *nir;
   struct pan_shader_info *info;
   gl_shader_stage stage;

   /* Index to precolour to r0 for an input blend colour */
   unsigned blend_input;

   /* Index to precolour to r2 for a dual-source blend colour */
   unsigned blend_src1;

   /* Count of spills and fills for shaderdb */
   unsigned spills;
   unsigned fills;

   /* Current NIR function */
   nir_function *func;

   /* Allocated compiler temporary counter */
   unsigned temp_alloc;

   /* Unordered list of midgard_blocks */
   int block_count;
   struct list_head blocks;

   /* TODO merge with block_count? */
   unsigned block_source_count;

   /* List of midgard_instructions emitted for the current block */
   midgard_block *current_block;

   /* If there is a preset after block, use this, otherwise emit_block will
    * create one if NULL */
   midgard_block *after_block;

   /* The current "depth" of the loop, for disambiguating breaks/continues
    * when using nested loops */
   int current_loop_depth;

   /* Total number of loops for shader-db */
   unsigned loop_count;

   /* Constants which have been loaded, for later inlining */
   struct hash_table_u64 *ssa_constants;

   int temp_count;
   int max_hash;

   /* Count of instructions emitted from NIR overall, across all blocks */
   int instruction_count;

   unsigned quadword_count;

   /* Bitmask of valid metadata */
   unsigned metadata;

   /* Model-specific quirk set */
   uint32_t quirks;

   /* Writeout instructions for each render target */
   midgard_instruction
      *writeout_branch[MIDGARD_NUM_RTS][MIDGARD_MAX_SAMPLE_ITER];

   /* Mask of UBOs that need to be uploaded */
   uint32_t ubo_mask;
} compiler_context;

/* Per-block live_in/live_out */
#define MIDGARD_METADATA_LIVENESS (1 << 0)

/* Helpers for manipulating the above structures (forming the driver IR) */

/* Append instruction to end of current block */

static inline midgard_instruction *
mir_upload_ins(struct compiler_context *ctx, struct midgard_instruction ins)
{
   midgard_instruction *heap = ralloc(ctx, struct midgard_instruction);
   memcpy(heap, &ins, sizeof(ins));
   return heap;
}

static inline midgard_instruction *
emit_mir_instruction(struct compiler_context *ctx,
                     struct midgard_instruction ins)
{
   midgard_instruction *u = mir_upload_ins(ctx, ins);
   list_addtail(&u->link, &ctx->current_block->base.instructions);
   return u;
}

static inline struct midgard_instruction *
mir_insert_instruction_before(struct compiler_context *ctx,
                              struct midgard_instruction *tag,
                              struct midgard_instruction ins)
{
   struct midgard_instruction *u = mir_upload_ins(ctx, ins);
   list_addtail(&u->link, &tag->link);
   return u;
}

static inline void
mir_remove_instruction(struct midgard_instruction *ins)
{
   list_del(&ins->link);
}

static inline midgard_instruction *
mir_prev_op(struct midgard_instruction *ins)
{
   return list_last_entry(&(ins->link), midgard_instruction, link);
}

static inline midgard_instruction *
mir_next_op(struct midgard_instruction *ins)
{
   return list_first_entry(&(ins->link), midgard_instruction, link);
}

#define mir_foreach_block(ctx, v)                                              \
   list_for_each_entry(pan_block, v, &ctx->blocks, link)

#define mir_foreach_block_from(ctx, from, v)                                   \
   list_for_each_entry_from(pan_block, v, &from->base, &ctx->blocks, link)

#define mir_foreach_instr_in_block(block, v)                                   \
   list_for_each_entry(struct midgard_instruction, v,                          \
                       &block->base.instructions, link)
#define mir_foreach_instr_in_block_rev(block, v)                               \
   list_for_each_entry_rev(struct midgard_instruction, v,                      \
                           &block->base.instructions, link)

#define mir_foreach_instr_in_block_safe(block, v)                              \
   list_for_each_entry_safe(struct midgard_instruction, v,                     \
                            &block->base.instructions, link)

#define mir_foreach_instr_in_block_safe_rev(block, v)                          \
   list_for_each_entry_safe_rev(struct midgard_instruction, v,                 \
                                &block->base.instructions, link)

#define mir_foreach_instr_in_block_from(block, v, from)                        \
   list_for_each_entry_from(struct midgard_instruction, v, from,               \
                            &block->base.instructions, link)

#define mir_foreach_instr_in_block_from_rev(block, v, from)                    \
   list_for_each_entry_from_rev(struct midgard_instruction, v, from,           \
                                &block->base.instructions, link)

#define mir_foreach_bundle_in_block(block, v)                                  \
   util_dynarray_foreach(&block->bundles, midgard_bundle, v)

#define mir_foreach_bundle_in_block_rev(block, v)                              \
   util_dynarray_foreach_reverse(&block->bundles, midgard_bundle, v)

#define mir_foreach_instr_in_block_scheduled_rev(block, v)                     \
   midgard_instruction *v;                                                     \
   signed i = 0;                                                               \
   mir_foreach_bundle_in_block_rev(block, _bundle)                             \
      for (i = (_bundle->instruction_count - 1), v = _bundle->instructions[i]; \
           i >= 0; --i, v = (i >= 0) ? _bundle->instructions[i] : NULL)

#define mir_foreach_instr_global(ctx, v)                                       \
   mir_foreach_block(ctx, v_block)                                             \
      mir_foreach_instr_in_block(((midgard_block *)v_block), v)

#define mir_foreach_instr_global_safe(ctx, v)                                  \
   mir_foreach_block(ctx, v_block)                                             \
      mir_foreach_instr_in_block_safe(((midgard_block *)v_block), v)

/* Based on set_foreach, expanded with automatic type casts */

#define mir_foreach_predecessor(blk, v)                                        \
   struct set_entry *_entry_##v;                                               \
   struct midgard_block *v;                                                    \
   for (_entry_##v = _mesa_set_next_entry(blk->base.predecessors, NULL),       \
       v = (struct midgard_block *)(_entry_##v ? _entry_##v->key : NULL);      \
        _entry_##v != NULL;                                                    \
        _entry_##v = _mesa_set_next_entry(blk->base.predecessors, _entry_##v), \
       v = (struct midgard_block *)(_entry_##v ? _entry_##v->key : NULL))

#define mir_foreach_src(ins, v)                                                \
   for (unsigned v = 0; v < ARRAY_SIZE(ins->src); ++v)

static inline midgard_instruction *
mir_last_in_block(struct midgard_block *block)
{
   return list_last_entry(&block->base.instructions, struct midgard_instruction,
                          link);
}

static inline midgard_block *
mir_get_block(compiler_context *ctx, int idx)
{
   struct list_head *lst = &ctx->blocks;

   while ((idx--) + 1)
      lst = lst->next;

   return (struct midgard_block *)lst;
}

static inline bool
mir_is_alu_bundle(midgard_bundle *bundle)
{
   return IS_ALU(bundle->tag);
}

static inline unsigned
make_compiler_temp(compiler_context *ctx)
{
   return (ctx->func->impl->ssa_alloc + ctx->temp_alloc++) << 1;
}

static inline unsigned
make_compiler_temp_reg(compiler_context *ctx)
{
   return ((ctx->func->impl->ssa_alloc + ctx->temp_alloc++) << 1) | PAN_IS_REG;
}

static inline bool
mir_is_ssa(unsigned index)
{
   return (index < SSA_FIXED_MINIMUM) && !(index & PAN_IS_REG);
}

static inline unsigned
nir_ssa_index(nir_def *ssa)
{
   return (ssa->index << 1) | 0;
}

static inline unsigned
nir_reg_index(nir_def *handle)
{
   return (handle->index << 1) | PAN_IS_REG;
}

static inline unsigned
nir_src_index(compiler_context *ctx, nir_src *src)
{
   nir_intrinsic_instr *load = nir_load_reg_for_def(src->ssa);

   if (load)
      return nir_reg_index(load->src[0].ssa);
   else
      return nir_ssa_index(src->ssa);
}

static inline unsigned
nir_def_index_with_mask(nir_def *def, uint16_t *write_mask)
{
   nir_intrinsic_instr *store = nir_store_reg_for_def(def);

   if (store) {
      *write_mask = nir_intrinsic_write_mask(store);
      return nir_reg_index(store->src[1].ssa);
   } else {
      *write_mask = (uint16_t)BITFIELD_MASK(def->num_components);
      return nir_ssa_index(def);
   }
}

static inline unsigned
nir_def_index(nir_def *def)
{
   uint16_t write_mask = 0;
   return nir_def_index_with_mask(def, &write_mask);
}

/* MIR manipulation */

void mir_rewrite_index(compiler_context *ctx, unsigned old, unsigned new);
void mir_rewrite_index_src(compiler_context *ctx, unsigned old, unsigned new);
void mir_rewrite_index_dst(compiler_context *ctx, unsigned old, unsigned new);
void mir_rewrite_index_dst_single(midgard_instruction *ins, unsigned old,
                                  unsigned new);
void mir_rewrite_index_src_single(midgard_instruction *ins, unsigned old,
                                  unsigned new);
void mir_rewrite_index_src_swizzle(compiler_context *ctx, unsigned old,
                                   unsigned new, unsigned *swizzle);
bool mir_single_use(compiler_context *ctx, unsigned value);
unsigned mir_use_count(compiler_context *ctx, unsigned value);
uint16_t mir_bytemask_of_read_components(midgard_instruction *ins,
                                         unsigned node);
uint16_t mir_bytemask_of_read_components_index(midgard_instruction *ins,
                                               unsigned i);
uint16_t mir_from_bytemask(uint16_t bytemask, unsigned bits);
uint16_t mir_bytemask(midgard_instruction *ins);
uint16_t mir_round_bytemask_up(uint16_t mask, unsigned bits);
void mir_set_bytemask(midgard_instruction *ins, uint16_t bytemask);
signed mir_upper_override(midgard_instruction *ins, unsigned inst_size);
unsigned mir_components_for_type(nir_alu_type T);
unsigned max_bitsize_for_alu(midgard_instruction *ins);
midgard_reg_mode reg_mode_for_bitsize(unsigned bitsize);

/* MIR printing */

void mir_print_instruction(midgard_instruction *ins);
void mir_print_bundle(midgard_bundle *ctx);
void mir_print_block(midgard_block *block);
void mir_print_shader(compiler_context *ctx);
bool mir_nontrivial_mod(midgard_instruction *ins, unsigned i,
                        bool check_swizzle);
bool mir_nontrivial_outmod(midgard_instruction *ins);

midgard_instruction *mir_insert_instruction_before_scheduled(
   compiler_context *ctx, midgard_block *block, midgard_instruction *tag,
   midgard_instruction ins);
midgard_instruction *mir_insert_instruction_after_scheduled(
   compiler_context *ctx, midgard_block *block, midgard_instruction *tag,
   midgard_instruction ins);
void mir_flip(midgard_instruction *ins);
void mir_compute_temp_count(compiler_context *ctx);

#define LDST_GLOBAL  (REGISTER_LDST_ZERO << 2)
#define LDST_SHARED  ((REGISTER_LDST_LOCAL_STORAGE_PTR << 2) | COMPONENT_Z)
#define LDST_SCRATCH ((REGISTER_LDST_PC_SP << 2) | COMPONENT_Z)

void mir_set_offset(compiler_context *ctx, midgard_instruction *ins,
                    nir_src *offset, unsigned seg);
void mir_set_ubo_offset(midgard_instruction *ins, nir_src *src, unsigned bias);

/* 'Intrinsic' move for aliasing */

static inline midgard_instruction
v_mov(unsigned src, unsigned dest)
{
   midgard_instruction ins = {
      .type = TAG_ALU_4,
      .mask = 0xF,
      .src = {~0, src, ~0, ~0},
      .src_types = {0, nir_type_uint32},
      .swizzle = SWIZZLE_IDENTITY,
      .dest = dest,
      .dest_type = nir_type_uint32,
      .op = midgard_alu_op_imov,
      .outmod = midgard_outmod_keeplo,
   };

   return ins;
}

/* Broad types of register classes so we can handle special
 * registers */

#define REG_CLASS_WORK 0
#define REG_CLASS_LDST 1
#define REG_CLASS_TEXR 3
#define REG_CLASS_TEXW 4

/* Like a move, but to thread local storage! */

static inline midgard_instruction
v_load_store_scratch(unsigned srcdest, unsigned index, bool is_store,
                     unsigned mask)
{
   /* We index by 32-bit vec4s */
   unsigned byte = (index * 4 * 4);

   midgard_instruction ins = {
      .type = TAG_LOAD_STORE_4,
      .mask = mask,
      .dest_type = nir_type_uint32,
      .dest = ~0,
      .src = {~0, ~0, ~0, ~0},
      .swizzle = SWIZZLE_IDENTITY_4,
      .op = is_store ? midgard_op_st_128 : midgard_op_ld_128,
      .load_store =
         {
            /* For register spilling - to thread local storage */
            .arg_reg = REGISTER_LDST_LOCAL_STORAGE_PTR,
            .arg_comp = COMPONENT_X,
            .bitsize_toggle = true,
            .index_format = midgard_index_address_u32,
            .index_reg = REGISTER_LDST_ZERO,
         },

      /* If we spill an unspill, RA goes into an infinite loop */
      .no_spill = (1 << REG_CLASS_WORK),
   };

   ins.constants.u32[0] = byte;

   if (is_store) {
      ins.src[0] = srcdest;
      ins.src_types[0] = nir_type_uint32;

      /* Ensure we are tightly swizzled so liveness analysis is
       * correct */

      for (unsigned i = 0; i < 4; ++i) {
         if (!(mask & (1 << i)))
            ins.swizzle[0][i] = COMPONENT_X;
      }
   } else
      ins.dest = srcdest;

   return ins;
}

static inline bool
mir_has_arg(midgard_instruction *ins, unsigned arg)
{
   if (!ins)
      return false;

   mir_foreach_src(ins, i) {
      if (ins->src[i] == arg)
         return true;
   }

   return false;
}

/* Scheduling */

void midgard_schedule_program(compiler_context *ctx);

void mir_ra(compiler_context *ctx);
void mir_squeeze_index(compiler_context *ctx);
void mir_lower_special_reads(compiler_context *ctx);
void mir_liveness_ins_update(uint16_t *live, midgard_instruction *ins,
                             unsigned max);
void mir_compute_liveness(compiler_context *ctx);
void mir_invalidate_liveness(compiler_context *ctx);
bool mir_is_live_after(compiler_context *ctx, midgard_block *block,
                       midgard_instruction *start, int src);

void mir_create_pipeline_registers(compiler_context *ctx);
void midgard_promote_uniforms(compiler_context *ctx);

void midgard_emit_derivatives(compiler_context *ctx, nir_alu_instr *instr);

void midgard_lower_derivatives(compiler_context *ctx, midgard_block *block);

bool mir_op_computes_derivatives(gl_shader_stage stage, unsigned op);

void mir_analyze_helper_terminate(compiler_context *ctx);
void mir_analyze_helper_requirements(compiler_context *ctx);

/* Final emission */

void emit_binary_bundle(compiler_context *ctx, midgard_block *block,
                        midgard_bundle *bundle, struct util_dynarray *emission,
                        int next_tag);

bool nir_fuse_io_16(nir_shader *shader);

bool midgard_nir_lod_errata(nir_shader *shader);

unsigned midgard_get_first_tag_from_block(compiler_context *ctx,
                                          unsigned block_idx);

/* Optimizations */

bool midgard_opt_copy_prop(compiler_context *ctx, midgard_block *block);
bool midgard_opt_prop(compiler_context *ctx);
bool midgard_opt_combine_projection(compiler_context *ctx,
                                    midgard_block *block);
bool midgard_opt_varying_projection(compiler_context *ctx,
                                    midgard_block *block);
bool midgard_opt_dead_code_eliminate(compiler_context *ctx);
bool midgard_opt_dead_move_eliminate(compiler_context *ctx,
                                     midgard_block *block);

#endif
