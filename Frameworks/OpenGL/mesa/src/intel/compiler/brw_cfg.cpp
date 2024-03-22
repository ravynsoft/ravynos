/*
 * Copyright © 2012 Intel Corporation
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
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

#include "brw_cfg.h"
#include "util/u_dynarray.h"
#include "brw_shader.h"

/** @file brw_cfg.cpp
 *
 * Walks the shader instructions generated and creates a set of basic
 * blocks with successor/predecessor edges connecting them.
 */

using namespace brw;

static bblock_t *
pop_stack(exec_list *list)
{
   bblock_link *link = (bblock_link *)list->get_tail();
   bblock_t *block = link->block;
   link->link.remove();

   return block;
}

static exec_node *
link(void *mem_ctx, bblock_t *block, enum bblock_link_kind kind)
{
   bblock_link *l = new(mem_ctx) bblock_link(block, kind);
   return &l->link;
}

void
push_stack(exec_list *list, void *mem_ctx, bblock_t *block)
{
   /* The kind of the link is immaterial, but we need to provide one since
    * this is (ab)using the edge data structure in order to implement a stack.
    */
   list->push_tail(link(mem_ctx, block, bblock_link_logical));
}

bblock_t::bblock_t(cfg_t *cfg) :
   cfg(cfg), start_ip(0), end_ip(0), end_ip_delta(0), num(0)
{
   instructions.make_empty();
   parents.make_empty();
   children.make_empty();
}

void
bblock_t::add_successor(void *mem_ctx, bblock_t *successor,
                        enum bblock_link_kind kind)
{
   successor->parents.push_tail(::link(mem_ctx, this, kind));
   children.push_tail(::link(mem_ctx, successor, kind));
}

bool
bblock_t::is_predecessor_of(const bblock_t *block,
                            enum bblock_link_kind kind) const
{
   foreach_list_typed_safe (bblock_link, parent, link, &block->parents) {
      if (parent->block == this && parent->kind <= kind) {
         return true;
      }
   }

   return false;
}

bool
bblock_t::is_successor_of(const bblock_t *block,
                          enum bblock_link_kind kind) const
{
   foreach_list_typed_safe (bblock_link, child, link, &block->children) {
      if (child->block == this && child->kind <= kind) {
         return true;
      }
   }

   return false;
}

static bool
ends_block(const backend_instruction *inst)
{
   enum opcode op = inst->opcode;

   return op == BRW_OPCODE_IF ||
          op == BRW_OPCODE_ELSE ||
          op == BRW_OPCODE_CONTINUE ||
          op == BRW_OPCODE_BREAK ||
          op == BRW_OPCODE_DO ||
          op == BRW_OPCODE_WHILE;
}

static bool
starts_block(const backend_instruction *inst)
{
   enum opcode op = inst->opcode;

   return op == BRW_OPCODE_DO ||
          op == BRW_OPCODE_ENDIF;
}

bool
bblock_t::can_combine_with(const bblock_t *that) const
{
   if ((const bblock_t *)this->link.next != that)
      return false;

   if (ends_block(this->end()) ||
       starts_block(that->start()))
      return false;

   return true;
}

void
bblock_t::combine_with(bblock_t *that)
{
   assert(this->can_combine_with(that));
   foreach_list_typed (bblock_link, link, link, &that->parents) {
      assert(link->block == this);
   }

   this->end_ip = that->end_ip;
   this->instructions.append_list(&that->instructions);

   this->cfg->remove_block(that);
}

void
bblock_t::dump(FILE *file) const
{
   const backend_shader *s = this->cfg->s;

   int ip = this->start_ip;
   foreach_inst_in_block(backend_instruction, inst, this) {
      fprintf(file, "%5d: ", ip);
      s->dump_instruction(inst, file);
      ip++;
   }
}

void
bblock_t::unlink_list(exec_list *list)
{
   assert(list == &parents || list == &children);
   const bool remove_parent = list == &children;

   foreach_list_typed_safe(bblock_link, link, link, list) {
      /* Also break the links from the other block back to this block. */
      exec_list *sub_list = remove_parent ? &link->block->parents : &link->block->children;

      foreach_list_typed_safe(bblock_link, sub_link, link, sub_list) {
         if (sub_link->block == this) {
            sub_link->link.remove();
            ralloc_free(sub_link);
         }
      }

      link->link.remove();
      ralloc_free(link);
   }
}

