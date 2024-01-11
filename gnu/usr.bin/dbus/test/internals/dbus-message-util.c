/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-message-util.c Would be in dbus-message.c, but only used by bus/tests
 *
 * Copyright 2002-2009 Red Hat, Inc.
 * Copyright 2002-2003 CodeFactory AB
 * Copyright 2007-2018 Collabora Ltd.
 * Copyright 2009 Scott James Remnant / Canonical Ltd.
 * Copyright 2009 William Lachance
 * Copyright 2010 Christian Dywan / Lanedo
 * Copyright 2013 Chengwei Yang / Intel
 * Copyright 2013 Vasiliy Balyasnyy / Samsung
 * Copyright 2014 Ralf Habacker
 * Copyright 2017 Endless Mobile, Inc.
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

#include "dbus-message-util.h"

#include "dbus/dbus-internals.h"
#include "dbus/dbus-test.h"
#include "dbus/dbus-message-private.h"
#include "dbus/dbus-marshal-recursive.h"
#include "dbus/dbus-string.h"
#ifdef HAVE_UNIX_FD_PASSING
#include "dbus/dbus-sysdeps-unix.h"
#endif
#include "dbus/dbus-test-tap.h"

#include "dbus-message-factory.h"

#include "test/test-utils.h"

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
/**
 * Reads arguments from a message iterator given a variable argument
 * list. Only arguments of basic type and arrays of fixed-length
 * basic type may be read with this function. See
 * dbus_message_get_args() for more details.
 *
 * @param iter the message iterator
 * @param error error to be filled in on failure
 * @param first_arg_type the first argument type
 * @param ... location for first argument value, then list of type-location pairs
 * @returns #FALSE if the error was set
 */
static dbus_bool_t
dbus_message_iter_get_args (DBusMessageIter *iter,
			    DBusError       *error,
			    int              first_arg_type,
			    ...)
{
  dbus_bool_t retval;
  va_list var_args;

  _dbus_return_val_if_fail (iter != NULL, FALSE);
  _dbus_return_val_if_error_is_set (error, FALSE);

  va_start (var_args, first_arg_type);
  retval = _dbus_message_iter_get_args_valist (iter, error, first_arg_type, var_args);
  va_end (var_args);

  return retval;
}

/* returns FALSE on fatal failure */
typedef dbus_bool_t (* DBusForeachMessageFileFunc) (const DBusString   *filename,
                                                    DBusValidity        expected_validity,
                                                    void               *data);

static dbus_bool_t try_message_data (const DBusString    *data,
                                     DBusValidity         expected_validity);

static int validities_seen[DBUS_VALIDITY_LAST + _DBUS_NEGATIVE_VALIDITY_COUNT];

static void
reset_validities_seen (void)
{
  int i;
  i = 0;
  while (i < _DBUS_N_ELEMENTS (validities_seen))
    {
      validities_seen[i] = 0;
      ++i;
    }
}

static void
record_validity_seen (DBusValidity validity)
{
  validities_seen[validity + _DBUS_NEGATIVE_VALIDITY_COUNT] += 1;
}

static void
print_validities_seen (dbus_bool_t not_seen)
{
  int i;
  i = 0;
  while (i < _DBUS_N_ELEMENTS (validities_seen))
    {
      if ((i - _DBUS_NEGATIVE_VALIDITY_COUNT) == DBUS_VALIDITY_UNKNOWN ||
          (i - _DBUS_NEGATIVE_VALIDITY_COUNT) == DBUS_INVALID_FOR_UNKNOWN_REASON)
        ;
      else if ((not_seen && validities_seen[i] == 0) ||
               (!not_seen && validities_seen[i] > 0))
        _dbus_test_diag ("validity %3d seen %d times",
                i - _DBUS_NEGATIVE_VALIDITY_COUNT,
                validities_seen[i]);
      ++i;
    }
}

static void
check_memleaks (void)
{
  dbus_shutdown ();

  if (_dbus_get_malloc_blocks_outstanding () != 0)
    {
      _dbus_test_fatal ("%d dbus_malloc blocks were not freed in %s",
                  _dbus_get_malloc_blocks_outstanding (), __FILE__);
    }
}

static dbus_bool_t
check_have_valid_message (DBusMessageLoader *loader)
{
  DBusMessage *message;
  dbus_bool_t retval;

  message = NULL;
  retval = FALSE;

  if (_dbus_message_loader_get_is_corrupted (loader))
    {
      _dbus_warn ("loader corrupted on message that was expected to be valid; invalid reason %d",
                  loader->corruption_reason);
      goto failed;
    }

  message = _dbus_message_loader_pop_message (loader);
  if (message == NULL)
    {
      _dbus_warn ("didn't load message that was expected to be valid (message not popped)");
      goto failed;
    }

  if (_dbus_string_get_length (&loader->data) > 0)
    {
      _dbus_warn ("had leftover bytes from expected-to-be-valid single message");
      goto failed;
    }

#if 0
  /* FIXME */
  /* Verify that we're able to properly deal with the message.
   * For example, this would detect improper handling of messages
   * in nonstandard byte order.
   */
  if (!check_message_handling (message))
    goto failed;
#endif

  record_validity_seen (DBUS_VALID);

  retval = TRUE;

 failed:
  if (message)
    dbus_message_unref (message);

  return retval;
}

static dbus_bool_t
check_invalid_message (DBusMessageLoader *loader,
                       DBusValidity       expected_validity)
{
  dbus_bool_t retval;

  retval = FALSE;

  if (!_dbus_message_loader_get_is_corrupted (loader))
    {
      _dbus_warn ("loader not corrupted on message that was expected to be invalid");
      goto failed;
    }

  record_validity_seen (loader->corruption_reason);

  if (expected_validity != DBUS_INVALID_FOR_UNKNOWN_REASON &&
      loader->corruption_reason != expected_validity)
    {
      _dbus_warn ("expected message to be corrupted for reason %d and was corrupted for %d instead",
                  expected_validity, loader->corruption_reason);
      goto failed;
    }

  retval = TRUE;

 failed:
  return retval;
}

static dbus_bool_t
check_incomplete_message (DBusMessageLoader *loader)
{
  DBusMessage *message;
  dbus_bool_t retval;

  message = NULL;
  retval = FALSE;

  if (_dbus_message_loader_get_is_corrupted (loader))
    {
      _dbus_warn ("loader corrupted on message that was expected to be valid (but incomplete), corruption reason %d",
                  loader->corruption_reason);
      goto failed;
    }

  message = _dbus_message_loader_pop_message (loader);
  if (message != NULL)
    {
      _dbus_warn ("loaded message that was expected to be incomplete");
      goto failed;
    }

  record_validity_seen (DBUS_VALID_BUT_INCOMPLETE);
  retval = TRUE;

 failed:
  if (message)
    dbus_message_unref (message);
  return retval;
}

static dbus_bool_t
check_loader_results (DBusMessageLoader      *loader,
                      DBusValidity            expected_validity)
{
  if (!_dbus_message_loader_queue_messages (loader))
    _dbus_test_fatal ("no memory to queue messages");

  if (expected_validity == DBUS_VALID)
    return check_have_valid_message (loader);
  else if (expected_validity == DBUS_VALID_BUT_INCOMPLETE)
    return check_incomplete_message (loader);
  else if (expected_validity == DBUS_VALIDITY_UNKNOWN)
    {
      /* here we just know we didn't segfault and that was the
       * only test. Also, we record that we got coverage
       * for the validity reason.
       */
      if (_dbus_message_loader_get_is_corrupted (loader))
        record_validity_seen (loader->corruption_reason);

      return TRUE;
    }
  else
    return check_invalid_message (loader, expected_validity);
}

