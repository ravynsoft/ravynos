/* Lisp format strings.
   Copyright (C) 2001-2004, 2006-2007, 2009, 2014, 2019, 2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

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

#include <stdbool.h>
#include <stdlib.h>

#include "format.h"
#include "c-ctype.h"
#include "gcd.h"
#include "xalloc.h"
#include "xvasprintf.h"
#include "format-invalid.h"
#include "minmax.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Assertion macro.  Could be defined to empty for speed.  */
#define ASSERT(expr) if (!(expr)) abort ();


/* Lisp format strings are described in the Common Lisp HyperSpec,
   chapter 22.3 "Formatted Output".  */

/* Data structure describing format string derived constraints for an
   argument list.  It is a recursive list structure.  Structure sharing
   is not allowed.  */

enum format_cdr_type
{
  FCT_REQUIRED, /* The format argument list cannot end before this argument.  */
  FCT_OPTIONAL  /* The format argument list may end before this argument.  */
};

enum format_arg_type
{
  FAT_OBJECT,                   /* Any object, type T.  */
  FAT_CHARACTER_INTEGER_NULL,   /* Type (OR CHARACTER INTEGER NULL).  */
  FAT_CHARACTER_NULL,           /* Type (OR CHARACTER NULL).  */
  FAT_CHARACTER,                /* Type CHARACTER.  */
  FAT_INTEGER_NULL,             /* Type (OR INTEGER NULL).  */
  FAT_INTEGER,                  /* Meant for objects of type INTEGER.  */
  FAT_REAL,                     /* Meant for objects of type REAL.  */
  FAT_LIST,                     /* Meant for proper lists.  */
  FAT_FORMATSTRING,             /* Format strings.  */
  FAT_FUNCTION                  /* Function.  */
};

struct format_arg
{
  unsigned int repcount; /* Number of consecutive arguments this constraint
                            applies to.  Normally 1, but unconstrained
                            arguments are often repeated.  */
  enum format_cdr_type presence; /* Can the argument list end right before
                                    this argument?  */
  enum format_arg_type type;    /* Possible values for this argument.  */
  struct format_arg_list *list; /* For FAT_LIST: List elements.  */
};

struct segment
{
  unsigned int count;   /* Number of format_arg records used.  */
  unsigned int allocated;
  struct format_arg *element;   /* Argument constraints.  */
  unsigned int length; /* Number of arguments represented by this segment.
                          This is the sum of all repcounts in the segment.  */
};

struct format_arg_list
{
  /* The constraints for the potentially infinite argument list are assumed
     to become ultimately periodic.  (Too complicated argument lists without
     a-priori period, like
            (format t "~@{~:[-~;~S~]~}" nil t 1 t 3 nil t 4)
     are described by a constraint that ends in a length 1 period of
     unconstrained arguments.)  Such a periodic sequence can be split into
     an initial segment and an endlessly repeated loop segment.
     A finite sequence is represented entirely in the initial segment; the
     loop segment is empty.  */

  struct segment initial;       /* Initial arguments segment.  */
  struct segment repeated;      /* Endlessly repeated segment.  */
};

struct spec
{
  unsigned int directives;
  struct format_arg_list *list;
};


/* Parameter for a directive.  */
enum param_type
{
  PT_NIL,       /* param not present */
  PT_CHARACTER, /* character */
  PT_INTEGER,   /* integer */
  PT_ARGCOUNT,  /* number of remaining arguments */
  PT_V          /* variable taken from argument list */
};

struct param
{
  enum param_type type;
  int value;            /* for PT_INTEGER: the value, for PT_V: the position */
};


/* Forward declaration of local functions.  */
#define union make_union
static void verify_list (const struct format_arg_list *list);
static void free_list (struct format_arg_list *list);
static struct format_arg_list * copy_list (const struct format_arg_list *list);
static bool equal_list (const struct format_arg_list *list1,
                        const struct format_arg_list *list2);
static struct format_arg_list * make_intersected_list
                                               (struct format_arg_list *list1,
                                                struct format_arg_list *list2);
static struct format_arg_list * make_intersection_with_empty_list
                                                (struct format_arg_list *list);
static struct format_arg_list * make_union_list
                                               (struct format_arg_list *list1,
                                                struct format_arg_list *list2);


/* ======================= Verify a format_arg_list ======================= */

/* Verify some invariants.  */
static void
verify_element (const struct format_arg * e)
{
  ASSERT (e->repcount > 0);
  if (e->type == FAT_LIST)
    verify_list (e->list);
}

/* Verify some invariants.  */
/* Memory effects: none.  */
static void
verify_list (const struct format_arg_list *list)
{
  unsigned int i;
  unsigned int total_repcount;

  ASSERT (list->initial.count <= list->initial.allocated);
  total_repcount = 0;
  for (i = 0; i < list->initial.count; i++)
    {
      verify_element (&list->initial.element[i]);
      total_repcount += list->initial.element[i].repcount;
    }
  ASSERT (total_repcount == list->initial.length);

  ASSERT (list->repeated.count <= list->repeated.allocated);
  total_repcount = 0;
  for (i = 0; i < list->repeated.count; i++)
    {
      verify_element (&list->repeated.element[i]);
      total_repcount += list->repeated.element[i].repcount;
    }
  ASSERT (total_repcount == list->repeated.length);
}

/* Assertion macro.  Could be defined to empty for speed.  */
#define VERIFY_LIST(list) verify_list (list)


/* ======================== Free a format_arg_list ======================== */

/* Free the data belonging to an argument list element.  */
static inline void
free_element (struct format_arg *element)
{
  if (element->type == FAT_LIST)
    free_list (element->list);
}

/* Free an argument list.  */
/* Memory effects: Frees list.  */
static void
free_list (struct format_arg_list *list)
{
  unsigned int i;

  for (i = 0; i < list->initial.count; i++)
    free_element (&list->initial.element[i]);
  if (list->initial.element != NULL)
    free (list->initial.element);

  for (i = 0; i < list->repeated.count; i++)
    free_element (&list->repeated.element[i]);
  if (list->repeated.element != NULL)
    free (list->repeated.element);
}


/* ======================== Copy a format_arg_list ======================== */

/* Copy the data belonging to an argument list element.  */
static inline void
copy_element (struct format_arg *newelement,
              const struct format_arg *oldelement)
{
  newelement->repcount = oldelement->repcount;
  newelement->presence = oldelement->presence;
  newelement->type = oldelement->type;
  if (oldelement->type == FAT_LIST)
    newelement->list = copy_list (oldelement->list);
}

/* Copy an argument list.  */
/* Memory effects: Freshly allocated result.  */
static struct format_arg_list *
copy_list (const struct format_arg_list *list)
{
  struct format_arg_list *newlist;
  unsigned int length;
  unsigned int i;

  VERIFY_LIST (list);

  newlist = XMALLOC (struct format_arg_list);

  newlist->initial.count = newlist->initial.allocated = list->initial.count;
  length = 0;
  if (list->initial.count == 0)
    newlist->initial.element = NULL;
  else
    {
      newlist->initial.element =
        XNMALLOC (newlist->initial.allocated, struct format_arg);
      for (i = 0; i < list->initial.count; i++)
        {
          copy_element (&newlist->initial.element[i],
                        &list->initial.element[i]);
          length += list->initial.element[i].repcount;
        }
    }
  ASSERT (length == list->initial.length);
  newlist->initial.length = length;

  newlist->repeated.count = newlist->repeated.allocated = list->repeated.count;
  length = 0;
  if (list->repeated.count == 0)
    newlist->repeated.element = NULL;
  else
    {
      newlist->repeated.element =
        XNMALLOC (newlist->repeated.allocated, struct format_arg);
      for (i = 0; i < list->repeated.count; i++)
        {
          copy_element (&newlist->repeated.element[i],
                        &list->repeated.element[i]);
          length += list->repeated.element[i].repcount;
        }
    }
  ASSERT (length == list->repeated.length);
  newlist->repeated.length = length;

  VERIFY_LIST (newlist);

  return newlist;
}


/* ===================== Compare two format_arg_lists ===================== */

/* Tests whether two normalized argument constraints are equivalent,
   ignoring the repcount.  */
static bool
equal_element (const struct format_arg * e1, const struct format_arg * e2)
{
  return (e1->presence == e2->presence
          && e1->type == e2->type
          && (e1->type == FAT_LIST ? equal_list (e1->list, e2->list) : true));
}

/* Tests whether two normalized argument list constraints are equivalent.  */
/* Memory effects: none.  */
static bool
equal_list (const struct format_arg_list *list1,
            const struct format_arg_list *list2)
{
  unsigned int n, i;

  VERIFY_LIST (list1);
  VERIFY_LIST (list2);

  n = list1->initial.count;
  if (n != list2->initial.count)
    return false;
  for (i = 0; i < n; i++)
    {
      const struct format_arg * e1 = &list1->initial.element[i];
      const struct format_arg * e2 = &list2->initial.element[i];

      if (!(e1->repcount == e2->repcount && equal_element (e1, e2)))
        return false;
    }

  n = list1->repeated.count;
  if (n != list2->repeated.count)
    return false;
  for (i = 0; i < n; i++)
    {
      const struct format_arg * e1 = &list1->repeated.element[i];
      const struct format_arg * e2 = &list2->repeated.element[i];

      if (!(e1->repcount == e2->repcount && equal_element (e1, e2)))
        return false;
    }

  return true;
}


/* ===================== Incremental memory allocation ===================== */

/* Ensure list->initial.allocated >= newcount.  */
static inline void
ensure_initial_alloc (struct format_arg_list *list, unsigned int newcount)
{
  if (newcount > list->initial.allocated)
    {
      list->initial.allocated =
        MAX (2 * list->initial.allocated + 1, newcount);
      list->initial.element =
        (struct format_arg *)
        xrealloc (list->initial.element,
                  list->initial.allocated * sizeof (struct format_arg));
    }
}

/* Ensure list->initial.allocated > list->initial.count.  */
static inline void
grow_initial_alloc (struct format_arg_list *list)
{
  if (list->initial.count >= list->initial.allocated)
    {
      list->initial.allocated =
        MAX (2 * list->initial.allocated + 1, list->initial.count + 1);
      list->initial.element =
        (struct format_arg *)
        xrealloc (list->initial.element,
                  list->initial.allocated * sizeof (struct format_arg));
    }
}

/* Ensure list->repeated.allocated >= newcount.  */
static inline void
ensure_repeated_alloc (struct format_arg_list *list, unsigned int newcount)
{
  if (newcount > list->repeated.allocated)
    {
      list->repeated.allocated =
        MAX (2 * list->repeated.allocated + 1, newcount);
      list->repeated.element =
        (struct format_arg *)
        xrealloc (list->repeated.element,
                  list->repeated.allocated * sizeof (struct format_arg));
    }
}

/* Ensure list->repeated.allocated > list->repeated.count.  */
static inline void
grow_repeated_alloc (struct format_arg_list *list)
{
  if (list->repeated.count >= list->repeated.allocated)
    {
      list->repeated.allocated =
        MAX (2 * list->repeated.allocated + 1, list->repeated.count + 1);
      list->repeated.element =
        (struct format_arg *)
        xrealloc (list->repeated.element,
                  list->repeated.allocated * sizeof (struct format_arg));
    }
}


/* ====================== Normalize a format_arg_list ====================== */

/* Normalize an argument list constraint, assuming all sublists are already
   normalized.  */
