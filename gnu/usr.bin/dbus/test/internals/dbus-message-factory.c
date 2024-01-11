/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-message-factory.c Generator of valid and invalid message data for test suite
 *
 * Copyright (C) 2005 Red Hat Inc.
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

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
#include "dbus-message-factory.h"

#include "dbus/dbus-message-private.h"
#include "dbus/dbus-signature.h"
#include "dbus/dbus-test.h"
#include "dbus/dbus-test-tap.h"

#include "dbus-marshal-recursive-util.h"

#include <stdio.h>

typedef enum
  {
    CHANGE_TYPE_ADJUST,
    CHANGE_TYPE_ABSOLUTE
  } ChangeType;

#define BYTE_ORDER_OFFSET  0
#define TYPE_OFFSET        1
#define BODY_LENGTH_OFFSET 4
#define FIELDS_ARRAY_LENGTH_OFFSET 12

static void
iter_recurse (DBusMessageDataIter *iter)
{
  iter->depth += 1;
  _dbus_assert (iter->depth < _DBUS_MESSAGE_DATA_MAX_NESTING);
  _dbus_assert (iter->sequence_nos[iter->depth] >= 0);
}

static int
iter_get_sequence (DBusMessageDataIter *iter)
{
  _dbus_assert (iter->sequence_nos[iter->depth] >= 0);
  return iter->sequence_nos[iter->depth];
}

static void
iter_set_sequence (DBusMessageDataIter *iter,
                   int                  sequence)
{
  _dbus_assert (sequence >= 0);
  iter->sequence_nos[iter->depth] = sequence;
}

static void
iter_unrecurse (DBusMessageDataIter *iter)
{
  iter->depth -= 1;
  _dbus_assert (iter->depth >= 0);
}

static void
iter_next (DBusMessageDataIter *iter)
{
  iter->sequence_nos[iter->depth] += 1;
}

static dbus_bool_t
iter_first_in_series (DBusMessageDataIter *iter)
{
  int i;

  i = iter->depth;
  while (i < _DBUS_MESSAGE_DATA_MAX_NESTING)
    {
      if (iter->sequence_nos[i] != 0)
        return FALSE;
      ++i;
    }
  return TRUE;
}

typedef dbus_bool_t (* DBusInnerGeneratorFunc)   (DBusMessageDataIter *iter,
                                                  DBusMessage        **message_p);
typedef dbus_bool_t (* DBusMessageGeneratorFunc) (DBusMessageDataIter *iter,
                                                  DBusString          *data,
                                                  DBusValidity        *expected_validity);

static void
set_reply_serial (DBusMessage *message)
{
  if (message == NULL)
    _dbus_test_fatal ("oom");
  if (!dbus_message_set_reply_serial (message, 100))
    _dbus_test_fatal ("oom");
}

static dbus_bool_t
generate_trivial_inner (DBusMessageDataIter *iter,
                        DBusMessage        **message_p)
{
  DBusMessage *message;

  switch (iter_get_sequence (iter))
    {
    case 0:
      message = dbus_message_new_method_call ("org.freedesktop.TextEditor",
                                              "/foo/bar",
                                              "org.freedesktop.DocumentFactory",
                                              "Create");
      break;
    case 1:
      message = dbus_message_new (DBUS_MESSAGE_TYPE_METHOD_RETURN);
      set_reply_serial (message);
      break;
    case 2:
      message = dbus_message_new_signal ("/foo/bar",
                                         "org.freedesktop.DocumentFactory",
                                         "Created");
      break;
    case 3:
      message = dbus_message_new (DBUS_MESSAGE_TYPE_ERROR);

      if (!dbus_message_set_error_name (message,
                                        "org.freedesktop.TestErrorName"))
        _dbus_test_fatal ("oom");

      {
        DBusMessageIter iter2;
        const char *v_STRING = "This is an error";

        dbus_message_iter_init_append (message, &iter2);
        if (!dbus_message_iter_append_basic (&iter2,
                                             DBUS_TYPE_STRING,
                                             &v_STRING))
          _dbus_test_fatal ("oom");
      }

      set_reply_serial (message);
      break;
    default:
      return FALSE;
    }

  if (message == NULL)
    _dbus_test_fatal ("oom");

  *message_p = message;

  return TRUE;
}

