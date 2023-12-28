/* Interface to hashtable implementations.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#include <ctf-impl.h>
#include <string.h>
#include "libiberty.h"
#include "hashtab.h"

/* We have three hashtable implementations:

   - ctf_hash_* is an interface to a fixed-size hash from const char * ->
     ctf_id_t with number of elements specified at creation time, that should
     support addition of items but need not support removal.

   - ctf_dynhash_* is an interface to a dynamically-expanding hash with
     unknown size that should support addition of large numbers of items, and
     removal as well, and is used only at type-insertion time and during
     linking.

   - ctf_dynset_* is an interface to a dynamically-expanding hash that contains
     only keys: no values.

   These can be implemented by the same underlying hashmap if you wish.  */

/* The helem is used for general key/value mappings in both the ctf_hash and
   ctf_dynhash: the owner may not have space allocated for it, and will be
   garbage (not NULL!) in that case.  */

typedef struct ctf_helem
{
  void *key;			 /* Either a pointer, or a coerced ctf_id_t.  */
  void *value;			 /* The value (possibly a coerced int).  */
  ctf_dynhash_t *owner;          /* The hash that owns us.  */
} ctf_helem_t;

/* Equally, the key_free and value_free may not exist.  */

struct ctf_dynhash
{
  struct htab *htab;
  ctf_hash_free_fun key_free;
  ctf_hash_free_fun value_free;
};

/* Hash and eq functions for the dynhash and hash. */

unsigned int
ctf_hash_integer (const void *ptr)
{
  ctf_helem_t *hep = (ctf_helem_t *) ptr;

  return htab_hash_pointer (hep->key);
}

int
ctf_hash_eq_integer (const void *a, const void *b)
{
  ctf_helem_t *hep_a = (ctf_helem_t *) a;
  ctf_helem_t *hep_b = (ctf_helem_t *) b;

  return htab_eq_pointer (hep_a->key, hep_b->key);
}

unsigned int
ctf_hash_string (const void *ptr)
{
  ctf_helem_t *hep = (ctf_helem_t *) ptr;

  return htab_hash_string (hep->key);
}

int
ctf_hash_eq_string (const void *a, const void *b)
{
  ctf_helem_t *hep_a = (ctf_helem_t *) a;
  ctf_helem_t *hep_b = (ctf_helem_t *) b;

  return !strcmp((const char *) hep_a->key, (const char *) hep_b->key);
}

/* Hash a type_key.  */
unsigned int
ctf_hash_type_key (const void *ptr)
{
  ctf_helem_t *hep = (ctf_helem_t *) ptr;
  ctf_link_type_key_t *k = (ctf_link_type_key_t *) hep->key;

  return htab_hash_pointer (k->cltk_fp) + 59
    * htab_hash_pointer ((void *) (uintptr_t) k->cltk_idx);
}

int
ctf_hash_eq_type_key (const void *a, const void *b)
{
  ctf_helem_t *hep_a = (ctf_helem_t *) a;
  ctf_helem_t *hep_b = (ctf_helem_t *) b;
  ctf_link_type_key_t *key_a = (ctf_link_type_key_t *) hep_a->key;
  ctf_link_type_key_t *key_b = (ctf_link_type_key_t *) hep_b->key;

  return (key_a->cltk_fp == key_b->cltk_fp)
    && (key_a->cltk_idx == key_b->cltk_idx);
}

/* Hash a type_id_key.  */
unsigned int
ctf_hash_type_id_key (const void *ptr)
{
  ctf_helem_t *hep = (ctf_helem_t *) ptr;
  ctf_type_id_key_t *k = (ctf_type_id_key_t *) hep->key;

  return htab_hash_pointer ((void *) (uintptr_t) k->ctii_input_num)
    + 59 * htab_hash_pointer ((void *) (uintptr_t) k->ctii_type);
}

int
ctf_hash_eq_type_id_key (const void *a, const void *b)
{
  ctf_helem_t *hep_a = (ctf_helem_t *) a;
  ctf_helem_t *hep_b = (ctf_helem_t *) b;
  ctf_type_id_key_t *key_a = (ctf_type_id_key_t *) hep_a->key;
  ctf_type_id_key_t *key_b = (ctf_type_id_key_t *) hep_b->key;

  return (key_a->ctii_input_num == key_b->ctii_input_num)
    && (key_a->ctii_type == key_b->ctii_type);
}

