/*
 * Copyright (c) 2017 Lima Project
 * Copyright (c) 2013 Connor Abbott
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef LIMA_IR_PP_PPIR_H
#define LIMA_IR_PP_PPIR_H

#include "util/u_math.h"
#include "util/list.h"
#include "util/set.h"

#include "ir/lima_ir.h"

typedef enum {
   ppir_op_unsupported = 0,
   ppir_op_mov,
   ppir_op_abs,
   ppir_op_neg,
   ppir_op_sat,
   ppir_op_add,

   ppir_op_ddx,
   ppir_op_ddy,

   ppir_op_mul,
   ppir_op_rcp,

   ppir_op_sin_lut,
   ppir_op_cos_lut,

   ppir_op_sum3,
   ppir_op_sum4,

   ppir_op_normalize2,
   ppir_op_normalize3,
   ppir_op_normalize4,

   ppir_op_select,

   ppir_op_sin,
   ppir_op_cos,
   ppir_op_tan,
   ppir_op_asin,
   ppir_op_acos,

   ppir_op_atan,
   ppir_op_atan2,
   ppir_op_atan_pt1,
   ppir_op_atan2_pt1,
   ppir_op_atan_pt2,

   ppir_op_exp,
   ppir_op_log,
   ppir_op_exp2,
   ppir_op_log2,
   ppir_op_sqrt,
   ppir_op_rsqrt,

   ppir_op_sign,
   ppir_op_floor,
   ppir_op_ceil,
   ppir_op_fract,
   ppir_op_mod,
   ppir_op_min,
   ppir_op_max,
   ppir_op_trunc,

   ppir_op_and,
   ppir_op_or,
   ppir_op_xor,

   ppir_op_lt,
   ppir_op_gt,
   ppir_op_le,
   ppir_op_ge,
   ppir_op_eq,
   ppir_op_ne,
   ppir_op_not,

   ppir_op_load_uniform,
   ppir_op_load_varying,
   ppir_op_load_coords,
   ppir_op_load_coords_reg,
   ppir_op_load_fragcoord,
   ppir_op_load_pointcoord,
   ppir_op_load_frontface,
   ppir_op_load_texture,
   ppir_op_load_temp,

   ppir_op_store_temp,

   ppir_op_const,

   ppir_op_discard,
   ppir_op_branch,

   ppir_op_undef,
   ppir_op_dummy,

   ppir_op_num,
} ppir_op;

typedef enum {
   ppir_node_type_alu,
   ppir_node_type_const,
   ppir_node_type_load,
   ppir_node_type_store,
   ppir_node_type_load_texture,
   ppir_node_type_discard,
   ppir_node_type_branch,
} ppir_node_type;

typedef struct {
   char *name;
   ppir_node_type type;
   int *slots;
} ppir_op_info;

extern const ppir_op_info ppir_op_infos[];

typedef enum {
   ppir_dep_src,
   ppir_dep_write_after_read,
   ppir_dep_sequence,
} ppir_dep_type;

typedef struct {
   void *pred, *succ;
   ppir_dep_type type;
   struct list_head pred_link;
   struct list_head succ_link;
} ppir_dep;

typedef struct ppir_node {
   struct list_head list;
   struct list_head sched_list;
   ppir_op op;
   ppir_node_type type;
   int index;
   char name[16];
   bool printed;
   struct ppir_instr *instr;
   int instr_pos;
   struct ppir_block *block;
   bool is_out;
   bool succ_different_block;

   /* for scheduler */
   struct list_head succ_list;
   struct list_head pred_list;
} ppir_node;

typedef enum {
   ppir_pipeline_reg_const0,
   ppir_pipeline_reg_const1,
   ppir_pipeline_reg_sampler,
   ppir_pipeline_reg_uniform,
   ppir_pipeline_reg_vmul,
   ppir_pipeline_reg_fmul,
   ppir_pipeline_reg_discard, /* varying load */
} ppir_pipeline;

typedef enum {
   ppir_output_color0,
   ppir_output_color1,
   ppir_output_depth,
   ppir_output_num,
   ppir_output_invalid = -1,
} ppir_output_type;

