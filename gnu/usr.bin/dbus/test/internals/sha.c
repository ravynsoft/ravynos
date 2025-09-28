/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-sha.c SHA-1 implementation
 *
 * Copyright (C) 2003 Red Hat Inc.
 * Copyright (C) 1995 A. M. Kuchling
 * Copyright (C) 2017 Thomas Zimmermann
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

#ifdef DBUS_ENABLE_EMBEDDED_TESTS

#include <stdio.h>
#include <string.h>

#include <dbus/dbus-internals.h>
#include <dbus/dbus-sha.h>
#include <dbus/dbus-test-tap.h>
#include <dbus/dbus-test.h>

static dbus_bool_t
check_sha_binary (const unsigned char *input,
                  int                  input_len,
                  const char          *expected)
{
  DBusString input_str;
  DBusString expected_str;
  DBusString results;

  _dbus_string_init_const_len (&input_str, (const char *) input, input_len);
  _dbus_string_init_const (&expected_str, expected);

  if (!_dbus_string_init (&results))
    _dbus_test_fatal ("no memory for SHA-1 results");

  if (!_dbus_sha_compute (&input_str, &results))
    _dbus_test_fatal ("no memory for SHA-1 results");

  if (!_dbus_string_equal (&expected_str, &results))
    {
      _dbus_warn ("Expected hash %s got %s for SHA-1 sum",
                  expected,
                  _dbus_string_get_const_data (&results));
      _dbus_string_free (&results);
      return FALSE;
    }

  _dbus_string_free (&results);
  return TRUE;
}

static dbus_bool_t
check_sha_str (const char *input,
               const char *expected)
{
  return check_sha_binary ((unsigned char *) input, strlen (input), expected);
}

static dbus_bool_t
decode_compact_string (const DBusString *line,
                       DBusString       *decoded)
{
  int n_bits;
  dbus_bool_t current_b;
  int offset;
  int next;
  long val;
  int length_bytes;

  offset = 0;
  next = 0;

  if (!_dbus_string_parse_int (line, offset, &val, &next))
    {
      fprintf (stderr, "could not parse length at start of compact string: %s\n",
               _dbus_string_get_const_data (line));
      return FALSE;
    }

  _dbus_string_skip_blank (line, next, &next);

  offset = next;
  if (!_dbus_string_parse_int (line, offset, &val, &next))
    {
      fprintf (stderr, "could not parse start bit 'b' in compact string: %s\n",
               _dbus_string_get_const_data (line));
      return FALSE;
    }

  if (!(val == 0 || val == 1))
    {
      fprintf (stderr, "the value 'b' must be 0 or 1, see sha-1/Readme.txt\n");
      return FALSE;
    }

  _dbus_string_skip_blank (line, next, &next);

  current_b = val;
  n_bits = 0;

  while (next < _dbus_string_get_length (line))
    {
      int total_bits;

      offset = next;

      if (_dbus_string_get_byte (line, offset) == '^')
        break;

      if (!_dbus_string_parse_int (line, offset, &val, &next))
        {
          fprintf (stderr, "could not parse bit count in compact string\n");
          return FALSE;
        }

      /* We now append "val" copies of "current_b" bits to the string */
      total_bits = n_bits + val;
      while (n_bits < total_bits)
        {
          int byte_containing_next_bit = n_bits / 8;
          int bit_containing_next_bit = 7 - (n_bits % 8);
          unsigned char old_byte;

          if (byte_containing_next_bit >= _dbus_string_get_length (decoded))
            {
              if (!_dbus_string_set_length (decoded, byte_containing_next_bit + 1))
                _dbus_test_fatal ("no memory to extend to next byte");
            }

          old_byte = _dbus_string_get_byte (decoded, byte_containing_next_bit);
          old_byte |= current_b << bit_containing_next_bit;

#if 0
          _dbus_test_diag ("Appending bit %d to byte %d at bit %d resulting in byte 0x%x",
                  current_b, byte_containing_next_bit,
                  bit_containing_next_bit, old_byte);
#endif

          _dbus_string_set_byte (decoded, byte_containing_next_bit, old_byte);

          ++n_bits;
        }

      _dbus_string_skip_blank (line, next, &next);

      current_b = !current_b;
    }

  length_bytes = (n_bits / 8 + ((n_bits % 8) ? 1 : 0));

  if (_dbus_string_get_length (decoded) != length_bytes)
    {
      fprintf (stderr, "Expected length %d bytes %d bits for compact string, got %d bytes\n",
               length_bytes, n_bits, _dbus_string_get_length (decoded));
      return FALSE;
    }
  else
    return TRUE;
}