/* Memory effects: Destructively modifies list.  */
static void
normalize_outermost_list (struct format_arg_list *list)
{
  unsigned int n, i, j;

  /* Step 1: Combine adjacent elements.
     Copy from i to j, keeping 0 <= j <= i.  */

  n = list->initial.count;
  for (i = j = 0; i < n; i++)
    if (j > 0
        && equal_element (&list->initial.element[i],
                          &list->initial.element[j-1]))
      {
        list->initial.element[j-1].repcount +=
          list->initial.element[i].repcount;
        free_element (&list->initial.element[i]);
      }
    else
      {
        if (j < i)
          list->initial.element[j] = list->initial.element[i];
        j++;
      }
  list->initial.count = j;

  n = list->repeated.count;
  for (i = j = 0; i < n; i++)
    if (j > 0
        && equal_element (&list->repeated.element[i],
                          &list->repeated.element[j-1]))
      {
        list->repeated.element[j-1].repcount +=
          list->repeated.element[i].repcount;
        free_element (&list->repeated.element[i]);
      }
    else
      {
        if (j < i)
          list->repeated.element[j] = list->repeated.element[i];
        j++;
      }
  list->repeated.count = j;

  /* Nothing more to be done if the loop segment is empty.  */
  if (list->repeated.count > 0)
    {
      unsigned int m, repcount0_extra;

      /* Step 2: Reduce the loop period.  */
      n = list->repeated.count;
      repcount0_extra = 0;
      if (n > 1
          && equal_element (&list->repeated.element[0],
                            &list->repeated.element[n-1]))
        {
          repcount0_extra = list->repeated.element[n-1].repcount;
          n--;
        }
      /* Proceed as if the loop period were n, with
         list->repeated.element[0].repcount incremented by repcount0_extra.  */
      for (m = 2; m <= n / 2; m++)
        if ((n % m) == 0)
          {
            /* m is a divisor of n.  Try to reduce the loop period to n.  */
            bool ok = true;

            for (i = 0; i < n - m; i++)
              if (!((list->repeated.element[i].repcount
                     + (i == 0 ? repcount0_extra : 0)
                     == list->repeated.element[i+m].repcount)
                    && equal_element (&list->repeated.element[i],
                                      &list->repeated.element[i+m])))
                {
                  ok = false;
                  break;
                }
            if (ok)
              {
                for (i = m; i < n; i++)
                  free_element (&list->repeated.element[i]);
                if (n < list->repeated.count)
                  list->repeated.element[m] = list->repeated.element[n];
                list->repeated.count = list->repeated.count - n + m;
                list->repeated.length /= n / m;
                break;
              }
          }
      if (list->repeated.count == 1)
        {
          /* The loop has period 1.  Normalize the repcount.  */
          list->repeated.element[0].repcount = 1;
          list->repeated.length = 1;
        }

      /* Step 3: Roll as much as possible of the initial segment's tail
         into the loop.  */
      if (list->repeated.count == 1)
        {
          if (list->initial.count > 0
              && equal_element (&list->initial.element[list->initial.count-1],
                                &list->repeated.element[0]))
            {
              /* Roll the last element of the initial segment into the loop.
                 Its repcount is irrelevant.  The second-to-last element is
                 certainly different and doesn't need to be considered.  */
              list->initial.length -=
                list->initial.element[list->initial.count-1].repcount;
              free_element (&list->initial.element[list->initial.count-1]);
              list->initial.count--;
            }
        }
      else
        {
          while (list->initial.count > 0
                 && equal_element (&list->initial.element[list->initial.count-1],
                                   &list->repeated.element[list->repeated.count-1]))
            {
              unsigned int moved_repcount =
                MIN (list->initial.element[list->initial.count-1].repcount,
                     list->repeated.element[list->repeated.count-1].repcount);

              /* Add the element at the start of list->repeated.  */
              if (equal_element (&list->repeated.element[0],
                                 &list->repeated.element[list->repeated.count-1]))
                list->repeated.element[0].repcount += moved_repcount;
              else
                {
                  unsigned int newcount = list->repeated.count + 1;
                  ensure_repeated_alloc (list, newcount);
                  for (i = newcount - 1; i > 0; i--)
                    list->repeated.element[i] = list->repeated.element[i-1];
                  list->repeated.count = newcount;
                  copy_element (&list->repeated.element[0],
                                &list->repeated.element[list->repeated.count-1]);
                  list->repeated.element[0].repcount = moved_repcount;
                }

              /* Remove the element from the end of list->repeated.  */
              list->repeated.element[list->repeated.count-1].repcount -=
                moved_repcount;
              if (list->repeated.element[list->repeated.count-1].repcount == 0)
                {
                  free_element (&list->repeated.element[list->repeated.count-1]);
                  list->repeated.count--;
                }

              /* Remove the element from the end of list->initial.  */
              list->initial.element[list->initial.count-1].repcount -=
                moved_repcount;
              if (list->initial.element[list->initial.count-1].repcount == 0)
                {
                  free_element (&list->initial.element[list->initial.count-1]);
                  list->initial.count--;
                }
              list->initial.length -= moved_repcount;
            }
        }
    }
}

/* Normalize an argument list constraint.  */
/* Memory effects: Destructively modifies list.  */
static void
normalize_list (struct format_arg_list *list)
{
  unsigned int n, i;

  VERIFY_LIST (list);

  /* First normalize all elements, recursively.  */
  n = list->initial.count;
  for (i = 0; i < n; i++)
    if (list->initial.element[i].type == FAT_LIST)
      normalize_list (list->initial.element[i].list);
  n = list->repeated.count;
  for (i = 0; i < n; i++)
    if (list->repeated.element[i].type == FAT_LIST)
      normalize_list (list->repeated.element[i].list);

  /* Then normalize the top level list.  */
  normalize_outermost_list (list);

  VERIFY_LIST (list);
}


/* ===================== Unconstrained and empty lists ===================== */

/* It's easier to allocate these on demand, than to be careful not to
   accidentally modify statically allocated lists.  */


/* Create an unconstrained argument list.  */
/* Memory effects: Freshly allocated result.  */
static struct format_arg_list *
make_unconstrained_list ()
{
  struct format_arg_list *list;

  list = XMALLOC (struct format_arg_list);
  list->initial.count = 0;
  list->initial.allocated = 0;
  list->initial.element = NULL;
  list->initial.length = 0;
  list->repeated.count = 1;
  list->repeated.allocated = 1;
  list->repeated.element = XNMALLOC (1, struct format_arg);
  list->repeated.element[0].repcount = 1;
  list->repeated.element[0].presence = FCT_OPTIONAL;
  list->repeated.element[0].type = FAT_OBJECT;
  list->repeated.length = 1;

  VERIFY_LIST (list);

  return list;
}


/* Create an empty argument list.  */
/* Memory effects: Freshly allocated result.  */
static struct format_arg_list *
make_empty_list ()
{
  struct format_arg_list *list;

  list = XMALLOC (struct format_arg_list);
  list->initial.count = 0;
  list->initial.allocated = 0;
  list->initial.element = NULL;
  list->initial.length = 0;
  list->repeated.count = 0;
  list->repeated.allocated = 0;
  list->repeated.element = NULL;
  list->repeated.length = 0;

  VERIFY_LIST (list);

  return list;
}


/* Test for an empty list.  */
/* Memory effects: none.  */
static bool
is_empty_list (const struct format_arg_list *list)
{
  return (list->initial.count == 0 && list->repeated.count == 0);
}


/* ======================== format_arg_list surgery ======================== */

/* Unfold list->repeated m times, where m >= 1.
   Assumes list->repeated.count > 0.  */
/* Memory effects: list is destructively modified.  */
static void
unfold_loop (struct format_arg_list *list, unsigned int m)
{
  unsigned int i, j, k;

  if (m > 1)
    {
      unsigned int newcount = list->repeated.count * m;
      ensure_repeated_alloc (list, newcount);
      i = list->repeated.count;
      for (k = 1; k < m; k++)
        for (j = 0; j < list->repeated.count; j++, i++)
          copy_element (&list->repeated.element[i], &list->repeated.element[j]);
      list->repeated.count = newcount;
      list->repeated.length = list->repeated.length * m;
    }
}

/* Ensure list->initial.length := m, where m >= list->initial.length.
   Assumes list->repeated.count > 0.  */
/* Memory effects: list is destructively modified.  */
static void
rotate_loop (struct format_arg_list *list, unsigned int m)
{
  if (m == list->initial.length)
    return;

  if (list->repeated.count == 1)
    {
      /* Instead of multiple copies of list->repeated.element[0], a single
         copy with higher repcount is appended to list->initial.  */
      unsigned int i, newcount;

      newcount = list->initial.count + 1;
      ensure_initial_alloc (list, newcount);
      i = list->initial.count;
      copy_element (&list->initial.element[i], &list->repeated.element[0]);
      list->initial.element[i].repcount = m - list->initial.length;
      list->initial.count = newcount;
      list->initial.length = m;
    }
  else
    {
      unsigned int n = list->repeated.length;

      /* Write m = list->initial.length + q * n + r with 0 <= r < n.  */
      unsigned int q = (m - list->initial.length) / n;
      unsigned int r = (m - list->initial.length) % n;

      /* Determine how many entries of list->repeated are needed for
         length r.  */
      unsigned int s;
      unsigned int t;

      for (t = r, s = 0;
           s < list->repeated.count && t >= list->repeated.element[s].repcount;
           t -= list->repeated.element[s].repcount, s++)
        ;

      /* s must be < list->repeated.count, otherwise r would have been >= n.  */
      ASSERT (s < list->repeated.count);

      /* So we need to add to list->initial:
         q full copies of list->repeated,
         plus the s first elements of list->repeated,
         plus, if t > 0, a splitoff of list->repeated.element[s].  */
      {
        unsigned int i, j, k, newcount;

        i = list->initial.count;
        newcount = i + q * list->repeated.count + s + (t > 0 ? 1 : 0);
        ensure_initial_alloc (list, newcount);
        for (k = 0; k < q; k++)
          for (j = 0; j < list->repeated.count; j++, i++)
            copy_element (&list->initial.element[i],
                          &list->repeated.element[j]);
        for (j = 0; j < s; j++, i++)
          copy_element (&list->initial.element[i], &list->repeated.element[j]);
        if (t > 0)
          {
            copy_element (&list->initial.element[i],
                          &list->repeated.element[j]);
            list->initial.element[i].repcount = t;
            i++;
          }
        ASSERT (i == newcount);
        list->initial.count = newcount;
        /* The new length of the initial segment is
           = list->initial.length
             + q * list->repeated.length
             + list->repeated[0..s-1].repcount + t
           = list->initial.length + q * n + r
           = m.
         */
        list->initial.length = m;
      }

      /* And rotate list->repeated.  */
      if (r > 0)
        {
          unsigned int i, j, oldcount, newcount;
          struct format_arg *newelement;

          oldcount = list->repeated.count;
          newcount = list->repeated.count + (t > 0 ? 1 : 0);
          newelement = XNMALLOC (newcount, struct format_arg);
          i = 0;
          for (j = s; j < oldcount; j++, i++)
            newelement[i] = list->repeated.element[j];
          for (j = 0; j < s; j++, i++)
            newelement[i] = list->repeated.element[j];
          if (t > 0)
            {
              copy_element (&newelement[oldcount], &newelement[0]);
              newelement[0].repcount -= t;
              newelement[oldcount].repcount = t;
            }
          free (list->repeated.element);
          list->repeated.element = newelement;
          list->repeated.count = newcount;
        }
    }
}


/* Ensure index n in the initial segment falls on a split between elements,
   i.e. if 0 < n < list->initial.length, then n-1 and n are covered by two
   different adjacent elements.  */
/* Memory effects: list is destructively modified.  */
static unsigned int
initial_splitelement (struct format_arg_list *list, unsigned int n)
{
  unsigned int s;
  unsigned int t;
  unsigned int oldrepcount;
  unsigned int newcount;
  unsigned int i;

  VERIFY_LIST (list);

  if (n > list->initial.length)
    {
      ASSERT (list->repeated.count > 0);
      rotate_loop (list, n);
      ASSERT (n <= list->initial.length);
    }

  /* Determine how many entries of list->initial need to be skipped.  */
  for (t = n, s = 0;
       s < list->initial.count && t >= list->initial.element[s].repcount;
       t -= list->initial.element[s].repcount, s++)
    ;

  if (t == 0)
    return s;

  ASSERT (s < list->initial.count);

  /* Split the entry into two entries.  */
  oldrepcount = list->initial.element[s].repcount;
  newcount = list->initial.count + 1;
  ensure_initial_alloc (list, newcount);
  for (i = list->initial.count - 1; i > s; i--)
    list->initial.element[i+1] = list->initial.element[i];
  copy_element (&list->initial.element[s+1], &list->initial.element[s]);
  list->initial.element[s].repcount = t;
  list->initial.element[s+1].repcount = oldrepcount - t;
  list->initial.count = newcount;

  VERIFY_LIST (list);

  return s+1;
}