static inline const char *ppir_output_type_to_str(ppir_output_type type)
{
   switch (type) {
   case ppir_output_color0:
      return "OUTPUT_COLOR0";
   case ppir_output_color1:
      return "OUTPUT_COLOR1";
   case ppir_output_depth:
      return "OUTPUT_DEPTH";
   default:
      return "INVALID";
   }
}

static inline ppir_output_type ppir_nir_output_to_ppir(gl_frag_result res, int dual_src_index)
{
   switch (res) {
   case FRAG_RESULT_COLOR:
   case FRAG_RESULT_DATA0:
      return ppir_output_color0 + dual_src_index;
   case FRAG_RESULT_DEPTH:
      return ppir_output_depth;
   default:
      return ppir_output_invalid;
   }
}

typedef struct ppir_reg {
   struct list_head list;
   int index;
   ppir_output_type out_type;
   int regalloc_index;
   int num_components;

   /* whether this reg has to start from the x component
    * of a full physical reg, this is true for reg used
    * in load/store instr which has no swizzle field */
   bool is_head;
   bool spilled;
   bool undef;
   bool out_reg;
} ppir_reg;

typedef enum {
   ppir_target_ssa,
   ppir_target_pipeline,
   ppir_target_register,
} ppir_target;

typedef struct ppir_src {
   ppir_target type;
   ppir_node *node;

   union {
      ppir_reg *ssa;
      ppir_reg *reg;
      ppir_pipeline pipeline;
   };

   uint8_t swizzle[4];
   bool absolute, negate;
} ppir_src;

typedef enum {
   ppir_outmod_none,
   ppir_outmod_clamp_fraction,
   ppir_outmod_clamp_positive,
   ppir_outmod_round,
} ppir_outmod;

typedef struct ppir_dest {
   ppir_target type;

   union {
      ppir_reg ssa;
      ppir_reg *reg;
      ppir_pipeline pipeline;
   };

   ppir_outmod modifier;
   unsigned write_mask : 4;
} ppir_dest;

typedef struct {
   ppir_node node;
   ppir_dest dest;
   ppir_src src[3];
   int num_src;
   int shift : 3; /* Only used for ppir_op_mul */
} ppir_alu_node;

typedef struct ppir_const {
   union fi value[4];
   int num;
} ppir_const;

typedef struct {
   ppir_node node;
   ppir_const constant;
   ppir_dest dest;
} ppir_const_node;

typedef enum {
   ppir_perspective_none = 0,
   ppir_perspective_z,
   ppir_perspective_w,
} ppir_perspective;

typedef struct {
   ppir_node node;
   int index;
   int num_components;
   ppir_dest dest;
   ppir_src src;
   int num_src;
   ppir_perspective perspective;
   int sampler_dim;
} ppir_load_node;

typedef struct {
   ppir_node node;
   int index;
   int num_components;
   ppir_src src;
} ppir_store_node;

typedef struct {
   ppir_node node;
   ppir_dest dest;
   ppir_src src[2];
   int num_src;
   int sampler;
   int sampler_dim;
   bool lod_bias_en;
   bool explicit_lod;
} ppir_load_texture_node;

typedef struct {
   ppir_node node;
} ppir_discard_node;

enum ppir_instr_slot {
   PPIR_INSTR_SLOT_VARYING,
   PPIR_INSTR_SLOT_TEXLD,
   PPIR_INSTR_SLOT_UNIFORM,
   PPIR_INSTR_SLOT_ALU_VEC_MUL,
   PPIR_INSTR_SLOT_ALU_SCL_MUL,
   PPIR_INSTR_SLOT_ALU_VEC_ADD,
   PPIR_INSTR_SLOT_ALU_SCL_ADD,
   PPIR_INSTR_SLOT_ALU_COMBINE,
   PPIR_INSTR_SLOT_STORE_TEMP,
   PPIR_INSTR_SLOT_BRANCH,
   PPIR_INSTR_SLOT_NUM,
   PPIR_INSTR_SLOT_END,
   PPIR_INSTR_SLOT_ALU_START = PPIR_INSTR_SLOT_ALU_VEC_MUL,
   PPIR_INSTR_SLOT_ALU_END = PPIR_INSTR_SLOT_ALU_COMBINE,
};

