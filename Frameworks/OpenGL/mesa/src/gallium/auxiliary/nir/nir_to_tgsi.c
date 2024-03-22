/*
 * Copyright © 2014-2015 Broadcom
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

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_deref.h"
#include "compiler/nir/nir_legacy.h"
#include "compiler/nir/nir_worklist.h"
#include "nir/nir_to_tgsi.h"
#include "pipe/p_screen.h"
#include "pipe/p_state.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_from_mesa.h"
#include "tgsi/tgsi_info.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_ureg.h"
#include "tgsi/tgsi_util.h"
#include "util/u_debug.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_dynarray.h"

struct ntt_insn {
   enum tgsi_opcode opcode;
   struct ureg_dst dst[2];
   struct ureg_src src[4];
   enum tgsi_texture_type tex_target;
   enum tgsi_return_type tex_return_type;
   struct tgsi_texture_offset tex_offset[4];

   unsigned mem_qualifier;
   enum pipe_format mem_format;

   bool is_tex : 1;
   bool is_mem : 1;
   bool precise : 1;
};

struct ntt_block {
   /* Array of struct ntt_insn */
   struct util_dynarray insns;
   int start_ip;
   int end_ip;
};

struct ntt_reg_interval {
   uint32_t start, end;
};

struct ntt_compile {
   nir_shader *s;
   nir_function_impl *impl;
   const struct nir_to_tgsi_options *options;
   struct pipe_screen *screen;
   struct ureg_program *ureg;

   bool needs_texcoord_semantic;
   bool native_integers;
   bool has_txf_lz;

   bool addr_declared[3];
   struct ureg_dst addr_reg[3];

   /* if condition set up at the end of a block, for ntt_emit_if(). */
   struct ureg_src if_cond;

   /* TGSI temps for our NIR SSA and register values. */
   struct ureg_dst *reg_temp;
   struct ureg_src *ssa_temp;

   struct ntt_reg_interval *liveness;

   /* Map from nir_block to ntt_block */
   struct hash_table *blocks;
   struct ntt_block *cur_block;
   unsigned current_if_else;
   unsigned cf_label;

   /* Whether we're currently emitting instructiosn for a precise NIR instruction. */
   bool precise;

   unsigned num_temps;
   unsigned first_non_array_temp;

   /* Mappings from driver_location to TGSI input/output number.
    *
    * We'll be declaring TGSI input/outputs in an arbitrary order, and they get
    * their numbers assigned incrementally, unlike inputs or constants.
    */
   struct ureg_src *input_index_map;
   uint64_t centroid_inputs;

   uint32_t first_ubo;
   uint32_t first_ssbo;

   struct ureg_src images[PIPE_MAX_SHADER_IMAGES];
};

static struct ureg_dst
ntt_temp(struct ntt_compile *c)
{
   return ureg_dst_register(TGSI_FILE_TEMPORARY, c->num_temps++);
}

static struct ntt_block *
ntt_block_from_nir(struct ntt_compile *c, struct nir_block *block)
{
   struct hash_entry *entry = _mesa_hash_table_search(c->blocks, block);
   return entry->data;
}

static void ntt_emit_cf_list(struct ntt_compile *c, struct exec_list *list);
static void ntt_emit_cf_list_ureg(struct ntt_compile *c, struct exec_list *list);

static struct ntt_insn *
ntt_insn(struct ntt_compile *c, enum tgsi_opcode opcode,
         struct ureg_dst dst,
         struct ureg_src src0, struct ureg_src src1,
         struct ureg_src src2, struct ureg_src src3)
{
   struct ntt_insn insn = {
      .opcode = opcode,
      .dst = { dst, ureg_dst_undef() },
      .src = { src0, src1, src2, src3 },
      .precise = c->precise,
   };
   util_dynarray_append(&c->cur_block->insns, struct ntt_insn, insn);
   return util_dynarray_top_ptr(&c->cur_block->insns, struct ntt_insn);
}

#define OP00( op )                                                                     \
static inline void ntt_##op(struct ntt_compile *c)                                     \
{                                                                                      \
   ntt_insn(c, TGSI_OPCODE_##op, ureg_dst_undef(), ureg_src_undef(), ureg_src_undef(), ureg_src_undef(), ureg_src_undef()); \
}

#define OP01( op )                                                                     \
static inline void ntt_##op(struct ntt_compile *c,                                     \
                     struct ureg_src src0)                                             \
{                                                                                      \
   ntt_insn(c, TGSI_OPCODE_##op, ureg_dst_undef(), src0, ureg_src_undef(), ureg_src_undef(), ureg_src_undef()); \
}


#define OP10( op )                                                                     \
static inline void ntt_##op(struct ntt_compile *c,                                     \
                     struct ureg_dst dst)                                              \
{                                                                                      \
   ntt_insn(c, TGSI_OPCODE_##op, dst, ureg_src_undef(), ureg_src_undef(), ureg_src_undef(), ureg_src_undef()); \
}

#define OP11( op )                                                                     \
static inline void ntt_##op(struct ntt_compile *c,                                     \
                     struct ureg_dst dst,                                              \
                     struct ureg_src src0)                                             \
{                                                                                      \
   ntt_insn(c, TGSI_OPCODE_##op, dst, src0, ureg_src_undef(), ureg_src_undef(), ureg_src_undef()); \
}

#define OP12( op )                                                                     \
static inline void ntt_##op(struct ntt_compile *c,                                     \
                     struct ureg_dst dst,                                              \
                     struct ureg_src src0,                                             \
                     struct ureg_src src1)                                             \
{                                                                                      \
   ntt_insn(c, TGSI_OPCODE_##op, dst, src0, src1, ureg_src_undef(), ureg_src_undef()); \
}

#define OP13( op )                                                                     \
static inline void ntt_##op(struct ntt_compile *c,                                     \
                     struct ureg_dst dst,                                              \
                     struct ureg_src src0,                                             \
                     struct ureg_src src1,                                             \
                     struct ureg_src src2)                                             \
{                                                                                      \
   ntt_insn(c, TGSI_OPCODE_##op, dst, src0, src1, src2, ureg_src_undef());             \
}

#define OP14( op )                                                                     \
static inline void ntt_##op(struct ntt_compile *c,                                     \
                     struct ureg_dst dst,                                              \
                     struct ureg_src src0,                                             \
                     struct ureg_src src1,                                             \
                     struct ureg_src src2,                                             \
                     struct ureg_src src3)                                             \
{                                                                                      \
   ntt_insn(c, TGSI_OPCODE_##op, dst, src0, src1, src2, src3);                         \
}

/* We hand-craft our tex instructions */
#define OP12_TEX(op)
#define OP14_TEX(op)

/* Use a template include to generate a correctly-typed ntt_OP()
 * function for each TGSI opcode:
 */
#include "gallium/auxiliary/tgsi/tgsi_opcode_tmp.h"

/**
 * Interprets a nir_load_const used as a NIR src as a uint.
 *
 * For non-native-integers drivers, nir_load_const_instrs used by an integer ALU
 * instruction (or in a phi-web used by an integer ALU instruction) were
 * converted to floats and the ALU instruction swapped to the float equivalent.
 * However, this means that integer load_consts used by intrinsics (which don't
 * normally get that conversion) may have been reformatted to be floats.  Given
 * that all of our intrinsic nir_src_as_uint() calls are expected to be small,
 * we can just look and see if they look like floats and convert them back to
 * ints.
 */
static uint32_t
ntt_src_as_uint(struct ntt_compile *c, nir_src src)
{
   uint32_t val = nir_src_as_uint(src);
   if (!c->native_integers && val >= fui(1.0))
      val = (uint32_t)uif(val);
   return val;
}

static unsigned
ntt_64bit_write_mask(unsigned write_mask)
{
   return ((write_mask & 1) ? 0x3 : 0) | ((write_mask & 2) ? 0xc : 0);
}

static struct ureg_src
ntt_64bit_1f(struct ntt_compile *c)
{
   return ureg_imm4u(c->ureg,
                     0x00000000, 0x3ff00000,
                     0x00000000, 0x3ff00000);
}

/* Per-channel masks of def/use within the block, and the per-channel
 * livein/liveout for the block as a whole.
 */
struct ntt_live_reg_block_state {
   uint8_t *def, *use, *livein, *liveout, *defin, *defout;
};

struct ntt_live_reg_state {
   unsigned bitset_words;

   struct ntt_reg_interval *regs;

   /* Used in propagate_across_edge() */
   BITSET_WORD *tmp_live;

   struct ntt_live_reg_block_state *blocks;

