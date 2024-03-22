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
#include "nir_to_rc.h"
#include "r300_nir.h"
#include "r300_screen.h"
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

struct ntr_insn {
   enum tgsi_opcode opcode;
   struct ureg_dst dst[2];
   struct ureg_src src[4];
   enum tgsi_texture_type tex_target;
   enum tgsi_return_type tex_return_type;
   struct tgsi_texture_offset tex_offset[4];

   unsigned mem_qualifier;
   enum pipe_format mem_format;

   bool is_tex : 1;
   bool precise : 1;
};

struct ntr_block {
   /* Array of struct ntr_insn */
   struct util_dynarray insns;
   int start_ip;
   int end_ip;
};

struct ntr_reg_interval {
   uint32_t start, end;
};

struct ntr_compile {
   nir_shader *s;
   nir_function_impl *impl;
   const struct nir_to_rc_options *options;
   struct pipe_screen *screen;
   struct ureg_program *ureg;

   bool addr_declared[3];
   struct ureg_dst addr_reg[3];

   /* if condition set up at the end of a block, for ntr_emit_if(). */
   struct ureg_src if_cond;

   /* TGSI temps for our NIR SSA and register values. */
   struct ureg_dst *reg_temp;
   struct ureg_src *ssa_temp;

   struct ntr_reg_interval *liveness;

   /* Map from nir_block to ntr_block */
   struct hash_table *blocks;
   struct ntr_block *cur_block;
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
};

static struct ureg_dst
ntr_temp(struct ntr_compile *c)
{
   return ureg_dst_register(TGSI_FILE_TEMPORARY, c->num_temps++);
}

static struct ntr_block *
ntr_block_from_nir(struct ntr_compile *c, struct nir_block *block)
{
   struct hash_entry *entry = _mesa_hash_table_search(c->blocks, block);
   return entry->data;
}

static void ntr_emit_cf_list(struct ntr_compile *c, struct exec_list *list);
static void ntr_emit_cf_list_ureg(struct ntr_compile *c, struct exec_list *list);

static struct ntr_insn *
ntr_insn(struct ntr_compile *c, enum tgsi_opcode opcode,
         struct ureg_dst dst,
         struct ureg_src src0, struct ureg_src src1,
         struct ureg_src src2, struct ureg_src src3)
{
   struct ntr_insn insn = {
      .opcode = opcode,
      .dst = { dst, ureg_dst_undef() },
      .src = { src0, src1, src2, src3 },
      .precise = c->precise,
   };
   util_dynarray_append(&c->cur_block->insns, struct ntr_insn, insn);
   return util_dynarray_top_ptr(&c->cur_block->insns, struct ntr_insn);
}

#define OP00( op )                                                                     \
static inline void ntr_##op(struct ntr_compile *c)                                     \
{                                                                                      \
   ntr_insn(c, TGSI_OPCODE_##op, ureg_dst_undef(), ureg_src_undef(), ureg_src_undef(), ureg_src_undef(), ureg_src_undef()); \
}

#define OP01( op )                                                                     \
static inline void ntr_##op(struct ntr_compile *c,                                     \
                     struct ureg_src src0)                                             \
{                                                                                      \
   ntr_insn(c, TGSI_OPCODE_##op, ureg_dst_undef(), src0, ureg_src_undef(), ureg_src_undef(), ureg_src_undef()); \
}


#define OP10( op )                                                                     \
static inline void ntr_##op(struct ntr_compile *c,                                     \
                     struct ureg_dst dst)                                              \
{                                                                                      \
   ntr_insn(c, TGSI_OPCODE_##op, dst, ureg_src_undef(), ureg_src_undef(), ureg_src_undef(), ureg_src_undef()); \
}

#define OP11( op )                                                                     \
static inline void ntr_##op(struct ntr_compile *c,                                     \
                     struct ureg_dst dst,                                              \
                     struct ureg_src src0)                                             \
{                                                                                      \
   ntr_insn(c, TGSI_OPCODE_##op, dst, src0, ureg_src_undef(), ureg_src_undef(), ureg_src_undef()); \
}

#define OP12( op )                                                                     \
static inline void ntr_##op(struct ntr_compile *c,                                     \
                     struct ureg_dst dst,                                              \
                     struct ureg_src src0,                                             \
                     struct ureg_src src1)                                             \
{                                                                                      \
   ntr_insn(c, TGSI_OPCODE_##op, dst, src0, src1, ureg_src_undef(), ureg_src_undef()); \
}

#define OP13( op )                                                                     \
static inline void ntr_##op(struct ntr_compile *c,                                     \
                     struct ureg_dst dst,                                              \
                     struct ureg_src src0,                                             \
                     struct ureg_src src1,                                             \
                     struct ureg_src src2)                                             \
{                                                                                      \
   ntr_insn(c, TGSI_OPCODE_##op, dst, src0, src1, src2, ureg_src_undef());             \
}

#define OP14( op )                                                                     \
static inline void ntr_##op(struct ntr_compile *c,                                     \
                     struct ureg_dst dst,                                              \
                     struct ureg_src src0,                                             \
                     struct ureg_src src1,                                             \
                     struct ureg_src src2,                                             \
                     struct ureg_src src3)                                             \
{                                                                                      \
   ntr_insn(c, TGSI_OPCODE_##op, dst, src0, src1, src2, src3);                         \
}

/* We hand-craft our tex instructions */
#define OP12_TEX(op)
#define OP14_TEX(op)

/* Use a template include to generate a correctly-typed ntr_OP()
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
ntr_src_as_uint(struct ntr_compile *c, nir_src src)
{
   uint32_t val = nir_src_as_uint(src);
   if (val >= fui(1.0))
      val = (uint32_t)uif(val);
   return val;
}

/* Per-channel masks of def/use within the block, and the per-channel
 * livein/liveout for the block as a whole.
 */
struct ntr_live_reg_block_state {
   uint8_t *def, *use, *livein, *liveout, *defin, *defout;
};

struct ntr_live_reg_state {
   unsigned bitset_words;

   struct ntr_reg_interval *regs;

   /* Used in propagate_across_edge() */
   BITSET_WORD *tmp_live;

   struct ntr_live_reg_block_state *blocks;

   nir_block_worklist worklist;
};

static void
ntr_live_reg_mark_use(struct ntr_compile *c, struct ntr_live_reg_block_state *bs,
                      int ip, unsigned index, unsigned used_mask)
{
   bs->use[index] |= used_mask & ~bs->def[index];

   c->liveness[index].start = MIN2(c->liveness[index].start, ip);
   c->liveness[index].end = MAX2(c->liveness[index].end, ip);

}
static void
ntr_live_reg_setup_def_use(struct ntr_compile *c, nir_function_impl *impl, struct ntr_live_reg_state *state)
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
      struct ntr_live_reg_block_state *bs = &state->blocks[block->index];
      struct ntr_block *ntr_block = ntr_block_from_nir(c, block);

      ntr_block->start_ip = ip;

      util_dynarray_foreach(&ntr_block->insns, struct ntr_insn, insn) {
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
            ntr_live_reg_mark_use(c, bs, ip, index, used_mask);
         }