/* The dynhash, used for hashes whose size is not known at creation time. */

/* Free a single ctf_helem with arbitrary key/value functions.  */

static void
ctf_dynhash_item_free (void *item)
{
  ctf_helem_t *helem = item;

  if (helem->owner->key_free && helem->key)
    helem->owner->key_free (helem->key);
  if (helem->owner->value_free && helem->value)
    helem->owner->value_free (helem->value);
  free (helem);
}

ctf_dynhash_t *
ctf_dynhash_create (ctf_hash_fun hash_fun, ctf_hash_eq_fun eq_fun,
                    ctf_hash_free_fun key_free, ctf_hash_free_fun value_free)
{
  ctf_dynhash_t *dynhash;
  htab_del del = ctf_dynhash_item_free;

  if (key_free || value_free)
    dynhash = malloc (sizeof (ctf_dynhash_t));
  else
    dynhash = malloc (offsetof (ctf_dynhash_t, key_free));
  if (!dynhash)
    return NULL;

  if (key_free == NULL && value_free == NULL)
    del = free;

  /* 7 is arbitrary and untested for now.  */
  if ((dynhash->htab = htab_create_alloc (7, (htab_hash) hash_fun, eq_fun,
					  del, xcalloc, free)) == NULL)
    {
      free (dynhash);
      return NULL;
    }

  if (key_free || value_free)
    {
      dynhash->key_free = key_free;
      dynhash->value_free = value_free;
    }

  return dynhash;
}

static ctf_helem_t **
ctf_hashtab_lookup (struct htab *htab, const void *key, enum insert_option insert)
{
  ctf_helem_t tmp = { .key = (void *) key };
  return (ctf_helem_t **) htab_find_slot (htab, &tmp, insert);
}

static ctf_helem_t *
ctf_hashtab_insert (struct htab *htab, void *key, void *value,
		    ctf_hash_free_fun key_free,
		    ctf_hash_free_fun value_free)
{
  ctf_helem_t **slot;

  slot = ctf_hashtab_lookup (htab, key, INSERT);

  if (!slot)
    {
      errno = ENOMEM;
      return NULL;
    }

  if (!*slot)
    {
      /* Only spend space on the owner if we're going to use it: if there is a
	 key or value freeing function.  */
      if (key_free || value_free)
	*slot = malloc (sizeof (ctf_helem_t));
      else
	*slot = malloc (offsetof (ctf_helem_t, owner));
      if (!*slot)
	return NULL;
      (*slot)->key = key;
    }
  else
    {
      if (key_free)
	  key_free (key);
      if (value_free)
	  value_free ((*slot)->value);
    }
  (*slot)->value = value;
  return *slot;
}

int
ctf_dynhash_insert (ctf_dynhash_t *hp, void *key, void *value)
{
  ctf_helem_t *slot;
  ctf_hash_free_fun key_free = NULL, value_free = NULL;

  if (hp->htab->del_f == ctf_dynhash_item_free)
    {
      key_free = hp->key_free;
      value_free = hp->value_free;
    }
  slot = ctf_hashtab_insert (hp->htab, key, value,
			     key_free, value_free);

  if (!slot)
    return errno;

  /* Keep track of the owner, so that the del function can get at the key_free
     and value_free functions.  Only do this if one of those functions is set:
     if not, the owner is not even present in the helem.  */

  if (key_free || value_free)
    slot->owner = hp;

  return 0;
}

void
ctf_dynhash_remove (ctf_dynhash_t *hp, const void *key)
{
  ctf_helem_t hep = { (void *) key, NULL, NULL };
  htab_remove_elt (hp->htab, &hep);
}

void
ctf_dynhash_empty (ctf_dynhash_t *hp)
{
  htab_empty (hp->htab);
}

size_t
ctf_dynhash_elements (ctf_dynhash_t *hp)
{
  return htab_elements (hp->htab);
}

void *
ctf_dynhash_lookup (ctf_dynhash_t *hp, const void *key)
{
  ctf_helem_t **slot;

  slot = ctf_hashtab_lookup (hp->htab, key, NO_INSERT);

  if (slot)
    return (*slot)->value;

  return NULL;
}