   nir_block_worklist worklist;
};

static void
ntt_live_reg_mark_use(struct ntt_compile *c, struct ntt_live_reg_block_state *bs,
                      int ip, unsigned index, unsigned used_mask)
{
   bs->use[index] |= used_mask & ~bs->def[index];

   c->liveness[index].start = MIN2(c->liveness[index].start, ip);
   c->liveness[index].end = MAX2(c->liveness[index].end, ip);

}
static void
ntt_live_reg_setup_def_use(struct ntt_compile *c, nir_function_impl *impl, struct ntt_live_reg_state *state)
{
   for (int i = 0; i < impl->num_blocks; i++) {
      state->blocks[i].def = rzalloc_array(state->blocks, uint8_t, c->num_temps);
      state->blocks[i].defin = rzalloc_array(state->blocks, uint8_t, c->num_temps);
      state->blocks[i].defout = rzalloc_array(state->blocks, uint8_t, c->num_temps);
      state->blocks[i].use = rzalloc_array(state->blocks, uint8_t, c->num_temps);
      state->blocks[i].livein = rzalloc_array(state->blocks, uint8_t, c->num_temps);
      state->blocks[i].liveout = rzalloc_array(state->blocks, uint8_t, c->num_temps);
   }

   int ip = 0;
   nir_foreach_block(block, impl) {
      struct ntt_live_reg_block_state *bs = &state->blocks[block->index];
      struct ntt_block *ntt_block = ntt_block_from_nir(c, block);

      ntt_block->start_ip = ip;

      util_dynarray_foreach(&ntt_block->insns, struct ntt_insn, insn) {
         const struct tgsi_opcode_info *opcode_info =
            tgsi_get_opcode_info(insn->opcode);

         /* Set up use[] for the srcs.
          *
          * Uses are the channels of the reg read in the block that don't have a
          * preceding def to screen them off.  Note that we don't do per-element
          * tracking of array regs, so they're never screened off.
          */
         for (int i = 0; i < opcode_info->num_src; i++) {
            if (insn->src[i].File != TGSI_FILE_TEMPORARY)
               continue;
            int index = insn->src[i].Index;

            uint32_t used_mask = tgsi_util_get_src_usage_mask(insn->opcode, i,
                                                              insn->dst->WriteMask,
                                                              insn->src[i].SwizzleX,
                                                              insn->src[i].SwizzleY,
                                                              insn->src[i].SwizzleZ,
                                                              insn->src[i].SwizzleW,
                                                              insn->tex_target,
                                                              insn->tex_target);

            assert(!insn->src[i].Indirect || index < c->first_non_array_temp);
            ntt_live_reg_mark_use(c, bs, ip, index, used_mask);
         }

         if (insn->is_tex) {
            for (int i = 0; i < ARRAY_SIZE(insn->tex_offset); i++) {
               if (insn->tex_offset[i].File == TGSI_FILE_TEMPORARY)
                  ntt_live_reg_mark_use(c, bs, ip, insn->tex_offset[i].Index, 0xf);
            }
         }

         /* Set up def[] for the srcs.
          *
          * Defs are the unconditionally-written (not R/M/W) channels of the reg in
          * the block that don't have a preceding use.
          */
         for (int i = 0; i < opcode_info->num_dst; i++) {
            if (insn->dst[i].File != TGSI_FILE_TEMPORARY)
               continue;
            int index = insn->dst[i].Index;
            uint32_t writemask = insn->dst[i].WriteMask;

            bs->def[index] |= writemask & ~bs->use[index];
            bs->defout[index] |= writemask;

            assert(!insn->dst[i].Indirect || index < c->first_non_array_temp);
            c->liveness[index].start = MIN2(c->liveness[index].start, ip);
            c->liveness[index].end = MAX2(c->liveness[index].end, ip);
         }
         ip++;
      }

      ntt_block->end_ip = ip;
   }
}

static void
ntt_live_regs(struct ntt_compile *c, nir_function_impl *impl)
{
   nir_metadata_require(impl, nir_metadata_block_index);

   c->liveness = rzalloc_array(c, struct ntt_reg_interval, c->num_temps);

   struct ntt_live_reg_state state = {
       .blocks = rzalloc_array(impl, struct ntt_live_reg_block_state, impl->num_blocks),
   };

   /* The intervals start out with start > end (indicating unused) */
   for (int i = 0; i < c->num_temps; i++)
      c->liveness[i].start = ~0;

   ntt_live_reg_setup_def_use(c, impl, &state);

   /* Make a forward-order worklist of all the blocks. */
   nir_block_worklist_init(&state.worklist, impl->num_blocks, NULL);
   nir_foreach_block(block, impl) {
      nir_block_worklist_push_tail(&state.worklist, block);
   }

   /* Propagate defin/defout down the CFG to calculate the live variables
    * potentially defined along any possible control flow path.  We'll use this
    * to keep things like conditional defs of the reg (or array regs where we
    * don't track defs!) from making the reg's live range extend back to the
    * start of the program.
    */
   while (!nir_block_worklist_is_empty(&state.worklist)) {
      nir_block *block = nir_block_worklist_pop_head(&state.worklist);
      for (int j = 0; j < ARRAY_SIZE(block->successors); j++) {
         nir_block *succ = block->successors[j];
         if (!succ || succ->index == impl->num_blocks)
            continue;

         for (int i = 0; i < c->num_temps; i++) {
            uint8_t new_def = state.blocks[block->index].defout[i] & ~state.blocks[succ->index].defin[i];

            if (new_def) {
               state.blocks[succ->index].defin[i] |= new_def;
               state.blocks[succ->index].defout[i] |= new_def;
               nir_block_worklist_push_tail(&state.worklist, succ);
            }
         }
      }
   }

   /* Make a reverse-order worklist of all the blocks. */
   nir_foreach_block(block, impl) {
      nir_block_worklist_push_head(&state.worklist, block);
   }

   /* We're now ready to work through the worklist and update the liveness sets
    * of each of the blocks.  As long as we keep the worklist up-to-date as we
    * go, everything will get covered.
    */
   while (!nir_block_worklist_is_empty(&state.worklist)) {
      /* We pop them off in the reverse order we pushed them on.  This way
       * the first walk of the instructions is backwards so we only walk
       * once in the case of no control flow.
       */
      nir_block *block = nir_block_worklist_pop_head(&state.worklist);
      struct ntt_block *ntt_block = ntt_block_from_nir(c, block);
      struct ntt_live_reg_block_state *bs = &state.blocks[block->index];

      for (int i = 0; i < c->num_temps; i++) {
         /* Collect livein from our successors to include in our liveout. */
         for (int j = 0; j < ARRAY_SIZE(block->successors); j++) {
            nir_block *succ = block->successors[j];
            if (!succ || succ->index == impl->num_blocks)
               continue;
            struct ntt_live_reg_block_state *sbs = &state.blocks[succ->index];

            uint8_t new_liveout = sbs->livein[i] & ~bs->liveout[i];
            if (new_liveout) {
               if (state.blocks[block->index].defout[i])
                  c->liveness[i].end = MAX2(c->liveness[i].end, ntt_block->end_ip);
               bs->liveout[i] |= sbs->livein[i];
            }
         }

         /* Propagate use requests from either our block's uses or our
          * non-screened-off liveout up to our predecessors.
          */
         uint8_t new_livein = ((bs->use[i] | (bs->liveout[i] & ~bs->def[i])) &
                               ~bs->livein[i]);
         if (new_livein) {
            bs->livein[i] |= new_livein;
            set_foreach(block->predecessors, entry) {
               nir_block *pred = (void *)entry->key;
               nir_block_worklist_push_tail(&state.worklist, pred);
            }

            if (new_livein & state.blocks[block->index].defin[i])
               c->liveness[i].start = MIN2(c->liveness[i].start, ntt_block->start_ip);
         }
      }
   }

   ralloc_free(state.blocks);
   nir_block_worklist_fini(&state.worklist);
}

static void
ntt_ra_check(struct ntt_compile *c, unsigned *ra_map, BITSET_WORD *released, int ip, unsigned index)
{
   if (index < c->first_non_array_temp)
      return;

   if (c->liveness[index].start == ip && ra_map[index] == ~0)
      ra_map[index] = ureg_DECL_temporary(c->ureg).Index;

   if (c->liveness[index].end == ip && !BITSET_TEST(released, index)) {
      ureg_release_temporary(c->ureg, ureg_dst_register(TGSI_FILE_TEMPORARY, ra_map[index]));
      BITSET_SET(released, index);
   }
}

static void
ntt_allocate_regs(struct ntt_compile *c, nir_function_impl *impl)
{
   ntt_live_regs(c, impl);

   unsigned *ra_map = ralloc_array(c, unsigned, c->num_temps);
   unsigned *released = rzalloc_array(c, BITSET_WORD, BITSET_WORDS(c->num_temps));

   /* No RA on NIR array regs */
   for (int i = 0; i < c->first_non_array_temp; i++)
      ra_map[i] = i;

   for (int i = c->first_non_array_temp; i < c->num_temps; i++)
      ra_map[i] = ~0;

   int ip = 0;
   nir_foreach_block(block, impl) {
      struct ntt_block *ntt_block = ntt_block_from_nir(c, block);

      for (int i = 0; i < c->num_temps; i++)
         ntt_ra_check(c, ra_map, released, ip, i);

      util_dynarray_foreach(&ntt_block->insns, struct ntt_insn, insn) {
         const struct tgsi_opcode_info *opcode_info =
            tgsi_get_opcode_info(insn->opcode);

         for (int i = 0; i < opcode_info->num_src; i++) {
            if (insn->src[i].File == TGSI_FILE_TEMPORARY) {
               ntt_ra_check(c, ra_map, released, ip, insn->src[i].Index);
               insn->src[i].Index = ra_map[insn->src[i].Index];
            }
         }

         if (insn->is_tex) {
            for (int i = 0; i < ARRAY_SIZE(insn->tex_offset); i++) {
               if (insn->tex_offset[i].File == TGSI_FILE_TEMPORARY) {
                  ntt_ra_check(c, ra_map, released, ip, insn->tex_offset[i].Index);
                  insn->tex_offset[i].Index = ra_map[insn->tex_offset[i].Index];
               }
            }
         }

         for (int i = 0; i < opcode_info->num_dst; i++) {
            if (insn->dst[i].File == TGSI_FILE_TEMPORARY) {
               ntt_ra_check(c, ra_map, released, ip, insn->dst[i].Index);
               insn->dst[i].Index = ra_map[insn->dst[i].Index];
            }
         }
         ip++;
      }

      for (int i = 0; i < c->num_temps; i++)
         ntt_ra_check(c, ra_map, released, ip, i);
   }
}

static void
ntt_allocate_regs_unoptimized(struct ntt_compile *c, nir_function_impl *impl)
{
   for (int i = c->first_non_array_temp; i < c->num_temps; i++)
      ureg_DECL_temporary(c->ureg);
}


/**
 * Try to find an iadd of a constant value with a non-constant value in the
 * nir_src's first component, returning the constant offset and replacing *src
 * with the non-constant component.
 */
static const uint32_t
ntt_extract_const_src_offset(nir_src *src)
{
   nir_scalar s = nir_get_scalar(src->ssa, 0);

   while (nir_scalar_is_alu(s)) {
      nir_alu_instr *alu = nir_instr_as_alu(s.def->parent_instr);

      if (alu->op == nir_op_iadd) {
         for (int i = 0; i < 2; i++) {
            nir_const_value *v = nir_src_as_const_value(alu->src[i].src);
            if (v != NULL) {
               *src = alu->src[1 - i].src;
               return v[alu->src[i].swizzle[s.comp]].u32;
            }
         }

         return 0;
      }

      /* We'd like to reuse nir_scalar_chase_movs(), but it assumes SSA and that
       * seems reasonable for something used in inner loops of the compiler.
       */
      if (alu->op == nir_op_mov) {
         s.def = alu->src[0].src.ssa;
         s.comp = alu->src[0].swizzle[s.comp];
      } else if (nir_op_is_vec(alu->op)) {
         s.def = alu->src[s.comp].src.ssa;
         s.comp = alu->src[s.comp].swizzle[0];
      } else {
         return 0;
      }
   }

   return 0;
}

static const struct glsl_type *
ntt_shader_input_type(struct ntt_compile *c,
                      struct nir_variable *var)
{
   switch (c->s->info.stage) {
   case MESA_SHADER_GEOMETRY:
   case MESA_SHADER_TESS_EVAL:
   case MESA_SHADER_TESS_CTRL:
      if (glsl_type_is_array(var->type))
         return glsl_get_array_element(var->type);
      else
         return var->type;
   default:
      return var->type;
   }
}

static void
ntt_get_gl_varying_semantic(struct ntt_compile *c, unsigned location,
                            unsigned *semantic_name, unsigned *semantic_index)
{
   /* We want to use most of tgsi_get_gl_varying_semantic(), but the
    * !texcoord shifting has already been applied, so avoid that.
    */
   if (!c->needs_texcoord_semantic &&
       (location >= VARYING_SLOT_VAR0 && location < VARYING_SLOT_PATCH0)) {
      *semantic_name = TGSI_SEMANTIC_GENERIC;
      *semantic_index = location - VARYING_SLOT_VAR0;
      return;
   }

   tgsi_get_gl_varying_semantic(location, true,
                                semantic_name, semantic_index);
}

/* TGSI varying declarations have a component usage mask associated (used by
 * r600 and svga).
 */
static uint32_t
ntt_tgsi_usage_mask(unsigned start_component, unsigned num_components,
                    bool is_64)
{
   uint32_t usage_mask =
      u_bit_consecutive(start_component, num_components);

   if (is_64) {
      if (start_component >= 2)
         usage_mask >>= 2;

      uint32_t tgsi_usage_mask = 0;

      if (usage_mask & TGSI_WRITEMASK_X)
         tgsi_usage_mask |= TGSI_WRITEMASK_XY;
      if (usage_mask & TGSI_WRITEMASK_Y)
         tgsi_usage_mask |= TGSI_WRITEMASK_ZW;

      return tgsi_usage_mask;
   } else {
      return usage_mask;
   }
}

/* TGSI varying declarations have a component usage mask associated (used by
 * r600 and svga).
 */
static uint32_t
ntt_tgsi_var_usage_mask(const struct nir_variable *var)
{
   const struct glsl_type *type_without_array =
      glsl_without_array(var->type);
   unsigned num_components = glsl_get_vector_elements(type_without_array);
   if (num_components == 0) /* structs */
      num_components = 4;

   return ntt_tgsi_usage_mask(var->data.location_frac, num_components,
                              glsl_type_is_64bit(type_without_array));
}

static struct ureg_dst
ntt_output_decl(struct ntt_compile *c, nir_intrinsic_instr *instr, uint32_t *frac)
{
   nir_io_semantics semantics = nir_intrinsic_io_semantics(instr);
   int base = nir_intrinsic_base(instr);
   *frac = nir_intrinsic_component(instr);
   bool is_64 = nir_src_bit_size(instr->src[0]) == 64;

   struct ureg_dst out;
   if (c->s->info.stage == MESA_SHADER_FRAGMENT) {
      unsigned semantic_name, semantic_index;
      tgsi_get_gl_frag_result_semantic(semantics.location,
                                       &semantic_name, &semantic_index);
      semantic_index += semantics.dual_source_blend_index;

      switch (semantics.location) {
      case FRAG_RESULT_DEPTH:
         *frac = 2; /* z write is the to the .z channel in TGSI */
         break;
      case FRAG_RESULT_STENCIL:
         *frac = 1;
         break;
      default:
         break;
      }

      out = ureg_DECL_output(c->ureg, semantic_name, semantic_index);
   } else {
      unsigned semantic_name, semantic_index;

      ntt_get_gl_varying_semantic(c, semantics.location,
                                  &semantic_name, &semantic_index);

      uint32_t usage_mask = ntt_tgsi_usage_mask(*frac,
                                                instr->num_components,
                                                is_64);
      uint32_t gs_streams = semantics.gs_streams;
      for (int i = 0; i < 4; i++) {
         if (!(usage_mask & (1 << i)))
            gs_streams &= ~(0x3 << 2 * i);
      }

      /* No driver appears to use array_id of outputs. */
      unsigned array_id = 0;

      /* This bit is lost in the i/o semantics, but it's unused in in-tree
       * drivers.
       */
      bool invariant = semantics.invariant;

      unsigned num_slots = semantics.num_slots;
      if (semantics.location == VARYING_SLOT_TESS_LEVEL_INNER ||
          semantics.location == VARYING_SLOT_TESS_LEVEL_OUTER) {
         /* Compact vars get a num_slots in NIR as number of components, but we
          * want the number of vec4 slots here.
          */
         num_slots = 1;
      }

      out = ureg_DECL_output_layout(c->ureg,
                                    semantic_name, semantic_index,
                                    gs_streams,
                                    base,
                                    usage_mask,
                                    array_id,
                                    num_slots,
                                    invariant);
   }

   unsigned write_mask;
   if (nir_intrinsic_has_write_mask(instr))
      write_mask = nir_intrinsic_write_mask(instr);
   else
      write_mask = ((1 << instr->num_components) - 1) << *frac;

   if (is_64) {
      write_mask = ntt_64bit_write_mask(write_mask);
      if (*frac >= 2)
         write_mask = write_mask << 2;
   } else {
      write_mask = write_mask << *frac;
   }
   return ureg_writemask(out, write_mask);
}

static bool
ntt_try_store_in_tgsi_output_with_use(struct ntt_compile *c,
                                      struct ureg_dst *dst,
                                      nir_src *src)
{
   *dst = ureg_dst_undef();

   switch (c->s->info.stage) {
   case MESA_SHADER_FRAGMENT:
   case MESA_SHADER_VERTEX:
      break;
   default:
      /* tgsi_exec (at least) requires that output stores happen per vertex
       * emitted, you don't get to reuse a previous output value for the next
       * vertex.
       */
      return false;
   }

   if (nir_src_is_if(src))
      return false;

   if (nir_src_parent_instr(src)->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(nir_src_parent_instr(src));
   if (intr->intrinsic != nir_intrinsic_store_output ||
       !nir_src_is_const(intr->src[1])) {
      return false;
   }

   uint32_t frac;
   *dst = ntt_output_decl(c, intr, &frac);
   dst->Index += ntt_src_as_uint(c, intr->src[1]);

   return frac == 0;
}

/* If this reg is used only for storing an output, then in the simple
 * cases we can write directly to the TGSI output instead of having
 * store_output emit its own MOV.
 */
static bool
ntt_try_store_reg_in_tgsi_output(struct ntt_compile *c, struct ureg_dst *dst,
                                 nir_intrinsic_instr *reg_decl)
{
   assert(reg_decl->intrinsic == nir_intrinsic_decl_reg);

   *dst = ureg_dst_undef();

   /* Look for a single use for try_store_in_tgsi_output */
   nir_src *use = NULL;
   nir_foreach_reg_load(src, reg_decl) {
      nir_intrinsic_instr *load = nir_instr_as_intrinsic(nir_src_parent_instr(src));
      nir_foreach_use_including_if(load_use, &load->def) {
         /* We can only have one use */
         if (use != NULL)
            return false;

         use = load_use;
      }
   }

   if (use == NULL)
      return false;

   return ntt_try_store_in_tgsi_output_with_use(c, dst, use);
}

/* If this SSA def is used only for storing an output, then in the simple
 * cases we can write directly to the TGSI output instead of having
 * store_output emit its own MOV.
 */
static bool
ntt_try_store_ssa_in_tgsi_output(struct ntt_compile *c, struct ureg_dst *dst,
                                 nir_def *def)
{
   *dst = ureg_dst_undef();

   if (!list_is_singular(&def->uses))
      return false;

   nir_foreach_use_including_if(use, def) {
      return ntt_try_store_in_tgsi_output_with_use(c, dst, use);
   }
   unreachable("We have one use");
}

static void
ntt_setup_inputs(struct ntt_compile *c)
{
   if (c->s->info.stage != MESA_SHADER_FRAGMENT)
      return;

   unsigned num_inputs = 0;
   int num_input_arrays = 0;

   nir_foreach_shader_in_variable(var, c->s) {
      const struct glsl_type *type = ntt_shader_input_type(c, var);
      unsigned array_len =
         glsl_count_attribute_slots(type, false);

      num_inputs = MAX2(num_inputs, var->data.driver_location + array_len);
   }

   c->input_index_map = ralloc_array(c, struct ureg_src, num_inputs);

   nir_foreach_shader_in_variable(var, c->s) {
      const struct glsl_type *type = ntt_shader_input_type(c, var);
      unsigned array_len =
         glsl_count_attribute_slots(type, false);

      unsigned interpolation = TGSI_INTERPOLATE_CONSTANT;
      unsigned sample_loc;
      struct ureg_src decl;

      if (c->s->info.stage == MESA_SHADER_FRAGMENT) {
         interpolation =
            tgsi_get_interp_mode(var->data.interpolation,
                                 var->data.location == VARYING_SLOT_COL0 ||
                                 var->data.location == VARYING_SLOT_COL1);

         if (var->data.location == VARYING_SLOT_POS)
            interpolation = TGSI_INTERPOLATE_LINEAR;
      }

      unsigned semantic_name, semantic_index;
      ntt_get_gl_varying_semantic(c, var->data.location,
                                  &semantic_name, &semantic_index);

      if (var->data.sample) {
         sample_loc = TGSI_INTERPOLATE_LOC_SAMPLE;
      } else if (var->data.centroid) {
         sample_loc = TGSI_INTERPOLATE_LOC_CENTROID;
         c->centroid_inputs |= (BITSET_MASK(array_len) <<
                                var->data.driver_location);
      } else {
         sample_loc = TGSI_INTERPOLATE_LOC_CENTER;
      }

      unsigned array_id = 0;
      if (glsl_type_is_array(type))
         array_id = ++num_input_arrays;

      uint32_t usage_mask = ntt_tgsi_var_usage_mask(var);

      decl = ureg_DECL_fs_input_centroid_layout(c->ureg,
                                                semantic_name,
                                                semantic_index,
                                                interpolation,
                                                sample_loc,
                                                var->data.driver_location,
                                                usage_mask,
                                                array_id, array_len);

      if (semantic_name == TGSI_SEMANTIC_FACE) {
         struct ureg_dst temp = ntt_temp(c);
         if (c->native_integers) {
            /* NIR is ~0 front and 0 back, while TGSI is +1 front */
            ntt_SGE(c, temp, decl, ureg_imm1f(c->ureg, 0));
         } else {
            /* tgsi docs say that floating point FACE will be positive for
             * frontface and negative for backface, but realistically
             * GLSL-to-TGSI had been doing MOV_SAT to turn it into 0.0 vs 1.0.
             * Copy that behavior, since some drivers (r300) have been doing a
             * 0.0 vs 1.0 backface (and I don't think anybody has a non-1.0
             * front face).
             */
            temp.Saturate = true;
            ntt_MOV(c, temp, decl);

         }
         decl = ureg_src(temp);
      }

      for (unsigned i = 0; i < array_len; i++) {
         c->input_index_map[var->data.driver_location + i] = decl;
         c->input_index_map[var->data.driver_location + i].Index += i;
      }
   }
}

static int
ntt_sort_by_location(const nir_variable *a, const nir_variable *b)
{
   return a->data.location - b->data.location;
}

/**
 * Workaround for virglrenderer requiring that TGSI FS output color variables
 * are declared in order.  Besides, it's a lot nicer to read the TGSI this way.
 */
static void
ntt_setup_outputs(struct ntt_compile *c)
{
   if (c->s->info.stage != MESA_SHADER_FRAGMENT)
      return;

   nir_sort_variables_with_modes(c->s, ntt_sort_by_location, nir_var_shader_out);

   nir_foreach_shader_out_variable(var, c->s) {
      if (var->data.location == FRAG_RESULT_COLOR)
         ureg_property(c->ureg, TGSI_PROPERTY_FS_COLOR0_WRITES_ALL_CBUFS, 1);

      unsigned semantic_name, semantic_index;
      tgsi_get_gl_frag_result_semantic(var->data.location,
                                       &semantic_name, &semantic_index);

      (void)ureg_DECL_output(c->ureg, semantic_name, semantic_index);
   }
}

static enum tgsi_texture_type
tgsi_texture_type_from_sampler_dim(enum glsl_sampler_dim dim, bool is_array, bool is_shadow)
{
   switch (dim) {
   case GLSL_SAMPLER_DIM_1D:
      if (is_shadow)
         return is_array ? TGSI_TEXTURE_SHADOW1D_ARRAY : TGSI_TEXTURE_SHADOW1D;
      else
         return is_array ? TGSI_TEXTURE_1D_ARRAY : TGSI_TEXTURE_1D;
   case GLSL_SAMPLER_DIM_2D:
   case GLSL_SAMPLER_DIM_EXTERNAL:
      if (is_shadow)
         return is_array ? TGSI_TEXTURE_SHADOW2D_ARRAY : TGSI_TEXTURE_SHADOW2D;
      else
         return is_array ? TGSI_TEXTURE_2D_ARRAY : TGSI_TEXTURE_2D;
   case GLSL_SAMPLER_DIM_3D:
      return TGSI_TEXTURE_3D;
   case GLSL_SAMPLER_DIM_CUBE:
      if (is_shadow)
         return is_array ? TGSI_TEXTURE_SHADOWCUBE_ARRAY : TGSI_TEXTURE_SHADOWCUBE;
      else
         return is_array ? TGSI_TEXTURE_CUBE_ARRAY : TGSI_TEXTURE_CUBE;
   case GLSL_SAMPLER_DIM_RECT:
      if (is_shadow)
         return TGSI_TEXTURE_SHADOWRECT;
      else
         return TGSI_TEXTURE_RECT;
   case GLSL_SAMPLER_DIM_MS:
      return is_array ? TGSI_TEXTURE_2D_ARRAY_MSAA : TGSI_TEXTURE_2D_MSAA;
   case GLSL_SAMPLER_DIM_BUF:
      return TGSI_TEXTURE_BUFFER;
   default:
      unreachable("unknown sampler dim");
   }
}

static enum tgsi_return_type
tgsi_return_type_from_base_type(enum glsl_base_type type)
{
   switch (type) {
   case GLSL_TYPE_INT:
      return TGSI_RETURN_TYPE_SINT;
   case GLSL_TYPE_UINT:
      return TGSI_RETURN_TYPE_UINT;
   case GLSL_TYPE_FLOAT:
     return TGSI_RETURN_TYPE_FLOAT;
   default:
      unreachable("unexpected texture type");
   }
}

static void
ntt_setup_uniforms(struct ntt_compile *c)
{
   nir_foreach_uniform_variable(var, c->s) {
      if (glsl_type_is_sampler(glsl_without_array(var->type)) ||
          glsl_type_is_texture(glsl_without_array(var->type))) {
         /* Don't use this size for the check for samplers -- arrays of structs
          * containing samplers should be ignored, and just the separate lowered
          * sampler uniform decl used.
          */
         int size = glsl_type_get_sampler_count(var->type) +
                    glsl_type_get_texture_count(var->type);

         const struct glsl_type *stype = glsl_without_array(var->type);
         enum tgsi_texture_type target = tgsi_texture_type_from_sampler_dim(glsl_get_sampler_dim(stype),
                                                                            glsl_sampler_type_is_array(stype),
                                                                            glsl_sampler_type_is_shadow(stype));
         enum tgsi_return_type ret_type = tgsi_return_type_from_base_type(glsl_get_sampler_result_type(stype));
         for (int i = 0; i < size; i++) {
            ureg_DECL_sampler_view(c->ureg, var->data.binding + i,
               target, ret_type, ret_type, ret_type, ret_type);
            ureg_DECL_sampler(c->ureg, var->data.binding + i);
         }
      } else if (glsl_contains_atomic(var->type)) {
         uint32_t offset = var->data.offset / 4;
         uint32_t size = glsl_atomic_size(var->type) / 4;
         ureg_DECL_hw_atomic(c->ureg, offset, offset + size - 1, var->data.binding, 0);
      }

      /* lower_uniforms_to_ubo lowered non-sampler uniforms to UBOs, so CB0
       * size declaration happens with other UBOs below.
       */
   }

   nir_foreach_image_variable(var, c->s) {
      int image_count = glsl_type_get_image_count(var->type);
      const struct glsl_type *itype = glsl_without_array(var->type);
      enum tgsi_texture_type tex_type =
            tgsi_texture_type_from_sampler_dim(glsl_get_sampler_dim(itype),
                                               glsl_sampler_type_is_array(itype), false);

      for (int i = 0; i < image_count; i++) {
         c->images[var->data.binding] = ureg_DECL_image(c->ureg,
                                                        var->data.binding + i,
                                                        tex_type,
                                                        var->data.image.format,
                                                        !(var->data.access & ACCESS_NON_WRITEABLE),
                                                        false);
      }
   }

   c->first_ubo = ~0;

   unsigned ubo_sizes[PIPE_MAX_CONSTANT_BUFFERS] = {0};
   nir_foreach_variable_with_modes(var, c->s, nir_var_mem_ubo) {
      int ubo = var->data.driver_location;
      if (ubo == -1)
         continue;

      if (!(ubo == 0 && c->s->info.first_ubo_is_default_ubo))
         c->first_ubo = MIN2(c->first_ubo, ubo);

      unsigned size = glsl_get_explicit_size(var->interface_type, false);

      int array_size = 1;
      if (glsl_type_is_interface(glsl_without_array(var->type)))
         array_size = MAX2(1, glsl_get_aoa_size(var->type));

      for (int i = 0; i < array_size; i++) {
         /* Even if multiple NIR variables are in the same uniform block, their
          * explicit size is the size of the block.
          */
         if (ubo_sizes[ubo + i])
            assert(ubo_sizes[ubo + i] == size);

         ubo_sizes[ubo + i] = size;
      }
   }

   for (int i = 0; i < ARRAY_SIZE(ubo_sizes); i++) {
      if (ubo_sizes[i])
         ureg_DECL_constant2D(c->ureg, 0, DIV_ROUND_UP(ubo_sizes[i], 16) - 1, i);
   }

   if (c->options->lower_ssbo_bindings) {
      c->first_ssbo = 255;
      nir_foreach_variable_with_modes(var, c->s, nir_var_mem_ssbo) {
         if (c->first_ssbo > var->data.binding)
            c->first_ssbo = var->data.binding;
      }
   } else
      c->first_ssbo = 0;

   /* XXX: nv50 uses the atomic flag to set caching for (lowered) atomic
    * counters
    */
   bool atomic = false;
   for (int i = 0; i < c->s->info.num_ssbos; ++i)
      ureg_DECL_buffer(c->ureg, c->first_ssbo + i, atomic);

}

static void
ntt_setup_registers(struct ntt_compile *c)
{
   assert(c->num_temps == 0);

   nir_foreach_reg_decl_safe(nir_reg, nir_shader_get_entrypoint(c->s)) {
      /* Permanently allocate all the array regs at the start. */
      unsigned num_array_elems = nir_intrinsic_num_array_elems(nir_reg);
      unsigned index = nir_reg->def.index;

      if (num_array_elems != 0) {
         struct ureg_dst decl = ureg_DECL_array_temporary(c->ureg, num_array_elems, true);
         c->reg_temp[index] = decl;
         assert(c->num_temps == decl.Index);
         c->num_temps += num_array_elems;
      }
   }
   c->first_non_array_temp = c->num_temps;

   /* After that, allocate non-array regs in our virtual space that we'll
    * register-allocate before ureg emit.
    */
   nir_foreach_reg_decl_safe(nir_reg, nir_shader_get_entrypoint(c->s)) {
      unsigned num_array_elems = nir_intrinsic_num_array_elems(nir_reg);
      unsigned num_components = nir_intrinsic_num_components(nir_reg);
      unsigned bit_size = nir_intrinsic_bit_size(nir_reg);
      unsigned index = nir_reg->def.index;

      /* We already handled arrays */
      if (num_array_elems == 0) {
         struct ureg_dst decl;
         uint32_t write_mask = BITFIELD_MASK(num_components);

         if (!ntt_try_store_reg_in_tgsi_output(c, &decl, nir_reg)) {
            if (bit_size == 64) {
               if (num_components > 2) {
                  fprintf(stderr, "NIR-to-TGSI: error: %d-component NIR r%d\n",
                          num_components, index);
               }

               write_mask = ntt_64bit_write_mask(write_mask);
            }

            decl = ureg_writemask(ntt_temp(c), write_mask);
         }
         c->reg_temp[index] = decl;
      }
   }
}

static struct ureg_src
ntt_get_load_const_src(struct ntt_compile *c, nir_load_const_instr *instr)
{
   int num_components = instr->def.num_components;

   if (!c->native_integers) {
      float values[4];
      assert(instr->def.bit_size == 32);
      for (int i = 0; i < num_components; i++)
         values[i] = uif(instr->value[i].u32);

      return ureg_DECL_immediate(c->ureg, values, num_components);
   } else {
      uint32_t values[4];

      if (instr->def.bit_size == 32) {
         for (int i = 0; i < num_components; i++)
            values[i] = instr->value[i].u32;
      } else {
         assert(num_components <= 2);
         for (int i = 0; i < num_components; i++) {
            values[i * 2 + 0] = instr->value[i].u64 & 0xffffffff;
            values[i * 2 + 1] = instr->value[i].u64 >> 32;
         }
         num_components *= 2;
      }

      return ureg_DECL_immediate_uint(c->ureg, values, num_components);
   }
}

static struct ureg_src
ntt_reladdr(struct ntt_compile *c, struct ureg_src addr, int addr_index)
{
   assert(addr_index < ARRAY_SIZE(c->addr_reg));

   for (int i = 0; i <= addr_index; i++) {
      if (!c->addr_declared[i]) {
         c->addr_reg[i] = ureg_writemask(ureg_DECL_address(c->ureg),
                                             TGSI_WRITEMASK_X);
         c->addr_declared[i] = true;
      }
   }

   if (c->native_integers)
      ntt_UARL(c, c->addr_reg[addr_index], addr);
   else
      ntt_ARL(c, c->addr_reg[addr_index], addr);
   return ureg_scalar(ureg_src(c->addr_reg[addr_index]), 0);
}

/* Forward declare for recursion with indirects */
static struct ureg_src
ntt_get_src(struct ntt_compile *c, nir_src src);

static struct ureg_src
ntt_get_chased_src(struct ntt_compile *c, nir_legacy_src *src)
{
   if (src->is_ssa) {
      if (src->ssa->parent_instr->type == nir_instr_type_load_const)
         return ntt_get_load_const_src(c, nir_instr_as_load_const(src->ssa->parent_instr));

      return c->ssa_temp[src->ssa->index];
   } else {
      struct ureg_dst reg_temp = c->reg_temp[src->reg.handle->index];
      reg_temp.Index += src->reg.base_offset;

      if (src->reg.indirect) {
         struct ureg_src offset = ntt_get_src(c, nir_src_for_ssa(src->reg.indirect));
         return ureg_src_indirect(ureg_src(reg_temp),
                                  ntt_reladdr(c, offset, 0));
      } else {
         return ureg_src(reg_temp);
      }
   }
}

static struct ureg_src
ntt_get_src(struct ntt_compile *c, nir_src src)
{
   nir_legacy_src chased = nir_legacy_chase_src(&src);
   return ntt_get_chased_src(c, &chased);
}

static struct ureg_src
ntt_get_alu_src(struct ntt_compile *c, nir_alu_instr *instr, int i)
{
   /* We only support 32-bit float modifiers.  The only other modifier type
    * officially supported by TGSI is 32-bit integer negates, but even those are
    * broken on virglrenderer, so skip lowering all integer and f64 float mods.
    *
    * The options->lower_fabs requests that we not have native source modifiers
    * for fabs, and instead emit MAX(a,-a) for nir_op_fabs.
    */
   nir_legacy_alu_src src =
      nir_legacy_chase_alu_src(&instr->src[i], !c->options->lower_fabs);
   struct ureg_src usrc = ntt_get_chased_src(c, &src.src);

   /* Expand double/dvec2 src references to TGSI swizzles using a pair of 32-bit
    * channels.  We skip this for undefs, as those don't get split to vec2s (but
    * the specific swizzles from an undef don't matter)
    */
   if (nir_src_bit_size(instr->src[i].src) == 64 &&
      !(src.src.is_ssa && src.src.ssa->parent_instr->type == nir_instr_type_undef)) {
      int chan1 = 1;
      if (nir_op_infos[instr->op].input_sizes[i] == 0) {
         chan1 = instr->def.num_components > 1 ? 1 : 0;
      }
      usrc = ureg_swizzle(usrc,
                          src.swizzle[0] * 2,
                          src.swizzle[0] * 2 + 1,
                          src.swizzle[chan1] * 2,
                          src.swizzle[chan1] * 2 + 1);
   } else {
      usrc = ureg_swizzle(usrc,
                          src.swizzle[0],
                          src.swizzle[1],
                          src.swizzle[2],
                          src.swizzle[3]);
   }

   if (src.fabs)
      usrc = ureg_abs(usrc);
   if (src.fneg)
      usrc = ureg_negate(usrc);

   return usrc;
}

/* Reswizzles a source so that the unset channels in the write mask still refer
 * to one of the channels present in the write mask.
 */
static struct ureg_src
ntt_swizzle_for_write_mask(struct ureg_src src, uint32_t write_mask)
{
   assert(write_mask);
   int first_chan = ffs(write_mask) - 1;
   return ureg_swizzle(src,
                       (write_mask & TGSI_WRITEMASK_X) ? TGSI_SWIZZLE_X : first_chan,
                       (write_mask & TGSI_WRITEMASK_Y) ? TGSI_SWIZZLE_Y : first_chan,
                       (write_mask & TGSI_WRITEMASK_Z) ? TGSI_SWIZZLE_Z : first_chan,
                       (write_mask & TGSI_WRITEMASK_W) ? TGSI_SWIZZLE_W : first_chan);
}

static struct ureg_dst
ntt_get_ssa_def_decl(struct ntt_compile *c, nir_def *ssa)
{
   uint32_t writemask = BITSET_MASK(ssa->num_components);
   if (ssa->bit_size == 64)
      writemask = ntt_64bit_write_mask(writemask);

   struct ureg_dst dst;
   if (!ntt_try_store_ssa_in_tgsi_output(c, &dst, ssa))
      dst = ntt_temp(c);

   c->ssa_temp[ssa->index] = ntt_swizzle_for_write_mask(ureg_src(dst), writemask);

   return ureg_writemask(dst, writemask);
}

static struct ureg_dst
ntt_get_chased_dest_decl(struct ntt_compile *c, nir_legacy_dest *dest)
{
   if (dest->is_ssa)
      return ntt_get_ssa_def_decl(c, dest->ssa);
   else
      return c->reg_temp[dest->reg.handle->index];
}

static struct ureg_dst
ntt_get_chased_dest(struct ntt_compile *c, nir_legacy_dest *dest)
{
   struct ureg_dst dst = ntt_get_chased_dest_decl(c, dest);

   if (!dest->is_ssa) {
      dst.Index += dest->reg.base_offset;

      if (dest->reg.indirect) {
         struct ureg_src offset = ntt_get_src(c, nir_src_for_ssa(dest->reg.indirect));
         dst = ureg_dst_indirect(dst, ntt_reladdr(c, offset, 0));
      }
   }

   return dst;
}

static struct ureg_dst
ntt_get_dest(struct ntt_compile *c, nir_def *def)
{
   nir_legacy_dest chased = nir_legacy_chase_dest(def);
   return ntt_get_chased_dest(c, &chased);
}

static struct ureg_dst
ntt_get_alu_dest(struct ntt_compile *c, nir_def *def)
{
   nir_legacy_alu_dest chased = nir_legacy_chase_alu_dest(def);
   struct ureg_dst dst = ntt_get_chased_dest(c, &chased.dest);

   if (chased.fsat)
      dst.Saturate = true;

   /* Only registers get write masks */
   if (chased.dest.is_ssa)
      return dst;

   int dst_64 = def->bit_size == 64;
   unsigned write_mask = chased.write_mask;

   if (dst_64)
      return ureg_writemask(dst, ntt_64bit_write_mask(write_mask));
   else
      return ureg_writemask(dst, write_mask);
}

/* For an SSA dest being populated by a constant src, replace the storage with
 * a copy of the ureg_src.
 */
static void
ntt_store_def(struct ntt_compile *c, nir_def *def, struct ureg_src src)
{
   if (!src.Indirect && !src.DimIndirect) {
      switch (src.File) {
      case TGSI_FILE_IMMEDIATE:
      case TGSI_FILE_INPUT:
      case TGSI_FILE_CONSTANT:
      case TGSI_FILE_SYSTEM_VALUE:
         c->ssa_temp[def->index] = src;
         return;
      }
   }

   ntt_MOV(c, ntt_get_ssa_def_decl(c, def), src);
}

static void
ntt_store(struct ntt_compile *c, nir_def *def, struct ureg_src src)
{
   nir_legacy_dest chased = nir_legacy_chase_dest(def);

   if (chased.is_ssa)
      ntt_store_def(c, chased.ssa, src);
   else {
      struct ureg_dst dst = ntt_get_chased_dest(c, &chased);
      ntt_MOV(c, dst, src);
   }
}

static void
ntt_emit_scalar(struct ntt_compile *c, unsigned tgsi_op,
                struct ureg_dst dst,
                struct ureg_src src0,
                struct ureg_src src1)
{
   unsigned i;

   /* POW is the only 2-operand scalar op. */
   if (tgsi_op != TGSI_OPCODE_POW)
      src1 = src0;

   for (i = 0; i < 4; i++) {
      if (dst.WriteMask & (1 << i)) {
         ntt_insn(c, tgsi_op,
                  ureg_writemask(dst, 1 << i),
                  ureg_scalar(src0, i),
                  ureg_scalar(src1, i),
                  ureg_src_undef(), ureg_src_undef());
      }
   }
}

static void
ntt_emit_alu(struct ntt_compile *c, nir_alu_instr *instr)
{
   struct ureg_src src[4];
   struct ureg_dst dst;
   unsigned i;
   int dst_64 = instr->def.bit_size == 64;
   int src_64 = nir_src_bit_size(instr->src[0].src) == 64;
   int num_srcs = nir_op_infos[instr->op].num_inputs;

   /* Don't try to translate folded fsat since their source won't be valid */
   if (instr->op == nir_op_fsat && nir_legacy_fsat_folds(instr))
      return;

   c->precise = instr->exact;

   assert(num_srcs <= ARRAY_SIZE(src));
   for (i = 0; i < num_srcs; i++)
      src[i] = ntt_get_alu_src(c, instr, i);
   for (; i < ARRAY_SIZE(src); i++)
      src[i] = ureg_src_undef();

   dst = ntt_get_alu_dest(c, &instr->def);

   static enum tgsi_opcode op_map[][2] = {
      [nir_op_mov] = { TGSI_OPCODE_MOV, TGSI_OPCODE_MOV },

      /* fabs/fneg 32-bit are special-cased below. */
      [nir_op_fabs] = { 0, TGSI_OPCODE_DABS },
      [nir_op_fneg] = { 0, TGSI_OPCODE_DNEG },

      [nir_op_fdot2] = { TGSI_OPCODE_DP2 },
      [nir_op_fdot3] = { TGSI_OPCODE_DP3 },
      [nir_op_fdot4] = { TGSI_OPCODE_DP4 },
      [nir_op_fdot2_replicated] = { TGSI_OPCODE_DP2 },
      [nir_op_fdot3_replicated] = { TGSI_OPCODE_DP3 },
      [nir_op_fdot4_replicated] = { TGSI_OPCODE_DP4 },
      [nir_op_ffloor] = { TGSI_OPCODE_FLR, TGSI_OPCODE_DFLR },
      [nir_op_ffract] = { TGSI_OPCODE_FRC, TGSI_OPCODE_DFRAC },
      [nir_op_fceil] = { TGSI_OPCODE_CEIL, TGSI_OPCODE_DCEIL },
      [nir_op_fround_even] = { TGSI_OPCODE_ROUND, TGSI_OPCODE_DROUND },
      [nir_op_fdiv] = { TGSI_OPCODE_DIV, TGSI_OPCODE_DDIV },
      [nir_op_idiv] = { TGSI_OPCODE_IDIV, TGSI_OPCODE_I64DIV },
      [nir_op_udiv] = { TGSI_OPCODE_UDIV, TGSI_OPCODE_U64DIV },

      [nir_op_frcp] = { 0, TGSI_OPCODE_DRCP },
      [nir_op_frsq] = { 0, TGSI_OPCODE_DRSQ },
      [nir_op_fsqrt] = { 0, TGSI_OPCODE_DSQRT },

      /* The conversions will have one combination of src and dst bitsize. */
      [nir_op_f2f32] = { 0, TGSI_OPCODE_D2F },
      [nir_op_f2f64] = { TGSI_OPCODE_F2D },
      [nir_op_i2i64] = { TGSI_OPCODE_I2I64 },

      [nir_op_f2i32] = { TGSI_OPCODE_F2I, TGSI_OPCODE_D2I },
      [nir_op_f2i64] = { TGSI_OPCODE_F2I64, TGSI_OPCODE_D2I64 },
      [nir_op_f2u32] = { TGSI_OPCODE_F2U, TGSI_OPCODE_D2U },
      [nir_op_f2u64] = { TGSI_OPCODE_F2U64, TGSI_OPCODE_D2U64 },
      [nir_op_i2f32] = { TGSI_OPCODE_I2F, TGSI_OPCODE_I642F },
      [nir_op_i2f64] = { TGSI_OPCODE_I2D, TGSI_OPCODE_I642D },
      [nir_op_u2f32] = { TGSI_OPCODE_U2F, TGSI_OPCODE_U642F },
      [nir_op_u2f64] = { TGSI_OPCODE_U2D, TGSI_OPCODE_U642D },

      [nir_op_slt] = { TGSI_OPCODE_SLT },
      [nir_op_sge] = { TGSI_OPCODE_SGE },
      [nir_op_seq] = { TGSI_OPCODE_SEQ },
      [nir_op_sne] = { TGSI_OPCODE_SNE },

      [nir_op_flt32] = { TGSI_OPCODE_FSLT, TGSI_OPCODE_DSLT },
      [nir_op_fge32] = { TGSI_OPCODE_FSGE, TGSI_OPCODE_DSGE },
      [nir_op_feq32] = { TGSI_OPCODE_FSEQ, TGSI_OPCODE_DSEQ },
      [nir_op_fneu32] = { TGSI_OPCODE_FSNE, TGSI_OPCODE_DSNE },

      [nir_op_ilt32] = { TGSI_OPCODE_ISLT, TGSI_OPCODE_I64SLT },
      [nir_op_ige32] = { TGSI_OPCODE_ISGE, TGSI_OPCODE_I64SGE },
      [nir_op_ieq32] = { TGSI_OPCODE_USEQ, TGSI_OPCODE_U64SEQ },
      [nir_op_ine32] = { TGSI_OPCODE_USNE, TGSI_OPCODE_U64SNE },

      [nir_op_ult32] = { TGSI_OPCODE_USLT, TGSI_OPCODE_U64SLT },
      [nir_op_uge32] = { TGSI_OPCODE_USGE, TGSI_OPCODE_U64SGE },

      [nir_op_iabs] = { TGSI_OPCODE_IABS, TGSI_OPCODE_I64ABS },
      [nir_op_ineg] = { TGSI_OPCODE_INEG, TGSI_OPCODE_I64NEG },
      [nir_op_fsign] = { TGSI_OPCODE_SSG, TGSI_OPCODE_DSSG },
      [nir_op_isign] = { TGSI_OPCODE_ISSG, TGSI_OPCODE_I64SSG },
      [nir_op_ftrunc] = { TGSI_OPCODE_TRUNC, TGSI_OPCODE_DTRUNC },
      [nir_op_fddx] = { TGSI_OPCODE_DDX },
      [nir_op_fddy] = { TGSI_OPCODE_DDY },
      [nir_op_fddx_coarse] = { TGSI_OPCODE_DDX },
      [nir_op_fddy_coarse] = { TGSI_OPCODE_DDY },
      [nir_op_fddx_fine] = { TGSI_OPCODE_DDX_FINE },
      [nir_op_fddy_fine] = { TGSI_OPCODE_DDY_FINE },
      [nir_op_pack_half_2x16] = { TGSI_OPCODE_PK2H },
      [nir_op_unpack_half_2x16] = { TGSI_OPCODE_UP2H },
      [nir_op_ibitfield_extract] = { TGSI_OPCODE_IBFE },
      [nir_op_ubitfield_extract] = { TGSI_OPCODE_UBFE },
      [nir_op_bitfield_insert] = { TGSI_OPCODE_BFI },
      [nir_op_bitfield_reverse] = { TGSI_OPCODE_BREV },
      [nir_op_bit_count] = { TGSI_OPCODE_POPC },
      [nir_op_ifind_msb] = { TGSI_OPCODE_IMSB },
      [nir_op_ufind_msb] = { TGSI_OPCODE_UMSB },
      [nir_op_find_lsb] = { TGSI_OPCODE_LSB },
      [nir_op_fadd] = { TGSI_OPCODE_ADD, TGSI_OPCODE_DADD },
      [nir_op_iadd] = { TGSI_OPCODE_UADD, TGSI_OPCODE_U64ADD },
      [nir_op_fmul] = { TGSI_OPCODE_MUL, TGSI_OPCODE_DMUL },
      [nir_op_imul] = { TGSI_OPCODE_UMUL, TGSI_OPCODE_U64MUL },
      [nir_op_imod] = { TGSI_OPCODE_MOD, TGSI_OPCODE_I64MOD },
      [nir_op_umod] = { TGSI_OPCODE_UMOD, TGSI_OPCODE_U64MOD },
      [nir_op_imul_high] = { TGSI_OPCODE_IMUL_HI },
      [nir_op_umul_high] = { TGSI_OPCODE_UMUL_HI },
      [nir_op_ishl] = { TGSI_OPCODE_SHL, TGSI_OPCODE_U64SHL },
      [nir_op_ishr] = { TGSI_OPCODE_ISHR, TGSI_OPCODE_I64SHR },
      [nir_op_ushr] = { TGSI_OPCODE_USHR, TGSI_OPCODE_U64SHR },

      /* These bitwise ops don't care about 32 vs 64 types, so they have the
       * same TGSI op.
       */
      [nir_op_inot] = { TGSI_OPCODE_NOT, TGSI_OPCODE_NOT },
      [nir_op_iand] = { TGSI_OPCODE_AND, TGSI_OPCODE_AND },
      [nir_op_ior] = { TGSI_OPCODE_OR, TGSI_OPCODE_OR },
      [nir_op_ixor] = { TGSI_OPCODE_XOR, TGSI_OPCODE_XOR },

      [nir_op_fmin] = { TGSI_OPCODE_MIN, TGSI_OPCODE_DMIN },
      [nir_op_imin] = { TGSI_OPCODE_IMIN, TGSI_OPCODE_I64MIN },
      [nir_op_umin] = { TGSI_OPCODE_UMIN, TGSI_OPCODE_U64MIN },
      [nir_op_fmax] = { TGSI_OPCODE_MAX, TGSI_OPCODE_DMAX },
      [nir_op_imax] = { TGSI_OPCODE_IMAX, TGSI_OPCODE_I64MAX },
      [nir_op_umax] = { TGSI_OPCODE_UMAX, TGSI_OPCODE_U64MAX },
      [nir_op_ffma] = { TGSI_OPCODE_MAD, TGSI_OPCODE_DMAD },
      [nir_op_ldexp] = { TGSI_OPCODE_LDEXP, 0 },
   };

   if (src_64 && !dst_64) {
      if (num_srcs == 2 || nir_op_infos[instr->op].output_type == nir_type_bool32) {
         /* TGSI's 64 bit compares storing to 32-bit are weird and write .xz instead
         * of .xy.
         */
         assert(!(dst.WriteMask & TGSI_WRITEMASK_YW));
      } else {
         /* TGSI 64bit-to-32-bit conversions only generate results in the .xy
         * channels and will need to get fixed up.
         */
        assert(!(dst.WriteMask & TGSI_WRITEMASK_ZW));
      }
   }

   bool table_op64 = src_64;
   if (instr->op < ARRAY_SIZE(op_map) && op_map[instr->op][table_op64] != 0) {
      /* The normal path for NIR to TGSI ALU op translation */
      ntt_insn(c, op_map[instr->op][table_op64],
                dst, src[0], src[1], src[2], src[3]);
   } else {
      /* Special cases for NIR to TGSI ALU op translation. */

      /* TODO: Use something like the ntt_store() path for the MOV calls so we
       * don't emit extra MOVs for swizzles/srcmods of inputs/const/imm.
       */

      switch (instr->op) {
      case nir_op_u2u64:
         ntt_AND(c, dst, ureg_swizzle(src[0],
                                             TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                                             TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Y),
                  ureg_imm4u(c->ureg, ~0, 0, ~0, 0));
         break;

      case nir_op_i2i32:
      case nir_op_u2u32:
         assert(src_64);
         ntt_MOV(c, dst, ureg_swizzle(src[0],
                                             TGSI_SWIZZLE_X, TGSI_SWIZZLE_Z,
                                             TGSI_SWIZZLE_X, TGSI_SWIZZLE_X));
         break;

      case nir_op_fabs:
         /* Try to eliminate */
         if (!c->options->lower_fabs && nir_legacy_float_mod_folds(instr))
            break;

         if (c->options->lower_fabs)
            ntt_MAX(c, dst, src[0], ureg_negate(src[0]));
         else
            ntt_MOV(c, dst, ureg_abs(src[0]));
         break;

      case nir_op_fsat:
         if (dst_64) {
            ntt_MIN(c, dst, src[0], ntt_64bit_1f(c));
            ntt_MAX(c, dst, ureg_src(dst), ureg_imm1u(c->ureg, 0));
         } else {
            ntt_MOV(c, ureg_saturate(dst), src[0]);
         }
         break;

      case nir_op_fneg:
         /* Try to eliminate */
         if (nir_legacy_float_mod_folds(instr))
            break;

         ntt_MOV(c, dst, ureg_negate(src[0]));
         break;

         /* NOTE: TGSI 32-bit math ops have the old "one source channel
          * replicated to all dst channels" behavior, while 64 is normal mapping
          * of src channels to dst.
          */
      case nir_op_frcp:
         assert(!dst_64);
         ntt_emit_scalar(c, TGSI_OPCODE_RCP, dst, src[0], ureg_src_undef());
         break;

      case nir_op_frsq:
         assert(!dst_64);
         ntt_emit_scalar(c, TGSI_OPCODE_RSQ, dst, src[0], ureg_src_undef());
         break;

      case nir_op_fsqrt:
         assert(!dst_64);
         ntt_emit_scalar(c, TGSI_OPCODE_SQRT, dst, src[0], ureg_src_undef());
         break;

      case nir_op_fexp2:
         assert(!dst_64);
         ntt_emit_scalar(c, TGSI_OPCODE_EX2, dst, src[0], ureg_src_undef());
         break;

      case nir_op_flog2:
         assert(!dst_64);
         ntt_emit_scalar(c, TGSI_OPCODE_LG2, dst, src[0], ureg_src_undef());
         break;

      case nir_op_b2f32:
         ntt_AND(c, dst, src[0], ureg_imm1f(c->ureg, 1.0));
         break;

      case nir_op_b2f64:
         ntt_AND(c, dst,
                  ureg_swizzle(src[0],
                               TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                               TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Y),
                  ntt_64bit_1f(c));
         break;

      case nir_op_b2i32:
         ntt_AND(c, dst, src[0], ureg_imm1u(c->ureg, 1));
         break;

      case nir_op_b2i64:
         ntt_AND(c, dst,
                  ureg_swizzle(src[0],
                               TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                               TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Y),
                  ureg_imm4u(c->ureg, 1, 0, 1, 0));
         break;

      case nir_op_fsin:
         ntt_emit_scalar(c, TGSI_OPCODE_SIN, dst, src[0], ureg_src_undef());
         break;

      case nir_op_fcos:
         ntt_emit_scalar(c, TGSI_OPCODE_COS, dst, src[0], ureg_src_undef());
         break;

      case nir_op_fsub:
         assert(!dst_64);
         ntt_ADD(c, dst, src[0], ureg_negate(src[1]));
         break;

      case nir_op_isub:
         assert(!dst_64);
         ntt_UADD(c, dst, src[0], ureg_negate(src[1]));
         break;

      case nir_op_fmod:
         unreachable("should be handled by .lower_fmod = true");
         break;

      case nir_op_fpow:
         ntt_emit_scalar(c, TGSI_OPCODE_POW, dst, src[0], src[1]);
         break;

      case nir_op_flrp:
         ntt_LRP(c, dst, src[2], src[1], src[0]);
         break;

      case nir_op_pack_64_2x32_split:
         ntt_MOV(c, ureg_writemask(dst, TGSI_WRITEMASK_XZ),
                  ureg_swizzle(src[0],
                               TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                               TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Y));
         ntt_MOV(c, ureg_writemask(dst, TGSI_WRITEMASK_YW),
                  ureg_swizzle(src[1],
                               TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                               TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Y));
         break;

      case nir_op_unpack_64_2x32_split_x:
         ntt_MOV(c, dst, ureg_swizzle(src[0],
                                             TGSI_SWIZZLE_X, TGSI_SWIZZLE_Z,
                                             TGSI_SWIZZLE_X, TGSI_SWIZZLE_Z));
         break;

      case nir_op_unpack_64_2x32_split_y:
         ntt_MOV(c, dst, ureg_swizzle(src[0],
                                             TGSI_SWIZZLE_Y, TGSI_SWIZZLE_W,
                                             TGSI_SWIZZLE_Y, TGSI_SWIZZLE_W));
         break;

      case nir_op_b32csel:
         if (nir_src_bit_size(instr->src[1].src) == 64) {
            ntt_UCMP(c, dst, ureg_swizzle(src[0],
                                                 TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                                                 TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Y),
                      src[1], src[2]);
         } else {
            ntt_UCMP(c, dst, src[0], src[1], src[2]);
         }
         break;

      case nir_op_fcsel:
         /* If CMP isn't supported, then the flags that enable NIR to generate
          * this opcode should also not be set.
          */
         assert(!c->options->lower_cmp);

         /* Implement this as CMP(-abs(src0), src1, src2). */
         ntt_CMP(c, dst, ureg_negate(ureg_abs(src[0])), src[1], src[2]);
         break;

      case nir_op_fcsel_gt:
         /* If CMP isn't supported, then the flags that enable NIR to generate
          * these opcodes should also not be set.
          */
         assert(!c->options->lower_cmp);

         ntt_CMP(c, dst, ureg_negate(src[0]), src[1], src[2]);
         break;

      case nir_op_fcsel_ge:
         /* If CMP isn't supported, then the flags that enable NIR to generate
          * these opcodes should also not be set.
          */
         assert(!c->options->lower_cmp);

         /* Implement this as if !(src0 < 0.0) was identical to src0 >= 0.0. */
         ntt_CMP(c, dst, src[0], src[2], src[1]);
         break;

      case nir_op_frexp_sig:
      case nir_op_frexp_exp:
         unreachable("covered by nir_lower_frexp()");
         break;

      case nir_op_ldexp:
         assert(dst_64); /* 32bit handled in table. */
         ntt_DLDEXP(c, dst, src[0],
                     ureg_swizzle(src[1],
                                  TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                                  TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Y));
         break;

      case nir_op_vec4:
      case nir_op_vec3:
      case nir_op_vec2:
         unreachable("covered by nir_lower_vec_to_movs()");

      default:
         fprintf(stderr, "Unknown NIR opcode: %s\n", nir_op_infos[instr->op].name);
         unreachable("Unknown NIR opcode");
      }
   }

   c->precise = false;
}

static struct ureg_src
ntt_ureg_src_indirect(struct ntt_compile *c, struct ureg_src usrc,
                      nir_src src, int addr_reg)
{
   if (nir_src_is_const(src)) {
      usrc.Index += ntt_src_as_uint(c, src);
      return usrc;
   } else {
      return ureg_src_indirect(usrc, ntt_reladdr(c, ntt_get_src(c, src), addr_reg));
   }
}

static struct ureg_dst
ntt_ureg_dst_indirect(struct ntt_compile *c, struct ureg_dst dst,
                      nir_src src)
{
   if (nir_src_is_const(src)) {
      dst.Index += ntt_src_as_uint(c, src);
      return dst;
   } else {
      return ureg_dst_indirect(dst, ntt_reladdr(c, ntt_get_src(c, src), 0));
   }
}

static struct ureg_src
ntt_ureg_src_dimension_indirect(struct ntt_compile *c, struct ureg_src usrc,
                         nir_src src)
{
   if (nir_src_is_const(src)) {
      return ureg_src_dimension(usrc, ntt_src_as_uint(c, src));
   }
   else
   {
      return ureg_src_dimension_indirect(usrc,
                                         ntt_reladdr(c, ntt_get_src(c, src), 1),
                                         0);
   }
}

static struct ureg_dst
ntt_ureg_dst_dimension_indirect(struct ntt_compile *c, struct ureg_dst udst,
                                nir_src src)
{
   if (nir_src_is_const(src)) {
      return ureg_dst_dimension(udst, ntt_src_as_uint(c, src));
   } else {
      return ureg_dst_dimension_indirect(udst,
                                         ntt_reladdr(c, ntt_get_src(c, src), 1),
                                         0);
   }
}
/* Some load operations in NIR will have a fractional offset that we need to
 * swizzle down before storing to the result register.
 */
static struct ureg_src
ntt_shift_by_frac(struct ureg_src src, unsigned frac, unsigned num_components)
{
   return ureg_swizzle(src,
                       frac,
                       frac + MIN2(num_components - 1, 1),
                       frac + MIN2(num_components - 1, 2),
                       frac + MIN2(num_components - 1, 3));
}


static void
ntt_emit_load_ubo(struct ntt_compile *c, nir_intrinsic_instr *instr)
{
   int bit_size = instr->def.bit_size;
   assert(bit_size == 32 || instr->num_components <= 2);

   struct ureg_src src = ureg_src_register(TGSI_FILE_CONSTANT, 0);

   struct ureg_dst addr_temp = ureg_dst_undef();

   if (nir_src_is_const(instr->src[0])) {
      src = ureg_src_dimension(src, ntt_src_as_uint(c, instr->src[0]));
   } else {
      /* virglrenderer requires that indirect UBO references have the UBO
       * array's base index in the Index field, not added to the indrect
       * address.
       *
       * Many nir intrinsics have a base address const value for the start of
       * their array indirection, but load_ubo doesn't.  We fake it by
       * subtracting it off here.
       */
      addr_temp = ntt_temp(c);
      ntt_UADD(c, addr_temp, ntt_get_src(c, instr->src[0]), ureg_imm1i(c->ureg, -c->first_ubo));
      src = ureg_src_dimension_indirect(src,
                                         ntt_reladdr(c, ureg_src(addr_temp), 1),
                                         c->first_ubo);
   }

   if (instr->intrinsic == nir_intrinsic_load_ubo_vec4) {
      /* !PIPE_CAP_LOAD_CONSTBUF: Just emit it as a vec4 reference to the const
       * file.
       */
      src.Index = nir_intrinsic_base(instr);

      if (nir_src_is_const(instr->src[1])) {
         src.Index += ntt_src_as_uint(c, instr->src[1]);
      } else {
         src = ureg_src_indirect(src, ntt_reladdr(c, ntt_get_src(c, instr->src[1]), 0));
      }

      int start_component = nir_intrinsic_component(instr);
      if (bit_size == 64)
         start_component *= 2;

      src = ntt_shift_by_frac(src, start_component,
                              instr->num_components * bit_size / 32);

      ntt_store(c, &instr->def, src);
   } else {
      /* PIPE_CAP_LOAD_CONSTBUF: Not necessarily vec4 aligned, emit a
       * TGSI_OPCODE_LOAD instruction from the const file.
       */
      struct ntt_insn *insn =
         ntt_insn(c, TGSI_OPCODE_LOAD,
                  ntt_get_dest(c, &instr->def),
                  src, ntt_get_src(c, instr->src[1]),
                  ureg_src_undef(), ureg_src_undef());
      insn->is_mem = true;
      insn->tex_target = 0;
      insn->mem_qualifier = 0;
      insn->mem_format = 0; /* unused */
   }
}

static unsigned
ntt_get_access_qualifier(nir_intrinsic_instr *instr)
{
   enum gl_access_qualifier access = nir_intrinsic_access(instr);
   unsigned qualifier = 0;

   if (access & ACCESS_COHERENT)
      qualifier |= TGSI_MEMORY_COHERENT;
   if (access & ACCESS_VOLATILE)
      qualifier |= TGSI_MEMORY_VOLATILE;
   if (access & ACCESS_RESTRICT)
      qualifier |= TGSI_MEMORY_RESTRICT;

   return qualifier;
}

static unsigned
ntt_translate_atomic_op(nir_atomic_op op)
{
   switch (op) {
   case nir_atomic_op_iadd: return TGSI_OPCODE_ATOMUADD;
   case nir_atomic_op_fadd: return TGSI_OPCODE_ATOMFADD;
   case nir_atomic_op_imin: return TGSI_OPCODE_ATOMIMIN;
   case nir_atomic_op_imax: return TGSI_OPCODE_ATOMIMAX;
   case nir_atomic_op_umin: return TGSI_OPCODE_ATOMUMIN;
   case nir_atomic_op_umax: return TGSI_OPCODE_ATOMUMAX;
   case nir_atomic_op_iand: return TGSI_OPCODE_ATOMAND;
   case nir_atomic_op_ixor: return TGSI_OPCODE_ATOMXOR;
   case nir_atomic_op_ior:  return TGSI_OPCODE_ATOMOR;
   case nir_atomic_op_xchg: return TGSI_OPCODE_ATOMXCHG;
   default: unreachable("invalid atomic");
   }
}

static void
ntt_emit_mem(struct ntt_compile *c, nir_intrinsic_instr *instr,
             nir_variable_mode mode)
{
   bool is_store = (instr->intrinsic == nir_intrinsic_store_ssbo ||
                    instr->intrinsic == nir_intrinsic_store_shared);
   bool is_load = (instr->intrinsic == nir_intrinsic_atomic_counter_read ||
                    instr->intrinsic == nir_intrinsic_load_ssbo ||
                    instr->intrinsic == nir_intrinsic_load_shared);
   unsigned opcode;
   struct ureg_src src[4];
   int num_src = 0;
   int next_src;
   struct ureg_dst addr_temp = ureg_dst_undef();

   struct ureg_src memory;
   switch (mode) {
   case nir_var_mem_ssbo:
      memory = ntt_ureg_src_indirect(c, ureg_src_register(TGSI_FILE_BUFFER,
                                                          c->first_ssbo),
                                     instr->src[is_store ? 1 : 0], 2);
      next_src = 1;
      break;
   case nir_var_mem_shared:
      memory = ureg_src_register(TGSI_FILE_MEMORY, 0);
      next_src = 0;
      break;
   case nir_var_uniform: { /* HW atomic buffers */
      nir_src src = instr->src[0];
      uint32_t offset = (ntt_extract_const_src_offset(&src) +
                         nir_intrinsic_range_base(instr)) / 4;

      memory = ureg_src_register(TGSI_FILE_HW_ATOMIC, offset);
      /* ntt_ureg_src_indirect, except dividing by 4 */
      if (nir_src_is_const(src)) {
         memory.Index += nir_src_as_uint(src) / 4;
      } else {
         addr_temp = ntt_temp(c);
         ntt_USHR(c, addr_temp, ntt_get_src(c, src), ureg_imm1i(c->ureg, 2));
         memory = ureg_src_indirect(memory, ntt_reladdr(c, ureg_src(addr_temp), 2));
      }
      memory = ureg_src_dimension(memory, nir_intrinsic_base(instr));
      next_src = 0;
      break;
   }

   default:
      unreachable("unknown memory type");
   }

   if (is_store) {
      src[num_src++] = ntt_get_src(c, instr->src[next_src + 1]); /* offset */
      src[num_src++] = ntt_get_src(c, instr->src[0]); /* value */
   } else {
      src[num_src++] = memory;
      if (instr->intrinsic != nir_intrinsic_get_ssbo_size) {
         src[num_src++] = ntt_get_src(c, instr->src[next_src++]); /* offset */
         switch (instr->intrinsic) {
         case nir_intrinsic_atomic_counter_inc:
            src[num_src++] = ureg_imm1i(c->ureg, 1);
            break;
         case nir_intrinsic_atomic_counter_post_dec:
            src[num_src++] = ureg_imm1i(c->ureg, -1);
            break;
         default:
            if (!is_load)
               src[num_src++] = ntt_get_src(c, instr->src[next_src++]); /* value */
            break;
         }
      }
   }


   switch (instr->intrinsic) {
   case nir_intrinsic_ssbo_atomic:
   case nir_intrinsic_shared_atomic:
      opcode = ntt_translate_atomic_op(nir_intrinsic_atomic_op(instr));
      break;
   case nir_intrinsic_atomic_counter_add:
   case nir_intrinsic_atomic_counter_inc:
   case nir_intrinsic_atomic_counter_post_dec:
      opcode = TGSI_OPCODE_ATOMUADD;
      break;
   case nir_intrinsic_atomic_counter_min:
      opcode = TGSI_OPCODE_ATOMIMIN;
      break;
   case nir_intrinsic_atomic_counter_max:
      opcode = TGSI_OPCODE_ATOMIMAX;
      break;
   case nir_intrinsic_atomic_counter_and:
      opcode = TGSI_OPCODE_ATOMAND;
      break;
   case nir_intrinsic_atomic_counter_or:
      opcode = TGSI_OPCODE_ATOMOR;
      break;
   case nir_intrinsic_atomic_counter_xor:
      opcode = TGSI_OPCODE_ATOMXOR;
      break;
   case nir_intrinsic_atomic_counter_exchange:
      opcode = TGSI_OPCODE_ATOMXCHG;
      break;
   case nir_intrinsic_atomic_counter_comp_swap:
   case nir_intrinsic_ssbo_atomic_swap:
   case nir_intrinsic_shared_atomic_swap:
      opcode = TGSI_OPCODE_ATOMCAS;
      src[num_src++] = ntt_get_src(c, instr->src[next_src++]);
      break;
   case nir_intrinsic_atomic_counter_read:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_load_shared:
      opcode = TGSI_OPCODE_LOAD;
      break;
   case nir_intrinsic_store_ssbo:
   case nir_intrinsic_store_shared:
      opcode = TGSI_OPCODE_STORE;
      break;
   case nir_intrinsic_get_ssbo_size:
      opcode = TGSI_OPCODE_RESQ;
      break;
   default:
      unreachable("unknown memory op");
   }

   unsigned qualifier = 0;
   if (mode == nir_var_mem_ssbo &&
       instr->intrinsic != nir_intrinsic_get_ssbo_size) {
      qualifier = ntt_get_access_qualifier(instr);
   }

   struct ureg_dst dst;
   if (is_store) {
      dst = ureg_dst(memory);

      unsigned write_mask = nir_intrinsic_write_mask(instr);
      if (nir_src_bit_size(instr->src[0]) == 64)
         write_mask = ntt_64bit_write_mask(write_mask);
      dst = ureg_writemask(dst, write_mask);
   } else {
      dst = ntt_get_dest(c, &instr->def);
   }

   struct ntt_insn *insn = ntt_insn(c, opcode, dst, src[0], src[1], src[2], src[3]);
   insn->tex_target = TGSI_TEXTURE_BUFFER;
   insn->mem_qualifier = qualifier;
   insn->mem_format = 0; /* unused */
   insn->is_mem = true;
}

static void
ntt_emit_image_load_store(struct ntt_compile *c, nir_intrinsic_instr *instr)
{
   unsigned op;
   struct ureg_src srcs[4];
   int num_src = 0;
   enum glsl_sampler_dim dim = nir_intrinsic_image_dim(instr);
   bool is_array = nir_intrinsic_image_array(instr);

   struct ureg_dst temp = ureg_dst_undef();

   enum tgsi_texture_type target = tgsi_texture_type_from_sampler_dim(dim, is_array, false);

   struct ureg_src resource;
   switch (instr->intrinsic) {
   case nir_intrinsic_bindless_image_load:
   case nir_intrinsic_bindless_image_store:
   case nir_intrinsic_bindless_image_size:
   case nir_intrinsic_bindless_image_samples:
   case nir_intrinsic_bindless_image_atomic:
   case nir_intrinsic_bindless_image_atomic_swap:
      resource = ntt_get_src(c, instr->src[0]);
      break;
   default:
      resource = ntt_ureg_src_indirect(c, ureg_src_register(TGSI_FILE_IMAGE, 0),
                                       instr->src[0], 2);
      resource.Index += nir_intrinsic_range_base(instr);
   }

   struct ureg_dst dst;
   if (instr->intrinsic == nir_intrinsic_image_store ||
       instr->intrinsic == nir_intrinsic_bindless_image_store) {
      dst = ureg_dst(resource);
   } else {
      srcs[num_src++] = resource;
      dst = ntt_get_dest(c, &instr->def);
   }
   struct ureg_dst opcode_dst = dst;

   if (instr->intrinsic != nir_intrinsic_image_size &&
       instr->intrinsic != nir_intrinsic_image_samples &&
       instr->intrinsic != nir_intrinsic_bindless_image_size &&
       instr->intrinsic != nir_intrinsic_bindless_image_samples) {
      struct ureg_src coord = ntt_get_src(c, instr->src[1]);

      if (dim == GLSL_SAMPLER_DIM_MS) {
         temp = ntt_temp(c);
         ntt_MOV(c, temp, coord);
         ntt_MOV(c, ureg_writemask(temp, TGSI_WRITEMASK_W),
                  ureg_scalar(ntt_get_src(c, instr->src[2]), TGSI_SWIZZLE_X));
         coord = ureg_src(temp);
      }
      srcs[num_src++] = coord;

      if (instr->intrinsic != nir_intrinsic_image_load &&
          instr->intrinsic != nir_intrinsic_bindless_image_load) {
         srcs[num_src++] = ntt_get_src(c, instr->src[3]); /* data */
         if (instr->intrinsic == nir_intrinsic_image_atomic_swap ||
             instr->intrinsic == nir_intrinsic_bindless_image_atomic_swap)
            srcs[num_src++] = ntt_get_src(c, instr->src[4]); /* data2 */
      }
   }

   switch (instr->intrinsic) {
   case nir_intrinsic_image_load:
   case nir_intrinsic_bindless_image_load:
      op = TGSI_OPCODE_LOAD;
      break;
   case nir_intrinsic_image_store:
   case nir_intrinsic_bindless_image_store:
      op = TGSI_OPCODE_STORE;
      break;
   case nir_intrinsic_image_size:
   case nir_intrinsic_bindless_image_size:
      op = TGSI_OPCODE_RESQ;
      break;
   case nir_intrinsic_image_samples:
   case nir_intrinsic_bindless_image_samples:
      op = TGSI_OPCODE_RESQ;
      opcode_dst = ureg_writemask(ntt_temp(c), TGSI_WRITEMASK_W);
      break;
   case nir_intrinsic_image_atomic:
   case nir_intrinsic_bindless_image_atomic:
      op = ntt_translate_atomic_op(nir_intrinsic_atomic_op(instr));
      break;
   case nir_intrinsic_image_atomic_swap:
   case nir_intrinsic_bindless_image_atomic_swap:
      op = TGSI_OPCODE_ATOMCAS;
      break;
   default:
      unreachable("bad op");
   }

   struct ntt_insn *insn = ntt_insn(c, op, opcode_dst, srcs[0], srcs[1], srcs[2], srcs[3]);
   insn->tex_target = target;
   insn->mem_qualifier = ntt_get_access_qualifier(instr);
   insn->mem_format = nir_intrinsic_format(instr);
   insn->is_mem = true;

   if (instr->intrinsic == nir_intrinsic_image_samples ||
       instr->intrinsic == nir_intrinsic_bindless_image_samples)
      ntt_MOV(c, dst, ureg_scalar(ureg_src(opcode_dst), 3));
}

static void
ntt_emit_load_input(struct ntt_compile *c, nir_intrinsic_instr *instr)
{
   uint32_t frac = nir_intrinsic_component(instr);
   uint32_t num_components = instr->num_components;
   unsigned base = nir_intrinsic_base(instr);
   struct ureg_src input;
   nir_io_semantics semantics = nir_intrinsic_io_semantics(instr);
   bool is_64 = instr->def.bit_size == 64;

   if (c->s->info.stage == MESA_SHADER_VERTEX) {
      input = ureg_DECL_vs_input(c->ureg, base);
      for (int i = 1; i < semantics.num_slots; i++)
         ureg_DECL_vs_input(c->ureg, base + i);
   } else if (c->s->info.stage != MESA_SHADER_FRAGMENT) {
      unsigned semantic_name, semantic_index;
      ntt_get_gl_varying_semantic(c, semantics.location,
                                  &semantic_name, &semantic_index);

      /* XXX: ArrayID is used in r600 gs inputs */
      uint32_t array_id = 0;

      input = ureg_DECL_input_layout(c->ureg,
                                     semantic_name,
                                     semantic_index,
                                     base,
                                     ntt_tgsi_usage_mask(frac,
                                                         instr->num_components,
                                                         is_64),
                                     array_id,
                                     semantics.num_slots);
   } else {
      input = c->input_index_map[base];
   }

   if (is_64)
      num_components *= 2;

   input = ntt_shift_by_frac(input, frac, num_components);

   switch (instr->intrinsic) {
   case nir_intrinsic_load_input:
      input = ntt_ureg_src_indirect(c, input, instr->src[0], 0);
      ntt_store(c, &instr->def, input);
      break;

   case nir_intrinsic_load_per_vertex_input:
      input = ntt_ureg_src_indirect(c, input, instr->src[1], 0);
      input = ntt_ureg_src_dimension_indirect(c, input, instr->src[0]);
      ntt_store(c, &instr->def, input);
      break;

   case nir_intrinsic_load_interpolated_input: {
      input = ntt_ureg_src_indirect(c, input, instr->src[1], 0);

      nir_intrinsic_instr *bary_instr =
         nir_instr_as_intrinsic(instr->src[0].ssa->parent_instr);

      switch (bary_instr->intrinsic) {
      case nir_intrinsic_load_barycentric_pixel:
      case nir_intrinsic_load_barycentric_sample:
         /* For these, we know that the barycentric load matches the
          * interpolation on the input declaration, so we can use it directly.
          */
         ntt_store(c, &instr->def, input);
         break;

      case nir_intrinsic_load_barycentric_centroid:
         /* If the input was declared centroid, then there's no need to
          * emit the extra TGSI interp instruction, we can just read the
          * input.
          */
         if (c->centroid_inputs & (1ull << nir_intrinsic_base(instr))) {
            ntt_store(c, &instr->def, input);
         } else {
            ntt_INTERP_CENTROID(c, ntt_get_dest(c, &instr->def), input);
         }
         break;

      case nir_intrinsic_load_barycentric_at_sample:
         /* We stored the sample in the fake "bary" dest. */
         ntt_INTERP_SAMPLE(c, ntt_get_dest(c, &instr->def), input,
                            ntt_get_src(c, instr->src[0]));
         break;

      case nir_intrinsic_load_barycentric_at_offset:
         /* We stored the offset in the fake "bary" dest. */
         ntt_INTERP_OFFSET(c, ntt_get_dest(c, &instr->def), input,
                            ntt_get_src(c, instr->src[0]));
         break;

      default:
         unreachable("bad barycentric interp intrinsic\n");
      }
      break;
   }

   default:
      unreachable("bad load input intrinsic\n");
   }
}

static void
ntt_emit_store_output(struct ntt_compile *c, nir_intrinsic_instr *instr)
{
   struct ureg_src src = ntt_get_src(c, instr->src[0]);

   if (src.File == TGSI_FILE_OUTPUT) {
      /* If our src is the output file, that's an indication that we were able
       * to emit the output stores in the generating instructions and we have
       * nothing to do here.
       */
      return;
   }

   uint32_t frac;
   struct ureg_dst out = ntt_output_decl(c, instr, &frac);

   if (instr->intrinsic == nir_intrinsic_store_per_vertex_output) {
      out = ntt_ureg_dst_indirect(c, out, instr->src[2]);
      out = ntt_ureg_dst_dimension_indirect(c, out, instr->src[1]);
   } else {
      out = ntt_ureg_dst_indirect(c, out, instr->src[1]);
   }

   uint8_t swizzle[4] = { 0, 0, 0, 0 };
   for (int i = frac; i <= 4; i++) {
      if (out.WriteMask & (1 << i))
         swizzle[i] = i - frac;
   }

   src = ureg_swizzle(src, swizzle[0], swizzle[1], swizzle[2], swizzle[3]);

   ntt_MOV(c, out, src);
}

static void
ntt_emit_load_output(struct ntt_compile *c, nir_intrinsic_instr *instr)
{
   nir_io_semantics semantics = nir_intrinsic_io_semantics(instr);

   /* ntt_try_store_in_tgsi_output() optimization is not valid if normal
    * load_output is present.
    */
   assert(c->s->info.stage != MESA_SHADER_VERTEX &&
          (c->s->info.stage != MESA_SHADER_FRAGMENT || semantics.fb_fetch_output));

   uint32_t frac;
   struct ureg_dst out = ntt_output_decl(c, instr, &frac);

   if (instr->intrinsic == nir_intrinsic_load_per_vertex_output) {
      out = ntt_ureg_dst_indirect(c, out, instr->src[1]);
      out = ntt_ureg_dst_dimension_indirect(c, out, instr->src[0]);
   } else {
      out = ntt_ureg_dst_indirect(c, out, instr->src[0]);
   }

   struct ureg_dst dst = ntt_get_dest(c, &instr->def);
   struct ureg_src out_src = ureg_src(out);

   /* Don't swizzling unavailable channels of the output in the writemasked-out
    * components. Avoids compile failures in virglrenderer with
    * TESS_LEVEL_INNER.
    */
   int fill_channel = ffs(dst.WriteMask) - 1;
   uint8_t swizzles[4] = { 0, 1, 2, 3 };
   for (int i = 0; i < 4; i++)
      if (!(dst.WriteMask & (1 << i)))
         swizzles[i] = fill_channel;
   out_src = ureg_swizzle(out_src, swizzles[0], swizzles[1], swizzles[2], swizzles[3]);

   if (semantics.fb_fetch_output)
      ntt_FBFETCH(c, dst, out_src);
   else
      ntt_MOV(c, dst, out_src);
}

static void
ntt_emit_load_sysval(struct ntt_compile *c, nir_intrinsic_instr *instr)
{
   gl_system_value sysval = nir_system_value_from_intrinsic(instr->intrinsic);
   enum tgsi_semantic semantic = tgsi_get_sysval_semantic(sysval);
   struct ureg_src sv = ureg_DECL_system_value(c->ureg, semantic, 0);

   /* virglrenderer doesn't like references to channels of the sysval that
    * aren't defined, even if they aren't really read.  (GLSL compile fails on
    * gl_NumWorkGroups.w, for example).
    */
   uint32_t write_mask = BITSET_MASK(instr->def.num_components);
   sv = ntt_swizzle_for_write_mask(sv, write_mask);

   /* TGSI and NIR define these intrinsics as always loading ints, but they can
    * still appear on hardware with non-native-integers fragment shaders using
    * the draw path (i915g).  In that case, having called nir_lower_int_to_float
    * means that we actually want floats instead.
    */
   if (!c->native_integers) {
      switch (instr->intrinsic) {
      case nir_intrinsic_load_vertex_id:
      case nir_intrinsic_load_instance_id:
         ntt_U2F(c, ntt_get_dest(c, &instr->def), sv);
         return;

      default:
         break;
      }
   }

   ntt_store(c, &instr->def, sv);
}

static void
ntt_emit_barrier(struct ntt_compile *c, nir_intrinsic_instr *intr)
{
   bool compute = gl_shader_stage_is_compute(c->s->info.stage);

   if (nir_intrinsic_memory_scope(intr) != SCOPE_NONE) {
      nir_variable_mode modes = nir_intrinsic_memory_modes(intr);
      unsigned membar = 0;

      if (modes & nir_var_image)
         membar |= TGSI_MEMBAR_SHADER_IMAGE;

      if (modes & nir_var_mem_shared)
         membar |= TGSI_MEMBAR_SHARED;

      /* Atomic counters are lowered to SSBOs, there's no NIR mode corresponding
       * exactly to atomics. Take the closest match.
       */
      if (modes & nir_var_mem_ssbo)
         membar |= TGSI_MEMBAR_SHADER_BUFFER | TGSI_MEMBAR_ATOMIC_BUFFER;

      if (modes & nir_var_mem_global)
         membar |= TGSI_MEMBAR_SHADER_BUFFER;

      /* Hack for virglrenderer: the GLSL specific memory barrier functions,
       * memoryBarrier{Buffer,Image,Shared,AtomicCounter}(), are only
       * available in compute shaders prior to GLSL 4.30.  In other stages,
       * it needs to use the full memoryBarrier().  It may be possible to
       * make them available via #extension directives in older versions,
       * but it's confusingly underspecified, and Mesa/virglrenderer don't
       * currently agree on how to do it.  So, just promote partial memory
       * barriers back to full ones outside of compute shaders when asked.
       */
      if (membar && !compute &&
          c->options->non_compute_membar_needs_all_modes) {
         membar |= TGSI_MEMBAR_SHADER_BUFFER |
                   TGSI_MEMBAR_ATOMIC_BUFFER |
                   TGSI_MEMBAR_SHADER_IMAGE |
                   TGSI_MEMBAR_SHARED;
      }

      /* If we only need workgroup scope (not device-scope), we might be able to
       * optimize a bit.
       */
      if (membar && compute &&
          nir_intrinsic_memory_scope(intr) == SCOPE_WORKGROUP) {

         membar |= TGSI_MEMBAR_THREAD_GROUP;
      }

      /* Only emit a memory barrier if there are any relevant modes */
      if (membar)
         ntt_MEMBAR(c, ureg_imm1u(c->ureg, membar));
   }

   if (nir_intrinsic_execution_scope(intr) != SCOPE_NONE) {
      assert(compute || c->s->info.stage == MESA_SHADER_TESS_CTRL);
      ntt_BARRIER(c);
   }
}

static void
ntt_emit_intrinsic(struct ntt_compile *c, nir_intrinsic_instr *instr)
{
   switch (instr->intrinsic) {
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_ubo_vec4:
      ntt_emit_load_ubo(c, instr);
      break;

      /* Vertex */
   case nir_intrinsic_load_vertex_id:
   case nir_intrinsic_load_vertex_id_zero_base:
   case nir_intrinsic_load_base_vertex:
   case nir_intrinsic_load_base_instance:
   case nir_intrinsic_load_instance_id:
   case nir_intrinsic_load_draw_id:
   case nir_intrinsic_load_invocation_id:
   case nir_intrinsic_load_frag_coord:
   case nir_intrinsic_load_point_coord:
   case nir_intrinsic_load_front_face:
   case nir_intrinsic_load_sample_id:
   case nir_intrinsic_load_sample_pos:
   case nir_intrinsic_load_sample_mask_in:
   case nir_intrinsic_load_helper_invocation:
   case nir_intrinsic_load_tess_coord:
   case nir_intrinsic_load_patch_vertices_in:
   case nir_intrinsic_load_primitive_id:
   case nir_intrinsic_load_tess_level_outer:
   case nir_intrinsic_load_tess_level_inner:
   case nir_intrinsic_load_local_invocation_id:
   case nir_intrinsic_load_workgroup_id:
   case nir_intrinsic_load_num_workgroups:
   case nir_intrinsic_load_workgroup_size:
   case nir_intrinsic_load_subgroup_size:
   case nir_intrinsic_load_subgroup_invocation:
   case nir_intrinsic_load_subgroup_eq_mask:
   case nir_intrinsic_load_subgroup_ge_mask:
   case nir_intrinsic_load_subgroup_gt_mask:
   case nir_intrinsic_load_subgroup_lt_mask:
   case nir_intrinsic_load_subgroup_le_mask:
      ntt_emit_load_sysval(c, instr);
      break;

   case nir_intrinsic_load_input:
   case nir_intrinsic_load_per_vertex_input:
   case nir_intrinsic_load_interpolated_input:
      ntt_emit_load_input(c, instr);
      break;

   case nir_intrinsic_store_output:
   case nir_intrinsic_store_per_vertex_output:
      ntt_emit_store_output(c, instr);
      break;

   case nir_intrinsic_load_output:
   case nir_intrinsic_load_per_vertex_output:
      ntt_emit_load_output(c, instr);
      break;

   case nir_intrinsic_demote:
      ntt_DEMOTE(c);
      break;

   case nir_intrinsic_discard:
      ntt_KILL(c);
      break;

   case nir_intrinsic_discard_if: {
      struct ureg_src cond = ureg_scalar(ntt_get_src(c, instr->src[0]), 0);

      if (c->native_integers) {
         struct ureg_dst temp = ureg_writemask(ntt_temp(c), 1);
         ntt_AND(c, temp, cond, ureg_imm1f(c->ureg, 1.0));
         ntt_KILL_IF(c, ureg_scalar(ureg_negate(ureg_src(temp)), 0));
      } else {
         /* For !native_integers, the bool got lowered to 1.0 or 0.0. */
         ntt_KILL_IF(c, ureg_negate(cond));
      }
      break;
   }

   case nir_intrinsic_is_helper_invocation:
      ntt_READ_HELPER(c, ntt_get_dest(c, &instr->def));
      break;

   case nir_intrinsic_vote_all:
      ntt_VOTE_ALL(c, ntt_get_dest(c, &instr->def), ntt_get_src(c,instr->src[0]));
      return;
   case nir_intrinsic_vote_any:
      ntt_VOTE_ANY(c, ntt_get_dest(c, &instr->def), ntt_get_src(c, instr->src[0]));
      return;
   case nir_intrinsic_vote_ieq:
      ntt_VOTE_EQ(c, ntt_get_dest(c, &instr->def), ntt_get_src(c, instr->src[0]));
      return;
   case nir_intrinsic_ballot:
      ntt_BALLOT(c, ntt_get_dest(c, &instr->def), ntt_get_src(c, instr->src[0]));
      return;
   case nir_intrinsic_read_first_invocation:
      ntt_READ_FIRST(c, ntt_get_dest(c, &instr->def), ntt_get_src(c, instr->src[0]));
      return;
   case nir_intrinsic_read_invocation:
      ntt_READ_INVOC(c, ntt_get_dest(c, &instr->def), ntt_get_src(c, instr->src[0]), ntt_get_src(c, instr->src[1]));
      return;

   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_store_ssbo:
   case nir_intrinsic_ssbo_atomic:
   case nir_intrinsic_ssbo_atomic_swap:
   case nir_intrinsic_get_ssbo_size:
      ntt_emit_mem(c, instr, nir_var_mem_ssbo);
      break;

   case nir_intrinsic_load_shared:
   case nir_intrinsic_store_shared:
   case nir_intrinsic_shared_atomic:
   case nir_intrinsic_shared_atomic_swap:
      ntt_emit_mem(c, instr, nir_var_mem_shared);
      break;

   case nir_intrinsic_atomic_counter_read:
   case nir_intrinsic_atomic_counter_add:
   case nir_intrinsic_atomic_counter_inc:
   case nir_intrinsic_atomic_counter_post_dec:
   case nir_intrinsic_atomic_counter_min:
   case nir_intrinsic_atomic_counter_max:
   case nir_intrinsic_atomic_counter_and:
   case nir_intrinsic_atomic_counter_or:
   case nir_intrinsic_atomic_counter_xor:
   case nir_intrinsic_atomic_counter_exchange:
   case nir_intrinsic_atomic_counter_comp_swap:
      ntt_emit_mem(c, instr, nir_var_uniform);
      break;
   case nir_intrinsic_atomic_counter_pre_dec:
      unreachable("Should be lowered by ntt_lower_atomic_pre_dec()");
      break;

   case nir_intrinsic_image_load:
   case nir_intrinsic_image_store:
   case nir_intrinsic_image_size:
   case nir_intrinsic_image_samples:
   case nir_intrinsic_image_atomic:
   case nir_intrinsic_image_atomic_swap:
   case nir_intrinsic_bindless_image_load:
   case nir_intrinsic_bindless_image_store:
   case nir_intrinsic_bindless_image_size:
   case nir_intrinsic_bindless_image_samples:
   case nir_intrinsic_bindless_image_atomic:
   case nir_intrinsic_bindless_image_atomic_swap:
      ntt_emit_image_load_store(c, instr);
      break;

   case nir_intrinsic_barrier:
      ntt_emit_barrier(c, instr);
      break;

   case nir_intrinsic_end_primitive:
      ntt_ENDPRIM(c, ureg_imm1u(c->ureg, nir_intrinsic_stream_id(instr)));
      break;

   case nir_intrinsic_emit_vertex:
      ntt_EMIT(c, ureg_imm1u(c->ureg, nir_intrinsic_stream_id(instr)));
      break;

      /* In TGSI we don't actually generate the barycentric coords, and emit
       * interp intrinsics later.  However, we do need to store the
       * load_barycentric_at_* argument so that we can use it at that point.
       */
   case nir_intrinsic_load_barycentric_pixel:
   case nir_intrinsic_load_barycentric_centroid:
   case nir_intrinsic_load_barycentric_sample:
      break;
   case nir_intrinsic_load_barycentric_at_sample:
   case nir_intrinsic_load_barycentric_at_offset:
      ntt_store(c, &instr->def, ntt_get_src(c, instr->src[0]));
      break;

   case nir_intrinsic_shader_clock:
      ntt_CLOCK(c, ntt_get_dest(c, &instr->def));
      break;

   case nir_intrinsic_decl_reg:
   case nir_intrinsic_load_reg:
   case nir_intrinsic_load_reg_indirect:
   case nir_intrinsic_store_reg:
   case nir_intrinsic_store_reg_indirect:
      /* fully consumed */
      break;

   default:
      fprintf(stderr, "Unknown intrinsic: ");
      nir_print_instr(&instr->instr, stderr);
      fprintf(stderr, "\n");
      break;
   }
}

struct ntt_tex_operand_state {
   struct ureg_src srcs[4];
   unsigned i;
};

static void
ntt_push_tex_arg(struct ntt_compile *c,
                 nir_tex_instr *instr,
                 nir_tex_src_type tex_src_type,
                 struct ntt_tex_operand_state *s)
{
   int tex_src = nir_tex_instr_src_index(instr, tex_src_type);
   if (tex_src < 0)
      return;

   nir_src *src = &instr->src[tex_src].src;

   /* virglrenderer workaround that's hard to do in tgsi_translate: Make sure
    * that TG4's immediate offset arg is float-typed.
    */
   if (instr->op == nir_texop_tg4 && tex_src_type == nir_tex_src_backend2 &&
       nir_src_is_const(*src)) {
      nir_const_value *consts = nir_src_as_const_value(*src);
      s->srcs[s->i++] = ureg_imm4f(c->ureg,
                                   consts[0].f32,
                                   consts[1].f32,
                                   consts[2].f32,
                                   consts[3].f32);
      return;
   }

   s->srcs[s->i++] = ntt_get_src(c, *src);
}

static void
ntt_emit_texture(struct ntt_compile *c, nir_tex_instr *instr)
{
   struct ureg_dst dst = ntt_get_dest(c, &instr->def);
   enum tgsi_texture_type target = tgsi_texture_type_from_sampler_dim(instr->sampler_dim, instr->is_array, instr->is_shadow);
   unsigned tex_opcode;

   int tex_handle_src = nir_tex_instr_src_index(instr, nir_tex_src_texture_handle);
   int sampler_handle_src = nir_tex_instr_src_index(instr, nir_tex_src_sampler_handle);

   struct ureg_src sampler;
   if (tex_handle_src >= 0 && sampler_handle_src >= 0) {
      /* It seems we can't get separate tex/sampler on GL, just use one of the handles */
      sampler = ntt_get_src(c, instr->src[tex_handle_src].src);
      assert(nir_tex_instr_src_index(instr, nir_tex_src_sampler_offset) == -1);
   } else {
      assert(tex_handle_src == -1 && sampler_handle_src == -1);
      sampler = ureg_DECL_sampler(c->ureg, instr->sampler_index);
      int sampler_src = nir_tex_instr_src_index(instr, nir_tex_src_sampler_offset);
      if (sampler_src >= 0) {
         struct ureg_src reladdr = ntt_get_src(c, instr->src[sampler_src].src);
         sampler = ureg_src_indirect(sampler, ntt_reladdr(c, reladdr, 2));
      }
   }

   switch (instr->op) {
   case nir_texop_tex:
      if (nir_tex_instr_src_size(instr, nir_tex_instr_src_index(instr, nir_tex_src_backend1)) >
         MAX2(instr->coord_components, 2) + instr->is_shadow)
         tex_opcode = TGSI_OPCODE_TXP;
      else
         tex_opcode = TGSI_OPCODE_TEX;
      break;
   case nir_texop_txf:
   case nir_texop_txf_ms:
      tex_opcode = TGSI_OPCODE_TXF;

      if (c->has_txf_lz) {
         int lod_src = nir_tex_instr_src_index(instr, nir_tex_src_lod);
         if (lod_src >= 0 &&
             nir_src_is_const(instr->src[lod_src].src) &&
             ntt_src_as_uint(c, instr->src[lod_src].src) == 0) {
            tex_opcode = TGSI_OPCODE_TXF_LZ;
         }
      }
      break;
   case nir_texop_txl:
      tex_opcode = TGSI_OPCODE_TXL;
      break;
   case nir_texop_txb:
      tex_opcode = TGSI_OPCODE_TXB;
      break;
   case nir_texop_txd:
      tex_opcode = TGSI_OPCODE_TXD;
      break;
   case nir_texop_txs:
      tex_opcode = TGSI_OPCODE_TXQ;
      break;
   case nir_texop_tg4:
      tex_opcode = TGSI_OPCODE_TG4;
      break;
   case nir_texop_query_levels:
      tex_opcode = TGSI_OPCODE_TXQ;
      break;
   case nir_texop_lod:
      tex_opcode = TGSI_OPCODE_LODQ;
      break;
   case nir_texop_texture_samples:
      tex_opcode = TGSI_OPCODE_TXQS;
      break;
   default:
      unreachable("unsupported tex op");
   }

   struct ntt_tex_operand_state s = { .i = 0 };
   ntt_push_tex_arg(c, instr, nir_tex_src_backend1, &s);
   ntt_push_tex_arg(c, instr, nir_tex_src_backend2, &s);

   /* non-coord arg for TXQ */
   if (tex_opcode == TGSI_OPCODE_TXQ) {
      ntt_push_tex_arg(c, instr, nir_tex_src_lod, &s);
      /* virglrenderer mistakenly looks at .w instead of .x, so make sure it's
       * scalar
       */
      s.srcs[s.i - 1] = ureg_scalar(s.srcs[s.i - 1], 0);
   }

   if (s.i > 1) {
      if (tex_opcode == TGSI_OPCODE_TEX)
         tex_opcode = TGSI_OPCODE_TEX2;
      if (tex_opcode == TGSI_OPCODE_TXB)
         tex_opcode = TGSI_OPCODE_TXB2;
      if (tex_opcode == TGSI_OPCODE_TXL)
         tex_opcode = TGSI_OPCODE_TXL2;
   }

   if (instr->op == nir_texop_txd) {
      /* Derivs appear in their own src args */
      int ddx = nir_tex_instr_src_index(instr, nir_tex_src_ddx);
      int ddy = nir_tex_instr_src_index(instr, nir_tex_src_ddy);
      s.srcs[s.i++] = ntt_get_src(c, instr->src[ddx].src);
      s.srcs[s.i++] = ntt_get_src(c, instr->src[ddy].src);
   }

   if (instr->op == nir_texop_tg4 && target != TGSI_TEXTURE_SHADOWCUBE_ARRAY) {
      if (c->screen->get_param(c->screen,
                               PIPE_CAP_TGSI_TG4_COMPONENT_IN_SWIZZLE)) {
         sampler = ureg_scalar(sampler, instr->component);
         s.srcs[s.i++] = ureg_src_undef();
      } else {
         s.srcs[s.i++] = ureg_imm1u(c->ureg, instr->component);
      }
   }

   s.srcs[s.i++] = sampler;

   enum tgsi_return_type tex_type;
   switch (instr->dest_type) {
   case nir_type_float32:
      tex_type = TGSI_RETURN_TYPE_FLOAT;
      break;
   case nir_type_int32:
      tex_type = TGSI_RETURN_TYPE_SINT;
      break;
   case nir_type_uint32:
      tex_type = TGSI_RETURN_TYPE_UINT;
      break;
   default:
      unreachable("unknown texture type");
   }

   struct ureg_dst tex_dst;
   if (instr->op == nir_texop_query_levels)
      tex_dst = ureg_writemask(ntt_temp(c), TGSI_WRITEMASK_W);
   else
      tex_dst = dst;

   while (s.i < 4)
      s.srcs[s.i++] = ureg_src_undef();

   struct ntt_insn *insn = ntt_insn(c, tex_opcode, tex_dst, s.srcs[0], s.srcs[1], s.srcs[2], s.srcs[3]);
   insn->tex_target = target;
   insn->tex_return_type = tex_type;
   insn->is_tex = true;

   int tex_offset_src = nir_tex_instr_src_index(instr, nir_tex_src_offset);
   if (tex_offset_src >= 0) {
      struct ureg_src offset = ntt_get_src(c, instr->src[tex_offset_src].src);

      insn->tex_offset[0].File = offset.File;
      insn->tex_offset[0].Index = offset.Index;
      insn->tex_offset[0].SwizzleX = offset.SwizzleX;
      insn->tex_offset[0].SwizzleY = offset.SwizzleY;
      insn->tex_offset[0].SwizzleZ = offset.SwizzleZ;
      insn->tex_offset[0].Padding = 0;
   }

   if (nir_tex_instr_has_explicit_tg4_offsets(instr)) {
      for (uint8_t i = 0; i < 4; ++i) {
         struct ureg_src imm = ureg_imm2i(c->ureg, instr->tg4_offsets[i][0], instr->tg4_offsets[i][1]);
         insn->tex_offset[i].File = imm.File;
         insn->tex_offset[i].Index = imm.Index;
         insn->tex_offset[i].SwizzleX = imm.SwizzleX;
         insn->tex_offset[i].SwizzleY = imm.SwizzleY;
         insn->tex_offset[i].SwizzleZ = imm.SwizzleZ;
      }
   }

   if (instr->op == nir_texop_query_levels)
      ntt_MOV(c, dst, ureg_scalar(ureg_src(tex_dst), 3));
}

static void
ntt_emit_jump(struct ntt_compile *c, nir_jump_instr *jump)
{
   switch (jump->type) {
   case nir_jump_break:
      ntt_BRK(c);
      break;

   case nir_jump_continue:
      ntt_CONT(c);
      break;

   default:
      fprintf(stderr, "Unknown jump instruction: ");
      nir_print_instr(&jump->instr, stderr);
      fprintf(stderr, "\n");
      abort();
   }
}

static void
ntt_emit_ssa_undef(struct ntt_compile *c, nir_undef_instr *instr)
{
   /* Nothing to do but make sure that we have some storage to deref. */
   (void)ntt_get_ssa_def_decl(c, &instr->def);
}

static void
ntt_emit_instr(struct ntt_compile *c, nir_instr *instr)
{
   switch (instr->type) {
   case nir_instr_type_deref:
      /* ignored, will be walked by nir_intrinsic_image_*_deref. */
      break;

   case nir_instr_type_alu:
      ntt_emit_alu(c, nir_instr_as_alu(instr));
      break;

   case nir_instr_type_intrinsic:
      ntt_emit_intrinsic(c, nir_instr_as_intrinsic(instr));
      break;

   case nir_instr_type_load_const:
      /* Nothing to do here, as load consts are done directly from
       * ntt_get_src() (since many constant NIR srcs will often get folded
       * directly into a register file index instead of as a TGSI src).
       */
      break;

   case nir_instr_type_tex:
      ntt_emit_texture(c, nir_instr_as_tex(instr));
      break;

   case nir_instr_type_jump:
      ntt_emit_jump(c, nir_instr_as_jump(instr));
      break;

   case nir_instr_type_undef:
      ntt_emit_ssa_undef(c, nir_instr_as_undef(instr));
      break;

   default:
      fprintf(stderr, "Unknown NIR instr type: ");
      nir_print_instr(instr, stderr);
      fprintf(stderr, "\n");
      abort();
   }
}

static void
ntt_emit_if(struct ntt_compile *c, nir_if *if_stmt)
{
   if (c->native_integers)
      ntt_UIF(c, c->if_cond);
   else
      ntt_IF(c, c->if_cond);

   ntt_emit_cf_list(c, &if_stmt->then_list);

   if (!nir_cf_list_is_empty_block(&if_stmt->else_list)) {
      ntt_ELSE(c);
      ntt_emit_cf_list(c, &if_stmt->else_list);
   }

   ntt_ENDIF(c);
}

static void
ntt_emit_loop(struct ntt_compile *c, nir_loop *loop)
{
   assert(!nir_loop_has_continue_construct(loop));
   ntt_BGNLOOP(c);
   ntt_emit_cf_list(c, &loop->body);
   ntt_ENDLOOP(c);
}

static void
ntt_emit_block(struct ntt_compile *c, nir_block *block)
{
   struct ntt_block *ntt_block = ntt_block_from_nir(c, block);
   c->cur_block = ntt_block;

   nir_foreach_instr(instr, block) {
      ntt_emit_instr(c, instr);

      /* Sanity check that we didn't accidentally ureg_OPCODE() instead of ntt_OPCODE(). */
      if (ureg_get_instruction_number(c->ureg) != 0) {
         fprintf(stderr, "Emitted ureg insn during: ");
         nir_print_instr(instr, stderr);
         fprintf(stderr, "\n");
         unreachable("emitted ureg insn");
      }
   }

   /* Set up the if condition for ntt_emit_if(), which we have to do before
    * freeing up the temps (the "if" is treated as inside the block for liveness
    * purposes, despite not being an instruction)
    *
    * Note that, while IF and UIF are supposed to look at only .x, virglrenderer
    * looks at all of .xyzw.  No harm in working around the bug.
    */
   nir_if *nif = nir_block_get_following_if(block);
   if (nif)
      c->if_cond = ureg_scalar(ntt_get_src(c, nif->condition), TGSI_SWIZZLE_X);
}

static void
ntt_emit_cf_list(struct ntt_compile *c, struct exec_list *list)
{
   foreach_list_typed(nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block:
         ntt_emit_block(c, nir_cf_node_as_block(node));
         break;

      case nir_cf_node_if:
         ntt_emit_if(c, nir_cf_node_as_if(node));
         break;

      case nir_cf_node_loop:
         ntt_emit_loop(c, nir_cf_node_as_loop(node));
         break;

      default:
         unreachable("unknown CF type");
      }
   }
}

static void
ntt_emit_block_ureg(struct ntt_compile *c, struct nir_block *block)
{
   struct ntt_block *ntt_block = ntt_block_from_nir(c, block);

   /* Emit the ntt insns to tgsi_ureg. */
   util_dynarray_foreach(&ntt_block->insns, struct ntt_insn, insn) {
      const struct tgsi_opcode_info *opcode_info =
         tgsi_get_opcode_info(insn->opcode);

      switch (insn->opcode) {
      case TGSI_OPCODE_UIF:
         ureg_UIF(c->ureg, insn->src[0], &c->cf_label);
         break;

      case TGSI_OPCODE_IF:
         ureg_IF(c->ureg, insn->src[0], &c->cf_label);
         break;

      case TGSI_OPCODE_ELSE:
         ureg_fixup_label(c->ureg, c->current_if_else, ureg_get_instruction_number(c->ureg));
         ureg_ELSE(c->ureg, &c->cf_label);
         c->current_if_else = c->cf_label;
         break;

      case TGSI_OPCODE_ENDIF:
         ureg_fixup_label(c->ureg, c->current_if_else, ureg_get_instruction_number(c->ureg));
         ureg_ENDIF(c->ureg);
         break;

      case TGSI_OPCODE_BGNLOOP:
         /* GLSL-to-TGSI never set the begin/end labels to anything, even though nvfx
          * does reference BGNLOOP's.  Follow the former behavior unless something comes up
          * with a need.
          */
         ureg_BGNLOOP(c->ureg, &c->cf_label);
         break;

      case TGSI_OPCODE_ENDLOOP:
         ureg_ENDLOOP(c->ureg, &c->cf_label);
         break;

      default:
         if (insn->is_tex) {
            int num_offsets = 0;
            for (int i = 0; i < ARRAY_SIZE(insn->tex_offset); i++) {
               if (insn->tex_offset[i].File != TGSI_FILE_NULL)
                  num_offsets = i + 1;
            }
            ureg_tex_insn(c->ureg, insn->opcode,
                          insn->dst, opcode_info->num_dst,
                          insn->tex_target, insn->tex_return_type,
                          insn->tex_offset,
                          num_offsets,
                          insn->src, opcode_info->num_src);
         } else if (insn->is_mem) {
            ureg_memory_insn(c->ureg, insn->opcode,
                             insn->dst, opcode_info->num_dst,
                             insn->src, opcode_info->num_src,
                             insn->mem_qualifier,
                             insn->tex_target,
                             insn->mem_format);
         } else {
            ureg_insn(c->ureg, insn->opcode,
                     insn->dst, opcode_info->num_dst,
                     insn->src, opcode_info->num_src,
                     insn->precise);
         }
      }
   }
}

static void
ntt_emit_if_ureg(struct ntt_compile *c, nir_if *if_stmt)
{
   /* Note: the last block emitted our IF opcode. */

   int if_stack = c->current_if_else;
   c->current_if_else = c->cf_label;

   /* Either the then or else block includes the ENDIF, which will fix up the
    * IF(/ELSE)'s label for jumping
    */
   ntt_emit_cf_list_ureg(c, &if_stmt->then_list);
   ntt_emit_cf_list_ureg(c, &if_stmt->else_list);

   c->current_if_else = if_stack;
}

static void
ntt_emit_cf_list_ureg(struct ntt_compile *c, struct exec_list *list)
{
   foreach_list_typed(nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block:
         ntt_emit_block_ureg(c, nir_cf_node_as_block(node));
         break;

      case nir_cf_node_if:
         ntt_emit_if_ureg(c, nir_cf_node_as_if(node));
         break;

      case nir_cf_node_loop:
         /* GLSL-to-TGSI never set the begin/end labels to anything, even though nvfx
          * does reference BGNLOOP's.  Follow the former behavior unless something comes up
          * with a need.
          */
         ntt_emit_cf_list_ureg(c, &nir_cf_node_as_loop(node)->body);
         break;

      default:
         unreachable("unknown CF type");
      }
   }
}