/* Ensure index n in the initial segment is not shared.  Return its index.  */
/* Memory effects: list is destructively modified.  */
static unsigned int
initial_unshare (struct format_arg_list *list, unsigned int n)
{
  /* This does the same side effects as
       initial_splitelement (list, n);
       initial_splitelement (list, n + 1);
   */
  unsigned int s;
  unsigned int t;

  VERIFY_LIST (list);

  if (n >= list->initial.length)
    {
      ASSERT (list->repeated.count > 0);
      rotate_loop (list, n + 1);
      ASSERT (n < list->initial.length);
    }

  /* Determine how many entries of list->initial need to be skipped.  */
  for (t = n, s = 0;
       s < list->initial.count && t >= list->initial.element[s].repcount;
       t -= list->initial.element[s].repcount, s++)
    ;

  /* s must be < list->initial.count.  */
  ASSERT (s < list->initial.count);

  if (list->initial.element[s].repcount > 1)
    {
      /* Split the entry into at most three entries: for indices < n,
         for index n, and for indices > n.  */
      unsigned int oldrepcount = list->initial.element[s].repcount;
      unsigned int newcount =
        list->initial.count + (t == 0 || t == oldrepcount - 1 ? 1 : 2);
      ensure_initial_alloc (list, newcount);
      if (t == 0 || t == oldrepcount - 1)
        {
          unsigned int i;

          for (i = list->initial.count - 1; i > s; i--)
            list->initial.element[i+1] = list->initial.element[i];
          copy_element (&list->initial.element[s+1], &list->initial.element[s]);
          if (t == 0)
            {
              list->initial.element[s].repcount = 1;
              list->initial.element[s+1].repcount = oldrepcount - 1;
            }
          else
            {
              list->initial.element[s].repcount = oldrepcount - 1;
              list->initial.element[s+1].repcount = 1;
            }
        }
      else
        {
          unsigned int i;

          for (i = list->initial.count - 1; i > s; i--)
            list->initial.element[i+2] = list->initial.element[i];
          copy_element (&list->initial.element[s+2], &list->initial.element[s]);
          copy_element (&list->initial.element[s+1], &list->initial.element[s]);
          list->initial.element[s].repcount = t;
          list->initial.element[s+1].repcount = 1;
          list->initial.element[s+2].repcount = oldrepcount - 1 - t;
        }
      list->initial.count = newcount;
      if (t > 0)
        s++;
    }

  /* Now the entry for index n has repcount 1.  */
  ASSERT (list->initial.element[s].repcount == 1);

  VERIFY_LIST (list);

  return s;
}


/* Add n unconstrained elements at the front of the list.  */
/* Memory effects: list is destructively modified.  */
static void
shift_list (struct format_arg_list *list, unsigned int n)
{
  VERIFY_LIST (list);

  if (n > 0)
    {
      unsigned int i;

      grow_initial_alloc (list);
      for (i = list->initial.count; i > 0; i--)
        list->initial.element[i] = list->initial.element[i-1];
      list->initial.element[0].repcount = n;
      list->initial.element[0].presence = FCT_REQUIRED;
      list->initial.element[0].type = FAT_OBJECT;
      list->initial.count++;
      list->initial.length += n;

      normalize_outermost_list (list);
    }

  VERIFY_LIST (list);
}


/* ================= Intersection of two format_arg_lists ================= */

/* Create the intersection (i.e. combined constraints) of two argument
   constraints.  Return false if the intersection is empty, i.e. if the
   two constraints give a contradiction.  */
/* Memory effects: Freshly allocated element's sublist.  */
static bool
make_intersected_element (struct format_arg *re,
                          const struct format_arg * e1,
                          const struct format_arg * e2)
{
  /* Intersect the cdr types.  */
  if (e1->presence == FCT_REQUIRED || e2->presence == FCT_REQUIRED)
    re->presence = FCT_REQUIRED;
  else
    re->presence = FCT_OPTIONAL;

  /* Intersect the arg types.  */
  if (e1->type == FAT_OBJECT)
    {
      re->type = e2->type;
      if (re->type == FAT_LIST)
        re->list = copy_list (e2->list);
    }
  else if (e2->type == FAT_OBJECT)
    {
      re->type = e1->type;
      if (re->type == FAT_LIST)
        re->list = copy_list (e1->list);
    }
  else if (e1->type == FAT_LIST
           && (e2->type == FAT_CHARACTER_INTEGER_NULL
               || e2->type == FAT_CHARACTER_NULL
               || e2->type == FAT_INTEGER_NULL))
    {
      re->type = e1->type;
      re->list = make_intersection_with_empty_list (e1->list);
      if (re->list == NULL)
        return false;
    }
  else if (e2->type == FAT_LIST
           && (e1->type == FAT_CHARACTER_INTEGER_NULL
               || e1->type == FAT_CHARACTER_NULL
               || e1->type == FAT_INTEGER_NULL))
    {
      re->type = e2->type;
      re->list = make_intersection_with_empty_list (e2->list);
      if (re->list == NULL)
        return false;
    }
  else if (e1->type == FAT_CHARACTER_INTEGER_NULL
           && (e2->type == FAT_CHARACTER_NULL || e2->type == FAT_CHARACTER
               || e2->type == FAT_INTEGER_NULL || e2->type == FAT_INTEGER))
    {
      re->type = e2->type;
    }
  else if (e2->type == FAT_CHARACTER_INTEGER_NULL
           && (e1->type == FAT_CHARACTER_NULL || e1->type == FAT_CHARACTER
               || e1->type == FAT_INTEGER_NULL || e1->type == FAT_INTEGER))
    {
      re->type = e1->type;
    }
  else if (e1->type == FAT_CHARACTER_NULL && e2->type == FAT_CHARACTER)
    {
      re->type = e2->type;
    }
  else if (e2->type == FAT_CHARACTER_NULL && e1->type == FAT_CHARACTER)
    {
      re->type = e1->type;
    }
  else if (e1->type == FAT_INTEGER_NULL && e2->type == FAT_INTEGER)
    {
      re->type = e2->type;
    }
  else if (e2->type == FAT_INTEGER_NULL && e1->type == FAT_INTEGER)
    {
      re->type = e1->type;
    }
  else if (e1->type == FAT_REAL && e2->type == FAT_INTEGER)
    {
      re->type = e2->type;
    }
  else if (e2->type == FAT_REAL && e1->type == FAT_INTEGER)
    {
      re->type = e1->type;
    }
  else if (e1->type == e2->type)
    {
      re->type = e1->type;
      if (re->type == FAT_LIST)
        {
          re->list = make_intersected_list (copy_list (e1->list),
                                            copy_list (e2->list));
          if (re->list == NULL)
            return false;
        }
    }
  else
    /* Each of FAT_CHARACTER, FAT_INTEGER, FAT_LIST, FAT_FORMATSTRING,
       FAT_FUNCTION matches only itself.  Contradiction.  */
    return false;

  return true;
}

/* Append list->repeated to list->initial, and clear list->repeated.  */
/* Memory effects: list is destructively modified.  */
static void
append_repeated_to_initial (struct format_arg_list *list)
{
  if (list->repeated.count > 0)
    {
      /* Move list->repeated over to list->initial.  */
      unsigned int i, j, newcount;

      newcount = list->initial.count + list->repeated.count;
      ensure_initial_alloc (list, newcount);
      i = list->initial.count;
      for (j = 0; j < list->repeated.count; j++, i++)
        list->initial.element[i] = list->repeated.element[j];
      list->initial.count = newcount;
      list->initial.length = list->initial.length + list->repeated.length;
      free (list->repeated.element);
      list->repeated.element = NULL;
      list->repeated.allocated = 0;
      list->repeated.count = 0;
      list->repeated.length = 0;
    }
}

/* Handle a contradiction during building of a format_arg_list.
   The list consists only of an initial segment.  The repeated segment is
   empty.  This function searches the last FCT_OPTIONAL and cuts off the
   list at this point, or - if none is found - returns NULL.  */
/* Memory effects: list is destructively modified.  If NULL is returned,
   list is freed.  */
static struct format_arg_list *
backtrack_in_initial (struct format_arg_list *list)
{
  ASSERT (list->repeated.count == 0);

  while (list->initial.count > 0)
    {
      unsigned int i = list->initial.count - 1;
      if (list->initial.element[i].presence == FCT_REQUIRED)
        {
          /* Throw away this element.  */
          list->initial.length -= list->initial.element[i].repcount;
          free_element (&list->initial.element[i]);
          list->initial.count = i;
        }
      else /* list->initial.element[i].presence == FCT_OPTIONAL */
        {
          /* The list must end here.  */
          list->initial.length--;
          if (list->initial.element[i].repcount > 1)
            list->initial.element[i].repcount--;
          else
            {
              free_element (&list->initial.element[i]);
              list->initial.count = i;
            }
          VERIFY_LIST (list);
          return list;
        }
    }

  free_list (list);
  return NULL;
}

/* Create the intersection (i.e. combined constraints) of two argument list
   constraints.  Free both argument lists when done.  Return NULL if the
   intersection is empty, i.e. if the two constraints give a contradiction.  */
/* Memory effects: list1 and list2 are freed.  The result, if non-NULL, is
   freshly allocated.  */
static struct format_arg_list *
make_intersected_list (struct format_arg_list *list1,
                       struct format_arg_list *list2)
{
  struct format_arg_list *result;

  VERIFY_LIST (list1);
  VERIFY_LIST (list2);

  if (list1->repeated.length > 0 && list2->repeated.length > 0)
    /* Step 1: Ensure list1->repeated.length == list2->repeated.length.  */
    {
      unsigned int n1 = list1->repeated.length;
      unsigned int n2 = list2->repeated.length;
      unsigned int g = gcd (n1, n2);
      unsigned int m1 = n2 / g; /* = lcm(n1,n2) / n1 */
      unsigned int m2 = n1 / g; /* = lcm(n1,n2) / n2 */

      unfold_loop (list1, m1);
      unfold_loop (list2, m2);
      /* Now list1->repeated.length = list2->repeated.length = lcm(n1,n2).  */
    }

  if (list1->repeated.length > 0 || list2->repeated.length > 0)
    /* Step 2: Ensure the initial segment of the result can be computed
       from the initial segments of list1 and list2.  If both have a
       repeated segment, this means to ensure
       list1->initial.length == list2->initial.length.  */
    {
      unsigned int m = MAX (list1->initial.length, list2->initial.length);

      if (list1->repeated.length > 0)
        rotate_loop (list1, m);
      if (list2->repeated.length > 0)
        rotate_loop (list2, m);
    }

  if (list1->repeated.length > 0 && list2->repeated.length > 0)
    {
      ASSERT (list1->initial.length == list2->initial.length);
      ASSERT (list1->repeated.length == list2->repeated.length);
    }

  /* Step 3: Allocate the result.  */
  result = XMALLOC (struct format_arg_list);
  result->initial.count = 0;
  result->initial.allocated = 0;
  result->initial.element = NULL;
  result->initial.length = 0;
  result->repeated.count = 0;
  result->repeated.allocated = 0;
  result->repeated.element = NULL;
  result->repeated.length = 0;

  /* Step 4: Elementwise intersection of list1->initial, list2->initial.  */
  {
    struct format_arg *e1;
    struct format_arg *e2;
    unsigned int c1;
    unsigned int c2;

    e1 = list1->initial.element; c1 = list1->initial.count;
    e2 = list2->initial.element; c2 = list2->initial.count;
    while (c1 > 0 && c2 > 0)
      {
        struct format_arg *re;

        /* Ensure room in result->initial.  */
        grow_initial_alloc (result);
        re = &result->initial.element[result->initial.count];
        re->repcount = MIN (e1->repcount, e2->repcount);

        /* Intersect the argument types.  */
        if (!make_intersected_element (re, e1, e2))
          {
            /* If re->presence == FCT_OPTIONAL, the result list ends here.  */
            if (re->presence == FCT_REQUIRED)
              /* Contradiction.  Backtrack.  */
              result = backtrack_in_initial (result);
            goto done;
          }

        result->initial.count++;
        result->initial.length += re->repcount;

        e1->repcount -= re->repcount;
        if (e1->repcount == 0)
          {
            e1++;
            c1--;
          }
        e2->repcount -= re->repcount;
        if (e2->repcount == 0)
          {
            e2++;
            c2--;
          }
      }

    if (list1->repeated.count == 0 && list2->repeated.count == 0)
      {
        /* Intersecting two finite lists.  */
        if (c1 > 0)
          {
            /* list1 longer than list2.  */
            if (e1->presence == FCT_REQUIRED)
              /* Contradiction.  Backtrack.  */
              result = backtrack_in_initial (result);
          }
        else if (c2 > 0)
          {
            /* list2 longer than list1.  */
            if (e2->presence == FCT_REQUIRED)
              /* Contradiction.  Backtrack.  */
              result = backtrack_in_initial (result);
          }
        goto done;
      }
    else if (list1->repeated.count == 0)
      {
        /* Intersecting a finite and an infinite list.  */
        ASSERT (c1 == 0);
        if ((c2 > 0 ? e2->presence : list2->repeated.element[0].presence)
            == FCT_REQUIRED)
          /* Contradiction.  Backtrack.  */
          result = backtrack_in_initial (result);
        goto done;
      }
    else if (list2->repeated.count == 0)
      {
        /* Intersecting an infinite and a finite list.  */
        ASSERT (c2 == 0);
        if ((c1 > 0 ? e1->presence : list1->repeated.element[0].presence)
            == FCT_REQUIRED)
          /* Contradiction.  Backtrack.  */
          result = backtrack_in_initial (result);
        goto done;
      }
    /* Intersecting two infinite lists.  */
    ASSERT (c1 == 0 && c2 == 0);
  }