cfg_t::cfg_t(const backend_shader *s, exec_list *instructions) :
   s(s)
{
   mem_ctx = ralloc_context(NULL);
   block_list.make_empty();
   blocks = NULL;
   num_blocks = 0;

   bblock_t *cur = NULL;
   int ip = 0;

   bblock_t *entry = new_block();
   bblock_t *cur_if = NULL;    /**< BB ending with IF. */
   bblock_t *cur_else = NULL;  /**< BB ending with ELSE. */
   bblock_t *cur_do = NULL;    /**< BB starting with DO. */
   bblock_t *cur_while = NULL; /**< BB immediately following WHILE. */
   exec_list if_stack, else_stack, do_stack, while_stack;
   bblock_t *next;

   set_next_block(&cur, entry, ip);

   foreach_in_list_safe(backend_instruction, inst, instructions) {
      /* set_next_block wants the post-incremented ip */
      ip++;

      inst->exec_node::remove();

      switch (inst->opcode) {
      case BRW_OPCODE_IF:
         cur->instructions.push_tail(inst);

	 /* Push our information onto a stack so we can recover from
	  * nested ifs.
	  */
         push_stack(&if_stack, mem_ctx, cur_if);
         push_stack(&else_stack, mem_ctx, cur_else);

	 cur_if = cur;
	 cur_else = NULL;

	 /* Set up our immediately following block, full of "then"
	  * instructions.
	  */
	 next = new_block();
         cur_if->add_successor(mem_ctx, next, bblock_link_logical);

	 set_next_block(&cur, next, ip);
	 break;

      case BRW_OPCODE_ELSE:
         cur->instructions.push_tail(inst);

         cur_else = cur;

	 next = new_block();
         assert(cur_if != NULL);
         cur_if->add_successor(mem_ctx, next, bblock_link_logical);
         cur_else->add_successor(mem_ctx, next, bblock_link_physical);

	 set_next_block(&cur, next, ip);
	 break;

      case BRW_OPCODE_ENDIF: {
         bblock_t *cur_endif;

         if (cur->instructions.is_empty()) {
            /* New block was just created; use it. */
            cur_endif = cur;
         } else {
            cur_endif = new_block();

            cur->add_successor(mem_ctx, cur_endif, bblock_link_logical);

            set_next_block(&cur, cur_endif, ip - 1);
         }

         cur->instructions.push_tail(inst);

         if (cur_else) {
            cur_else->add_successor(mem_ctx, cur_endif, bblock_link_logical);
         } else {
            assert(cur_if != NULL);
            cur_if->add_successor(mem_ctx, cur_endif, bblock_link_logical);
         }

         assert(cur_if->end()->opcode == BRW_OPCODE_IF);
         assert(!cur_else || cur_else->end()->opcode == BRW_OPCODE_ELSE);

	 /* Pop the stack so we're in the previous if/else/endif */
	 cur_if = pop_stack(&if_stack);
	 cur_else = pop_stack(&else_stack);
	 break;
      }
      case BRW_OPCODE_DO:
	 /* Push our information onto a stack so we can recover from
	  * nested loops.
	  */
         push_stack(&do_stack, mem_ctx, cur_do);
         push_stack(&while_stack, mem_ctx, cur_while);

	 /* Set up the block just after the while.  Don't know when exactly
	  * it will start, yet.
	  */
	 cur_while = new_block();

         if (cur->instructions.is_empty()) {
            /* New block was just created; use it. */
            cur_do = cur;
         } else {
            cur_do = new_block();

            cur->add_successor(mem_ctx, cur_do, bblock_link_logical);

            set_next_block(&cur, cur_do, ip - 1);
         }

         cur->instructions.push_tail(inst);

         /* Represent divergent execution of the loop as a pair of alternative
          * edges coming out of the DO instruction: For any physical iteration
          * of the loop a given logical thread can either start off enabled
          * (which is represented as the "next" successor), or disabled (if it
          * has reached a non-uniform exit of the loop during a previous
          * iteration, which is represented as the "cur_while" successor).
          *
          * The disabled edge will be taken by the logical thread anytime we
          * arrive at the DO instruction through a back-edge coming from a
          * conditional exit of the loop where divergent control flow started.
          *
          * This guarantees that there is a control-flow path from any
          * divergence point of the loop into the convergence point
          * (immediately past the WHILE instruction) such that it overlaps the
          * whole IP region of divergent control flow (potentially the whole
          * loop) *and* doesn't imply the execution of any instructions part
          * of the loop (since the corresponding execution mask bit will be
          * disabled for a diverging thread).
          *
          * This way we make sure that any variables that are live throughout
          * the region of divergence for an inactive logical thread are also
          * considered to interfere with any other variables assigned by
          * active logical threads within the same physical region of the
          * program, since otherwise we would risk cross-channel data
          * corruption.
          */
         next = new_block();
         cur->add_successor(mem_ctx, next, bblock_link_logical);
         cur->add_successor(mem_ctx, cur_while, bblock_link_physical);
         set_next_block(&cur, next, ip);
	 break;

      case BRW_OPCODE_CONTINUE:
         cur->instructions.push_tail(inst);

         /* A conditional CONTINUE may start a region of divergent control
          * flow until the start of the next loop iteration (*not* until the
          * end of the loop which is why the successor is not the top-level
          * divergence point at cur_do).  The live interval of any variable
          * extending through a CONTINUE edge is guaranteed to overlap the
          * whole region of divergent execution, because any variable live-out
          * at the CONTINUE instruction will also be live-in at the top of the
          * loop, and therefore also live-out at the bottom-most point of the
          * loop which is reachable from the top (since a control flow path
          * exists from a definition of the variable through this CONTINUE
          * instruction, the top of the loop, the (reachable) bottom of the
          * loop, the top of the loop again, into a use of the variable).
          */
         assert(cur_do != NULL);
         cur->add_successor(mem_ctx, cur_do->next(), bblock_link_logical);

	 next = new_block();
	 if (inst->predicate)
            cur->add_successor(mem_ctx, next, bblock_link_logical);
         else
            cur->add_successor(mem_ctx, next, bblock_link_physical);

	 set_next_block(&cur, next, ip);
	 break;

      case BRW_OPCODE_BREAK:
         cur->instructions.push_tail(inst);

         /* A conditional BREAK instruction may start a region of divergent
          * control flow until the end of the loop if the condition is
          * non-uniform, in which case the loop will execute additional
          * iterations with the present channel disabled.  We model this as a
          * control flow path from the divergence point to the convergence
          * point that overlaps the whole IP range of the loop and skips over
          * the execution of any other instructions part of the loop.
          *
          * See the DO case for additional explanation.
          */
         assert(cur_do != NULL);
         cur->add_successor(mem_ctx, cur_do, bblock_link_physical);
         cur->add_successor(mem_ctx, cur_while, bblock_link_logical);

	 next = new_block();
	 if (inst->predicate)
            cur->add_successor(mem_ctx, next, bblock_link_logical);
         else
            cur->add_successor(mem_ctx, next, bblock_link_physical);

	 set_next_block(&cur, next, ip);
	 break;

      case BRW_OPCODE_WHILE:
         cur->instructions.push_tail(inst);

         assert(cur_do != NULL && cur_while != NULL);

         /* A conditional WHILE instruction may start a region of divergent
          * control flow until the end of the loop, just like the BREAK
          * instruction.  See the BREAK case for more details.  OTOH an
          * unconditional WHILE instruction is non-divergent (just like an
          * unconditional CONTINUE), and will necessarily lead to the
          * execution of an additional iteration of the loop for all enabled
          * channels, so we may skip over the divergence point at the top of
          * the loop to keep the CFG as unambiguous as possible.
          */
         if (inst->predicate) {
            cur->add_successor(mem_ctx, cur_do, bblock_link_logical);
         } else {
            cur->add_successor(mem_ctx, cur_do->next(), bblock_link_logical);
         }

	 set_next_block(&cur, cur_while, ip);

	 /* Pop the stack so we're in the previous loop */
	 cur_do = pop_stack(&do_stack);
	 cur_while = pop_stack(&while_stack);
	 break;

      default:
         cur->instructions.push_tail(inst);
	 break;
      }
   }

   cur->end_ip = ip - 1;

   make_block_array();
}