static void
ntt_emit_impl(struct ntt_compile *c, nir_function_impl *impl)
{
   c->impl = impl;

   c->ssa_temp = rzalloc_array(c, struct ureg_src, impl->ssa_alloc);
   c->reg_temp = rzalloc_array(c, struct ureg_dst, impl->ssa_alloc);

   /* Set up the struct ntt_blocks to put insns in */
   c->blocks = _mesa_pointer_hash_table_create(c);
   nir_foreach_block(block, impl) {
      struct ntt_block *ntt_block = rzalloc(c->blocks, struct ntt_block);
      util_dynarray_init(&ntt_block->insns, ntt_block);
      _mesa_hash_table_insert(c->blocks, block, ntt_block);
   }


   ntt_setup_registers(c);

   c->cur_block = ntt_block_from_nir(c, nir_start_block(impl));
   ntt_setup_inputs(c);
   ntt_setup_outputs(c);
   ntt_setup_uniforms(c);

   /* Emit the ntt insns */
   ntt_emit_cf_list(c, &impl->body);

   /* Don't do optimized RA if the driver requests it, unless the number of
    * temps is too large to be covered by the 16 bit signed int that TGSI
    * allocates for the register index */
   if (!c->options->unoptimized_ra || c->num_temps > 0x7fff)
      ntt_allocate_regs(c, impl);
   else
      ntt_allocate_regs_unoptimized(c, impl);

   /* Turn the ntt insns into actual TGSI tokens */
   ntt_emit_cf_list_ureg(c, &impl->body);

   ralloc_free(c->liveness);
   c->liveness = NULL;

}

