/* Fast fuzzy searching among messages.
   Copyright (C) 2006, 2008, 2011, 2013, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "msgl-fsearch.h"

#include <math.h>
#include <stdlib.h>

#include "xalloc.h"
#include "po-charset.h"


/* Fuzzy searching of L strings in a large set of N messages (assuming
   they have all the same small size) takes O(L * N) when using repeated
   fstrcmp() calls.  When for example L = 800 and N = 69000, this is slow.

   So we preprocess the set of N messages, yielding a data structure
   that allows to select the similar messages for any given string, and
   apply fstrcmp() only to the search result.  This allows to reduce the
   time to something between O(L * 1) and O(L * N) - depending on the
   structure of the strings.  In the example with L = 800 and N = 69000,
   memory use increases by a factor of 2 and the time decreases from
   9068 s to 230 s.

   The data structure is a hash table mapping each n-gram (here with n=4)
   occurring in the message strings to the list of messages that contains
   it.  For example, if the message list is
      [0] "close"
      [1] "losers"
   the index is a hash table mapping
      "clos" -> { 0 }
      "lose" -> { 0, 1 }
      "oser" -> { 1 }
      "sers" -> { 1 }
   Searching the similar messages to, say, "closest", is done by looking up
   all its 4-grams in the hash table and summing up the results:
      "clos" -> { 0 }
      "lose" -> { 0, 1 }
      "oses" -> {}
      "sest" -> {}
   => total: { 2x0, 1x1 }
   The list is sorted according to decreasing frequency: { 0, 1, ... }
   and only the first few messages from this frequency list are passed to
   fstrcmp().

   The n-gram similarity and the fstrcmp() similarity are quite different
   metrics.  For example, "close" and "c l o s e" have no n-grams in common
   (even for n as small as n = 2); however, fstrcmp() will find that they
   are quite similar (= 0.71).  Conversely, "AAAA BBBB ... ZZZZ" and
   "ZZZZ ... BBBB AAAA" have many 4-grams in common, but their fstrcmp() is
   only 0.02.  Also, repeated n-grams don't have the same effect on the
   two similarity measures.  But all this doesn't matter much in practice.

   We chose n = 4 because for alphabetic languages, with n = 3 the occurrence
   lists are likely too long.  (Well, with ideographic languages like Chinese,
   n = 3 might actually be quite good.  Anyway, n = 4 is not bad in this case
   either.)

   The units are characters in the current encoding.  Not just bytes,
   because 4 consecutive bytes in UTF-8 or GB18030 don't mean much.  */

/* Each message is represented by its index in the message list.  */
typedef unsigned int index_ty;

/* An index list has its allocated size and length tacked at the front.
   The indices are sorted in ascending order.  */
typedef index_ty *index_list_ty;
#define IL_ALLOCATED 0
#define IL_LENGTH 1

/* Create a new index list containing only a given index.  */
static inline index_list_ty
new_index (index_ty idx)
{
  index_ty *list = XNMALLOC (2 + 1, index_ty);
  list[IL_ALLOCATED] = 1;
  list[IL_LENGTH] = 1;
  list[2] = idx;

  return list;
}

/* Add a given index, greater or equal than all previous indices, to an
   index list.
   Return the new index list, if it had to be reallocated, or NULL if it
   didn't change.  */
static inline index_list_ty
addlast_index (index_list_ty list, index_ty idx)
{
  index_list_ty result;
  size_t length = list[IL_LENGTH];

  /* Look whether it should be inserted.  */
  if (length > 0 && list[2 + (length - 1)] == idx)
    return NULL;

  /* Now make room for one more list element.  */
  result = NULL;
  if (length == list[IL_ALLOCATED])
    {
      size_t new_allocated = 2 * length - (length >> 6);
      list = (index_ty *) xrealloc (list, (2 + new_allocated) * sizeof (index_ty));
      list[IL_ALLOCATED] = new_allocated;
      result = list;
    }
  list[2 + length] = idx;
  list[IL_LENGTH] = length + 1;
  return result;
}

/* Add a given index to an index list.
   Return the new index list, if it had to be reallocated, or NULL if it
   didn't change.  */
