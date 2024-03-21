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

#ifndef _PERFAN_VEC_H
#define _PERFAN_VEC_H

#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

// This package implements a vector of items.

#define Destroy(x)      if (x) { (x)->destroy(); delete (x); (x) = NULL; }
#define VecSize(x)      ((x) ? (x)->size() : 0)

void destroy (void *vec); // Free up the "two-dimension" Vectors

typedef int (*CompareFunc)(const void*, const void*);
typedef int (*ExtCompareFunc)(const void*, const void*, const void*);
typedef int (*SearchFunc)(char*, char*);

extern "C"
{
  typedef int (*StdCompareFunc)(const void*, const void*);
}

enum Search_type
{
  LINEAR,
  BINARY,
  HASH
};

enum Direction
{
  FORWARD,
  REVERSE
};

enum VecType
{
  VEC_VOID = 0,
  VEC_INTEGER,
  VEC_CHAR,
  VEC_BOOL,
  VEC_DOUBLE,
  VEC_LLONG,
  VEC_VOIDARR,
  VEC_STRING,
  VEC_INTARR,
  VEC_BOOLARR,
  VEC_LLONGARR,
  VEC_STRINGARR,
  VEC_DOUBLEARR
};

template <class ITEM> void
qsort (ITEM *, size_t, ExtCompareFunc, void *);

template <typename ITEM> class Vector
{
public:

  Vector ()
  {
    count = 0;
    data = NULL;
    limit = 0;
    sorted = false;
  };

  Vector (long sz);

  virtual
  ~Vector ()
  {
    free (data);
  }

  void append (const ITEM item);
  void addAll (Vector<ITEM> *vec);
  Vector<ITEM> *copy ();        // Return a copy of "this".

  ITEM
  fetch (long index)
  {
    return data[index];
  }

  ITEM
  get (long index)
  {
    return data[index];
  }

  // Return the first index in "this" that equals "item".
  // Return -1 if "item" is not found.
  long find (const ITEM item);
  long find_r (const ITEM item);

  // Insert "item" into "index"'th slot of "this",
  // moving everything over by 1.
  void insert (long index, const ITEM item);

  // Insert "item" after locating its appropriate index
  void incorporate (const ITEM item, CompareFunc func);

  // Remove and return the "index"'th item from "this",
  // moving everything over by 1.
  ITEM remove (long index);

  // Swap two items in "this",
  void swap (long index1, long index2);

  long
  size ()
  {
    return count;
  }

  // Store "item" into the "index"'th slot of "this".
  void store (long index, const ITEM item);

  void
  put (long index, const ITEM item)
  {
    store (index, item);
  }

  // Sort the vector according to compare
  void
  sort (CompareFunc compare, void *arg = NULL)
  {
    qsort (data, count, (ExtCompareFunc) compare, arg);
    sorted = true;
  }

  // Binary search, vector must be sorted
  long bisearch (long start, long end, void *key, CompareFunc func);
  void destroy ();      // delete all vector elements (must be pointers!)

  void
  reset ()
  {
    count = 0;
    sorted = false;
  }

  bool
  is_sorted ()
  {
    return sorted;
  }

  virtual VecType
  type ()
  {
    return VEC_VOID;
  }

  virtual void
  dump (const char * /* msg */)
  {
    return;
  }

private:

  void resize (long index);

  ITEM *data;   // Pointer to data vector
  long count;   // Number of items
  long limit;   // Vector length (power of 2)
  bool sorted;
};

template<> VecType Vector<int>::type ();
template<> VecType Vector<unsigned>::type ();
template<> VecType Vector<char>::type ();
template<> VecType Vector<bool>::type ();
template<> VecType Vector<double>::type ();
template<> VecType Vector<long long>::type ();
template<> VecType Vector<uint64_t>::type ();
template<> VecType Vector<void*>::type ();
template<> VecType Vector<char*>::type ();
template<> VecType Vector<Vector<int>*>::type ();
template<> VecType Vector<Vector<char*>*>::type ();
template<> VecType Vector<Vector<long long>*>::type ();
template<> void Vector<char *>::destroy ();

#define KILOCHUNK   1024
#define MEGACHUNK   1024*1024
#define GIGACHUNK   1024*1024*1024