static int
type_size(const struct glsl_type *type, bool bindless)
{
   return glsl_count_attribute_slots(type, false);
}

/* Allow vectorizing of ALU instructions, but avoid vectorizing past what we
 * can handle for 64-bit values in TGSI.
 */
static uint8_t
ntt_should_vectorize_instr(const nir_instr *instr, const void *data)
{
   if (instr->type != nir_instr_type_alu)
      return 0;

   nir_alu_instr *alu = nir_instr_as_alu(instr);

   switch (alu->op) {
   case nir_op_ibitfield_extract:
   case nir_op_ubitfield_extract:
   case nir_op_bitfield_insert:
      /* virglrenderer only looks at the .x channel of the offset/bits operands
       * when translating to GLSL.  tgsi.rst doesn't seem to require scalar
       * offset/bits operands.
       *
       * https://gitlab.freedesktop.org/virgl/virglrenderer/-/issues/195
       */
      return 1;

   default:
      break;
   }

   int src_bit_size = nir_src_bit_size(alu->src[0].src);
   int dst_bit_size = alu->def.bit_size;

   if (src_bit_size == 64 || dst_bit_size == 64) {
      /* Avoid vectorizing 64-bit instructions at all.  Despite tgsi.rst
       * claiming support, virglrenderer generates bad shaders on the host when
       * presented with them.  Maybe we can make virgl avoid tickling the
       * virglrenderer bugs, but given that glsl-to-TGSI didn't generate vector
       * 64-bit instrs in the first place, I don't see much reason to care about
       * this.
       */
      return 1;
   }

   return 4;
}