         if (insn->is_tex) {
            for (int i = 0; i < ARRAY_SIZE(insn->tex_offset); i++) {
               if (insn->tex_offset[i].File == TGSI_FILE_TEMPORARY)
                  ntr_live_reg_mark_use(c, bs, ip, insn->tex_offset[i].Index, 0xf);
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

      ntr_block->end_ip = ip;
   }
}

static void
ntr_live_regs(struct ntr_compile *c, nir_function_impl *impl)
{
   nir_metadata_require(impl, nir_metadata_block_index);

   c->liveness = rzalloc_array(c, struct ntr_reg_interval, c->num_temps);

   struct ntr_live_reg_state state = {
       .blocks = rzalloc_array(impl, struct ntr_live_reg_block_state, impl->num_blocks),
   };

   /* The intervals start out with start > end (indicating unused) */
   for (int i = 0; i < c->num_temps; i++)
      c->liveness[i].start = ~0;

   ntr_live_reg_setup_def_use(c, impl, &state);

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
      struct ntr_block *ntr_block = ntr_block_from_nir(c, block);
      struct ntr_live_reg_block_state *bs = &state.blocks[block->index];

      for (int i = 0; i < c->num_temps; i++) {
         /* Collect livein from our successors to include in our liveout. */
         for (int j = 0; j < ARRAY_SIZE(block->successors); j++) {
            nir_block *succ = block->successors[j];
            if (!succ || succ->index == impl->num_blocks)
               continue;
            struct ntr_live_reg_block_state *sbs = &state.blocks[succ->index];

            uint8_t new_liveout = sbs->livein[i] & ~bs->liveout[i];
            if (new_liveout) {
               if (state.blocks[block->index].defout[i])
                  c->liveness[i].end = MAX2(c->liveness[i].end, ntr_block->end_ip);
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
               c->liveness[i].start = MIN2(c->liveness[i].start, ntr_block->start_ip);
         }
      }
   }

   ralloc_free(state.blocks);
   nir_block_worklist_fini(&state.worklist);
}

static void
ntr_ra_check(struct ntr_compile *c, unsigned *ra_map, BITSET_WORD *released, int ip, unsigned index)
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
ntr_allocate_regs(struct ntr_compile *c, nir_function_impl *impl)
{
   ntr_live_regs(c, impl);

   unsigned *ra_map = ralloc_array(c, unsigned, c->num_temps);
   unsigned *released = rzalloc_array(c, BITSET_WORD, BITSET_WORDS(c->num_temps));

   /* No RA on NIR array regs */
   for (int i = 0; i < c->first_non_array_temp; i++)
      ra_map[i] = i;

   for (int i = c->first_non_array_temp; i < c->num_temps; i++)
      ra_map[i] = ~0;

   int ip = 0;
   nir_foreach_block(block, impl) {
      struct ntr_block *ntr_block = ntr_block_from_nir(c, block);

      for (int i = 0; i < c->num_temps; i++)
         ntr_ra_check(c, ra_map, released, ip, i);

      util_dynarray_foreach(&ntr_block->insns, struct ntr_insn, insn) {
         const struct tgsi_opcode_info *opcode_info =
            tgsi_get_opcode_info(insn->opcode);

         for (int i = 0; i < opcode_info->num_src; i++) {
            if (insn->src[i].File == TGSI_FILE_TEMPORARY) {
               ntr_ra_check(c, ra_map, released, ip, insn->src[i].Index);
               insn->src[i].Index = ra_map[insn->src[i].Index];
            }
         }

         if (insn->is_tex) {
            for (int i = 0; i < ARRAY_SIZE(insn->tex_offset); i++) {
               if (insn->tex_offset[i].File == TGSI_FILE_TEMPORARY) {
                  ntr_ra_check(c, ra_map, released, ip, insn->tex_offset[i].Index);
                  insn->tex_offset[i].Index = ra_map[insn->tex_offset[i].Index];
               }
            }
         }

         for (int i = 0; i < opcode_info->num_dst; i++) {
            if (insn->dst[i].File == TGSI_FILE_TEMPORARY) {
               ntr_ra_check(c, ra_map, released, ip, insn->dst[i].Index);
               insn->dst[i].Index = ra_map[insn->dst[i].Index];
            }
         }
         ip++;
      }

      for (int i = 0; i < c->num_temps; i++)
         ntr_ra_check(c, ra_map, released, ip, i);
   }
}

static void
ntr_allocate_regs_unoptimized(struct ntr_compile *c, nir_function_impl *impl)
{
   for (int i = c->first_non_array_temp; i < c->num_temps; i++)
      ureg_DECL_temporary(c->ureg);
}

/* TGSI varying declarations have a component usage mask associated (used by
 * r600 and svga).
 */
static uint32_t
ntr_tgsi_var_usage_mask(const struct nir_variable *var)
{
   const struct glsl_type *type_without_array =
      glsl_without_array(var->type);
   unsigned num_components = glsl_get_vector_elements(type_without_array);
   if (num_components == 0) /* structs */
      num_components = 4;

   return u_bit_consecutive(var->data.location_frac, num_components);
}

static struct ureg_dst
ntr_output_decl(struct ntr_compile *c, nir_intrinsic_instr *instr, uint32_t *frac)
{
   nir_io_semantics semantics = nir_intrinsic_io_semantics(instr);
   int base = nir_intrinsic_base(instr);
   *frac = nir_intrinsic_component(instr);

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

      tgsi_get_gl_varying_semantic(semantics.location, true,
                                   &semantic_name, &semantic_index);

      uint32_t usage_mask = u_bit_consecutive(*frac, instr->num_components);
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

      out = ureg_DECL_output_layout(c->ureg,
                                    semantic_name, semantic_index,
                                    gs_streams,
                                    base,
                                    usage_mask,
                                    array_id,
                                    semantics.num_slots,
                                    invariant);
   }

   unsigned write_mask;
   if (nir_intrinsic_has_write_mask(instr))
      write_mask = nir_intrinsic_write_mask(instr);
   else
      write_mask = ((1 << instr->num_components) - 1) << *frac;

   write_mask = write_mask << *frac;
   return ureg_writemask(out, write_mask);
}

static bool
ntr_try_store_in_tgsi_output_with_use(struct ntr_compile *c,
                                      struct ureg_dst *dst,
                                      nir_src *src)
{
   *dst = ureg_dst_undef();

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
   *dst = ntr_output_decl(c, intr, &frac);
   dst->Index += ntr_src_as_uint(c, intr->src[1]);

   return frac == 0;
}

/* If this reg is used only for storing an output, then in the simple
 * cases we can write directly to the TGSI output instead of having
 * store_output emit its own MOV.
 */
static bool
ntr_try_store_reg_in_tgsi_output(struct ntr_compile *c, struct ureg_dst *dst,
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

   return ntr_try_store_in_tgsi_output_with_use(c, dst, use);
}

/* If this SSA def is used only for storing an output, then in the simple
 * cases we can write directly to the TGSI output instead of having
 * store_output emit its own MOV.
 */
static bool
ntr_try_store_ssa_in_tgsi_output(struct ntr_compile *c, struct ureg_dst *dst,
                                 nir_def *def)
{
   *dst = ureg_dst_undef();

   if (!list_is_singular(&def->uses))
      return false;

   nir_foreach_use_including_if(use, def) {
      return ntr_try_store_in_tgsi_output_with_use(c, dst, use);
   }
   unreachable("We have one use");
}

