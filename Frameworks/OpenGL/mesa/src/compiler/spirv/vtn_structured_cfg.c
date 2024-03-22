/*
 * Copyright © 2015-2023 Intel Corporation
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

#include "vtn_private.h"
#include "spirv_info.h"
#include "util/u_math.h"

/* Handle SPIR-V structured control flow, mapping SPIR-V constructs into
 * equivalent NIR constructs.
 *
 * Because SPIR-V can represent more complex control flow than NIR, some
 * constructs are mapped into a combination of nir_if and nir_loop nodes.  For
 * example, an selection construct with an "if-break" (an early branch into
 * the end of the construct) will be mapped into NIR as a loop (to allow the
 * break) with a nested if (to handle the actual selection).
 *
 * Note that using NIR loops this way requires us to propagate breaks and
 * continues that are meant to outer constructs when a nir_loop is used for a
 * SPIR-V construct other than Loop.
 *
 * The process of identifying and ordering the blocks before the NIR
 * translation is similar to what's done in Tint, using the "reverse
 * structured post-order traversal".  See also the file comments
 * src/reader/spirv/function.cc in the Tint repository.
 */

enum vtn_construct_type {
   /* Not formally a SPIR-V construct but used to represent the entire
    * function.
    */
   vtn_construct_type_function,

   /* Selection construct uses a nir_if and optionally a nir_loop to handle
    * if-breaks.
    */
   vtn_construct_type_selection,

   /* Loop construct uses a nir_loop and optionally a nir_if to handle an
    * OpBranchConditional as part of the head of the loop.
    */
   vtn_construct_type_loop,

   /* Continue construct maps to the NIR continue construct of the corresponding
    * loop.  For convenience, unlike in SPIR-V, the parent of this construct is
    * always the loop construct.  Continue construct is omitted for single-block
    * loops.
    */
   vtn_construct_type_continue,

   /* Switch construct is not directly mapped into any NIR structure, the work
    * is handled by the case constructs.  It does keep a nir_variable for
    * handling case fallback logic.
    */
   vtn_construct_type_switch,

   /* Case construct uses a nir_if and optionally a nir_loop to handle early
    * breaks.  Note switch_breaks are handled by each case.
    */
   vtn_construct_type_case,
};

static const char *
vtn_construct_type_to_string(enum vtn_construct_type t)
{
#define CASE(typ) case vtn_construct_type_##typ: return #typ
   switch (t) {
   CASE(function);
   CASE(selection);
   CASE(loop);
   CASE(continue);
   CASE(switch);
   CASE(case);
   }
#undef CASE
   unreachable("invalid construct type");
   return "";
}

struct vtn_construct {
   enum vtn_construct_type type;

   bool needs_nloop;
   bool needs_break_propagation;
   bool needs_continue_propagation;
   bool needs_fallthrough;

   struct vtn_construct *parent;

   struct vtn_construct *innermost_loop;
   struct vtn_construct *innermost_switch;
   struct vtn_construct *innermost_case;

   unsigned start_pos;
   unsigned end_pos;

   /* Usually the same as end_pos, but may be different in case of an "early
    * merge" after divergence caused by an OpBranchConditional.  This can
    * happen in selection and loop constructs.
    */
   unsigned merge_pos;

   /* Valid when not zero, indicates the block that starts the then and else
    * paths in a condition.  This may be used by selection constructs.
    */
   unsigned then_pos;
   unsigned else_pos;

   /* Indicates where the continue block is, marking the end of the body of
    * the loop.  Note the block ordering will always give us first the loop
    * body blocks then the continue block.  Used by loop construct.
    */
   unsigned continue_pos;

   /* For the list of all constructs in vtn_function. */
   struct list_head link;

   /* NIR nodes that are associated with this construct.  See
    * vtn_construct_type for an overview.
    */
   nir_loop *nloop;
   nir_if *nif;

   /* This variable will be set by an inner construct to indicate that a break
    * is necessary.  We need to use variables here for situations when the
    * inner construct has a loop of its own for other reasons.
    */
   nir_variable *break_var;

   /* Same logic but for continue. */
   nir_variable *continue_var;

   /* This is used by each case to force entering in the case regardless of
    * the condition.  We always set it when handling a branch that is a
    * switch_break or a switch_fallthrough.
    */
   nir_variable *fallthrough_var;

   unsigned index;
};

enum vtn_branch_type {
   vtn_branch_type_none,
   vtn_branch_type_forward,
   vtn_branch_type_if_break,
   vtn_branch_type_switch_break,
   vtn_branch_type_switch_fallthrough,
   vtn_branch_type_loop_break,
   vtn_branch_type_loop_continue,
   vtn_branch_type_loop_back_edge,
   vtn_branch_type_discard,
   vtn_branch_type_terminate_invocation,
   vtn_branch_type_ignore_intersection,
   vtn_branch_type_terminate_ray,
   vtn_branch_type_emit_mesh_tasks,
   vtn_branch_type_return,
};

static const char *
vtn_branch_type_to_string(enum vtn_branch_type t)
{
#define CASE(typ) case vtn_branch_type_##typ: return #typ
   switch (t) {
   CASE(none);
   CASE(forward);
   CASE(if_break);
   CASE(switch_break);
   CASE(switch_fallthrough);
   CASE(loop_break);
   CASE(loop_continue);
   CASE(loop_back_edge);
   CASE(discard);
   CASE(terminate_invocation);
   CASE(ignore_intersection);
   CASE(terminate_ray);
   CASE(emit_mesh_tasks);
   CASE(return);
   }
#undef CASE
   unreachable("unknown branch type");
   return "";
}

struct vtn_successor {
   struct vtn_block *block;
   enum vtn_branch_type branch_type;
};

static bool
vtn_is_single_block_loop(const struct vtn_construct *c)
{
   return c->type == vtn_construct_type_loop &&
          c->start_pos == c->continue_pos;
}

static struct vtn_construct *
vtn_find_innermost(enum vtn_construct_type type, struct vtn_construct *c)
{
   while (c && c->type != type)
      c = c->parent;
   return c;
}

static void
print_ordered_blocks(const struct vtn_function *func)
{
   for (unsigned i = 0; i < func->ordered_blocks_count; i++) {
      struct vtn_block *block = func->ordered_blocks[i];
      printf("[id=%-6u] %4u", block->label[1], block->pos);
      if (block->successors_count > 0) {
         printf(" ->");
         for (unsigned j = 0; j < block->successors_count; j++) {
            printf(" ");
            if (block->successors[j].block)
               printf("%u/", block->successors[j].block->pos);
            printf("%s", vtn_branch_type_to_string(block->successors[j].branch_type));
         }
      }
      if (!block->visited)
         printf("  NOT VISITED");
      printf("\n");
   }
}

static struct vtn_case *
vtn_find_fallthrough_target(struct vtn_builder *b, const uint32_t *switch_merge,
                            struct vtn_block *source_block, struct vtn_block *block)
{
   if (block->visited)
      return NULL;

   if (block->label[1] == switch_merge[1])
      return NULL;

   /* Don't consider the initial source block a fallthrough target of itself. */
   if (block->switch_case && block != source_block)
      return block->switch_case;