static bool
ntt_should_vectorize_io(unsigned align, unsigned bit_size,
                        unsigned num_components, unsigned high_offset,
                        nir_intrinsic_instr *low, nir_intrinsic_instr *high,
                        void *data)
{
   if (bit_size != 32)
      return false;

   /* Our offset alignment should aways be at least 4 bytes */
   if (align < 4)
      return false;

   /* No wrapping off the end of a TGSI reg.  We could do a bit better by
    * looking at low's actual offset.  XXX: With LOAD_CONSTBUF maybe we don't
    * need this restriction.
    */
   unsigned worst_start_component = align == 4 ? 3 : align / 4;
   if (worst_start_component + num_components > 4)
      return false;

   return true;
}

static nir_variable_mode
ntt_no_indirects_mask(nir_shader *s, struct pipe_screen *screen)
{
   unsigned pipe_stage = pipe_shader_type_from_mesa(s->info.stage);
   unsigned indirect_mask = 0;

   if (!screen->get_shader_param(screen, pipe_stage,
                                 PIPE_SHADER_CAP_INDIRECT_INPUT_ADDR)) {
      indirect_mask |= nir_var_shader_in;
   }

   if (!screen->get_shader_param(screen, pipe_stage,
                                 PIPE_SHADER_CAP_INDIRECT_OUTPUT_ADDR)) {
      indirect_mask |= nir_var_shader_out;
   }

   if (!screen->get_shader_param(screen, pipe_stage,
                                 PIPE_SHADER_CAP_INDIRECT_TEMP_ADDR)) {
      indirect_mask |= nir_var_function_temp;
   }

   return indirect_mask;
}