/* TRUE/FALSE return.  */
int
ctf_dynhash_lookup_kv (ctf_dynhash_t *hp, const void *key,
		       const void **orig_key, void **value)
{
  ctf_helem_t **slot;

  slot = ctf_hashtab_lookup (hp->htab, key, NO_INSERT);

  if (slot)
    {
      if (orig_key)
	*orig_key = (*slot)->key;
      if (value)
	*value = (*slot)->value;
      return 1;
    }
  return 0;
}

typedef struct ctf_traverse_cb_arg
{
  ctf_hash_iter_f fun;
  void *arg;
} ctf_traverse_cb_arg_t;

static int
ctf_hashtab_traverse (void **slot, void *arg_)
{
  ctf_helem_t *helem = *((ctf_helem_t **) slot);
  ctf_traverse_cb_arg_t *arg = (ctf_traverse_cb_arg_t *) arg_;

  arg->fun (helem->key, helem->value, arg->arg);
  return 1;
}

void
ctf_dynhash_iter (ctf_dynhash_t *hp, ctf_hash_iter_f fun, void *arg_)
{
  ctf_traverse_cb_arg_t arg = { fun, arg_ };
  htab_traverse (hp->htab, ctf_hashtab_traverse, &arg);
}

typedef struct ctf_traverse_find_cb_arg
{
  ctf_hash_iter_find_f fun;
  void *arg;
  void *found_key;
} ctf_traverse_find_cb_arg_t;

static int
ctf_hashtab_traverse_find (void **slot, void *arg_)
{
  ctf_helem_t *helem = *((ctf_helem_t **) slot);
  ctf_traverse_find_cb_arg_t *arg = (ctf_traverse_find_cb_arg_t *) arg_;

  if (arg->fun (helem->key, helem->value, arg->arg))
    {
      arg->found_key = helem->key;
      return 0;
    }
  return 1;
}

void *
ctf_dynhash_iter_find (ctf_dynhash_t *hp, ctf_hash_iter_find_f fun, void *arg_)
{
  ctf_traverse_find_cb_arg_t arg = { fun, arg_, NULL };
  htab_traverse (hp->htab, ctf_hashtab_traverse_find, &arg);
  return arg.found_key;
}

typedef struct ctf_traverse_remove_cb_arg
{
  struct htab *htab;
  ctf_hash_iter_remove_f fun;
  void *arg;
} ctf_traverse_remove_cb_arg_t;

static int
ctf_hashtab_traverse_remove (void **slot, void *arg_)
{
  ctf_helem_t *helem = *((ctf_helem_t **) slot);
  ctf_traverse_remove_cb_arg_t *arg = (ctf_traverse_remove_cb_arg_t *) arg_;

  if (arg->fun (helem->key, helem->value, arg->arg))
    htab_clear_slot (arg->htab, slot);
  return 1;
}

void
ctf_dynhash_iter_remove (ctf_dynhash_t *hp, ctf_hash_iter_remove_f fun,
                         void *arg_)
{
  ctf_traverse_remove_cb_arg_t arg = { hp->htab, fun, arg_ };
  htab_traverse (hp->htab, ctf_hashtab_traverse_remove, &arg);
}

/* Traverse a dynhash in arbitrary order, in _next iterator form.

   Mutating the dynhash while iterating is not supported (just as it isn't for
   htab_traverse).

   Note: unusually, this returns zero on success and a *positive* value on
   error, because it does not take an fp, taking an error pointer would be
   incredibly clunky, and nearly all error-handling ends up stuffing the result
   of this into some sort of errno or ctf_errno, which is invariably
   positive.  So doing this simplifies essentially all callers.  */