static dbus_bool_t
get_next_expected_result (DBusString *results,
                          DBusString *result)
{
  DBusString line;
  dbus_bool_t retval;

  retval = FALSE;

  if (!_dbus_string_init (&line))
    _dbus_test_fatal ("no memory");

 next_iteration:
  while (_dbus_string_pop_line (results, &line))
    {
      _dbus_string_delete_leading_blanks (&line);

      if (_dbus_string_get_length (&line) == 0)
        goto next_iteration;
      else if (_dbus_string_starts_with_c_str (&line, "#"))
        goto next_iteration;
      else if (_dbus_string_starts_with_c_str (&line, "H>"))
        {
          /* don't print */
        }
      else if (_dbus_string_starts_with_c_str (&line, "D>") ||
               _dbus_string_starts_with_c_str (&line, "<D"))
        goto next_iteration;
      else
        {
          int i;

          if (!_dbus_string_move (&line, 0, result, 0))
            _dbus_test_fatal ("no memory");

          i = 0;
          while (i < _dbus_string_get_length (result))
            {
              unsigned char c = _dbus_string_get_byte (result, i);

              switch (c)
                {
                case 'A':
                  _dbus_string_set_byte (result, i, 'a');
                  break;
                case 'B':
                  _dbus_string_set_byte (result, i, 'b');
                  break;
                case 'C':
                  _dbus_string_set_byte (result, i, 'c');
                  break;
                case 'D':
                  _dbus_string_set_byte (result, i, 'd');
                  break;
                case 'E':
                  _dbus_string_set_byte (result, i, 'e');
                  break;
                case 'F':
                  _dbus_string_set_byte (result, i, 'f');
                  break;
                case '^':
                case ' ':
                  _dbus_string_delete (result, i, 1);
                  --i; /* to offset ++i below */
                  break;
                default:
                  if ((c < '0' || c > '9') && (c < 'a' || c > 'f'))
                    _dbus_test_fatal ("invalid SHA-1 test script");
                }

              ++i;
            }

          break;
        }
    }

  retval = TRUE;

  /* out: */
  _dbus_string_free (&line);
  return retval;
}

