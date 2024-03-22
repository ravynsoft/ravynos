/*
 * Copyright © 2019 Intel Corporation
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

#include "sparse_array.h"
#include "os_memory.h"

/* Aligning our allocations to 64 has two advantages:
 *
 *  1. On x86 platforms, it means that they are cache-line aligned so we
 *     reduce the likelihood that one of our allocations shares a cache line
 *     with some other allocation.
 *
 *  2. It lets us use the bottom 6 bits of the pointer to store the tree level
 *     of the node so we can avoid some pointer indirections.
 */
#define NODE_ALLOC_ALIGN 64

void
util_sparse_array_init(struct util_sparse_array *arr,
                       size_t elem_size, size_t node_size)
{
   memset(arr, 0, sizeof(*arr));
   arr->elem_size = elem_size;
   arr->node_size_log2 = util_logbase2_64(node_size);
   assert(node_size >= 2 && node_size == (1ull << arr->node_size_log2));
}

#define NODE_PTR_MASK (~((uintptr_t)NODE_ALLOC_ALIGN - 1))
#define NODE_LEVEL_MASK ((uintptr_t)NODE_ALLOC_ALIGN - 1)
#define NULL_NODE 0

static inline uintptr_t
_util_sparse_array_node(void *data, unsigned level)
{
   assert(data != NULL);
   assert(((uintptr_t)data & NODE_LEVEL_MASK) == 0);
   assert((level & NODE_PTR_MASK) == 0);
   return (uintptr_t)data | level;
}

static inline void *
_util_sparse_array_node_data(uintptr_t handle)
{
   return (void *)(handle & NODE_PTR_MASK);
}

static inline unsigned
_util_sparse_array_node_level(uintptr_t handle)
{
   return handle & NODE_LEVEL_MASK;
}

static inline void
_util_sparse_array_node_finish(struct util_sparse_array *arr,
                               uintptr_t node)
{
   if (_util_sparse_array_node_level(node) > 0) {
      uintptr_t *children = _util_sparse_array_node_data(node);
      size_t node_size = 1ull << arr->node_size_log2;
      for (size_t i = 0; i < node_size; i++) {
         if (children[i])
            _util_sparse_array_node_finish(arr, children[i]);
      }
   }

   os_free_aligned(_util_sparse_array_node_data(node));
}

void
util_sparse_array_finish(struct util_sparse_array *arr)
{
   if (arr->root)
      _util_sparse_array_node_finish(arr, arr->root);
}

static inline uintptr_t
_util_sparse_array_node_alloc(struct util_sparse_array *arr,
                              unsigned level)
{
   size_t size;
   if (level == 0) {
      size = arr->elem_size << arr->node_size_log2;
   } else {
      size = sizeof(uintptr_t) << arr->node_size_log2;
   }

   void *data = os_malloc_aligned(size, NODE_ALLOC_ALIGN);
   memset(data, 0, size);

   return _util_sparse_array_node(data, level);
}

static inline uintptr_t
_util_sparse_array_set_or_free_node(uintptr_t *node_ptr,
                                    uintptr_t cmp_node,
                                    uintptr_t node)
{
   uintptr_t prev_node = p_atomic_cmpxchg(node_ptr, cmp_node, node);

   if (prev_node != cmp_node) {
      /* We lost the race.  Free this one and return the one that was already
       * allocated.
       */
      os_free_aligned(_util_sparse_array_node_data(node));
      return prev_node;
   } else {
      return node;
   }
}

void *
util_sparse_array_get(struct util_sparse_array *arr, uint64_t idx)
{
   const unsigned node_size_log2 = arr->node_size_log2;
   uintptr_t root = p_atomic_read(&arr->root);
   if (unlikely(!root)) {
      unsigned root_level = 0;
      uint64_t idx_iter = idx >> node_size_log2;
      while (idx_iter) {
         idx_iter >>= node_size_log2;
         root_level++;
      }
      uintptr_t new_root = _util_sparse_array_node_alloc(arr, root_level);
      root = _util_sparse_array_set_or_free_node(&arr->root,
                                                 NULL_NODE, new_root);
   }

   while (1) {
      unsigned root_level = _util_sparse_array_node_level(root);
      uint64_t root_idx = idx >> (root_level * node_size_log2);
      if (likely(root_idx < (1ull << node_size_log2)))
         break;

      /* In this case, we have a root but its level is low enough that the
       * requested index is out-of-bounds.
       */
      uintptr_t new_root = _util_sparse_array_node_alloc(arr, root_level + 1);

      uintptr_t *new_root_children = _util_sparse_array_node_data(new_root);
      new_root_children[0] = root;

      /* We only add one at a time instead of the whole tree because it's
       * easier to ensure correctness of both the tree building and the
       * clean-up path.  Because we're only adding one node we never have to
       * worry about trying to free multiple things without freeing the old
       * things.
       */
      root = _util_sparse_array_set_or_free_node(&arr->root, root, new_root);
   }

   void *node_data = _util_sparse_array_node_data(root);
   unsigned node_level = _util_sparse_array_node_level(root);
   while (node_level > 0) {
      uint64_t child_idx = (idx >> (node_level * node_size_log2)) &
                           ((1ull << node_size_log2) - 1);

      uintptr_t *children = node_data;
      uintptr_t child = p_atomic_read(&children[child_idx]);

      if (unlikely(!child)) {
         child = _util_sparse_array_node_alloc(arr, node_level - 1);
         child = _util_sparse_array_set_or_free_node(&children[child_idx],
                                                     NULL_NODE, child);
      }

      node_data = _util_sparse_array_node_data(child);
      node_level = _util_sparse_array_node_level(child);
   }

   uint64_t elem_idx = idx & ((1ull << node_size_log2) - 1);
   return (void *)((char *)node_data + (elem_idx * arr->elem_size));
}