static dbus_bool_t
generate_many_bodies_inner (DBusMessageDataIter *iter,
                            DBusMessage        **message_p)
{
  DBusMessage *message;
  DBusString signature;
  DBusString body;
  char byte_order;

  /* Keeping this small makes things go faster */
  message = dbus_message_new_method_call ("o.z.F",
                                          "/",
                                          "o.z.B",
                                          "Nah");
  if (message == NULL)
    _dbus_test_fatal ("oom");

  byte_order = _dbus_header_get_byte_order (&message->header);

  set_reply_serial (message);

  if (!_dbus_string_init (&signature) || !_dbus_string_init (&body))
    _dbus_test_fatal ("oom");

  if (_dbus_test_generate_bodies (iter_get_sequence (iter), byte_order,
                                  &signature, &body))
    {
      const char *v_SIGNATURE;

      v_SIGNATURE = _dbus_string_get_const_data (&signature);
      if (!_dbus_header_set_field_basic (&message->header,
                                         DBUS_HEADER_FIELD_SIGNATURE,
                                         DBUS_TYPE_SIGNATURE,
                                         &v_SIGNATURE))
        _dbus_test_fatal ("oom");

      if (!_dbus_string_move (&body, 0, &message->body, 0))
        _dbus_test_fatal ("oom");

      _dbus_marshal_set_uint32 (&message->header.data, BODY_LENGTH_OFFSET,
                                _dbus_string_get_length (&message->body),
                                byte_order);

      *message_p = message;
    }
  else
    {
      dbus_message_unref (message);
      *message_p = NULL;
    }

  _dbus_string_free (&signature);
  _dbus_string_free (&body);

  return *message_p != NULL;
}

static void
generate_from_message (DBusString            *data,
                       DBusValidity          *expected_validity,
                       DBusMessage           *message)
{
  dbus_message_set_serial (message, 1);
  dbus_message_lock (message);

  *expected_validity = DBUS_VALID;

  /* move for efficiency, since we'll nuke the message anyway */
  if (!_dbus_string_move (&message->header.data, 0,
                          data, 0))
    _dbus_test_fatal ("oom");

  if (!_dbus_string_copy (&message->body, 0,
                          data, _dbus_string_get_length (data)))
    _dbus_test_fatal ("oom");
}

static dbus_bool_t
generate_outer (DBusMessageDataIter   *iter,
                DBusString            *data,
                DBusValidity          *expected_validity,
                DBusInnerGeneratorFunc func)
{
  DBusMessage *message;

  message = NULL;
  if (!(*func)(iter, &message))
    return FALSE;

  iter_next (iter);

  _dbus_assert (message != NULL);

  generate_from_message (data, expected_validity, message);

  dbus_message_unref (message);

  return TRUE;
}

static dbus_bool_t
generate_trivial (DBusMessageDataIter   *iter,
                  DBusString            *data,
                  DBusValidity          *expected_validity)
{
  return generate_outer (iter, data, expected_validity,
                         generate_trivial_inner);
}

static dbus_bool_t
generate_many_bodies (DBusMessageDataIter   *iter,
                      DBusString            *data,
                      DBusValidity          *expected_validity)
{
  return generate_outer (iter, data, expected_validity,
                         generate_many_bodies_inner);
}

static DBusMessage*
simple_method_call (void)
{
  DBusMessage *message;
  /* Keeping this small makes stuff go faster */
  message = dbus_message_new_method_call ("o.b.Q",
                                          "/f/b",
                                          "o.b.Z",
                                          "Fro");
  if (message == NULL)
    _dbus_test_fatal ("oom");
  return message;
}

static DBusMessage*
simple_signal (void)
{
  DBusMessage *message;
  message = dbus_message_new_signal ("/f/b",
                                     "o.b.Z",
                                     "Fro");
  if (message == NULL)
    _dbus_test_fatal ("oom");
  return message;
}

static DBusMessage*
simple_method_return (void)
{
  DBusMessage *message;
  message =  dbus_message_new (DBUS_MESSAGE_TYPE_METHOD_RETURN);
  if (message == NULL)
    _dbus_test_fatal ("oom");

  set_reply_serial (message);

  return message;
}

static DBusMessage*
simple_error (void)
{
  DBusMessage *message;
  message =  dbus_message_new (DBUS_MESSAGE_TYPE_ERROR);
  if (message == NULL)
    _dbus_test_fatal ("oom");

  if (!dbus_message_set_error_name (message, "foo.bar"))
    _dbus_test_fatal ("oom");

  set_reply_serial (message);

  return message;
}

static DBusMessage*
message_with_nesting_levels (int levels)
{
  DBusMessage *message;
  dbus_int32_t v_INT32;
  DBusMessageIter *parents;
  DBusMessageIter *children;
  int i;

  /* If levels is higher it breaks sig_refcount in DBusMessageRealIter
   * in dbus-message.c, this assert is just to help you know you need
   * to fix that if you hit it
   */
  _dbus_assert (levels < 256);

  parents = dbus_new(DBusMessageIter, levels + 1);
  children = dbus_new(DBusMessageIter, levels + 1);

  v_INT32 = 42;
  message = simple_method_call ();

  i = 0;
  dbus_message_iter_init_append (message, &parents[i]);
  while (i < levels)
    {
      if (!dbus_message_iter_open_container (&parents[i], DBUS_TYPE_VARIANT,
                                             i == (levels - 1) ?
                                             DBUS_TYPE_INT32_AS_STRING :
                                             DBUS_TYPE_VARIANT_AS_STRING,
                                             &children[i]))
        _dbus_test_fatal ("oom");
      ++i;
      parents[i] = children[i-1];
    }
  --i;

  if (!dbus_message_iter_append_basic (&children[i], DBUS_TYPE_INT32, &v_INT32))
    _dbus_test_fatal ("oom");

  while (i >= 0)
    {
      if (!dbus_message_iter_close_container (&parents[i], &children[i]))
        _dbus_test_fatal ("oom");
      --i;
    }

  dbus_free(parents);
  dbus_free(children);

  return message;
}