static void
ntt_optimize_nir(struct nir_shader *s, struct pipe_screen *screen,
                 const struct nir_to_tgsi_options *options)
{
   bool progress;
   unsigned pipe_stage = pipe_shader_type_from_mesa(s->info.stage);
   unsigned control_flow_depth =
      screen->get_shader_param(screen, pipe_stage,
                               PIPE_SHADER_CAP_MAX_CONTROL_FLOW_DEPTH);
   do {
      progress = false;

      NIR_PASS_V(s, nir_lower_vars_to_ssa);
      NIR_PASS_V(s, nir_split_64bit_vec3_and_vec4);

      NIR_PASS(progress, s, nir_copy_prop);
      NIR_PASS(progress, s, nir_opt_algebraic);
      NIR_PASS(progress, s, nir_opt_constant_folding);
      NIR_PASS(progress, s, nir_opt_remove_phis);
      NIR_PASS(progress, s, nir_opt_conditional_discard);
      NIR_PASS(progress, s, nir_opt_dce);
      NIR_PASS(progress, s, nir_opt_dead_cf);
      NIR_PASS(progress, s, nir_opt_cse);
      NIR_PASS(progress, s, nir_opt_find_array_copies);
      NIR_PASS(progress, s, nir_opt_copy_prop_vars);
      NIR_PASS(progress, s, nir_opt_dead_write_vars);

      NIR_PASS(progress, s, nir_opt_if, nir_opt_if_optimize_phi_true_false);
      NIR_PASS(progress, s, nir_opt_peephole_select,
               control_flow_depth == 0 ? ~0 : 8, true, true);
      NIR_PASS(progress, s, nir_opt_algebraic);
      NIR_PASS(progress, s, nir_opt_constant_folding);
      nir_load_store_vectorize_options vectorize_opts = {
         .modes = nir_var_mem_ubo,
         .callback = ntt_should_vectorize_io,
         .robust_modes = 0,
      };
      NIR_PASS(progress, s, nir_opt_load_store_vectorize, &vectorize_opts);
      NIR_PASS(progress, s, nir_opt_shrink_stores, true);
      NIR_PASS(progress, s, nir_opt_shrink_vectors);
      NIR_PASS(progress, s, nir_opt_loop);
      NIR_PASS(progress, s, nir_opt_vectorize, ntt_should_vectorize_instr, NULL);
      NIR_PASS(progress, s, nir_opt_undef);
      NIR_PASS(progress, s, nir_opt_loop_unroll);

      /* Try to fold addressing math into ubo_vec4's base to avoid load_consts
       * and ALU ops for it.
       */
      nir_opt_offsets_options offset_options = {
         .ubo_vec4_max = ~0,

         /* No const offset in TGSI for shared accesses. */
         .shared_max = 0,

         /* unused intrinsics */
         .uniform_max = 0,
         .buffer_max = 0,
      };

      if (options->ubo_vec4_max)
         offset_options.ubo_vec4_max = options->ubo_vec4_max;

      NIR_PASS(progress, s, nir_opt_offsets, &offset_options);
   } while (progress);

   NIR_PASS_V(s, nir_lower_var_copies);
}