  /* Step 5: Elementwise intersection of list1->repeated, list2->repeated.  */
  {
    struct format_arg *e1;
    struct format_arg *e2;
    unsigned int c1;
    unsigned int c2;

    e1 = list1->repeated.element; c1 = list1->repeated.count;
    e2 = list2->repeated.element; c2 = list2->repeated.count;
    while (c1 > 0 && c2 > 0)
      {
        struct format_arg *re;

        /* Ensure room in result->repeated.  */
        grow_repeated_alloc (result);
        re = &result->repeated.element[result->repeated.count];
        re->repcount = MIN (e1->repcount, e2->repcount);

        /* Intersect the argument types.  */
        if (!make_intersected_element (re, e1, e2))
          {
            bool re_is_required = re->presence == FCT_REQUIRED;

            append_repeated_to_initial (result);

            /* If re->presence == FCT_OPTIONAL, the result list ends here.  */
            if (re_is_required)
              /* Contradiction.  Backtrack.  */
              result = backtrack_in_initial (result);

            goto done;
          }

        result->repeated.count++;
        result->repeated.length += re->repcount;

        e1->repcount -= re->repcount;
        if (e1->repcount == 0)
          {
            e1++;
            c1--;
          }
        e2->repcount -= re->repcount;
        if (e2->repcount == 0)
          {
            e2++;
            c2--;
          }
      }
    ASSERT (c1 == 0 && c2 == 0);
  }

 done:
  free_list (list1);
  free_list (list2);
  if (result != NULL)
    {
      /* Undo the loop unfolding and unrolling done above.  */
      normalize_outermost_list (result);
      VERIFY_LIST (result);
    }
  return result;
}


/* Create the intersection of an argument list and the empty list.
   Return NULL if the intersection is empty.  */
/* Memory effects: The result, if non-NULL, is freshly allocated.  */
static struct format_arg_list *
make_intersection_with_empty_list (struct format_arg_list *list)
{
#if 0 /* equivalent but slower */
  return make_intersected_list (copy_list (list), make_empty_list ());
#else
  if (list->initial.count > 0
      ? list->initial.element[0].presence == FCT_REQUIRED
      : list->repeated.count > 0
        && list->repeated.element[0].presence == FCT_REQUIRED)
    return NULL;
  else
    return make_empty_list ();
#endif
}


#ifdef unused
/* Create the intersection of two argument list constraints.  NULL stands
   for an impossible situation, i.e. a contradiction.  */
/* Memory effects: list1 and list2 are freed if non-NULL.  The result,
   if non-NULL, is freshly allocated.  */
static struct format_arg_list *
intersection (struct format_arg_list *list1, struct format_arg_list *list2)
{
  if (list1 != NULL)
    {
      if (list2 != NULL)
        return make_intersected_list (list1, list2);
      else
        {
          free_list (list1);
          return NULL;
        }
    }
  else
    {
      if (list2 != NULL)
        {
          free_list (list2);
          return NULL;
        }
      else
        return NULL;
    }
}
#endif


/* ===================== Union of two format_arg_lists ===================== */

/* Create the union (i.e. alternative constraints) of two argument
   constraints.  */
static void
make_union_element (struct format_arg *re,
                    const struct format_arg * e1,
                    const struct format_arg * e2)
{
  /* Union of the cdr types.  */
  if (e1->presence == FCT_REQUIRED && e2->presence == FCT_REQUIRED)
    re->presence = FCT_REQUIRED;
  else /* Either one of them is FCT_OPTIONAL.  */
    re->presence = FCT_OPTIONAL;

  /* Union of the arg types.  */
  if (e1->type == e2->type)
    {
      re->type = e1->type;
      if (re->type == FAT_LIST)
        re->list = make_union_list (copy_list (e1->list),
                                    copy_list (e2->list));
    }
  else if (e1->type == FAT_CHARACTER_INTEGER_NULL
           && (e2->type == FAT_CHARACTER_NULL || e2->type == FAT_CHARACTER
               || e2->type == FAT_INTEGER_NULL || e2->type == FAT_INTEGER))
    {
      re->type = e1->type;
    }
  else if (e2->type == FAT_CHARACTER_INTEGER_NULL
           && (e1->type == FAT_CHARACTER_NULL || e1->type == FAT_CHARACTER
               || e1->type == FAT_INTEGER_NULL || e1->type == FAT_INTEGER))
    {
      re->type = e2->type;
    }
  else if (e1->type == FAT_CHARACTER_NULL && e2->type == FAT_CHARACTER)
    {
      re->type = e1->type;
    }
  else if (e2->type == FAT_CHARACTER_NULL && e1->type == FAT_CHARACTER)
    {
      re->type = e2->type;
    }
  else if (e1->type == FAT_INTEGER_NULL && e2->type == FAT_INTEGER)
    {
      re->type = e1->type;
    }
  else if (e2->type == FAT_INTEGER_NULL && e1->type == FAT_INTEGER)
    {
      re->type = e2->type;
    }
  else if (e1->type == FAT_REAL && e2->type == FAT_INTEGER)
    {
      re->type = e1->type;
    }
  else if (e2->type == FAT_REAL && e1->type == FAT_INTEGER)
    {
      re->type = e2->type;
    }
  else if (e1->type == FAT_LIST && is_empty_list (e1->list))
    {
      if (e2->type == FAT_CHARACTER_INTEGER_NULL
          || e2->type == FAT_CHARACTER_NULL
          || e2->type == FAT_INTEGER_NULL)
        re->type = e2->type;
      else if (e2->type == FAT_CHARACTER)
        re->type = FAT_CHARACTER_NULL;
      else if (e2->type == FAT_INTEGER)
        re->type = FAT_INTEGER_NULL;
      else
        re->type = FAT_OBJECT;
    }
  else if (e2->type == FAT_LIST && is_empty_list (e2->list))
    {
      if (e1->type == FAT_CHARACTER_INTEGER_NULL
          || e1->type == FAT_CHARACTER_NULL
          || e1->type == FAT_INTEGER_NULL)
        re->type = e1->type;
      else if (e1->type == FAT_CHARACTER)
        re->type = FAT_CHARACTER_NULL;
      else if (e1->type == FAT_INTEGER)
        re->type = FAT_INTEGER_NULL;
      else
        re->type = FAT_OBJECT;
    }
  else if ((e1->type == FAT_CHARACTER || e1->type == FAT_CHARACTER_NULL)
           && (e2->type == FAT_INTEGER || e2->type == FAT_INTEGER_NULL))
    {
      re->type = FAT_CHARACTER_INTEGER_NULL;
    }
  else if ((e2->type == FAT_CHARACTER || e2->type == FAT_CHARACTER_NULL)
           && (e1->type == FAT_INTEGER || e1->type == FAT_INTEGER_NULL))
    {
      re->type = FAT_CHARACTER_INTEGER_NULL;
    }
  else
    {
      /* Other union types are too hard to describe precisely.  */
      re->type = FAT_OBJECT;
    }
}

/* Create the union (i.e. alternative constraints) of two argument list
   constraints.  Free both argument lists when done.  */
/* Memory effects: list1 and list2 are freed.  The result is freshly
   allocated.  */
static struct format_arg_list *
make_union_list (struct format_arg_list *list1, struct format_arg_list *list2)
{
  struct format_arg_list *result;

  VERIFY_LIST (list1);
  VERIFY_LIST (list2);

  if (list1->repeated.length > 0 && list2->repeated.length > 0)
    {
      /* Step 1: Ensure list1->repeated.length == list2->repeated.length.  */
      {
        unsigned int n1 = list1->repeated.length;
        unsigned int n2 = list2->repeated.length;
        unsigned int g = gcd (n1, n2);
        unsigned int m1 = n2 / g; /* = lcm(n1,n2) / n1 */
        unsigned int m2 = n1 / g; /* = lcm(n1,n2) / n2 */

        unfold_loop (list1, m1);
        unfold_loop (list2, m2);
        /* Now list1->repeated.length = list2->repeated.length = lcm(n1,n2).  */
      }

      /* Step 2: Ensure that list1->initial.length == list2->initial.length.  */
      {
        unsigned int m = MAX (list1->initial.length, list2->initial.length);

        rotate_loop (list1, m);
        rotate_loop (list2, m);
      }

      ASSERT (list1->initial.length == list2->initial.length);
      ASSERT (list1->repeated.length == list2->repeated.length);
    }
  else if (list1->repeated.length > 0)
    {
      /* Ensure the initial segment of the result can be computed from the
         initial segment of list1.  */
      if (list2->initial.length >= list1->initial.length)
        {
          rotate_loop (list1, list2->initial.length);
          if (list1->repeated.element[0].presence == FCT_REQUIRED)
            rotate_loop (list1, list1->initial.length + 1);
        }
    }
  else if (list2->repeated.length > 0)
    {
      /* Ensure the initial segment of the result can be computed from the
         initial segment of list2.  */
      if (list1->initial.length >= list2->initial.length)
        {
          rotate_loop (list2, list1->initial.length);
          if (list2->repeated.element[0].presence == FCT_REQUIRED)
            rotate_loop (list2, list2->initial.length + 1);
        }
    }

  /* Step 3: Allocate the result.  */
  result = XMALLOC (struct format_arg_list);
  result->initial.count = 0;
  result->initial.allocated = 0;
  result->initial.element = NULL;
  result->initial.length = 0;
  result->repeated.count = 0;
  result->repeated.allocated = 0;
  result->repeated.element = NULL;
  result->repeated.length = 0;

  /* Step 4: Elementwise union of list1->initial, list2->initial.  */
  {
    struct format_arg *e1;
    struct format_arg *e2;
    unsigned int c1;
    unsigned int c2;

    e1 = list1->initial.element; c1 = list1->initial.count;
    e2 = list2->initial.element; c2 = list2->initial.count;
    while (c1 > 0 && c2 > 0)
      {
        struct format_arg *re;

        /* Ensure room in result->initial.  */
        grow_initial_alloc (result);
        re = &result->initial.element[result->initial.count];
        re->repcount = MIN (e1->repcount, e2->repcount);

        /* Union of the argument types.  */
        make_union_element (re, e1, e2);

        result->initial.count++;
        result->initial.length += re->repcount;

        e1->repcount -= re->repcount;
        if (e1->repcount == 0)
          {
            e1++;
            c1--;
          }
        e2->repcount -= re->repcount;
        if (e2->repcount == 0)
          {
            e2++;
            c2--;
          }
       }

    if (c1 > 0)
      {
        /* list2 already terminated, but still more elements in list1->initial.
           Copy them all, but turn the first presence to FCT_OPTIONAL.  */
        ASSERT (list2->repeated.count == 0);

        if (e1->presence == FCT_REQUIRED)
          {
            struct format_arg *re;

            /* Ensure room in result->initial.  */
            grow_initial_alloc (result);
            re = &result->initial.element[result->initial.count];
            copy_element (re, e1);
            re->presence = FCT_OPTIONAL;
            re->repcount = 1;
            result->initial.count++;
            result->initial.length += 1;
            e1->repcount -= 1;
            if (e1->repcount == 0)
              {
                e1++;
                c1--;
              }
          }

        /* Ensure room in result->initial.  */
        ensure_initial_alloc (result, result->initial.count + c1);
        while (c1 > 0)
          {
            struct format_arg *re;

            re = &result->initial.element[result->initial.count];
            copy_element (re, e1);
            result->initial.count++;
            result->initial.length += re->repcount;
            e1++;
            c1--;
          }
      }
    else if (c2 > 0)
      {
        /* list1 already terminated, but still more elements in list2->initial.
           Copy them all, but turn the first presence to FCT_OPTIONAL.  */
        ASSERT (list1->repeated.count == 0);

        if (e2->presence == FCT_REQUIRED)
          {
            struct format_arg *re;

            /* Ensure room in result->initial.  */
            grow_initial_alloc (result);
            re = &result->initial.element[result->initial.count];
            copy_element (re, e2);
            re->presence = FCT_OPTIONAL;
            re->repcount = 1;
            result->initial.count++;
            result->initial.length += 1;
            e2->repcount -= 1;
            if (e2->repcount == 0)
              {
                e2++;
                c2--;
              }
          }

        /* Ensure room in result->initial.  */
        ensure_initial_alloc (result, result->initial.count + c2);
        while (c2 > 0)
          {
            struct format_arg *re;

            re = &result->initial.element[result->initial.count];
            copy_element (re, e2);
            result->initial.count++;
            result->initial.length += re->repcount;
            e2++;
            c2--;
          }
      }
    ASSERT (c1 == 0 && c2 == 0);
  }

  if (list1->repeated.length > 0 && list2->repeated.length > 0)
    /* Step 5: Elementwise union of list1->repeated, list2->repeated.  */
    {
      struct format_arg *e1;
      struct format_arg *e2;
      unsigned int c1;
      unsigned int c2;

      e1 = list1->repeated.element; c1 = list1->repeated.count;
      e2 = list2->repeated.element; c2 = list2->repeated.count;
      while (c1 > 0 && c2 > 0)
        {
          struct format_arg *re;

          /* Ensure room in result->repeated.  */
          grow_repeated_alloc (result);
          re = &result->repeated.element[result->repeated.count];
          re->repcount = MIN (e1->repcount, e2->repcount);

          /* Union of the argument types.  */
          make_union_element (re, e1, e2);

          result->repeated.count++;
          result->repeated.length += re->repcount;

          e1->repcount -= re->repcount;
          if (e1->repcount == 0)
            {
              e1++;
              c1--;
            }
          e2->repcount -= re->repcount;
          if (e2->repcount == 0)
            {
              e2++;
              c2--;
            }
        }
      ASSERT (c1 == 0 && c2 == 0);
    }
  else if (list1->repeated.length > 0)
    {
      /* Turning FCT_REQUIRED into FCT_OPTIONAL was already handled in the
         initial segment.  Just copy the repeated segment of list1.  */
      unsigned int i;

      result->repeated.count = list1->repeated.count;
      result->repeated.allocated = result->repeated.count;
      result->repeated.element =
        XNMALLOC (result->repeated.allocated, struct format_arg);
      for (i = 0; i < list1->repeated.count; i++)
        copy_element (&result->repeated.element[i],
                      &list1->repeated.element[i]);
      result->repeated.length = list1->repeated.length;
    }
  else if (list2->repeated.length > 0)
    {
      /* Turning FCT_REQUIRED into FCT_OPTIONAL was already handled in the
         initial segment.  Just copy the repeated segment of list2.  */
      unsigned int i;

      result->repeated.count = list2->repeated.count;
      result->repeated.allocated = result->repeated.count;
      result->repeated.element =
        XNMALLOC (result->repeated.allocated, struct format_arg);
      for (i = 0; i < list2->repeated.count; i++)
        copy_element (&result->repeated.element[i],
                      &list2->repeated.element[i]);
      result->repeated.length = list2->repeated.length;
    }

  free_list (list1);
  free_list (list2);
  /* Undo the loop unfolding and unrolling done above.  */
  normalize_outermost_list (result);
  VERIFY_LIST (result);
  return result;
}