static dbus_bool_t
generate_special (DBusMessageDataIter   *iter,
                  DBusString            *data,
                  DBusValidity          *expected_validity)
{
  int item_seq;
  DBusMessage *message;
  int pos;
  dbus_int32_t v_INT32;

  _dbus_assert (_dbus_string_get_length (data) == 0);

  message = NULL;
  pos = -1;
  v_INT32 = 42;
  item_seq = iter_get_sequence (iter);

  if (item_seq == 0)
    {
      message = simple_method_call ();
      if (!dbus_message_append_args (message,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INVALID))
        _dbus_test_fatal ("oom");

      _dbus_header_get_field_raw (&message->header,
                                  DBUS_HEADER_FIELD_SIGNATURE,
                                  NULL, &pos);
      generate_from_message (data, expected_validity, message);

      /* set an invalid typecode */
      _dbus_string_set_byte (data, pos + 1, '$');

      *expected_validity = DBUS_INVALID_UNKNOWN_TYPECODE;
    }
  else if (item_seq == 1)
    {
      char long_sig[DBUS_MAXIMUM_TYPE_RECURSION_DEPTH+2];
      const char *v_STRING;
      int i;

      message = simple_method_call ();
      if (!dbus_message_append_args (message,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INVALID))
        _dbus_test_fatal ("oom");

      i = 0;
      while (i < (DBUS_MAXIMUM_TYPE_RECURSION_DEPTH + 1))
        {
          long_sig[i] = DBUS_TYPE_ARRAY;
          ++i;
        }
      long_sig[i] = DBUS_TYPE_INVALID;

      v_STRING = long_sig;
      if (!_dbus_header_set_field_basic (&message->header,
                                         DBUS_HEADER_FIELD_SIGNATURE,
                                         DBUS_TYPE_SIGNATURE,
                                         &v_STRING))
        _dbus_test_fatal ("oom");

      _dbus_header_get_field_raw (&message->header,
                                  DBUS_HEADER_FIELD_SIGNATURE,
                                  NULL, &pos);
      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_INVALID_EXCEEDED_MAXIMUM_ARRAY_RECURSION;
    }
  else if (item_seq == 2)
    {
      char long_sig[DBUS_MAXIMUM_TYPE_RECURSION_DEPTH*2+4];
      const char *v_STRING;
      int i;

      message = simple_method_call ();
      if (!dbus_message_append_args (message,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INVALID))
        _dbus_test_fatal ("oom");

      i = 0;
      while (i <= (DBUS_MAXIMUM_TYPE_RECURSION_DEPTH + 1))
        {
          long_sig[i] = DBUS_STRUCT_BEGIN_CHAR;
          ++i;
        }

      long_sig[i] = DBUS_TYPE_INT32;
      ++i;

      while (i < (DBUS_MAXIMUM_TYPE_RECURSION_DEPTH*2 + 3))
        {
          long_sig[i] = DBUS_STRUCT_END_CHAR;
          ++i;
        }
      long_sig[i] = DBUS_TYPE_INVALID;

      v_STRING = long_sig;
      if (!_dbus_header_set_field_basic (&message->header,
                                         DBUS_HEADER_FIELD_SIGNATURE,
                                         DBUS_TYPE_SIGNATURE,
                                         &v_STRING))
        _dbus_test_fatal ("oom");

      _dbus_header_get_field_raw (&message->header,
                                  DBUS_HEADER_FIELD_SIGNATURE,
                                  NULL, &pos);
      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_INVALID_EXCEEDED_MAXIMUM_STRUCT_RECURSION;
    }
  else if (item_seq == 3)
    {
      message = simple_method_call ();
      if (!dbus_message_append_args (message,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INVALID))
        _dbus_test_fatal ("oom");

      _dbus_header_get_field_raw (&message->header,
                                  DBUS_HEADER_FIELD_SIGNATURE,
                                  NULL, &pos);
      generate_from_message (data, expected_validity, message);

      _dbus_string_set_byte (data, pos + 1, DBUS_STRUCT_BEGIN_CHAR);

      *expected_validity = DBUS_INVALID_STRUCT_STARTED_BUT_NOT_ENDED;
    }
  else if (item_seq == 4)
    {
      message = simple_method_call ();
      if (!dbus_message_append_args (message,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INVALID))
        _dbus_test_fatal ("oom");

      _dbus_header_get_field_raw (&message->header,
                                  DBUS_HEADER_FIELD_SIGNATURE,
                                  NULL, &pos);
      generate_from_message (data, expected_validity, message);

      _dbus_string_set_byte (data, pos + 1, DBUS_STRUCT_END_CHAR);

      *expected_validity = DBUS_INVALID_STRUCT_ENDED_BUT_NOT_STARTED;
    }
  else if (item_seq == 5)
    {
      message = simple_method_call ();
      if (!dbus_message_append_args (message,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INVALID))
        _dbus_test_fatal ("oom");

      _dbus_header_get_field_raw (&message->header,
                                  DBUS_HEADER_FIELD_SIGNATURE,
                                  NULL, &pos);
      generate_from_message (data, expected_validity, message);

      _dbus_string_set_byte (data, pos + 1, DBUS_STRUCT_BEGIN_CHAR);
      _dbus_string_set_byte (data, pos + 2, DBUS_STRUCT_END_CHAR);

      *expected_validity = DBUS_INVALID_STRUCT_HAS_NO_FIELDS;
    }
  else if (item_seq == 6)
    {
      message = simple_method_call ();
      generate_from_message (data, expected_validity, message);

      _dbus_string_set_byte (data, TYPE_OFFSET, DBUS_MESSAGE_TYPE_INVALID);

      *expected_validity = DBUS_INVALID_BAD_MESSAGE_TYPE;
    }
  else if (item_seq == 7)
    {
      /* Messages of unknown type are considered valid */
      message = simple_method_call ();
      generate_from_message (data, expected_validity, message);

      _dbus_string_set_byte (data, TYPE_OFFSET, 100);

      *expected_validity = DBUS_VALID;
    }
  else if (item_seq == 8)
    {
      char byte_order;

      message = simple_method_call ();
      byte_order = _dbus_header_get_byte_order (&message->header);
      generate_from_message (data, expected_validity, message);

      _dbus_marshal_set_uint32 (data, BODY_LENGTH_OFFSET,
                                DBUS_MAXIMUM_MESSAGE_LENGTH / 2 + 4,
                                byte_order);
      _dbus_marshal_set_uint32 (data, FIELDS_ARRAY_LENGTH_OFFSET,
                                DBUS_MAXIMUM_MESSAGE_LENGTH / 2 + 4,
                                byte_order);
      *expected_validity = DBUS_INVALID_MESSAGE_TOO_LONG;
    }
  else if (item_seq == 9)
    {
      const char *v_STRING = "not a valid bus name";
      message = simple_method_call ();

      if (!_dbus_header_set_field_basic (&message->header,
                                         DBUS_HEADER_FIELD_SENDER,
                                         DBUS_TYPE_STRING, &v_STRING))
        _dbus_test_fatal ("oom");

      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_INVALID_BAD_SENDER;
    }
  else if (item_seq == 10)
    {
      message = simple_method_call ();

      if (!dbus_message_set_interface (message, DBUS_INTERFACE_LOCAL))
        _dbus_test_fatal ("oom");

      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_INVALID_USES_LOCAL_INTERFACE;
    }
  else if (item_seq == 11)
    {
      message = simple_method_call ();

      if (!dbus_message_set_path (message, DBUS_PATH_LOCAL))
        _dbus_test_fatal ("oom");

      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_INVALID_USES_LOCAL_PATH;
    }
  else if (item_seq == 12)
    {
      /* Method calls don't have to have interface */
      message = simple_method_call ();

      if (!dbus_message_set_interface (message, NULL))
        _dbus_test_fatal ("oom");

      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_VALID;
    }
  else if (item_seq == 13)
    {
      /* Signals require an interface */
      message = simple_signal ();

      if (!dbus_message_set_interface (message, NULL))
        _dbus_test_fatal ("oom");

      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_INVALID_MISSING_INTERFACE;
    }
  else if (item_seq == 14)
    {
      message = simple_method_return ();

      if (!_dbus_header_delete_field (&message->header, DBUS_HEADER_FIELD_REPLY_SERIAL))
        _dbus_test_fatal ("oom");

      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_INVALID_MISSING_REPLY_SERIAL;
    }
  else if (item_seq == 15)
    {
      message = simple_error ();

      if (!dbus_message_set_error_name (message, NULL))
        _dbus_test_fatal ("oom");

      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_INVALID_MISSING_ERROR_NAME;
    }
  else if (item_seq == 16)
    {
      char long_sig[DBUS_MAXIMUM_TYPE_RECURSION_DEPTH*4+10];
      const char *v_STRING;
      int i;
      int n_begins;

      message = simple_method_call ();
      if (!dbus_message_append_args (message,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INVALID))
        _dbus_test_fatal ("oom");

      i = 0;
      while (i <= (DBUS_MAXIMUM_TYPE_RECURSION_DEPTH*3 + 3))
        {
          long_sig[i] = DBUS_TYPE_ARRAY;
          ++i;
          long_sig[i] = DBUS_DICT_ENTRY_BEGIN_CHAR;
          ++i;
          long_sig[i] = DBUS_TYPE_INT32;
          ++i;
        }
      n_begins = i / 3;

      long_sig[i] = DBUS_TYPE_INT32;
      ++i;

      while (n_begins > 0)
        {
          long_sig[i] = DBUS_DICT_ENTRY_END_CHAR;
          ++i;
          n_begins -= 1;
        }
      long_sig[i] = DBUS_TYPE_INVALID;

      v_STRING = long_sig;
      if (!_dbus_header_set_field_basic (&message->header,
                                         DBUS_HEADER_FIELD_SIGNATURE,
                                         DBUS_TYPE_SIGNATURE,
                                         &v_STRING))
        _dbus_test_fatal ("oom");

      _dbus_header_get_field_raw (&message->header,
                                  DBUS_HEADER_FIELD_SIGNATURE,
                                  NULL, &pos);
      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_INVALID_EXCEEDED_MAXIMUM_DICT_ENTRY_RECURSION;
    }
  else if (item_seq == 17)
    {
      message = simple_method_call ();
      if (!dbus_message_append_args (message,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INVALID))
        _dbus_test_fatal ("oom");

      _dbus_header_get_field_raw (&message->header,
                                  DBUS_HEADER_FIELD_SIGNATURE,
                                  NULL, &pos);
      generate_from_message (data, expected_validity, message);

      _dbus_string_set_byte (data, pos + 1, DBUS_TYPE_ARRAY);
      _dbus_string_set_byte (data, pos + 2, DBUS_DICT_ENTRY_BEGIN_CHAR);

      *expected_validity = DBUS_INVALID_DICT_ENTRY_STARTED_BUT_NOT_ENDED;
    }
  else if (item_seq == 18)
    {
      message = simple_method_call ();
      if (!dbus_message_append_args (message,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INVALID))
        _dbus_test_fatal ("oom");

      _dbus_header_get_field_raw (&message->header,
                                  DBUS_HEADER_FIELD_SIGNATURE,
                                  NULL, &pos);
      generate_from_message (data, expected_validity, message);

      _dbus_string_set_byte (data, pos + 1, DBUS_DICT_ENTRY_END_CHAR);

      *expected_validity = DBUS_INVALID_DICT_ENTRY_ENDED_BUT_NOT_STARTED;
    }
  else if (item_seq == 19)
    {
      message = simple_method_call ();
      if (!dbus_message_append_args (message,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INT32, &v_INT32,
                                     DBUS_TYPE_INVALID))
        _dbus_test_fatal ("oom");

      _dbus_header_get_field_raw (&message->header,
                                  DBUS_HEADER_FIELD_SIGNATURE,
                                  NULL, &pos);
      generate_from_message (data, expected_validity, message);

      _dbus_string_set_byte (data, pos + 1, DBUS_TYPE_ARRAY);
      _dbus_string_set_byte (data, pos + 2, DBUS_DICT_ENTRY_BEGIN_CHAR);
      _dbus_string_set_byte (data, pos + 3, DBUS_DICT_ENTRY_END_CHAR);

      *expected_validity = DBUS_INVALID_DICT_ENTRY_HAS_NO_FIELDS;
    }
  else if (item_seq == 20)
    {
      /* 64 levels of nesting is OK */
      message = message_with_nesting_levels(64);

      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_VALID;
    }
  else if (item_seq == 21)
    {
      /* 65 levels of nesting is not OK */
      message = message_with_nesting_levels(65);

      generate_from_message (data, expected_validity, message);

      *expected_validity = DBUS_INVALID_NESTED_TOO_DEEPLY;
    }
  else
    {
      return FALSE;
    }

  if (message)
    dbus_message_unref (message);

  iter_next (iter);
  return TRUE;
}

