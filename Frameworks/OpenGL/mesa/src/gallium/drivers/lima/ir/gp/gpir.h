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

#ifndef LIMA_IR_GP_GPIR_H
#define LIMA_IR_GP_GPIR_H

#include "util/list.h"
#include "util/u_math.h"

#include "ir/lima_ir.h"

/* list of operations that a node can do. */
typedef enum {
   gpir_op_unsupported = 0,
   gpir_op_mov,

   /* mul ops */
   gpir_op_mul,
   gpir_op_select,
   gpir_op_complex1,
   gpir_op_complex2,

   /* add ops */
   gpir_op_add,
   gpir_op_floor,
   gpir_op_sign,
   gpir_op_ge,
   gpir_op_lt,
   gpir_op_min,
   gpir_op_max,
   gpir_op_abs,
   gpir_op_not,

   /* mul/add ops */
   gpir_op_neg,

   /* passthrough ops */
   gpir_op_clamp_const,
   gpir_op_preexp2,
   gpir_op_postlog2,

   /* complex ops */
   gpir_op_exp2_impl,
   gpir_op_log2_impl,
   gpir_op_rcp_impl,
   gpir_op_rsqrt_impl,

   /* load/store ops */
   gpir_op_load_uniform,
   gpir_op_load_temp,
   gpir_op_load_attribute,
   gpir_op_load_reg,
   gpir_op_store_temp,
   gpir_op_store_reg,
   gpir_op_store_varying,
   gpir_op_store_temp_load_off0,
   gpir_op_store_temp_load_off1,
   gpir_op_store_temp_load_off2,

   /* branch */
   gpir_op_branch_cond,

   /* const (emulated) */
   gpir_op_const,

   /* emulated ops */
   gpir_op_exp2,
   gpir_op_log2,
   gpir_op_rcp,
   gpir_op_rsqrt,
   gpir_op_ceil,
   gpir_op_exp,
   gpir_op_log,
   gpir_op_sin,
   gpir_op_cos,
   gpir_op_tan,
   gpir_op_branch_uncond,
   gpir_op_eq,
   gpir_op_ne,

   /* auxiliary ops */
   gpir_op_dummy_f,
   gpir_op_dummy_m,

   gpir_op_num,
} gpir_op;

typedef enum {
   gpir_node_type_alu,
   gpir_node_type_const,
   gpir_node_type_load,
   gpir_node_type_store,
   gpir_node_type_branch,
} gpir_node_type;

typedef struct {
   char *name;
   bool dest_neg;
   bool src_neg[4];
   int *slots;
   gpir_node_type type;
   bool spillless;
   bool schedule_first;
   bool may_consume_two_slots;
} gpir_op_info;

extern const gpir_op_info gpir_op_infos[];

typedef struct {
   enum {
      GPIR_DEP_INPUT,     /* def is the input of use */
      GPIR_DEP_OFFSET,    /* def is the offset of use (i.e. temp store) */
      GPIR_DEP_READ_AFTER_WRITE,
      GPIR_DEP_WRITE_AFTER_READ,
   } type;

   /* node execute before succ */
   struct gpir_node *pred;
   /* node execute after pred */
   struct gpir_node *succ;

   /* for node pred_list */
   struct list_head pred_link;
   /* for ndoe succ_list */
   struct list_head succ_link;
} gpir_dep;

struct gpir_instr;
struct gpir_store_node;

typedef struct gpir_node {
   struct list_head list;
   gpir_op op;
   gpir_node_type type;
   int index;
   char name[16];
   bool printed;
   struct gpir_block *block;

   /* for nodes relationship */
   /* for node who uses this node (successor) */
   struct list_head succ_list;
   /* for node this node uses (predecessor) */
   struct list_head pred_list;

   /* for scheduler and regalloc */
   int value_reg;
   union {
      struct {
         struct gpir_instr *instr;
         struct gpir_store_node *physreg_store;
         int pos;
         int dist;
         int index;
         bool ready;
         bool inserted;
         bool max_node, next_max_node;
         bool complex_allowed;
      } sched;
      struct {
         int parent_index;
         float reg_pressure;
         int est;
         bool scheduled;
      } rsched;
      struct {
         float index;
         struct gpir_node *last;
      } vreg;
      struct {
         int index;
      } preg;
   };
} gpir_node;

typedef struct {
   gpir_node node;

   gpir_node *children[3];
   bool children_negate[3];
   int num_child;

   bool dest_negate;
} gpir_alu_node;

typedef struct {
   gpir_node node;
   union fi value;
} gpir_const_node;

typedef struct {
   int index;
   struct list_head list;
} gpir_reg;

typedef struct {
   gpir_node node;

   unsigned index;
   unsigned component;

   gpir_reg *reg;
   struct list_head reg_link;
} gpir_load_node;

typedef struct gpir_store_node {
   gpir_node node;

   unsigned index;
   unsigned component;
   gpir_node *child;

   gpir_reg *reg;
} gpir_store_node;

