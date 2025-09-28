/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 1991-1993 The Regents of the University of California
 * Copyright 1994 Sun Microsystems, Inc.
 * Copyright 2002-2009 Red Hat, Inc.
 * Copyright 2003 Joe Shaw
 * Copyright 2011-2018 Collabora Ltd.
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
 * The following license applies to code from the Tcl distribution,
 * if there is any in this file:
 *
 * Copyright (c) 1991-1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * This software is copyrighted by the Regents of the University of
 * California, Sun Microsystems, Inc., Scriptics Corporation, and
 * other parties.  The following terms apply to all files associated
 * with the software unless explicitly disclaimed in individual files.
 *
 * The authors hereby grant permission to use, copy, modify,
 * distribute, and license this software and its documentation for any
 * purpose, provided that existing copyright notices are retained in
 * all copies and that this notice is included verbatim in any
 * distributions. No written agreement, license, or royalty fee is
 * required for any of the authorized uses.  Modifications to this
 * software may be copyrighted by their authors and need not follow
 * the licensing terms described here, provided that the new terms are
 * clearly indicated on the first page of each file where they apply.
 *
 * IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY
 * PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION,
 * OR ANY DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS,
 * AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * GOVERNMENT USE: If you are acquiring this software on behalf of the
 * U.S. government, the Government shall have only "Restricted Rights"
 * in the software and related documentation as defined in the Federal
 * Acquisition Regulations (FARs) in Clause 52.227.19 (c) (2).  If you
 * are acquiring the software on behalf of the Department of Defense,
 * the software shall be classified as "Commercial Computer Software"
 * and the Government shall have only "Restricted Rights" as defined
 * in Clause 252.227-7013 (c) (1) of DFARs.  Notwithstanding the
 * foregoing, the authors grant the U.S. Government and others acting
 * in its behalf permission to use and distribute the software in
 * accordance with the terms specified in this license.
 */

#include <config.h>

#include <stdio.h>

#include "dbus/dbus-hash.h"
#include "dbus/dbus-internals.h"
#include "dbus/dbus-test.h"
#include "dbus/dbus-test-tap.h"
#include "test/test-utils.h"

/* If you're wondering why the hash table test takes
 * forever to run, it's because we call this function
 * in inner loops thus making things quadratic.
 */
static int
count_entries (DBusHashTable *table)
{
  DBusHashIter iter;
  int count;

  count = 0;
  _dbus_hash_iter_init (table, &iter);
  while (_dbus_hash_iter_next (&iter))
    ++count;

  _dbus_test_check (count == _dbus_hash_table_get_n_entries (table));

  return count;
}

static inline void *
steal (void *ptr)
{
  /* @ptr is passed in as void* to avoid casting in the call */
  void **_ptr = (void **) ptr;
  void *val;

  val = *_ptr;
  *_ptr = NULL;

  return val;
}

/**
 * @ingroup DBusHashTableInternals
 * Unit test for DBusHashTable
 * @returns #TRUE on success.
 */
static dbus_bool_t
_dbus_hash_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  int i;
  DBusHashTable *table1;
  DBusHashTable *table2;
  DBusHashTable *table3;
  DBusHashIter iter;