typedef struct ppir_instr {
   struct list_head list;
   int index;
   bool printed;
   int seq; /* command sequence after schedule */

   ppir_node *slots[PPIR_INSTR_SLOT_NUM];
   ppir_const constant[2];
   bool stop;

   /* for scheduler */
   struct list_head succ_list;
   struct list_head pred_list;
   float reg_pressure;
   int est; /* earliest start time */
   int parent_index;
   bool scheduled;
   int offset;
   int encode_size;

   /* for liveness analysis */
   BITSET_WORD *live_set;
   uint8_t *live_mask; /* mask for non-ssa registers */
   /* live_internal is to mark registers only live within an
    * instruction, without propagation */
   BITSET_WORD *live_internal;
} ppir_instr;

typedef struct ppir_block {
   struct list_head list;
   struct list_head node_list;
   struct list_head instr_list;
   bool stop;

   struct ppir_block *successors[2];

   struct ppir_compiler *comp;

   /* for scheduler */
   int sched_instr_index;
   int sched_instr_base;
   int index;
} ppir_block;

typedef struct {
   ppir_node node;
   ppir_src src[2];
   int num_src;
   bool cond_gt;
   bool cond_eq;
   bool cond_lt;
   bool negate;
   ppir_block *target;
} ppir_branch_node;

struct ra_regs;
struct lima_fs_compiled_shader;

typedef struct ppir_compiler {
   struct list_head block_list;
   struct hash_table_u64 *blocks;
   int cur_index;
   int cur_instr_index;
   int *out_type_to_reg;

   struct list_head reg_list;
   int reg_num;

   /* array for searching ssa/reg node */
   ppir_node **var_nodes;

   struct ra_regs *ra;
   struct lima_fs_compiled_shader *prog;
   bool uses_discard;
   bool dual_source_blend;

   /* for scheduler */
   int sched_instr_base;

   /* for regalloc spilling debug */
   int force_spilling;

   /* shaderdb */
   int num_loops;
   int num_spills;
   int num_fills;

   ppir_block *discard_block;
   ppir_block *current_block;
   ppir_block *loop_break_block;
   ppir_block *loop_cont_block;
} ppir_compiler;

void *ppir_node_create(ppir_block *block, ppir_op op, int index, unsigned mask);
void ppir_node_add_dep(ppir_node *succ, ppir_node *pred, ppir_dep_type type);
void ppir_node_remove_dep(ppir_dep *dep);
void ppir_node_delete(ppir_node *node);
void ppir_node_print_prog(ppir_compiler *comp);
void ppir_node_replace_child(ppir_node *parent, ppir_node *old_child, ppir_node *new_child);
void ppir_node_replace_all_succ(ppir_node *dst, ppir_node *src);
void ppir_node_replace_pred(ppir_dep *dep, ppir_node *new_pred);
ppir_dep *ppir_dep_for_pred(ppir_node *node, ppir_node *pred);
/* Assumes that node successors are in the same block */
ppir_node *ppir_node_insert_mov(ppir_node *node);

static inline bool ppir_node_is_root(ppir_node *node)
{
   return list_is_empty(&node->succ_list);
}

static inline bool ppir_node_is_leaf(ppir_node *node)
{
   return list_is_empty(&node->pred_list);
}

static inline bool ppir_node_has_single_succ(ppir_node *node)
{
   return list_is_singular(&node->succ_list)
      && !node->succ_different_block;
}

bool ppir_node_has_single_src_succ(ppir_node *node);

static inline ppir_node *ppir_node_first_succ(ppir_node *node)
{
   return list_first_entry(&node->succ_list, ppir_dep, succ_link)->succ;
}

static inline bool ppir_node_has_single_pred(ppir_node *node)
{
   return list_is_singular(&node->pred_list);
}

static inline ppir_node *ppir_node_first_pred(ppir_node *node)
{
   return list_first_entry(&node->pred_list, ppir_dep, pred_link)->pred;
}

#define ppir_node_foreach_succ(node, dep) \
   list_for_each_entry(ppir_dep, dep, &node->succ_list, succ_link)
#define ppir_node_foreach_succ_safe(node, dep) \
   list_for_each_entry_safe(ppir_dep, dep, &node->succ_list, succ_link)
#define ppir_node_foreach_pred(node, dep) \
   list_for_each_entry(ppir_dep, dep, &node->pred_list, pred_link)
#define ppir_node_foreach_pred_safe(node, dep) \
   list_for_each_entry_safe(ppir_dep, dep, &node->pred_list, pred_link)