   if (block->merge)
      return vtn_find_fallthrough_target(b, switch_merge, source_block,
                                         vtn_block(b, block->merge[1]));

   const uint32_t *branch = block->branch;
   vtn_assert(branch);

   switch (branch[0] & SpvOpCodeMask) {
   case SpvOpBranch:
      return vtn_find_fallthrough_target(b, switch_merge, source_block,
                                         vtn_block(b, branch[1]));
   case SpvOpBranchConditional: {
      struct vtn_case *target =
         vtn_find_fallthrough_target(b, switch_merge, source_block,
                                     vtn_block(b, branch[2]));
      if (!target)
         target = vtn_find_fallthrough_target(b, switch_merge, source_block,
                                              vtn_block(b, branch[3]));
      return target;
   }
   default:
      return NULL;
   }
}

static void
structured_post_order_traversal(struct vtn_builder *b, struct vtn_block *block)
{
   if (block->visited)
      return;

   block->visited = true;

   if (block->merge) {
      structured_post_order_traversal(b, vtn_block(b, block->merge[1]));

      SpvOp merge_op = block->merge[0] & SpvOpCodeMask;
      if (merge_op == SpvOpLoopMerge) {
         struct vtn_block *continue_block = vtn_block(b, block->merge[2]);
         structured_post_order_traversal(b, continue_block);
      }
   }

   const uint32_t *branch = block->branch;
   vtn_assert(branch);

   switch (branch[0] & SpvOpCodeMask) {
   case SpvOpBranch:
      block->successors_count = 1;
      block->successors = vtn_zalloc(b, struct vtn_successor);
      block->successors[0].block = vtn_block(b, branch[1]);
      structured_post_order_traversal(b, block->successors[0].block);
      break;

   case SpvOpBranchConditional:
      block->successors_count = 2;
      block->successors = vtn_zalloc_array(b, struct vtn_successor, 2);
      block->successors[0].block = vtn_block(b, branch[2]);
      block->successors[1].block = vtn_block(b, branch[3]);

      /* The result of the traversal will be reversed, so to provide a
       * more natural order, with THEN blocks appearing before ELSE blocks,
       * we need to traverse them in the reversed order.
       */
      int order[] = { 1, 0 };

      /* There's a catch when traversing case fallthroughs: we want to avoid
       * walking part of a case construct, then the fallthrough -- possibly
       * visiting another entire case construct, and back to the other part
       * of that original case construct. So if the THEN path is a fallthrough,
       * swap the visit order.
       */
      if (block->successors[0].block->switch_case) {
         order[0] = !order[0];
         order[1] = !order[1];
      }

      structured_post_order_traversal(b, block->successors[order[0]].block);
      structured_post_order_traversal(b, block->successors[order[1]].block);
      break;

   case SpvOpSwitch: {
      /* TODO: Save this to use during Switch construct creation. */
      struct list_head cases;
      list_inithead(&cases);
      vtn_parse_switch(b, block->branch, &cases);

      block->successors_count = list_length(&cases);
      block->successors = vtn_zalloc_array(b, struct vtn_successor, block->successors_count);

      /* The 'Rules for Structured Control-flow constructs' already guarantee
       * that the labels of the targets are ordered in a way that if
       * there is a fallthrough, they will appear consecutively.  The only
       * exception is Default, which is always the first in the list.
       *
       * Because we are doing a DFS from the end of the cases, the
       * traversal already handle a Case falling through Default.
       *
       * The scenario that needs fixing is when no case falls to Default, but
       * Default falls to another case.  For that scenario we move the Default
       * right before the case it falls to.
       */

      struct vtn_case *default_case = list_first_entry(&cases, struct vtn_case, link);
      vtn_assert(default_case && default_case->is_default);

      struct vtn_case *fall_target =
         vtn_find_fallthrough_target(b, block->merge, default_case->block,
                                     default_case->block);
      if (fall_target)
         list_move_to(&default_case->link, &fall_target->link);

      /* Because the result of the traversal will be reversed, loop backwards
       * in the case list.
       */
      unsigned i = 0;
      list_for_each_entry_rev(struct vtn_case, cse, &cases, link) {
         structured_post_order_traversal(b, cse->block);
         block->successors[i].block = cse->block;
         i++;
      }

      break;
   }

   case SpvOpKill:
   case SpvOpTerminateInvocation:
   case SpvOpIgnoreIntersectionKHR:
   case SpvOpTerminateRayKHR:
   case SpvOpReturn:
   case SpvOpReturnValue:
   case SpvOpEmitMeshTasksEXT:
   case SpvOpUnreachable:
      block->successors_count = 1;
      block->successors = vtn_zalloc(b, struct vtn_successor);
      break;

   default:
      unreachable("invalid branch opcode");
   }

   b->func->ordered_blocks[b->func->ordered_blocks_count++] = block;
}

static void
sort_blocks(struct vtn_builder *b)
{
   struct vtn_block **ordered_blocks =
      vtn_zalloc_array(b, struct vtn_block *, b->func->block_count);

   b->func->ordered_blocks = ordered_blocks;

   structured_post_order_traversal(b, b->func->start_block);

   /* Reverse it, so that blocks appear before their successors. */
   unsigned count = b->func->ordered_blocks_count;
   for (unsigned i = 0; i < (count / 2); i++) {
      unsigned j = count - i - 1;
      struct vtn_block *tmp = ordered_blocks[i];
      ordered_blocks[i] = ordered_blocks[j];
      ordered_blocks[j] = tmp;
   }

   for (unsigned i = 0; i < count; i++)
      ordered_blocks[i]->pos = i;
}

static void
print_construct(const struct vtn_function *func,
                const struct vtn_construct *c)
{
   for (const struct vtn_construct *p = c->parent; p; p = p->parent)
      printf("    ");
   printf("C%u/%s ", c->index, vtn_construct_type_to_string(c->type));
   printf("  %u->%u", c->start_pos, c->end_pos);
   if (c->merge_pos)
      printf("  merge=%u", c->merge_pos);
   if (c->then_pos)
      printf("  then=%u", c->then_pos);
   if (c->else_pos)
      printf("  else=%u", c->else_pos);
   if (c->needs_nloop)
      printf("  nloop");
   if (c->needs_break_propagation)
      printf("  break_prop");
   if (c->needs_continue_propagation)
      printf("  continue_prop");
   if (c->type == vtn_construct_type_loop) {
      if (vtn_is_single_block_loop(c))
         printf("  single_block_loop");
      else
         printf("  cont=%u", c->continue_pos);
   }
   if (c->type == vtn_construct_type_case) {
      struct vtn_block *block = func->ordered_blocks[c->start_pos];
      if (block->switch_case->is_default) {
         printf(" [default]");
      } else {
         printf(" [values:");
         util_dynarray_foreach(&block->switch_case->values, uint64_t, val)
            printf(" %" PRIu64, *val);
         printf("]");
      }
   }
   printf("\n");
}

static void
print_constructs(struct vtn_function *func)
{
   list_for_each_entry(struct vtn_construct, c, &func->constructs, link)
      print_construct(func, c);
}

struct vtn_construct_stack {
   /* Array of `struct vtn_construct *`. */
   struct util_dynarray data;
};