static void
ntr_setup_inputs(struct ntr_compile *c)
{
   if (c->s->info.stage != MESA_SHADER_FRAGMENT)
      return;

   unsigned num_inputs = 0;
   int num_input_arrays = 0;

   nir_foreach_shader_in_variable(var, c->s) {
      const struct glsl_type *type = var->type;
      unsigned array_len =
         glsl_count_attribute_slots(type, false);

      num_inputs = MAX2(num_inputs, var->data.driver_location + array_len);
   }

   c->input_index_map = ralloc_array(c, struct ureg_src, num_inputs);

   nir_foreach_shader_in_variable(var, c->s) {
      const struct glsl_type *type = var->type;
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
      tgsi_get_gl_varying_semantic(var->data.location, true,
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

      uint32_t usage_mask = ntr_tgsi_var_usage_mask(var);

      decl = ureg_DECL_fs_input_centroid_layout(c->ureg,
                                                semantic_name,
                                                semantic_index,
                                                interpolation,
                                                sample_loc,
                                                var->data.driver_location,
                                                usage_mask,
                                                array_id, array_len);

      if (semantic_name == TGSI_SEMANTIC_FACE) {
         struct ureg_dst temp = ntr_temp(c);
         /* tgsi docs say that floating point FACE will be positive for
          * frontface and negative for backface, but realistically
          * GLSL-to-TGSI had been doing MOV_SAT to turn it into 0.0 vs 1.0.
          * Copy that behavior, since some drivers (r300) have been doing a
          * 0.0 vs 1.0 backface (and I don't think anybody has a non-1.0
          * front face).
          */
         temp.Saturate = true;
         ntr_MOV(c, temp, decl);
         decl = ureg_src(temp);
      }

      for (unsigned i = 0; i < array_len; i++) {
         c->input_index_map[var->data.driver_location + i] = decl;
         c->input_index_map[var->data.driver_location + i].Index += i;
      }
   }
}

static int
ntr_sort_by_location(const nir_variable *a, const nir_variable *b)
{
   return a->data.location - b->data.location;
}

/**
 * Workaround for virglrenderer requiring that TGSI FS output color variables
 * are declared in order.  Besides, it's a lot nicer to read the TGSI this way.
 */
static void
ntr_setup_outputs(struct ntr_compile *c)
{
   if (c->s->info.stage != MESA_SHADER_FRAGMENT)
      return;

   nir_sort_variables_with_modes(c->s, ntr_sort_by_location, nir_var_shader_out);

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
ntr_setup_uniforms(struct ntr_compile *c)
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

      /* lower_uniforms_to_ubo lowered non-sampler uniforms to UBOs, so CB0
       * size declaration happens with other UBOs below.
       */
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
      ubo_sizes[ubo] = size;
   }

   for (int i = 0; i < ARRAY_SIZE(ubo_sizes); i++) {
      if (ubo_sizes[i])
         ureg_DECL_constant2D(c->ureg, 0, DIV_ROUND_UP(ubo_sizes[i], 16) - 1, i);
   }
}

static void
ntr_setup_registers(struct ntr_compile *c)
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
      unsigned index = nir_reg->def.index;

      /* We already handled arrays */
      if (num_array_elems == 0) {
         struct ureg_dst decl;
         uint32_t write_mask = BITFIELD_MASK(num_components);

         if (!ntr_try_store_reg_in_tgsi_output(c, &decl, nir_reg)) {
            decl = ureg_writemask(ntr_temp(c), write_mask);
         }
         c->reg_temp[index] = decl;
      }
   }
}

static struct ureg_src
ntr_get_load_const_src(struct ntr_compile *c, nir_load_const_instr *instr)
{
   int num_components = instr->def.num_components;

   float values[4];
   assert(instr->def.bit_size == 32);
   for (int i = 0; i < num_components; i++)
      values[i] = uif(instr->value[i].u32);

   return ureg_DECL_immediate(c->ureg, values, num_components);
}

static struct ureg_src
ntr_reladdr(struct ntr_compile *c, struct ureg_src addr, int addr_index)
{
   assert(addr_index < ARRAY_SIZE(c->addr_reg));

   for (int i = 0; i <= addr_index; i++) {
      if (!c->addr_declared[i]) {
         c->addr_reg[i] = ureg_writemask(ureg_DECL_address(c->ureg),
                                             TGSI_WRITEMASK_X);
         c->addr_declared[i] = true;
      }
   }

   ntr_ARL(c, c->addr_reg[addr_index], addr);
   return ureg_scalar(ureg_src(c->addr_reg[addr_index]), 0);
}

/* Forward declare for recursion with indirects */
static struct ureg_src
ntr_get_src(struct ntr_compile *c, nir_src src);

static struct ureg_src
ntr_get_chased_src(struct ntr_compile *c, nir_legacy_src *src)
{
   if (src->is_ssa) {
      if (src->ssa->parent_instr->type == nir_instr_type_load_const)
         return ntr_get_load_const_src(c, nir_instr_as_load_const(src->ssa->parent_instr));

      return c->ssa_temp[src->ssa->index];
   } else {
      struct ureg_dst reg_temp = c->reg_temp[src->reg.handle->index];
      reg_temp.Index += src->reg.base_offset;

      if (src->reg.indirect) {
         struct ureg_src offset = ntr_get_src(c, nir_src_for_ssa(src->reg.indirect));
         return ureg_src_indirect(ureg_src(reg_temp),
                                  ntr_reladdr(c, offset, 0));
      } else {
         return ureg_src(reg_temp);
      }
   }
}

static struct ureg_src
ntr_get_src(struct ntr_compile *c, nir_src src)
{
   nir_legacy_src chased = nir_legacy_chase_src(&src);
   return ntr_get_chased_src(c, &chased);
}

static struct ureg_src
ntr_get_alu_src(struct ntr_compile *c, nir_alu_instr *instr, int i)
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
   struct ureg_src usrc = ntr_get_chased_src(c, &src.src);

   usrc = ureg_swizzle(usrc,
                       src.swizzle[0],
                       src.swizzle[1],
                       src.swizzle[2],
                       src.swizzle[3]);

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
ntr_swizzle_for_write_mask(struct ureg_src src, uint32_t write_mask)
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
ntr_get_ssa_def_decl(struct ntr_compile *c, nir_def *ssa)
{
   uint32_t writemask = BITSET_MASK(ssa->num_components);

   struct ureg_dst dst;
   if (!ntr_try_store_ssa_in_tgsi_output(c, &dst, ssa))
      dst = ntr_temp(c);

   c->ssa_temp[ssa->index] = ntr_swizzle_for_write_mask(ureg_src(dst), writemask);

   return ureg_writemask(dst, writemask);
}

static struct ureg_dst
ntr_get_chased_dest_decl(struct ntr_compile *c, nir_legacy_dest *dest)
{
   if (dest->is_ssa)
      return ntr_get_ssa_def_decl(c, dest->ssa);
   else
      return c->reg_temp[dest->reg.handle->index];
}

static struct ureg_dst
ntr_get_chased_dest(struct ntr_compile *c, nir_legacy_dest *dest)
{
   struct ureg_dst dst = ntr_get_chased_dest_decl(c, dest);

   if (!dest->is_ssa) {
      dst.Index += dest->reg.base_offset;

      if (dest->reg.indirect) {
         struct ureg_src offset = ntr_get_src(c, nir_src_for_ssa(dest->reg.indirect));
         dst = ureg_dst_indirect(dst, ntr_reladdr(c, offset, 0));
      }
   }

   return dst;
}

static struct ureg_dst
ntr_get_dest(struct ntr_compile *c, nir_def *def)
{
   nir_legacy_dest chased = nir_legacy_chase_dest(def);
   return ntr_get_chased_dest(c, &chased);
}

static struct ureg_dst
ntr_get_alu_dest(struct ntr_compile *c, nir_def *def)
{
   nir_legacy_alu_dest chased = nir_legacy_chase_alu_dest(def);
   struct ureg_dst dst = ntr_get_chased_dest(c, &chased.dest);

   if (chased.fsat)
      dst.Saturate = true;

   /* Only registers get write masks */
   if (chased.dest.is_ssa)
      return dst;

   return ureg_writemask(dst, chased.write_mask);
}

