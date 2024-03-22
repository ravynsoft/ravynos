/*
 * Copyright 2010 Marek Olšák <maraeo@gmail.com>
 * Copyright 2016 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "slab.h"
#include "macros.h"
#include "u_atomic.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define SLAB_MAGIC_ALLOCATED 0xcafe4321
#define SLAB_MAGIC_FREE 0x7ee01234

#ifndef NDEBUG
#define SET_MAGIC(element, value)   (element)->magic = (value)
#define CHECK_MAGIC(element, value) assert((element)->magic == (value))
#else
#define SET_MAGIC(element, value)
#define CHECK_MAGIC(element, value)
#endif

/* One array element within a big buffer. */
struct slab_element_header {
   /* The next element in the free or migrated list. */
   struct slab_element_header *next;

   /* This is either
    * - a pointer to the child pool to which this element belongs, or
    * - a pointer to the orphaned page of the element, with the least
    *   significant bit set to 1.
    */
   intptr_t owner;

#ifndef NDEBUG
   intptr_t magic;
#endif
};

/* The page is an array of allocations in one block. */
struct slab_page_header {
   union {
      /* Next page in the same child pool. */
      struct slab_page_header *next;

      /* Number of remaining, non-freed elements (for orphaned pages). */
      unsigned num_remaining;
   } u;
   /* Memory after the last member is dedicated to the page itself.
    * The allocated size is always larger than this structure.
    */
};


static struct slab_element_header *
slab_get_element(struct slab_parent_pool *parent,
                 struct slab_page_header *page, unsigned index)
{
   return (struct slab_element_header*)
          ((uint8_t*)&page[1] + (parent->element_size * index));
}

/* The given object/element belongs to an orphaned page (i.e. the owning child
 * pool has been destroyed). Mark the element as freed and free the whole page
 * when no elements are left in it.
 */
static void
slab_free_orphaned(struct slab_element_header *elt)
{
   struct slab_page_header *page;

   assert(elt->owner & 1);

   page = (struct slab_page_header *)(elt->owner & ~(intptr_t)1);
   if (!p_atomic_dec_return(&page->u.num_remaining))
      free(page);
}

/**
 * Create a parent pool for the allocation of same-sized objects.
 *
 * \param item_size     Size of one object.
 * \param num_items     Number of objects to allocate at once.
 */
void
slab_create_parent(struct slab_parent_pool *parent,
                   unsigned item_size,
                   unsigned num_items)
{
   simple_mtx_init(&parent->mutex, mtx_plain);
   parent->element_size = ALIGN_POT(sizeof(struct slab_element_header) + item_size,
                                    sizeof(intptr_t));
   parent->num_elements = num_items;
   parent->item_size = item_size;
}

void
slab_destroy_parent(struct slab_parent_pool *parent)
{
   simple_mtx_destroy(&parent->mutex);
}

/**
 * Create a child pool linked to the given parent.
 */
void slab_create_child(struct slab_child_pool *pool,
                       struct slab_parent_pool *parent)
{
   pool->parent = parent;
   pool->pages = NULL;
   pool->free = NULL;
   pool->migrated = NULL;
}

/**
 * Destroy the child pool.
 *
 * Pages associated to the pool will be orphaned. They are eventually freed
 * when all objects in them are freed.
 */
void slab_destroy_child(struct slab_child_pool *pool)
{
   if (!pool->parent)
      return; /* the slab probably wasn't even created */

   simple_mtx_lock(&pool->parent->mutex);

   while (pool->pages) {
      struct slab_page_header *page = pool->pages;
      pool->pages = page->u.next;
      p_atomic_set(&page->u.num_remaining, pool->parent->num_elements);

      for (unsigned i = 0; i < pool->parent->num_elements; ++i) {
         struct slab_element_header *elt = slab_get_element(pool->parent, page, i);
         p_atomic_set(&elt->owner, (intptr_t)page | 1);
      }
   }

   while (pool->migrated) {
      struct slab_element_header *elt = pool->migrated;
      pool->migrated = elt->next;
      slab_free_orphaned(elt);
   }

   simple_mtx_unlock(&pool->parent->mutex);

   while (pool->free) {
      struct slab_element_header *elt = pool->free;
      pool->free = elt->next;
      slab_free_orphaned(elt);
   }

   /* Guard against use-after-free. */
   pool->parent = NULL;
}