static void
validate_node_level(struct util_sparse_array *arr,
                    uintptr_t node, unsigned level)
{
   assert(_util_sparse_array_node_level(node) == level);

   if (_util_sparse_array_node_level(node) > 0) {
      uintptr_t *children = _util_sparse_array_node_data(node);
      size_t node_size = 1ull << arr->node_size_log2;
      for (size_t i = 0; i < node_size; i++) {
         if (children[i])
            validate_node_level(arr, children[i], level - 1);
      }
   }
}

void
util_sparse_array_validate(struct util_sparse_array *arr)
{
   uintptr_t root = p_atomic_read(&arr->root);
   validate_node_level(arr, root, _util_sparse_array_node_level(root));
}

void
util_sparse_array_free_list_init(struct util_sparse_array_free_list *fl,
                                 struct util_sparse_array *arr,
                                 uint32_t sentinel,
                                 uint32_t next_offset)
{
   fl->head = sentinel;
   fl->arr = arr;
   fl->sentinel = sentinel;
   fl->next_offset = next_offset;
}

static uint64_t
free_list_head(uint64_t old, uint32_t next)
{
   return ((old & 0xffffffff00000000ull) + 0x100000000ull) | next;
}

void
util_sparse_array_free_list_push(struct util_sparse_array_free_list *fl,
                                 uint32_t *items, unsigned num_items)
{
   assert(num_items > 0);
   assert(items[0] != fl->sentinel);
   void *last_elem = util_sparse_array_get(fl->arr, items[0]);
   uint32_t *last_next = (uint32_t *)((char *)last_elem + fl->next_offset);
   for (unsigned i = 1; i < num_items; i++) {
      p_atomic_set(last_next, items[i]);
      assert(items[i] != fl->sentinel);
      last_elem = util_sparse_array_get(fl->arr, items[i]);
      last_next = (uint32_t *)((char *)last_elem + fl->next_offset);
   }

   uint64_t current_head, old_head;
   old_head = p_atomic_read(&fl->head);
   do {
      current_head = old_head;
      p_atomic_set(last_next, (uint32_t)current_head); /* Index is the bottom 32 bits */
      uint64_t new_head = free_list_head(current_head, items[0]);
      old_head = p_atomic_cmpxchg(&fl->head, current_head, new_head);
   } while (old_head != current_head);
}

uint32_t
util_sparse_array_free_list_pop_idx(struct util_sparse_array_free_list *fl)
{
   uint64_t current_head;

   current_head = p_atomic_read(&fl->head);
   while (1) {
      if ((uint32_t)current_head == fl->sentinel)
         return fl->sentinel;

      uint32_t head_idx = current_head; /* Index is the bottom 32 bits */
      void *head_elem = util_sparse_array_get(fl->arr, head_idx);
      uint32_t *head_next = (uint32_t *)((char *)head_elem + fl->next_offset);
      uint64_t new_head = free_list_head(current_head, p_atomic_read(head_next));
      uint64_t old_head = p_atomic_cmpxchg(&fl->head, current_head, new_head);
      if (old_head == current_head)
         return head_idx;
      current_head = old_head;
   }
}

void *
util_sparse_array_free_list_pop_elem(struct util_sparse_array_free_list *fl)
{
   uint64_t current_head;

   current_head = p_atomic_read(&fl->head);
   while (1) {
      if ((uint32_t)current_head == fl->sentinel)
         return NULL;

      uint32_t head_idx = current_head; /* Index is the bottom 32 bits */
      void *head_elem = util_sparse_array_get(fl->arr, head_idx);
      uint32_t *head_next = (uint32_t *)((char *)head_elem + fl->next_offset);
      uint64_t new_head = free_list_head(current_head, p_atomic_read(head_next));
      uint64_t old_head = p_atomic_cmpxchg(&fl->head, current_head, new_head);
      if (old_head == current_head)
         return head_elem;
      current_head = old_head;
   }
}