cfg_t::~cfg_t()
{
   ralloc_free(mem_ctx);
}

void
cfg_t::remove_block(bblock_t *block)
{
   foreach_list_typed_safe (bblock_link, predecessor, link, &block->parents) {
      /* cfg_t::validate checks that predecessor and successor lists are well
       * formed, so it is known that the loop here would find exactly one
       * block. Set old_link_kind to silence "variable used but not set"
       * warnings.
       */
      bblock_link_kind old_link_kind = bblock_link_logical;

      /* Remove block from all of its predecessors' successor lists. */
      foreach_list_typed_safe (bblock_link, successor, link,
                               &predecessor->block->children) {
         if (block == successor->block) {
            old_link_kind = successor->kind;
            successor->link.remove();
            ralloc_free(successor);
            break;
         }
      }

      /* Add removed-block's successors to its predecessors' successor lists. */
      foreach_list_typed (bblock_link, successor, link, &block->children) {
         bool need_to_link = true;
         bblock_link_kind new_link_kind = MAX2(old_link_kind, successor->kind);

         foreach_list_typed_safe (bblock_link, child, link, &predecessor->block->children) {
            /* There is already a link between the two blocks. If the links
             * are the same kind or the link is logical, do nothing. If the
             * existing link is physical and the proposed new link is logical,
             * promote the existing link to logical.
             *
             * This is accomplished by taking the minimum of the existing link
             * kind and the proposed link kind.
             */
            if (child->block == successor->block) {
               child->kind = MIN2(child->kind, new_link_kind);
               need_to_link = false;
               break;
            }
         }

         if (need_to_link) {
            predecessor->block->children.push_tail(link(mem_ctx,
                                                        successor->block,
                                                        new_link_kind));
         }
      }
   }

   foreach_list_typed_safe (bblock_link, successor, link, &block->children) {
      /* cfg_t::validate checks that predecessor and successor lists are well
       * formed, so it is known that the loop here would find exactly one
       * block. Set old_link_kind to silence "variable used but not set"
       * warnings.
       */
      bblock_link_kind old_link_kind = bblock_link_logical;

      /* Remove block from all of its childrens' parents lists. */
      foreach_list_typed_safe (bblock_link, predecessor, link,
                               &successor->block->parents) {
         if (block == predecessor->block) {
            old_link_kind = predecessor->kind;
            predecessor->link.remove();
            ralloc_free(predecessor);
         }
      }

      /* Add removed-block's predecessors to its successors' predecessor lists. */
      foreach_list_typed (bblock_link, predecessor, link, &block->parents) {
         bool need_to_link = true;
         bblock_link_kind new_link_kind = MAX2(old_link_kind, predecessor->kind);

         foreach_list_typed_safe (bblock_link, parent, link, &successor->block->parents) {
            /* There is already a link between the two blocks. If the links
             * are the same kind or the link is logical, do nothing. If the
             * existing link is physical and the proposed new link is logical,
             * promote the existing link to logical.
             *
             * This is accomplished by taking the minimum of the existing link
             * kind and the proposed link kind.
             */
            if (parent->block == predecessor->block) {
               parent->kind = MIN2(parent->kind, new_link_kind);
               need_to_link = false;
               break;
            }
         }

         if (need_to_link) {
            successor->block->parents.push_tail(link(mem_ctx,
                                                     predecessor->block,
                                                     new_link_kind));
         }
      }
   }

   block->link.remove();

   for (int b = block->num; b < this->num_blocks - 1; b++) {
      this->blocks[b] = this->blocks[b + 1];
      this->blocks[b]->num = b;
   }

   this->blocks[this->num_blocks - 1]->num = this->num_blocks - 2;
   this->num_blocks--;
}