// A standard looping construct:
#define Vec_loop(ITEM, vec, index, item) \
if (vec != NULL) \
    for (index = 0, item = ((vec)->size() > 0) ? (vec)->fetch(0) : (ITEM)0; \
	 index < (vec)->size(); \
	 item = (++index < (vec)->size()) ? (vec)->fetch(index) : (ITEM)0)

template <typename ITEM>
Vector<ITEM>::Vector (long sz)
{
  count = 0;
  limit = sz > 0 ? sz : KILOCHUNK; // was 0;
  data = limit ? (ITEM *) malloc (sizeof (ITEM) * limit) : NULL;
  sorted = false;
}

template <typename ITEM> void
Vector<ITEM>
::resize (long index)
{
  if (index < limit)
    return;
  if (limit < 16)
    limit = 16;
  while (index >= limit)
    {
      if (limit > GIGACHUNK)
	limit += GIGACHUNK; // Deoptimization for large experiments
      else
	limit = limit * 2;
    }
  data = (ITEM *) realloc (data, limit * sizeof (ITEM));
}

template <typename ITEM> void
Vector<ITEM>::append (const ITEM item)
{
  // This routine will append "item" to the end of "this".
  if (count >= limit)
    resize (count);
  data[count++] = item;
}

template <typename ITEM> void
Vector<ITEM>::addAll (Vector<ITEM> *vec)
{
  if (vec)
    for (int i = 0, sz = vec->size (); i < sz; i++)
      append (vec->fetch (i));
}

template <typename ITEM> Vector<ITEM> *
Vector<ITEM>::copy ()
{
  // This routine will return a copy of "this".
  Vector<ITEM> *vector;
  vector = new Vector<ITEM>;
  vector->count = count;
  vector->limit = limit;
  vector->data = (ITEM *) malloc (sizeof (ITEM) * limit);
  (void) memcpy ((char *) vector->data, (char *) data, sizeof (ITEM) * count);
  return vector;
}

template <typename ITEM> long
Vector<ITEM>::find (const ITEM match_item)
{
  for (long i = 0; i < size (); i++)
    if (match_item == get (i))
      return i;
  return -1;
}

template <typename ITEM> long
Vector<ITEM>::find_r (const ITEM match_item)
{
  for (long i = size () - 1; i >= 0; i--)
    if (match_item == get (i))
      return i;
  return -1;
}

template <typename ITEM> void
Vector<ITEM>::insert (long index, const ITEM item)
{
  // This routine will insert "item" into the "index"'th slot of "this".
  // An error occurs if "index" > size().
  // "index" is allowed to be equal to "count" in the case that
  // you are inserting past the last element of the vector.
  // In that case, the bcopy below becomes a no-op.
  assert (index >= 0);
  assert (index <= count);
  append (item);
  (void) memmove (((char *) (&data[index + 1])), (char *) (&data[index]),
		  (count - index - 1) * sizeof (ITEM));
  data[index] = item;
}

template <typename ITEM> ITEM
Vector<ITEM>::remove (long index)
{
  // This routine will remove the "index"'th item from "this" and
  // return it.  An error occurs if "index" >= size();.
  assert (index >= 0);
  assert (index < count);
  ITEM item = data[index];
  for (long i = index + 1; i < count; i++)
    data[i - 1] = data[i];
  count--;
  // Bad code that works good when ITEM is a pointer type
  data[count] = item;
  return data[count];
}

template <typename ITEM> void
Vector<ITEM>::swap (long index1, long index2)
{
  ITEM item;
  item = data[index1];
  data[index1] = data[index2];
  data[index2] = item;
}

template <typename ITEM> void
Vector<ITEM>::store (long index, const ITEM item)
{
  if (index >= count)
    {
      resize (index);
      memset (&data[count], 0, (index - count) * sizeof (ITEM));
      count = index + 1;
    }
  data[index] = item;
}