int
ctf_dynhash_next (ctf_dynhash_t *h, ctf_next_t **it, void **key, void **value)
{
  ctf_next_t *i = *it;
  ctf_helem_t *slot;

  if (!i)
    {
      size_t size = htab_size (h->htab);

      /* If the table has too many entries to fit in an ssize_t, just give up.
	 This might be spurious, but if any type-related hashtable has ever been
	 nearly as large as that then something very odd is going on.  */
      if (((ssize_t) size) < 0)
	return EDOM;

      if ((i = ctf_next_create ()) == NULL)
	return ENOMEM;

      i->u.ctn_hash_slot = h->htab->entries;
      i->cu.ctn_h = h;
      i->ctn_n = 0;
      i->ctn_size = (ssize_t) size;
      i->ctn_iter_fun = (void (*) (void)) ctf_dynhash_next;
      *it = i;
    }

  if ((void (*) (void)) ctf_dynhash_next != i->ctn_iter_fun)
    return ECTF_NEXT_WRONGFUN;

  if (h != i->cu.ctn_h)
    return ECTF_NEXT_WRONGFP;

  if ((ssize_t) i->ctn_n == i->ctn_size)
    goto hash_end;

  while ((ssize_t) i->ctn_n < i->ctn_size
	 && (*i->u.ctn_hash_slot == HTAB_EMPTY_ENTRY
	     || *i->u.ctn_hash_slot == HTAB_DELETED_ENTRY))
    {
      i->u.ctn_hash_slot++;
      i->ctn_n++;
    }

  if ((ssize_t) i->ctn_n == i->ctn_size)
    goto hash_end;

  slot = *i->u.ctn_hash_slot;

  if (key)
    *key = slot->key;
  if (value)
    *value = slot->value;

  i->u.ctn_hash_slot++;
  i->ctn_n++;

  return 0;

 hash_end:
  ctf_next_destroy (i);
  *it = NULL;
  return ECTF_NEXT_END;
}

int
ctf_dynhash_sort_by_name (const ctf_next_hkv_t *one, const ctf_next_hkv_t *two,
			  void *unused _libctf_unused_)
{
  return strcmp ((char *) one->hkv_key, (char *) two->hkv_key);
}

/* Traverse a sorted dynhash, in _next iterator form.

   See ctf_dynhash_next for notes on error returns, etc.

   Sort keys before iterating over them using the SORT_FUN and SORT_ARG.

   If SORT_FUN is null, thunks to ctf_dynhash_next.  */
int
ctf_dynhash_next_sorted (ctf_dynhash_t *h, ctf_next_t **it, void **key,
			 void **value, ctf_hash_sort_f sort_fun, void *sort_arg)
{
  ctf_next_t *i = *it;

  if (sort_fun == NULL)
    return ctf_dynhash_next (h, it, key, value);

  if (!i)
    {
      size_t els = ctf_dynhash_elements (h);
      ctf_next_t *accum_i = NULL;
      void *key, *value;
      int err;
      ctf_next_hkv_t *walk;

      if (((ssize_t) els) < 0)
	return EDOM;

      if ((i = ctf_next_create ()) == NULL)
	return ENOMEM;

      if ((i->u.ctn_sorted_hkv = calloc (els, sizeof (ctf_next_hkv_t))) == NULL)
	{
	  ctf_next_destroy (i);
	  return ENOMEM;
	}
      walk = i->u.ctn_sorted_hkv;

      i->cu.ctn_h = h;

      while ((err = ctf_dynhash_next (h, &accum_i, &key, &value)) == 0)
	{
	  walk->hkv_key = key;
	  walk->hkv_value = value;
	  walk++;
	}
      if (err != ECTF_NEXT_END)
	{
	  ctf_next_destroy (i);
	  return err;
	}

      if (sort_fun)
	  ctf_qsort_r (i->u.ctn_sorted_hkv, els, sizeof (ctf_next_hkv_t),
		       (int (*) (const void *, const void *, void *)) sort_fun,
		       sort_arg);
      i->ctn_n = 0;
      i->ctn_size = (ssize_t) els;
      i->ctn_iter_fun = (void (*) (void)) ctf_dynhash_next_sorted;
      *it = i;
    }

  if ((void (*) (void)) ctf_dynhash_next_sorted != i->ctn_iter_fun)
    return ECTF_NEXT_WRONGFUN;

  if (h != i->cu.ctn_h)
    return ECTF_NEXT_WRONGFP;

  if ((ssize_t) i->ctn_n == i->ctn_size)
    {
      ctf_next_destroy (i);
      *it = NULL;
      return ECTF_NEXT_END;
    }

  if (key)
    *key = i->u.ctn_sorted_hkv[i->ctn_n].hkv_key;
  if (value)
    *value = i->u.ctn_sorted_hkv[i->ctn_n].hkv_value;
  i->ctn_n++;
  return 0;
}

void
ctf_dynhash_destroy (ctf_dynhash_t *hp)
{
  if (hp != NULL)
    htab_delete (hp->htab);
  free (hp);
}

