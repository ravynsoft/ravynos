/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * Hash table implementation.
 *
 * This file provides a hash implementation that is capable of dealing
 * with collisions. It stores colliding entries in linked list. All
 * functions operating on the hash return an iterator. The iterator
 * itself points to the collision list. If there wasn't any collision
 * the list will have just one entry, otherwise client code should
 * iterate over the entries to find the exact entry among ones that
 * had the same key (e.g. memcmp could be used on the data to check
 * that)
 *
 * @author Zack Rusin <zackr@vmware.com>
 */

#ifndef CSO_HASH_H
#define CSO_HASH_H

#include "util/compiler.h"

#ifdef __cplusplus
extern "C" {
#endif


struct cso_node {
   struct cso_node *next;
   void *value;
   unsigned key;
};

struct cso_hash_iter {
   struct cso_hash *hash;
   struct cso_node *node;
};

struct cso_hash {
   struct cso_node *fakeNext;
   struct cso_node **buckets;
   struct cso_node *end;
   int size;
   short userNumBits;
   short numBits;
   int numBuckets;
};


void
cso_hash_init(struct cso_hash *hash);

void
cso_hash_deinit(struct cso_hash *hash);

int
cso_hash_size(const struct cso_hash *hash);


/**
 * Adds a data with the given key to the hash. If entry with the given
 * key is already in the hash, this current entry is instered before it
 * in the collision list.
 * Function returns iterator pointing to the inserted item in the hash.
 */
struct cso_hash_iter
cso_hash_insert(struct cso_hash *hash, unsigned key, void *data);


/**
 * Removes the item pointed to by the current iterator from the hash.
 * Note that the data itself is not erased and if it was a malloc'ed pointer
 * it will have to be freed after calling this function by the callee.
 * Function returns iterator pointing to the item after the removed one in
 * the hash.
 */
struct cso_hash_iter
cso_hash_erase(struct cso_hash *hash, struct cso_hash_iter iter);

void *
cso_hash_take(struct cso_hash *hash, unsigned key);

struct cso_hash_iter
cso_hash_first_node(struct cso_hash *hash);

/**
 * Returns true if a value with the given key exists in the hash
 */
bool
cso_hash_contains(struct cso_hash *hash, unsigned key);

unsigned
cso_hash_iter_key(struct cso_hash_iter iter);


/**
 * Convenience routine to iterate over the collision list while doing a memory
 * comparison to see which entry in the list is a direct copy of our template
 * and returns that entry.
 */
void *
cso_hash_find_data_from_template(struct cso_hash *hash,
                                 unsigned hash_key,
                                 void *templ,
                                 int size);

struct cso_node *
cso_hash_data_next(struct cso_node *node);


static inline bool
cso_hash_iter_is_null(struct cso_hash_iter iter)
{
   return !iter.node || iter.node == iter.hash->end;
}


static inline void *
cso_hash_iter_data(struct cso_hash_iter iter)
{
   if (!iter.node || iter.hash->end == iter.node)
      return NULL;
   return iter.node->value;
}


static inline struct cso_node **
cso_hash_find_node(struct cso_hash *hash, unsigned akey)
{
   struct cso_node **node;

   if (hash->numBuckets) {
      node = &hash->buckets[akey % hash->numBuckets];
      assert(*node == hash->end || (*node)->next);
      while (*node != hash->end && (*node)->key != akey)
         node = &(*node)->next;
   } else {
      node = &hash->end;
   }
   return node;
}


/**
 * Return an iterator pointing to the first entry in the collision list.
 */
static inline struct cso_hash_iter
cso_hash_find(struct cso_hash *hash, unsigned key)
{
   struct cso_node **nextNode = cso_hash_find_node(hash, key);
   struct cso_hash_iter iter = {hash, *nextNode};
   return iter;
}


static inline struct cso_hash_iter
cso_hash_iter_next(struct cso_hash_iter iter)
{
   struct cso_hash_iter next = {iter.hash, cso_hash_data_next(iter.node)};
   return next;
}

#ifdef __cplusplus
}
#endif

#endif