static dbus_bool_t
generate_wrong_length (DBusMessageDataIter *iter,
                       DBusString          *data,
                       DBusValidity        *expected_validity)
{
  int lengths[] = { -42, -17, -16, -15, -9, -8, -7, -6, -5, -4, -3, -2, -1,
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 16, 30 };
  int adjust;
  int len_seq;

 restart:
  len_seq = iter_get_sequence (iter);
  if (len_seq == _DBUS_N_ELEMENTS (lengths))
    return FALSE;

  _dbus_assert (len_seq < _DBUS_N_ELEMENTS (lengths));

  iter_recurse (iter);
  if (!generate_many_bodies (iter, data, expected_validity))
    {
      iter_set_sequence (iter, 0); /* reset to first body */
      iter_unrecurse (iter);
      iter_next (iter);            /* next length adjustment */
      goto restart;
    }
  iter_unrecurse (iter);

  adjust = lengths[len_seq];

  if (adjust < 0)
    {
      if ((_dbus_string_get_length (data) + adjust) < DBUS_MINIMUM_HEADER_SIZE)
        _dbus_string_set_length (data, DBUS_MINIMUM_HEADER_SIZE);
      else
        _dbus_string_shorten (data, - adjust);
      *expected_validity = DBUS_INVALID_FOR_UNKNOWN_REASON;
    }
  else
    {
      if (!_dbus_string_lengthen (data, adjust))
        _dbus_test_fatal ("oom");
      *expected_validity = DBUS_INVALID_TOO_MUCH_DATA;
    }

  /* Fixup lengths */
  {
    int old_body_len;
    int new_body_len;
    int byte_order;

    _dbus_assert (_dbus_string_get_length (data) >= DBUS_MINIMUM_HEADER_SIZE);

    byte_order = _dbus_string_get_byte (data, BYTE_ORDER_OFFSET);
    old_body_len = _dbus_marshal_read_uint32 (data,
                                              BODY_LENGTH_OFFSET,
                                              byte_order,
                                              NULL);
    _dbus_assert (old_body_len < _dbus_string_get_length (data));
    new_body_len = old_body_len + adjust;
    if (new_body_len < 0)
      {
        new_body_len = 0;
        /* we just munged the header, and aren't sure how */
        *expected_validity = DBUS_VALIDITY_UNKNOWN;
      }

    _dbus_verbose ("changing body len from %u to %u by adjust %d\n",
                   old_body_len, new_body_len, adjust);

    _dbus_marshal_set_uint32 (data, BODY_LENGTH_OFFSET,
                              new_body_len,
                              byte_order);
  }

  return TRUE;
}