/* The dynset, used for sets of keys with no value.  The implementation of this
   can be much simpler, because without a value the slot can simply be the
   stored key, which means we don't need to store the freeing functions and the
   dynset itself is just a htab.  */

ctf_dynset_t *
ctf_dynset_create (htab_hash hash_fun, htab_eq eq_fun,
		   ctf_hash_free_fun key_free)
{
  /* 7 is arbitrary and untested for now.  */
  return (ctf_dynset_t *) htab_create_alloc (7, (htab_hash) hash_fun, eq_fun,
					     key_free, xcalloc, free);
}

/* The dynset has one complexity: the underlying implementation reserves two
   values for internal hash table implementation details (empty versus deleted
   entries).  These values are otherwise very useful for pointers cast to ints,
   so transform the ctf_dynset_inserted value to allow for it.  (This
   introduces an ambiguity in that one can no longer store these two values in
   the dynset, but if we pick high enough values this is very unlikely to be a
   problem.)

   We leak this implementation detail to the freeing functions on the grounds
   that any use of these functions is overwhelmingly likely to be in sets using
   real pointers, which will be unaffected.  */

#define DYNSET_EMPTY_ENTRY_REPLACEMENT ((void *) (uintptr_t) -64)
#define DYNSET_DELETED_ENTRY_REPLACEMENT ((void *) (uintptr_t) -63)

static void *
key_to_internal (const void *key)
{
  if (key == HTAB_EMPTY_ENTRY)
    return DYNSET_EMPTY_ENTRY_REPLACEMENT;
  else if (key == HTAB_DELETED_ENTRY)
    return DYNSET_DELETED_ENTRY_REPLACEMENT;

  return (void *) key;
}

static void *
internal_to_key (const void *internal)
{
  if (internal == DYNSET_EMPTY_ENTRY_REPLACEMENT)
    return HTAB_EMPTY_ENTRY;
  else if (internal == DYNSET_DELETED_ENTRY_REPLACEMENT)
    return HTAB_DELETED_ENTRY;
  return (void *) internal;
}

int
ctf_dynset_insert (ctf_dynset_t *hp, void *key)
{
  struct htab *htab = (struct htab *) hp;
  void **slot;

  slot = htab_find_slot (htab, key, INSERT);

  if (!slot)
    {
      errno = ENOMEM;
      return -errno;
    }

  if (*slot)
    {
      if (htab->del_f)
	(*htab->del_f) (*slot);
    }

  *slot = key_to_internal (key);

  return 0;
}

void
ctf_dynset_remove (ctf_dynset_t *hp, const void *key)
{
  htab_remove_elt ((struct htab *) hp, key_to_internal (key));
}

void
ctf_dynset_destroy (ctf_dynset_t *hp)
{
  if (hp != NULL)
    htab_delete ((struct htab *) hp);
}

void *
ctf_dynset_lookup (ctf_dynset_t *hp, const void *key)
{
  void **slot = htab_find_slot ((struct htab *) hp,
				key_to_internal (key), NO_INSERT);

  if (slot)
    return internal_to_key (*slot);
  return NULL;
}

size_t
ctf_dynset_elements (ctf_dynset_t *hp)
{
  return htab_elements ((struct htab *) hp);
}

/* TRUE/FALSE return.  */
int
ctf_dynset_exists (ctf_dynset_t *hp, const void *key, const void **orig_key)
{
  void **slot = htab_find_slot ((struct htab *) hp,
				key_to_internal (key), NO_INSERT);

  if (orig_key && slot)
    *orig_key = internal_to_key (*slot);
  return (slot != NULL);
}

/* Look up a completely random value from the set, if any exist.
   Keys with value zero cannot be distinguished from a nonexistent key.  */
void *
ctf_dynset_lookup_any (ctf_dynset_t *hp)
{
  struct htab *htab = (struct htab *) hp;
  void **slot = htab->entries;
  void **limit = slot + htab_size (htab);

  while (slot < limit
	 && (*slot == HTAB_EMPTY_ENTRY || *slot == HTAB_DELETED_ENTRY))
      slot++;

  if (slot < limit)
    return internal_to_key (*slot);
  return NULL;
}

/* Traverse a dynset in arbitrary order, in _next iterator form.

   Otherwise, just like ctf_dynhash_next.  */