/* For an SSA dest being populated by a constant src, replace the storage with
 * a copy of the ureg_src.
 */
static void
ntr_store_def(struct ntr_compile *c, nir_def *def, struct ureg_src src)
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

   ntr_MOV(c, ntr_get_ssa_def_decl(c, def), src);
}

static void
ntr_store(struct ntr_compile *c, nir_def *def, struct ureg_src src)
{
   nir_legacy_dest chased = nir_legacy_chase_dest(def);

   if (chased.is_ssa)
      ntr_store_def(c, chased.ssa, src);
   else {
      struct ureg_dst dst = ntr_get_chased_dest(c, &chased);
      ntr_MOV(c, dst, src);
   }
}

static void
ntr_emit_scalar(struct ntr_compile *c, unsigned tgsi_op,
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
         ntr_insn(c, tgsi_op,
                  ureg_writemask(dst, 1 << i),
                  ureg_scalar(src0, i),
                  ureg_scalar(src1, i),
                  ureg_src_undef(), ureg_src_undef());
      }
   }
}

static void
ntr_emit_alu(struct ntr_compile *c, nir_alu_instr *instr)
{
   struct ureg_src src[4];
   struct ureg_dst dst;
   unsigned i;
   int num_srcs = nir_op_infos[instr->op].num_inputs;

   /* Don't try to translate folded fsat since their source won't be valid */
   if (instr->op == nir_op_fsat && nir_legacy_fsat_folds(instr))
      return;

   c->precise = instr->exact;

   assert(num_srcs <= ARRAY_SIZE(src));
   for (i = 0; i < num_srcs; i++)
      src[i] = ntr_get_alu_src(c, instr, i);
   for (; i < ARRAY_SIZE(src); i++)
      src[i] = ureg_src_undef();

   dst = ntr_get_alu_dest(c, &instr->def);

   static enum tgsi_opcode op_map[] = {
      [nir_op_mov] = TGSI_OPCODE_MOV,

      [nir_op_fdot2_replicated] = TGSI_OPCODE_DP2,
      [nir_op_fdot3_replicated] = TGSI_OPCODE_DP3,
      [nir_op_fdot4_replicated] = TGSI_OPCODE_DP4,
      [nir_op_ffloor] = TGSI_OPCODE_FLR,
      [nir_op_ffract] = TGSI_OPCODE_FRC,
      [nir_op_fceil] = TGSI_OPCODE_CEIL,
      [nir_op_fround_even] = TGSI_OPCODE_ROUND,

      [nir_op_slt] = TGSI_OPCODE_SLT,
      [nir_op_sge] = TGSI_OPCODE_SGE,
      [nir_op_seq] = TGSI_OPCODE_SEQ,
      [nir_op_sne] = TGSI_OPCODE_SNE,

      [nir_op_ftrunc] = TGSI_OPCODE_TRUNC,
      [nir_op_fddx] = TGSI_OPCODE_DDX,
      [nir_op_fddy] = TGSI_OPCODE_DDY,
      [nir_op_fddx_coarse] = TGSI_OPCODE_DDX,
      [nir_op_fddy_coarse] = TGSI_OPCODE_DDY,
      [nir_op_fadd] = TGSI_OPCODE_ADD,
      [nir_op_fmul] = TGSI_OPCODE_MUL,

      [nir_op_fmin] = TGSI_OPCODE_MIN,
      [nir_op_fmax] = TGSI_OPCODE_MAX,
      [nir_op_ffma] = TGSI_OPCODE_MAD,
   };

   if (instr->op < ARRAY_SIZE(op_map) && op_map[instr->op] > 0) {
      /* The normal path for NIR to TGSI ALU op translation */
      ntr_insn(c, op_map[instr->op],
                dst, src[0], src[1], src[2], src[3]);
   } else {
      /* Special cases for NIR to TGSI ALU op translation. */

      /* TODO: Use something like the ntr_store() path for the MOV calls so we
       * don't emit extra MOVs for swizzles/srcmods of inputs/const/imm.
       */

      switch (instr->op) {
      case nir_op_fabs:
         /* Try to eliminate */
         if (!c->options->lower_fabs && nir_legacy_float_mod_folds(instr))
            break;

         if (c->options->lower_fabs)
            ntr_MAX(c, dst, src[0], ureg_negate(src[0]));
         else
            ntr_MOV(c, dst, ureg_abs(src[0]));
         break;

      case nir_op_fsat:
         ntr_MOV(c, ureg_saturate(dst), src[0]);
         break;

      case nir_op_fneg:
         /* Try to eliminate */
         if (nir_legacy_float_mod_folds(instr))
            break;

         ntr_MOV(c, dst, ureg_negate(src[0]));
         break;

         /* NOTE: TGSI 32-bit math ops have the old "one source channel
          * replicated to all dst channels" behavior, while 64 is normal mapping
          * of src channels to dst.
          */
      case nir_op_frcp:
         ntr_emit_scalar(c, TGSI_OPCODE_RCP, dst, src[0], ureg_src_undef());
         break;

      case nir_op_frsq:
         ntr_emit_scalar(c, TGSI_OPCODE_RSQ, dst, src[0], ureg_src_undef());
         break;

      case nir_op_fexp2:
         ntr_emit_scalar(c, TGSI_OPCODE_EX2, dst, src[0], ureg_src_undef());
         break;

      case nir_op_flog2:
         ntr_emit_scalar(c, TGSI_OPCODE_LG2, dst, src[0], ureg_src_undef());
         break;

      case nir_op_fsin:
         ntr_emit_scalar(c, TGSI_OPCODE_SIN, dst, src[0], ureg_src_undef());
         break;

      case nir_op_fcos:
         ntr_emit_scalar(c, TGSI_OPCODE_COS, dst, src[0], ureg_src_undef());
         break;

      case nir_op_fsub:
         ntr_ADD(c, dst, src[0], ureg_negate(src[1]));
         break;

      case nir_op_fmod:
         unreachable("should be handled by .lower_fmod = true");
         break;

      case nir_op_fpow:
         ntr_emit_scalar(c, TGSI_OPCODE_POW, dst, src[0], src[1]);
         break;

      case nir_op_flrp:
         ntr_LRP(c, dst, src[2], src[1], src[0]);
         break;

      case nir_op_fcsel:
         /* If CMP isn't supported, then the flags that enable NIR to generate
          * this opcode should also not be set.
          */
         assert(!c->options->lower_cmp);

         /* Implement this as CMP(-abs(src0), src1, src2). */
         ntr_CMP(c, dst, ureg_negate(ureg_abs(src[0])), src[1], src[2]);
         break;

      case nir_op_fcsel_gt:
         /* If CMP isn't supported, then the flags that enable NIR to generate
          * these opcodes should also not be set.
          */
         assert(!c->options->lower_cmp);

         ntr_CMP(c, dst, ureg_negate(src[0]), src[1], src[2]);
         break;

      case nir_op_fcsel_ge:
         /* If CMP isn't supported, then the flags that enable NIR to generate
          * these opcodes should also not be set.
          */
         assert(!c->options->lower_cmp);

         /* Implement this as if !(src0 < 0.0) was identical to src0 >= 0.0. */
         ntr_CMP(c, dst, src[0], src[2], src[1]);
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
ntr_ureg_src_indirect(struct ntr_compile *c, struct ureg_src usrc,
                      nir_src src, int addr_reg)
{
   if (nir_src_is_const(src)) {
      usrc.Index += ntr_src_as_uint(c, src);
      return usrc;
   } else {
      return ureg_src_indirect(usrc, ntr_reladdr(c, ntr_get_src(c, src), addr_reg));
   }
}

static struct ureg_dst
ntr_ureg_dst_indirect(struct ntr_compile *c, struct ureg_dst dst,
                      nir_src src)
{
   if (nir_src_is_const(src)) {
      dst.Index += ntr_src_as_uint(c, src);
      return dst;
   } else {
      return ureg_dst_indirect(dst, ntr_reladdr(c, ntr_get_src(c, src), 0));
   }
}

static struct ureg_dst
ntr_ureg_dst_dimension_indirect(struct ntr_compile *c, struct ureg_dst udst,
                                nir_src src)
{
   if (nir_src_is_const(src)) {
      return ureg_dst_dimension(udst, ntr_src_as_uint(c, src));
   } else {
      return ureg_dst_dimension_indirect(udst,
                                         ntr_reladdr(c, ntr_get_src(c, src), 1),
                                         0);
   }
}
/* Some load operations in NIR will have a fractional offset that we need to
 * swizzle down before storing to the result register.
 */
static struct ureg_src
ntr_shift_by_frac(struct ureg_src src, unsigned frac, unsigned num_components)
{
   return ureg_swizzle(src,
                       frac,
                       frac + MIN2(num_components - 1, 1),
                       frac + MIN2(num_components - 1, 2),
                       frac + MIN2(num_components - 1, 3));
}


static void
ntr_emit_load_ubo(struct ntr_compile *c, nir_intrinsic_instr *instr)
{
   struct ureg_src src = ureg_src_register(TGSI_FILE_CONSTANT, 0);

   struct ureg_dst addr_temp = ureg_dst_undef();

   if (nir_src_is_const(instr->src[0])) {
      src = ureg_src_dimension(src, ntr_src_as_uint(c, instr->src[0]));
   } else {
      /* virglrenderer requires that indirect UBO references have the UBO
       * array's base index in the Index field, not added to the indrect
       * address.
       *
       * Many nir intrinsics have a base address const value for the start of
       * their array indirection, but load_ubo doesn't.  We fake it by
       * subtracting it off here.
       */
      addr_temp = ntr_temp(c);
      ntr_UADD(c, addr_temp, ntr_get_src(c, instr->src[0]), ureg_imm1i(c->ureg, -c->first_ubo));
      src = ureg_src_dimension_indirect(src,
                                         ntr_reladdr(c, ureg_src(addr_temp), 1),
                                         c->first_ubo);
   }

   /* !PIPE_CAP_LOAD_CONSTBUF: Just emit it as a vec4 reference to the const
    * file.
    */
   src.Index = nir_intrinsic_base(instr);

   if (nir_src_is_const(instr->src[1])) {
      src.Index += ntr_src_as_uint(c, instr->src[1]);
   } else {
      src = ureg_src_indirect(src, ntr_reladdr(c, ntr_get_src(c, instr->src[1]), 0));
   }

   int start_component = nir_intrinsic_component(instr);

   src = ntr_shift_by_frac(src, start_component, instr->num_components);

   ntr_store(c, &instr->def, src);
}

static void
ntr_emit_load_input(struct ntr_compile *c, nir_intrinsic_instr *instr)
{
   uint32_t frac = nir_intrinsic_component(instr);
   uint32_t num_components = instr->num_components;
   unsigned base = nir_intrinsic_base(instr);
   struct ureg_src input;
   nir_io_semantics semantics = nir_intrinsic_io_semantics(instr);

   if (c->s->info.stage == MESA_SHADER_VERTEX) {
      input = ureg_DECL_vs_input(c->ureg, base);
      for (int i = 1; i < semantics.num_slots; i++)
         ureg_DECL_vs_input(c->ureg, base + i);
   } else {
      input = c->input_index_map[base];
   }

   input = ntr_shift_by_frac(input, frac, num_components);

   switch (instr->intrinsic) {
   case nir_intrinsic_load_input:
      input = ntr_ureg_src_indirect(c, input, instr->src[0], 0);
      ntr_store(c, &instr->def, input);
      break;

   case nir_intrinsic_load_interpolated_input: {
      input = ntr_ureg_src_indirect(c, input, instr->src[1], 0);

      nir_intrinsic_instr *bary_instr =
         nir_instr_as_intrinsic(instr->src[0].ssa->parent_instr);

      switch (bary_instr->intrinsic) {
      case nir_intrinsic_load_barycentric_pixel:
      case nir_intrinsic_load_barycentric_sample:
         /* For these, we know that the barycentric load matches the
          * interpolation on the input declaration, so we can use it directly.
          */
         ntr_store(c, &instr->def, input);
         break;

      case nir_intrinsic_load_barycentric_centroid:
         /* If the input was declared centroid, then there's no need to
          * emit the extra TGSI interp instruction, we can just read the
          * input.
          */
         if (c->centroid_inputs & (1ull << nir_intrinsic_base(instr))) {
            ntr_store(c, &instr->def, input);
         } else {
            ntr_INTERP_CENTROID(c, ntr_get_dest(c, &instr->def), input);
         }
         break;

      case nir_intrinsic_load_barycentric_at_sample:
         /* We stored the sample in the fake "bary" dest. */
         ntr_INTERP_SAMPLE(c, ntr_get_dest(c, &instr->def), input,
                            ntr_get_src(c, instr->src[0]));
         break;

      case nir_intrinsic_load_barycentric_at_offset:
         /* We stored the offset in the fake "bary" dest. */
         ntr_INTERP_OFFSET(c, ntr_get_dest(c, &instr->def), input,
                            ntr_get_src(c, instr->src[0]));
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
ntr_emit_store_output(struct ntr_compile *c, nir_intrinsic_instr *instr)
{
   struct ureg_src src = ntr_get_src(c, instr->src[0]);

   if (src.File == TGSI_FILE_OUTPUT) {
      /* If our src is the output file, that's an indication that we were able
       * to emit the output stores in the generating instructions and we have
       * nothing to do here.
       */
      return;
   }

   uint32_t frac;
   struct ureg_dst out = ntr_output_decl(c, instr, &frac);

   if (instr->intrinsic == nir_intrinsic_store_per_vertex_output) {
      out = ntr_ureg_dst_indirect(c, out, instr->src[2]);
      out = ntr_ureg_dst_dimension_indirect(c, out, instr->src[1]);
   } else {
      out = ntr_ureg_dst_indirect(c, out, instr->src[1]);
   }

   uint8_t swizzle[4] = { 0, 0, 0, 0 };
   for (int i = frac; i < 4; i++) {
      if (out.WriteMask & (1 << i))
         swizzle[i] = i - frac;
   }

   src = ureg_swizzle(src, swizzle[0], swizzle[1], swizzle[2], swizzle[3]);

   ntr_MOV(c, out, src);
}

static void
ntr_emit_load_output(struct ntr_compile *c, nir_intrinsic_instr *instr)
{
   nir_io_semantics semantics = nir_intrinsic_io_semantics(instr);

   /* ntr_try_store_in_tgsi_output() optimization is not valid if normal
    * load_output is present.
    */
   assert(c->s->info.stage != MESA_SHADER_VERTEX &&
          (c->s->info.stage != MESA_SHADER_FRAGMENT || semantics.fb_fetch_output));

   uint32_t frac;
   struct ureg_dst out = ntr_output_decl(c, instr, &frac);

   if (instr->intrinsic == nir_intrinsic_load_per_vertex_output) {
      out = ntr_ureg_dst_indirect(c, out, instr->src[1]);
      out = ntr_ureg_dst_dimension_indirect(c, out, instr->src[0]);
   } else {
      out = ntr_ureg_dst_indirect(c, out, instr->src[0]);
   }

   struct ureg_dst dst = ntr_get_dest(c, &instr->def);
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
      ntr_FBFETCH(c, dst, out_src);
   else
      ntr_MOV(c, dst, out_src);
}

static void
ntr_emit_load_sysval(struct ntr_compile *c, nir_intrinsic_instr *instr)
{
   gl_system_value sysval = nir_system_value_from_intrinsic(instr->intrinsic);
   enum tgsi_semantic semantic = tgsi_get_sysval_semantic(sysval);
   struct ureg_src sv = ureg_DECL_system_value(c->ureg, semantic, 0);

   /* virglrenderer doesn't like references to channels of the sysval that
    * aren't defined, even if they aren't really read.  (GLSL compile fails on
    * gl_NumWorkGroups.w, for example).
    */
   uint32_t write_mask = BITSET_MASK(instr->def.num_components);
   sv = ntr_swizzle_for_write_mask(sv, write_mask);

   /* TGSI and NIR define these intrinsics as always loading ints, but they can
    * still appear on hardware with non-native-integers fragment shaders using
    * the draw path (i915g).  In that case, having called nir_lower_int_to_float
    * means that we actually want floats instead.
    */
   switch (instr->intrinsic) {
   case nir_intrinsic_load_vertex_id:
   case nir_intrinsic_load_instance_id:
      ntr_U2F(c, ntr_get_dest(c, &instr->def), sv);
      return;

   default:
      break;
   }

   ntr_store(c, &instr->def, sv);
}

static void
ntr_emit_intrinsic(struct ntr_compile *c, nir_intrinsic_instr *instr)
{
   switch (instr->intrinsic) {
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_ubo_vec4:
      ntr_emit_load_ubo(c, instr);
      break;

      /* Vertex */
   case nir_intrinsic_load_draw_id:
   case nir_intrinsic_load_invocation_id:
   case nir_intrinsic_load_frag_coord:
   case nir_intrinsic_load_point_coord:
   case nir_intrinsic_load_front_face:
      ntr_emit_load_sysval(c, instr);
      break;

   case nir_intrinsic_load_input:
   case nir_intrinsic_load_per_vertex_input:
   case nir_intrinsic_load_interpolated_input:
      ntr_emit_load_input(c, instr);
      break;

   case nir_intrinsic_store_output:
   case nir_intrinsic_store_per_vertex_output:
      ntr_emit_store_output(c, instr);
      break;

   case nir_intrinsic_load_output:
   case nir_intrinsic_load_per_vertex_output:
      ntr_emit_load_output(c, instr);
      break;

   case nir_intrinsic_discard:
      ntr_KILL(c);
      break;

   case nir_intrinsic_discard_if: {
      struct ureg_src cond = ureg_scalar(ntr_get_src(c, instr->src[0]), 0);
      /* For !native_integers, the bool got lowered to 1.0 or 0.0. */
      ntr_KILL_IF(c, ureg_negate(cond));
      break;
   }
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
      ntr_store(c, &instr->def, ntr_get_src(c, instr->src[0]));
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

struct ntr_tex_operand_state {
   struct ureg_src srcs[4];
   unsigned i;
};

static void
ntr_push_tex_arg(struct ntr_compile *c,
                 nir_tex_instr *instr,
                 nir_tex_src_type tex_src_type,
                 struct ntr_tex_operand_state *s)
{
   int tex_src = nir_tex_instr_src_index(instr, tex_src_type);
   if (tex_src < 0)
      return;

   nir_src *src = &instr->src[tex_src].src;
   s->srcs[s->i++] = ntr_get_src(c, *src);
}

static void
ntr_emit_texture(struct ntr_compile *c, nir_tex_instr *instr)
{
   struct ureg_dst dst = ntr_get_dest(c, &instr->def);
   enum tgsi_texture_type target = tgsi_texture_type_from_sampler_dim(instr->sampler_dim, instr->is_array, instr->is_shadow);
   unsigned tex_opcode;

   int tex_handle_src = nir_tex_instr_src_index(instr, nir_tex_src_texture_handle);
   int sampler_handle_src = nir_tex_instr_src_index(instr, nir_tex_src_sampler_handle);

   struct ureg_src sampler;
   if (tex_handle_src >= 0 && sampler_handle_src >= 0) {
      /* It seems we can't get separate tex/sampler on GL, just use one of the handles */
      sampler = ntr_get_src(c, instr->src[tex_handle_src].src);
      assert(nir_tex_instr_src_index(instr, nir_tex_src_sampler_offset) == -1);
   } else {
      assert(tex_handle_src == -1 && sampler_handle_src == -1);
      sampler = ureg_DECL_sampler(c->ureg, instr->sampler_index);
      int sampler_src = nir_tex_instr_src_index(instr, nir_tex_src_sampler_offset);
      if (sampler_src >= 0) {
         struct ureg_src reladdr = ntr_get_src(c, instr->src[sampler_src].src);
         sampler = ureg_src_indirect(sampler, ntr_reladdr(c, reladdr, 2));
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

   struct ntr_tex_operand_state s = { .i = 0 };
   ntr_push_tex_arg(c, instr, nir_tex_src_backend1, &s);
   ntr_push_tex_arg(c, instr, nir_tex_src_backend2, &s);

   /* non-coord arg for TXQ */
   if (tex_opcode == TGSI_OPCODE_TXQ) {
      ntr_push_tex_arg(c, instr, nir_tex_src_lod, &s);
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
      s.srcs[s.i++] = ntr_get_src(c, instr->src[ddx].src);
      s.srcs[s.i++] = ntr_get_src(c, instr->src[ddy].src);
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
      tex_dst = ureg_writemask(ntr_temp(c), TGSI_WRITEMASK_W);
   else
      tex_dst = dst;

   while (s.i < 4)
      s.srcs[s.i++] = ureg_src_undef();

   struct ntr_insn *insn = ntr_insn(c, tex_opcode, tex_dst, s.srcs[0], s.srcs[1], s.srcs[2], s.srcs[3]);
   insn->tex_target = target;
   insn->tex_return_type = tex_type;
   insn->is_tex = true;

   int tex_offset_src = nir_tex_instr_src_index(instr, nir_tex_src_offset);
   if (tex_offset_src >= 0) {
      struct ureg_src offset = ntr_get_src(c, instr->src[tex_offset_src].src);

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
      ntr_MOV(c, dst, ureg_scalar(ureg_src(tex_dst), 3));
}

static void
ntr_emit_jump(struct ntr_compile *c, nir_jump_instr *jump)
{
   switch (jump->type) {
   case nir_jump_break:
      ntr_BRK(c);
      break;

   case nir_jump_continue:
      ntr_CONT(c);
      break;

   default:
      fprintf(stderr, "Unknown jump instruction: ");
      nir_print_instr(&jump->instr, stderr);
      fprintf(stderr, "\n");
      abort();
   }
}

static void
ntr_emit_ssa_undef(struct ntr_compile *c, nir_undef_instr *instr)
{
   /* Nothing to do but make sure that we have some storage to deref. */
   (void)ntr_get_ssa_def_decl(c, &instr->def);
}

static void
ntr_emit_instr(struct ntr_compile *c, nir_instr *instr)
{
   switch (instr->type) {
   case nir_instr_type_deref:
      /* ignored, will be walked by nir_intrinsic_image_*_deref. */
      break;

   case nir_instr_type_alu:
      ntr_emit_alu(c, nir_instr_as_alu(instr));
      break;

   case nir_instr_type_intrinsic:
      ntr_emit_intrinsic(c, nir_instr_as_intrinsic(instr));
      break;

   case nir_instr_type_load_const:
      /* Nothing to do here, as load consts are done directly from
       * ntr_get_src() (since many constant NIR srcs will often get folded
       * directly into a register file index instead of as a TGSI src).
       */
      break;

   case nir_instr_type_tex:
      ntr_emit_texture(c, nir_instr_as_tex(instr));
      break;

   case nir_instr_type_jump:
      ntr_emit_jump(c, nir_instr_as_jump(instr));
      break;

   case nir_instr_type_undef:
      ntr_emit_ssa_undef(c, nir_instr_as_undef(instr));
      break;

   default:
      fprintf(stderr, "Unknown NIR instr type: ");
      nir_print_instr(instr, stderr);
      fprintf(stderr, "\n");
      abort();
   }
}

static void
ntr_emit_if(struct ntr_compile *c, nir_if *if_stmt)
{
   ntr_IF(c, c->if_cond);

   ntr_emit_cf_list(c, &if_stmt->then_list);

   if (!nir_cf_list_is_empty_block(&if_stmt->else_list)) {
      ntr_ELSE(c);
      ntr_emit_cf_list(c, &if_stmt->else_list);
   }

   ntr_ENDIF(c);
}

static void
ntr_emit_loop(struct ntr_compile *c, nir_loop *loop)
{
   assert(!nir_loop_has_continue_construct(loop));
   ntr_BGNLOOP(c);
   ntr_emit_cf_list(c, &loop->body);
   ntr_ENDLOOP(c);
}

static void
ntr_emit_block(struct ntr_compile *c, nir_block *block)
{
   struct ntr_block *ntr_block = ntr_block_from_nir(c, block);
   c->cur_block = ntr_block;

   nir_foreach_instr(instr, block) {
      ntr_emit_instr(c, instr);

      /* Sanity check that we didn't accidentally ureg_OPCODE() instead of ntr_OPCODE(). */
      if (ureg_get_instruction_number(c->ureg) != 0) {
         fprintf(stderr, "Emitted ureg insn during: ");
         nir_print_instr(instr, stderr);
         fprintf(stderr, "\n");
         unreachable("emitted ureg insn");
      }
   }

   /* Set up the if condition for ntr_emit_if(), which we have to do before
    * freeing up the temps (the "if" is treated as inside the block for liveness
    * purposes, despite not being an instruction)
    *
    * Note that, while IF and UIF are supposed to look at only .x, virglrenderer
    * looks at all of .xyzw.  No harm in working around the bug.
    */
   nir_if *nif = nir_block_get_following_if(block);
   if (nif)
      c->if_cond = ureg_scalar(ntr_get_src(c, nif->condition), TGSI_SWIZZLE_X);
}

static void
ntr_emit_cf_list(struct ntr_compile *c, struct exec_list *list)
{
   foreach_list_typed(nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block:
         ntr_emit_block(c, nir_cf_node_as_block(node));
         break;

      case nir_cf_node_if:
         ntr_emit_if(c, nir_cf_node_as_if(node));
         break;

      case nir_cf_node_loop:
         ntr_emit_loop(c, nir_cf_node_as_loop(node));
         break;

      default:
         unreachable("unknown CF type");
      }
   }
}

static void
ntr_emit_block_ureg(struct ntr_compile *c, struct nir_block *block)
{
   struct ntr_block *ntr_block = ntr_block_from_nir(c, block);

   /* Emit the ntr insns to tgsi_ureg. */
   util_dynarray_foreach(&ntr_block->insns, struct ntr_insn, insn) {
      const struct tgsi_opcode_info *opcode_info =
         tgsi_get_opcode_info(insn->opcode);

      switch (insn->opcode) {
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
ntr_emit_if_ureg(struct ntr_compile *c, nir_if *if_stmt)
{
   /* Note: the last block emitted our IF opcode. */

   int if_stack = c->current_if_else;
   c->current_if_else = c->cf_label;

   /* Either the then or else block includes the ENDIF, which will fix up the
    * IF(/ELSE)'s label for jumping
    */
   ntr_emit_cf_list_ureg(c, &if_stmt->then_list);
   ntr_emit_cf_list_ureg(c, &if_stmt->else_list);

   c->current_if_else = if_stack;
}

static void
ntr_emit_cf_list_ureg(struct ntr_compile *c, struct exec_list *list)
{
   foreach_list_typed(nir_cf_node, node, node, list) {
      switch (node->type) {
      case nir_cf_node_block:
         ntr_emit_block_ureg(c, nir_cf_node_as_block(node));
         break;

      case nir_cf_node_if:
         ntr_emit_if_ureg(c, nir_cf_node_as_if(node));
         break;

      case nir_cf_node_loop:
         /* GLSL-to-TGSI never set the begin/end labels to anything, even though nvfx
          * does reference BGNLOOP's.  Follow the former behavior unless something comes up
          * with a need.
          */
         ntr_emit_cf_list_ureg(c, &nir_cf_node_as_loop(node)->body);
         break;

      default:
         unreachable("unknown CF type");
      }
   }
}

static void
ntr_emit_impl(struct ntr_compile *c, nir_function_impl *impl)
{
   c->impl = impl;

   c->ssa_temp = rzalloc_array(c, struct ureg_src, impl->ssa_alloc);
   c->reg_temp = rzalloc_array(c, struct ureg_dst, impl->ssa_alloc);

   /* Set up the struct ntr_blocks to put insns in */
   c->blocks = _mesa_pointer_hash_table_create(c);
   nir_foreach_block(block, impl) {
      struct ntr_block *ntr_block = rzalloc(c->blocks, struct ntr_block);
      util_dynarray_init(&ntr_block->insns, ntr_block);
      _mesa_hash_table_insert(c->blocks, block, ntr_block);
   }


   ntr_setup_registers(c);

   c->cur_block = ntr_block_from_nir(c, nir_start_block(impl));
   ntr_setup_inputs(c);
   ntr_setup_outputs(c);
   ntr_setup_uniforms(c);

   /* Emit the ntr insns */
   ntr_emit_cf_list(c, &impl->body);

   /* Don't do optimized RA if the driver requests it, unless the number of
    * temps is too large to be covered by the 16 bit signed int that TGSI
    * allocates for the register index */
   if (!c->options->unoptimized_ra || c->num_temps > 0x7fff)
      ntr_allocate_regs(c, impl);
   else
      ntr_allocate_regs_unoptimized(c, impl);

   /* Turn the ntr insns into actual TGSI tokens */
   ntr_emit_cf_list_ureg(c, &impl->body);

   ralloc_free(c->liveness);
   c->liveness = NULL;

}

static int
type_size(const struct glsl_type *type, bool bindless)
{
   return glsl_count_attribute_slots(type, false);
}

/* Allow vectorizing of ALU instructions.
 */
static uint8_t
ntr_should_vectorize_instr(const nir_instr *instr, const void *data)
{
   if (instr->type != nir_instr_type_alu)
      return 0;

   return 4;
}

static bool
ntr_should_vectorize_io(unsigned align, unsigned bit_size,
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
ntr_no_indirects_mask(nir_shader *s, struct pipe_screen *screen)
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

struct ntr_lower_tex_state {
   nir_scalar channels[8];
   unsigned i;
};

static void
nir_to_rc_lower_tex_instr_arg(nir_builder *b,
                                nir_tex_instr *instr,
                                nir_tex_src_type tex_src_type,
                                struct ntr_lower_tex_state *s)
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
nir_to_rc_lower_tex_instr(nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);

   if (nir_tex_instr_src_index(tex, nir_tex_src_coord) < 0)
      return false;

   b->cursor = nir_before_instr(instr);

   struct ntr_lower_tex_state s = {0};

   nir_to_rc_lower_tex_instr_arg(b, tex, nir_tex_src_coord, &s);
   /* We always have at least two slots for the coordinate, even on 1D. */
   s.i = MAX2(s.i, 2);

   nir_to_rc_lower_tex_instr_arg(b, tex, nir_tex_src_comparator, &s);
   s.i = MAX2(s.i, 3);

   nir_to_rc_lower_tex_instr_arg(b, tex, nir_tex_src_bias, &s);

   /* XXX: LZ */
   nir_to_rc_lower_tex_instr_arg(b, tex, nir_tex_src_lod, &s);
   nir_to_rc_lower_tex_instr_arg(b, tex, nir_tex_src_projector, &s);
   nir_to_rc_lower_tex_instr_arg(b, tex, nir_tex_src_ms_index, &s);

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
nir_to_rc_lower_tex(nir_shader *s)
{
   return nir_shader_instructions_pass(s,
                                       nir_to_rc_lower_tex_instr,
                                       nir_metadata_block_index |
                                       nir_metadata_dominance,
                                       NULL);
}

/* Lowers texture projectors if we can't do them as TGSI_OPCODE_TXP. */
static void
nir_to_rc_lower_txp(nir_shader *s)
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

const void *
nir_to_rc(struct nir_shader *s,
            struct pipe_screen *screen)
{
   static const struct nir_to_rc_options default_ntr_options = {0};
   return nir_to_rc_options(s, screen, &default_ntr_options);
}

/**
 * Translates the NIR shader to TGSI.
 *
 * This requires some lowering of the NIR shader to prepare it for translation.
 * We take ownership of the NIR shader passed, returning a reference to the new
 * TGSI tokens instead.  If you need to keep the NIR, then pass us a clone.
 */
const void *nir_to_rc_options(struct nir_shader *s,
                                struct pipe_screen *screen,
                                const struct nir_to_rc_options *options)
{
   struct ntr_compile *c;
   const void *tgsi_tokens;
   bool is_r500 = r300_screen(screen)->caps.is_r500;
   nir_variable_mode no_indirects_mask = ntr_no_indirects_mask(s, screen);

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

   NIR_PASS_V(s, nir_lower_io, nir_var_shader_in | nir_var_shader_out,
              type_size, (nir_lower_io_options)0);

   nir_to_rc_lower_txp(s);
   NIR_PASS_V(s, nir_to_rc_lower_tex);

   if (!s->options->lower_uniforms_to_ubo) {
      NIR_PASS_V(s, nir_lower_uniforms_to_ubo,
                 screen->get_param(screen, PIPE_CAP_PACKED_UNIFORMS),
                 true);
   }

   if (!screen->get_param(screen, PIPE_CAP_LOAD_CONSTBUF))
      NIR_PASS_V(s, nir_lower_ubo_vec4);

   bool progress;
   NIR_PASS_V(s, nir_opt_constant_folding);

   /* Clean up after triginometric input normalization. */
   NIR_PASS_V(s, nir_opt_vectorize, ntr_should_vectorize_instr, NULL);
   do {
      progress = false;
      NIR_PASS(progress, s, nir_opt_shrink_vectors);
   } while (progress);
   NIR_PASS_V(s, nir_copy_prop);
   NIR_PASS_V(s, nir_opt_cse);
   NIR_PASS_V(s, nir_opt_dce);
   NIR_PASS_V(s, nir_opt_shrink_stores, true);

   NIR_PASS_V(s, nir_lower_indirect_derefs, no_indirects_mask, UINT32_MAX);

   /* Lower demote_if to if (cond) { demote } because TGSI doesn't have a DEMOTE_IF. */
   NIR_PASS_V(s, nir_lower_discard_if, nir_lower_demote_if_to_cf);

   NIR_PASS_V(s, nir_lower_frexp);

   do {
      progress = false;
      NIR_PASS(progress, s, nir_opt_algebraic_late);
      if (progress) {
         NIR_PASS_V(s, nir_copy_prop);
         NIR_PASS_V(s, nir_opt_dce);
         NIR_PASS_V(s, nir_opt_cse);
      }
   } while (progress);

   if (s->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS_V(s, r300_nir_prepare_presubtract);
   }

   NIR_PASS_V(s, nir_lower_int_to_float);
   NIR_PASS_V(s, nir_copy_prop);
   NIR_PASS_V(s, r300_nir_post_integer_lowering);
   NIR_PASS_V(s, nir_lower_bool_to_float,
              !options->lower_cmp && !options->lower_fabs);
   /* bool_to_float generates MOVs for b2f32 that we want to clean up. */
   NIR_PASS_V(s, nir_copy_prop);
   /* CSE cleanup after late ftrunc lowering. */
   NIR_PASS_V(s, nir_opt_cse);
   /* At this point we need to clean;
    *  a) fcsel_gt that come from the ftrunc lowering on R300,
    *  b) all flavours of fcsels that read three different temp sources on R500.
    */
   if (s->info.stage == MESA_SHADER_VERTEX) {
      if (is_r500)
         NIR_PASS_V(s, r300_nir_lower_fcsel_r500);
      else
         NIR_PASS_V(s, r300_nir_lower_fcsel_r300);
      NIR_PASS_V(s, r300_nir_lower_flrp);
   }
   NIR_PASS_V(s, r300_nir_clean_double_fneg);
   NIR_PASS_V(s, nir_opt_dce);

   nir_move_options move_all =
       nir_move_const_undef | nir_move_load_ubo | nir_move_load_input |
       nir_move_comparisons | nir_move_copies | nir_move_load_ssbo;

   NIR_PASS_V(s, nir_opt_move, move_all);
   NIR_PASS_V(s, nir_move_vec_src_uses_to_dest, true);
   /* Late vectorizing after nir_move_vec_src_uses_to_dest helps instructions but
    * increases register usage. Testing shows this is beneficial only in VS.
    */
   if (s->info.stage == MESA_SHADER_VERTEX)
      NIR_PASS_V(s, nir_opt_vectorize, ntr_should_vectorize_instr, NULL);

   NIR_PASS_V(s, nir_convert_from_ssa, true);
   NIR_PASS_V(s, nir_lower_vec_to_regs, NULL, NULL);

   /* locals_to_reg_intrinsics will leave dead derefs that are good to clean up.
    */
   NIR_PASS_V(s, nir_lower_locals_to_regs, 32);
   NIR_PASS_V(s, nir_opt_dce);

   /* See comment in ntr_get_alu_src for supported modifiers */
   NIR_PASS_V(s, nir_legacy_trivialize, !options->lower_fabs);

   if (NIR_DEBUG(TGSI)) {
      fprintf(stderr, "NIR before translation to TGSI:\n");
      nir_print_shader(s, stderr);
   }

   c = rzalloc(NULL, struct ntr_compile);
   c->screen = screen;
   c->options = options;

   c->s = s;
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
   ntr_emit_impl(c, impl);
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