static dbus_bool_t
generate_byte_changed (DBusMessageDataIter *iter,
                       DBusString          *data,
                       DBusValidity        *expected_validity)
{
  int byte_seq;
  int v_BYTE;

  /* This is a little convoluted to make the bodies the
   * outer loop and each byte of each body the inner
   * loop
   */

 restart:
  if (!generate_many_bodies (iter, data, expected_validity))
    return FALSE;

  iter_recurse (iter);
  byte_seq = iter_get_sequence (iter);
  iter_next (iter);
  iter_unrecurse (iter);

  if (byte_seq == _dbus_string_get_length (data))
    {
      _dbus_string_set_length (data, 0);
      /* reset byte count */
      iter_recurse (iter);
      iter_set_sequence (iter, 0);
      iter_unrecurse (iter);
      goto restart;
    }
  else
    {
      /* Undo the "next" in generate_many_bodies */
      iter_set_sequence (iter, iter_get_sequence (iter) - 1);
    }

  _dbus_assert (byte_seq < _dbus_string_get_length (data));
  v_BYTE = _dbus_string_get_byte (data, byte_seq);
  v_BYTE += byte_seq; /* arbitrary but deterministic change to the byte */
  _dbus_string_set_byte (data, byte_seq, v_BYTE);
  *expected_validity = DBUS_VALIDITY_UNKNOWN;

  return TRUE;
}

