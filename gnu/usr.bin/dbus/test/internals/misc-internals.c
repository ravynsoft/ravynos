/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2002-2011 Red Hat, Inc.
 * Copyright 2006 Julio M. Merino Vidal
 * Copyright 2006 Ralf Habacker
 * Copyright 2011-2019 Collabora Ltd.
 * Copyright 2012 Lennart Poettering
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

#include <dbus/dbus.h>
#include "dbus/dbus-connection-internal.h"
#include "dbus/dbus-internals.h"
#include "dbus/dbus-test.h"
#include "dbus/dbus-test-tap.h"
#include "test/test-utils.h"

#ifdef DBUS_UNIX
#include "dbus/dbus-userdb.h"
#endif

#include "misc-internals.h"

static void
verify_list (DBusList **list)
{
  DBusList *link;
  int length;

  link = *list;

  if (link == NULL)
    return;

  if (link->next == link)
    {
      _dbus_test_check (link->prev == link);
      _dbus_test_check (*list == link);
      return;
    }

  length = 0;
  do
    {
      length += 1;
      _dbus_test_check (link->prev->next == link);
      _dbus_test_check (link->next->prev == link);
      link = link->next;
    }
  while (link != *list);

  _dbus_test_check (length == _dbus_list_get_length (list));

  if (length == 1)
    _dbus_test_check (_dbus_list_length_is_one (list));
  else
    _dbus_test_check (!_dbus_list_length_is_one (list));
}

static dbus_bool_t
is_ascending_sequence (DBusList **list)
{
  DBusList *link;
  int prev;

  prev = _DBUS_INT_MIN;

  link = _dbus_list_get_first_link (list);
  while (link != NULL)
    {
      int v = _DBUS_POINTER_TO_INT (link->data);

      if (v <= prev)
        return FALSE;

      prev = v;

      link = _dbus_list_get_next_link (list, link);
    }

  return TRUE;
}

static dbus_bool_t
is_descending_sequence (DBusList **list)
{
  DBusList *link;
  int prev;

  prev = _DBUS_INT_MAX;

  link = _dbus_list_get_first_link (list);
  while (link != NULL)
    {
      int v = _DBUS_POINTER_TO_INT (link->data);

      if (v >= prev)
        return FALSE;

      prev = v;

      link = _dbus_list_get_next_link (list, link);
    }

  return TRUE;
}

static dbus_bool_t
all_even_values (DBusList **list)
{
  DBusList *link;

  link = _dbus_list_get_first_link (list);
  while (link != NULL)
    {
      int v = _DBUS_POINTER_TO_INT (link->data);

      if ((v % 2) != 0)
        return FALSE;

      link = _dbus_list_get_next_link (list, link);
    }

  return TRUE;
}

static dbus_bool_t
all_odd_values (DBusList **list)
{
  DBusList *link;

  link = _dbus_list_get_first_link (list);
  while (link != NULL)
    {
      int v = _DBUS_POINTER_TO_INT (link->data);

      if ((v % 2) == 0)
        return FALSE;

      link = _dbus_list_get_next_link (list, link);
    }

  return TRUE;
}

static dbus_bool_t
lists_equal (DBusList **list1,
             DBusList **list2)
{
  DBusList *link1;
  DBusList *link2;

  link1 = _dbus_list_get_first_link (list1);
  link2 = _dbus_list_get_first_link (list2);
  while (link1 && link2)
    {
      if (link1->data != link2->data)
        return FALSE;

      link1 = _dbus_list_get_next_link (list1, link1);
      link2 = _dbus_list_get_next_link (list2, link2);
    }

  if (link1 || link2)
    return FALSE;

  return TRUE;
}

/**
 * @ingroup DBusListInternals
 * Unit test for DBusList
 * @returns #TRUE on success.
 */