static inline index_list_ty
add_index (index_list_ty list, index_ty idx)
{
  index_list_ty result;
  size_t length = list[IL_LENGTH];

  /* Look where it should be inserted.  */
  size_t lo = 0;
  size_t hi = length;
  while (lo < hi)
    {
      /* Here we know that idx must be inserted at a position >= lo, <= hi.  */
      size_t mid = (lo + hi) / 2; /* lo <= mid < hi */
      index_ty val = list[2 + mid];
      if (val < idx)
        lo = mid + 1;
      else if (val > idx)
        hi = mid;
      else
        return NULL;
    }

  /* Now make room for one more list element.  */
  result = NULL;
  if (length == list[IL_ALLOCATED])
    {
      size_t new_allocated = 2 * length - (length >> 6);
      list = (index_ty *) xrealloc (list, (2 + new_allocated) * sizeof (index_ty));
      list[IL_ALLOCATED] = new_allocated;
      result = list;
    }
  list[IL_LENGTH] = length + 1;
  for (; length > hi; length--)
    list[2 + length] = list[1 + length];
  list[2 + length] = idx;
  return result;
}

/* We use 4-grams, therefore strings with less than 4 characters cannot be
   handled through the 4-grams table and need to be handled specially.
   Since every character occupies at most 4 bytes (see po-charset.c),
   this means the size of such short strings is bounded by:  */
#define SHORT_STRING_MAX_CHARACTERS (4 - 1)
#define SHORT_STRING_MAX_BYTES (SHORT_STRING_MAX_CHARACTERS * 4)

/* Such short strings are handled by direct comparison with all messages
   of appropriate size.  Note that for two strings of length 0 <= l1 <= l2,
   fstrcmp() is <= 2 * l1 / (l1 + l2).  Since we are only interested in
   fstrcmp() values >= FUZZY_THRESHOLD, we can for a given string of length l
   limit the search to lengths l' in the range
     l / (2 / FUZZY_THRESHOLD - 1) <= l' <= l * (2 / FUZZY_THRESHOLD - 1)
   Thus we need the list of the short strings up to length:  */
#if !defined __SUNPRO_C
# define SHORT_MSG_MAX (int) (SHORT_STRING_MAX_BYTES * (2 / FUZZY_THRESHOLD - 1))
#else
/* Sun C on Solaris 8 cannot compute this constant expression.  */
# define SHORT_MSG_MAX 28
#endif

/* A fuzzy index contains a hash table mapping all n-grams to their
   occurrences list.  */
struct message_fuzzy_index_ty
{
  message_ty **messages;
  character_iterator_t iterator;
  hash_table gram4;
  size_t firstfew;
  message_list_ty **short_messages;
};

/* Allocate a fuzzy index corresponding to a given list of messages.
   The list of messages and the msgctxt and msgid fields of the messages
   inside it must not be modified while the returned fuzzy index is in use.  */