#if 0
/* This is really expensive and doesn't add too much coverage */

static dbus_bool_t
find_next_typecode (DBusMessageDataIter *iter,
                    DBusString          *data,
                    DBusValidity        *expected_validity)
{
  int body_seq;
  int byte_seq;
  int base_depth;

  base_depth = iter->depth;

 restart:
  _dbus_assert (iter->depth == (base_depth + 0));
  _dbus_string_set_length (data, 0);

  body_seq = iter_get_sequence (iter);

  if (!generate_many_bodies (iter, data, expected_validity))
    return FALSE;
  /* Undo the "next" in generate_many_bodies */
  iter_set_sequence (iter, body_seq);

  iter_recurse (iter);
  while (TRUE)
    {
      _dbus_assert (iter->depth == (base_depth + 1));

      byte_seq = iter_get_sequence (iter);

      _dbus_assert (byte_seq <= _dbus_string_get_length (data));

      if (byte_seq == _dbus_string_get_length (data))
        {
          /* reset byte count */
          iter_set_sequence (iter, 0);
          iter_unrecurse (iter);
          _dbus_assert (iter->depth == (base_depth + 0));
          iter_next (iter); /* go to the next body */
          goto restart;
        }

      _dbus_assert (byte_seq < _dbus_string_get_length (data));

      if (dbus_type_is_valid (_dbus_string_get_byte (data, byte_seq)))
        break;
      else
        iter_next (iter);
    }

  _dbus_assert (byte_seq == iter_get_sequence (iter));
  _dbus_assert (byte_seq < _dbus_string_get_length (data));

  iter_unrecurse (iter);

  _dbus_assert (iter->depth == (base_depth + 0));

  return TRUE;
}

