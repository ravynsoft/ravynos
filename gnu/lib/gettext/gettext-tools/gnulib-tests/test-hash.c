/*
 * Copyright (C) 2009-2023 Free Software Foundation, Inc.
 * Written by Jim Meyering
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

#include "hash.h"
#include "hash-pjw.h"
#include "inttostr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "macros.h"

#define STREQ(a, b) (strcmp (a, b) == 0)
#define ARRAY_CARDINALITY(Array) (sizeof (Array) / sizeof *(Array))

static bool
hash_compare_strings (void const *x, void const *y)
{
  ASSERT (x != y);
  return STREQ (x, y) ? true : false;
}

static void
hash_freer (void *x)
{
  free (x);
}

static void
insert_new (Hash_table *ht, const void *ent)
{
  void *e = hash_insert (ht, ent);
  ASSERT (e == ent);
}

static bool
walk (void *ent, void *data)
{
  char *str = ent;
  unsigned int *map = data;
  switch (*str)
    {
    case 'a': *map |= 1; return true;
    case 'b': *map |= 2; return true;
    case 'c': *map |= 4; return true;
    }
  *map |= 8;
  return false;
}

static int
get_seed (char const *str, unsigned int *seed)
{
  size_t len = strlen (str);
  if (len == 0 || strspn (str, "0123456789") != len || 10 < len)
    return 1;

  *seed = atoi (str);
  return 0;
}

int
main (int argc, char **argv)
{
  unsigned int i;
  unsigned int k;
  unsigned int table_size[] = {1, 2, 3, 4, 5, 23, 53};
  Hash_table *ht;
  Hash_tuning tuning;

  hash_reset_tuning (&tuning);
  tuning.shrink_threshold = 0.3;
  tuning.shrink_factor = 0.707;
  tuning.growth_threshold = 1.5;
  tuning.growth_factor = 2.0;
  tuning.is_n_buckets = true;

  if (1 < argc)
    {
      unsigned int seed;
      if (get_seed (argv[1], &seed) != 0)
        {
          fprintf (stderr, "invalid seed: %s\n", argv[1]);
          exit (EXIT_FAILURE);
        }

      srand (seed);
    }

  for (i = 0; i < ARRAY_CARDINALITY (table_size); i++)
    {
      size_t sz = table_size[i];
      ht = hash_initialize (sz, NULL, hash_pjw, hash_compare_strings, NULL);
      ASSERT (ht);
      insert_new (ht, "a");
      {
        char *str1 = strdup ("a");
        char *str2;
        ASSERT (str1);
        str2 = hash_insert (ht, str1);
        ASSERT (str1 != str2);
        ASSERT (STREQ (str1, str2));
        free (str1);
      }
      insert_new (ht, "b");
      insert_new (ht, "c");
      i = 0;
      ASSERT (hash_do_for_each (ht, walk, &i) == 3);
      ASSERT (i == 7);
      {
        void *buf[5] = { NULL };
        ASSERT (hash_get_entries (ht, NULL, 0) == 0);
        ASSERT (hash_get_entries (ht, buf, 5) == 3);
        ASSERT (STREQ (buf[0], "a") || STREQ (buf[0], "b") || STREQ (buf[0], "c"));
      }
      ASSERT (hash_remove (ht, "a"));
      ASSERT (hash_remove (ht, "a") == NULL);
      ASSERT (hash_remove (ht, "b"));
      ASSERT (hash_remove (ht, "c"));

      ASSERT (hash_rehash (ht, 47));
      ASSERT (hash_rehash (ht, 467));

      /* Free an empty table. */
      hash_clear (ht);
      hash_free (ht);

      ht = hash_initialize (sz, NULL, hash_pjw, hash_compare_strings, NULL);
      ASSERT (ht);

      insert_new (ht, "z");
      insert_new (ht, "y");
      insert_new (ht, "x");
      insert_new (ht, "w");
      insert_new (ht, "v");
      insert_new (ht, "u");

      hash_clear (ht);
      ASSERT (hash_get_n_entries (ht) == 0);
      hash_free (ht);

      /* Test pointer hashing.  */
      ht = hash_initialize (sz, NULL, NULL, NULL, NULL);
      ASSERT (ht);
      {
        char *str = strdup ("a");
        ASSERT (str);
        insert_new (ht, "a");
        insert_new (ht, str);
        ASSERT (hash_lookup (ht, str) == str);
        free (str);
      }
      hash_free (ht);
    }

  hash_reset_tuning (&tuning);
  tuning.shrink_threshold = 0.3;
  tuning.shrink_factor = 0.707;
  tuning.growth_threshold = 1.5;
  tuning.growth_factor = 2.0;
  tuning.is_n_buckets = true;
  /* Invalid tuning.  */
  ht = hash_initialize (4651, &tuning, hash_pjw, hash_compare_strings,
                        hash_freer);
  ASSERT (!ht);

  /* Alternate tuning.  */
  tuning.growth_threshold = 0.89;

  /* Run with default tuning, then with custom tuning settings.  */
  for (k = 0; k < 2; k++)
    {
      Hash_tuning const *tune = (k == 0 ? NULL : &tuning);
      /* Now, each entry is malloc'd.  */
      ht = hash_initialize (4651, tune, hash_pjw,
                            hash_compare_strings, hash_freer);
      ASSERT (ht);
      for (i = 0; i < 10000; i++)
        {
          unsigned int op = rand () % 10;
          switch (op)
            {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
              {
                char buf[50];
                char const *p = uinttostr (i, buf);
                char *p_dup = strdup (p);
                ASSERT (p_dup);
                insert_new (ht, p_dup);
              }
              break;

            case 6:
              {
                size_t n = hash_get_n_entries (ht);
                ASSERT (hash_rehash (ht, n + rand () % 20));
              }
              break;

            case 7:
              {
                size_t n = hash_get_n_entries (ht);
                size_t delta = rand () % 20;
                if (delta < n)
                  ASSERT (hash_rehash (ht, n - delta));
              }
              break;

            case 8:
            case 9:
              {
                /* Delete a random entry.  */
                size_t n = hash_get_n_entries (ht);
                if (n)
                  {
                    size_t kk = rand () % n;
                    void const *p;
                    void *v;
                    for (p = hash_get_first (ht); kk;
                         --kk, p = hash_get_next (ht, p))
                      {
                        /* empty */
                      }
                    ASSERT (p);
                    v = hash_remove (ht, p);
                    ASSERT (v);
                    free (v);
                  }
                break;
              }
            }
          ASSERT (hash_table_ok (ht));
        }

      hash_free (ht);
    }

  return 0;
}