static bool
slab_add_new_page(struct slab_child_pool *pool)
{
   struct slab_page_header *page = malloc(sizeof(struct slab_page_header) +
      pool->parent->num_elements * pool->parent->element_size);

   if (!page)
      return false;

   for (unsigned i = 0; i < pool->parent->num_elements; ++i) {
      struct slab_element_header *elt = slab_get_element(pool->parent, page, i);
      elt->owner = (intptr_t)pool;
      assert(!(elt->owner & 1));

      elt->next = pool->free;
      pool->free = elt;
      SET_MAGIC(elt, SLAB_MAGIC_FREE);
   }

   page->u.next = pool->pages;
   pool->pages = page;

   return true;
}

/**
 * Allocate an object from the child pool. Single-threaded (i.e. the caller
 * must ensure that no operation happens on the same child pool in another
 * thread).
 */
void *
slab_alloc(struct slab_child_pool *pool)
{
   struct slab_element_header *elt;

   if (!pool->free) {
      /* First, collect elements that belong to us but were freed from a
       * different child pool.
       */
      simple_mtx_lock(&pool->parent->mutex);
      pool->free = pool->migrated;
      pool->migrated = NULL;
      simple_mtx_unlock(&pool->parent->mutex);

      /* Now allocate a new page. */
      if (!pool->free && !slab_add_new_page(pool))
         return NULL;
   }

   elt = pool->free;
   pool->free = elt->next;

   CHECK_MAGIC(elt, SLAB_MAGIC_FREE);
   SET_MAGIC(elt, SLAB_MAGIC_ALLOCATED);

   return &elt[1];
}

/**
 * Same as slab_alloc but memset the returned object to 0.
 */
void *
slab_zalloc(struct slab_child_pool *pool)
{
   void *r = slab_alloc(pool);
   if (r)
      memset(r, 0, pool->parent->item_size);
   return r;
}

/**
 * Free an object allocated from the slab. Single-threaded (i.e. the caller
 * must ensure that no operation happens on the same child pool in another
 * thread).
 *
 * Freeing an object in a different child pool from the one where it was
 * allocated is allowed, as long the pool belong to the same parent. No
 * additional locking is required in this case.
 */
void slab_free(struct slab_child_pool *pool, void *ptr)
{
   struct slab_element_header *elt = ((struct slab_element_header*)ptr - 1);
   intptr_t owner_int;

   CHECK_MAGIC(elt, SLAB_MAGIC_ALLOCATED);
   SET_MAGIC(elt, SLAB_MAGIC_FREE);

   if (p_atomic_read(&elt->owner) == (intptr_t)pool) {
      /* This is the simple case: The caller guarantees that we can safely
       * access the free list.
       */
      elt->next = pool->free;
      pool->free = elt;
      return;
   }

   /* The slow case: migration or an orphaned page. */
   if (pool->parent)
      simple_mtx_lock(&pool->parent->mutex);

   /* Note: we _must_ re-read elt->owner here because the owning child pool
    * may have been destroyed by another thread in the meantime.
    */
   owner_int = p_atomic_read(&elt->owner);

   if (!(owner_int & 1)) {
      struct slab_child_pool *owner = (struct slab_child_pool *)owner_int;
      elt->next = owner->migrated;
      owner->migrated = elt;
      if (pool->parent)
         simple_mtx_unlock(&pool->parent->mutex);
   } else {
      if (pool->parent)
         simple_mtx_unlock(&pool->parent->mutex);

      slab_free_orphaned(elt);
   }
}

/**
 * Allocate an object from the slab. Single-threaded (no mutex).
 */
void *
slab_alloc_st(struct slab_mempool *mempool)
{
   return slab_alloc(&mempool->child);
}

/**
 * Free an object allocated from the slab. Single-threaded (no mutex).
 */
void
slab_free_st(struct slab_mempool *mempool, void *ptr)
{
   slab_free(&mempool->child, ptr);
}

void
slab_destroy(struct slab_mempool *mempool)
{
   slab_destroy_child(&mempool->child);
   slab_destroy_parent(&mempool->parent);
}

/**
 * Create an allocator for same-sized objects.
 *
 * \param item_size     Size of one object.
 * \param num_items     Number of objects to allocate at once.
 */
void
slab_create(struct slab_mempool *mempool,
            unsigned item_size,
            unsigned num_items)
{
   slab_create_parent(&mempool->parent, item_size, num_items);
   slab_create_child(&mempool->child, &mempool->parent);
}