message_fuzzy_index_ty *
message_fuzzy_index_alloc (const message_list_ty *mlp,
                           const char *canon_charset)
{
  message_fuzzy_index_ty *findex = XMALLOC (message_fuzzy_index_ty);
  size_t count = mlp->nitems;
  size_t j;
  size_t l;

  findex->messages = mlp->item;
  findex->iterator = po_charset_character_iterator (canon_charset);

  /* Setup hash table.  */
  hash_init (&findex->gram4, 10 * count);
  for (j = 0; j < count; j++)
    {
      message_ty *mp = mlp->item[j];

      if (mp->msgstr != NULL && mp->msgstr[0] != '\0')
        {
          const char *str = mp->msgid;

          /* Let p0 < p1 < p2 < p3 < p4 walk through the string.  */
          const char *p0 = str;
          if (*p0 != '\0')
            {
              const char *p1 = p0 + findex->iterator (p0);
              if (*p1 != '\0')
                {
                  const char *p2 = p1 + findex->iterator (p1);
                  if (*p2 != '\0')
                    {
                      const char *p3 = p2 + findex->iterator (p2);
                      if (*p3 != '\0')
                        {
                          const char *p4 = p3 + findex->iterator (p3);
                          for (;;)
                            {
                              /* The segment from p0 to p4 is a 4-gram of
                                 characters.  Add a hash table entry that maps
                                 it to the index j, or extend the existing
                                 hash table entry accordingly.  */
                              void *found;

                              if (hash_find_entry (&findex->gram4, p0, p4 - p0,
                                                   &found) == 0)
                                {
                                  index_list_ty list = (index_list_ty) found;
                                  list = addlast_index (list, j);
                                  if (list != NULL)
                                    hash_set_value (&findex->gram4, p0, p4 - p0,
                                                    list);
                                }
                              else
                                hash_insert_entry (&findex->gram4, p0, p4 - p0,
                                                   new_index (j));

                              /* Advance.  */
                              if (*p4 == '\0')
                                break;
                              p0 = p1;
                              p1 = p2;
                              p2 = p3;
                              p3 = p4;
                              p4 = p4 + findex->iterator (p4);
                            }
                        }
                    }
                }
            }
        }
    }

  /* Shrink memory used by the hash table.  */
  {
    void *iter;
    const void *key;
    size_t keylen;
    void **valuep;

    iter = NULL;
    while (hash_iterate_modify (&findex->gram4, &iter, &key, &keylen, &valuep)
           == 0)
      {
        index_list_ty list = (index_list_ty) *valuep;
        index_ty length = list[IL_LENGTH];

        if (length < list[IL_ALLOCATED])
          {
            list[IL_ALLOCATED] = length;
            *valuep = xrealloc (list, (2 + length) * sizeof (index_ty));
          }
      }
  }

  findex->firstfew = (int) sqrt ((double) count);
  if (findex->firstfew < 10)
    findex->firstfew = 10;

  /* Setup lists of short messages.  */
  findex->short_messages = XNMALLOC (SHORT_MSG_MAX + 1, message_list_ty *);
  for (l = 0; l <= SHORT_MSG_MAX; l++)
    findex->short_messages[l] = message_list_alloc (false);
  for (j = 0; j < count; j++)
    {
      message_ty *mp = mlp->item[j];

      if (mp->msgstr != NULL && mp->msgstr[0] != '\0')
        {
          const char *str = mp->msgid;
          size_t len = strlen (str);

          if (len <= SHORT_MSG_MAX)
            message_list_append (findex->short_messages[len], mp);
        }
    }

  /* Shrink memory used by the lists of short messages.  */
  for (l = 0; l <= SHORT_MSG_MAX; l++)
    {
      message_list_ty *smlp = findex->short_messages[l];

      if (smlp->nitems < smlp->nitems_max)
        {
          smlp->nitems_max = smlp->nitems;
          smlp->item =
            (message_ty **)
            xrealloc (smlp->item, smlp->nitems_max * sizeof (message_ty *));
        }
    }

  return findex;
}

/* An index with multiplicity.  */
struct mult_index
{
  index_ty index;
  unsigned int count;
};

/* A list of indices with multiplicity.
   The indices are sorted in ascending order.  */
struct mult_index_list
{
  struct mult_index *item;
  size_t nitems;
  size_t nitems_max;
  /* Work area.  */
  struct mult_index *item2;
  size_t nitems2_max;
};

/* Initialize an empty list of indices with multiplicity.  */
static inline void
mult_index_list_init (struct mult_index_list *accu)
{
  accu->nitems = 0;
  accu->nitems_max = 0;
  accu->item = NULL;
  accu->nitems2_max = 0;
  accu->item2 = NULL;
}