static inline void
init_construct_stack(struct vtn_construct_stack *stack, void *mem_ctx)
{
   assert(mem_ctx);
   util_dynarray_init(&stack->data, mem_ctx);
}

static inline unsigned
count_construct_stack(struct vtn_construct_stack *stack)
{
   return util_dynarray_num_elements(&stack->data, struct vtn_construct *);
}

static inline struct vtn_construct *
top_construct(struct vtn_construct_stack *stack)
{
   assert(count_construct_stack(stack) > 0);
   return util_dynarray_top(&stack->data, struct vtn_construct *);
}

static inline void
pop_construct(struct vtn_construct_stack *stack)
{
   assert(count_construct_stack(stack) > 0);
   (void)util_dynarray_pop(&stack->data, struct vtn_construct *);
}

static inline void
push_construct(struct vtn_construct_stack *stack, struct vtn_construct *c)
{
   util_dynarray_append(&stack->data, struct vtn_construct *, c);
}

static int
cmp_succ_block_pos(const void *pa, const void *pb)
{
   const struct vtn_successor *sa = pa;
   const struct vtn_successor *sb = pb;
   const unsigned a = sa->block->pos;
   const unsigned b = sb->block->pos;
   if (a < b)
      return -1;
   if (a > b)
      return 1;
   return 0;
}

static void
create_constructs(struct vtn_builder *b)
{
   struct vtn_construct *func_construct = vtn_zalloc(b, struct vtn_construct);
   func_construct->type = vtn_construct_type_function;
   func_construct->start_pos = 0;
   func_construct->end_pos = b->func->ordered_blocks_count;

   for (unsigned i = 0; i < b->func->ordered_blocks_count; i++) {
      struct vtn_block *block = b->func->ordered_blocks[i];

      if (block->merge) {
         SpvOp merge_op = block->merge[0] & SpvOpCodeMask;
         SpvOp branch_op = block->branch[0] & SpvOpCodeMask;

         const unsigned end_pos = vtn_block(b, block->merge[1])->pos;

         if (merge_op == SpvOpLoopMerge) {
            struct vtn_construct *loop = vtn_zalloc(b, struct vtn_construct);
            loop->type = vtn_construct_type_loop;
            loop->start_pos = block->pos;
            loop->end_pos = end_pos;

            loop->parent = block->parent;
            block->parent = loop;

            struct vtn_block *continue_block = vtn_block(b, block->merge[2]);
            loop->continue_pos = continue_block->pos;

            if (!vtn_is_single_block_loop(loop)) {
               struct vtn_construct *cont = vtn_zalloc(b, struct vtn_construct);
               cont->type = vtn_construct_type_continue;
               cont->parent = loop;
               cont->start_pos = loop->continue_pos;
               cont->end_pos = end_pos;

               cont->parent = loop;
               continue_block->parent = cont;
            }

            /* Not all combinations of OpLoopMerge and OpBranchConditional are valid,
             * workaround for invalid combinations by injecting an extra selection.
             *
             * Old versions of dxil-spirv generated this.
             */
            if (branch_op == SpvOpBranchConditional) {
               vtn_assert(block->successors_count == 2);
               const unsigned then_pos = block->successors[0].block ?
                                         block->successors[0].block->pos : 0;
               const unsigned else_pos = block->successors[1].block ?
                                         block->successors[1].block->pos : 0;

               if (then_pos > loop->start_pos && then_pos < loop->continue_pos &&
                   else_pos > loop->start_pos && else_pos < loop->continue_pos) {
                  vtn_warn("An OpSelectionMerge instruction is required to precede "
                           "an OpBranchConditional instruction that has different "
                           "True Label and False Label operands where neither are "
                           "declared merge blocks or Continue Targets.");
                  struct vtn_construct *sel = vtn_zalloc(b, struct vtn_construct);
                  sel->type = vtn_construct_type_selection;
                  sel->start_pos = loop->start_pos;
                  sel->end_pos = loop->continue_pos;
                  sel->then_pos = then_pos;
                  sel->else_pos = else_pos;
                  sel->parent = loop;
                  block->parent = sel;
               }
            }

         } else if (branch_op == SpvOpSwitch) {
            vtn_assert(merge_op == SpvOpSelectionMerge);

            struct vtn_construct *swtch = vtn_zalloc(b, struct vtn_construct);
            swtch->type = vtn_construct_type_switch;
            swtch->start_pos = block->pos;
            swtch->end_pos = end_pos;

            swtch->parent = block->parent;
            block->parent = swtch;

            struct list_head cases;
            list_inithead(&cases);
            vtn_parse_switch(b, block->branch, &cases);

            vtn_foreach_case_safe(cse, &cases) {
               if (cse->block->pos < end_pos) {
                  struct vtn_block *case_block = cse->block;
                  struct vtn_construct *c = vtn_zalloc(b, struct vtn_construct);
                  c->type = vtn_construct_type_case;
                  c->parent = swtch;
                  c->start_pos = case_block->pos;

                  /* Upper bound, will be updated right after. */
                  c->end_pos = swtch->end_pos;

                  vtn_assert(case_block->parent == NULL || case_block->parent == swtch);
                  case_block->parent = c;
               } else {
                  /* A target in OpSwitch must point either to one of the case
                   * constructs or to the Merge block.  No outer break/continue
                   * is allowed.
                   */
                  vtn_assert(cse->block->pos == end_pos);
               }
               list_delinit(&cse->link);
            }

            /* Case constructs don't overlap, so they end as the next one
             * begins.
             */
            qsort(block->successors, block->successors_count,
                  sizeof(struct vtn_successor), cmp_succ_block_pos);
            for (unsigned succ_idx = 1; succ_idx < block->successors_count; succ_idx++) {
               unsigned succ_pos = block->successors[succ_idx].block->pos;
               /* The successors are ordered, so once we see a successor point
                * to the merge block, we are done fixing the cases.
                */
               if (succ_pos >= swtch->end_pos)
                  break;
               struct vtn_construct *prev_cse =
                  vtn_find_innermost(vtn_construct_type_case,
                                     block->successors[succ_idx - 1].block->parent);
               vtn_assert(prev_cse);
               prev_cse->end_pos = succ_pos;
            }

         } else {
            vtn_assert(merge_op == SpvOpSelectionMerge);
            vtn_assert(branch_op == SpvOpBranchConditional);

            struct vtn_construct *sel = vtn_zalloc(b, struct vtn_construct);
            sel->type = vtn_construct_type_selection;
            sel->start_pos = block->pos;
            sel->end_pos = end_pos;
            sel->parent = block->parent;
            block->parent = sel;

            vtn_assert(block->successors_count == 2);
            struct vtn_block *then_block = block->successors[0].block;
            struct vtn_block *else_block = block->successors[1].block;

            sel->then_pos = then_block ? then_block->pos : 0;
            sel->else_pos = else_block ? else_block->pos : 0;
         }
      }
   }

   /* Link the constructs with their parents and with the remaining blocks
    * that do not start one.  This will also build the ordered list of
    * constructs.
    */
   struct vtn_construct_stack stack;
   init_construct_stack(&stack, b);
   push_construct(&stack, func_construct);
   list_addtail(&func_construct->link, &b->func->constructs);

   for (unsigned i = 0; i < b->func->ordered_blocks_count; i++) {
      struct vtn_block *block = b->func->ordered_blocks[i];

      while (block->pos == top_construct(&stack)->end_pos)
         pop_construct(&stack);

      /* Identify the start of a continue construct. */
      if (top_construct(&stack)->type == vtn_construct_type_loop &&
          !vtn_is_single_block_loop(top_construct(&stack)) &&
          top_construct(&stack)->continue_pos == block->pos) {
         struct vtn_construct *c = vtn_find_innermost(vtn_construct_type_continue, block->parent);
         vtn_assert(c);
         vtn_assert(c->parent == top_construct(&stack));

         list_addtail(&c->link, &b->func->constructs);
         push_construct(&stack, c);
      }

      if (top_construct(&stack)->type == vtn_construct_type_switch) {
         struct vtn_block *header = b->func->ordered_blocks[top_construct(&stack)->start_pos];
         for (unsigned succ_idx = 0; succ_idx < header->successors_count; succ_idx++) {
            struct vtn_successor *succ = &header->successors[succ_idx];
            if (block == succ->block) {
               struct vtn_construct *c = vtn_find_innermost(vtn_construct_type_case, succ->block->parent);
               if (c) {
                  vtn_assert(c->parent == top_construct(&stack));

                  list_addtail(&c->link, &b->func->constructs);
                  push_construct(&stack, c);
               }
               break;
            }
         }
      }

      if (block->merge) {
         switch (block->merge[0] & SpvOpCodeMask) {
         case SpvOpSelectionMerge: {
            struct vtn_construct *c = block->parent;
            vtn_assert(c->type == vtn_construct_type_selection ||
                       c->type == vtn_construct_type_switch);

            c->parent = top_construct(&stack);

            list_addtail(&c->link, &b->func->constructs);
            push_construct(&stack, c);
            break;
         }

         case SpvOpLoopMerge: {
            struct vtn_construct *c = block->parent;
            struct vtn_construct *loop = c;

            /* A loop might have an extra selection injected, skip it. */
            if (c->type == vtn_construct_type_selection)
               loop = c->parent;

            vtn_assert(loop->type == vtn_construct_type_loop);
            loop->parent = top_construct(&stack);

            list_addtail(&loop->link, &b->func->constructs);
            push_construct(&stack, loop);

            if (loop != c) {
               /* Make sure we also "enter" the extra construct. */
               list_addtail(&c->link, &b->func->constructs);
               push_construct(&stack, c);
            }
            break;
         }

         default:
            unreachable("invalid merge opcode");
         }
      }

      block->parent = top_construct(&stack);
   }

   vtn_assert(count_construct_stack(&stack) == 1);
   vtn_assert(top_construct(&stack)->type == vtn_construct_type_function);

   unsigned index = 0;
   list_for_each_entry(struct vtn_construct, c, &b->func->constructs, link)
      c->index = index++;
}