static dbus_bool_t
_dbus_list_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusList *list1;
  DBusList *list2;
  DBusList *link1;
  DBusList *link2;
  DBusList *copy1;
  DBusList *copy2;
  int i;

  list1 = NULL;
  list2 = NULL;

  /* Test append and prepend */

  i = 0;
  while (i < 10)
    {
      if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for append");

      if (!_dbus_list_prepend (&list2, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("count not allocate for prepend");
      ++i;

      verify_list (&list1);
      verify_list (&list2);

      _dbus_test_check (_dbus_list_get_length (&list1) == i);
      _dbus_test_check (_dbus_list_get_length (&list2) == i);
    }

  _dbus_test_check (is_ascending_sequence (&list1));
  _dbus_test_check (is_descending_sequence (&list2));

  /* Test list clear */
  _dbus_list_clear (&list1);
  _dbus_list_clear (&list2);

  verify_list (&list1);
  verify_list (&list2);

  /* Test get_first, get_last, pop_first, pop_last */

  i = 0;
  while (i < 10)
    {
      if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for append");
      if (!_dbus_list_prepend (&list2, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for prepend");
      ++i;
    }

  --i;
  while (i >= 0)
    {
      void *got_data1;
      void *got_data2;

      void *data1;
      void *data2;

      got_data1 = _dbus_list_get_last (&list1);
      got_data2 = _dbus_list_get_first (&list2);

      data1 = _dbus_list_pop_last (&list1);
      data2 = _dbus_list_pop_first (&list2);

      _dbus_test_check (got_data1 == data1);
      _dbus_test_check (got_data2 == data2);

      _dbus_test_check (_DBUS_POINTER_TO_INT (data1) == i);
      _dbus_test_check (_DBUS_POINTER_TO_INT (data2) == i);

      verify_list (&list1);
      verify_list (&list2);

      _dbus_test_check (is_ascending_sequence (&list1));
      _dbus_test_check (is_descending_sequence (&list2));

      --i;
    }

  _dbus_test_check (list1 == NULL);
  _dbus_test_check (list2 == NULL);

  /* Test get_first_link, get_last_link, pop_first_link, pop_last_link */

  i = 0;
  while (i < 10)
    {
      if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for append");
      if (!_dbus_list_prepend (&list2, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for prepend");
      ++i;
    }

  --i;
  while (i >= 0)
    {
      DBusList *got_link1;
      DBusList *got_link2;

      void *data1_indirect;
      void *data1;
      void *data2;

      got_link1 = _dbus_list_get_last_link (&list1);
      got_link2 = _dbus_list_get_first_link (&list2);

      link2 = _dbus_list_pop_first_link (&list2);

      _dbus_test_check (got_link2 == link2);

      data1_indirect = got_link1->data;
      /* this call makes got_link1 invalid */
      data1 = _dbus_list_pop_last (&list1);
      _dbus_test_check (data1 == data1_indirect);
      data2 = link2->data;

      _dbus_list_free_link (link2);

      _dbus_test_check (_DBUS_POINTER_TO_INT (data1) == i);
      _dbus_test_check (_DBUS_POINTER_TO_INT (data2) == i);

      verify_list (&list1);
      verify_list (&list2);

      _dbus_test_check (is_ascending_sequence (&list1));
      _dbus_test_check (is_descending_sequence (&list2));

      --i;
    }

  _dbus_test_check (list1 == NULL);
  _dbus_test_check (list2 == NULL);

  /* Test iteration */

  i = 0;
  while (i < 10)
    {
      if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for append");
      if (!_dbus_list_prepend (&list2, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for prepend");
      ++i;

      verify_list (&list1);
      verify_list (&list2);

      _dbus_test_check (_dbus_list_get_length (&list1) == i);
      _dbus_test_check (_dbus_list_get_length (&list2) == i);
    }

  _dbus_test_check (is_ascending_sequence (&list1));
  _dbus_test_check (is_descending_sequence (&list2));

  --i;
  link2 = _dbus_list_get_first_link (&list2);
  while (link2 != NULL)
    {
      verify_list (&link2); /* pretend this link is the head */

      _dbus_test_check (_DBUS_POINTER_TO_INT (link2->data) == i);

      link2 = _dbus_list_get_next_link (&list2, link2);
      --i;
    }

  i = 0;
  link1 = _dbus_list_get_first_link (&list1);
  while (link1 != NULL)
    {
      verify_list (&link1); /* pretend this link is the head */

      _dbus_test_check (_DBUS_POINTER_TO_INT (link1->data) == i);

      link1 = _dbus_list_get_next_link (&list1, link1);
      ++i;
    }

  --i;
  link1 = _dbus_list_get_last_link (&list1);
  while (link1 != NULL)
    {
      verify_list (&link1); /* pretend this link is the head */

      _dbus_test_check (_DBUS_POINTER_TO_INT (link1->data) == i);

      link1 = _dbus_list_get_prev_link (&list1, link1);
      --i;
    }

  _dbus_list_clear (&list1);
  _dbus_list_clear (&list2);

  /* Test remove */

  i = 0;
  while (i < 10)
    {
      if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for append");
      if (!_dbus_list_prepend (&list2, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for prepend");
      ++i;
    }

  --i;
  while (i >= 0)
    {
      if ((i % 2) == 0)
        {
          if (!_dbus_list_remove (&list1, _DBUS_INT_TO_POINTER (i)))
            _dbus_test_fatal ("element should have been in list");
          if (!_dbus_list_remove (&list2, _DBUS_INT_TO_POINTER (i)))
            _dbus_test_fatal ("element should have been in list");

          verify_list (&list1);
          verify_list (&list2);
        }
      --i;
    }

  _dbus_test_check (all_odd_values (&list1));
  _dbus_test_check (all_odd_values (&list2));

  _dbus_list_clear (&list1);
  _dbus_list_clear (&list2);

  /* test removing the other half of the elements */

  i = 0;
  while (i < 10)
    {
      if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for append");
      if (!_dbus_list_prepend (&list2, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for prepend");
      ++i;
    }

  --i;
  while (i >= 0)
    {
      if ((i % 2) != 0)
        {
          if (!_dbus_list_remove (&list1, _DBUS_INT_TO_POINTER (i)))
            _dbus_test_fatal ("element should have been in list");
          if (!_dbus_list_remove (&list2, _DBUS_INT_TO_POINTER (i)))
            _dbus_test_fatal ("element should have been in list");

          verify_list (&list1);
          verify_list (&list2);
        }
      --i;
    }

  _dbus_test_check (all_even_values (&list1));
  _dbus_test_check (all_even_values (&list2));

  /* clear list using remove_link */
  while (list1 != NULL)
    {
      _dbus_list_remove_link (&list1, list1);
      verify_list (&list1);
    }
  while (list2 != NULL)
    {
      _dbus_list_remove_link (&list2, list2);
      verify_list (&list2);
    }

  /* Test remove link more generally */
  i = 0;
  while (i < 10)
    {
      if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for append");
      if (!_dbus_list_prepend (&list2, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for prepend");
      ++i;
    }

  --i;
  link2 = _dbus_list_get_first_link (&list2);
  while (link2 != NULL)
    {
      DBusList *next = _dbus_list_get_next_link (&list2, link2);

      _dbus_test_check (_DBUS_POINTER_TO_INT (link2->data) == i);

      if ((i % 2) == 0)
        _dbus_list_remove_link (&list2, link2);

      verify_list (&list2);

      link2 = next;
      --i;
    }

  _dbus_test_check (all_odd_values (&list2));
  _dbus_list_clear (&list2);

  i = 0;
  link1 = _dbus_list_get_first_link (&list1);
  while (link1 != NULL)
    {
      DBusList *next = _dbus_list_get_next_link (&list1, link1);

      _dbus_test_check (_DBUS_POINTER_TO_INT (link1->data) == i);

      if ((i % 2) != 0)
        _dbus_list_remove_link (&list1, link1);

      verify_list (&list1);

      link1 = next;
      ++i;
    }

  _dbus_test_check (all_even_values (&list1));
  _dbus_list_clear (&list1);

  /* Test copying a list */
  i = 0;
  while (i < 10)
    {
      if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for append");
      if (!_dbus_list_prepend (&list2, _DBUS_INT_TO_POINTER (i)))
        _dbus_test_fatal ("could not allocate for prepend");
      ++i;
    }

  /* bad pointers, because they are allowed in the copy dest */
  copy1 = _DBUS_INT_TO_POINTER (0x342234);
  copy2 = _DBUS_INT_TO_POINTER (23);

  _dbus_list_copy (&list1, &copy1);
  verify_list (&list1);
  verify_list (&copy1);
  _dbus_test_check (lists_equal (&list1, &copy1));

  _dbus_list_copy (&list2, &copy2);
  verify_list (&list2);
  verify_list (&copy2);
  _dbus_test_check (lists_equal (&list2, &copy2));

  /* Now test copying empty lists */
  _dbus_list_clear (&list1);
  _dbus_list_clear (&list2);
  _dbus_list_clear (&copy1);
  _dbus_list_clear (&copy2);

  /* bad pointers, because they are allowed in the copy dest */
  copy1 = _DBUS_INT_TO_POINTER (0x342234);
  copy2 = _DBUS_INT_TO_POINTER (23);

  _dbus_list_copy (&list1, &copy1);
  verify_list (&list1);
  verify_list (&copy1);
  _dbus_test_check (lists_equal (&list1, &copy1));

  _dbus_list_copy (&list2, &copy2);
  verify_list (&list2);
  verify_list (&copy2);
  _dbus_test_check (lists_equal (&list2, &copy2));

  _dbus_list_clear (&list1);
  _dbus_list_clear (&list2);

  /* insert_after on empty list */
  _dbus_list_insert_after (&list1, NULL,
                           _DBUS_INT_TO_POINTER (0));
  verify_list (&list1);

  /* inserting after first element */
  _dbus_list_insert_after (&list1, list1,
                           _DBUS_INT_TO_POINTER (1));
  verify_list (&list1);
  _dbus_test_check (is_ascending_sequence (&list1));

  /* inserting at the end */
  _dbus_list_insert_after (&list1, list1->next,
                           _DBUS_INT_TO_POINTER (2));
  verify_list (&list1);
  _dbus_test_check (is_ascending_sequence (&list1));

  /* using insert_after to prepend */
  _dbus_list_insert_after (&list1, NULL,
                           _DBUS_INT_TO_POINTER (-1));
  verify_list (&list1);
  _dbus_test_check (is_ascending_sequence (&list1));

  _dbus_list_clear (&list1);

  /* using remove_last */
  if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (2)))
    _dbus_test_fatal ("could not allocate for append");
  if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (1)))
    _dbus_test_fatal ("could not allocate for append");
  if (!_dbus_list_append (&list1, _DBUS_INT_TO_POINTER (3)))
    _dbus_test_fatal ("could not allocate for append");

  _dbus_list_remove_last (&list1, _DBUS_INT_TO_POINTER (2));

  verify_list (&list1);
  _dbus_test_check (is_ascending_sequence (&list1));

  _dbus_list_clear (&list1);

  return TRUE;
}

static dbus_bool_t
_dbus_misc_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  int major, minor, micro;
  DBusString str;

  /* make sure we don't crash on NULL */
  dbus_get_version (NULL, NULL, NULL);

  /* Now verify that all the compile-time version stuff
   * is right and matches the runtime. These tests
   * are mostly intended to catch various kinds of
   * typo (mixing up major and minor, that sort of thing).
   */
  dbus_get_version (&major, &minor, &micro);

  _dbus_test_check (major == DBUS_MAJOR_VERSION);
  _dbus_test_check (minor == DBUS_MINOR_VERSION);
  _dbus_test_check (micro == DBUS_MICRO_VERSION);

#define MAKE_VERSION(x, y, z) (((x) << 16) | ((y) << 8) | (z))

  /* check that MAKE_VERSION works and produces the intended ordering */
  _dbus_test_check (MAKE_VERSION (1, 0, 0) > MAKE_VERSION (0, 0, 0));
  _dbus_test_check (MAKE_VERSION (1, 1, 0) > MAKE_VERSION (1, 0, 0));
  _dbus_test_check (MAKE_VERSION (1, 1, 1) > MAKE_VERSION (1, 1, 0));

  _dbus_test_check (MAKE_VERSION (2, 0, 0) > MAKE_VERSION (1, 1, 1));
  _dbus_test_check (MAKE_VERSION (2, 1, 0) > MAKE_VERSION (1, 1, 1));
  _dbus_test_check (MAKE_VERSION (2, 1, 1) > MAKE_VERSION (1, 1, 1));

  /* check DBUS_VERSION */
  _dbus_test_check (MAKE_VERSION (major, minor, micro) == DBUS_VERSION);

  /* check that ordering works with DBUS_VERSION */
  _dbus_test_check (MAKE_VERSION (major - 1, minor, micro) < DBUS_VERSION);
  _dbus_test_check (MAKE_VERSION (major, minor - 1, micro) < DBUS_VERSION);
  _dbus_test_check (MAKE_VERSION (major, minor, micro - 1) < DBUS_VERSION);

  _dbus_test_check (MAKE_VERSION (major + 1, minor, micro) > DBUS_VERSION);
  _dbus_test_check (MAKE_VERSION (major, minor + 1, micro) > DBUS_VERSION);
  _dbus_test_check (MAKE_VERSION (major, minor, micro + 1) > DBUS_VERSION);

  /* Check DBUS_VERSION_STRING */

  if (!_dbus_string_init (&str))
    _dbus_test_fatal ("no memory");

  if (!(_dbus_string_append_int (&str, major) &&
        _dbus_string_append_byte (&str, '.') &&
        _dbus_string_append_int (&str, minor) &&
        _dbus_string_append_byte (&str, '.') &&
        _dbus_string_append_int (&str, micro)))
    _dbus_test_fatal ("no memory");

  _dbus_test_check (_dbus_string_equal_c_str (&str, DBUS_VERSION_STRING));

  _dbus_string_free (&str);

  return TRUE;
}

static dbus_bool_t
_dbus_server_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  const char *valid_addresses[] = {
    "tcp:port=1234",
    "tcp:host=localhost,port=1234",
    "tcp:host=localhost,port=1234;tcp:port=5678",
#ifdef DBUS_UNIX
    "unix:path=./boogie",
    "tcp:port=1234;unix:path=./boogie",
#endif
  };

  DBusServer *server;
  int i;

  for (i = 0; i < _DBUS_N_ELEMENTS (valid_addresses); i++)
    {
      DBusError error = DBUS_ERROR_INIT;
      char *address;
      char *id;

      server = dbus_server_listen (valid_addresses[i], &error);
      if (server == NULL)
        {
          _dbus_warn ("server listen error: %s: %s", error.name, error.message);
          dbus_error_free (&error);
          _dbus_test_fatal ("Failed to listen for valid address.");
        }

      id = dbus_server_get_id (server);
      _dbus_test_check (id != NULL);
      address = dbus_server_get_address (server);
      _dbus_test_check (address != NULL);

      if (strstr (address, id) == NULL)
        {
          _dbus_warn ("server id '%s' is not in the server address '%s'",
                      id, address);
          _dbus_test_fatal ("bad server id or address");
        }

      dbus_free (id);
      dbus_free (address);

      dbus_server_disconnect (server);
      dbus_server_unref (server);
    }

  return TRUE;
}

/**
 * @ingroup DBusSignatureInternals
 * Unit test for DBusSignature.
 *
 * @returns #TRUE on success.
 */
static dbus_bool_t
_dbus_signature_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusSignatureIter iter;
  DBusSignatureIter subiter;
  DBusSignatureIter subsubiter;
  DBusSignatureIter subsubsubiter;
  const char *sig;
  dbus_bool_t boolres;

  sig = "";
  _dbus_test_check (dbus_signature_validate (sig, NULL));
  _dbus_test_check (!dbus_signature_validate_single (sig, NULL));
  dbus_signature_iter_init (&iter, sig);
  _dbus_test_check (dbus_signature_iter_get_current_type (&iter) == DBUS_TYPE_INVALID);

  sig = DBUS_TYPE_STRING_AS_STRING;
  _dbus_test_check (dbus_signature_validate (sig, NULL));
  _dbus_test_check (dbus_signature_validate_single (sig, NULL));
  dbus_signature_iter_init (&iter, sig);
  _dbus_test_check (dbus_signature_iter_get_current_type (&iter) == DBUS_TYPE_STRING);

  sig = DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_BYTE_AS_STRING;
  _dbus_test_check (dbus_signature_validate (sig, NULL));
  dbus_signature_iter_init (&iter, sig);
  _dbus_test_check (dbus_signature_iter_get_current_type (&iter) == DBUS_TYPE_STRING);
  boolres = dbus_signature_iter_next (&iter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&iter) == DBUS_TYPE_BYTE);

  sig = DBUS_TYPE_UINT16_AS_STRING
    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_UINT32_AS_STRING
    DBUS_TYPE_VARIANT_AS_STRING
    DBUS_TYPE_DOUBLE_AS_STRING
    DBUS_STRUCT_END_CHAR_AS_STRING;
  _dbus_test_check (dbus_signature_validate (sig, NULL));
  dbus_signature_iter_init (&iter, sig);
  _dbus_test_check (dbus_signature_iter_get_current_type (&iter) == DBUS_TYPE_UINT16);
  boolres = dbus_signature_iter_next (&iter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&iter) == DBUS_TYPE_STRUCT);
  dbus_signature_iter_recurse (&iter, &subiter);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subiter) == DBUS_TYPE_STRING);
  boolres = dbus_signature_iter_next (&subiter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subiter) == DBUS_TYPE_UINT32);
  boolres = dbus_signature_iter_next (&subiter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subiter) == DBUS_TYPE_VARIANT);
  boolres = dbus_signature_iter_next (&subiter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subiter) == DBUS_TYPE_DOUBLE);

  sig = DBUS_TYPE_UINT16_AS_STRING
    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_UINT32_AS_STRING
    DBUS_TYPE_BYTE_AS_STRING
    DBUS_TYPE_ARRAY_AS_STRING
    DBUS_TYPE_ARRAY_AS_STRING
    DBUS_TYPE_DOUBLE_AS_STRING
    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_BYTE_AS_STRING
    DBUS_STRUCT_END_CHAR_AS_STRING
    DBUS_STRUCT_END_CHAR_AS_STRING;
  _dbus_test_check (dbus_signature_validate (sig, NULL));
  dbus_signature_iter_init (&iter, sig);
  _dbus_test_check (dbus_signature_iter_get_current_type (&iter) == DBUS_TYPE_UINT16);
  boolres = dbus_signature_iter_next (&iter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&iter) == DBUS_TYPE_STRUCT);
  dbus_signature_iter_recurse (&iter, &subiter);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subiter) == DBUS_TYPE_UINT32);
  boolres = dbus_signature_iter_next (&subiter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subiter) == DBUS_TYPE_BYTE);
  boolres = dbus_signature_iter_next (&subiter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subiter) == DBUS_TYPE_ARRAY);
  _dbus_test_check (dbus_signature_iter_get_element_type (&subiter) == DBUS_TYPE_ARRAY);

  dbus_signature_iter_recurse (&subiter, &subsubiter);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subsubiter) == DBUS_TYPE_ARRAY);
  _dbus_test_check (dbus_signature_iter_get_element_type (&subsubiter) == DBUS_TYPE_DOUBLE);

  dbus_signature_iter_recurse (&subsubiter, &subsubsubiter);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subsubsubiter) == DBUS_TYPE_DOUBLE);
  boolres = dbus_signature_iter_next (&subiter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subiter) == DBUS_TYPE_STRUCT);
  dbus_signature_iter_recurse (&subiter, &subsubiter);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subsubiter) == DBUS_TYPE_BYTE);

  sig = DBUS_TYPE_ARRAY_AS_STRING
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_INT16_AS_STRING
    DBUS_TYPE_STRING_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING
    DBUS_TYPE_VARIANT_AS_STRING;
  _dbus_test_check (dbus_signature_validate (sig, NULL));
  _dbus_test_check (!dbus_signature_validate_single (sig, NULL));
  dbus_signature_iter_init (&iter, sig);
  _dbus_test_check (dbus_signature_iter_get_current_type (&iter) == DBUS_TYPE_ARRAY);
  _dbus_test_check (dbus_signature_iter_get_element_type (&iter) == DBUS_TYPE_DICT_ENTRY);

  dbus_signature_iter_recurse (&iter, &subiter);
  dbus_signature_iter_recurse (&subiter, &subsubiter);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subsubiter) == DBUS_TYPE_INT16);
  boolres = dbus_signature_iter_next (&subsubiter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&subsubiter) == DBUS_TYPE_STRING);
  boolres = dbus_signature_iter_next (&subsubiter);
  _dbus_test_check (!boolres);

  boolres = dbus_signature_iter_next (&iter);
  _dbus_test_check (boolres);
  _dbus_test_check (dbus_signature_iter_get_current_type (&iter) == DBUS_TYPE_VARIANT);
  boolres = dbus_signature_iter_next (&iter);
  _dbus_test_check (!boolres);

  sig = DBUS_TYPE_DICT_ENTRY_AS_STRING;
  _dbus_test_check (!dbus_signature_validate (sig, NULL));

  sig = DBUS_TYPE_ARRAY_AS_STRING;
  _dbus_test_check (!dbus_signature_validate (sig, NULL));

  sig = DBUS_TYPE_UINT32_AS_STRING
    DBUS_TYPE_ARRAY_AS_STRING;
  _dbus_test_check (!dbus_signature_validate (sig, NULL));

  sig = DBUS_TYPE_ARRAY_AS_STRING
    DBUS_TYPE_DICT_ENTRY_AS_STRING;
  _dbus_test_check (!dbus_signature_validate (sig, NULL));

  sig = DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING;
  _dbus_test_check (!dbus_signature_validate (sig, NULL));

  sig = DBUS_DICT_ENTRY_END_CHAR_AS_STRING;
  _dbus_test_check (!dbus_signature_validate (sig, NULL));

  sig = DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_INT32_AS_STRING;
  _dbus_test_check (!dbus_signature_validate (sig, NULL));

  sig = DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_INT32_AS_STRING
    DBUS_TYPE_STRING_AS_STRING;
  _dbus_test_check (!dbus_signature_validate (sig, NULL));

  sig = DBUS_STRUCT_END_CHAR_AS_STRING
    DBUS_STRUCT_BEGIN_CHAR_AS_STRING;
  _dbus_test_check (!dbus_signature_validate (sig, NULL));

  sig = DBUS_STRUCT_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_BOOLEAN_AS_STRING;
  _dbus_test_check (!dbus_signature_validate (sig, NULL));
  return TRUE;
