/* hash - implement simple hashing table where the keys are memory blocks.
   Copyright (C) 1994-1995, 2000-2006, 2018, 2020, 2023 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, October 1994.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "mem-hash-map.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>

/* Since this simple implementation of hash tables allows only insertion, no
   removal of entries, the right data structure for the memory holding all keys
   is an obstack.  */
#include "obstack.h"

/* Use checked memory allocation.  */
#include "xalloc.h"

#define obstack_chunk_alloc xmalloc
#define obstack_chunk_free free


typedef struct hash_entry
{
  unsigned long used;  /* Hash code of the key, or 0 for an unused entry.  */
  const void *key;     /* Key.  */
  size_t keylen;
  void *data;          /* Value.  */
  struct hash_entry *next;
}
hash_entry;


/* Given an odd CANDIDATE > 1, return true if it is a prime number.  */
static int
is_prime (unsigned long int candidate)
{
  /* No even number and none less than 10 will be passed here.  */
  unsigned long int divn = 3;
  unsigned long int sq = divn * divn;

  while (sq < candidate && candidate % divn != 0)
    {
      ++divn;
      sq += 4 * divn;
      ++divn;
    }

  return candidate % divn != 0;
}


/* Given SEED > 1, return the smallest odd prime number >= SEED.  */
unsigned long
next_prime (unsigned long int seed)
{
  /* Make it definitely odd.  */
  seed |= 1;

  while (!is_prime (seed))
    seed += 2;

  return seed;
}


/* Initialize a hash table.  INIT_SIZE > 1 is the initial number of available
   entries.
   Return 0 always.  */
int
hash_init (hash_table *htab, unsigned long int init_size)
{
  /* We need the size to be a prime.  */
  init_size = next_prime (init_size);

  /* Initialize the data structure.  */
  htab->size = init_size;
  htab->filled = 0;
  htab->first = NULL;
  htab->table = XCALLOC (init_size + 1, hash_entry);

  obstack_init (&htab->mem_pool);

  return 0;
}


/* Delete a hash table's contents.
   Return 0 always.  */
int
hash_destroy (hash_table *htab)
{
  free (htab->table);
  obstack_free (&htab->mem_pool, NULL);
  return 0;
}


/* Compute a hash code for a key consisting of KEYLEN bytes starting at KEY
   in memory.  */
static unsigned long
compute_hashval (const void *key, size_t keylen)
{
  size_t cnt;
  unsigned long int hval;

  /* Compute the hash value for the given string.  The algorithm
     is taken from [Aho,Sethi,Ullman], fixed according to
     https://haible.de/bruno/hashfunc.html.  */
  cnt = 0;
  hval = keylen;
  while (cnt < keylen)
    {
      hval = (hval << 9) | (hval >> (sizeof (unsigned long) * CHAR_BIT - 9));
      hval += (unsigned long int) *(((const char *) key) + cnt++);
    }
  return hval != 0 ? hval : ~((unsigned long) 0);
}


/* References:
   [Aho,Sethi,Ullman] Compilers: Principles, Techniques and Tools, 1986
   [Knuth]            The Art of Computer Programming, part3 (6.4) */

/* Look up a given key in the hash table.
   Return the index of the entry, if present, or otherwise the index a free
   entry where it could be inserted.  */
static size_t
lookup (hash_table *htab,
        const void *key, size_t keylen,
        unsigned long int hval)
{
  unsigned long int hash;
  size_t idx;
  hash_entry *table = htab->table;

  /* First hash function: simply take the modul but prevent zero.  */
  hash = 1 + hval % htab->size;

  idx = hash;

  if (table[idx].used)
    {
      if (table[idx].used == hval && table[idx].keylen == keylen
          && memcmp (table[idx].key, key, keylen) == 0)
        return idx;

      /* Second hash function as suggested in [Knuth].  */
      hash = 1 + hval % (htab->size - 2);

      do
        {
          if (idx <= hash)
            idx = htab->size + idx - hash;
          else
            idx -= hash;

          /* If entry is found use it.  */
          if (table[idx].used == hval && table[idx].keylen == keylen
              && memcmp (table[idx].key, key, keylen) == 0)
            return idx;
        }
      while (table[idx].used);
    }
  return idx;
}


/* Look up the value of a key in the given table.
   If found, return 0 and set *RESULT to it.  Otherwise return -1.  */
int
hash_find_entry (hash_table *htab, const void *key, size_t keylen,
                 void **result)
{
  hash_entry *table = htab->table;
  size_t idx = lookup (htab, key, keylen, compute_hashval (key, keylen));

  if (table[idx].used == 0)
    return -1;

  *result = table[idx].data;
  return 0;
}


/* Insert the pair (KEY[0..KEYLEN-1], DATA) in the hash table at index IDX.
   HVAL is the key's hash code.  IDX depends on it.  The table entry at index
   IDX is known to be unused.  */