static void
validate_constructs(struct vtn_builder *b)
{
   list_for_each_entry(struct vtn_construct, c, &b->func->constructs, link) {
      if (c->type == vtn_construct_type_function)
         vtn_assert(c->parent == NULL);
      else
         vtn_assert(c->parent);

      switch (c->type) {
      case vtn_construct_type_continue:
         vtn_assert(c->parent->type == vtn_construct_type_loop);
         break;
      case vtn_construct_type_case:
         vtn_assert(c->parent->type == vtn_construct_type_switch);
         break;
      default:
         /* Nothing to do. */
         break;
      }
   }
}

static void
find_innermost_constructs(struct vtn_builder *b)
{
   list_for_each_entry(struct vtn_construct, c, &b->func->constructs, link) {
      if (c->type == vtn_construct_type_function) {
         c->innermost_loop = NULL;
         c->innermost_switch = NULL;
         c->innermost_case = NULL;
         continue;
      }

      if (c->type == vtn_construct_type_loop)
         c->innermost_loop = c;
      else
         c->innermost_loop = c->parent->innermost_loop;

      if (c->type == vtn_construct_type_switch)
         c->innermost_switch = c;
      else
         c->innermost_switch = c->parent->innermost_switch;

      if (c->type == vtn_construct_type_case)
         c->innermost_case = c;
      else
         c->innermost_case = c->parent->innermost_case;
   }

   list_for_each_entry(struct vtn_construct, c, &b->func->constructs, link) {
      vtn_assert(vtn_find_innermost(vtn_construct_type_loop, c) == c->innermost_loop);
      vtn_assert(vtn_find_innermost(vtn_construct_type_switch, c) == c->innermost_switch);
      vtn_assert(vtn_find_innermost(vtn_construct_type_case, c) == c->innermost_case);
   }
}

static void
set_needs_continue_propagation(struct vtn_construct *c)
{
   for (; c != c->innermost_loop; c = c->parent)
      c->needs_continue_propagation = true;
}

static void
set_needs_break_propagation(struct vtn_construct *c,
                            struct vtn_construct *to_break)
{
   for (; c != to_break; c = c->parent)
      c->needs_break_propagation = true;
}

static enum vtn_branch_type
branch_type_for_successor(struct vtn_builder *b, struct vtn_block *block,
                          struct vtn_successor *succ)
{
   unsigned pos = block->pos;
   unsigned succ_pos = succ->block->pos;

   struct vtn_construct *inner = block->parent;
   vtn_assert(inner);

   /* Identify the types of branches, applying the "Rules for Structured
    * Control-flow Constructs" from SPIR-V spec.
    */

   struct vtn_construct *innermost_loop = inner->innermost_loop;
   if (innermost_loop) {
      /* Entering the innermost loop’s continue construct. */
      if (!vtn_is_single_block_loop(innermost_loop) &&
          succ_pos == innermost_loop->continue_pos) {
         set_needs_continue_propagation(inner);
         return vtn_branch_type_loop_continue;
      }

      /* Breaking from the innermost loop (and branching from back-edge block
       * to loop merge).
       */
      if (succ_pos == innermost_loop->end_pos) {
         set_needs_break_propagation(inner, innermost_loop);
         return vtn_branch_type_loop_break;
      }

      /* Next loop iteration.  There can be only a single loop back-edge
       * for each loop construct.
       */
      if (succ_pos == innermost_loop->start_pos) {
         vtn_assert(inner->type == vtn_construct_type_continue ||
                    vtn_is_single_block_loop(innermost_loop));
         return vtn_branch_type_loop_back_edge;
      }
   }

   struct vtn_construct *innermost_switch = inner->innermost_switch;
   if (innermost_switch) {
      struct vtn_construct *innermost_cse = inner->innermost_case;

      /* Breaking from the innermost switch construct. */
      if (succ_pos == innermost_switch->end_pos) {
         /* Use a nloop if this is not a natural exit from a case construct. */
         if (innermost_cse && pos != innermost_cse->end_pos - 1) {
            innermost_cse->needs_nloop = true;
            set_needs_break_propagation(inner, innermost_cse);
         }
         return vtn_branch_type_switch_break;
      }

      /* Branching from one case construct to another. */
      if (inner != innermost_switch) {
         vtn_assert(innermost_cse);
         vtn_assert(innermost_cse->parent == innermost_switch);

         if (succ->block->switch_case) {
            /* Both cases should be from the same Switch construct. */
            struct vtn_construct *target_cse = succ->block->parent->innermost_case;
            vtn_assert(target_cse->parent == innermost_switch);
            target_cse->needs_fallthrough = true;
            return vtn_branch_type_switch_fallthrough;
         }
      }
   }

