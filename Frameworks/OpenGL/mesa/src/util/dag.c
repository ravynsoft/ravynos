/*
 * Copyright © 2019 Broadcom
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

#include "util/set.h"
#include "util/dag.h"
#include <stdio.h>

static void
append_edge(struct dag_node *parent, struct dag_node *child, uintptr_t data)
{
   /* Remove the child as a DAG head. */
   list_delinit(&child->link);

   struct dag_edge edge = {
      .child = child,
      .data = data,
   };

   util_dynarray_append(&parent->edges, struct dag_edge, edge);
   child->parent_count++;
}

/**
 * Adds a directed edge from the parent node to the child.
 *
 * Both nodes should have been initialized with dag_init_node().  The edge
 * list may contain multiple edges to the same child with different data.
 */
void
dag_add_edge(struct dag_node *parent, struct dag_node *child, uintptr_t data)
{
   util_dynarray_foreach(&parent->edges, struct dag_edge, edge) {
      if (edge->child == child && edge->data == data)
         return;
   }

   append_edge(parent, child, data);
}

/**
 * Adds a directed edge from the parent node to the child.
 *
 * Both nodes should have been initialized with dag_init_node(). If there is
 * already an existing edge, the data is updated to the maximum of the
 * previous data and the new data. This is useful if the data represents a
 * delay.
 */
void
dag_add_edge_max_data(struct dag_node *parent, struct dag_node *child,
                      uintptr_t data)
{
   util_dynarray_foreach(&parent->edges, struct dag_edge, edge) {
      if (edge->child == child) {
         edge->data = MAX2(edge->data, data);
         return;
      }
   }

   append_edge(parent, child, data);
}

/* Removes a single edge from the graph, promoting the child to a DAG head.
 *
 * Note that calling this other than through dag_prune_head() means that you
 * need to be careful when iterating the edges of remaining nodes for NULL
 * children.
 */
void
dag_remove_edge(struct dag *dag, struct dag_edge *edge)
{
   if (!edge->child)
      return;

   struct dag_node *child = edge->child;
   child->parent_count--;
   if (child->parent_count == 0)
      list_addtail(&child->link, &dag->heads);

   edge->child = NULL;
   edge->data = 0;
}

/**
 * Removes a DAG head from the graph, and moves any new dag heads into the
 * heads list.
 */
void
dag_prune_head(struct dag *dag, struct dag_node *node)
{
   assert(!node->parent_count);

   list_delinit(&node->link);

   util_dynarray_foreach(&node->edges, struct dag_edge, edge) {
      dag_remove_edge(dag, edge);
   }
}

/**
 * Initializes DAG node (probably embedded in some other datastructure in the
 * user).
 */
void
dag_init_node(struct dag *dag, struct dag_node *node)
{
   util_dynarray_init(&node->edges, dag);
   list_addtail(&node->link, &dag->heads);
}

struct dag_traverse_bottom_up_state {
   struct set *seen;
   void (*cb)(struct dag_node *node, void *data);
   void *data;
};

static void
dag_traverse_bottom_up_node(struct dag_node *node,
                            struct dag_traverse_bottom_up_state *state)
{
   if (_mesa_set_search(state->seen, node))
      return;

   struct util_dynarray stack;
   util_dynarray_init(&stack, NULL);

   do {
      assert(node);

      while (node->edges.size != 0) {
         util_dynarray_append(&stack, struct dag_node *, node);

         /* Push unprocessed children onto stack in reverse order. Note that
          * it's possible for any of the children nodes to already be on the
          * stack.
          */
         util_dynarray_foreach_reverse(&node->edges, struct dag_edge, edge) {
            if (!_mesa_set_search(state->seen, edge->child)) {
               util_dynarray_append(&stack, struct dag_node *, edge->child);
            }
         }

         /* Get last element pushed: either left-most child or current node.
          * If it's the current node, that means that we've processed all its
          * children already.
          */
         struct dag_node *top = util_dynarray_pop(&stack, struct dag_node *);
         if (top == node)
            break;
         node = top;
      }

      /* Process the node */
      state->cb(node, state->data);
      _mesa_set_add(state->seen, node);

      /* Find the next unprocessed node in the stack */
      do {
         node = NULL;
         if (stack.size == 0)
            break;

         node = util_dynarray_pop(&stack, struct dag_node *);
      } while (_mesa_set_search(state->seen, node));
   } while (node);

   util_dynarray_fini(&stack);
}

/**
 * Walks the DAG from leaves to the root, ensuring that each node is only seen
 * once its children have been, and each node is only traversed once.
 */
void
dag_traverse_bottom_up(struct dag *dag, void (*cb)(struct dag_node *node,
                                                   void *data), void *data)
{
   struct dag_traverse_bottom_up_state state = {
      .seen = _mesa_pointer_set_create(NULL),
      .data = data,
      .cb = cb,
   };

   list_for_each_entry(struct dag_node, node, &dag->heads, link) {
      dag_traverse_bottom_up_node(node, &state);
   }

   ralloc_free(state.seen);
}

/**
 * Creates an empty DAG datastructure.
 */
struct dag *
dag_create(void *mem_ctx)
{
   struct dag *dag = rzalloc(mem_ctx, struct dag);

   list_inithead(&dag->heads);

   return dag;
}

struct dag_validate_state {
   struct util_dynarray stack;
   struct set *stack_set;
   struct set *seen;
   void (*cb)(const struct dag_node *node, void *data);
   void *data;
};

static void
dag_validate_node(struct dag_node *node,
                  struct dag_validate_state *state)
{
   if (_mesa_set_search(state->stack_set, node)) {
      fprintf(stderr, "DAG validation failed at:\n");
      fprintf(stderr, "  %p: ", node);
      state->cb(node, state->data);
      fprintf(stderr, "\n");
      fprintf(stderr, "Nodes in stack:\n");
      util_dynarray_foreach(&state->stack, struct dag_node *, nodep) {
         struct dag_node *node = *nodep;
         fprintf(stderr, "  %p: ", node);
         state->cb(node, state->data);
         fprintf(stderr, "\n");
      }
      abort();
   }

   if (_mesa_set_search(state->seen, node))
      return;

   _mesa_set_add(state->stack_set, node);
   _mesa_set_add(state->seen, node);
   util_dynarray_append(&state->stack, struct dag_node *, node);

   util_dynarray_foreach(&node->edges, struct dag_edge, edge) {
      dag_validate_node(edge->child, state);
   }

   (void)util_dynarray_pop(&state->stack, struct dag_node *);
   _mesa_set_remove_key(state->stack_set, node);
}

void
dag_validate(struct dag *dag, void (*cb)(const struct dag_node *node,
                                         void *data),
             void *data)
{
   void *mem_ctx = ralloc_context(NULL);
   struct dag_validate_state state = {
      .stack_set = _mesa_pointer_set_create(mem_ctx),
      .seen = _mesa_pointer_set_create(mem_ctx),
      .cb = cb,
      .data = data,
   };

   util_dynarray_init(&state.stack, mem_ctx);

   list_validate(&dag->heads);

   list_for_each_entry(struct dag_node, node, &dag->heads, link) {
      dag_validate_node(node, &state);
   }

   ralloc_free(mem_ctx);
}