/* Scalarizes all 64-bit ALU ops.  Note that we only actually need to
 * scalarize vec3/vec4s, should probably fix that.
 */
static bool
scalarize_64bit(const nir_instr *instr, const void *data)
{
   const nir_alu_instr *alu = nir_instr_as_alu(instr);

   return (alu->def.bit_size == 64 ||
           nir_src_bit_size(alu->src[0].src) == 64);
}

static bool
nir_to_tgsi_lower_64bit_intrinsic(nir_builder *b, nir_intrinsic_instr *instr)
{
   b->cursor = nir_after_instr(&instr->instr);

   switch (instr->intrinsic) {
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_ubo_vec4:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_load_input:
   case nir_intrinsic_load_interpolated_input:
   case nir_intrinsic_load_per_vertex_input:
   case nir_intrinsic_store_output:
   case nir_intrinsic_store_per_vertex_output:
   case nir_intrinsic_store_ssbo:
      break;
   default:
      return false;
   }

   if (instr->num_components <= 2)
      return false;

   bool has_dest = nir_intrinsic_infos[instr->intrinsic].has_dest;
   if (has_dest) {
      if (instr->def.bit_size != 64)
         return false;
   } else  {
      if (nir_src_bit_size(instr->src[0]) != 64)
          return false;
   }

   nir_intrinsic_instr *first =
      nir_instr_as_intrinsic(nir_instr_clone(b->shader, &instr->instr));
   nir_intrinsic_instr *second =
      nir_instr_as_intrinsic(nir_instr_clone(b->shader, &instr->instr));

   switch (instr->intrinsic) {
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_ubo_vec4:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_store_ssbo:
      break;

   default: {
      nir_io_semantics semantics = nir_intrinsic_io_semantics(second);
      semantics.location++;
      semantics.num_slots--;
      nir_intrinsic_set_io_semantics(second, semantics);

      nir_intrinsic_set_base(second, nir_intrinsic_base(second) + 1);
      break;
   }
   }

   first->num_components = 2;
   second->num_components -= 2;
   if (has_dest) {
      first->def.num_components = 2;
      second->def.num_components -= 2;
   }

   nir_builder_instr_insert(b, &first->instr);
   nir_builder_instr_insert(b, &second->instr);

   if (has_dest) {
      /* Merge the two loads' results back into a vector. */
      nir_scalar channels[4] = {
         nir_get_scalar(&first->def, 0),
         nir_get_scalar(&first->def, 1),
         nir_get_scalar(&second->def, 0),
         nir_get_scalar(&second->def, second->num_components > 1 ? 1 : 0),
      };
      nir_def *new = nir_vec_scalars(b, channels, instr->num_components);
      nir_def_rewrite_uses(&instr->def, new);
   } else {
      /* Split the src value across the two stores. */
      b->cursor = nir_before_instr(&instr->instr);

      nir_def *src0 = instr->src[0].ssa;
      nir_scalar channels[4] = { 0 };
      for (int i = 0; i < instr->num_components; i++)
         channels[i] = nir_get_scalar(src0, i);

      nir_intrinsic_set_write_mask(first, nir_intrinsic_write_mask(instr) & 3);
      nir_intrinsic_set_write_mask(second, nir_intrinsic_write_mask(instr) >> 2);

      nir_src_rewrite(&first->src[0], nir_vec_scalars(b, channels, 2));
      nir_src_rewrite(&second->src[0],
                      nir_vec_scalars(b, &channels[2], second->num_components));
   }

   int offset_src = -1;
   uint32_t offset_amount = 16;

   switch (instr->intrinsic) {
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_load_ubo:
      offset_src = 1;
      break;
   case nir_intrinsic_load_ubo_vec4:
      offset_src = 1;
      offset_amount = 1;
      break;
   case nir_intrinsic_store_ssbo:
      offset_src = 2;
      break;
   default:
      break;
   }
   if (offset_src != -1) {
      b->cursor = nir_before_instr(&second->instr);
      nir_def *second_offset =
         nir_iadd_imm(b, second->src[offset_src].ssa, offset_amount);
      nir_src_rewrite(&second->src[offset_src], second_offset);
   }

   /* DCE stores we generated with no writemask (nothing else does this
    * currently).
    */
   if (!has_dest) {
      if (nir_intrinsic_write_mask(first) == 0)
         nir_instr_remove(&first->instr);
      if (nir_intrinsic_write_mask(second) == 0)
         nir_instr_remove(&second->instr);
   }

   nir_instr_remove(&instr->instr);

   return true;
}

static bool
nir_to_tgsi_lower_64bit_load_const(nir_builder *b, nir_load_const_instr *instr)
{
   int num_components = instr->def.num_components;

   if (instr->def.bit_size != 64 || num_components <= 2)
      return false;

   b->cursor = nir_before_instr(&instr->instr);

   nir_load_const_instr *first =
      nir_load_const_instr_create(b->shader, 2, 64);
   nir_load_const_instr *second =
      nir_load_const_instr_create(b->shader, num_components - 2, 64);

   first->value[0] = instr->value[0];
   first->value[1] = instr->value[1];
   second->value[0] = instr->value[2];
   if (num_components == 4)
      second->value[1] = instr->value[3];

   nir_builder_instr_insert(b, &first->instr);
   nir_builder_instr_insert(b, &second->instr);

   nir_def *channels[4] = {
      nir_channel(b, &first->def, 0),
      nir_channel(b, &first->def, 1),
      nir_channel(b, &second->def, 0),
      num_components == 4 ? nir_channel(b, &second->def, 1) : NULL,
   };
   nir_def *new = nir_vec(b, channels, num_components);
   nir_def_rewrite_uses(&instr->def, new);
   nir_instr_remove(&instr->instr);

   return true;
}

static bool
nir_to_tgsi_lower_64bit_to_vec2_instr(nir_builder *b, nir_instr *instr,
                                      void *data)
{
   switch (instr->type) {
   case nir_instr_type_load_const:
      return nir_to_tgsi_lower_64bit_load_const(b, nir_instr_as_load_const(instr));

   case nir_instr_type_intrinsic:
      return nir_to_tgsi_lower_64bit_intrinsic(b, nir_instr_as_intrinsic(instr));
   default:
      return false;
   }
}

static bool
nir_to_tgsi_lower_64bit_to_vec2(nir_shader *s)
{
   return nir_shader_instructions_pass(s,
                                       nir_to_tgsi_lower_64bit_to_vec2_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}

struct ntt_lower_tex_state {
   nir_scalar channels[8];
   unsigned i;
};

static void
nir_to_tgsi_lower_tex_instr_arg(nir_builder *b,
                                nir_tex_instr *instr,
                                nir_tex_src_type tex_src_type,
                                struct ntt_lower_tex_state *s)
{
   int tex_src = nir_tex_instr_src_index(instr, tex_src_type);
   if (tex_src < 0)
      return;

   nir_def *def = instr->src[tex_src].src.ssa;
   for (int i = 0; i < def->num_components; i++) {
      s->channels[s->i++] = nir_get_scalar(def, i);
   }

   nir_tex_instr_remove_src(instr, tex_src);
}

/**
 * Merges together a vec4 of tex coordinate/compare/bias/lod into a backend tex
 * src.  This lets NIR handle the coalescing of the vec4 rather than trying to
 * manage it on our own, and may lead to more vectorization.
 */
static bool
nir_to_tgsi_lower_tex_instr(nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);

   if (nir_tex_instr_src_index(tex, nir_tex_src_coord) < 0)
      return false;

   b->cursor = nir_before_instr(instr);

   struct ntt_lower_tex_state s = {0};

   nir_to_tgsi_lower_tex_instr_arg(b, tex, nir_tex_src_coord, &s);
   /* We always have at least two slots for the coordinate, even on 1D. */
   s.i = MAX2(s.i, 2);

   nir_to_tgsi_lower_tex_instr_arg(b, tex, nir_tex_src_comparator, &s);
   s.i = MAX2(s.i, 3);

   nir_to_tgsi_lower_tex_instr_arg(b, tex, nir_tex_src_bias, &s);

   /* XXX: LZ */
   nir_to_tgsi_lower_tex_instr_arg(b, tex, nir_tex_src_lod, &s);
   nir_to_tgsi_lower_tex_instr_arg(b, tex, nir_tex_src_projector, &s);
   nir_to_tgsi_lower_tex_instr_arg(b, tex, nir_tex_src_ms_index, &s);

   /* No need to pack undefs in unused channels of the tex instr */
   while (!s.channels[s.i - 1].def)
      s.i--;

   /* Instead of putting undefs in the unused slots of the vecs, just put in
    * another used channel.  Otherwise, we'll get unnecessary moves into
    * registers.
    */
   assert(s.channels[0].def != NULL);
   for (int i = 1; i < s.i; i++) {
      if (!s.channels[i].def)
         s.channels[i] = s.channels[0];
   }

   nir_tex_instr_add_src(tex, nir_tex_src_backend1,
                         nir_vec_scalars(b, s.channels, MIN2(s.i, 4)));
   if (s.i > 4)
      nir_tex_instr_add_src(tex, nir_tex_src_backend2,
                            nir_vec_scalars(b, &s.channels[4], s.i - 4));

   return true;
}