   if (inner->type == vtn_construct_type_selection) {
      /* Branches from the header block that were not categorized above will
       * follow to the then/else paths or to the merge block, and are handled
       * by the nir_if node.
       */
      if (block->merge)
         return vtn_branch_type_forward;

      /* Breaking from a selection construct. */
      if (succ_pos == inner->end_pos) {
         /* Identify cases where the break would be a natural flow in the NIR
          * construct.  We don't need the extra loop in such cases.
          *
          * Because then/else are not ordered, we need to find which one happens
          * later.  For non early merges, the branch from the block right before
          * the second side of the if starts will also jumps naturally to the
          * end of the if.
          */
         const bool has_early_merge = inner->merge_pos != inner->end_pos;
         const unsigned second_pos = MAX2(inner->then_pos, inner->else_pos);

         const bool natural_exit_from_if =
            pos + 1 == inner->end_pos ||
            (!has_early_merge && (pos + 1 == second_pos));

         inner->needs_nloop = !natural_exit_from_if;
         return vtn_branch_type_if_break;
      }
   }

   if (succ_pos < inner->end_pos)
      return vtn_branch_type_forward;

   const enum nir_spirv_debug_level level = NIR_SPIRV_DEBUG_LEVEL_ERROR;
   const size_t offset = 0;

   vtn_logf(b, level, offset,
            "SPIR-V parsing FAILED:\n"
            "    Unrecognized branch from block pos %u (id=%u) "
            "to block pos %u (id=%u)",
            block->pos, block->label[1],
            succ->block->pos, succ->block->label[1]);

   vtn_logf(b, level, offset,
            "    Inner construct '%s': %u -> %u  (merge=%u then=%u else=%u)",
            vtn_construct_type_to_string(inner->type),
            inner->start_pos, inner->end_pos, inner->merge_pos, inner->then_pos, inner->else_pos);

   struct vtn_construct *outer = inner->parent;
   if (outer) {
      vtn_logf(b, level, offset,
               "    Outer construct '%s': %u -> %u  (merge=%u then=%u else=%u)",
               vtn_construct_type_to_string(outer->type),
               outer->start_pos, outer->end_pos, outer->merge_pos, outer->then_pos, outer->else_pos);
   }

   vtn_fail("Unable to identify branch type");
   return vtn_branch_type_none;
}

static enum vtn_branch_type
branch_type_for_terminator(struct vtn_builder *b, struct vtn_block *block)
{
   vtn_assert(block->successors_count == 1);
   vtn_assert(block->successors[0].block == NULL);

   switch (block->branch[0] & SpvOpCodeMask) {
   case SpvOpKill:
      return vtn_branch_type_discard;
   case SpvOpTerminateInvocation:
      return vtn_branch_type_terminate_invocation;
   case SpvOpIgnoreIntersectionKHR:
      return vtn_branch_type_ignore_intersection;
   case SpvOpTerminateRayKHR:
      return vtn_branch_type_terminate_ray;
   case SpvOpEmitMeshTasksEXT:
      return vtn_branch_type_emit_mesh_tasks;
   case SpvOpReturn:
   case SpvOpReturnValue:
   case SpvOpUnreachable:
      return vtn_branch_type_return;
   default:
      unreachable("unexpected terminator operation");
      return vtn_branch_type_none;
   }
}

static void
set_branch_types(struct vtn_builder *b)
{
   for (unsigned i = 0; i < b->func->ordered_blocks_count; i++) {
      struct vtn_block *block = b->func->ordered_blocks[i];
      for (unsigned j = 0; j < block->successors_count; j++) {
         struct vtn_successor *succ = &block->successors[j];

         if (succ->block)
            succ->branch_type = branch_type_for_successor(b, block, succ);
         else
            succ->branch_type = branch_type_for_terminator(b, block);

         vtn_assert(succ->branch_type != vtn_branch_type_none);
      }
   }
}

static void
find_merge_pos(struct vtn_builder *b)
{
   /* Merges are at the end of the construct by construction... */
   list_for_each_entry(struct vtn_construct, c, &b->func->constructs, link)
      c->merge_pos = c->end_pos;

   /* ...except when we have an "early merge", i.e. a branch that converges
    * before the declared merge point.  For these cases the actual merge is
    * stored in merge_pos.
    *
    * Look at all header blocks for constructs that may have such early
    * merge, and check whether they fit
    */
   for (unsigned i = 0; i < b->func->ordered_blocks_count; i++) {
      if (!b->func->ordered_blocks[i]->merge)
         continue;

      struct vtn_block *header = b->func->ordered_blocks[i];
      if (header->successors_count != 2)
         continue;

      /* Ignore single-block loops (i.e. header thats in a continue
       * construct).  Because the loop has no body, no block will
       * be identified in the then/else sides, the vtn_emit_branch
       * calls will be enough.
       */

      struct vtn_construct *c = header->parent;
      if (c->type != vtn_construct_type_selection)
         continue;

      const unsigned first_pos = MIN2(c->then_pos, c->else_pos);
      const unsigned second_pos = MAX2(c->then_pos, c->else_pos);

      /* The first side ends where the second starts.  The second side ends
       * either the continue position (that is guaranteed to appear after the
       * body of a loop) or the actual end of the construct.
       *
       * Because of the way we ordered the blocks, if there's an early merge,
       * the first side of the if will have a branch inside the second side.
       */
      const unsigned first_end = second_pos;
      const unsigned second_end = c->end_pos;

      unsigned early_merge_pos = 0;
      for (unsigned pos = first_pos; pos < first_end; pos++) {
         /* For each block in first... */
         struct vtn_block *block = b->func->ordered_blocks[pos];
         for (unsigned s = 0; s < block->successors_count; s++) {
            if (block->successors[s].block) {
               /* ...see if one of its successors branches to the second side. */
               const unsigned succ_pos = block->successors[s].block->pos;
               if (succ_pos >= second_pos && succ_pos < second_end) {
                  vtn_fail_if(early_merge_pos,
                              "A single selection construct cannot "
                              "have multiple early merges");
                  early_merge_pos = succ_pos;
               }
            }
         }

         if (early_merge_pos) {
            c->merge_pos = early_merge_pos;
            break;
         }
      }
   }
}

void
vtn_build_structured_cfg(struct vtn_builder *b, const uint32_t *words, const uint32_t *end)
{
   vtn_foreach_function(func, &b->functions) {
      b->func = func;

      sort_blocks(b);

      create_constructs(b);

      validate_constructs(b);

      find_innermost_constructs(b);

      find_merge_pos(b);

      set_branch_types(b);

      if (MESA_SPIRV_DEBUG(STRUCTURED)) {
         printf("\nBLOCKS (%u):\n", func->ordered_blocks_count);
         print_ordered_blocks(func);
         printf("\nCONSTRUCTS (%u):\n", list_length(&func->constructs));
         print_constructs(func);
         printf("\n");
      }
   }
}