static void
insert_entry_2 (hash_table *htab,
                const void *key, size_t keylen,
                unsigned long int hval, size_t idx, void *data)
{
  hash_entry *table = htab->table;

  table[idx].used = hval;
  table[idx].key = key;
  table[idx].keylen = keylen;
  table[idx].data = data;

  /* List the new value in the list.  */
  if (htab->first == NULL)
    {
      table[idx].next = &table[idx];
      htab->first = &table[idx];
    }
  else
    {
      table[idx].next = htab->first->next;
      htab->first->next = &table[idx];
      htab->first = &table[idx];
    }

  ++htab->filled;
}


/* Grow the hash table.  */
static void
resize (hash_table *htab)
{
  unsigned long int old_size = htab->size;
  hash_entry *table = htab->table;
  size_t idx;

  htab->size = next_prime (htab->size * 2);
  htab->filled = 0;
  htab->first = NULL;
  htab->table = XCALLOC (1 + htab->size, hash_entry);

  for (idx = 1; idx <= old_size; ++idx)
    if (table[idx].used)
      insert_entry_2 (htab, table[idx].key, table[idx].keylen,
                      table[idx].used,
                      lookup (htab, table[idx].key, table[idx].keylen,
                              table[idx].used),
                      table[idx].data);

  free (table);
}


/* Try to insert the pair (KEY[0..KEYLEN-1], DATA) in the hash table.
   Return non-NULL (more precisely, the address of the KEY inside the table's
   memory pool) if successful, or NULL if there is already an entry with the
   given key.  */
const void *
hash_insert_entry (hash_table *htab,
                   const void *key, size_t keylen,
                   void *data)
{
  unsigned long int hval = compute_hashval (key, keylen);
  hash_entry *table = htab->table;
  size_t idx = lookup (htab, key, keylen, hval);

  if (table[idx].used)
    /* We don't want to overwrite the old value.  */
    return NULL;
  else
    {
      /* An empty bucket has been found.  */
      void *keycopy = obstack_copy (&htab->mem_pool, key, keylen);
      insert_entry_2 (htab, keycopy, keylen, hval, idx, data);
      if (100 * htab->filled > 75 * htab->size)
        /* Table is filled more than 75%.  Resize the table.  */
        resize (htab);
      return keycopy;
    }
}


/* Insert the pair (KEY[0..KEYLEN-1], DATA) in the hash table.
   Return 0.  */
int
hash_set_value (hash_table *htab,
                const void *key, size_t keylen,
                void *data)
{
  unsigned long int hval = compute_hashval (key, keylen);
  hash_entry *table = htab->table;
  size_t idx = lookup (htab, key, keylen, hval);

  if (table[idx].used)
    {
      /* Overwrite the old value.  */
      table[idx].data = data;
      return 0;
    }
  else
    {
      /* An empty bucket has been found.  */
      void *keycopy = obstack_copy (&htab->mem_pool, key, keylen);
      insert_entry_2 (htab, keycopy, keylen, hval, idx, data);
      if (100 * htab->filled > 75 * htab->size)
        /* Table is filled more than 75%.  Resize the table.  */
        resize (htab);
      return 0;
    }
}


/* Steps *PTR forward to the next used entry in the given hash table.  *PTR
   should be initially set to NULL.  Store information about the next entry
   in *KEY, *KEYLEN, *DATA.
   Return 0 normally, -1 when the whole hash table has been traversed.  */
int
hash_iterate (hash_table *htab, void **ptr, const void **key, size_t *keylen,
              void **data)
{
  hash_entry *curr;

  if (*ptr == NULL)
    {
      if (htab->first == NULL)
        return -1;
      curr = htab->first;
    }
  else
    {
      if (*ptr == htab->first)
        return -1;
      curr = (hash_entry *) *ptr;
    }
  curr = curr->next;
  *ptr = (void *) curr;

  *key = curr->key;
  *keylen = curr->keylen;
  *data = curr->data;
  return 0;
}


/* Steps *PTR forward to the next used entry in the given hash table.  *PTR
   should be initially set to NULL.  Store information about the next entry
   in *KEY, *KEYLEN, *DATAP.  *DATAP is set to point to the storage of the
   value; modifying **DATAP will modify the value of the entry.
   Return 0 normally, -1 when the whole hash table has been traversed.  */
int
hash_iterate_modify (hash_table *htab, void **ptr,
                     const void **key, size_t *keylen,
                     void ***datap)
{
  hash_entry *curr;

  if (*ptr == NULL)
    {
      if (htab->first == NULL)
        return -1;
      curr = htab->first;
    }
  else
    {
      if (*ptr == htab->first)
        return -1;
      curr = (hash_entry *) *ptr;
    }
  curr = curr->next;
  *ptr = (void *) curr;

  *key = curr->key;
  *keylen = curr->keylen;
  *datap = &curr->data;
  return 0;
}