#define ppir_node_to_alu(node) ((ppir_alu_node *)(node))
#define ppir_node_to_const(node) ((ppir_const_node *)(node))
#define ppir_node_to_load(node) ((ppir_load_node *)(node))
#define ppir_node_to_store(node) ((ppir_store_node *)(node))
#define ppir_node_to_load_texture(node) ((ppir_load_texture_node *)(node))
#define ppir_node_to_discard(node) ((ppir_discard_node *)(node))
#define ppir_node_to_branch(node) ((ppir_branch_node *)(node))

static inline ppir_dest *ppir_node_get_dest(ppir_node *node)
{
   assert(node);
   switch (node->type) {
   case ppir_node_type_alu:
      return &ppir_node_to_alu(node)->dest;
   case ppir_node_type_load:
      return &ppir_node_to_load(node)->dest;
   case ppir_node_type_const:
      return &ppir_node_to_const(node)->dest;
   case ppir_node_type_load_texture:
      return &ppir_node_to_load_texture(node)->dest;
   default:
      return NULL;
   }
}

static inline int ppir_node_get_src_num(ppir_node *node)
{
   assert(node);
   switch (node->type) {
   case ppir_node_type_alu:
      return ppir_node_to_alu(node)->num_src;
   case ppir_node_type_branch:
      return ppir_node_to_branch(node)->num_src;
   case ppir_node_type_load:
      return ppir_node_to_load(node)->num_src;
   case ppir_node_type_load_texture:
      return ppir_node_to_load_texture(node)->num_src;
   case ppir_node_type_store:
      return 1;
   default:
      return 0;
   }

   return 0;
}

static inline ppir_src *ppir_node_get_src(ppir_node *node, int idx)
{
   if (idx < 0 || idx >= ppir_node_get_src_num(node))
      return NULL;

   switch (node->type) {
   case ppir_node_type_alu:
      return &ppir_node_to_alu(node)->src[idx];
   case ppir_node_type_branch:
      return &ppir_node_to_branch(node)->src[idx];
   case ppir_node_type_load_texture:
      return &ppir_node_to_load_texture(node)->src[idx];
   case ppir_node_type_load:
      return &ppir_node_to_load(node)->src;
   case ppir_node_type_store:
      return &ppir_node_to_store(node)->src;
   default:
      break;
   }

   return NULL;
}

static inline ppir_reg *ppir_src_get_reg(ppir_src *src)
{
   switch (src->type) {
   case ppir_target_ssa:
      return src->ssa;
   case ppir_target_register:
      return src->reg;
   default:
      return NULL;
   }
}

static inline ppir_reg *ppir_dest_get_reg(ppir_dest *dest)
{
   switch (dest->type) {
   case ppir_target_ssa:
      return &dest->ssa;
   case ppir_target_register:
      return dest->reg;
   default:
      return NULL;
   }
}

static inline void ppir_node_target_assign(ppir_src *src, ppir_node *node)
{
   ppir_dest *dest = ppir_node_get_dest(node);
   src->type = dest->type;
   switch (src->type) {
   case ppir_target_ssa:
      src->ssa = &dest->ssa;
      src->node = node;
      break;
   case ppir_target_register:
      src->reg = dest->reg;
      /* Registers can be assigned from multiple nodes, so don't keep
       * pointer to the node here
       */
      src->node = NULL;
      break;
   case ppir_target_pipeline:
      src->pipeline = dest->pipeline;
      src->node = node;
      break;
   }
}

static inline bool ppir_node_target_equal(ppir_src *src, ppir_dest *dest)
{
   if (src->type != dest->type ||
       (src->type == ppir_target_ssa && src->ssa != &dest->ssa) ||
       (src->type == ppir_target_register && src->reg != dest->reg) ||
       (src->type == ppir_target_pipeline && src->pipeline != dest->pipeline))
      return false;

   return true;
}

static inline int ppir_target_get_src_reg_index(ppir_src *src)
{
   switch (src->type) {
   case ppir_target_ssa:
      if (src->ssa)
         return src->ssa->index;
      break;
   case ppir_target_register:
      if (src->reg)
         return src->reg->index;
      break;
   case ppir_target_pipeline:
      if (src->pipeline == ppir_pipeline_reg_discard)
         return 15 * 4;
      return (src->pipeline + 12) * 4;
   }

   return -1;
}