static int
vtn_set_break_vars_between(struct vtn_builder *b,
                           struct vtn_construct *from,
                           struct vtn_construct *to)
{
   vtn_assert(from);
   vtn_assert(to);

   int count = 0;
   for (struct vtn_construct *c = from; c != to; c = c->parent) {
      if (c->break_var) {
         vtn_assert(c->nloop);
         count++;

         /* There's no need to set break_var for the from block an actual break will be emitted
          * by the callers.
          */
         if (c != from)
            nir_store_var(&b->nb, c->break_var, nir_imm_true(&b->nb), 1);
      } else {
         /* There's a 1:1 correspondence between break_vars and nloops. */
         vtn_assert(!c->nloop);
      }
   }

   return count;
}

static void
vtn_emit_break_for_construct(struct vtn_builder *b,
                             const struct vtn_block *block,
                             struct vtn_construct *to_break)
{
   vtn_assert(to_break);
   vtn_assert(to_break->nloop);

   bool has_intermediate = vtn_set_break_vars_between(b, block->parent, to_break);
   if (has_intermediate)
      nir_store_var(&b->nb, to_break->break_var, nir_imm_true(&b->nb), 1);

   nir_jump(&b->nb, nir_jump_break);
}

static void
vtn_emit_continue_for_construct(struct vtn_builder *b,
                                const struct vtn_block *block,
                                struct vtn_construct *to_continue)
{
   vtn_assert(to_continue);
   vtn_assert(to_continue->type == vtn_construct_type_loop);
   vtn_assert(to_continue->nloop);

   bool has_intermediate = vtn_set_break_vars_between(b, block->parent, to_continue);
   if (has_intermediate) {
      nir_store_var(&b->nb, to_continue->continue_var, nir_imm_true(&b->nb), 1);
      nir_jump(&b->nb, nir_jump_break);
   } else {
      nir_jump(&b->nb, nir_jump_continue);
   }
}

static void
vtn_emit_branch(struct vtn_builder *b, const struct vtn_block *block,
                const struct vtn_successor *succ)
{
   switch (succ->branch_type) {
   case vtn_branch_type_none:
      vtn_assert(!"invalid branch type");
      break;

   case vtn_branch_type_forward:
      /* Nothing to do. */
      break;

   case vtn_branch_type_if_break: {
      struct vtn_construct *inner_if = block->parent;
      vtn_assert(inner_if->type == vtn_construct_type_selection);
      if (inner_if->nloop) {
         vtn_emit_break_for_construct(b, block, inner_if);
      } else {
         /* Nothing to do. This is a natural exit from an if construct. */
      }
      break;
   }

   case vtn_branch_type_switch_break: {
      struct vtn_construct *swtch = block->parent->innermost_switch;
      vtn_assert(swtch);

      struct vtn_construct *cse = block->parent->innermost_case;
      if (cse && cse->parent == swtch && cse->nloop) {
         vtn_emit_break_for_construct(b, block, cse);
      } else {
         /* Nothing to do.  This case doesn't have a loop, so this is a
          * natural break from a case.
          */
      }
      break;
   }

   case vtn_branch_type_switch_fallthrough: {
      struct vtn_construct *cse = block->parent->innermost_case;
      vtn_assert(cse);

      struct vtn_construct *swtch = cse->parent;
      vtn_assert(swtch->type == vtn_construct_type_switch);

      /* Successor is the start of another case construct with the same parent
       * switch construct.
       */
      vtn_assert(succ->block->switch_case != NULL);
      struct vtn_construct *target = succ->block->parent->innermost_case;
      vtn_assert(target != NULL && target->type == vtn_construct_type_case);
      vtn_assert(target->parent == swtch);
      vtn_assert(target->fallthrough_var);

      nir_store_var(&b->nb, target->fallthrough_var, nir_imm_true(&b->nb), 1);
      if (cse->nloop)
         vtn_emit_break_for_construct(b, block, cse);
      break;
   }

   case vtn_branch_type_loop_break: {
      struct vtn_construct *loop = block->parent->innermost_loop;
      vtn_assert(loop);
      vtn_emit_break_for_construct(b, block, loop);
      break;
   }

   case vtn_branch_type_loop_continue: {
      struct vtn_construct *loop = block->parent->innermost_loop;
      vtn_assert(loop);
      vtn_emit_continue_for_construct(b, block, loop);
      break;
   }

   case vtn_branch_type_loop_back_edge:
      /* Nothing to do: naturally handled by NIR loop node. */
      break;

   case vtn_branch_type_return:
      vtn_assert(block);
      vtn_emit_ret_store(b, block);
      nir_jump(&b->nb, nir_jump_return);
      break;

   case vtn_branch_type_discard:
      if (b->convert_discard_to_demote)
         nir_demote(&b->nb);
      else
         nir_discard(&b->nb);
      break;

   case vtn_branch_type_terminate_invocation:
      nir_terminate(&b->nb);
      break;

   case vtn_branch_type_ignore_intersection:
      nir_ignore_ray_intersection(&b->nb);
      nir_jump(&b->nb, nir_jump_halt);
      break;

   case vtn_branch_type_terminate_ray:
      nir_terminate_ray(&b->nb);
      nir_jump(&b->nb, nir_jump_halt);
      break;

   case vtn_branch_type_emit_mesh_tasks: {
      vtn_assert(block);
      vtn_assert(block->branch);

      const uint32_t *w = block->branch;
      vtn_assert((w[0] & SpvOpCodeMask) == SpvOpEmitMeshTasksEXT);

      /* Launches mesh shader workgroups from the task shader.
       * Arguments are: vec(x, y, z), payload pointer
       */
      nir_def *dimensions =
         nir_vec3(&b->nb, vtn_get_nir_ssa(b, w[1]),
                          vtn_get_nir_ssa(b, w[2]),
                          vtn_get_nir_ssa(b, w[3]));

      /* The payload variable is optional.
       * We don't have a NULL deref in NIR, so just emit the explicit
       * intrinsic when there is no payload.
       */
      const unsigned count = w[0] >> SpvWordCountShift;
      if (count == 4)
         nir_launch_mesh_workgroups(&b->nb, dimensions);
      else if (count == 5)
         nir_launch_mesh_workgroups_with_payload_deref(&b->nb, dimensions,
                                                       vtn_get_nir_ssa(b, w[4]));
      else
         vtn_fail("Invalid EmitMeshTasksEXT.");

      nir_jump(&b->nb, nir_jump_halt);
      break;
   }

   default:
      vtn_fail("Invalid branch type");
   }
}

static nir_selection_control
vtn_selection_control(struct vtn_builder *b, SpvSelectionControlMask control)
{
   if (control == SpvSelectionControlMaskNone)
      return nir_selection_control_none;
   else if (control & SpvSelectionControlDontFlattenMask)
      return nir_selection_control_dont_flatten;
   else if (control & SpvSelectionControlFlattenMask)
      return nir_selection_control_flatten;
   else
      vtn_fail("Invalid selection control");
}