/* Create the union of an argument list and the empty list.  */
/* Memory effects: list is freed.  The result is freshly allocated.  */
static struct format_arg_list *
make_union_with_empty_list (struct format_arg_list *list)
{
#if 0 /* equivalent but slower */
  return make_union_list (list, make_empty_list ());
#else
  VERIFY_LIST (list);

  if (list->initial.count > 0
      ? list->initial.element[0].presence == FCT_REQUIRED
      : list->repeated.count > 0
        && list->repeated.element[0].presence == FCT_REQUIRED)
    {
      initial_splitelement (list, 1);
      ASSERT (list->initial.count > 0);
      ASSERT (list->initial.element[0].repcount == 1);
      ASSERT (list->initial.element[0].presence == FCT_REQUIRED);
      list->initial.element[0].presence = FCT_OPTIONAL;

      /* We might need to merge list->initial.element[0] and
         list->initial.element[1].  */
      normalize_outermost_list (list);
    }

  VERIFY_LIST (list);

  return list;
#endif
}


/* Create the union of two argument list constraints.  NULL stands for an
   impossible situation, i.e. a contradiction.  */
/* Memory effects: list1 and list2 are freed if non-NULL.  The result,
   if non-NULL, is freshly allocated.  */
static struct format_arg_list *
union (struct format_arg_list *list1, struct format_arg_list *list2)
{
  if (list1 != NULL)
    {
      if (list2 != NULL)
        return make_union_list (list1, list2);
      else
        return list1;
    }
  else
    {
      if (list2 != NULL)
        return list2;
      else
        return NULL;
    }
}


/* =========== Adding specific constraints to a format_arg_list =========== */


/* Test whether arguments 0..n are required arguments in a list.  */
static bool
is_required (const struct format_arg_list *list, unsigned int n)
{
  unsigned int s;
  unsigned int t;

  /* We'll check whether the first n+1 presence flags are FCT_REQUIRED.  */
  t = n + 1;

  /* Walk the list->initial segment.  */
  for (s = 0;
       s < list->initial.count && t >= list->initial.element[s].repcount;
       t -= list->initial.element[s].repcount, s++)
    if (list->initial.element[s].presence != FCT_REQUIRED)
      return false;

  if (t == 0)
    return true;

  if (s < list->initial.count)
    {
      if (list->initial.element[s].presence != FCT_REQUIRED)
        return false;
      else
        return true;
    }

  /* Walk the list->repeated segment.  */
  if (list->repeated.count == 0)
    return false;

  for (s = 0;
       s < list->repeated.count && t >= list->repeated.element[s].repcount;
       t -= list->repeated.element[s].repcount, s++)
    if (list->repeated.element[s].presence != FCT_REQUIRED)
      return false;

  if (t == 0)
    return true;

  if (s < list->repeated.count)
    {
      if (list->repeated.element[s].presence != FCT_REQUIRED)
        return false;
      else
        return true;
    }

  /* The list->repeated segment consists only of FCT_REQUIRED.  So,
     regardless how many more passes through list->repeated would be
     needed until t becomes 0, the result is true.  */
  return true;
}


/* Add a constraint to an argument list, namely that the arguments 0...n are
   present.  NULL stands for an impossible situation, i.e. a contradiction.  */
/* Memory effects: list is freed.  The result is freshly allocated.  */
static struct format_arg_list *
add_required_constraint (struct format_arg_list *list, unsigned int n)
{
  unsigned int i, rest;

  if (list == NULL)
    return NULL;

  VERIFY_LIST (list);

  if (list->repeated.count == 0 && list->initial.length <= n)
    {
      /* list is already constrained to have at most length n.
         Contradiction.  */
      free_list (list);
      return NULL;
    }

  initial_splitelement (list, n + 1);

  for (i = 0, rest = n + 1; rest > 0; )
    {
      list->initial.element[i].presence = FCT_REQUIRED;
      rest -= list->initial.element[i].repcount;
      i++;
    }

  VERIFY_LIST (list);

  return list;
}


/* Add a constraint to an argument list, namely that the argument n is
   never present.  NULL stands for an impossible situation, i.e. a
   contradiction.  */
/* Memory effects: list is freed.  The result is freshly allocated.  */
static struct format_arg_list *
add_end_constraint (struct format_arg_list *list, unsigned int n)
{
  unsigned int s, i;
  enum format_cdr_type n_presence;

  if (list == NULL)
    return NULL;

  VERIFY_LIST (list);

  if (list->repeated.count == 0 && list->initial.length <= n)
    /* list is already constrained to have at most length n.  */
    return list;

  s = initial_splitelement (list, n);
  n_presence =
    (s < list->initial.count
     ? /* n < list->initial.length */ list->initial.element[s].presence
     : /* n >= list->initial.length */ list->repeated.element[0].presence);

  for (i = s; i < list->initial.count; i++)
    {
      list->initial.length -= list->initial.element[i].repcount;
      free_element (&list->initial.element[i]);
    }
  list->initial.count = s;

  for (i = 0; i < list->repeated.count; i++)
    free_element (&list->repeated.element[i]);
  if (list->repeated.element != NULL)
    free (list->repeated.element);
  list->repeated.element = NULL;
  list->repeated.allocated = 0;
  list->repeated.count = 0;
  list->repeated.length = 0;

  if (n_presence == FCT_REQUIRED)
    return backtrack_in_initial (list);
  else
    return list;
}


/* Add a constraint to an argument list, namely that the argument n is
   of a given type.  NULL stands for an impossible situation, i.e. a
   contradiction.  Assumes a preceding add_required_constraint (list, n).  */
/* Memory effects: list is freed.  The result is freshly allocated.  */
static struct format_arg_list *
add_type_constraint (struct format_arg_list *list, unsigned int n,
                     enum format_arg_type type)
{
  unsigned int s;
  struct format_arg newconstraint;
  struct format_arg tmpelement;

  if (list == NULL)
    return NULL;

  /* Through the previous add_required_constraint, we can assume
     list->initial.length >= n+1.  */

  s = initial_unshare (list, n);

  newconstraint.presence = FCT_OPTIONAL;
  newconstraint.type = type;
  if (!make_intersected_element (&tmpelement,
                                 &list->initial.element[s], &newconstraint))
    list = add_end_constraint (list, n);
  else
    {
      free_element (&list->initial.element[s]);
      list->initial.element[s].type = tmpelement.type;
      list->initial.element[s].list = tmpelement.list;
    }

  if (list != NULL)
    VERIFY_LIST (list);

  return list;
}


/* Add a constraint to an argument list, namely that the argument n is
   of a given list type.  NULL stands for an impossible situation, i.e. a
   contradiction.  Assumes a preceding add_required_constraint (list, n).  */
/* Memory effects: list is freed.  The result is freshly allocated.  */
static struct format_arg_list *
add_listtype_constraint (struct format_arg_list *list, unsigned int n,
                         enum format_arg_type type,
                         struct format_arg_list *sublist)
{
  unsigned int s;
  struct format_arg newconstraint;
  struct format_arg tmpelement;

  if (list == NULL)
    return NULL;

  /* Through the previous add_required_constraint, we can assume
     list->initial.length >= n+1.  */

  s = initial_unshare (list, n);

  newconstraint.presence = FCT_OPTIONAL;
  newconstraint.type = type;
  newconstraint.list = sublist;
  if (!make_intersected_element (&tmpelement,
                                 &list->initial.element[s], &newconstraint))
    list = add_end_constraint (list, n);
  else
    {
      free_element (&list->initial.element[s]);
      list->initial.element[s].type = tmpelement.type;
      list->initial.element[s].list = tmpelement.list;
    }

  if (list != NULL)
    VERIFY_LIST (list);

  return list;
}


/* ============= Subroutines used by the format string parser ============= */

static void
add_req_type_constraint (struct format_arg_list **listp,
                         unsigned int position, enum format_arg_type type)
{
  *listp = add_required_constraint (*listp, position);
  *listp = add_type_constraint (*listp, position, type);
}


static void
add_req_listtype_constraint (struct format_arg_list **listp,
                             unsigned int position, enum format_arg_type type,
                             struct format_arg_list *sublist)
{
  *listp = add_required_constraint (*listp, position);
  *listp = add_listtype_constraint (*listp, position, type, sublist);
}


/* Create an endless repeated list whose elements are lists constrained
   by sublist.  */
/* Memory effects: sublist is freed.  The result is freshly allocated.  */
static struct format_arg_list *
make_repeated_list_of_lists (struct format_arg_list *sublist)
{
  if (sublist == NULL)
    /* The list cannot have a single element.  */
    return make_empty_list ();
  else
    {
      struct format_arg_list *listlist;

      listlist = XMALLOC (struct format_arg_list);

      listlist->initial.count = 0;
      listlist->initial.allocated = 0;
      listlist->initial.element = NULL;
      listlist->initial.length = 0;
      listlist->repeated.count = 1;
      listlist->repeated.allocated = 1;
      listlist->repeated.element = XNMALLOC (1, struct format_arg);
      listlist->repeated.element[0].repcount = 1;
      listlist->repeated.element[0].presence = FCT_OPTIONAL;
      listlist->repeated.element[0].type = FAT_LIST;
      listlist->repeated.element[0].list = sublist;
      listlist->repeated.length = 1;

      VERIFY_LIST (listlist);

      return listlist;
    }
}


/* Create an endless repeated list which represents the union of a finite
   number of copies of L, each time shifted by period:
     ()
     L
     L and (*^period L)
     L and (*^period L) and (*^{2 period} L)
     L and (*^period L) and (*^{2 period} L) and (*^{3 period} L)
     ...
 */