enum gpir_instr_slot {
   GPIR_INSTR_SLOT_MUL0,
   GPIR_INSTR_SLOT_MUL1,
   GPIR_INSTR_SLOT_ADD0,
   GPIR_INSTR_SLOT_ADD1,
   GPIR_INSTR_SLOT_PASS,
   GPIR_INSTR_SLOT_COMPLEX,
   GPIR_INSTR_SLOT_REG0_LOAD0,
   GPIR_INSTR_SLOT_REG0_LOAD1,
   GPIR_INSTR_SLOT_REG0_LOAD2,
   GPIR_INSTR_SLOT_REG0_LOAD3,
   GPIR_INSTR_SLOT_REG1_LOAD0,
   GPIR_INSTR_SLOT_REG1_LOAD1,
   GPIR_INSTR_SLOT_REG1_LOAD2,
   GPIR_INSTR_SLOT_REG1_LOAD3,
   GPIR_INSTR_SLOT_MEM_LOAD0,
   GPIR_INSTR_SLOT_MEM_LOAD1,
   GPIR_INSTR_SLOT_MEM_LOAD2,
   GPIR_INSTR_SLOT_MEM_LOAD3,
   GPIR_INSTR_SLOT_STORE0,
   GPIR_INSTR_SLOT_STORE1,
   GPIR_INSTR_SLOT_STORE2,
   GPIR_INSTR_SLOT_STORE3,
   GPIR_INSTR_SLOT_NUM,
   GPIR_INSTR_SLOT_END,
   GPIR_INSTR_SLOT_ALU_BEGIN      = GPIR_INSTR_SLOT_MUL0,
   GPIR_INSTR_SLOT_ALU_END        = GPIR_INSTR_SLOT_COMPLEX,
   GPIR_INSTR_SLOT_DIST_TWO_BEGIN = GPIR_INSTR_SLOT_MUL0,
   GPIR_INSTR_SLOT_DIST_TWO_END   = GPIR_INSTR_SLOT_PASS,
};

typedef struct gpir_instr {
   int index;
   struct list_head list;

   gpir_node *slots[GPIR_INSTR_SLOT_NUM];

   /* The number of ALU slots free for moves. */
   int alu_num_slot_free;

   /* The number of ALU slots free for moves, except for the complex slot. */
   int alu_non_cplx_slot_free;

   /* We need to make sure that we can insert moves in the following cases:
    * (1) There was a use of a value two cycles ago.
    * (2) There were more than 5 uses of a value 1 cycle ago (or else we can't
    *     possibly satisfy (1) for the next cycle).
    * (3) There is a store instruction scheduled, but not its child.
    *
    * The complex slot cannot be used for a move in case (1), since it only
    * has a FIFO depth of 1, but it can be used for (2) as well as (3) as long
    * as the uses aren't in certain slots. It turns out that we don't have to
    * worry about nodes that can't use the complex slot for (2), since there
    * are at most 4 uses 1 cycle ago that can't use the complex slot, but we
    * do have to worry about (3). This means tracking stores whose children
    * cannot be in the complex slot. In order to ensure that we have enough
    * space for all three, we maintain the following invariants:
    *
    * (1) alu_num_slot_free >= alu_num_slot_needed_by_store +
    *       alu_num_slot_needed_by_max +
    *       max(alu_num_unscheduled_next_max - alu_max_allowed_next_max, 0)
    * (2) alu_non_cplx_slot_free >= alu_num_slot_needed_by_max +
    *       alu_num_slot_needed_by_non_cplx_store
    *
    * alu_max_allowed_next_max is normally 5 (since there can be at most 5 max
    * nodes for the next instruction) but when there is a complex1 node in
    * this instruction it reduces to 4 to reserve a slot for complex2 in the
    * next instruction.
    */
   int alu_num_slot_needed_by_store;
   int alu_num_slot_needed_by_non_cplx_store;
   int alu_num_slot_needed_by_max;
   int alu_num_unscheduled_next_max;
   int alu_max_allowed_next_max;

   /* Used to communicate to the scheduler how many slots need to be cleared
    * up in order to satisfy the invariants.
    */
   int slot_difference;
   int non_cplx_slot_difference;

   int reg0_use_count;
   bool reg0_is_attr;
   int reg0_index;

   int reg1_use_count;
   int reg1_index;

   int mem_use_count;
   bool mem_is_temp;
   int mem_index;

   enum {
      GPIR_INSTR_STORE_NONE,
      GPIR_INSTR_STORE_VARYING,
      GPIR_INSTR_STORE_REG,
      GPIR_INSTR_STORE_TEMP,
   } store_content[2];
   int store_index[2];
} gpir_instr;

