/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

// java.util.HashMap

#ifndef _DBE_HASHMAP_H
#define _DBE_HASHMAP_H

#include "vec.h"
#include "util.h"
#include "StringBuilder.h"
#include "Histable.h"
#include "MemObject.h"

template <typename Key_t> inline int get_hash_code (Key_t a);
template <typename Key_t> inline bool is_equals (Key_t a, Key_t b);
template <typename Key_t> inline Key_t copy_key (Key_t a);
template <typename Key_t> inline void delete_key (Key_t a);

// Specialization for char*
template<> inline int
get_hash_code (char *a)
{
  return (int) (crc64 (a, strlen (a)) & 0x7fffffff);
}

template<> inline bool
is_equals (char *a, char *b)
{
  return dbe_strcmp (a, b) == 0;
}

template<> inline char *
copy_key (char *a)
{
  return dbe_strdup (a);
}

template<> inline void
delete_key (char *a)
{
  return free (a);
}

template<> inline int
get_hash_code (uint64_t a)
{
  return (int) (a & 0x7fffffff);
}

template<> inline bool
is_equals (uint64_t a, uint64_t b)
{
  return a == b;
}

template<> inline uint64_t
copy_key (uint64_t a)
{
  return a;
}

template<> inline void
delete_key (uint64_t a)
{
  a = a;
}

template<> inline int
get_hash_code (Histable* a)
{
  return (int) (a->id & 0x7fffffff);
}

template<> inline bool
is_equals (Histable* a, Histable* b)
{
  return a == b;
}

template<> inline Histable*
copy_key (Histable* a)
{
  return a;
}

template<> inline void
delete_key (Histable* a)
{
  a->id = a->id;
}

template<> inline int
get_hash_code (MemObj* a)
{
  return (int) (a->id & 0x7fffffff);
}

template<> inline bool
is_equals (MemObj* a, MemObj* b)
{
  return a == b;
}

template<> inline MemObj*
copy_key (MemObj* a)
{
  return a;
}

template<> inline void
delete_key (MemObj* a)
{
  a->id = a->id;
}

template <typename Key_t, typename Value_t>
class HashMap
{
public:
  HashMap (int initialCapacity = 0);

  ~HashMap ()
  {
    clear ();
    delete vals;
    delete[] hashTable;
  }

  Value_t put (Key_t key, Value_t val);
  Value_t get (Key_t key);
  Value_t get (Key_t key, Value_t val); // Create a new map if key is not here
  void clear ();
  Value_t remove (Key_t);
  Vector<Value_t> *values (Key_t key);  // Return a list of values for 'key'

  bool
  containsKey (Key_t key)
  {
    Value_t p = get (key);
    return p != NULL;
  };

  Vector<Value_t> *
  values ()
  {
    return vals;
  }

  void
  reset ()
  {
    clear ();
  }

  int
  get_phaseIdx ()
  {
    return phaseIdx;
  }

  void
  set_phaseIdx (int phase)
  {
    if (phase == 0) clear ();
    phaseIdx = phase;
  }
  char *dump ();

private:

  enum
  {
    HASH_SIZE       = 511,
    MAX_HASH_SIZE   = 1048575
  };

  typedef struct Hash
  {
    Key_t key;
    Value_t val;
    struct Hash *next;
  } Hash_t;

  void resize ();

  int
  hashCode (Key_t key)
  {
    return get_hash_code (key) % hash_sz;
  }

  bool
  equals (Key_t a, Key_t b)
  {
    return is_equals (a, b);
  }

  Key_t
  copy (Key_t key)
  {
    return copy_key (key);
  }

  Hash_t **hashTable;
  Vector<Value_t> *vals;
  int phaseIdx;
  int hash_sz;
  int nelem;
};

template <typename Key_t, typename Value_t>
HashMap<Key_t, Value_t>
::HashMap (int initialCapacity)
{
  if (initialCapacity > 0)
    vals = new Vector<Value_t>(initialCapacity);
  else
    vals = new Vector<Value_t>();
  phaseIdx = 0;
  nelem = 0;
  hash_sz = HASH_SIZE;
  hashTable = new Hash_t*[hash_sz];
  for (int i = 0; i < hash_sz; i++)
    hashTable[i] = NULL;
}

template <typename Key_t, typename Value_t>
void
HashMap<Key_t, Value_t>::clear ()
{
  vals->reset ();
  phaseIdx = 0;
  nelem = 0;
  for (int i = 0; i < hash_sz; i++)
    {
      Hash_t *next;
      for (Hash_t *p = hashTable[i]; p; p = next)
	{
	  next = p->next;
	  delete_key (p->key);
	  delete p;
	}
      hashTable[i] = NULL;
    }
}