/**
 * Loads the message in the given message file.
 *
 * @param filename filename to load
 * @param data string to load message into
 * @returns #TRUE if the message was loaded
 */
static dbus_bool_t
load_message_file (const DBusString    *filename,
                   DBusString          *data)
{
  dbus_bool_t retval;
  DBusError error = DBUS_ERROR_INIT;

  retval = FALSE;

  _dbus_verbose ("Loading raw %s\n", _dbus_string_get_const_data (filename));
  if (!_dbus_file_get_contents (data, filename, &error))
    {
      _dbus_warn ("Could not load message file %s: %s",
                  _dbus_string_get_const_data (filename),
                  error.message);
      dbus_error_free (&error);
      goto failed;
    }

  retval = TRUE;

 failed:

  return retval;
}

/**
 * Tries loading the message in the given message file
 * and verifies that DBusMessageLoader can handle it.
 *
 * @param filename filename to load
 * @param expected_validity what the message has to be like to return #TRUE
 * @param unused ignored
 * @returns #TRUE if the message has the expected validity
 */
static dbus_bool_t
try_message_file (const DBusString    *filename,
                  DBusValidity         expected_validity,
                  void                *unused)
{
  DBusString data;
  dbus_bool_t retval;

  retval = FALSE;

  if (!_dbus_string_init (&data))
    _dbus_test_fatal ("could not allocate string");

  if (!load_message_file (filename, &data))
    goto failed;

  retval = try_message_data (&data, expected_validity);

 failed:

  if (!retval)
    {
      if (_dbus_string_get_length (&data) > 0)
        _dbus_verbose_bytes_of_string (&data, 0,
                                       _dbus_string_get_length (&data));

      _dbus_warn ("Failed message loader test on %s",
                  _dbus_string_get_const_data (filename));
    }

  _dbus_string_free (&data);

  return retval;
}

/**
 * Tries loading the given message data.
 *
 *
 * @param data the message data
 * @param expected_validity what the message has to be like to return #TRUE
 * @returns #TRUE if the message has the expected validity
 */
static dbus_bool_t
try_message_data (const DBusString    *data,
                  DBusValidity         expected_validity)
{
  DBusMessageLoader *loader;
  dbus_bool_t retval;
  int len;
  int i;

  loader = NULL;
  retval = FALSE;

  /* Write the data one byte at a time */

  loader = _dbus_message_loader_new ();
  if (loader == NULL)
    goto failed;

  /* check some trivial loader functions */
  _dbus_message_loader_ref (loader);
  _dbus_message_loader_unref (loader);

  len = _dbus_string_get_length (data);
  for (i = 0; i < len; i++)
    {
      DBusString *buffer;

      _dbus_message_loader_get_buffer (loader, &buffer, NULL, NULL);
      if (!_dbus_string_append_byte (buffer,
                                     _dbus_string_get_byte (data, i)))
        goto failed;
      _dbus_message_loader_return_buffer (loader, buffer);
    }

  if (!check_loader_results (loader, expected_validity))
    goto failed;

  _dbus_message_loader_unref (loader);
  loader = NULL;

  /* Write the data all at once */

  loader = _dbus_message_loader_new ();
  if (loader == NULL)
    goto failed;

  {
    DBusString *buffer;

    _dbus_message_loader_get_buffer (loader, &buffer, NULL, NULL);
    if (!_dbus_string_copy (data, 0, buffer,
                            _dbus_string_get_length (buffer)))
        goto failed;
    _dbus_message_loader_return_buffer (loader, buffer);
  }

  if (!check_loader_results (loader, expected_validity))
    goto failed;

  _dbus_message_loader_unref (loader);
  loader = NULL;

  /* Write the data 2 bytes at a time */

  loader = _dbus_message_loader_new ();
  if (loader == NULL)
    goto failed;

  len = _dbus_string_get_length (data);
  for (i = 0; i < len; i += 2)
    {
      DBusString *buffer;

      _dbus_message_loader_get_buffer (loader, &buffer, NULL, NULL);
      if (!_dbus_string_append_byte (buffer,
                                     _dbus_string_get_byte (data, i)))
        goto failed;

      if ((i+1) < len)
        {
          if (!_dbus_string_append_byte (buffer,
                                         _dbus_string_get_byte (data, i+1)))
            goto failed;
        }

      _dbus_message_loader_return_buffer (loader, buffer);
    }

  if (!check_loader_results (loader, expected_validity))
    goto failed;

  _dbus_message_loader_unref (loader);
  loader = NULL;

  retval = TRUE;

 failed:

  if (loader)
    _dbus_message_loader_unref (loader);

  return retval;
}

static dbus_bool_t
process_test_subdir (const DBusString          *test_base_dir,
                     const char                *subdir,
                     DBusValidity               expected_validity,
                     DBusForeachMessageFileFunc function,
                     void                      *user_data)
{
  DBusString test_directory;
  DBusString filename;
  DBusDirIter *dir;
  dbus_bool_t retval;
  DBusError error = DBUS_ERROR_INIT;

  retval = FALSE;
  dir = NULL;

  if (!_dbus_string_init (&test_directory))
    _dbus_test_fatal ("didn't allocate test_directory");

  _dbus_string_init_const (&filename, subdir);

  if (!_dbus_string_copy (test_base_dir, 0,
                          &test_directory, 0))
    _dbus_test_fatal ("couldn't copy test_base_dir to test_directory");

  if (!_dbus_concat_dir_and_file (&test_directory, &filename))
    _dbus_test_fatal ("couldn't allocate full path");

  _dbus_string_free (&filename);
  if (!_dbus_string_init (&filename))
    _dbus_test_fatal ("didn't allocate filename string");

  dir = _dbus_directory_open (&test_directory, &error);
  if (dir == NULL)
    {
      _dbus_warn ("Could not open %s: %s",
                  _dbus_string_get_const_data (&test_directory),
                  error.message);
      dbus_error_free (&error);
      goto failed;
    }

  _dbus_test_diag ("Testing %s:", subdir);

 next:
  while (_dbus_directory_get_next_file (dir, &filename, &error))
    {
      DBusString full_path;

      if (!_dbus_string_init (&full_path))
        _dbus_test_fatal ("couldn't init string");

      if (!_dbus_string_copy (&test_directory, 0, &full_path, 0))
        _dbus_test_fatal ("couldn't copy dir to full_path");

      if (!_dbus_concat_dir_and_file (&full_path, &filename))
        _dbus_test_fatal ("couldn't concat file to dir");

      if (_dbus_string_ends_with_c_str (&filename, ".message-raw"))
        ;
      else
        {
          _dbus_verbose ("Skipping non-.message-raw file %s\n",
                         _dbus_string_get_const_data (&filename));
	  _dbus_string_free (&full_path);
          goto next;
        }

      _dbus_test_diag ("    %s",
              _dbus_string_get_const_data (&filename));

      if (! (*function) (&full_path,
                         expected_validity, user_data))
        {
          _dbus_string_free (&full_path);
          goto failed;
        }
      else
        _dbus_string_free (&full_path);
    }

  if (dbus_error_is_set (&error))
    {
      _dbus_warn ("Could not get next file in %s: %s",
                  _dbus_string_get_const_data (&test_directory),
                  error.message);
      dbus_error_free (&error);
      goto failed;
    }

  retval = TRUE;

 failed:

  if (dir)
    _dbus_directory_close (dir);
  _dbus_string_free (&test_directory);
  _dbus_string_free (&filename);

  return retval;
}