/* Add an index list to a list of indices with multiplicity.  */
static inline void
mult_index_list_accumulate (struct mult_index_list *accu, index_list_ty list)
{
  size_t len1 = accu->nitems;
  size_t len2 = list[IL_LENGTH];
  size_t need = len1 + len2;
  struct mult_index *ptr1;
  struct mult_index *ptr1_end;
  index_ty *ptr2;
  index_ty *ptr2_end;
  struct mult_index *destptr;

  /* Make the work area large enough.  */
  if (accu->nitems2_max < need)
    {
      size_t new_max = 2 * accu->nitems2_max + 1;

      if (new_max < need)
        new_max = need;
      if (accu->item2 != NULL)
        free (accu->item2);
      accu->item2 = XNMALLOC (new_max, struct mult_index);
      accu->nitems2_max = new_max;
    }

  /* Make a linear pass through accu and list simultaneously.  */
  ptr1 = accu->item;
  ptr1_end = ptr1 + len1;
  ptr2 = list + 2;
  ptr2_end = ptr2 + len2;
  destptr = accu->item2;
  while (ptr1 < ptr1_end && ptr2 < ptr2_end)
    {
      if (ptr1->index < *ptr2)
        {
          *destptr = *ptr1;
          ptr1++;
        }
      else if (ptr1->index > *ptr2)
        {
          destptr->index = *ptr2;
          destptr->count = 1;
          ptr2++;
        }
      else /* ptr1->index == list[2 + i2] */
        {
          destptr->index = ptr1->index;
          destptr->count = ptr1->count + 1;
          ptr1++;
          ptr2++;
        }
      destptr++;
    }
  while (ptr1 < ptr1_end)
    {
      *destptr = *ptr1;
      ptr1++;
      destptr++;
    }
  while (ptr2 < ptr2_end)
    {
      destptr->index = *ptr2;
      destptr->count = 1;
      ptr2++;
      destptr++;
    }

  /* Swap accu->item and accu->item2.  */
  {
    struct mult_index *dest = accu->item2;
    size_t dest_max = accu->nitems2_max;

    accu->item2 = accu->item;
    accu->nitems2_max = accu->nitems_max;

    accu->item = dest;
    accu->nitems = destptr - dest;
    accu->nitems_max = dest_max;
  }
}

/* Compares two indices with multiplicity, according to their multiplicity.  */
static int
mult_index_compare (const void *p1, const void *p2)
{
  const struct mult_index *ptr1 = (const struct mult_index *) p1;
  const struct mult_index *ptr2 = (const struct mult_index *) p2;

  if (ptr1->count < ptr2->count)
    return 1;
  if (ptr1->count > ptr2->count)
    return -1;
  /* For reproduceable results, also take into account the indices.  */
  if (ptr1->index > ptr2->index)
    return 1;
  if (ptr1->index < ptr2->index)
    return -1;
  return 0;
}

/* Sort a list of indices with multiplicity according to decreasing
   multiplicity.  */
static inline void
mult_index_list_sort (struct mult_index_list *accu)
{
  if (accu->nitems > 1)
    qsort (accu->item, accu->nitems, sizeof (struct mult_index),
           mult_index_compare);
}

/* Frees a list of indices with multiplicity.  */
static inline void
mult_index_list_free (struct mult_index_list *accu)
{
  if (accu->item != NULL)
    free (accu->item);
  if (accu->item2 != NULL)
    free (accu->item2);
}

/* Find a good match for the given msgctxt and msgid in the given fuzzy index.
   The match does not need to be optimal.
   Ignore matches for which the fuzzy_search_goal_function is < LOWER_BOUND.
   LOWER_BOUND must be >= FUZZY_THRESHOLD.
   If HEURISTIC is true, only the few best messages among the list - according
   to a certain heuristic - are considered.  If HEURISTIC is false, all
   messages with a fuzzy_search_goal_function > FUZZY_THRESHOLD are considered,
   like in message_list_search_fuzzy (except that in ambiguous cases where
   several best matches exist, message_list_search_fuzzy chooses the one with
   the smallest index whereas message_fuzzy_index_search makes a better
   choice).  */