/* Memory effects: sublist is freed.  The result is freshly allocated.  */
static struct format_arg_list *
make_repeated_list (struct format_arg_list *sublist, unsigned int period)
{
  struct segment tmp;
  struct segment *srcseg;
  struct format_arg_list *list;
  unsigned int p, n, i, si, ti, j, sj, tj, splitindex, newcount;
  bool ended;

  VERIFY_LIST (sublist);

  ASSERT (period > 0);

  if (sublist->repeated.count == 0)
    {
      /* L is a finite list.  */

      if (sublist->initial.length < period)
        /* L and (*^period L) is a contradition, so we need to consider
           only 1 and 0 iterations.  */
        return make_union_with_empty_list (sublist);

      srcseg = &sublist->initial;
      p = period;
    }
  else
    {
      /* L is an infinite list.  */
      /* p := lcm (period, period of L)  */
      unsigned int Lp = sublist->repeated.length;
      unsigned int m = period / gcd (period, Lp); /* = lcm(period,Lp) / Lp */

      unfold_loop (sublist, m);
      p = m * Lp;

      /* Concatenate the initial and the repeated segments into a single
         segment.  */
      tmp.count = sublist->initial.count + sublist->repeated.count;
      tmp.allocated = tmp.count;
      tmp.element = XNMALLOC (tmp.allocated, struct format_arg);
      for (i = 0; i < sublist->initial.count; i++)
        tmp.element[i] = sublist->initial.element[i];
      for (j = 0; j < sublist->repeated.count; i++, j++)
        tmp.element[i] = sublist->repeated.element[j];
      tmp.length = sublist->initial.length + sublist->repeated.length;

      srcseg = &tmp;
    }

  n = srcseg->length;

  /* Example: n = 7, p = 2
     Let L = (A B C D E F G).

     L                 =    A     B     C     D      E      F      G
     L & L<<p          =    A     B    C&A   D&B    E&C    F&D    G&E
     L & L<<p & L<<2p  =    A     B    C&A   D&B   E&C&A  F&D&B  G&E&C
     ...               =    A     B    C&A   D&B   E&C&A  F&D&B G&E&C&A

     Thus the result has an initial segment of length n - p and a period
     of p, and can be computed by floor(n/p) intersection operations.
     Or by a single incremental intersection operation, going from left
     to right.  */

  list = XMALLOC (struct format_arg_list);
  list->initial.count = 0;
  list->initial.allocated = 0;
  list->initial.element = NULL;
  list->initial.length = 0;
  list->repeated.count = 0;
  list->repeated.allocated = 0;
  list->repeated.element = NULL;
  list->repeated.length = 0;

  /* Sketch:
     for (i = 0; i < p; i++)
       list->initial.element[i] = srcseg->element[i];
     list->initial.element[0].presence = FCT_OPTIONAL;  // union with empty list
     for (i = p, j = 0; i < n; i++, j++)
       list->initial.element[i] = srcseg->element[i] & list->initial.element[j];
   */

  ended = false;

  i = 0, ti = 0, si = 0;
  while (i < p)
    {
      unsigned int k = MIN (srcseg->element[si].repcount - ti, p - i);

      /* Ensure room in list->initial.  */
      grow_initial_alloc (list);
      copy_element (&list->initial.element[list->initial.count],
                    &srcseg->element[si]);
      list->initial.element[list->initial.count].repcount = k;
      list->initial.count++;
      list->initial.length += k;

      i += k;
      ti += k;
      if (ti == srcseg->element[si].repcount)
        {
          ti = 0;
          si++;
        }
    }

  ASSERT (list->initial.count > 0);
  if (list->initial.element[0].presence == FCT_REQUIRED)
    {
      initial_splitelement (list, 1);
      ASSERT (list->initial.element[0].presence == FCT_REQUIRED);
      ASSERT (list->initial.element[0].repcount == 1);
      list->initial.element[0].presence = FCT_OPTIONAL;
    }

  j = 0, tj = 0, sj = 0;
  while (i < n)
    {
      unsigned int k =
        MIN (srcseg->element[si].repcount - ti,
             list->initial.element[sj].repcount - tj);

      /* Ensure room in list->initial.  */
      grow_initial_alloc (list);
      if (!make_intersected_element (&list->initial.element[list->initial.count],
                                     &srcseg->element[si],
                                     &list->initial.element[sj]))
        {
          if (list->initial.element[list->initial.count].presence == FCT_REQUIRED)
            {
              /* Contradiction.  Backtrack.  */
              list = backtrack_in_initial (list);
              ASSERT (list != NULL); /* at least the empty list is valid */
              return list;
            }
          else
            {
              /* The list ends here.  */
              ended = true;
              break;
            }
        }
      list->initial.element[list->initial.count].repcount = k;
      list->initial.count++;
      list->initial.length += k;

      i += k;
      ti += k;
      if (ti == srcseg->element[si].repcount)
        {
          ti = 0;
          si++;
        }

      j += k;
      tj += k;
      if (tj == list->initial.element[sj].repcount)
        {
          tj = 0;
          sj++;
        }
    }
  if (!ended)
    ASSERT (list->initial.length == n);

  /* Add optional exit points at 0, period, 2*period etc.
     FIXME: Not sure this is correct in all cases.  */
  for (i = 0; i < list->initial.length; i += period)
    {
      si = initial_unshare (list, i);
      list->initial.element[si].presence = FCT_OPTIONAL;
    }

  if (!ended)
    {
      /* Now split off the repeated part.  */
      splitindex = initial_splitelement (list, n - p);
      newcount = list->initial.count - splitindex;
      if (newcount > list->repeated.allocated)
        {
          list->repeated.allocated = newcount;
          list->repeated.element = XNMALLOC (newcount, struct format_arg);
        }
      for (i = splitindex, j = 0; j < newcount; i++, j++)
        list->repeated.element[j] = list->initial.element[i];
      list->repeated.count = newcount;
      list->repeated.length = p;
      list->initial.count = splitindex;
      list->initial.length = n - p;
    }

  VERIFY_LIST (list);

  return list;
}


/* ================= Handling of format string directives ================= */

/* Possible signatures of format directives.  */
static const enum format_arg_type I [1] = { FAT_INTEGER_NULL };
static const enum format_arg_type II [2] = {
  FAT_INTEGER_NULL, FAT_INTEGER_NULL
};
static const enum format_arg_type ICCI [4] = {
  FAT_INTEGER_NULL, FAT_CHARACTER_NULL, FAT_CHARACTER_NULL, FAT_INTEGER_NULL
};
static const enum format_arg_type IIIC [4] = {
  FAT_INTEGER_NULL, FAT_INTEGER_NULL, FAT_INTEGER_NULL, FAT_CHARACTER_NULL
};
static const enum format_arg_type IICCI [5] = {
  FAT_INTEGER_NULL, FAT_INTEGER_NULL, FAT_CHARACTER_NULL, FAT_CHARACTER_NULL,
  FAT_INTEGER_NULL
};
static const enum format_arg_type IIICC [5] = {
  FAT_INTEGER_NULL, FAT_INTEGER_NULL, FAT_INTEGER_NULL, FAT_CHARACTER_NULL,
  FAT_CHARACTER_NULL
};
static const enum format_arg_type IIIICCC [7] = {
  FAT_INTEGER_NULL, FAT_INTEGER_NULL, FAT_INTEGER_NULL, FAT_INTEGER_NULL,
  FAT_CHARACTER_NULL, FAT_CHARACTER_NULL, FAT_CHARACTER_NULL
};
static const enum format_arg_type THREE [3] = {
  FAT_CHARACTER_INTEGER_NULL, FAT_CHARACTER_INTEGER_NULL,
  FAT_CHARACTER_INTEGER_NULL
};


/* Check the parameters.  For V params, add the constraint to the argument
   list.  Return false and fill in *invalid_reason if the format string is
   invalid.  */
static bool
check_params (struct format_arg_list **listp,
              unsigned int paramcount, struct param *params,
              unsigned int t_count, const enum format_arg_type *t_types,
              unsigned int directives, char **invalid_reason)
{
  unsigned int orig_paramcount = paramcount;
  unsigned int orig_t_count = t_count;

  for (; paramcount > 0 && t_count > 0;
         params++, paramcount--, t_types++, t_count--)
    {
      switch (*t_types)
        {
        case FAT_CHARACTER_INTEGER_NULL:
          break;
        case FAT_CHARACTER_NULL:
          switch (params->type)
            {
            case PT_NIL: case PT_CHARACTER: case PT_V:
              break;
            case PT_INTEGER: case PT_ARGCOUNT:
              /* wrong param type */
              *invalid_reason =
                xasprintf (_("In the directive number %u, parameter %u is of type '%s' but a parameter of type '%s' is expected."), directives, orig_paramcount - paramcount + 1, "integer", "character");
              return false;
            }
          break;
        case FAT_INTEGER_NULL:
          switch (params->type)
            {
            case PT_NIL: case PT_INTEGER: case PT_ARGCOUNT: case PT_V:
              break;
            case PT_CHARACTER:
              /* wrong param type */
              *invalid_reason =
                xasprintf (_("In the directive number %u, parameter %u is of type '%s' but a parameter of type '%s' is expected."), directives, orig_paramcount - paramcount + 1, "character", "integer");
              return false;
            }
          break;
        default:
          abort ();
        }
      if (params->type == PT_V)
        {
          int position = params->value;
          if (position >= 0)
            add_req_type_constraint (listp, position, *t_types);
        }
    }

  for (; paramcount > 0; params++, paramcount--)
    switch (params->type)
      {
      case PT_NIL:
        break;
      case PT_CHARACTER: case PT_INTEGER: case PT_ARGCOUNT:
        /* too many params for directive */
        *invalid_reason =
          xasprintf (ngettext ("In the directive number %u, too many parameters are given; expected at most %u parameter.",
                               "In the directive number %u, too many parameters are given; expected at most %u parameters.",
                               orig_t_count),
                     directives, orig_t_count);
        return false;
      case PT_V:
        /* Force argument to be NIL.  */
        {
          int position = params->value;
          if (position >= 0)
            {
              struct format_arg_list *empty_list = make_empty_list ();
              add_req_listtype_constraint (listp, position,
                                           FAT_LIST, empty_list);
              free_list (empty_list);
            }
        }
        break;
      }

  return true;
}


/* Handle the parameters, without a priori type information.
   For V params, add the constraint to the argument list.
   Return false and fill in *invalid_reason if the format string is
   invalid.  */
static bool
nocheck_params (struct format_arg_list **listp,
                unsigned int paramcount, struct param *params,
                unsigned int directives, char **invalid_reason)
{
  (void) directives;
  (void) invalid_reason;

  for (; paramcount > 0; params++, paramcount--)
    if (params->type == PT_V)
      {
        int position = params->value;
        if (position >= 0)
          add_req_type_constraint (listp, position, FAT_CHARACTER_INTEGER_NULL);
      }

  return true;
}


/* ======================= The format string parser ======================= */

/* Parse a piece of format string, until the matching terminating format
   directive is encountered.
   format is the remainder of the format string.
   position is the position in this argument list, if known, or -1 if unknown.
   list represents the argument list constraints at the current parse point.
   NULL stands for a contradiction.
   escape represents the union of the argument list constraints at all the
   currently pending FORMAT-UP-AND-OUT points. NULL stands for a contradiction
   or an empty union.
   All four are updated upon valid return.
   *separatorp is set to true if the parse terminated due to a ~; separator,
   more precisely to 2 if with colon, or to 1 if without colon.
   spec is the global struct spec.
   terminator is the directive that terminates this parse.
   separator specifies if ~; separators are allowed.
   fdi is an array to be filled with format directive indicators, or NULL.
   If the format string is invalid, false is returned and *invalid_reason is
   set to an error message explaining why.  */