// This routine performs a binary search across
// the entire vector, with "start" being the low boundary.
// It is assumed that the vector is SORTED in
// ASCENDING ORDER by the same criteria as the
// compare function.
// If no match is found, -1 is returned.
template <typename ITEM> long
Vector<ITEM>::bisearch (long start, long end, void *key, CompareFunc compare)
{
  ITEM *itemp;
  if (end == -1)
    end = count;
  if (start >= end)
    return -1; // start exceeds limit
  itemp = (ITEM *) bsearch ((char *) key, (char *) &data[start],
			  end - start, sizeof (ITEM), (StdCompareFunc) compare);
  if (itemp == (ITEM *) 0)
    return -1; // not found
  return (long) (itemp - data);
}

template <typename ITEM> void
Vector<ITEM>::incorporate (const ITEM item, CompareFunc compare)
{
  long lt = 0;
  long rt = count - 1;
  while (lt <= rt)
    {
      long md = (lt + rt) / 2;
      if (compare (data[md], item) < 0)
	lt = md + 1;
      else
	rt = md - 1;
    }
  if (lt == count)
    append (item);
  else
    insert (lt, item);
}

#define QSTHRESH 6

template <typename ITEM> void
qsort (ITEM *base, size_t nelem, ExtCompareFunc qcmp, void *arg)
{
  for (;;)
    {
      // For small arrays use insertion sort
      if (nelem < QSTHRESH)
	{
	  for (size_t i = 1; i < nelem; i++)
	    {
	      ITEM *p = base + i;
	      ITEM *q = p - 1;
	      if (qcmp (q, p, arg) > 0)
		{
		  ITEM t = *p;
		  *p = *q;
		  while (q > base && qcmp (q - 1, &t, arg) > 0)
		    {
		      *q = *(q - 1);
		      --q;
		    }
		  *q = t;
		}
	    }
	  return;
	}

      ITEM *last = base + nelem - 1;
      ITEM *mid = base + nelem / 2;
      // Sort the first, middle, and last elements
      ITEM *a1 = base, *a2, *a3;
      if (qcmp (base, mid, arg) > 0)
	{
	  if (qcmp (mid, last, arg) > 0)
	    { // l-m-b
	      a2 = last;
	      a3 = last;
	    }
	  else if (qcmp (base, last, arg) > 0)
	    { // l-b-m
	      a2 = mid;
	      a3 = last;
	    }
	  else
	    { // m-b-l
	      a2 = mid;
	      a3 = mid;
	    }
	}
      else if (qcmp (mid, last, arg) > 0)
	{
	  a1 = mid;
	  a3 = last;
	  if (qcmp (base, last, arg) > 0)  // m-l-b
	    a2 = base;
	  else  // b-l-m
	    a2 = a3;
	}
      else // b-m-l
	a3 = a2 = a1;
      if (a1 != a2)
	{
	  ITEM t = *a1;
	  *a1 = *a2;
	  if (a2 != a3)
	    *a2 = *a3;
	  *a3 = t;
	}

      // Partition
      ITEM *i = base + 1;
      ITEM *j = last - 1;
      for (;;)
	{
	  while (i < mid && qcmp (i, mid, arg) <= 0)
	    i++;
	  while (j > mid && qcmp (mid, j, arg) <= 0)
	    j--;
	  if (i == j)
	    break;
	  ITEM t = *i;
	  *i = *j;
	  *j = t;
	  if (i == mid)
	    {
	      mid = j;
	      i++;
	    }
	  else if (j == mid)
	    {
	      mid = i;
	      j--;
	    }
	  else
	    {
	      i++;
	      j--;
	    }
	}

      // Compare two partitions. Do the smaller one by recursion
      // and loop over the larger one.
      size_t nleft = mid - base;
      size_t nright = nelem - nleft - 1;
      if (nleft <= nright)
	{
	  qsort (base, nleft, qcmp, arg);
	  base = mid + 1;
	  nelem = nright;
	}
      else
	{
	  qsort (mid + 1, nright, qcmp, arg);
	  nelem = nleft;
	}
    }
}

template<> inline void
Vector<char*>::destroy ()
{
  for (long i = 0; i < count; i++)
    free (data[i]);
  count = 0;
}

template <typename ITEM> inline void
Vector<ITEM>::destroy ()
{
  for (long i = 0; i < count; i++)
    delete data[i];
  count = 0;
}

#endif /* _VEC_H */