int
ctf_dynset_next (ctf_dynset_t *hp, ctf_next_t **it, void **key)
{
  struct htab *htab = (struct htab *) hp;
  ctf_next_t *i = *it;
  void *slot;

  if (!i)
    {
      size_t size = htab_size (htab);

      /* If the table has too many entries to fit in an ssize_t, just give up.
	 This might be spurious, but if any type-related hashtable has ever been
	 nearly as large as that then somthing very odd is going on.  */

      if (((ssize_t) size) < 0)
	return EDOM;

      if ((i = ctf_next_create ()) == NULL)
	return ENOMEM;

      i->u.ctn_hash_slot = htab->entries;
      i->cu.ctn_s = hp;
      i->ctn_n = 0;
      i->ctn_size = (ssize_t) size;
      i->ctn_iter_fun = (void (*) (void)) ctf_dynset_next;
      *it = i;
    }

  if ((void (*) (void)) ctf_dynset_next != i->ctn_iter_fun)
    return ECTF_NEXT_WRONGFUN;

  if (hp != i->cu.ctn_s)
    return ECTF_NEXT_WRONGFP;

  if ((ssize_t) i->ctn_n == i->ctn_size)
    goto set_end;

  while ((ssize_t) i->ctn_n < i->ctn_size
	 && (*i->u.ctn_hash_slot == HTAB_EMPTY_ENTRY
	     || *i->u.ctn_hash_slot == HTAB_DELETED_ENTRY))
    {
      i->u.ctn_hash_slot++;
      i->ctn_n++;
    }

  if ((ssize_t) i->ctn_n == i->ctn_size)
    goto set_end;

  slot = *i->u.ctn_hash_slot;

  if (key)
    *key = internal_to_key (slot);

  i->u.ctn_hash_slot++;
  i->ctn_n++;

  return 0;

 set_end:
  ctf_next_destroy (i);
  *it = NULL;
  return ECTF_NEXT_END;
}

/* ctf_hash, used for fixed-size maps from const char * -> ctf_id_t without
   removal.  This is a straight cast of a hashtab.  */

ctf_hash_t *
ctf_hash_create (unsigned long nelems, ctf_hash_fun hash_fun,
		 ctf_hash_eq_fun eq_fun)
{
  return (ctf_hash_t *) htab_create_alloc (nelems, (htab_hash) hash_fun,
					   eq_fun, free, xcalloc, free);
}

uint32_t
ctf_hash_size (const ctf_hash_t *hp)
{
  return htab_elements ((struct htab *) hp);
}

int
ctf_hash_insert_type (ctf_hash_t *hp, ctf_dict_t *fp, uint32_t type,
		      uint32_t name)
{
  const char *str = ctf_strraw (fp, name);

  if (type == 0)
    return EINVAL;

  if (str == NULL
      && CTF_NAME_STID (name) == CTF_STRTAB_1
      && fp->ctf_syn_ext_strtab == NULL
      && fp->ctf_str[CTF_NAME_STID (name)].cts_strs == NULL)
    return ECTF_STRTAB;

  if (str == NULL)
    return ECTF_BADNAME;

  if (str[0] == '\0')
    return 0;		   /* Just ignore empty strings on behalf of caller.  */

  if (ctf_hashtab_insert ((struct htab *) hp, (char *) str,
			  (void *) (ptrdiff_t) type, NULL, NULL) != NULL)
    return 0;
  return errno;
}

/* if the key is already in the hash, override the previous definition with
   this new official definition. If the key is not present, then call
   ctf_hash_insert_type and hash it in.  */
int
ctf_hash_define_type (ctf_hash_t *hp, ctf_dict_t *fp, uint32_t type,
                      uint32_t name)
{
  /* This matches the semantics of ctf_hash_insert_type in this
     implementation anyway.  */

  return ctf_hash_insert_type (hp, fp, type, name);
}

ctf_id_t
ctf_hash_lookup_type (ctf_hash_t *hp, ctf_dict_t *fp __attribute__ ((__unused__)),
		      const char *key)
{
  ctf_helem_t **slot;

  slot = ctf_hashtab_lookup ((struct htab *) hp, key, NO_INSERT);

  if (slot)
    return (ctf_id_t) (uintptr_t) ((*slot)->value);

  return 0;
}

void
ctf_hash_destroy (ctf_hash_t *hp)
{
  if (hp != NULL)
    htab_delete ((struct htab *) hp);
}
