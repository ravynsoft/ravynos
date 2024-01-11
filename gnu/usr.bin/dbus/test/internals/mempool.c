/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-mempool.h Memory pools
 *
 * Copyright (C) 2002, 2003  Red Hat, Inc.
 * Copyright (C) 2003  CodeFactory AB
 * Copyright (C) 2013  Chengwei Yang / Intel
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <config.h>
#include "misc-internals.h"

#include <stdio.h>
#include <time.h>

#include <dbus/dbus-mempool.h>
#include <dbus/dbus-internals.h>

static void
time_for_size (int size)
{
  int i;
  int j;
#ifdef DBUS_ENABLE_VERBOSE_MODE
  clock_t start;
  clock_t end;
#endif
#define FREE_ARRAY_SIZE 512
#define N_ITERATIONS FREE_ARRAY_SIZE * 512
  void *to_free[FREE_ARRAY_SIZE];
  DBusMemPool *pool;

  _dbus_verbose ("Timings for size %d\n", size);

  _dbus_verbose (" malloc\n");

#ifdef DBUS_ENABLE_VERBOSE_MODE
  start = clock ();
#endif

  i = 0;
  j = 0;
  while (i < N_ITERATIONS)
    {
      to_free[j] = dbus_malloc (size);
      _dbus_assert (to_free[j] != NULL); /* in a real app of course this is wrong */

      ++j;

      if (j == FREE_ARRAY_SIZE)
        {
          j = 0;
          while (j < FREE_ARRAY_SIZE)
            {
              dbus_free (to_free[j]);
              ++j;
            }

          j = 0;
        }

      ++i;
    }

#ifdef DBUS_ENABLE_VERBOSE_MODE
  end = clock ();

  _dbus_verbose ("  created/destroyed %d elements in %g seconds\n",
                 N_ITERATIONS, (end - start) / (double) CLOCKS_PER_SEC);



  _dbus_verbose (" mempools\n");

  start = clock ();
#endif

  pool = _dbus_mem_pool_new (size, FALSE);

  i = 0;
  j = 0;
  while (i < N_ITERATIONS)
    {
      to_free[j] = _dbus_mem_pool_alloc (pool);
      _dbus_assert (to_free[j] != NULL);  /* in a real app of course this is wrong */

      ++j;

      if (j == FREE_ARRAY_SIZE)
        {
          j = 0;
          while (j < FREE_ARRAY_SIZE)
            {
              _dbus_mem_pool_dealloc (pool, to_free[j]);
              ++j;
            }

          j = 0;
        }

      ++i;
    }

  _dbus_mem_pool_free (pool);

#ifdef DBUS_ENABLE_VERBOSE_MODE
  end = clock ();

  _dbus_verbose ("  created/destroyed %d elements in %g seconds\n",
                 N_ITERATIONS, (end - start) / (double) CLOCKS_PER_SEC);

  _dbus_verbose (" zeroed malloc\n");

  start = clock ();
#endif

  i = 0;
  j = 0;
  while (i < N_ITERATIONS)
    {
      to_free[j] = dbus_malloc0 (size);
      _dbus_assert (to_free[j] != NULL); /* in a real app of course this is wrong */

      ++j;

      if (j == FREE_ARRAY_SIZE)
        {
          j = 0;
          while (j < FREE_ARRAY_SIZE)
            {
              dbus_free (to_free[j]);
              ++j;
            }

          j = 0;
        }

      ++i;
    }

#ifdef DBUS_ENABLE_VERBOSE_MODE
  end = clock ();

  _dbus_verbose ("  created/destroyed %d elements in %g seconds\n",
                 N_ITERATIONS, (end - start) / (double) CLOCKS_PER_SEC);

  _dbus_verbose (" zeroed mempools\n");

  start = clock ();
#endif

  pool = _dbus_mem_pool_new (size, TRUE);

  i = 0;
  j = 0;
  while (i < N_ITERATIONS)
    {
      to_free[j] = _dbus_mem_pool_alloc (pool);
      _dbus_assert (to_free[j] != NULL);  /* in a real app of course this is wrong */

      ++j;

      if (j == FREE_ARRAY_SIZE)
        {
          j = 0;
          while (j < FREE_ARRAY_SIZE)
            {
              _dbus_mem_pool_dealloc (pool, to_free[j]);
              ++j;
            }

          j = 0;
        }

      ++i;
    }

  _dbus_mem_pool_free (pool);

#ifdef DBUS_ENABLE_VERBOSE_MODE
  end = clock ();

  _dbus_verbose ("  created/destroyed %d elements in %g seconds\n",
                 N_ITERATIONS, (end - start) / (double) CLOCKS_PER_SEC);
#endif
}

/**
 * @ingroup DBusMemPoolInternals
 * Unit test for DBusMemPool
 * @returns #TRUE on success.
 */
dbus_bool_t
_dbus_mem_pool_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  int i;
  int element_sizes[] = { 4, 8, 16, 50, 124 };

  i = 0;
  while (i < _DBUS_N_ELEMENTS (element_sizes))
    {
      time_for_size (element_sizes[i]);
      ++i;
    }

  return TRUE;
}