static inline int ppir_target_get_dest_reg_index(ppir_dest *dest)
{
   switch (dest->type) {
   case ppir_target_ssa:
      return dest->ssa.index;
   case ppir_target_register:
      return dest->reg->index;
   case ppir_target_pipeline:
      if (dest->pipeline == ppir_pipeline_reg_discard)
         return 15 * 4;
      return (dest->pipeline + 12) * 4;
   }

   return -1;
}

static inline int ppir_src_get_mask(ppir_src *src)
{
   ppir_reg *reg = ppir_src_get_reg(src);
   int mask = 0;

   for (int i = 0; i < reg->num_components; i++)
      mask |= (1 << src->swizzle[i]);

   return mask;
}

static inline bool ppir_target_is_scalar(ppir_dest *dest)
{
   switch (dest->type) {
   case ppir_target_ssa:
      return dest->ssa.num_components == 1;
   case ppir_target_register:
      /* only one bit in mask is set */
      if ((dest->write_mask & 0x3) == 0x3 ||
          (dest->write_mask & 0x5) == 0x5 ||
          (dest->write_mask & 0x9) == 0x9 ||
          (dest->write_mask & 0x6) == 0x6 ||
          (dest->write_mask & 0xa) == 0xa ||
          (dest->write_mask & 0xc) == 0xc)
         return false;
      else
         return true;
   case ppir_target_pipeline:
      if (dest->pipeline == ppir_pipeline_reg_fmul)
         return true;
      else
         return false;
   default:
      return false;
   }
}

static inline bool ppir_node_schedulable_slot(ppir_node *node,
                                              enum ppir_instr_slot slot)
{
   int *slots = ppir_op_infos[node->op].slots;
   for (int i = 0; slots[i] != PPIR_INSTR_SLOT_END; i++)
      if (slots[i] == slot)
         return true;

   return false;
}

ppir_instr *ppir_instr_create(ppir_block *block);
bool ppir_instr_insert_node(ppir_instr *instr, ppir_node *node);
void ppir_instr_add_dep(ppir_instr *succ, ppir_instr *pred);
void ppir_instr_print_list(ppir_compiler *comp);
void ppir_instr_print_dep(ppir_compiler *comp);
void ppir_instr_insert_mul_node(ppir_node *add, ppir_node *mul);

#define ppir_instr_foreach_succ(instr, dep) \
   list_for_each_entry(ppir_dep, dep, &instr->succ_list, succ_link)
#define ppir_instr_foreach_succ_safe(instr, dep) \
   list_for_each_entry_safe(ppir_dep, dep, &instr->succ_list, succ_link)
#define ppir_instr_foreach_pred(instr, dep) \
   list_for_each_entry(ppir_dep, dep, &instr->pred_list, pred_link)
#define ppir_instr_foreach_pred_safe(instr, dep) \
   list_for_each_entry_safe(ppir_dep, dep, &instr->pred_list, pred_link)

static inline bool ppir_instr_is_root(ppir_instr *instr)
{
   return list_is_empty(&instr->succ_list);
}

static inline bool ppir_instr_is_leaf(ppir_instr *instr)
{
   return list_is_empty(&instr->pred_list);
}

bool ppir_lower_prog(ppir_compiler *comp);
bool ppir_node_to_instr(ppir_compiler *comp);
bool ppir_schedule_prog(ppir_compiler *comp);
bool ppir_regalloc_prog(ppir_compiler *comp);
bool ppir_codegen_prog(ppir_compiler *comp);
void ppir_liveness_analysis(ppir_compiler *comp);

static inline unsigned int reg_mask_size(unsigned int num_reg)
{
   return (num_reg + 1) / 2;
}

static inline uint8_t get_reg_mask(uint8_t *set, unsigned index)
{
   unsigned int i = index / 2;
   unsigned int shift = index % 2 ? 4 : 0;
   uint8_t mask = 0x0f << shift;
   return (set[i] & mask) >> shift;
}

static inline void set_reg_mask(uint8_t *set, unsigned int index, uint8_t bits)
{
   unsigned int i = index / 2;
   unsigned int shift = index % 2 ? 4 : 0;
   uint8_t mask = 0x0f << shift;
   set[i] &= ~mask;
   set[i] |= (bits << shift);
}

#endif