#if 0
 oom:
  _dbus_test_fatal ("out of memory");
  return FALSE;
#endif
}

#ifdef DBUS_UNIX
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
static dbus_bool_t
_dbus_transport_unix_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusConnection *c;
  DBusError error;
  dbus_bool_t ret;
  const char *address;

  dbus_error_init (&error);

  c = dbus_connection_open ("unixexec:argv0=false,argv1=foobar,path=/bin/false", &error);
  _dbus_test_check (c != NULL);
  _dbus_test_check (!dbus_error_is_set (&error));

  address = _dbus_connection_get_address (c);
  _dbus_test_check (address != NULL);

  /* Let's see if the address got parsed, reordered and formatted correctly */
  ret = strcmp (address, "unixexec:path=/bin/false,argv0=false,argv1=foobar") == 0;

  dbus_connection_unref (c);

  return ret;
}
#endif

/**
 * Unit test for dbus-userdb.c.
 *
 * @returns #TRUE on success.
 */
static dbus_bool_t
_dbus_userdb_test (const char *test_data_dir)
{
  const DBusString *username;
  const DBusString *homedir;
  dbus_uid_t uid;
  unsigned long *group_ids;
  int n_group_ids, i;
  DBusError error = DBUS_ERROR_INIT;

  if (!_dbus_username_from_current_process (&username))
    _dbus_test_fatal ("didn't get username");

  if (!_dbus_homedir_from_current_process (&homedir))
    _dbus_test_fatal ("didn't get homedir");

  if (!_dbus_get_user_id (username, &uid))
    _dbus_test_fatal ("didn't get uid");

  if (!_dbus_groups_from_uid (uid, &group_ids, &n_group_ids, &error))
    _dbus_test_fatal ("didn't get groups: %s: %s", error.name, error.message);

  _dbus_test_diag ("    Current user: %s homedir: %s gids:",
          _dbus_string_get_const_data (username),
          _dbus_string_get_const_data (homedir));

  for (i=0; i<n_group_ids; i++)
      _dbus_test_diag ("- %ld", group_ids[i]);

  dbus_error_init (&error);
  _dbus_test_diag ("Is Console user: %i",
          _dbus_is_console_user (uid, &error));
  _dbus_test_diag ("Invocation was OK: %s", error.message ? error.message : "yes");
  dbus_error_free (&error);
  _dbus_test_diag ("Is Console user 4711: %i",
          _dbus_is_console_user (4711, &error));
  _dbus_test_diag ("Invocation was OK: %s", error.message ? error.message : "yes");
  dbus_error_free (&error);

  dbus_free (group_ids);

  return TRUE;
}
#endif