bblock_t *
cfg_t::new_block()
{
   bblock_t *block = new(mem_ctx) bblock_t(this);

   return block;
}

void
cfg_t::set_next_block(bblock_t **cur, bblock_t *block, int ip)
{
   if (*cur) {
      (*cur)->end_ip = ip - 1;
   }

   block->start_ip = ip;
   block->num = num_blocks++;
   block_list.push_tail(&block->link);
   *cur = block;
}

void
cfg_t::make_block_array()
{
   blocks = ralloc_array(mem_ctx, bblock_t *, num_blocks);

   int i = 0;
   foreach_block (block, this) {
      blocks[i++] = block;
   }
   assert(i == num_blocks);
}

namespace {

struct link_desc {
   char kind;
   int num;
};

int
compare_link_desc(const void *a, const void *b)
{
   const link_desc *la = (const link_desc *)a;
   const link_desc *lb = (const link_desc *)b;

   return la->num < lb->num ? -1 :
          la->num > lb->num ? +1 :
          la->kind < lb->kind ? -1 :
          la->kind > lb->kind ? +1 :
          0;
}

void
sort_links(util_dynarray *scratch, exec_list *list)
{
   util_dynarray_clear(scratch);
   foreach_list_typed(bblock_link, link, link, list) {
      link_desc l;
      l.kind = link->kind == bblock_link_logical ? '-' : '~';
      l.num = link->block->num;
      util_dynarray_append(scratch, link_desc, l);
   }
   qsort(scratch->data, util_dynarray_num_elements(scratch, link_desc),
         sizeof(link_desc), compare_link_desc);
}

} /* namespace */