message_ty *
message_fuzzy_index_search (message_fuzzy_index_ty *findex,
                            const char *msgctxt, const char *msgid,
                            double lower_bound,
                            bool heuristic)
{
  const char *str = msgid;

  /* Let p0 < p1 < p2 < p3 < p4 walk through the string.  */
  const char *p0 = str;
  if (*p0 != '\0')
    {
      const char *p1 = p0 + findex->iterator (p0);
      if (*p1 != '\0')
        {
          const char *p2 = p1 + findex->iterator (p1);
          if (*p2 != '\0')
            {
              const char *p3 = p2 + findex->iterator (p2);
              if (*p3 != '\0')
                {
                  const char *p4 = p3 + findex->iterator (p3);
                  struct mult_index_list accu;

                  mult_index_list_init (&accu);
                  for (;;)
                    {
                      /* The segment from p0 to p4 is a 4-gram of
                         characters.  Get the hash table entry containing
                         a list of indices, and add it to the accu.  */
                      void *found;

                      if (hash_find_entry (&findex->gram4, p0, p4 - p0,
                                           &found) == 0)
                        {
                          index_list_ty list = (index_list_ty) found;
                          mult_index_list_accumulate (&accu, list);
                        }

                      /* Advance.  */
                      if (*p4 == '\0')
                        break;
                      p0 = p1;
                      p1 = p2;
                      p2 = p3;
                      p3 = p4;
                      p4 = p4 + findex->iterator (p4);
                    }

                  /* Sort in decreasing count order.  */
                  mult_index_list_sort (&accu);

                  /* Iterate over this sorted list, and maximize the
                     fuzzy_search_goal_function() result.
                     If HEURISTIC is true, take only the first few messages.
                     If HEURISTIC is false, consider all messages - to match
                     the behaviour of message_list_search_fuzzy -, but process
                     them in the order of the sorted list.  This increases
                     the chances that the later calls to fstrcmp_bounded() (via
                     fuzzy_search_goal_function()) terminate quickly, thanks
                     to the best_weight which will be quite high already after
                     the first few messages.  */
                  {
                    size_t count;
                    struct mult_index *ptr;
                    message_ty *best_mp;
                    double best_weight;

                    count = accu.nitems;
                    if (heuristic)
                      {
                        if (count > findex->firstfew)
                          count = findex->firstfew;
                      }

                    best_weight = lower_bound;
                    best_mp = NULL;
                    for (ptr = accu.item; count > 0; ptr++, count--)
                      {
                        message_ty *mp = findex->messages[ptr->index];
                        double weight =
                          fuzzy_search_goal_function (mp, msgctxt, msgid,
                                                      best_weight);

                        if (weight > best_weight)
                          {
                            best_weight = weight;
                            best_mp = mp;
                          }
                      }

                    mult_index_list_free (&accu);

                    return best_mp;
                  }
                }
            }
        }
    }

  /* The string had less than 4 characters.  */
  {
    size_t l = strlen (str);
    size_t lmin, lmax;
    message_ty *best_mp;
    double best_weight;

    if (!(l <= SHORT_STRING_MAX_BYTES))
      abort ();

    /* Walk through those short messages which have an appropriate length.
       See the comment before SHORT_MSG_MAX.  */
    lmin = (int) ceil (l / (2 / FUZZY_THRESHOLD - 1));
    lmax = (int) (l * (2 / FUZZY_THRESHOLD - 1));
    if (!(lmax <= SHORT_MSG_MAX))
      abort ();

    best_weight = lower_bound;
    best_mp = NULL;
    for (l = lmin; l <= lmax; l++)
      {
        message_list_ty *mlp = findex->short_messages[l];
        size_t j;

        for (j = 0; j < mlp->nitems; j++)
          {
            message_ty *mp = mlp->item[j];
            double weight =
              fuzzy_search_goal_function (mp, msgctxt, msgid, best_weight);

            if (weight > best_weight)
              {
                best_weight = weight;
                best_mp = mp;
              }
          }
      }

    return best_mp;
  }
}

/* Free a fuzzy index.  */
void
message_fuzzy_index_free (message_fuzzy_index_ty *findex)
{
  size_t l;
  void *iter;
  const void *key;
  size_t keylen;
  void *data;

  /* Free the short lists.  */
  for (l = 0; l <= SHORT_MSG_MAX; l++)
    message_list_free (findex->short_messages[l], 1);
  free (findex->short_messages);

  /* Free the index lists occurring as values in the hash tables.  */
  iter = NULL;
  while (hash_iterate (&findex->gram4, &iter, &key, &keylen, &data) == 0)
    free ((index_list_ty *) data);
  /* Free the hash table itself.  */
  hash_destroy (&findex->gram4);

  free (findex);
}