static DBusTestCase tests[] =
{
  { "misc", _dbus_misc_test },
  { "address", _dbus_address_test },
  { "server", _dbus_server_test },
  { "signature", _dbus_signature_test },
  { "mem-pool", _dbus_mem_pool_test },
  { "list", _dbus_list_test },

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  { "auth", _dbus_auth_test },
  { "byteswap", _dbus_marshal_byteswap_test },
  { "credentials", _dbus_credentials_test },
  { "data-slot", _dbus_data_slot_test },
  { "keyring", _dbus_keyring_test },
  { "marshal-validate", _dbus_marshal_validate_test },
  { "marshalling", _dbus_marshal_test },
  { "memory", _dbus_memory_test },
  { "object-tree", _dbus_object_tree_test },
  { "sha", _dbus_sha_test },
  { "string", _dbus_string_test },
  { "sysdeps", _dbus_sysdeps_test },
#endif

#if defined(DBUS_UNIX)
  { "userdb", _dbus_userdb_test },
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
  { "transport-unix", _dbus_transport_unix_test },
#endif
#endif

  { NULL }
};

int
main (int    argc,
      char **argv)
{
  return _dbus_test_main (argc, argv, _DBUS_N_ELEMENTS (tests), tests,
                          DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS,
                          NULL, NULL);
}