void
cfg_t::dump(FILE *file)
{
   const idom_tree *idom = (s ? &s->idom_analysis.require() : NULL);

   /* Temporary storage to sort the lists of blocks.  This normalizes the
    * output, making it possible to use it for certain tests.
    */
   util_dynarray scratch;
   util_dynarray_init(&scratch, NULL);

   foreach_block (block, this) {
      if (idom && idom->parent(block))
         fprintf(file, "START B%d IDOM(B%d)", block->num,
                 idom->parent(block)->num);
      else
         fprintf(file, "START B%d IDOM(none)", block->num);

      sort_links(&scratch, &block->parents);
      util_dynarray_foreach(&scratch, link_desc, l)
         fprintf(file, " <%cB%d", l->kind, l->num);
      fprintf(file, "\n");

      if (s != NULL)
         block->dump(file);
      fprintf(file, "END B%d", block->num);

      sort_links(&scratch, &block->children);
      util_dynarray_foreach(&scratch, link_desc, l)
         fprintf(file, " %c>B%d", l->kind, l->num);
      fprintf(file, "\n");
   }

   util_dynarray_fini(&scratch);
}

/* Calculates the immediate dominator of each block, according to "A Simple,
 * Fast Dominance Algorithm" by Keith D. Cooper, Timothy J. Harvey, and Ken
 * Kennedy.
 *
 * The authors claim that for control flow graphs of sizes normally encountered
 * (less than 1000 nodes) that this algorithm is significantly faster than
 * others like Lengauer-Tarjan.
 */
idom_tree::idom_tree(const backend_shader *s) :
   num_parents(s->cfg->num_blocks),
   parents(new bblock_t *[num_parents]())
{
   bool changed;

   parents[0] = s->cfg->blocks[0];

   do {
      changed = false;

      foreach_block(block, s->cfg) {
         if (block->num == 0)
            continue;

         bblock_t *new_idom = NULL;
         foreach_list_typed(bblock_link, parent_link, link, &block->parents) {
            if (parent(parent_link->block)) {
               new_idom = (new_idom ? intersect(new_idom, parent_link->block) :
                           parent_link->block);
            }
         }

         if (parent(block) != new_idom) {
            parents[block->num] = new_idom;
            changed = true;
         }
      }
   } while (changed);
}

idom_tree::~idom_tree()
{
   delete[] parents;
}