static const int typecodes[] = {
  DBUS_TYPE_INVALID,
  DBUS_TYPE_BYTE,
  DBUS_TYPE_BOOLEAN,
  DBUS_TYPE_INT16,
  DBUS_TYPE_UINT16,
  DBUS_TYPE_INT32,
  DBUS_TYPE_UINT32,
  DBUS_TYPE_INT64,
  DBUS_TYPE_UINT64,
  DBUS_TYPE_DOUBLE,
  DBUS_TYPE_STRING,
  DBUS_TYPE_OBJECT_PATH,
  DBUS_TYPE_SIGNATURE,
  DBUS_TYPE_ARRAY,
  DBUS_TYPE_VARIANT,
  DBUS_STRUCT_BEGIN_CHAR,
  DBUS_STRUCT_END_CHAR,
  DBUS_DICT_ENTRY_BEGIN_CHAR,
  DBUS_DICT_ENTRY_END_CHAR,
  DBUS_TYPE_UNIX_FD,
  255 /* random invalid typecode */
};

static dbus_bool_t
generate_typecode_changed (DBusMessageDataIter *iter,
                           DBusString          *data,
                           DBusValidity        *expected_validity)
{
  int byte_seq;
  int typecode_seq;
  int base_depth;

  base_depth = iter->depth;

 restart:
  _dbus_assert (iter->depth == (base_depth + 0));
  _dbus_string_set_length (data, 0);

  if (!find_next_typecode (iter, data, expected_validity))
    return FALSE;

  iter_recurse (iter);
  byte_seq = iter_get_sequence (iter);

  _dbus_assert (byte_seq < _dbus_string_get_length (data));

  iter_recurse (iter);
  typecode_seq = iter_get_sequence (iter);
  iter_next (iter);

  _dbus_assert (typecode_seq <= _DBUS_N_ELEMENTS (typecodes));

  if (typecode_seq == _DBUS_N_ELEMENTS (typecodes))
    {
      _dbus_assert (iter->depth == (base_depth + 2));
      iter_set_sequence (iter, 0); /* reset typecode sequence */
      iter_unrecurse (iter);
      _dbus_assert (iter->depth == (base_depth + 1));
      iter_next (iter); /* go to the next byte_seq */
      iter_unrecurse (iter);
      _dbus_assert (iter->depth == (base_depth + 0));
      goto restart;
    }

  _dbus_assert (iter->depth == (base_depth + 2));
  iter_unrecurse (iter);
  _dbus_assert (iter->depth == (base_depth + 1));
  iter_unrecurse (iter);
  _dbus_assert (iter->depth == (base_depth + 0));

#if 0
  _dbus_test_diag ("Changing byte %d in message %d to %c",
          byte_seq, iter_get_sequence (iter), typecodes[typecode_seq]);
#endif

  _dbus_string_set_byte (data, byte_seq, typecodes[typecode_seq]);
  *expected_validity = DBUS_VALIDITY_UNKNOWN;
  return TRUE;
}
#endif

typedef struct
{
  ChangeType type;
  dbus_uint32_t value; /* cast to signed for adjusts */
} UIntChange;

static const UIntChange uint32_changes[] = {
  { CHANGE_TYPE_ADJUST, (dbus_uint32_t) -1 },
  { CHANGE_TYPE_ADJUST, (dbus_uint32_t) -2 },
  { CHANGE_TYPE_ADJUST, (dbus_uint32_t) -3 },
  { CHANGE_TYPE_ADJUST, (dbus_uint32_t) 1 },
  { CHANGE_TYPE_ADJUST, (dbus_uint32_t) 2 },
  { CHANGE_TYPE_ADJUST, (dbus_uint32_t) 3 },
  { CHANGE_TYPE_ABSOLUTE, _DBUS_UINT32_MAX },
  { CHANGE_TYPE_ABSOLUTE, 0 },
  { CHANGE_TYPE_ABSOLUTE, 1 },
  { CHANGE_TYPE_ABSOLUTE, _DBUS_UINT32_MAX - 1 },
  { CHANGE_TYPE_ABSOLUTE, _DBUS_UINT32_MAX - 5 }
};