static dbus_bool_t
process_test_data (const char *test_data_dir)
{
  DBusString tests_file;
  DBusString results_file;
  DBusString tests;
  DBusString results;
  DBusString line;
  DBusString tmp;
  int line_no;
  dbus_bool_t retval;
  int success_count;
  DBusError error = DBUS_ERROR_INIT;

  retval = FALSE;

  if (!_dbus_string_init (&tests_file))
    _dbus_test_fatal ("no memory");

  if (!_dbus_string_init (&results_file))
    _dbus_test_fatal ("no memory");

  if (!_dbus_string_init (&tests))
    _dbus_test_fatal ("no memory");

  if (!_dbus_string_init (&results))
    _dbus_test_fatal ("no memory");

  if (!_dbus_string_init (&line))
    _dbus_test_fatal ("no memory");

  if (!_dbus_string_append (&tests_file, test_data_dir))
    _dbus_test_fatal ("no memory");

  if (!_dbus_string_append (&results_file, test_data_dir))
    _dbus_test_fatal ("no memory");

  _dbus_string_init_const (&tmp, "sha-1/byte-messages.sha1");
  if (!_dbus_concat_dir_and_file (&tests_file, &tmp))
    _dbus_test_fatal ("no memory");

  _dbus_string_init_const (&tmp, "sha-1/byte-hashes.sha1");
  if (!_dbus_concat_dir_and_file (&results_file, &tmp))
    _dbus_test_fatal ("no memory");

  if (!_dbus_file_get_contents (&tests, &tests_file, &error))
    {
      fprintf (stderr, "could not load test data file %s: %s\n",
               _dbus_string_get_const_data (&tests_file),
               error.message);
      dbus_error_free (&error);
      goto out;
    }

  if (!_dbus_file_get_contents (&results, &results_file, &error))
    {
      fprintf (stderr, "could not load results data file %s: %s\n",
               _dbus_string_get_const_data (&results_file), error.message);
      dbus_error_free (&error);
      goto out;
    }

  success_count = 0;
  line_no = 0;
 next_iteration:
  while (_dbus_string_pop_line (&tests, &line))
    {
      line_no += 1;

      _dbus_string_delete_leading_blanks (&line);

      if (_dbus_string_get_length (&line) == 0)
        goto next_iteration;
      else if (_dbus_string_starts_with_c_str (&line, "#"))
        goto next_iteration;
      else if (_dbus_string_starts_with_c_str (&line, "H>"))
        {
          _dbus_test_diag ("SHA-1: %s", _dbus_string_get_const_data (&line));

          if (_dbus_string_find (&line, 0, "Type 3", NULL))
            {
              /* See sha-1/Readme.txt - the "Type 3" tests are
               * random seeds, rather than data to be hashed.
               * we'd have to do a little bit more implementation
               * to use those tests.
               */

              _dbus_test_diag (" (ending tests due to Type 3 tests seen - this is normal)");
              break;
            }
        }
      else if (_dbus_string_starts_with_c_str (&line, "D>") ||
               _dbus_string_starts_with_c_str (&line, "<D"))
        goto next_iteration;
      else
        {
          DBusString test;
          DBusString result;
          DBusString next_line;
          DBusString expected;
          dbus_bool_t success;

          success = FALSE;

          if (!_dbus_string_init (&next_line))
            _dbus_test_fatal ("no memory");

          if (!_dbus_string_init (&expected))
            _dbus_test_fatal ("no memory");

          if (!_dbus_string_init (&test))
            _dbus_test_fatal ("no memory");

          if (!_dbus_string_init (&result))
            _dbus_test_fatal ("no memory");

          /* the "compact strings" are "^"-terminated not
           * newline-terminated so readahead to find the
           * "^"
           */
          while (!_dbus_string_find (&line, 0, "^", NULL) &&
                 _dbus_string_pop_line (&tests, &next_line))
            {
              if (!_dbus_string_append_byte (&line, ' ') ||
                  !_dbus_string_move (&next_line, 0, &line,
                                      _dbus_string_get_length (&line)))
                _dbus_test_fatal ("no memory");
            }

          if (!decode_compact_string (&line, &test))
            {
              fprintf (stderr, "Failed to decode line %d as a compact string\n",
                       line_no);
              goto failure;
            }

          if (!_dbus_sha_compute (&test, &result))
            _dbus_test_fatal ("no memory for SHA-1 result");

          if (!get_next_expected_result (&results, &expected))
            {
              fprintf (stderr, "Failed to read an expected result\n");
              goto failure;
            }

          if (!_dbus_string_equal (&result, &expected))
            {
              fprintf (stderr, " for line %d got hash %s expected %s\n",
                       line_no,
                       _dbus_string_get_const_data (&result),
                       _dbus_string_get_const_data (&expected));
              goto failure;
            }
          else
            {
              success_count += 1;
            }

          success = TRUE;

        failure:
          _dbus_string_free (&test);
          _dbus_string_free (&result);
          _dbus_string_free (&next_line);
          _dbus_string_free (&expected);

          if (!success)
            goto out;
        }
    }

  retval = TRUE;

  _dbus_test_diag ("Passed the %d SHA-1 tests in the test file",
          success_count);

 out:
  _dbus_string_free (&tests_file);
  _dbus_string_free (&results_file);
  _dbus_string_free (&tests);
  _dbus_string_free (&results);
  _dbus_string_free (&line);

  return retval;
}

/**
 * @ingroup DBusSHAInternals
 * Unit test for SHA computation.
 *
 * @returns #TRUE on success.
 */
dbus_bool_t
_dbus_sha_test (const char *test_data_dir)
{
  unsigned char all_bytes[256];
  int i;

  if (test_data_dir != NULL)
    {
      if (!process_test_data (test_data_dir))
        return FALSE;
    }
  else
    _dbus_test_diag ("No test data dir");

  i = 0;
  while (i < 256)
    {
      all_bytes[i] = i;
      ++i;
    }

  if (!check_sha_binary (all_bytes, 256,
                         "4916d6bdb7f78e6803698cab32d1586ea457dfc8"))
    return FALSE;

#define CHECK(input,expected) if (!check_sha_str (input, expected)) return FALSE

  CHECK ("", "da39a3ee5e6b4b0d3255bfef95601890afd80709");
  CHECK ("a", "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8");
  CHECK ("abc", "a9993e364706816aba3e25717850c26c9cd0d89d");
  CHECK ("message digest", "c12252ceda8be8994d5fa0290a47231c1d16aae3");
  CHECK ("abcdefghijklmnopqrstuvwxyz", "32d10c7b8cf96570ca04ce37f2a19d84240d3a89");
  CHECK ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
         "761c457bf73b14d27e9e9265c46f4b4dda11f940");
  CHECK ("12345678901234567890123456789012345678901234567890123456789012345678901234567890",
         "50abf5706a150990a08b2c5ea40fa0e585554732");

  return TRUE;
}

#endif