bblock_t *
idom_tree::intersect(bblock_t *b1, bblock_t *b2) const
{
   /* Note, the comparisons here are the opposite of what the paper says
    * because we index blocks from beginning -> end (i.e. reverse post-order)
    * instead of post-order like they assume.
    */
   while (b1->num != b2->num) {
      while (b1->num > b2->num)
         b1 = parent(b1);
      while (b2->num > b1->num)
         b2 = parent(b2);
   }
   assert(b1);
   return b1;
}

void
idom_tree::dump() const
{
   printf("digraph DominanceTree {\n");
   for (unsigned i = 0; i < num_parents; i++)
      printf("\t%d -> %d\n", parents[i]->num, i);
   printf("}\n");
}

void
cfg_t::dump_cfg()
{
   printf("digraph CFG {\n");
   for (int b = 0; b < num_blocks; b++) {
      bblock_t *block = this->blocks[b];

      foreach_list_typed_safe (bblock_link, child, link, &block->children) {
         printf("\t%d -> %d\n", b, child->block->num);
      }
   }
   printf("}\n");
}

#define cfgv_assert(assertion)                                          \
   {                                                                    \
      if (!(assertion)) {                                               \
         fprintf(stderr, "ASSERT: CFG validation in %s failed!\n", stage_abbrev); \
         fprintf(stderr, "%s:%d: '%s' failed\n", __FILE__, __LINE__, #assertion);  \
         abort();                                                       \
      }                                                                 \
   }

#ifndef NDEBUG
void
cfg_t::validate(const char *stage_abbrev)
{
   foreach_block(block, this) {
      foreach_list_typed(bblock_link, successor, link, &block->children) {
         /* Each successor of a block must have one predecessor link back to
          * the block.
          */
         bool successor_links_back_to_predecessor = false;
         bblock_t *succ_block = successor->block;

         foreach_list_typed(bblock_link, predecessor, link, &succ_block->parents) {
            if (predecessor->block == block) {
               cfgv_assert(!successor_links_back_to_predecessor);
               cfgv_assert(successor->kind == predecessor->kind);
               successor_links_back_to_predecessor = true;
            }
         }

         cfgv_assert(successor_links_back_to_predecessor);

         /* Each successor block must appear only once in the list of
          * successors.
          */
         foreach_list_typed_from(bblock_link, later_successor, link,
                                 &block->children, successor->link.next) {
            cfgv_assert(successor->block != later_successor->block);
         }
      }

      foreach_list_typed(bblock_link, predecessor, link, &block->parents) {
         /* Each predecessor of a block must have one successor link back to
          * the block.
          */
         bool predecessor_links_back_to_successor = false;
         bblock_t *pred_block = predecessor->block;

         foreach_list_typed(bblock_link, successor, link, &pred_block->children) {
            if (successor->block == block) {
               cfgv_assert(!predecessor_links_back_to_successor);
               cfgv_assert(successor->kind == predecessor->kind);
               predecessor_links_back_to_successor = true;
            }
         }

         cfgv_assert(predecessor_links_back_to_successor);

         /* Each precessor block must appear only once in the list of
          * precessors.
          */
         foreach_list_typed_from(bblock_link, later_precessor, link,
                                 &block->parents, predecessor->link.next) {
            cfgv_assert(predecessor->block != later_precessor->block);
         }
      }

      backend_instruction *first_inst = block->start();
      if (first_inst->opcode == BRW_OPCODE_DO) {
         /* DO instructions both begin and end a block, so the DO instruction
          * must be the only instruction in the block.
          */
         cfgv_assert(exec_list_is_singular(&block->instructions));

         /* A block starting with DO should have exactly two successors. One
          * is a physical link to the block starting after the WHILE
          * instruction. The other is a logical link to the block starting the
          * body of the loop.
          */
         bblock_t *physical_block = nullptr;
         bblock_t *logical_block = nullptr;

         foreach_list_typed(bblock_link, child, link, &block->children) {
            if (child->kind == bblock_link_physical) {
               cfgv_assert(physical_block == nullptr);
               physical_block = child->block;
            } else {
               cfgv_assert(logical_block == nullptr);
               logical_block = child->block;
            }
         }

         cfgv_assert(logical_block != nullptr);
         cfgv_assert(physical_block != nullptr);
      }
   }
}
#endif