static bool
nir_to_tgsi_lower_tex(nir_shader *s)
{
   return nir_shader_instructions_pass(s,
                                       nir_to_tgsi_lower_tex_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}

static void
ntt_fix_nir_options(struct pipe_screen *screen, struct nir_shader *s,
                    const struct nir_to_tgsi_options *ntt_options)
{
   const struct nir_shader_compiler_options *options = s->options;
   bool lower_fsqrt =
      !screen->get_shader_param(screen, pipe_shader_type_from_mesa(s->info.stage),
                                PIPE_SHADER_CAP_TGSI_SQRT_SUPPORTED);

   bool force_indirect_unrolling_sampler =
      screen->get_param(screen, PIPE_CAP_GLSL_FEATURE_LEVEL) < 400;

   nir_variable_mode no_indirects_mask = ntt_no_indirects_mask(s, screen);

   if (!options->lower_extract_byte ||
       !options->lower_extract_word ||
       !options->lower_insert_byte ||
       !options->lower_insert_word ||
       !options->lower_fdph ||
       !options->lower_flrp64 ||
       !options->lower_fmod ||
       !options->lower_uadd_carry ||
       !options->lower_usub_borrow ||
       !options->lower_uadd_sat ||
       !options->lower_usub_sat ||
       !options->lower_uniforms_to_ubo ||
       !options->lower_vector_cmp ||
       options->has_rotate8 ||
       options->has_rotate16 ||
       options->has_rotate32 ||
       options->lower_fsqrt != lower_fsqrt ||
       options->force_indirect_unrolling != no_indirects_mask ||
       force_indirect_unrolling_sampler) {
      nir_shader_compiler_options *new_options = ralloc(s, nir_shader_compiler_options);
      *new_options = *s->options;

      new_options->lower_extract_byte = true;
      new_options->lower_extract_word = true;
      new_options->lower_insert_byte = true;
      new_options->lower_insert_word = true;
      new_options->lower_fdph = true;
      new_options->lower_flrp64 = true;
      new_options->lower_fmod = true;
      new_options->lower_uadd_carry = true;
      new_options->lower_usub_borrow = true;
      new_options->lower_uadd_sat = true;
      new_options->lower_usub_sat = true;
      new_options->lower_uniforms_to_ubo = true;
      new_options->lower_vector_cmp = true;
      new_options->lower_fsqrt = lower_fsqrt;
      new_options->has_rotate8 = false;
      new_options->has_rotate16 = false;
      new_options->has_rotate32 = false;
      new_options->force_indirect_unrolling = no_indirects_mask;
      new_options->force_indirect_unrolling_sampler = force_indirect_unrolling_sampler;

      s->options = new_options;
   }
}

static bool
ntt_lower_atomic_pre_dec_filter(const nir_instr *instr, const void *_data)
{
   return (instr->type == nir_instr_type_intrinsic &&
           nir_instr_as_intrinsic(instr)->intrinsic == nir_intrinsic_atomic_counter_pre_dec);
}

static nir_def *
ntt_lower_atomic_pre_dec_lower(nir_builder *b, nir_instr *instr, void *_data)
{
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   nir_def *old_result = &intr->def;
   intr->intrinsic = nir_intrinsic_atomic_counter_post_dec;

   return nir_iadd_imm(b, old_result, -1);
}

static bool
ntt_lower_atomic_pre_dec(nir_shader *s)
{
   return nir_shader_lower_instructions(s,
                                        ntt_lower_atomic_pre_dec_filter,
                                        ntt_lower_atomic_pre_dec_lower, NULL);
}

/* Lowers texture projectors if we can't do them as TGSI_OPCODE_TXP. */
static void
nir_to_tgsi_lower_txp(nir_shader *s)
{
   nir_lower_tex_options lower_tex_options = {
       .lower_txp = 0,
   };

   nir_foreach_block(block, nir_shader_get_entrypoint(s)) {
      nir_foreach_instr(instr, block) {
         if (instr->type != nir_instr_type_tex)
            continue;
         nir_tex_instr *tex = nir_instr_as_tex(instr);

         if (nir_tex_instr_src_index(tex, nir_tex_src_projector) < 0)
            continue;

         bool has_compare = nir_tex_instr_src_index(tex, nir_tex_src_comparator) >= 0;
         bool has_lod = nir_tex_instr_src_index(tex, nir_tex_src_lod) >= 0 || s->info.stage != MESA_SHADER_FRAGMENT;
         bool has_offset = nir_tex_instr_src_index(tex, nir_tex_src_offset) >= 0;

         /* We can do TXP for any tex (not txg) where we can fit all the
          * coordinates and comparator and projector in one vec4 without any
          * other modifiers to add on.
          *
          * nir_lower_tex() only handles the lowering on a sampler-dim basis, so
          * if we get any funny projectors then we just blow them all away.
          */
         if (tex->op != nir_texop_tex || has_lod || has_offset || (tex->coord_components >= 3 && has_compare))
            lower_tex_options.lower_txp |= 1 << tex->sampler_dim;
      }
   }

   /* nir_lower_tex must be run even if no options are set, because we need the
    * LOD to be set for query_levels and for non-fragment shaders.
    */
   NIR_PASS_V(s, nir_lower_tex, &lower_tex_options);
}

static bool
nir_lower_primid_sysval_to_input_filter(const nir_instr *instr, const void *_data)
{
   return (instr->type == nir_instr_type_intrinsic &&
           nir_instr_as_intrinsic(instr)->intrinsic == nir_intrinsic_load_primitive_id);
}

static nir_def *
nir_lower_primid_sysval_to_input_lower(nir_builder *b, nir_instr *instr, void *data)
{
   nir_variable *var = nir_get_variable_with_location(b->shader, nir_var_shader_in,
                                                      VARYING_SLOT_PRIMITIVE_ID, glsl_uint_type());

   nir_io_semantics semantics = {
      .location = var->data.location,
       .num_slots = 1
   };
   return nir_load_input(b, 1, 32, nir_imm_int(b, 0),
                         .base = var->data.driver_location,
                         .io_semantics = semantics);
}

static bool
nir_lower_primid_sysval_to_input(nir_shader *s)
{
   return nir_shader_lower_instructions(s,
                                        nir_lower_primid_sysval_to_input_filter,
                                        nir_lower_primid_sysval_to_input_lower, NULL);
}

const void *
nir_to_tgsi(struct nir_shader *s,
            struct pipe_screen *screen)
{
   static const struct nir_to_tgsi_options default_ntt_options = {0};
   return nir_to_tgsi_options(s, screen, &default_ntt_options);
}

/* Prevent lower_vec_to_mov from coalescing 64-to-32 conversions and comparisons
 * into unsupported channels of registers.
 */
static bool
ntt_vec_to_mov_writemask_cb(const nir_instr *instr, unsigned writemask, UNUSED const void *_data)
{
   if (instr->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(instr);
   int dst_32 = alu->def.bit_size == 32;
   int src_64 = nir_src_bit_size(alu->src[0].src) == 64;

   if (src_64 && dst_32) {
      int num_srcs = nir_op_infos[alu->op].num_inputs;

      if (num_srcs == 2 || nir_op_infos[alu->op].output_type == nir_type_bool32) {
         /* TGSI's 64 bit compares storing to 32-bit are weird and write .xz
          * instead of .xy.  Just support scalar compares storing to .x,
          * GLSL-to-TGSI only ever emitted scalar ops anyway.
          */
        if (writemask != TGSI_WRITEMASK_X)
           return false;
      } else {
         /* TGSI's 64-to-32-bit conversions can only store to .xy (since a TGSI
          * register can only store a dvec2).  Don't try to coalesce to write to
          * .zw.
          */
         if (writemask & ~(TGSI_WRITEMASK_XY))
            return false;
      }
   }

   return true;
}

/**
 * Translates the NIR shader to TGSI.
 *
 * This requires some lowering of the NIR shader to prepare it for translation.
 * We take ownership of the NIR shader passed, returning a reference to the new
 * TGSI tokens instead.  If you need to keep the NIR, then pass us a clone.
 */
const void *nir_to_tgsi_options(struct nir_shader *s,
                                struct pipe_screen *screen,
                                const struct nir_to_tgsi_options *options)
{
   struct ntt_compile *c;
   const void *tgsi_tokens;
   nir_variable_mode no_indirects_mask = ntt_no_indirects_mask(s, screen);
   bool native_integers = screen->get_shader_param(screen,
                                                   pipe_shader_type_from_mesa(s->info.stage),
                                                   PIPE_SHADER_CAP_INTEGERS);
   const struct nir_shader_compiler_options *original_options = s->options;

   ntt_fix_nir_options(screen, s, options);

   /* Lower array indexing on FS inputs.  Since we don't set
    * ureg->supports_any_inout_decl_range, the TGSI input decls will be split to
    * elements by ureg, and so dynamically indexing them would be invalid.
    * Ideally we would set that ureg flag based on
    * PIPE_SHADER_CAP_TGSI_ANY_INOUT_DECL_RANGE, but can't due to mesa/st
    * splitting NIR VS outputs to elements even if the FS doesn't get the
    * corresponding splitting, and virgl depends on TGSI across link boundaries
    * having matching declarations.
    */
   if (s->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS_V(s, nir_lower_indirect_derefs, nir_var_shader_in, UINT32_MAX);
      NIR_PASS_V(s, nir_remove_dead_variables, nir_var_shader_in, NULL);
   }

   /* Lower tesslevel indirect derefs for tessellation shader.
    * tesslevels are now a compact array variable and nir expects a constant
    * array index into the compact array variable.
    */
   if (s->info.stage == MESA_SHADER_TESS_CTRL ||
       s->info.stage == MESA_SHADER_TESS_EVAL) {
      NIR_PASS_V(s, nir_lower_indirect_derefs, 0 , UINT32_MAX);
   }

   NIR_PASS_V(s, nir_lower_io, nir_var_shader_in | nir_var_shader_out,
              type_size, (nir_lower_io_options)0);

   nir_to_tgsi_lower_txp(s);
   NIR_PASS_V(s, nir_to_tgsi_lower_tex);

   /* While TGSI can represent PRIMID as either an input or a system value,
    * glsl-to-tgsi had the GS (not TCS or TES) primid as an input, and drivers
    * depend on that.
    */
   if (s->info.stage == MESA_SHADER_GEOMETRY)
      NIR_PASS_V(s, nir_lower_primid_sysval_to_input);

   if (s->info.num_abos)
      NIR_PASS_V(s, ntt_lower_atomic_pre_dec);

   if (!original_options->lower_uniforms_to_ubo) {
      NIR_PASS_V(s, nir_lower_uniforms_to_ubo,
                 screen->get_param(screen, PIPE_CAP_PACKED_UNIFORMS),
                 !native_integers);
   }

   /* Do lowering so we can directly translate f64/i64 NIR ALU ops to TGSI --
    * TGSI stores up to a vec2 in each slot, so to avoid a whole bunch of op
    * duplication logic we just make it so that we only see vec2s.
    */
   NIR_PASS_V(s, nir_lower_alu_to_scalar, scalarize_64bit, NULL);
   NIR_PASS_V(s, nir_to_tgsi_lower_64bit_to_vec2);

   if (!screen->get_param(screen, PIPE_CAP_LOAD_CONSTBUF))
      NIR_PASS_V(s, nir_lower_ubo_vec4);

   ntt_optimize_nir(s, screen, options);

   NIR_PASS_V(s, nir_lower_indirect_derefs, no_indirects_mask, UINT32_MAX);

   /* Lower demote_if to if (cond) { demote } because TGSI doesn't have a DEMOTE_IF. */
   NIR_PASS_V(s, nir_lower_discard_if, nir_lower_demote_if_to_cf);

   NIR_PASS_V(s, nir_lower_frexp);

   bool progress;
   do {
      progress = false;
      NIR_PASS(progress, s, nir_opt_algebraic_late);
      if (progress) {
         NIR_PASS_V(s, nir_copy_prop);
         NIR_PASS_V(s, nir_opt_dce);
         NIR_PASS_V(s, nir_opt_cse);
      }
   } while (progress);

   NIR_PASS_V(s, nir_opt_combine_barriers, NULL, NULL);

   if (screen->get_shader_param(screen,
                                pipe_shader_type_from_mesa(s->info.stage),
                                PIPE_SHADER_CAP_INTEGERS)) {
      NIR_PASS_V(s, nir_lower_bool_to_int32);
   } else {
      NIR_PASS_V(s, nir_lower_int_to_float);
      NIR_PASS_V(s, nir_lower_bool_to_float,
                 !options->lower_cmp && !options->lower_fabs);
      /* bool_to_float generates MOVs for b2f32 that we want to clean up. */
      NIR_PASS_V(s, nir_copy_prop);
      NIR_PASS_V(s, nir_opt_dce);
   }

   nir_move_options move_all =
       nir_move_const_undef | nir_move_load_ubo | nir_move_load_input |
       nir_move_comparisons | nir_move_copies | nir_move_load_ssbo;

   NIR_PASS_V(s, nir_opt_move, move_all);

   NIR_PASS_V(s, nir_convert_from_ssa, true);
   NIR_PASS_V(s, nir_lower_vec_to_regs, ntt_vec_to_mov_writemask_cb, NULL);

   /* locals_to_reg_intrinsics will leave dead derefs that are good to clean up.
    */
   NIR_PASS_V(s, nir_lower_locals_to_regs, 32);
   NIR_PASS_V(s, nir_opt_dce);

   /* See comment in ntt_get_alu_src for supported modifiers */
   NIR_PASS_V(s, nir_legacy_trivialize, !options->lower_fabs);

   if (NIR_DEBUG(TGSI)) {
      fprintf(stderr, "NIR before translation to TGSI:\n");
      nir_print_shader(s, stderr);
   }

   c = rzalloc(NULL, struct ntt_compile);
   c->screen = screen;
   c->options = options;

   c->needs_texcoord_semantic =
      screen->get_param(screen, PIPE_CAP_TGSI_TEXCOORD);
   c->has_txf_lz =
      screen->get_param(screen, PIPE_CAP_TGSI_TEX_TXF_LZ);

   c->s = s;
   c->native_integers = native_integers;
   c->ureg = ureg_create(pipe_shader_type_from_mesa(s->info.stage));
   ureg_setup_shader_info(c->ureg, &s->info);
   if (s->info.use_legacy_math_rules && screen->get_param(screen, PIPE_CAP_LEGACY_MATH_RULES))
      ureg_property(c->ureg, TGSI_PROPERTY_LEGACY_MATH_RULES, 1);

   if (s->info.stage == MESA_SHADER_FRAGMENT) {
      /* The draw module's polygon stipple layer doesn't respect the chosen
       * coordinate mode, so leave it as unspecified unless we're actually
       * reading the position in the shader already.  See
       * gl-2.1-polygon-stipple-fs on softpipe.
       */
      if ((s->info.inputs_read & VARYING_BIT_POS) ||
          BITSET_TEST(s->info.system_values_read, SYSTEM_VALUE_FRAG_COORD)) {
         ureg_property(c->ureg, TGSI_PROPERTY_FS_COORD_ORIGIN,
                       s->info.fs.origin_upper_left ?
                       TGSI_FS_COORD_ORIGIN_UPPER_LEFT :
                       TGSI_FS_COORD_ORIGIN_LOWER_LEFT);

         ureg_property(c->ureg, TGSI_PROPERTY_FS_COORD_PIXEL_CENTER,
                       s->info.fs.pixel_center_integer ?
                       TGSI_FS_COORD_PIXEL_CENTER_INTEGER :
                       TGSI_FS_COORD_PIXEL_CENTER_HALF_INTEGER);
      }
   }
   /* Emit the main function */
   nir_function_impl *impl = nir_shader_get_entrypoint(c->s);
   ntt_emit_impl(c, impl);
   ureg_END(c->ureg);

   tgsi_tokens = ureg_get_tokens(c->ureg, NULL);

   if (NIR_DEBUG(TGSI)) {
      fprintf(stderr, "TGSI after translation from NIR:\n");
      tgsi_dump(tgsi_tokens, 0);
   }

   ureg_destroy(c->ureg);

   ralloc_free(c);
   ralloc_free(s);

   return tgsi_tokens;
}

static const nir_shader_compiler_options nir_to_tgsi_compiler_options = {
   .fdot_replicates = true,
   .fuse_ffma32 = true,
   .fuse_ffma64 = true,
   .lower_extract_byte = true,
   .lower_extract_word = true,
   .lower_insert_byte = true,
   .lower_insert_word = true,
   .lower_fdph = true,
   .lower_flrp64 = true,
   .lower_fmod = true,
   .lower_uniforms_to_ubo = true,
   .lower_uadd_carry = true,
   .lower_usub_borrow = true,
   .lower_uadd_sat = true,
   .lower_usub_sat = true,
   .lower_vector_cmp = true,
   .lower_int64_options = nir_lower_imul_2x32_64,
   .use_interpolated_input_intrinsics = true,

   /* TGSI doesn't have a semantic for local or global index, just local and
    * workgroup id.
    */
   .lower_cs_local_index_to_id = true,
};

/* Returns a default compiler options for drivers with only nir-to-tgsi-based
 * NIR support.
 */
const void *
nir_to_tgsi_get_compiler_options(struct pipe_screen *pscreen,
                                 enum pipe_shader_ir ir,
                                 unsigned shader)
{
   assert(ir == PIPE_SHADER_IR_NIR);
   return &nir_to_tgsi_compiler_options;
}

/** Helper for getting TGSI tokens to store for a pipe_shader_state CSO. */
const void *
pipe_shader_state_to_tgsi_tokens(struct pipe_screen *screen,
                                 const struct pipe_shader_state *cso)
{
   if (cso->type == PIPE_SHADER_IR_NIR) {
      return nir_to_tgsi((nir_shader *)cso->ir.nir, screen);
   } else {
      assert(cso->type == PIPE_SHADER_IR_TGSI);
      /* we need to keep a local copy of the tokens */
      return tgsi_dup_tokens(cso->tokens);
   }
}