template <typename Key_t, typename Value_t>
void
HashMap<Key_t, Value_t>::resize ()
{
  int old_hash_sz = hash_sz;
  hash_sz = old_hash_sz * 2 + 1;
  Hash_t **old_hash_table = hashTable;
  hashTable = new Hash_t*[hash_sz];
  for (int i = 0; i < hash_sz; i++)
    hashTable[i] = NULL;
  nelem = 0;
  for (int i = 0; i < old_hash_sz; i++)
    {
      if (old_hash_table[i] != NULL)
	{
	  Hash_t *old_p;
	  Hash_t *p = old_hash_table[i];
	  while (p != NULL)
	    {
	      put (p->key, p->val);
	      old_p = p;
	      p = p->next;
	      delete old_p;
	    }
	}
    }
  delete[] old_hash_table;
}

template <typename Key_t, typename Value_t>
Value_t
HashMap<Key_t, Value_t>::get (Key_t key)
{
  int hash_code = hashCode (key);
  for (Hash_t *p = hashTable[hash_code]; p; p = p->next)
    if (equals (key, p->key))
      return p->val;
  return NULL;
}

template <typename Key_t, typename Value_t>
Vector<Value_t> *
HashMap<Key_t, Value_t>::values (Key_t key)
{
  Vector<Value_t> *list = new Vector<Value_t>();
  int hash_code = hashCode (key);
  for (Hash_t *p = hashTable[hash_code]; p; p = p->next)
    {
      if (equals (key, p->key))
	list->append (p->val);
    }
  return list;
}

template <typename Key_t, typename Value_t>
Value_t
HashMap<Key_t, Value_t>::get (const Key_t key, Value_t val)
{
  int hash_code = hashCode (key);
  Hash_t *p, *first = NULL;
  for (p = hashTable[hash_code]; p; p = p->next)
    {
      if (equals (key, p->key))
	{
	  if (first == NULL)
	    first = p;
	  if (val == p->val)
	    return first->val; // Always return the first value
	}
    }
  vals->append (val);
  p = new Hash_t ();
  p->val = val;
  p->key = copy (key);
  if (first)
    {
      p->next = first->next;
      first->next = p;
      return first->val; // Always return the first value
    }
  else
    {
      p->next = hashTable[hash_code];
      hashTable[hash_code] = p;
      return val;
    }
}

template <typename Key_t, typename Value_t>
Value_t
HashMap<Key_t, Value_t>::remove (Key_t key)
{
  int hash_code = hashCode (key);
  Value_t val = NULL;
  for (Hash_t *prev = NULL, *p = hashTable[hash_code]; p != NULL;)
    {
      if (equals (key, p->key))
	{
	  if (prev == NULL)
	    hashTable[hash_code] = p->next;
	  else
	    prev->next = p->next;
	  if (val == NULL)
	    val = p->val;
	  delete_key (p->key);
	  delete p;
	}
      else
	{
	  prev = p;
	  p = p->next;
	}
    }
  return val;
}

template <typename Key_t, typename Value_t>
Value_t
HashMap<Key_t, Value_t>::put (Key_t key, Value_t val)
{
  int hash_code = hashCode (key);
  vals->append (val);
  for (Hash_t *p = hashTable[hash_code]; p != NULL; p = p->next)
    {
      if (equals (key, p->key))
	{
	  Value_t v = p->val;
	  p->val = val;
	  return v;
	}
    }
  Hash_t *p = new Hash_t ();
  p->val = val;
  p->key = copy (key);
  p->next = hashTable[hash_code];
  hashTable[hash_code] = p;
  nelem++;
  if (nelem == hash_sz)
    resize ();
  return val;
}

template <typename Key_t, typename Value_t>
char *
HashMap<Key_t, Value_t>::dump ()
{
  StringBuilder sb;
  char buf[128];
  snprintf (buf, sizeof (buf), "HashMap: size=%d ##########\n", vals->size ());
  sb.append (buf);
  for (int i = 0; i < hash_sz; i++)
    {
      if (hashTable[i] == NULL)
	continue;
      snprintf (buf, sizeof (buf), "%3d:", i);
      sb.append (buf);
      char *s = NTXT (" ");
      for (Hash_t *p = hashTable[i]; p; p = p->next)
	{
	  sb.append (s);
	  s = NTXT ("     ");
	  sb.append (p->key);
	  snprintf (buf, sizeof (buf), " --> 0x%p '%s'\n",
		    p->val, p->val->get_name ());
	  sb.append (buf);
	}
    }
  return sb.toString ();
}

#endif // _DBE_HASHMAP_H
