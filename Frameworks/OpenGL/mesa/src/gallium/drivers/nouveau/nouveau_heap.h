/*
 * Copyright 2007 Nouveau Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __NOUVEAU_HEAP_H__
#define __NOUVEAU_HEAP_H__

/* This datastructure represents a memory allocation heap. Fundamentally, this
 * is a doubly-linked list with a few properties, and a usage convention.
 *
 * On initial allocation, there is a single node with the full size that's
 * marked as not in-use. As allocations are made, blocks are taken off the end
 * of that first node, and inserted right after it. If the first node doesn't
 * have enough free space, we look for free space down in the rest of the
 * list. This can happen if an allocation is made and then freed.
 *
 * The first node will remain with in_use == 0 even if the whole heap is
 * exhausted. Another invariant is that there will never be two sequential
 * in_use == 0 nodes. If a node is freed and it has one (or both) adjacent
 * free nodes, they are merged into one, and the relevant heap entries are
 * freed.
 *
 * The pattern to free the whole heap is to start with the first node and then
 * just free the "next" node, until there is no next node. This should assure
 * that at the end the first (and only) node is not in use and contains the
 * full size of the heap.
 */
struct nouveau_heap {
   struct nouveau_heap *prev;
   struct nouveau_heap *next;

   void *priv;

   unsigned start;
   unsigned size;

   int in_use;
};

int
nouveau_heap_init(struct nouveau_heap **heap, unsigned start,
                  unsigned size);

void
nouveau_heap_destroy(struct nouveau_heap **heap);

int
nouveau_heap_alloc(struct nouveau_heap *heap, unsigned size, void *priv,
                   struct nouveau_heap **);

void
nouveau_heap_free(struct nouveau_heap **);

#endif