static void
vtn_emit_block(struct vtn_builder *b, struct vtn_block *block,
               vtn_instruction_handler handler)
{
   const uint32_t *block_start = block->label;
   const uint32_t *block_end = block->merge ? block->merge :
                                              block->branch;

   block_start = vtn_foreach_instruction(b, block_start, block_end,
                                         vtn_handle_phis_first_pass);

   vtn_foreach_instruction(b, block_start, block_end, handler);

   block->end_nop = nir_nop(&b->nb);

   if (block->parent->type == vtn_construct_type_switch) {
      /* Switch is handled as a sequence of NIR if for each of the cases. */

   } else if (block->successors_count == 1) {
      vtn_assert(block->successors[0].branch_type != vtn_branch_type_none);
      vtn_emit_branch(b, block, &block->successors[0]);

   } else if (block->successors_count == 2) {
      struct vtn_successor *then_succ = &block->successors[0];
      struct vtn_successor *else_succ = &block->successors[1];
      struct vtn_construct *c = block->parent;

      nir_def *cond = vtn_get_nir_ssa(b, block->branch[1]);
      if (then_succ->block == else_succ->block)
         cond = nir_imm_true(&b->nb);

      /* The branches will already be emitted here, so for paths that
       * doesn't have blocks inside the construct, e.g. that are an
       * exit from the construct, nothing else is needed.
       */
      nir_if *sel = nir_push_if(&b->nb, cond);
      vtn_emit_branch(b, block, then_succ);
      if (then_succ->block != else_succ->block) {
         nir_push_else(&b->nb, NULL);
         vtn_emit_branch(b, block, else_succ);
      }
      nir_pop_if(&b->nb, NULL);

      if (c->type == vtn_construct_type_selection &&
          block->pos == c->start_pos) {
         /* This is the start of a selection construct. Record the nir_if in
          * the construct so we can close it properly and handle the then and
          * else cases in block iteration.
          */
         vtn_assert(c->nif == NULL);
         c->nif = sel;

         vtn_assert(block->merge != NULL);

         SpvOp merge_op = block->merge[0] & SpvOpCodeMask;
         if (merge_op == SpvOpSelectionMerge)
            sel->control = vtn_selection_control(b, block->merge[2]);

         /* In most cases, vtn_emit_cf_func_structured() will place the cursor
          * in the correct side of the nir_if. However, in the case where the
          * selection construct is empty, we need to ensure that the cursor is
          * at least inside the nir_if or NIR will assert when we try to close
          * it with nir_pop_if().
          */
         b->nb.cursor = nir_before_cf_list(&sel->then_list);
      } else {
         vtn_fail_if(then_succ->branch_type == vtn_branch_type_forward &&
                     else_succ->branch_type == vtn_branch_type_forward &&
                     then_succ->block != else_succ->block,
                     "An OpSelectionMerge instruction is required to precede "
                     "an OpBranchConditional instruction that has different "
                     "True Label and False Label operands where neither are "
                     "declared merge blocks or Continue Targets.");

         if (then_succ->branch_type == vtn_branch_type_forward) {
            b->nb.cursor = nir_before_cf_list(&sel->then_list);
         } else if (else_succ->branch_type == vtn_branch_type_forward) {
            b->nb.cursor = nir_before_cf_list(&sel->else_list);
         } else {
            /* Leave it alone */
         }
      }
   }
}

static nir_def *
vtn_switch_case_condition(struct vtn_builder *b, struct vtn_construct *swtch,
                          nir_def *sel, struct vtn_case *cse)
{
   vtn_assert(swtch->type == vtn_construct_type_switch);

   if (cse->is_default) {
      nir_def *any = nir_imm_false(&b->nb);

      struct vtn_block *header = b->func->ordered_blocks[swtch->start_pos];

      for (unsigned j = 0; j < header->successors_count; j++) {
         struct vtn_successor *succ = &header->successors[j];
         struct vtn_case *other = succ->block->switch_case;

         if (other->is_default)
            continue;
         any = nir_ior(&b->nb, any,
                       vtn_switch_case_condition(b, swtch, sel, other));
      }

      return nir_inot(&b->nb, any);
   } else {
      nir_def *cond = nir_imm_false(&b->nb);
      util_dynarray_foreach(&cse->values, uint64_t, val)
         cond = nir_ior(&b->nb, cond, nir_ieq_imm(&b->nb, sel, *val));
      return cond;
   }
}

static nir_loop_control
vtn_loop_control(struct vtn_builder *b, SpvLoopControlMask control)
{
   if (control == SpvLoopControlMaskNone)
      return nir_loop_control_none;
   else if (control & SpvLoopControlDontUnrollMask)
      return nir_loop_control_dont_unroll;
   else if (control & SpvLoopControlUnrollMask)
      return nir_loop_control_unroll;
   else if ((control & SpvLoopControlDependencyInfiniteMask) ||
            (control & SpvLoopControlDependencyLengthMask) ||
            (control & SpvLoopControlMinIterationsMask) ||
            (control & SpvLoopControlMaxIterationsMask) ||
            (control & SpvLoopControlIterationMultipleMask) ||
            (control & SpvLoopControlPeelCountMask) ||
            (control & SpvLoopControlPartialCountMask)) {
      /* We do not do anything special with these yet. */
      return nir_loop_control_none;
   } else {
      vtn_fail("Invalid loop control");
   }
}

static void
vtn_emit_control_flow_propagation(struct vtn_builder *b,
                                  struct vtn_construct *top)
{
   if (top->type == vtn_construct_type_function ||
       top->type == vtn_construct_type_continue ||
       top->type == vtn_construct_type_switch)
      return;

   /* Find the innermost parent with a NIR loop. */
   struct vtn_construct *parent_with_nloop = NULL;
   for (struct vtn_construct *c = top->parent; c; c = c->parent) {
      if (c->nloop) {
         parent_with_nloop = c;
         break;
      }
   }
   if (parent_with_nloop == NULL)
      return;

   /* If there's another nloop in the parent chain, decide whether we need
    * to emit conditional continue/break after top construct is closed.
    */

   if (top->needs_continue_propagation &&
       parent_with_nloop == top->innermost_loop) {
      struct vtn_construct *loop = top->innermost_loop;
      vtn_assert(loop);
      vtn_assert(loop != top);

      nir_push_if(&b->nb, nir_load_var(&b->nb, loop->continue_var));
      nir_jump(&b->nb, nir_jump_continue);
      nir_pop_if(&b->nb, NULL);
   }

   if (top->needs_break_propagation) {
      vtn_assert(parent_with_nloop->break_var);
      nir_push_if(&b->nb, nir_load_var(&b->nb, parent_with_nloop->break_var));
      nir_jump(&b->nb, nir_jump_break);
      nir_pop_if(&b->nb, NULL);
   }
}

static inline nir_variable *
vtn_create_local_bool(struct vtn_builder *b, const char *name)
{
   return nir_local_variable_create(b->nb.impl, glsl_bool_type(), name);
}