typedef struct gpir_block {
   struct list_head list;
   struct list_head node_list;
   struct list_head instr_list;
   struct gpir_compiler *comp;

   struct gpir_block *successors[2];
   struct list_head predecessors;
   struct list_head predecessors_node;

   /* for regalloc */

   /* The set of live registers, i.e. registers whose value may be used
    * eventually, at the beginning of the block.
    */
   BITSET_WORD *live_in;

   /* Set of live registers at the end of the block. */
   BITSET_WORD *live_out;

   /* Set of registers that may have a value defined at the end of the
    * block.
    */
   BITSET_WORD *def_out;

   /* After register allocation, the set of live physical registers at the end
    * of the block. Needed for scheduling.
    */
   uint64_t live_out_phys;

   /* For codegen, the offset in the final program. */
   unsigned instr_offset;

   /* for scheduler */
   union {
      struct {
         int instr_index;
      } sched;
      struct {
         int node_index;
      } rsched;
   };
} gpir_block;

typedef struct {
   gpir_node node;
   gpir_block *dest;
   gpir_node *cond;
} gpir_branch_node;

struct lima_vs_compiled_shader;

#define GPIR_VECTOR_SSA_VIEWPORT_SCALE  0
#define GPIR_VECTOR_SSA_VIEWPORT_OFFSET 1
#define GPIR_VECTOR_SSA_NUM 2

typedef struct gpir_compiler {
   struct list_head block_list;
   int cur_index;

   /* Find the gpir node for a given NIR SSA def. */
   gpir_node **node_for_ssa;

   /* Find the gpir register for a given NIR SSA def. */
   gpir_reg **reg_for_ssa;

   /* gpir block for NIR block. */
   gpir_block **blocks;

   /* for physical reg */
   struct list_head reg_list;
   int cur_reg;

   /* lookup for vector ssa */
   struct {
      int ssa;
      gpir_node *nodes[4];
   } vector_ssa[GPIR_VECTOR_SSA_NUM];

   struct lima_vs_compiled_shader *prog;
   int constant_base;

   /* shaderdb */
   int num_instr;
   int num_loops;
   int num_spills;
   int num_fills;
} gpir_compiler;

#define GPIR_VALUE_REG_NUM 11
#define GPIR_PHYSICAL_REG_NUM 64

void *gpir_node_create(gpir_block *block, gpir_op op);
gpir_dep *gpir_node_add_dep(gpir_node *succ, gpir_node *pred, int type);
void gpir_node_remove_dep(gpir_node *succ, gpir_node *pred);
void gpir_node_replace_succ(gpir_node *dst, gpir_node *src);
void gpir_node_replace_pred(gpir_dep *dep, gpir_node *new_pred);
void gpir_node_replace_child(gpir_node *parent, gpir_node *old_child, gpir_node *new_child);
void gpir_node_insert_child(gpir_node *parent, gpir_node *child, gpir_node *insert_child);
void gpir_node_delete(gpir_node *node);
void gpir_node_print_prog_dep(gpir_compiler *comp);
void gpir_node_print_prog_seq(gpir_compiler *comp);

#define gpir_node_foreach_succ(node, dep) \
   list_for_each_entry(gpir_dep, dep, &node->succ_list, succ_link)
#define gpir_node_foreach_succ_safe(node, dep) \
   list_for_each_entry_safe(gpir_dep, dep, &node->succ_list, succ_link)
#define gpir_node_foreach_pred(node, dep) \
   list_for_each_entry(gpir_dep, dep, &node->pred_list, pred_link)
#define gpir_node_foreach_pred_safe(node, dep) \
   list_for_each_entry_safe(gpir_dep, dep, &node->pred_list, pred_link)

static inline bool gpir_node_is_root(gpir_node *node)
{
   return list_is_empty(&node->succ_list);
}

static inline bool gpir_node_is_leaf(gpir_node *node)
{
   return list_is_empty(&node->pred_list);
}

#define gpir_node_to_alu(node) ((gpir_alu_node *)(node))
#define gpir_node_to_const(node) ((gpir_const_node *)(node))
#define gpir_node_to_load(node) ((gpir_load_node *)(node))
#define gpir_node_to_store(node) ((gpir_store_node *)(node))
#define gpir_node_to_branch(node) ((gpir_branch_node *)(node))

gpir_instr *gpir_instr_create(gpir_block *block);
bool gpir_instr_try_insert_node(gpir_instr *instr, gpir_node *node);
void gpir_instr_remove_node(gpir_instr *instr, gpir_node *node);
void gpir_instr_print_prog(gpir_compiler *comp);

bool gpir_codegen_acc_same_op(gpir_op op1, gpir_op op2);

bool gpir_optimize(gpir_compiler *comp);
bool gpir_pre_rsched_lower_prog(gpir_compiler *comp);
bool gpir_reduce_reg_pressure_schedule_prog(gpir_compiler *comp);
bool gpir_regalloc_prog(gpir_compiler *comp);
bool gpir_schedule_prog(gpir_compiler *comp);
bool gpir_codegen_prog(gpir_compiler *comp);

gpir_reg *gpir_create_reg(gpir_compiler *comp);

#endif