static bool
parse_upto (const char **formatp,
            int *positionp, struct format_arg_list **listp,
            struct format_arg_list **escapep, int *separatorp,
            struct spec *spec, char terminator, bool separator,
            char *fdi, char **invalid_reason)
{
  const char *format = *formatp;
  const char *const format_start = format;
  int position = *positionp;
  struct format_arg_list *list = *listp;
  struct format_arg_list *escape = *escapep;

  for (; *format != '\0'; )
    if (*format++ == '~')
      {
        bool colon_p = false;
        bool atsign_p = false;
        unsigned int paramcount = 0;
        struct param *params = NULL;

        FDI_SET (format - 1, FMTDIR_START);

        /* Count number of directives.  */
        spec->directives++;

        /* Parse parameters.  */
        for (;;)
          {
            enum param_type type = PT_NIL;
            int value = 0;

            if (c_isdigit (*format))
              {
                type = PT_INTEGER;
                do
                  {
                    value = 10 * value + (*format - '0');
                    format++;
                  }
                while (c_isdigit (*format));
              }
            else if (*format == '+' || *format == '-')
              {
                bool negative = (*format == '-');
                type = PT_INTEGER;
                format++;
                if (!c_isdigit (*format))
                  {
                    if (*format == '\0')
                      {
                        *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                        FDI_SET (format - 1, FMTDIR_ERROR);
                      }
                    else
                      {
                        *invalid_reason =
                          xasprintf (_("In the directive number %u, '%c' is not followed by a digit."), spec->directives, format[-1]);
                        FDI_SET (format, FMTDIR_ERROR);
                      }
                    return false;
                  }
                do
                  {
                    value = 10 * value + (*format - '0');
                    format++;
                  }
                while (c_isdigit (*format));
                if (negative)
                  value = -value;
              }
            else if (*format == '\'')
              {
                type = PT_CHARACTER;
                format++;
                if (*format == '\0')
                  {
                    *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    return false;
                  }
                format++;
              }
            else if (*format == 'V' || *format == 'v')
              {
                type = PT_V;
                format++;
                value = position;
                /* Consumes an argument.  */
                if (position >= 0)
                  position++;
              }
            else if (*format == '#')
              {
                type = PT_ARGCOUNT;
                format++;
              }

            params =
              (struct param *)
              xrealloc (params, (paramcount + 1) * sizeof (struct param));
            params[paramcount].type = type;
            params[paramcount].value = value;
            paramcount++;

            if (*format == ',')
              format++;
            else
              break;
          }

        /* Parse modifiers.  */
        for (;;)
          {
            if (*format == ':')
              {
                format++;
                colon_p = true;
              }
            else if (*format == '@')
              {
                format++;
                atsign_p = true;
              }
            else
              break;
          }

        /* Parse directive.  */
        switch (*format++)
          {
          case 'A': case 'a': /* 22.3.4.1 FORMAT-ASCII */
          case 'S': case 's': /* 22.3.4.2 FORMAT-S-EXPRESSION */
            if (!check_params (&list, paramcount, params, 4, IIIC,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_OBJECT);
            break;

          case 'W': case 'w': /* 22.3.4.3 FORMAT-WRITE */
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_OBJECT);
            break;

          case 'D': case 'd': /* 22.3.2.2 FORMAT-DECIMAL */
          case 'B': case 'b': /* 22.3.2.3 FORMAT-BINARY */
          case 'O': case 'o': /* 22.3.2.4 FORMAT-OCTAL */
          case 'X': case 'x': /* 22.3.2.5 FORMAT-HEXADECIMAL */
            if (!check_params (&list, paramcount, params, 4, ICCI,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_INTEGER);
            break;

          case 'R': case 'r': /* 22.3.2.1 FORMAT-RADIX */
            if (!check_params (&list, paramcount, params, 5, IICCI,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_INTEGER);
            break;

          case 'P': case 'p': /* 22.3.8.3 FORMAT-PLURAL */
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (colon_p)
              {
                /* Go back by 1 argument.  */
                if (position > 0)
                  position--;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_OBJECT);
            break;

          case 'C': case 'c': /* 22.3.1.1 FORMAT-CHARACTER */
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_CHARACTER);
            break;

          case 'F': case 'f': /* 22.3.3.1 FORMAT-FIXED-FLOAT */
            if (!check_params (&list, paramcount, params, 5, IIICC,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_REAL);
            break;

          case 'E': case 'e': /* 22.3.3.2 FORMAT-EXPONENTIAL-FLOAT */
          case 'G': case 'g': /* 22.3.3.3 FORMAT-GENERAL-FLOAT */
            if (!check_params (&list, paramcount, params, 7, IIIICCC,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_REAL);
            break;

          case '$': /* 22.3.3.4 FORMAT-DOLLARS-FLOAT */
            if (!check_params (&list, paramcount, params, 4, IIIC,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_REAL);
            break;

          case '%': /* 22.3.1.2 FORMAT-TERPRI */
          case '&': /* 22.3.1.3 FORMAT-FRESH-LINE */
          case '|': /* 22.3.1.4 FORMAT-PAGE */
          case '~': /* 22.3.1.5 FORMAT-TILDE */
          case 'I': case 'i': /* 22.3.5.3 */
            if (!check_params (&list, paramcount, params, 1, I,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            break;

          case '\n': /* 22.3.9.3 #\Newline */
          case '_': /* 22.3.5.1 */
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            break;

          case 'T': case 't': /* 22.3.6.1 FORMAT-TABULATE */
            if (!check_params (&list, paramcount, params, 2, II,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            break;

          case '*': /* 22.3.7.1 FORMAT-GOTO */
            if (!check_params (&list, paramcount, params, 1, I,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            {
              int n; /* value of first parameter */
              if (paramcount == 0
                  || (paramcount >= 1 && params[0].type == PT_NIL))
                n = (atsign_p ? 0 : 1);
              else if (paramcount >= 1 && params[0].type == PT_INTEGER)
                n = params[0].value;
              else
                {
                  /* Unknown argument, leads to an unknown position.  */
                  position = -1;
                  break;
                }
              if (n < 0)
                {
                  /* invalid argument */
                  *invalid_reason =
                    xasprintf (_("In the directive number %u, the argument %d is negative."), spec->directives, n);
                  FDI_SET (format - 1, FMTDIR_ERROR);
                  return false;
                }
              if (atsign_p)
                {
                  /* Absolute goto.  */
                  position = n;
                }
              else if (colon_p)
                {
                  /* Backward goto.  */
                  if (n > 0)
                    {
                      if (position >= 0)
                        {
                          if (position >= n)
                            position -= n;
                          else
                            position = 0;
                        }
                      else
                        position = -1;
                   }
                }
              else
                {
                  /* Forward goto.  */
                  if (position >= 0)
                    position += n;
                }
            }
            break;

          case '?': /* 22.3.7.6 FORMAT-INDIRECTION */
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_FORMATSTRING);
            if (atsign_p)
              position = -1;
            else
              if (position >= 0)
                {
                  struct format_arg_list *sublist = make_unconstrained_list ();
                  add_req_listtype_constraint (&list, position++,
                                               FAT_LIST, sublist);
                  free_list (sublist);
                }
            break;

          case '/': /* 22.3.5.4 FORMAT-CALL-USER-FUNCTION */
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              add_req_type_constraint (&list, position++, FAT_OBJECT);
            while (*format != '\0' && *format != '/')
              format++;
            if (*format == '\0')
              {
                *invalid_reason =
                  xstrdup (_("The string ends in the middle of a ~/.../ directive."));
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            format++;
            break;

          case '(': /* 22.3.8.1 FORMAT-CASE-CONVERSION */
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            *formatp = format;
            *positionp = position;
            *listp = list;
            *escapep = escape;
            {
              if (!parse_upto (formatp, positionp, listp, escapep,
                               NULL, spec, ')', false,
                               NULL, invalid_reason))
                {
                  FDI_SET (**formatp == '\0' ? *formatp - 1 : *formatp,
                           FMTDIR_ERROR);
                  return false;
                }
            }
            format = *formatp;
            position = *positionp;
            list = *listp;
            escape = *escapep;
            break;

          case ')': /* 22.3.8.2 FORMAT-CASE-CONVERSION-END */
            if (terminator != ')')
              {
                *invalid_reason =
                  xasprintf (_("Found '~%c' without matching '~%c'."), ')', '(');
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            *formatp = format;
            *positionp = position;
            *listp = list;
            *escapep = escape;
            return true;

          case '[': /* 22.3.7.2 FORMAT-CONDITIONAL */
            if (atsign_p && colon_p)
              {
                *invalid_reason =
                  xasprintf (_("In the directive number %u, both the @ and the : modifiers are given."), spec->directives);
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            else if (atsign_p)
              {
                struct format_arg_list *nil_list;
                struct format_arg_list *union_list;

                if (!check_params (&list, paramcount, params, 0, NULL,
                                   spec->directives, invalid_reason))
                  {
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    return false;
                  }

                *formatp = format;
                *escapep = escape;

                /* First alternative: argument is NIL.  */
                nil_list = (list != NULL ? copy_list (list) : NULL);
                if (position >= 0)
                  {
                    struct format_arg_list *empty_list = make_empty_list ();
                    add_req_listtype_constraint (&nil_list, position,
                                                 FAT_LIST, empty_list);
                    free_list (empty_list);
                  }

                /* Second alternative: use sub-format.  */
                {
                  int sub_position = position;
                  struct format_arg_list *sub_list =
                    (list != NULL ? copy_list (list) : NULL);
                  if (!parse_upto (formatp, &sub_position, &sub_list, escapep,
                                   NULL, spec, ']', false,
                                   NULL, invalid_reason))
                    {
                      FDI_SET (**formatp == '\0' ? *formatp - 1 : *formatp,
                               FMTDIR_ERROR);
                      return false;
                    }
                  if (sub_list != NULL)
                    {
                      if (position >= 0)
                        {
                          if (sub_position == position + 1)
                            /* new position is branch independent */
                            position = position + 1;
                          else
                            /* new position is branch dependent */
                            position = -1;
                        }
                    }
                  else
                    {
                      if (position >= 0)
                        position = position + 1;
                    }
                  union_list = union (nil_list, sub_list);
                }

                format = *formatp;
                escape = *escapep;

                if (list != NULL)
                  free_list (list);
                list = union_list;
              }
            else if (colon_p)
              {
                int union_position;
                struct format_arg_list *union_list;

                if (!check_params (&list, paramcount, params, 0, NULL,
                                   spec->directives, invalid_reason))
                  {
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    return false;
                  }

                if (position >= 0)
                  add_req_type_constraint (&list, position++, FAT_OBJECT);

                *formatp = format;
                *escapep = escape;
                union_position = -2;
                union_list = NULL;

                /* First alternative.  */
                {
                  int sub_position = position;
                  struct format_arg_list *sub_list =
                    (list != NULL ? copy_list (list) : NULL);
                  int sub_separator = 0;
                  if (position >= 0)
                    {
                      struct format_arg_list *empty_list = make_empty_list ();
                      add_req_listtype_constraint (&sub_list, position - 1,
                                                   FAT_LIST, empty_list);
                      free_list (empty_list);
                    }
                  if (!parse_upto (formatp, &sub_position, &sub_list, escapep,
                                   &sub_separator, spec, ']', true,
                                   NULL, invalid_reason))
                    {
                      FDI_SET (**formatp == '\0' ? *formatp - 1 : *formatp,
                               FMTDIR_ERROR);
                      return false;
                    }
                  if (!sub_separator)
                    {
                      *invalid_reason =
                        xasprintf (_("In the directive number %u, '~:[' is not followed by two clauses, separated by '~;'."), spec->directives);
                      FDI_SET (**formatp == '\0' ? *formatp - 1 : *formatp,
                               FMTDIR_ERROR);
                      return false;
                    }
                  if (sub_list != NULL)
                    union_position = sub_position;
                  union_list = union (union_list, sub_list);
                }

                /* Second alternative.  */
                {
                  int sub_position = position;
                  struct format_arg_list *sub_list =
                    (list != NULL ? copy_list (list) : NULL);
                  if (!parse_upto (formatp, &sub_position, &sub_list, escapep,
                                   NULL, spec, ']', false,
                                   NULL, invalid_reason))
                    {
                      FDI_SET (**formatp == '\0' ? *formatp - 1 : *formatp,
                               FMTDIR_ERROR);
                      return false;
                    }
                  if (sub_list != NULL)
                    {
                      if (union_position == -2)
                        union_position = sub_position;
                      else if (sub_position < 0
                               || sub_position != union_position)
                        union_position = -1;
                    }
                  union_list = union (union_list, sub_list);
                }

                format = *formatp;
                escape = *escapep;

                if (union_position != -2)
                  position = union_position;
                if (list != NULL)
                  free_list (list);
                list = union_list;
              }
            else
              {
                int arg_position;
                int union_position;
                struct format_arg_list *union_list;
                bool last_alternative;

                if (!check_params (&list, paramcount, params, 1, I,
                                   spec->directives, invalid_reason))
                  {
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    return false;
                  }

                /* If there was no first parameter, an argument is consumed.  */
                arg_position = -1;
                if (!(paramcount >= 1 && params[0].type != PT_NIL))
                  if (position >= 0)
                    {
                      arg_position = position;
                      add_req_type_constraint (&list, position++, FAT_OBJECT);
                    }

                *formatp = format;
                *escapep = escape;

                union_position = -2;
                union_list = NULL;
                last_alternative = false;
                for (;;)
                  {
                    /* Next alternative.  */
                    int sub_position = position;
                    struct format_arg_list *sub_list =
                      (list != NULL ? copy_list (list) : NULL);
                    int sub_separator = 0;
                    if (!parse_upto (formatp, &sub_position, &sub_list, escapep,
                                     &sub_separator, spec, ']', !last_alternative,
                                     NULL, invalid_reason))
                      {
                        FDI_SET (**formatp == '\0' ? *formatp - 1 : *formatp,
                                 FMTDIR_ERROR);
                        return false;
                      }
                    /* If this alternative is chosen, the argument arg_position
                       is an integer, namely the index of this alternative.  */
                    if (!last_alternative && arg_position >= 0)
                      add_req_type_constraint (&sub_list, arg_position,
                                               FAT_INTEGER);
                    if (sub_list != NULL)
                      {
                        if (union_position == -2)
                          union_position = sub_position;
                        else if (sub_position < 0
                                 || sub_position != union_position)
                          union_position = -1;
                      }
                    union_list = union (union_list, sub_list);
                    if (sub_separator == 2)
                      last_alternative = true;
                    if (!sub_separator)
                      break;
                  }
                if (!last_alternative)
                  {
                    /* An implicit default alternative.  */
                    if (union_position == -2)
                      union_position = position;
                    else if (position < 0 || position != union_position)
                      union_position = -1;
                    if (list != NULL)
                      union_list = union (union_list, copy_list (list));
                  }

                format = *formatp;
                escape = *escapep;

                if (union_position != -2)
                  position = union_position;
                if (list != NULL)
                  free_list (list);
                list = union_list;
              }
            break;

          case ']': /* 22.3.7.3 FORMAT-CONDITIONAL-END */
            if (terminator != ']')
              {
                *invalid_reason =
                  xasprintf (_("Found '~%c' without matching '~%c'."), ']', '[');
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            *formatp = format;
            *positionp = position;
            *listp = list;
            *escapep = escape;
            return true;

          case '{': /* 22.3.7.4 FORMAT-ITERATION */
            if (!check_params (&list, paramcount, params, 1, I,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            *formatp = format;
            {
              int sub_position = 0;
              struct format_arg_list *sub_list = make_unconstrained_list ();
              struct format_arg_list *sub_escape = NULL;
              struct spec sub_spec;
              sub_spec.directives = 0;
              sub_spec.list = sub_list;
              if (!parse_upto (formatp, &sub_position, &sub_list, &sub_escape,
                               NULL, &sub_spec, '}', false,
                               NULL, invalid_reason))
                {
                  FDI_SET (**formatp == '\0' ? *formatp - 1 : *formatp,
                           FMTDIR_ERROR);
                  return false;
                }
              spec->directives += sub_spec.directives;

              /* If the sub-formatstring is empty, except for the terminating
                 ~} directive, a formatstring argument is consumed.  */
              if (*format == '~' && sub_spec.directives == 1)
                if (position >= 0)
                  add_req_type_constraint (&list, position++, FAT_FORMATSTRING);

              if (colon_p)
                {
                  /* Each iteration uses a new sublist.  */
                  struct format_arg_list *listlist;

                  /* ~{ catches ~^.  */
                  sub_list = union (sub_list, sub_escape);

                  listlist = make_repeated_list_of_lists (sub_list);

                  sub_list = listlist;
                }
              else
                {
                  /* Each iteration's arguments are all concatenated in a
                     single list.  */
                  struct format_arg_list *looplist;

                  /* FIXME: This is far from correct.  Test cases:
                     abc~{~^~}
                     abc~{~S~^~S~}
                     abc~{~D~^~C~}
                     abc~{~D~^~D~}
                     abc~{~D~^~S~}
                     abc~{~D~^~C~}~:*~{~S~^~D~}
                   */

                  /* ~{ catches ~^.  */
                  sub_list = union (sub_list, sub_escape);

                  if (sub_list == NULL)
                    looplist = make_empty_list ();
                  else
                    if (sub_position < 0 || sub_position == 0)
                      /* Too hard to track the possible argument types
                         when the iteration is performed 2 times or more.
                         So be satisfied with the constraints of executing
                         the iteration 1 or 0 times.  */
                      looplist = make_union_with_empty_list (sub_list);
                    else
                      looplist = make_repeated_list (sub_list, sub_position);

                  sub_list = looplist;
                }

              if (atsign_p)
                {
                  /* All remaining arguments are used.  */
                  if (list != NULL && position >= 0)
                    {
                      shift_list (sub_list, position);
                      list = make_intersected_list (list, sub_list);
                    }
                  position = -1;
                }
              else
                {
                  /* The argument is a list.  */
                  if (position >= 0)
                    add_req_listtype_constraint (&list, position++,
                                                 FAT_LIST, sub_list);
                }
            }
            format = *formatp;
            break;

          case '}': /* 22.3.7.5 FORMAT-ITERATION-END */
            if (terminator != '}')
              {
                *invalid_reason =
                  xasprintf (_("Found '~%c' without matching '~%c'."), '}', '{');
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            *formatp = format;
            *positionp = position;
            *listp = list;
            *escapep = escape;
            return true;

          case '<': /* 22.3.6.2, 22.3.5.2 FORMAT-JUSTIFICATION */
            if (!check_params (&list, paramcount, params, 4, IIIC,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            {
              struct format_arg_list *sub_escape = NULL;

              *formatp = format;
              *positionp = position;
              *listp = list;

              for (;;)
                {
                  int sub_separator = 0;
                  if (!parse_upto (formatp, positionp, listp, &sub_escape,
                                   &sub_separator, spec, '>', true,
                                   NULL, invalid_reason))
                    {
                      FDI_SET (**formatp == '\0' ? *formatp - 1 : *formatp,
                               FMTDIR_ERROR);
                      return false;
                    }
                  if (!sub_separator)
                    break;
                }

              format = *formatp;
              position = *positionp;
              list = *listp;

              /* ~< catches ~^.  */
              if (sub_escape != NULL)
                position = -1;
              list = union (list, sub_escape);
            }
            break;

          case '>': /* 22.3.6.3 FORMAT-JUSTIFICATION-END */
            if (terminator != '>')
              {
                *invalid_reason =
                  xasprintf (_("Found '~%c' without matching '~%c'."), '>', '<');
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (!check_params (&list, paramcount, params, 0, NULL,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            *formatp = format;
            *positionp = position;
            *listp = list;
            *escapep = escape;
            return true;

          case '^': /* 22.3.9.2 FORMAT-UP-AND-OUT */
            if (!check_params (&list, paramcount, params, 3, THREE,
                               spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0 && list != NULL && is_required (list, position))
              /* This ~^ can never be executed.  Ignore it.  */
              break;
            if (list != NULL)
              {
                struct format_arg_list *this_escape = copy_list (list);
                if (position >= 0)
                  this_escape = add_end_constraint (this_escape, position);
                escape = union (escape, this_escape);
              }
            if (position >= 0)
              list = add_required_constraint (list, position);
            break;

          case ';': /* 22.3.9.1 FORMAT-SEPARATOR */
            if (!separator)
              {
                *invalid_reason =
                  xasprintf (_("In the directive number %u, '~;' is used in an invalid position."), spec->directives);
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (terminator == '>')
              {
                if (!check_params (&list, paramcount, params, 1, I,
                                   spec->directives, invalid_reason))
                  {
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    return false;
                  }
              }
            else
              {
                if (!check_params (&list, paramcount, params, 0, NULL,
                                   spec->directives, invalid_reason))
                  {
                    FDI_SET (format - 1, FMTDIR_ERROR);
                    return false;
                  }
              }
            *formatp = format;
            *positionp = position;
            *listp = list;
            *escapep = escape;
            *separatorp = (colon_p ? 2 : 1);
            return true;

          case '!': /* FORMAT-CALL, a CLISP extension */
            if (!nocheck_params (&list, paramcount, params,
                                 spec->directives, invalid_reason))
              {
                FDI_SET (format - 1, FMTDIR_ERROR);
                return false;
              }
            if (position >= 0)
              {
                add_req_type_constraint (&list, position++, FAT_FUNCTION);
                add_req_type_constraint (&list, position++, FAT_OBJECT);
              }
            break;

          default:
            --format;
            if (*format == '\0')
              {
                *invalid_reason = INVALID_UNTERMINATED_DIRECTIVE ();
                FDI_SET (format - 1, FMTDIR_ERROR);
              }
            else
              {
                *invalid_reason =
                  INVALID_CONVERSION_SPECIFIER (spec->directives, *format);
                FDI_SET (format, FMTDIR_ERROR);
              }
            return false;
          }

        FDI_SET (format - 1, FMTDIR_END);

        free (params);
      }

  *formatp = format;
  *positionp = position;
  *listp = list;
  *escapep = escape;
  if (terminator != '\0')
    {
      *invalid_reason =
        xasprintf (_("Found '~%c' without matching '~%c'."), terminator - 1, terminator);
      return false;
    }
  return true;
}


/* ============== Top level format string handling functions ============== */

static void *
format_parse (const char *format, bool translated, char *fdi,
              char **invalid_reason)
{
  struct spec spec;
  struct spec *result;
  int position = 0;
  struct format_arg_list *escape;

  spec.directives = 0;
  spec.list = make_unconstrained_list ();
  escape = NULL;

  if (!parse_upto (&format, &position, &spec.list, &escape,
                   NULL, &spec, '\0', false,
                   fdi, invalid_reason))
    /* Invalid format string.  */
    return NULL;

  /* Catch ~^ here.  */
  spec.list = union (spec.list, escape);

  if (spec.list == NULL)
    {
      /* Contradictory argument type information.  */
      *invalid_reason =
        xstrdup (_("The string refers to some argument in incompatible ways."));
      return NULL;
    }

  /* Normalize the result.  */
  normalize_list (spec.list);

  result = XMALLOC (struct spec);
  *result = spec;
  return result;
}

static void
format_free (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  free_list (spec->list);
}

static int
format_get_number_of_directives (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  return spec->directives;
}

static bool
format_check (void *msgid_descr, void *msgstr_descr, bool equality,
              formatstring_error_logger_t error_logger,
              const char *pretty_msgid, const char *pretty_msgstr)
{
  struct spec *spec1 = (struct spec *) msgid_descr;
  struct spec *spec2 = (struct spec *) msgstr_descr;
  bool err = false;

  if (equality)
    {
      if (!equal_list (spec1->list, spec2->list))
        {
          if (error_logger)
            error_logger (_("format specifications in '%s' and '%s' are not equivalent"),
                          pretty_msgid, pretty_msgstr);
          err = true;
        }
    }
  else
    {
      struct format_arg_list *intersection =
        make_intersected_list (copy_list (spec1->list),
                               copy_list (spec2->list));

      if (!(intersection != NULL
            && (normalize_list (intersection),
                equal_list (intersection, spec2->list))))
        {
          if (error_logger)
            error_logger (_("format specifications in '%s' are not a subset of those in '%s'"),
                          pretty_msgstr, pretty_msgid);
          err = true;
        }
    }

  return err;
}


struct formatstring_parser formatstring_lisp =
{
  format_parse,
  format_free,
  format_get_number_of_directives,
  NULL,
  format_check
};


/* ============================= Testing code ============================= */

#undef union

#ifdef TEST

/* Test program: Print the argument list specification returned by
   format_parse for strings read from standard input.  */

#include <stdio.h>

static void print_list (struct format_arg_list *list);

static void
print_element (struct format_arg *element)
{
  switch (element->presence)
    {
    case FCT_REQUIRED:
      break;
    case FCT_OPTIONAL:
      printf (". ");
      break;
    default:
      abort ();
    }

  switch (element->type)
    {
    case FAT_OBJECT:
      printf ("*");
      break;
    case FAT_CHARACTER_INTEGER_NULL:
      printf ("ci()");
      break;
    case FAT_CHARACTER_NULL:
      printf ("c()");
      break;
    case FAT_CHARACTER:
      printf ("c");
      break;
    case FAT_INTEGER_NULL:
      printf ("i()");
      break;
    case FAT_INTEGER:
      printf ("i");
      break;
    case FAT_REAL:
      printf ("r");
      break;
    case FAT_LIST:
      print_list (element->list);
      break;
    case FAT_FORMATSTRING:
      printf ("~");
      break;
    case FAT_FUNCTION:
      printf ("f");
      break;
    default:
      abort ();
    }
}

static void
print_list (struct format_arg_list *list)
{
  unsigned int i, j;

  printf ("(");

  for (i = 0; i < list->initial.count; i++)
    for (j = 0; j < list->initial.element[i].repcount; j++)
      {
        if (i > 0 || j > 0)
          printf (" ");
        print_element (&list->initial.element[i]);
      }

  if (list->repeated.count > 0)
    {
      printf (" |");
      for (i = 0; i < list->repeated.count; i++)
        for (j = 0; j < list->repeated.element[i].repcount; j++)
          {
            printf (" ");
            print_element (&list->repeated.element[i]);
          }
    }

  printf (")");
}

static void
format_print (void *descr)
{
  struct spec *spec = (struct spec *) descr;

  if (spec == NULL)
    {
      printf ("INVALID");
      return;
    }

  print_list (spec->list);
}

int
main ()
{
  for (;;)
    {
      char *line = NULL;
      size_t line_size = 0;
      int line_len;
      char *invalid_reason;
      void *descr;

      line_len = getline (&line, &line_size, stdin);
      if (line_len < 0)
        break;
      if (line_len > 0 && line[line_len - 1] == '\n')
        line[--line_len] = '\0';

      invalid_reason = NULL;
      descr = format_parse (line, false, NULL, &invalid_reason);

      format_print (descr);
      printf ("\n");
      if (descr == NULL)
        printf ("%s\n", invalid_reason);

      free (invalid_reason);
      free (line);
    }

  return 0;
}

/*
 * For Emacs M-x compile
 * Local Variables:
 * compile-command: "/bin/sh ../libtool --tag=CC --mode=link gcc -o a.out -static -O -g -Wall -I.. -I../gnulib-lib -I../../gettext-runtime/intl -DHAVE_CONFIG_H -DTEST format-lisp.c ../gnulib-lib/libgettextlib.la"
 * End:
 */

#endif /* TEST */