void
vtn_emit_cf_func_structured(struct vtn_builder *b, struct vtn_function *func,
                            vtn_instruction_handler handler)
{
   struct vtn_construct *current =
      list_first_entry(&func->constructs, struct vtn_construct, link);
   vtn_assert(current->type == vtn_construct_type_function);

   /* Walk the blocks in order keeping track of the constructs that started
    * but haven't ended yet.  When constructs start and end, add extra code to
    * setup the NIR control flow (different for each construct), also add
    * extra code for propagating certain branch types.
    */

   struct vtn_construct_stack stack;
   init_construct_stack(&stack, b);
   push_construct(&stack, current);

   for (unsigned i = 0; i < func->ordered_blocks_count; i++) {
      struct vtn_block *block = func->ordered_blocks[i];
      struct vtn_construct *top = top_construct(&stack);

      /* Close out any past constructs and make sure the cursor is at the
       * right place to start this block. For each block, there are three
       * cases we care about here:
       *
       *  1. It is the block at the end (in our reverse structured post-order
       *     traversal) of one or more constructs and closes them.
       *
       *  2. It is an early merge of a selection construct.
       *
       *  3. It is the start of the then or else case of a selection construct
       *     and we may have previously been emitting code in the other side.
       */

      /* Close (or early merge) any constructs that end at this block. */
      bool merged_any_constructs = false;
      while (top->end_pos == block->pos || top->merge_pos == block->pos) {
         merged_any_constructs = true;
         if (top->nif) {
            const bool has_early_merge = top->merge_pos != top->end_pos;

            if (!has_early_merge) {
               nir_pop_if(&b->nb, top->nif);
            } else if (block->pos == top->merge_pos) {
               /* This is an early merge. */

               nir_pop_if(&b->nb, top->nif);

               /* The extra dummy "if (true)" for the merged part avoids
                * generating multiple jumps in sequence and upsetting
                * NIR rules.  We'll pop it in the case below when we reach
                * the end_pos block.
                */
               nir_push_if(&b->nb, nir_imm_true(&b->nb));

               /* Stop since this construct still has more blocks. */
               break;
            } else {
               /* Pop the dummy if added for the blocks after the early merge. */
               vtn_assert(block->pos == top->end_pos);
               nir_pop_if(&b->nb, NULL);
            }
         }

         if (top->nloop) {
            /* For constructs that are not SPIR-V loop, a NIR loop may be used
             * to provide richer control flow.  So we add a nir break to cause
             * the loop stop at the first iteration, unless there's already a
             * jump at the end of the last block.
             */
            if (top->type != vtn_construct_type_loop) {
               nir_block *last = nir_loop_last_block(top->nloop);
               if (!nir_block_ends_in_jump(last)) {
                  b->nb.cursor = nir_after_block(last);
                  nir_jump(&b->nb, nir_jump_break);
               }
            }

            nir_pop_loop(&b->nb, top->nloop);
         }

         vtn_emit_control_flow_propagation(b, top);

         pop_construct(&stack);
         top = top_construct(&stack);
      }

      /* We are fully inside the current top. */
      vtn_assert(block->pos < top->end_pos);

      /* Move the cursor to the right side of a selection construct.
       *
       * If we merged any constructs, we don't need to move because
       * either: this is an early merge and we already set the cursor above;
       * or a construct ended, and this is a 'merge block' for that
       * construct, so it can't also be a 'Target' for an outer conditional.
       */
      if (!merged_any_constructs && top->type == vtn_construct_type_selection &&
          (block->pos == top->then_pos || block->pos == top->else_pos)) {
         vtn_assert(top->nif);

         struct vtn_block *header = func->ordered_blocks[top->start_pos];
         vtn_assert(header->successors_count == 2);

         if (block->pos == top->then_pos)
            b->nb.cursor = nir_before_cf_list(&top->nif->then_list);
         else
            b->nb.cursor = nir_before_cf_list(&top->nif->else_list);
      }

      /* Open any constructs which start at this block.
       *
       * Constructs which are designated by Op*Merge are considered to start
       * at the block which contains the merge instruction.  This means that
       * loops constructs start at the first block inside the loop while
       * selection and switch constructs start at the block containing the
       * OpBranchConditional or OpSwitch.
       */
      while (current->link.next != &func->constructs) {
         struct vtn_construct *next =
            list_entry(current->link.next, struct vtn_construct, link);

         /* Stop once we find a construct that doesn't start in this block. */
         if (next->start_pos != block->pos)
            break;

         switch (next->type) {
         case vtn_construct_type_function:
            unreachable("should've already entered function construct");
            break;

         case vtn_construct_type_selection: {
            /* Add the wrapper loop now and the nir_if, along the contents of
             * this entire block, will get added inside the loop as part of
             * vtn_emit_block() below.
             */
            if (next->needs_nloop) {
               next->break_var = vtn_create_local_bool(b, "if_break");
               nir_store_var(&b->nb, next->break_var, nir_imm_false(&b->nb), 1);
               next->nloop = nir_push_loop(&b->nb);
            }
            break;
         }

         case vtn_construct_type_loop: {
            next->break_var = vtn_create_local_bool(b, "loop_break");
            next->continue_var = vtn_create_local_bool(b, "loop_continue");

            nir_store_var(&b->nb, next->break_var, nir_imm_false(&b->nb), 1);
            next->nloop = nir_push_loop(&b->nb);
            nir_store_var(&b->nb, next->continue_var, nir_imm_false(&b->nb), 1);

            next->nloop->control = vtn_loop_control(b, block->merge[3]);

            break;
         }

         case vtn_construct_type_continue: {
            struct vtn_construct *loop = next->parent;
            assert(loop->type == vtn_construct_type_loop);
            assert(!vtn_is_single_block_loop(loop));

            nir_push_continue(&b->nb, loop->nloop);

            break;
         }

         case vtn_construct_type_switch: {
            /* Switch is not translated to any NIR node, all is handled by
             * each individual case construct.
             */
            for (unsigned j = 0; j < block->successors_count; j++) {
               struct vtn_successor *s = &block->successors[j];
               if (s->block && s->block->pos < next->end_pos) {
                  struct vtn_construct *c = s->block->parent->innermost_case;
                  vtn_assert(c->type == vtn_construct_type_case);
                  if (c->needs_fallthrough) {
                     c->fallthrough_var = vtn_create_local_bool(b, "fallthrough");
                     nir_store_var(&b->nb, c->fallthrough_var, nir_imm_false(&b->nb), 1);
                  }
               }
            }
            break;
         }

         case vtn_construct_type_case: {
            struct vtn_construct *swtch = next->parent;
            struct vtn_block *header = func->ordered_blocks[swtch->start_pos];

            nir_def *sel = vtn_get_nir_ssa(b, header->branch[1]);
            nir_def *case_condition =
               vtn_switch_case_condition(b, swtch, sel, block->switch_case);
            if (next->fallthrough_var) {
               case_condition =
                  nir_ior(&b->nb, case_condition,
                          nir_load_var(&b->nb, next->fallthrough_var));
            }

            if (next->needs_nloop) {
               next->break_var = vtn_create_local_bool(b, "case_break");
               nir_store_var(&b->nb, next->break_var, nir_imm_false(&b->nb), 1);
               next->nloop = nir_push_loop(&b->nb);
            }

            next->nif = nir_push_if(&b->nb, case_condition);

            break;
         }
         }

         current = next;
         push_construct(&stack, next);
      }

      vtn_emit_block(b, block, handler);
   }

   vtn_assert(count_construct_stack(&stack) == 1);
}