#define N_HASH_KEYS 5000
  char **keys;
  dbus_bool_t ret = FALSE;
  char *str_key = NULL;
  char *str_value = NULL;

  keys = dbus_new (char *, N_HASH_KEYS);
  if (keys == NULL)
    _dbus_test_fatal ("no memory");

  for (i = 0; i < N_HASH_KEYS; i++)
    {
      keys[i] = dbus_malloc (128);

      if (keys[i] == NULL)
        _dbus_test_fatal ("no memory");
    }

  _dbus_test_diag ("Computing test hash keys...");
  i = 0;
  while (i < N_HASH_KEYS)
    {
      int len;

      len = sprintf (keys[i], "Hash key %d", i);
      _dbus_test_check (*(keys[i] + len) == '\0');
      ++i;
    }
  _dbus_test_diag ("... done.");

  table1 = _dbus_hash_table_new (DBUS_HASH_STRING,
                                 dbus_free, dbus_free);
  if (table1 == NULL)
    goto out;

  table2 = _dbus_hash_table_new (DBUS_HASH_INT,
                                 NULL, dbus_free);
  if (table2 == NULL)
    goto out;

  table3 = _dbus_hash_table_new (DBUS_HASH_UINTPTR,
                                 NULL, dbus_free);
  if (table3 == NULL)
    goto out;

  /* Insert and remove a bunch of stuff, counting the table in between
   * to be sure it's not broken and that iteration works
   */
  i = 0;
  while (i < 3000)
    {
      const void *out_value;

      str_key = _dbus_strdup (keys[i]);
      if (str_key == NULL)
        goto out;
      str_value = _dbus_strdup ("Value!");
      if (str_value == NULL)
        goto out;

      if (!_dbus_hash_table_insert_string (table1,
                                           steal (&str_key),
                                           steal (&str_value)))
        goto out;

      str_value = _dbus_strdup (keys[i]);
      if (str_value == NULL)
        goto out;

      if (!_dbus_hash_table_insert_int (table2,
                                        i, steal (&str_value)))
        goto out;

      str_value = _dbus_strdup (keys[i]);
      if (str_value == NULL)
        goto out;

      if (!_dbus_hash_table_insert_uintptr (table3,
                                            i, steal (&str_value)))
        goto out;

      _dbus_test_check (count_entries (table1) == i + 1);
      _dbus_test_check (count_entries (table2) == i + 1);
      _dbus_test_check (count_entries (table3) == i + 1);

      out_value = _dbus_hash_table_lookup_string (table1, keys[i]);
      _dbus_test_check (out_value != NULL);
      _dbus_test_check (strcmp (out_value, "Value!") == 0);

      out_value = _dbus_hash_table_lookup_int (table2, i);
      _dbus_test_check (out_value != NULL);
      _dbus_test_check (strcmp (out_value, keys[i]) == 0);

      out_value = _dbus_hash_table_lookup_uintptr (table3, i);
      _dbus_test_check (out_value != NULL);
      _dbus_test_check (strcmp (out_value, keys[i]) == 0);

      ++i;
    }

  --i;
  while (i >= 0)
    {
      _dbus_hash_table_remove_string (table1,
                                      keys[i]);

      _dbus_hash_table_remove_int (table2, i);

      _dbus_hash_table_remove_uintptr (table3, i);

      _dbus_test_check (count_entries (table1) == i);
      _dbus_test_check (count_entries (table2) == i);
      _dbus_test_check (count_entries (table3) == i);

      --i;
    }

  _dbus_hash_table_ref (table1);
  _dbus_hash_table_ref (table2);
  _dbus_hash_table_ref (table3);
  _dbus_hash_table_unref (table1);
  _dbus_hash_table_unref (table2);
  _dbus_hash_table_unref (table3);
  _dbus_hash_table_unref (table1);
  _dbus_hash_table_unref (table2);
  _dbus_hash_table_unref (table3);
  table3 = NULL;

  /* Insert a bunch of stuff then check
   * that iteration works correctly (finds the right
   * values, iter_set_value works, etc.)
   */
  table1 = _dbus_hash_table_new (DBUS_HASH_STRING,
                                 dbus_free, dbus_free);
  if (table1 == NULL)
    goto out;

  table2 = _dbus_hash_table_new (DBUS_HASH_INT,
                                 NULL, dbus_free);
  if (table2 == NULL)
    goto out;

  i = 0;
  while (i < 5000)
    {
      str_key = _dbus_strdup (keys[i]);
      if (str_key == NULL)
        goto out;
      str_value = _dbus_strdup ("Value!");
      if (str_value == NULL)
        goto out;

      if (!_dbus_hash_table_insert_string (table1,
                                           steal (&str_key),
                                           steal (&str_value)))
        goto out;

      str_value = _dbus_strdup (keys[i]);
      if (str_value == NULL)
        goto out;

      if (!_dbus_hash_table_insert_int (table2,
                                        i, steal (&str_value)))
        goto out;

      _dbus_test_check (count_entries (table1) == i + 1);
      _dbus_test_check (count_entries (table2) == i + 1);

      ++i;
    }

  _dbus_hash_iter_init (table1, &iter);
  while (_dbus_hash_iter_next (&iter))
    {
      const char *key;
      const void *value;

      key = _dbus_hash_iter_get_string_key (&iter);
      value = _dbus_hash_iter_get_value (&iter);

      _dbus_test_check (_dbus_hash_table_lookup_string (table1, key) == value);

      str_value = _dbus_strdup ("Different value!");
      if (str_value == NULL)
        goto out;

      value = str_value;
      _dbus_hash_iter_set_value (&iter, steal (&str_value));
      _dbus_test_check (_dbus_hash_table_lookup_string (table1, key) == value);
    }

  _dbus_hash_iter_init (table1, &iter);
  while (_dbus_hash_iter_next (&iter))
    {
      _dbus_hash_iter_remove_entry (&iter);
      _dbus_test_check (count_entries (table1) == i - 1);
      --i;
    }

  _dbus_hash_iter_init (table2, &iter);
  while (_dbus_hash_iter_next (&iter))
    {
      int key;
      const void *value;

      key = _dbus_hash_iter_get_int_key (&iter);
      value = _dbus_hash_iter_get_value (&iter);

      _dbus_test_check (_dbus_hash_table_lookup_int (table2, key) == value);

      str_value = _dbus_strdup ("Different value!");
      if (str_value == NULL)
        goto out;

      value = str_value;
      _dbus_hash_iter_set_value (&iter, steal (&str_value));
      _dbus_test_check (_dbus_hash_table_lookup_int (table2, key) == value);
    }

  i = count_entries (table2);
  _dbus_hash_iter_init (table2, &iter);
  while (_dbus_hash_iter_next (&iter))
    {
      _dbus_hash_iter_remove_entry (&iter);
      _dbus_test_check (count_entries (table2) + 1 == i);
      --i;
    }

  /* add/remove interleaved, to check that we grow/shrink the table
   * appropriately
   */
  i = 0;
  while (i < 1000)
    {
      str_key = _dbus_strdup (keys[i]);
      if (str_key == NULL)
        goto out;

      str_value = _dbus_strdup ("Value!");
      if (str_value == NULL)
        goto out;

      if (!_dbus_hash_table_insert_string (table1,
                                           steal (&str_key),
                                           steal (&str_value)))
        goto out;

      ++i;
    }

  --i;
  while (i >= 0)
    {
      str_key = _dbus_strdup (keys[i]);
      if (str_key == NULL)
        goto out;
      str_value = _dbus_strdup ("Value!");
      if (str_value == NULL)
        goto out;

      if (!_dbus_hash_table_remove_string (table1, keys[i]))
        goto out;

      if (!_dbus_hash_table_insert_string (table1,
                                           steal (&str_key),
                                           steal (&str_value)))
        goto out;

      if (!_dbus_hash_table_remove_string (table1, keys[i]))
        goto out;

      _dbus_test_check (_dbus_hash_table_get_n_entries (table1) == i);

      --i;
    }

  /* nuke these tables */
  _dbus_hash_table_unref (table1);
  _dbus_hash_table_unref (table2);


  /* Now do a bunch of things again using _dbus_hash_iter_lookup() to
   * be sure that interface works.
   */
  table1 = _dbus_hash_table_new (DBUS_HASH_STRING,
                                 dbus_free, dbus_free);
  if (table1 == NULL)
    goto out;

  table2 = _dbus_hash_table_new (DBUS_HASH_INT,
                                 NULL, dbus_free);
  if (table2 == NULL)
    goto out;

  i = 0;
  while (i < 3000)
    {
      const void *out_value;

      str_key = _dbus_strdup (keys[i]);
      if (str_key == NULL)
        goto out;
      str_value = _dbus_strdup ("Value!");
      if (str_value == NULL)
        goto out;

      if (!_dbus_hash_iter_lookup (table1,
                                   steal (&str_key), TRUE, &iter))
        goto out;
      _dbus_test_check (_dbus_hash_iter_get_value (&iter) == NULL);
      _dbus_hash_iter_set_value (&iter, steal (&str_value));

      str_value = _dbus_strdup (keys[i]);
      if (str_value == NULL)
        goto out;

      if (!_dbus_hash_iter_lookup (table2,
                                   _DBUS_INT_TO_POINTER (i), TRUE, &iter))
        goto out;
      _dbus_test_check (_dbus_hash_iter_get_value (&iter) == NULL);
      _dbus_hash_iter_set_value (&iter, steal (&str_value));

      _dbus_test_check (count_entries (table1) == i + 1);
      _dbus_test_check (count_entries (table2) == i + 1);

      if (!_dbus_hash_iter_lookup (table1, keys[i], FALSE, &iter))
        goto out;

      out_value = _dbus_hash_iter_get_value (&iter);
      _dbus_test_check (out_value != NULL);
      _dbus_test_check (strcmp (out_value, "Value!") == 0);

      /* Iterate just to be sure it works, though
       * it's a stupid thing to do
       */
      while (_dbus_hash_iter_next (&iter))
        ;

      if (!_dbus_hash_iter_lookup (table2, _DBUS_INT_TO_POINTER (i), FALSE, &iter))
        goto out;

      out_value = _dbus_hash_iter_get_value (&iter);
      _dbus_test_check (out_value != NULL);
      _dbus_test_check (strcmp (out_value, keys[i]) == 0);

      /* Iterate just to be sure it works, though
       * it's a stupid thing to do
       */
      while (_dbus_hash_iter_next (&iter))
        ;

      ++i;
    }

  --i;
  while (i >= 0)
    {
      if (!_dbus_hash_iter_lookup (table1, keys[i], FALSE, &iter))
        _dbus_test_fatal ("hash entry should have existed");
      _dbus_hash_iter_remove_entry (&iter);

      if (!_dbus_hash_iter_lookup (table2, _DBUS_INT_TO_POINTER (i), FALSE, &iter))
        _dbus_test_fatal ("hash entry should have existed");
      _dbus_hash_iter_remove_entry (&iter);

      _dbus_test_check (count_entries (table1) == i);
      _dbus_test_check (count_entries (table2) == i);

      --i;
    }

  _dbus_hash_table_unref (table1);
  _dbus_hash_table_unref (table2);

  ret = TRUE;

 out:
  for (i = 0; i < N_HASH_KEYS; i++)
    dbus_free (keys[i]);

  dbus_free (keys);

  dbus_free (str_key);
  dbus_free (str_value);

  return ret;
}

static DBusTestCase test = { "hash", _dbus_hash_test };

int
main (int    argc,
      char **argv)
{
  return _dbus_test_main (argc, argv, 1, &test,
                          DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS,
                          NULL, NULL);
}