/**
 * Runs the given function on every message file in the test suite.
 * The function should return #FALSE on test failure or fatal error.
 *
 * @param test_data_dir root dir of the test suite data files (top_srcdir/test/data)
 * @param func the function to run
 * @param user_data data for function
 * @returns #FALSE if there's a failure
 */
static dbus_bool_t
foreach_message_file (const char                *test_data_dir,
                      DBusForeachMessageFileFunc func,
                      void                      *user_data)
{
  DBusString test_directory;
  dbus_bool_t retval;

  retval = FALSE;

  _dbus_string_init_const (&test_directory, test_data_dir);

  if (!process_test_subdir (&test_directory, "valid-messages",
                            DBUS_VALID, func, user_data))
    goto failed;

  if (!process_test_subdir (&test_directory, "invalid-messages",
                            DBUS_INVALID_FOR_UNKNOWN_REASON, func, user_data))
    goto failed;

  check_memleaks ();

  retval = TRUE;

 failed:

  _dbus_string_free (&test_directory);

  return retval;
}

#if 0
#define GET_AND_CHECK(iter, typename, literal)                                  \
  do {                                                                          \
    if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_##typename)         \
      _dbus_test_fatal ("got wrong argument type from message iter");   \
    dbus_message_iter_get_basic (&iter, &v_##typename);                         \
    if (v_##typename != literal)                                                \
      _dbus_test_fatal ("got wrong value from message iter");           \
  } while (0)

#define GET_AND_CHECK_STRCMP(iter, typename, literal)                           \
  do {                                                                          \
    if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_##typename)         \
      _dbus_test_fatal ("got wrong argument type from message iter");   \
    dbus_message_iter_get_basic (&iter, &v_##typename);                         \
    if (strcmp (v_##typename, literal) != 0)                                    \
      _dbus_test_fatal ("got wrong value from message iter");           \
  } while (0)

#define GET_AND_CHECK_AND_NEXT(iter, typename, literal)         \
  do {                                                          \
    GET_AND_CHECK(iter, typename, literal);                     \
    if (!dbus_message_iter_next (&iter))                        \
      _dbus_test_fatal ("failed to move iter to next"); \
  } while (0)

#define GET_AND_CHECK_STRCMP_AND_NEXT(iter, typename, literal)  \
  do {                                                          \
    GET_AND_CHECK_STRCMP(iter, typename, literal);              \
    if (!dbus_message_iter_next (&iter))                        \
      _dbus_test_fatal ("failed to move iter to next"); \
  } while (0)

static void
message_iter_test (DBusMessage *message)
{
  DBusMessageIter iter, array, array2;
  const char *v_STRING;
  double v_DOUBLE;
  dbus_int16_t v_INT16;
  dbus_uint16_t v_UINT16;
  dbus_int32_t v_INT32;
  dbus_uint32_t v_UINT32;
  dbus_int64_t v_INT64;
  dbus_uint64_t v_UINT64;
  unsigned char v_BYTE;
  dbus_bool_t v_BOOLEAN;

  const dbus_int32_t *our_int_array;
  int len;

  dbus_message_iter_init (message, &iter);

  GET_AND_CHECK_STRCMP_AND_NEXT (iter, STRING, "Test string");
  GET_AND_CHECK_AND_NEXT (iter, INT32, -0x12345678);
  GET_AND_CHECK_AND_NEXT (iter, UINT32, 0xedd1e);
  GET_AND_CHECK_AND_NEXT (iter, DOUBLE, 3.14159);

  if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_ARRAY)
    _dbus_test_fatal ("Argument type not an array");

  if (dbus_message_iter_get_element_type (&iter) != DBUS_TYPE_DOUBLE)
    _dbus_test_fatal ("Array type not double");

  dbus_message_iter_recurse (&iter, &array);

  GET_AND_CHECK_AND_NEXT (array, DOUBLE, 1.5);
  GET_AND_CHECK (array, DOUBLE, 2.5);

  if (dbus_message_iter_next (&array))
    _dbus_test_fatal ("Didn't reach end of array");

  if (!dbus_message_iter_next (&iter))
    _dbus_test_fatal ("Reached end of arguments");

  GET_AND_CHECK_AND_NEXT (iter, BYTE, 0xF0);

  if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_ARRAY)
    _dbus_test_fatal ("no array");

  if (dbus_message_iter_get_element_type (&iter) != DBUS_TYPE_INT32)
    _dbus_test_fatal ("Array type not int32");

  /* Empty array */
  dbus_message_iter_recurse (&iter, &array);

  if (dbus_message_iter_next (&array))
    _dbus_test_fatal ("Didn't reach end of array");

  if (!dbus_message_iter_next (&iter))
    _dbus_test_fatal ("Reached end of arguments");

  GET_AND_CHECK (iter, BYTE, 0xF0);

  if (dbus_message_iter_next (&iter))
    _dbus_test_fatal ("Didn't reach end of arguments");
}
#endif

static void
verify_test_message (DBusMessage *message)
{
  DBusMessageIter iter;
  DBusError error = DBUS_ERROR_INIT;
  dbus_int16_t our_int16;
  dbus_uint16_t our_uint16;
  dbus_int32_t our_int;
  dbus_uint32_t our_uint;
  const char *our_str;
  double our_double;
  double v_DOUBLE;
  dbus_bool_t our_bool;
  unsigned char our_byte_1, our_byte_2;
  const dbus_uint32_t *our_uint32_array = (void*)0xdeadbeef;
  int our_uint32_array_len;
  dbus_int32_t *our_int32_array = (void*)0xdeadbeef;
  int our_int32_array_len;
  dbus_int64_t our_int64;
  dbus_uint64_t our_uint64;
  dbus_int64_t *our_uint64_array = (void*)0xdeadbeef;
  int our_uint64_array_len;
  const dbus_int64_t *our_int64_array = (void*)0xdeadbeef;
  int our_int64_array_len;
  const double *our_double_array = (void*)0xdeadbeef;
  int our_double_array_len;
  const unsigned char *our_byte_array = (void*)0xdeadbeef;
  int our_byte_array_len;
  const dbus_bool_t *our_boolean_array = (void*)0xdeadbeef;
  int our_boolean_array_len;
  char **our_string_array;
  int our_string_array_len;

  dbus_message_iter_init (message, &iter);

  if (!dbus_message_iter_get_args (&iter, &error,
                                   DBUS_TYPE_INT16, &our_int16,
                                   DBUS_TYPE_UINT16, &our_uint16,
				   DBUS_TYPE_INT32, &our_int,
                                   DBUS_TYPE_UINT32, &our_uint,
                                   DBUS_TYPE_INT64, &our_int64,
                                   DBUS_TYPE_UINT64, &our_uint64,
				   DBUS_TYPE_STRING, &our_str,
				   DBUS_TYPE_DOUBLE, &our_double,
				   DBUS_TYPE_BOOLEAN, &our_bool,
				   DBUS_TYPE_BYTE, &our_byte_1,
				   DBUS_TYPE_BYTE, &our_byte_2,
				   DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32,
                                   &our_uint32_array, &our_uint32_array_len,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_INT32,
                                   &our_int32_array, &our_int32_array_len,
				   DBUS_TYPE_ARRAY, DBUS_TYPE_UINT64,
                                   &our_uint64_array, &our_uint64_array_len,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_INT64,
                                   &our_int64_array, &our_int64_array_len,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_DOUBLE,
                                   &our_double_array, &our_double_array_len,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE,
                                   &our_byte_array, &our_byte_array_len,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_BOOLEAN,
                                   &our_boolean_array, &our_boolean_array_len,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_STRING,
                                   &our_string_array, &our_string_array_len,
				   0))
    {
      _dbus_test_fatal ("Could not get arguments: %s - %s", error.name,
                  (error.message != NULL) ? error.message : "no message");
    }

  if (our_int16 != -0x123)
    _dbus_test_fatal ("16-bit integers differ!");

  if (our_uint16 != 0x123)
    _dbus_test_fatal ("16-bit uints differ!");

  if (our_int != -0x12345678)
    _dbus_test_fatal ("integers differ!");

  if (our_uint != 0x12300042)
    _dbus_test_fatal ("uints differ!");

  if (our_int64 != DBUS_INT64_CONSTANT (-0x123456789abcd))
    _dbus_test_fatal ("64-bit integers differ!");
  if (our_uint64 != DBUS_UINT64_CONSTANT (0x123456789abcd))
    _dbus_test_fatal ("64-bit unsigned integers differ!");

  v_DOUBLE = 3.14159;
  if (! _DBUS_DOUBLES_BITWISE_EQUAL (our_double, v_DOUBLE))
    _dbus_test_fatal ("doubles differ!");

  if (strcmp (our_str, "Test string") != 0)
    _dbus_test_fatal ("strings differ!");

  if (!our_bool)
    _dbus_test_fatal ("booleans differ");

  if (our_byte_1 != 42)
    _dbus_test_fatal ("bytes differ!");

  if (our_byte_2 != 24)
    _dbus_test_fatal ("bytes differ!");

  if (our_uint32_array_len != 4 ||
      our_uint32_array[0] != 0x12345678 ||
      our_uint32_array[1] != 0x23456781 ||
      our_uint32_array[2] != 0x34567812 ||
      our_uint32_array[3] != 0x45678123)
    _dbus_test_fatal ("uint array differs");

  if (our_int32_array_len != 4 ||
      our_int32_array[0] != 0x12345678 ||
      our_int32_array[1] != -0x23456781 ||
      our_int32_array[2] != 0x34567812 ||
      our_int32_array[3] != -0x45678123)
    _dbus_test_fatal ("int array differs");

  if (our_uint64_array_len != 4 ||
      our_uint64_array[0] != 0x12345678 ||
      our_uint64_array[1] != 0x23456781 ||
      our_uint64_array[2] != 0x34567812 ||
      our_uint64_array[3] != 0x45678123)
    _dbus_test_fatal ("uint64 array differs");

  if (our_int64_array_len != 4 ||
      our_int64_array[0] != 0x12345678 ||
      our_int64_array[1] != -0x23456781 ||
      our_int64_array[2] != 0x34567812 ||
      our_int64_array[3] != -0x45678123)
    _dbus_test_fatal ("int64 array differs");

  if (our_double_array_len != 3)
    _dbus_test_fatal ("double array had wrong length");

  /* On all IEEE machines (i.e. everything sane) exact equality
   * should be preserved over the wire
   */
  v_DOUBLE = 0.1234;
  if (! _DBUS_DOUBLES_BITWISE_EQUAL (our_double_array[0], v_DOUBLE))
    _dbus_test_fatal ("double array had wrong values");
  v_DOUBLE = 9876.54321;
  if (! _DBUS_DOUBLES_BITWISE_EQUAL (our_double_array[1], v_DOUBLE))
    _dbus_test_fatal ("double array had wrong values");
  v_DOUBLE = -300.0;
  if (! _DBUS_DOUBLES_BITWISE_EQUAL (our_double_array[2], v_DOUBLE))
    _dbus_test_fatal ("double array had wrong values");

  if (our_byte_array_len != 4)
    _dbus_test_fatal ("byte array had wrong length");

  if (our_byte_array[0] != 'a' ||
      our_byte_array[1] != 'b' ||
      our_byte_array[2] != 'c' ||
      our_byte_array[3] != 234)
    _dbus_test_fatal ("byte array had wrong values");

  if (our_boolean_array_len != 5)
    _dbus_test_fatal ("bool array had wrong length");

  if (our_boolean_array[0] != TRUE ||
      our_boolean_array[1] != FALSE ||
      our_boolean_array[2] != TRUE ||
      our_boolean_array[3] != TRUE ||
      our_boolean_array[4] != FALSE)
    _dbus_test_fatal ("bool array had wrong values");

  if (our_string_array_len != 4)
    _dbus_test_fatal ("string array was wrong length");

  if (strcmp (our_string_array[0], "Foo") != 0 ||
      strcmp (our_string_array[1], "bar") != 0 ||
      strcmp (our_string_array[2], "") != 0 ||
      strcmp (our_string_array[3], "woo woo woo woo") != 0)
    _dbus_test_fatal ("string array had wrong values");

  dbus_free_string_array (our_string_array);

  if (dbus_message_iter_next (&iter))
    _dbus_test_fatal ("Didn't reach end of arguments");
}

static void
verify_test_message_args_ignored (DBusMessage *message,
                                  const char  *context)
{
  DBusMessageIter iter;
  DBusError error = DBUS_ERROR_INIT;
  dbus_uint32_t our_uint;
  DBusInitialFDs *initial_fds;

  initial_fds = _dbus_check_fdleaks_enter ();

  /* parse with empty signature: "" */
  dbus_message_iter_init (message, &iter);
  if (!dbus_message_iter_get_args (&iter, &error,
                                   DBUS_TYPE_INVALID))
    {
      _dbus_warn ("error: %s - %s", error.name,
                     (error.message != NULL) ? error.message : "no message");
    }
  else
    {
      _dbus_assert (!dbus_error_is_set (&error));
      _dbus_verbose ("arguments ignored.\n");
    }

  /* parse with shorter signature: "u" */
  dbus_message_iter_init (message, &iter);
  if (!dbus_message_iter_get_args (&iter, &error,
                                   DBUS_TYPE_UINT32, &our_uint,
                                   DBUS_TYPE_INVALID))
    {
      _dbus_warn ("error: %s - %s", error.name,
                     (error.message != NULL) ? error.message : "no message");
    }
  else
    {
      _dbus_assert (!dbus_error_is_set (&error));
      _dbus_verbose ("arguments ignored.\n");
    }

  _dbus_check_fdleaks_leave (initial_fds, context);
}

static void
verify_test_message_memleak (DBusMessage *message,
                             const char  *context)
{
  DBusMessageIter iter;
  DBusError error = DBUS_ERROR_INIT;
  dbus_uint32_t our_uint1;
  dbus_uint32_t our_uint2;
  dbus_uint32_t our_uint3;
  char **our_string_array1;
  int our_string_array_len1;
  char **our_string_array2;
  int our_string_array_len2;
#ifdef HAVE_UNIX_FD_PASSING
  int our_unix_fd1;
  int our_unix_fd2;
#endif
  DBusInitialFDs *initial_fds;

  initial_fds = _dbus_check_fdleaks_enter ();

  /* parse with wrong signature: "uashuu" */
  dbus_error_free (&error);
  dbus_message_iter_init (message, &iter);
  if (!dbus_message_iter_get_args (&iter, &error,
                                   DBUS_TYPE_UINT32, &our_uint1,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_STRING,
                                   &our_string_array1, &our_string_array_len1,
#ifdef HAVE_UNIX_FD_PASSING
                                   DBUS_TYPE_UNIX_FD, &our_unix_fd1,
#endif
                                   DBUS_TYPE_UINT32, &our_uint2,
                                   DBUS_TYPE_UINT32, &our_uint3,
                                   DBUS_TYPE_INVALID))
    {
      _dbus_verbose ("expected error: %s - %s\n", error.name,
                     (error.message != NULL) ? error.message : "no message");
      /* ensure array of string and unix fd not leaked */
      _dbus_assert (our_string_array1 == NULL);
#ifdef HAVE_UNIX_FD_PASSING
      _dbus_assert (our_unix_fd1 == -1);
#endif
    }
  else
    {
      _dbus_test_fatal ("error: parse with wrong signature: 'uashuu'.");
    }

  /* parse with wrong signature: "uashuashu" */
  dbus_message_iter_init (message, &iter);
  dbus_error_free (&error);
  if (!dbus_message_iter_get_args (&iter, &error,
                                   DBUS_TYPE_UINT32, &our_uint1,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_STRING,
                                   &our_string_array1, &our_string_array_len1,
#ifdef HAVE_UNIX_FD_PASSING
                                   DBUS_TYPE_UNIX_FD, &our_unix_fd1,
#endif
                                   DBUS_TYPE_UINT32, &our_uint2,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_STRING,
                                   &our_string_array2, &our_string_array_len2,
#ifdef HAVE_UNIX_FD_PASSING
                                   DBUS_TYPE_UNIX_FD, &our_unix_fd2,
#endif
                                   DBUS_TYPE_UINT32, &our_uint3,
                                   DBUS_TYPE_INVALID))
    {
      _dbus_verbose ("expected error: %s - %s\n", error.name,
                     (error.message != NULL) ? error.message : "no message");
      /* ensure array of string and unix fd not leaked */
      _dbus_assert (our_string_array1 == NULL);
      _dbus_assert (our_string_array2 == NULL);
#ifdef HAVE_UNIX_FD_PASSING
      _dbus_assert (our_unix_fd1 == -1);
      _dbus_assert (our_unix_fd2 == -1);
#endif
    }
  else
    {
      _dbus_test_fatal ("error: parse with wrong signature: 'uashuashu'.");
    }

  /* parse with correct signature: "uashuash" */
  dbus_message_iter_init (message, &iter);
  dbus_error_free (&error);
  if (!dbus_message_iter_get_args (&iter, &error,
                                   DBUS_TYPE_UINT32, &our_uint1,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_STRING,
                                   &our_string_array1, &our_string_array_len1,
#ifdef HAVE_UNIX_FD_PASSING
                                   DBUS_TYPE_UNIX_FD, &our_unix_fd1,
#endif
                                   DBUS_TYPE_UINT32, &our_uint2,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_STRING,
                                   &our_string_array2, &our_string_array_len2,
#ifdef HAVE_UNIX_FD_PASSING
                                   DBUS_TYPE_UNIX_FD, &our_unix_fd2,
#endif
                                   DBUS_TYPE_INVALID))
    {
      _dbus_test_fatal ("Could not get arguments: %s - %s", error.name,
                  (error.message != NULL) ? error.message : "no message");
    }
  else
    {
      dbus_free_string_array (our_string_array1);
      dbus_free_string_array (our_string_array2);
#ifdef HAVE_UNIX_FD_PASSING
      _dbus_close (our_unix_fd1, &error);
      _dbus_close (our_unix_fd2, &error);
#endif
    }
  _dbus_check_fdleaks_leave (initial_fds, context);
}

/**
 * @ingroup DBusMessageInternals
 * Unit test for DBusMessage.
 *
 * @returns #TRUE on success.
 */
dbus_bool_t
_dbus_message_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusMessage *message, *message_without_unix_fds;
  DBusMessageLoader *loader;
  int i;
  const char *data;
  DBusMessage *copy;
  const char *name1;
  const char *name2;
  const dbus_uint32_t our_uint32_array[] =
    { 0x12345678, 0x23456781, 0x34567812, 0x45678123 };
  const dbus_int32_t our_int32_array[] =
    { 0x12345678, -0x23456781, 0x34567812, -0x45678123 };
  const dbus_uint32_t *v_ARRAY_UINT32 = our_uint32_array;
  const dbus_int32_t *v_ARRAY_INT32 = our_int32_array;
  const dbus_uint64_t our_uint64_array[] =
    { 0x12345678, 0x23456781, 0x34567812, 0x45678123 };
  const dbus_int64_t our_int64_array[] =
    { 0x12345678, -0x23456781, 0x34567812, -0x45678123 };
  const dbus_uint64_t *v_ARRAY_UINT64 = our_uint64_array;
  const dbus_int64_t *v_ARRAY_INT64 = our_int64_array;
  const char *our_string_array[] = { "Foo", "bar", "", "woo woo woo woo" };
  const char *our_string_array1[] = { "foo", "Bar", "", "Woo woo Woo woo" };
  const char **v_ARRAY_STRING = our_string_array;
  const char **v1_ARRAY_STRING = our_string_array1;
  const double our_double_array[] = { 0.1234, 9876.54321, -300.0 };
  const double *v_ARRAY_DOUBLE = our_double_array;
  const unsigned char our_byte_array[] = { 'a', 'b', 'c', 234 };
  const unsigned char *v_ARRAY_BYTE = our_byte_array;
  const dbus_bool_t our_boolean_array[] = { TRUE, FALSE, TRUE, TRUE, FALSE };
  const dbus_bool_t *v_ARRAY_BOOLEAN = our_boolean_array;
  char sig[64];
  const char *s;
  const char *v_STRING;
  double v_DOUBLE;
  dbus_int16_t v_INT16;
  dbus_uint16_t v_UINT16;
  dbus_int32_t v_INT32;
  dbus_uint32_t v_UINT32;
  dbus_uint32_t v1_UINT32;
  dbus_int64_t v_INT64;
  dbus_uint64_t v_UINT64;
  unsigned char v_BYTE;
  unsigned char v2_BYTE;
  dbus_bool_t v_BOOLEAN;
  DBusMessageIter iter, array_iter, struct_iter;
#ifdef HAVE_UNIX_FD_PASSING
  int v_UNIX_FD;
  int v1_UNIX_FD;
#endif
  char **decomposed;
  DBusInitialFDs *initial_fds;
  dbus_bool_t ok;
  char basic_types[] = DBUS_TYPE_BYTE_AS_STRING \
                       DBUS_TYPE_BOOLEAN_AS_STRING \
                       DBUS_TYPE_INT16_AS_STRING \
                       DBUS_TYPE_INT32_AS_STRING \
                       DBUS_TYPE_INT64_AS_STRING \
                       DBUS_TYPE_UINT16_AS_STRING \
                       DBUS_TYPE_UINT32_AS_STRING \
                       DBUS_TYPE_UINT64_AS_STRING \
                       DBUS_TYPE_DOUBLE_AS_STRING \
                       DBUS_TYPE_STRING_AS_STRING;

  initial_fds = _dbus_check_fdleaks_enter ();

  message = dbus_message_new_method_call ("org.freedesktop.DBus.TestService",
                                          "/org/freedesktop/TestPath",
                                          "Foo.TestInterface",
                                          "TestMethod");
  _dbus_assert (dbus_message_has_destination (message, "org.freedesktop.DBus.TestService"));
  _dbus_assert (dbus_message_is_method_call (message, "Foo.TestInterface",
                                             "TestMethod"));
  _dbus_assert (strcmp (dbus_message_get_path (message),
                        "/org/freedesktop/TestPath") == 0);
  dbus_message_set_serial (message, 1234);

  /* string length including nul byte not a multiple of 4 */
  if (!dbus_message_set_sender (message, "org.foo.bar1"))
    _dbus_test_fatal ("out of memory");

  _dbus_assert (dbus_message_has_sender (message, "org.foo.bar1"));
  dbus_message_set_reply_serial (message, 5678);

  _dbus_verbose_bytes_of_string (&message->header.data, 0,
                                 _dbus_string_get_length (&message->header.data));
  _dbus_verbose_bytes_of_string (&message->body, 0,
                                 _dbus_string_get_length (&message->body));

  if (!dbus_message_set_sender (message, NULL))
    _dbus_test_fatal ("out of memory");


  _dbus_verbose_bytes_of_string (&message->header.data, 0,
                                 _dbus_string_get_length (&message->header.data));
  _dbus_verbose_bytes_of_string (&message->body, 0,
                                 _dbus_string_get_length (&message->body));


  _dbus_assert (!dbus_message_has_sender (message, "org.foo.bar1"));
  _dbus_assert (dbus_message_get_serial (message) == 1234);
  _dbus_assert (dbus_message_get_reply_serial (message) == 5678);
  _dbus_assert (dbus_message_has_destination (message, "org.freedesktop.DBus.TestService"));

  _dbus_assert (dbus_message_get_no_reply (message) == FALSE);
  dbus_message_set_no_reply (message, TRUE);
  _dbus_assert (dbus_message_get_no_reply (message) == TRUE);
  dbus_message_set_no_reply (message, FALSE);
  _dbus_assert (dbus_message_get_no_reply (message) == FALSE);

  /* Set/get some header fields */

  if (!dbus_message_set_path (message, "/foo"))
    _dbus_test_fatal ("out of memory");
  _dbus_assert (strcmp (dbus_message_get_path (message),
                        "/foo") == 0);

  if (!dbus_message_set_container_instance (message, "/org/freedesktop/DBus/Containers1/c42"))
    _dbus_test_fatal ("out of memory");
  _dbus_assert (strcmp (dbus_message_get_container_instance (message),
                        "/org/freedesktop/DBus/Containers1/c42") == 0);

  if (!dbus_message_set_interface (message, "org.Foo"))
    _dbus_test_fatal ("out of memory");
  _dbus_assert (strcmp (dbus_message_get_interface (message),
                        "org.Foo") == 0);

  if (!dbus_message_set_member (message, "Bar"))
    _dbus_test_fatal ("out of memory");
  _dbus_assert (strcmp (dbus_message_get_member (message),
                        "Bar") == 0);

  /* Set/get them with longer values */
  if (!dbus_message_set_path (message, "/foo/bar"))
    _dbus_test_fatal ("out of memory");
  _dbus_assert (strcmp (dbus_message_get_path (message),
                        "/foo/bar") == 0);

  if (!dbus_message_set_interface (message, "org.Foo.Bar"))
    _dbus_test_fatal ("out of memory");
  _dbus_assert (strcmp (dbus_message_get_interface (message),
                        "org.Foo.Bar") == 0);

  if (!dbus_message_set_member (message, "BarFoo"))
    _dbus_test_fatal ("out of memory");
  _dbus_assert (strcmp (dbus_message_get_member (message),
                        "BarFoo") == 0);

  /* Realloc shorter again */

  if (!dbus_message_set_path (message, "/foo"))
    _dbus_test_fatal ("out of memory");
  _dbus_assert (strcmp (dbus_message_get_path (message),
                        "/foo") == 0);

  if (!dbus_message_set_interface (message, "org.Foo"))
    _dbus_test_fatal ("out of memory");
  _dbus_assert (strcmp (dbus_message_get_interface (message),
                        "org.Foo") == 0);

  if (!dbus_message_set_member (message, "Bar"))
    _dbus_test_fatal ("out of memory");
  _dbus_assert (strcmp (dbus_message_get_member (message),
                        "Bar") == 0);

  /* Path decomposing */
  dbus_message_set_path (message, NULL);
  dbus_message_get_path_decomposed (message, &decomposed);
  _dbus_assert (decomposed == NULL);
  dbus_free_string_array (decomposed);

  dbus_message_set_path (message, "/");
  dbus_message_get_path_decomposed (message, &decomposed);
  _dbus_assert (decomposed != NULL);
  _dbus_assert (decomposed[0] == NULL);
  dbus_free_string_array (decomposed);

  dbus_message_set_path (message, "/a/b");
  dbus_message_get_path_decomposed (message, &decomposed);
  _dbus_assert (decomposed != NULL);
  _dbus_assert (strcmp (decomposed[0], "a") == 0);
  _dbus_assert (strcmp (decomposed[1], "b") == 0);
  _dbus_assert (decomposed[2] == NULL);
  dbus_free_string_array (decomposed);

  dbus_message_set_path (message, "/spam/eggs");
  dbus_message_get_path_decomposed (message, &decomposed);
  _dbus_assert (decomposed != NULL);
  _dbus_assert (strcmp (decomposed[0], "spam") == 0);
  _dbus_assert (strcmp (decomposed[1], "eggs") == 0);
  _dbus_assert (decomposed[2] == NULL);
  dbus_free_string_array (decomposed);

  dbus_message_unref (message);

  /* Test the vararg functions */
  message = dbus_message_new_method_call ("org.freedesktop.DBus.TestService",
                                          "/org/freedesktop/TestPath",
                                          "Foo.TestInterface",
                                          "TestMethod");
  dbus_message_set_serial (message, 1);
  dbus_message_set_reply_serial (message, 5678);

  v_INT16 = -0x123;
  v_UINT16 = 0x123;
  v_INT32 = -0x12345678;
  v_UINT32 = 0x12300042;
  v_INT64 = DBUS_INT64_CONSTANT (-0x123456789abcd);
  v_UINT64 = DBUS_UINT64_CONSTANT (0x123456789abcd);
  v_STRING = "Test string";
  v_DOUBLE = 3.14159;
  v_BOOLEAN = TRUE;
  v_BYTE = 42;
  v2_BYTE = 24;
#ifdef HAVE_UNIX_FD_PASSING
  v_UNIX_FD = 1;
  v1_UNIX_FD = 2;
#endif

  dbus_message_append_args (message,
                            DBUS_TYPE_INT16, &v_INT16,
                            DBUS_TYPE_UINT16, &v_UINT16,
			    DBUS_TYPE_INT32, &v_INT32,
                            DBUS_TYPE_UINT32, &v_UINT32,
                            DBUS_TYPE_INT64, &v_INT64,
                            DBUS_TYPE_UINT64, &v_UINT64,
			    DBUS_TYPE_STRING, &v_STRING,
			    DBUS_TYPE_DOUBLE, &v_DOUBLE,
			    DBUS_TYPE_BOOLEAN, &v_BOOLEAN,
			    DBUS_TYPE_BYTE, &v_BYTE,
			    DBUS_TYPE_BYTE, &v2_BYTE,
			    DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &v_ARRAY_UINT32,
                            _DBUS_N_ELEMENTS (our_uint32_array),
                            DBUS_TYPE_ARRAY, DBUS_TYPE_INT32, &v_ARRAY_INT32,
                            _DBUS_N_ELEMENTS (our_int32_array),
                            DBUS_TYPE_ARRAY, DBUS_TYPE_UINT64, &v_ARRAY_UINT64,
                            _DBUS_N_ELEMENTS (our_uint64_array),
                            DBUS_TYPE_ARRAY, DBUS_TYPE_INT64, &v_ARRAY_INT64,
                            _DBUS_N_ELEMENTS (our_int64_array),
                            DBUS_TYPE_ARRAY, DBUS_TYPE_DOUBLE, &v_ARRAY_DOUBLE,
                            _DBUS_N_ELEMENTS (our_double_array),
                            DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &v_ARRAY_BYTE,
                            _DBUS_N_ELEMENTS (our_byte_array),
                            DBUS_TYPE_ARRAY, DBUS_TYPE_BOOLEAN, &v_ARRAY_BOOLEAN,
                            _DBUS_N_ELEMENTS (our_boolean_array),
                            DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &v_ARRAY_STRING,
                            _DBUS_N_ELEMENTS (our_string_array),

			    DBUS_TYPE_INVALID);

  i = 0;
  sig[i++] = DBUS_TYPE_INT16;
  sig[i++] = DBUS_TYPE_UINT16;
  sig[i++] = DBUS_TYPE_INT32;
  sig[i++] = DBUS_TYPE_UINT32;
  sig[i++] = DBUS_TYPE_INT64;
  sig[i++] = DBUS_TYPE_UINT64;
  sig[i++] = DBUS_TYPE_STRING;
  sig[i++] = DBUS_TYPE_DOUBLE;
  sig[i++] = DBUS_TYPE_BOOLEAN;
  sig[i++] = DBUS_TYPE_BYTE;
  sig[i++] = DBUS_TYPE_BYTE;
  sig[i++] = DBUS_TYPE_ARRAY;
  sig[i++] = DBUS_TYPE_UINT32;
  sig[i++] = DBUS_TYPE_ARRAY;
  sig[i++] = DBUS_TYPE_INT32;
  sig[i++] = DBUS_TYPE_ARRAY;
  sig[i++] = DBUS_TYPE_UINT64;
  sig[i++] = DBUS_TYPE_ARRAY;
  sig[i++] = DBUS_TYPE_INT64;
  sig[i++] = DBUS_TYPE_ARRAY;
  sig[i++] = DBUS_TYPE_DOUBLE;
  sig[i++] = DBUS_TYPE_ARRAY;
  sig[i++] = DBUS_TYPE_BYTE;
  sig[i++] = DBUS_TYPE_ARRAY;
  sig[i++] = DBUS_TYPE_BOOLEAN;
  sig[i++] = DBUS_TYPE_ARRAY;
  sig[i++] = DBUS_TYPE_STRING;

  message_without_unix_fds = dbus_message_copy(message);
  _dbus_assert(message_without_unix_fds);
#ifdef HAVE_UNIX_FD_PASSING
  dbus_message_append_args (message,
                            DBUS_TYPE_UNIX_FD, &v_UNIX_FD,
			    DBUS_TYPE_INVALID);
  sig[i++] = DBUS_TYPE_UNIX_FD;
#endif
  sig[i++] = DBUS_TYPE_INVALID;

  _dbus_assert (i < (int) _DBUS_N_ELEMENTS (sig));

  _dbus_verbose ("HEADER\n");
  _dbus_verbose_bytes_of_string (&message->header.data, 0,
                                 _dbus_string_get_length (&message->header.data));
  _dbus_verbose ("BODY\n");
  _dbus_verbose_bytes_of_string (&message->body, 0,
                                 _dbus_string_get_length (&message->body));

  _dbus_verbose ("Signature expected \"%s\" actual \"%s\"\n",
                 sig, dbus_message_get_signature (message));

  s = dbus_message_get_signature (message);

  _dbus_assert (dbus_message_has_signature (message, sig));
  _dbus_assert (strcmp (s, sig) == 0);

  verify_test_message (message);

  copy = dbus_message_copy (message);

  _dbus_assert (dbus_message_get_reply_serial (message) ==
                dbus_message_get_reply_serial (copy));
  _dbus_assert (message->header.padding == copy->header.padding);

  _dbus_assert (_dbus_string_get_length (&message->header.data) ==
                _dbus_string_get_length (&copy->header.data));

  _dbus_assert (_dbus_string_get_length (&message->body) ==
                _dbus_string_get_length (&copy->body));

  verify_test_message (copy);

  name1 = dbus_message_get_interface (message);
  name2 = dbus_message_get_interface (copy);

  _dbus_assert (strcmp (name1, name2) == 0);

  name1 = dbus_message_get_member (message);
  name2 = dbus_message_get_member (copy);

  _dbus_assert (strcmp (name1, name2) == 0);

  dbus_message_unref (copy);

  /* Message loader test */
  dbus_message_lock (message);
  loader = _dbus_message_loader_new ();

  /* check ref/unref */
  _dbus_message_loader_ref (loader);
  _dbus_message_loader_unref (loader);

  /* Write the header data one byte at a time */
  data = _dbus_string_get_const_data (&message->header.data);
  for (i = 0; i < _dbus_string_get_length (&message->header.data); i++)
    {
      DBusString *buffer;

      _dbus_message_loader_get_buffer (loader, &buffer, NULL, NULL);
      _dbus_string_append_byte (buffer, data[i]);
      _dbus_message_loader_return_buffer (loader, buffer);
    }

  /* Write the body data one byte at a time */
  data = _dbus_string_get_const_data (&message->body);
  for (i = 0; i < _dbus_string_get_length (&message->body); i++)
    {
      DBusString *buffer;

      _dbus_message_loader_get_buffer (loader, &buffer, NULL, NULL);
      _dbus_string_append_byte (buffer, data[i]);
      _dbus_message_loader_return_buffer (loader, buffer);
    }

#ifdef HAVE_UNIX_FD_PASSING
  {
    int *unix_fds;
    unsigned n_unix_fds;
    /* Write unix fd */
    _dbus_message_loader_get_unix_fds(loader, &unix_fds, &n_unix_fds);
    _dbus_assert(n_unix_fds > 0);
    _dbus_assert(message->n_unix_fds == 1);
    unix_fds[0] = _dbus_dup(message->unix_fds[0], NULL);
    _dbus_assert(unix_fds[0] >= 0);
    _dbus_message_loader_return_unix_fds(loader, unix_fds, 1);
  }
#endif

  dbus_message_unref (message);

  /* Now pop back the message */
  if (!_dbus_message_loader_queue_messages (loader))
    _dbus_test_fatal ("no memory to queue messages");

  if (_dbus_message_loader_get_is_corrupted (loader))
    _dbus_test_fatal ("message loader corrupted");

  message = _dbus_message_loader_pop_message (loader);
  if (!message)
    _dbus_test_fatal ("received a NULL message");

  if (dbus_message_get_reply_serial (message) != 5678)
    _dbus_test_fatal ("reply serial fields differ");

  dbus_message_unref (message);

  /* ovveride the serial, since it was reset by dbus_message_copy() */
  dbus_message_set_serial(message_without_unix_fds, 8901);

  dbus_message_lock (message_without_unix_fds);

  verify_test_message (message_without_unix_fds);

    {
      /* Marshal and demarshal the message. */

      DBusMessage *message2;
      DBusError error = DBUS_ERROR_INIT;
      char *marshalled = NULL;
      int len = 0;
      char garbage_header[DBUS_MINIMUM_HEADER_SIZE] = "xxx";

      if (!dbus_message_marshal (message_without_unix_fds, &marshalled, &len))
        _dbus_test_fatal ("failed to marshal message");

      _dbus_assert (len != 0);
      _dbus_assert (marshalled != NULL);

      _dbus_assert (dbus_message_demarshal_bytes_needed (marshalled, len) == len);
      message2 = dbus_message_demarshal (marshalled, len, &error);

      _dbus_assert (message2 != NULL);
      _dbus_assert (!dbus_error_is_set (&error));
      verify_test_message (message2);

      dbus_message_unref (message2);
      dbus_free (marshalled);

      /* Demarshal invalid message. */

      message2 = dbus_message_demarshal ("invalid", 7, &error);
      _dbus_assert (message2 == NULL);
      _dbus_assert (dbus_error_is_set (&error));
      dbus_error_free (&error);

      /* Demarshal invalid (empty) message. */

      message2 = dbus_message_demarshal ("", 0, &error);
      _dbus_assert (message2 == NULL);
      _dbus_assert (dbus_error_is_set (&error));
      dbus_error_free (&error);

      /* Bytes needed to demarshal empty message: 0 (more) */

      _dbus_assert (dbus_message_demarshal_bytes_needed ("", 0) == 0);

      /* Bytes needed to demarshal invalid message: -1 (error). */

      _dbus_assert (dbus_message_demarshal_bytes_needed (garbage_header, DBUS_MINIMUM_HEADER_SIZE) == -1);
    }

  dbus_message_unref (message_without_unix_fds);
  _dbus_message_loader_unref (loader);

  check_memleaks ();
  _dbus_check_fdleaks_leave (initial_fds, _DBUS_FILE_LINE);
  initial_fds = _dbus_check_fdleaks_enter ();

  /* Test enumeration of array elements */
  for (i = strlen (basic_types) - 1; i > 0; i--)
    {
      DBusBasicValue val;
      int some;
      char* signature = _dbus_strdup ("?");

      signature[0] = basic_types[i];
      s = "SomeThingToSay";
      memset (&val, '\0', sizeof (val));

      message = dbus_message_new_method_call ("de.ende.test",
        "/de/ende/test", "de.ende.Test", "ArtistName");
      _dbus_assert (message != NULL);
      dbus_message_iter_init_append (message, &iter);
      dbus_message_iter_open_container (&iter, DBUS_TYPE_ARRAY,
                                        signature, &array_iter);
      for (some = 0; some < 3; some++)
        {
          if (basic_types[i] == DBUS_TYPE_STRING)
            dbus_message_iter_append_basic (&array_iter, DBUS_TYPE_STRING, &s);
          else
            dbus_message_iter_append_basic (&array_iter, basic_types[i], &val);
        }
      dbus_message_iter_close_container (&iter, &array_iter);
      dbus_message_iter_init (message, &iter);
      _dbus_assert (dbus_message_iter_get_element_count (&iter) == some);
      dbus_message_unref (message);
      dbus_free (signature);
    }
  /* Array of structs */
  message = dbus_message_new_method_call ("de.ende.test",
      "/de/ende/test", "de.ende.Test", "ArtistName");
  _dbus_assert (message != NULL);
  dbus_message_iter_init_append (message, &iter);
  dbus_message_iter_open_container (&iter, DBUS_TYPE_ARRAY,
                                    DBUS_STRUCT_BEGIN_CHAR_AS_STRING
                                    DBUS_TYPE_STRING_AS_STRING
                                    DBUS_STRUCT_END_CHAR_AS_STRING, &array_iter);
  dbus_message_iter_open_container (&array_iter, DBUS_TYPE_STRUCT,
                                    NULL, &struct_iter);
  s = "SpamAndEggs";
  dbus_message_iter_append_basic (&struct_iter, DBUS_TYPE_STRING, &s);
  dbus_message_iter_close_container (&array_iter, &struct_iter);
  dbus_message_iter_close_container (&iter, &array_iter);
  dbus_message_iter_init (message, &iter);
  _dbus_assert (dbus_message_iter_get_element_count (&iter) == 1);
  dbus_message_unref (message);
  check_memleaks ();

  /* Check that we can abandon a container */
  message = dbus_message_new_method_call ("org.freedesktop.DBus.TestService",
					  "/org/freedesktop/TestPath",
					  "Foo.TestInterface",
					  "Method");

  dbus_message_iter_init_append (message, &iter);

  ok = dbus_message_iter_open_container (&iter, DBUS_TYPE_ARRAY,
						  (DBUS_STRUCT_BEGIN_CHAR_AS_STRING
						   DBUS_TYPE_STRING_AS_STRING
						   DBUS_TYPE_STRING_AS_STRING
						   DBUS_STRUCT_END_CHAR_AS_STRING),
                          &array_iter);
  _dbus_assert (ok);
  ok = dbus_message_iter_open_container (&array_iter, DBUS_TYPE_STRUCT,
                          NULL, &struct_iter);
  _dbus_assert (ok);
  s = "peaches";
  ok = dbus_message_iter_append_basic (&struct_iter, DBUS_TYPE_STRING, &s);
  _dbus_assert (ok);

  /* uh-oh, error, try and unwind */

  dbus_message_iter_abandon_container (&array_iter, &struct_iter);
  dbus_message_iter_abandon_container (&array_iter, &iter);

  dbus_message_unref (message);

  /* Check we should not leak array of string or unix fd, fd.o#21259 */
  message = dbus_message_new_method_call ("org.freedesktop.DBus.TestService",
                                          "/org/freedesktop/TestPath",
                                          "Foo.TestInterface",
                                          "Method");

  /* signature "uashuash" */
  dbus_message_append_args (message,
                            DBUS_TYPE_UINT32, &v_UINT32,
                            DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &v_ARRAY_STRING,
                            _DBUS_N_ELEMENTS (our_string_array),
#ifdef HAVE_UNIX_FD_PASSING
                            DBUS_TYPE_UNIX_FD, &v_UNIX_FD,
#endif
                            DBUS_TYPE_UINT32, &v1_UINT32,
                            DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &v1_ARRAY_STRING,
                            _DBUS_N_ELEMENTS (our_string_array1),
#ifdef HAVE_UNIX_FD_PASSING
                            DBUS_TYPE_UNIX_FD, &v1_UNIX_FD,
#endif

                            DBUS_TYPE_INVALID);

  i = 0;
  sig[i++] = DBUS_TYPE_UINT32;
  sig[i++] = DBUS_TYPE_ARRAY;
  sig[i++] = DBUS_TYPE_STRING;
#ifdef HAVE_UNIX_FD_PASSING
  sig[i++] = DBUS_TYPE_UNIX_FD;
#endif
  sig[i++] = DBUS_TYPE_UINT32;
  sig[i++] = DBUS_TYPE_ARRAY;
  sig[i++] = DBUS_TYPE_STRING;
#ifdef HAVE_UNIX_FD_PASSING
  sig[i++] = DBUS_TYPE_UNIX_FD;
#endif
  sig[i++] = DBUS_TYPE_INVALID;

  _dbus_assert (i < (int) _DBUS_N_ELEMENTS (sig));

  verify_test_message_args_ignored (message, _DBUS_FILE_LINE);
  verify_test_message_memleak (message, _DBUS_FILE_LINE);

  dbus_message_unref (message);

  /* Load all the sample messages from the message factory */
  {
    DBusMessageDataIter diter;
    DBusMessageData mdata;
    int count;

    reset_validities_seen ();

    count = 0;
    _dbus_message_data_iter_init (&diter);

    while (_dbus_message_data_iter_get_and_next (&diter,
                                                 &mdata))
      {
        if (!try_message_data (&mdata.data, mdata.expected_validity))
          {
            _dbus_test_fatal ("expected validity %d and did not get it",
                        mdata.expected_validity);
          }

        _dbus_message_data_free (&mdata);

        count += 1;
      }

    _dbus_test_diag ("%d sample messages tested", count);

    print_validities_seen (FALSE);
    print_validities_seen (TRUE);
  }

  check_memleaks ();
  _dbus_check_fdleaks_leave (initial_fds, _DBUS_FILE_LINE);

  /* Now load every message in test_data_dir if we have one */
  if (test_data_dir == NULL)
    return TRUE;

  initial_fds = _dbus_check_fdleaks_enter ();

  if (!foreach_message_file (test_data_dir, try_message_file, NULL))
    _dbus_test_fatal ("foreach_message_file test failed");

  _dbus_check_fdleaks_leave (initial_fds, _DBUS_FILE_LINE);

  return TRUE;
}

#endif /* DBUS_ENABLE_EMBEDDED_TESTS */