static dbus_bool_t
generate_uint32_changed (DBusMessageDataIter *iter,
                         DBusString          *data,
                         DBusValidity        *expected_validity)
{
  int body_seq;
  int byte_seq;
  int change_seq;
  dbus_uint32_t v_UINT32;
  int byte_order;
  const UIntChange *change;
  int base_depth;

  /* Outer loop is each body, next loop is each change,
   * inner loop is each change location
   */

  base_depth = iter->depth;

 next_body:
  _dbus_assert (iter->depth == (base_depth + 0));
  _dbus_string_set_length (data, 0);
  body_seq = iter_get_sequence (iter);

  if (!generate_many_bodies (iter, data, expected_validity))
    return FALSE;

  _dbus_assert (iter->depth == (base_depth + 0));

  iter_set_sequence (iter, body_seq); /* undo the "next" from generate_many_bodies */
  iter_recurse (iter);
 next_change:
  _dbus_assert (iter->depth == (base_depth + 1));
  change_seq = iter_get_sequence (iter);

  if (change_seq == _DBUS_N_ELEMENTS (uint32_changes))
    {
      /* Reset change count */
      iter_set_sequence (iter, 0);
      iter_unrecurse (iter);
      iter_next (iter);
      goto next_body;
    }

  _dbus_assert (iter->depth == (base_depth + 1));

  iter_recurse (iter);
  _dbus_assert (iter->depth == (base_depth + 2));
  byte_seq = iter_get_sequence (iter);
  /* skip 4 bytes at a time */
  iter_next (iter);
  iter_next (iter);
  iter_next (iter);
  iter_next (iter);
  iter_unrecurse (iter);

  _dbus_assert (_DBUS_ALIGN_VALUE (byte_seq, 4) == (unsigned) byte_seq);
  if (byte_seq >= (_dbus_string_get_length (data) - 4))
    {
      /* reset byte count */
      _dbus_assert (iter->depth == (base_depth + 1));
      iter_recurse (iter);
      _dbus_assert (iter->depth == (base_depth + 2));
      iter_set_sequence (iter, 0);
      iter_unrecurse (iter);
      _dbus_assert (iter->depth == (base_depth + 1));
      iter_next (iter);
      goto next_change;
    }

  _dbus_assert (byte_seq <= (_dbus_string_get_length (data) - 4));

  byte_order = _dbus_string_get_byte (data, BYTE_ORDER_OFFSET);

  v_UINT32 = _dbus_marshal_read_uint32 (data, byte_seq, byte_order, NULL);

  change = &uint32_changes[change_seq];

  if (change->type == CHANGE_TYPE_ADJUST)
    {
      v_UINT32 += (int) change->value;
    }
  else
    {
      v_UINT32 = change->value;
    }

#if 0
  _dbus_test_diag ("body %d change %d pos %d ",
          body_seq, change_seq, byte_seq);

  if (change->type == CHANGE_TYPE_ADJUST)
    _dbus_test_diag ("adjust by %d", (int) change->value);
  else
    _dbus_test_diag ("set to %u", change->value);

  _dbus_test_diag (" \t%u -> %u",
          _dbus_marshal_read_uint32 (data, byte_seq, byte_order, NULL),
          v_UINT32);
#endif

  _dbus_marshal_set_uint32 (data, byte_seq, v_UINT32, byte_order);
  *expected_validity = DBUS_VALIDITY_UNKNOWN;

  _dbus_assert (iter->depth == (base_depth + 1));
  iter_unrecurse (iter);
  _dbus_assert (iter->depth == (base_depth + 0));

  return TRUE;
}

typedef struct
{
  const char *name;
  DBusMessageGeneratorFunc func;
} DBusMessageGenerator;

static const DBusMessageGenerator generators[] = {
  { "trivial example of each message type", generate_trivial },
  { "assorted arguments", generate_many_bodies },
  { "assorted special cases", generate_special },
  { "each uint32 modified", generate_uint32_changed },
  { "wrong body lengths", generate_wrong_length },
  { "each byte modified", generate_byte_changed },
#if 0
  /* This is really expensive and doesn't add too much coverage */
  { "change each typecode", generate_typecode_changed }
#endif
};

void
_dbus_message_data_free (DBusMessageData *data)
{
  _dbus_string_free (&data->data);
}

void
_dbus_message_data_iter_init (DBusMessageDataIter *iter)
{
  int i;

  iter->depth = 0;
  i = 0;
  while (i < _DBUS_MESSAGE_DATA_MAX_NESTING)
    {
      iter->sequence_nos[i] = 0;
      ++i;
    }
  iter->count = 0;
}

dbus_bool_t
_dbus_message_data_iter_get_and_next (DBusMessageDataIter *iter,
                                      DBusMessageData     *data)
{
  DBusMessageGeneratorFunc func;
  int generator;

 restart:
  generator = iter_get_sequence (iter);

  if (generator == _DBUS_N_ELEMENTS (generators))
    return FALSE;

  iter_recurse (iter);

  if (iter_first_in_series (iter))
    {
      _dbus_test_diag (" testing message loading: %s ", generators[generator].name);
    }

  func = generators[generator].func;

  if (!_dbus_string_init (&data->data))
    _dbus_test_fatal ("oom");

  if ((*func)(iter, &data->data, &data->expected_validity))
    ;
  else
    {
      iter_set_sequence (iter, 0);
      iter_unrecurse (iter);
      iter_next (iter); /* next generator */
      _dbus_string_free (&data->data);
      _dbus_test_diag ("%d test loads cumulative", iter->count);
      goto restart;
    }
  iter_unrecurse (iter);

  iter->count += 1;
  return TRUE;
}

#endif /* !DOXYGEN_SHOULD_SKIP_THIS */

#endif /* DBUS_ENABLE_EMBEDDED_TESTS */
