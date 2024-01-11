/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-marshal-recursive-util.c  Would be in dbus-marshal-recursive.c, but only used in bus/tests
 *
 * Copyright (C) 2004, 2005 Red Hat, Inc.
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

#include "dbus-marshal-recursive-util.h"

#ifdef DBUS_ENABLE_EMBEDDED_TESTS

#include "dbus/dbus-marshal-recursive.h"
#include "dbus/dbus-marshal-basic.h"
#include "dbus/dbus-signature.h"
#include "dbus/dbus-internals.h"
#include <dbus/dbus-test-tap.h>
#include <string.h>

#if !defined(PRIx64) && defined(DBUS_WIN)
#define PRIx64 "I64x"
#endif

/** turn this on to get deluged in TypeWriter verbose spam */
#define RECURSIVE_MARSHAL_WRITE_TRACE 0

static void
basic_value_zero (DBusBasicValue *value)
{
  value->u64 = 0;
}

static dbus_bool_t
basic_value_equal (int             type,
                   DBusBasicValue *lhs,
                   DBusBasicValue *rhs)
{
  if (type == DBUS_TYPE_STRING ||
      type == DBUS_TYPE_SIGNATURE ||
      type == DBUS_TYPE_OBJECT_PATH)
    {
      return strcmp (lhs->str, rhs->str) == 0;
    }
  else
    {
      return lhs->u64 == rhs->u64;
    }
}

static dbus_bool_t
equal_values_helper (DBusTypeReader *lhs,
                     DBusTypeReader *rhs)
{
  int lhs_type;
  int rhs_type;

  lhs_type = _dbus_type_reader_get_current_type (lhs);
  rhs_type = _dbus_type_reader_get_current_type (rhs);

  if (lhs_type != rhs_type)
    return FALSE;

  if (lhs_type == DBUS_TYPE_INVALID)
    return TRUE;

  if (dbus_type_is_basic (lhs_type))
    {
      DBusBasicValue lhs_value;
      DBusBasicValue rhs_value;

      basic_value_zero (&lhs_value);
      basic_value_zero (&rhs_value);

      _dbus_type_reader_read_basic (lhs, &lhs_value);
      _dbus_type_reader_read_basic (rhs, &rhs_value);

      return basic_value_equal (lhs_type, &lhs_value, &rhs_value);
    }
  else
    {
      DBusTypeReader lhs_sub;
      DBusTypeReader rhs_sub;

      _dbus_type_reader_recurse (lhs, &lhs_sub);
      _dbus_type_reader_recurse (rhs, &rhs_sub);

      return equal_values_helper (&lhs_sub, &rhs_sub);
    }
}

/**
 * See whether the two readers point to identical data blocks.
 *
 * @param lhs reader 1
 * @param rhs reader 2
 * @returns #TRUE if the data blocks have the same values
 */
dbus_bool_t
_dbus_type_reader_equal_values (const DBusTypeReader *lhs,
                                const DBusTypeReader *rhs)
{
  DBusTypeReader copy_lhs = *lhs;
  DBusTypeReader copy_rhs = *rhs;

  return equal_values_helper (&copy_lhs, &copy_rhs);
}

/* TESTS */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "dbus/dbus-test.h"
#include "dbus/dbus-list.h"
#include <stdio.h>
#include <stdlib.h>

/* Whether to do the OOM stuff (only with other expensive tests) */
#define TEST_OOM_HANDLING 0
/* We do start offset 0 through 9, to get various alignment cases. Still this
 * obviously makes the test suite run 10x as slow.
 */
#define MAX_INITIAL_OFFSET 9

/* Largest iteration count to test copying, realignment,
 * etc. with. i.e. we only test this stuff with some of the smaller
 * data sets.
 */
#define MAX_ITERATIONS_FOR_EXPENSIVE_TESTS 1000

typedef struct
{
  int byte_order;
  int initial_offset;
  DBusString signature;
  DBusString body;
} DataBlock;

typedef struct
{
  int saved_sig_len;
  int saved_body_len;
} DataBlockState;

#define N_FENCE_BYTES 5
#define FENCE_BYTES_STR "abcde"
#define INITIAL_PADDING_BYTE '\0'

static dbus_bool_t
data_block_init (DataBlock *block,
                 int        byte_order,
                 int        initial_offset)
{
  if (!_dbus_string_init (&block->signature))
    return FALSE;

  if (!_dbus_string_init (&block->body))
    {
      _dbus_string_free (&block->signature);
      return FALSE;
    }

  if (!_dbus_string_insert_bytes (&block->signature, 0, initial_offset,
                                  INITIAL_PADDING_BYTE) ||
      !_dbus_string_insert_bytes (&block->body, 0, initial_offset,
                                  INITIAL_PADDING_BYTE) ||
      !_dbus_string_append (&block->signature, FENCE_BYTES_STR) ||
      !_dbus_string_append (&block->body, FENCE_BYTES_STR))
    {
      _dbus_string_free (&block->signature);
      _dbus_string_free (&block->body);
      return FALSE;
    }

  block->byte_order = byte_order;
  block->initial_offset = initial_offset;

  return TRUE;
}

static void
data_block_save (DataBlock      *block,
                 DataBlockState *state)
{
  state->saved_sig_len = _dbus_string_get_length (&block->signature) - N_FENCE_BYTES;
  state->saved_body_len = _dbus_string_get_length (&block->body) - N_FENCE_BYTES;
}

static void
data_block_restore (DataBlock      *block,
                    DataBlockState *state)
{
  _dbus_string_delete (&block->signature,
                       state->saved_sig_len,
                       _dbus_string_get_length (&block->signature) - state->saved_sig_len - N_FENCE_BYTES);
  _dbus_string_delete (&block->body,
                       state->saved_body_len,
                       _dbus_string_get_length (&block->body) - state->saved_body_len - N_FENCE_BYTES);
}

static void
data_block_verify (DataBlock *block)
{
  if (!_dbus_string_ends_with_c_str (&block->signature,
                                     FENCE_BYTES_STR))
    {
      int offset;

      offset = _dbus_string_get_length (&block->signature) - N_FENCE_BYTES - 8;
      if (offset < 0)
        offset = 0;

      _dbus_verbose_bytes_of_string (&block->signature,
                                     offset,
                                     _dbus_string_get_length (&block->signature) - offset);
      _dbus_test_fatal ("block did not verify: bad bytes at end of signature");
    }
  if (!_dbus_string_ends_with_c_str (&block->body,
                                     FENCE_BYTES_STR))
    {
      int offset;

      offset = _dbus_string_get_length (&block->body) - N_FENCE_BYTES - 8;
      if (offset < 0)
        offset = 0;

      _dbus_verbose_bytes_of_string (&block->body,
                                     offset,
                                     _dbus_string_get_length (&block->body) - offset);
      _dbus_test_fatal ("block did not verify: bad bytes at end of body");
    }

  _dbus_assert (_dbus_string_validate_nul (&block->signature,
                                           0, block->initial_offset));
  _dbus_assert (_dbus_string_validate_nul (&block->body,
                                           0, block->initial_offset));
}

static void
data_block_free (DataBlock *block)
{
  data_block_verify (block);

  _dbus_string_free (&block->signature);
  _dbus_string_free (&block->body);
}

static void
data_block_reset (DataBlock *block)
{
  data_block_verify (block);

  _dbus_string_delete (&block->signature,
                       block->initial_offset,
                       _dbus_string_get_length (&block->signature) - N_FENCE_BYTES - block->initial_offset);
  _dbus_string_delete (&block->body,
                       block->initial_offset,
                       _dbus_string_get_length (&block->body) - N_FENCE_BYTES - block->initial_offset);

  data_block_verify (block);
}

static void
data_block_init_reader_writer (DataBlock      *block,
                               DBusTypeReader *reader,
                               DBusTypeWriter *writer)
{
  if (reader)
    _dbus_type_reader_init (reader,
                            block->byte_order,
                            &block->signature,
                            block->initial_offset,
                            &block->body,
                            block->initial_offset);

  if (writer)
    _dbus_type_writer_init (writer,
                            block->byte_order,
                            &block->signature,
                            _dbus_string_get_length (&block->signature) - N_FENCE_BYTES,
                            &block->body,
                            _dbus_string_get_length (&block->body) - N_FENCE_BYTES);
}

static void
real_check_expected_type (DBusTypeReader *reader,
                          int             expected,
                          const char     *funcname,
                          int             line)
{
  int t;

  t = _dbus_type_reader_get_current_type (reader);

  if (t != expected)
    {
      _dbus_test_fatal ("Read wrong type: read type %s while expecting %s at %s line %d",
                  _dbus_type_to_string (t),
                  _dbus_type_to_string (expected),
                  funcname, line);
    }
}

#define check_expected_type(reader, expected) real_check_expected_type (reader, expected, _DBUS_FUNCTION_NAME, __LINE__)

#define NEXT_EXPECTING_TRUE(reader)  do { if (!_dbus_type_reader_next (reader))         \
 {                                                                                      \
    _dbus_test_fatal ("_dbus_type_reader_next() should have returned TRUE at %s %d",    \
                              _DBUS_FUNCTION_NAME, __LINE__);                           \
 }                                                                                      \
} while (0)

#define NEXT_EXPECTING_FALSE(reader) do { if (_dbus_type_reader_next (reader))          \
 {                                                                                      \
    _dbus_test_fatal ("_dbus_type_reader_next() should have returned FALSE at %s %d",   \
                              _DBUS_FUNCTION_NAME, __LINE__);                           \
 }                                                                                      \
 check_expected_type (reader, DBUS_TYPE_INVALID);                                       \
} while (0)

typedef struct TestTypeNode               TestTypeNode;
typedef struct TestTypeNodeClass          TestTypeNodeClass;
typedef struct TestTypeNodeContainer      TestTypeNodeContainer;
typedef struct TestTypeNodeContainerClass TestTypeNodeContainerClass;

struct TestTypeNode
{
  const TestTypeNodeClass *klass;
};

struct TestTypeNodeContainer
{
  TestTypeNode base;
  DBusList    *children;
};

struct TestTypeNodeClass
{
  int typecode;

  int instance_size;

  int subclass_detail; /* a bad hack to avoid a bunch of subclass casting */

  dbus_bool_t   (* construct)     (TestTypeNode   *node);
  void          (* destroy)       (TestTypeNode   *node);

  dbus_bool_t (* write_value)     (TestTypeNode   *node,
                                   DataBlock      *block,
                                   DBusTypeWriter *writer,
                                   unsigned int    seed);
  dbus_bool_t (* read_value)      (TestTypeNode   *node,
                                   DBusTypeReader *reader,
                                   unsigned int    seed);
  dbus_bool_t (* set_value)       (TestTypeNode   *node,
                                   DBusTypeReader *reader,
                                   DBusTypeReader *realign_root,
                                   unsigned int    seed);
  dbus_bool_t (* build_signature) (TestTypeNode   *node,
                                   DBusString     *str);
  dbus_bool_t (* write_multi)     (TestTypeNode   *node,
                                   DataBlock      *block,
                                   DBusTypeWriter *writer,
                                   unsigned int    seed,
                                   int             count);
  dbus_bool_t (* read_multi)      (TestTypeNode   *node,
                                   DBusTypeReader *reader,
                                   unsigned int    seed,
                                   int             count);
};

struct TestTypeNodeContainerClass
{
  TestTypeNodeClass base;
};

/* FIXME this could be chilled out substantially by unifying
 * the basic types into basic_write_value/basic_read_value
 * and by merging read_value and set_value into one function
 * taking a flag argument.
 */
static dbus_bool_t uint16_write_value      (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t uint16_read_value       (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t uint16_set_value        (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t uint16_write_multi      (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed,
                                            int             count);
static dbus_bool_t uint16_read_multi       (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed,
                                            int             count);
static dbus_bool_t uint32_write_value      (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t uint32_read_value       (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t uint32_set_value        (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t uint32_write_multi      (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed,
                                            int             count);
static dbus_bool_t uint32_read_multi       (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed,
                                            int             count);
static dbus_bool_t uint64_write_value      (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t uint64_read_value       (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t uint64_set_value        (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t string_write_value      (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t string_read_value       (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t string_set_value        (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t bool_write_value        (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t bool_read_value         (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t bool_set_value          (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t byte_write_value        (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t byte_read_value         (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t byte_set_value          (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t double_write_value      (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t double_read_value       (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t double_set_value        (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t object_path_write_value (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t object_path_read_value  (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t object_path_set_value   (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t signature_write_value   (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t signature_read_value    (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t signature_set_value     (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t struct_write_value      (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t struct_read_value       (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t struct_set_value        (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t struct_build_signature  (TestTypeNode   *node,
                                            DBusString     *str);
static dbus_bool_t dict_write_value        (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t dict_read_value         (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t dict_set_value          (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t dict_build_signature    (TestTypeNode   *node,
                                            DBusString     *str);
static dbus_bool_t array_write_value       (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t array_read_value        (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t array_set_value         (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static dbus_bool_t array_build_signature   (TestTypeNode   *node,
                                            DBusString     *str);
static dbus_bool_t variant_write_value     (TestTypeNode   *node,
                                            DataBlock      *block,
                                            DBusTypeWriter *writer,
                                            unsigned int    seed);
static dbus_bool_t variant_read_value      (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            unsigned int    seed);
static dbus_bool_t variant_set_value       (TestTypeNode   *node,
                                            DBusTypeReader *reader,
                                            DBusTypeReader *realign_root,
                                            unsigned int    seed);
static void        container_destroy       (TestTypeNode   *node);



static const TestTypeNodeClass int16_class = {
  DBUS_TYPE_INT16,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  uint16_write_value, /* recycle from uint16 */
  uint16_read_value,
  uint16_set_value,
  NULL,
  uint16_write_multi,
  uint16_read_multi
};

static const TestTypeNodeClass uint16_class = {
  DBUS_TYPE_UINT16,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  uint16_write_value,
  uint16_read_value,
  uint16_set_value,
  NULL,
  uint16_write_multi,
  uint16_read_multi
};

static const TestTypeNodeClass int32_class = {
  DBUS_TYPE_INT32,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  uint32_write_value, /* recycle from int32 */
  uint32_read_value,
  uint32_set_value,
  NULL,
  uint32_write_multi,
  uint32_read_multi
};

static const TestTypeNodeClass uint32_class = {
  DBUS_TYPE_UINT32,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  uint32_write_value,
  uint32_read_value,
  uint32_set_value,
  NULL,
  uint32_write_multi,
  uint32_read_multi
};

static const TestTypeNodeClass int64_class = {
  DBUS_TYPE_INT64,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  uint64_write_value, /* recycle from uint64 */
  uint64_read_value,
  uint64_set_value,
  NULL,
  NULL, /* FIXME */
  NULL  /* FIXME */
};

static const TestTypeNodeClass uint64_class = {
  DBUS_TYPE_UINT64,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  uint64_write_value,
  uint64_read_value,
  uint64_set_value,
  NULL,
  NULL, /* FIXME */
  NULL  /* FIXME */
};

static const TestTypeNodeClass string_0_class = {
  DBUS_TYPE_STRING,
  sizeof (TestTypeNode),
  0, /* string length */
  NULL,
  NULL,
  string_write_value,
  string_read_value,
  string_set_value,
  NULL,
  NULL,
  NULL
};

static const TestTypeNodeClass string_1_class = {
  DBUS_TYPE_STRING,
  sizeof (TestTypeNode),
  1, /* string length */
  NULL,
  NULL,
  string_write_value,
  string_read_value,
  string_set_value,
  NULL,
  NULL,
  NULL
};

/* with nul, a len 3 string should fill 4 bytes and thus is "special" */
static const TestTypeNodeClass string_3_class = {
  DBUS_TYPE_STRING,
  sizeof (TestTypeNode),
  3, /* string length */
  NULL,
  NULL,
  string_write_value,
  string_read_value,
  string_set_value,
  NULL,
  NULL,
  NULL
};

/* with nul, a len 8 string should fill 9 bytes and thus is "special" (far-fetched I suppose) */
static const TestTypeNodeClass string_8_class = {
  DBUS_TYPE_STRING,
  sizeof (TestTypeNode),
  8, /* string length */
  NULL,
  NULL,
  string_write_value,
  string_read_value,
  string_set_value,
  NULL,
  NULL,
  NULL
};

static const TestTypeNodeClass bool_class = {
  DBUS_TYPE_BOOLEAN,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  bool_write_value,
  bool_read_value,
  bool_set_value,
  NULL,
  NULL, /* FIXME */
  NULL  /* FIXME */
};

static const TestTypeNodeClass byte_class = {
  DBUS_TYPE_BYTE,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  byte_write_value,
  byte_read_value,
  byte_set_value,
  NULL,
  NULL, /* FIXME */
  NULL  /* FIXME */
};

static const TestTypeNodeClass double_class = {
  DBUS_TYPE_DOUBLE,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  double_write_value,
  double_read_value,
  double_set_value,
  NULL,
  NULL, /* FIXME */
  NULL  /* FIXME */
};

static const TestTypeNodeClass object_path_class = {
  DBUS_TYPE_OBJECT_PATH,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  object_path_write_value,
  object_path_read_value,
  object_path_set_value,
  NULL,
  NULL,
  NULL
};

static const TestTypeNodeClass signature_class = {
  DBUS_TYPE_SIGNATURE,
  sizeof (TestTypeNode),
  0,
  NULL,
  NULL,
  signature_write_value,
  signature_read_value,
  signature_set_value,
  NULL,
  NULL,
  NULL
};

static const TestTypeNodeClass struct_1_class = {
  DBUS_TYPE_STRUCT,
  sizeof (TestTypeNodeContainer),
  1, /* number of times children appear as fields */
  NULL,
  container_destroy,
  struct_write_value,
  struct_read_value,
  struct_set_value,
  struct_build_signature,
  NULL,
  NULL
};

static const TestTypeNodeClass struct_2_class = {
  DBUS_TYPE_STRUCT,
  sizeof (TestTypeNodeContainer),
  2, /* number of times children appear as fields */
  NULL,
  container_destroy,
  struct_write_value,
  struct_read_value,
  struct_set_value,
  struct_build_signature,
  NULL,
  NULL
};

static const TestTypeNodeClass dict_1_class = {
  DBUS_TYPE_ARRAY, /* this is correct, a dict is an array of dict entry */
  sizeof (TestTypeNodeContainer),
  1, /* number of entries */
  NULL,
  container_destroy,
  dict_write_value,
  dict_read_value,
  dict_set_value,
  dict_build_signature,
  NULL,
  NULL
};

static dbus_bool_t arrays_write_fixed_in_blocks = FALSE;

static const TestTypeNodeClass array_0_class = {
  DBUS_TYPE_ARRAY,
  sizeof (TestTypeNodeContainer),
  0, /* number of array elements */
  NULL,
  container_destroy,
  array_write_value,
  array_read_value,
  array_set_value,
  array_build_signature,
  NULL,
  NULL
};

static const TestTypeNodeClass array_1_class = {
  DBUS_TYPE_ARRAY,
  sizeof (TestTypeNodeContainer),
  1, /* number of array elements */
  NULL,
  container_destroy,
  array_write_value,
  array_read_value,
  array_set_value,
  array_build_signature,
  NULL,
  NULL
};

static const TestTypeNodeClass array_2_class = {
  DBUS_TYPE_ARRAY,
  sizeof (TestTypeNodeContainer),
  2, /* number of array elements */
  NULL,
  container_destroy,
  array_write_value,
  array_read_value,
  array_set_value,
  array_build_signature,
  NULL,
  NULL
};

static const TestTypeNodeClass array_9_class = {
  DBUS_TYPE_ARRAY,
  sizeof (TestTypeNodeContainer),
  9, /* number of array elements */
  NULL,
  container_destroy,
  array_write_value,
  array_read_value,
  array_set_value,
  array_build_signature,
  NULL,
  NULL
};

static const TestTypeNodeClass variant_class = {
  DBUS_TYPE_VARIANT,
  sizeof (TestTypeNodeContainer),
  0,
  NULL,
  container_destroy,
  variant_write_value,
  variant_read_value,
  variant_set_value,
  NULL,
  NULL,
  NULL
};

static const TestTypeNodeClass* const
basic_nodes[] = {
  &int16_class,
  &uint16_class,
  &int32_class,
  &uint32_class,
  &int64_class,
  &uint64_class,
  &bool_class,
  &byte_class,
  &double_class,
  &string_0_class,
  &string_1_class,
  &string_3_class,
  &string_8_class,
  &object_path_class,
  &signature_class
};
#define N_BASICS (_DBUS_N_ELEMENTS (basic_nodes))

static const TestTypeNodeClass* const
container_nodes[] = {
  &struct_1_class,
  &array_1_class,
  &struct_2_class,
  &array_0_class,
  &array_2_class,
  &variant_class,
  &dict_1_class /* last since we want struct and array before it */
  /* array_9_class is omitted on purpose, it's too slow;
   * we only use it in one hardcoded test below
   */
};
#define N_CONTAINERS (_DBUS_N_ELEMENTS (container_nodes))

static TestTypeNode*
node_new (const TestTypeNodeClass *klass)
{
  TestTypeNode *node;

  node = dbus_malloc0 (klass->instance_size);
  if (node == NULL)
    return NULL;

  node->klass = klass;

  if (klass->construct)
    {
      if (!(* klass->construct) (node))
        {
          dbus_free (node);
          return NULL;
        }
    }

  return node;
}

static void
node_destroy (TestTypeNode *node)
{
  if (node->klass->destroy)
    (* node->klass->destroy) (node);
  dbus_free (node);
}

static dbus_bool_t
node_write_value (TestTypeNode   *node,
                  DataBlock      *block,
                  DBusTypeWriter *writer,
                  unsigned int    seed)
{
  dbus_bool_t retval;

  retval = (* node->klass->write_value) (node, block, writer, seed);

#if 0
  /* Handy to see where things break, but too expensive to do all the time */
  data_block_verify (block);
#endif

  return retval;
}

static dbus_bool_t
node_read_value (TestTypeNode   *node,
                 DBusTypeReader *reader,
                 unsigned int    seed)
{
  /* DBusTypeReader restored; */

  if (!(* node->klass->read_value) (node, reader, seed))
    return FALSE;

  return TRUE;
}

/* Warning: if this one fails due to OOM, it has side effects (can
 * modify only some of the sub-values). OK in a test suite, but we
 * never do this in real code.
 */
static dbus_bool_t
node_set_value (TestTypeNode   *node,
                DBusTypeReader *reader,
                DBusTypeReader *realign_root,
                unsigned int    seed)
{
  if (!(* node->klass->set_value) (node, reader, realign_root, seed))
    return FALSE;

  return TRUE;
}

static dbus_bool_t
node_build_signature (TestTypeNode *node,
                      DBusString   *str)
{
  if (node->klass->build_signature)
    return (* node->klass->build_signature) (node, str);
  else
    return _dbus_string_append_byte (str, node->klass->typecode);
}

static dbus_bool_t
node_append_child (TestTypeNode *node,
                   TestTypeNode *child)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;

  _dbus_assert (node->klass->instance_size >= (int) sizeof (TestTypeNodeContainer));

  if (!_dbus_list_append (&container->children, child))
    _dbus_test_fatal ("no memory"); /* we never check the return value on node_append_child anyhow - it's run from outside the malloc-failure test code */

  return TRUE;
}

static dbus_bool_t
node_write_multi (TestTypeNode   *node,
                  DataBlock      *block,
                  DBusTypeWriter *writer,
                  unsigned int    seed,
                  int             n_copies)
{
  dbus_bool_t retval;

  _dbus_assert (node->klass->write_multi != NULL);
  retval = (* node->klass->write_multi) (node, block, writer, seed, n_copies);

#if 0
  /* Handy to see where things break, but too expensive to do all the time */
  data_block_verify (block);
#endif

  return retval;
}

static dbus_bool_t
node_read_multi (TestTypeNode   *node,
                 DBusTypeReader *reader,
                 unsigned int    seed,
                 int             n_copies)
{
  _dbus_assert (node->klass->read_multi != NULL);

  if (!(* node->klass->read_multi) (node, reader, seed, n_copies))
    return FALSE;

  return TRUE;
}

static int n_iterations_completed_total = 0;
static int n_iterations_completed_this_test = 0;
static int n_iterations_expected_this_test = 0;

typedef struct
{
  const DBusString   *signature;
  DataBlock          *block;
  int                 type_offset;
  TestTypeNode      **nodes;
  int                 n_nodes;
} NodeIterationData;

static dbus_bool_t
run_test_copy (NodeIterationData *nid)
{
  DataBlock *src;
  DataBlock dest;
  dbus_bool_t retval;
  DBusTypeReader reader;
  DBusTypeWriter writer;

  _dbus_verbose ("\n");

  src = nid->block;

  retval = FALSE;

  if (!data_block_init (&dest, src->byte_order, src->initial_offset))
    return FALSE;

  data_block_init_reader_writer (src, &reader, NULL);
  data_block_init_reader_writer (&dest, NULL, &writer);

  /* DBusTypeWriter assumes it's writing into an existing signature,
   * so doesn't add nul on its own. We have to do that.
   */
  if (!_dbus_string_insert_byte (&dest.signature,
                                 dest.initial_offset, '\0'))
    goto out;

  if (!_dbus_type_writer_write_reader (&writer, &reader))
    goto out;

  /* Data blocks should now be identical */
  if (!_dbus_string_equal (&src->signature, &dest.signature))
    {
      _dbus_verbose ("SOURCE\n");
      _dbus_verbose_bytes_of_string (&src->signature, 0,
                                     _dbus_string_get_length (&src->signature));
      _dbus_verbose ("DEST\n");
      _dbus_verbose_bytes_of_string (&dest.signature, 0,
                                     _dbus_string_get_length (&dest.signature));
      _dbus_test_fatal ("signatures did not match");
    }

  if (!_dbus_string_equal (&src->body, &dest.body))
    {
      _dbus_verbose ("SOURCE\n");
      _dbus_verbose_bytes_of_string (&src->body, 0,
                                     _dbus_string_get_length (&src->body));
      _dbus_verbose ("DEST\n");
      _dbus_verbose_bytes_of_string (&dest.body, 0,
                                     _dbus_string_get_length (&dest.body));
      _dbus_test_fatal ("bodies did not match");
    }

  retval = TRUE;

 out:

  data_block_free (&dest);

  return retval;
}

static dbus_bool_t
run_test_values_only_write (NodeIterationData *nid)
{
  DBusTypeReader reader;
  DBusTypeWriter writer;
  int i;
  dbus_bool_t retval;
  int sig_len;

  _dbus_verbose ("\n");

  retval = FALSE;

  data_block_reset (nid->block);

  sig_len = _dbus_string_get_length (nid->signature);

  _dbus_type_writer_init_values_only (&writer,
                                      nid->block->byte_order,
                                      nid->signature, 0,
                                      &nid->block->body,
                                      _dbus_string_get_length (&nid->block->body) - N_FENCE_BYTES);
  _dbus_type_reader_init (&reader,
                          nid->block->byte_order,
                          nid->signature, 0,
                          &nid->block->body,
                          nid->block->initial_offset);

  i = 0;
  while (i < nid->n_nodes)
    {
      if (!node_write_value (nid->nodes[i], nid->block, &writer, i))
        goto out;

      ++i;
    }

  /* if we wrote any typecodes then this would fail */
  _dbus_assert (sig_len == _dbus_string_get_length (nid->signature));

  /* But be sure we wrote out the values correctly */
  i = 0;
  while (i < nid->n_nodes)
    {
      if (!node_read_value (nid->nodes[i], &reader, i))
        goto out;

      if (i + 1 == nid->n_nodes)
        NEXT_EXPECTING_FALSE (&reader);
      else
        NEXT_EXPECTING_TRUE (&reader);

      ++i;
    }

  retval = TRUE;

 out:
  data_block_reset (nid->block);
  return retval;
}

/* offset the seed for setting, so we set different numbers than
 * we originally wrote. Don't offset by a huge number since in
 * some cases it's value = possibilities[seed % n_possibilities]
 * and we don't want to wrap around. bool_from_seed
 * is just seed % 2 even.
 */
#define SET_SEED 1
static dbus_bool_t
run_test_set_values (NodeIterationData *nid)
{
  DBusTypeReader reader;
  DBusTypeReader realign_root;
  dbus_bool_t retval;
  int i;

  _dbus_verbose ("\n");

  retval = FALSE;

  data_block_init_reader_writer (nid->block,
                                 &reader, NULL);

  realign_root = reader;

  i = 0;
  while (i < nid->n_nodes)
    {
      if (!node_set_value (nid->nodes[i],
                           &reader, &realign_root,
                           i + SET_SEED))
        goto out;

      if (i + 1 == nid->n_nodes)
        NEXT_EXPECTING_FALSE (&reader);
      else
        NEXT_EXPECTING_TRUE (&reader);

      ++i;
    }

  /* Check that the new values were set */

  reader = realign_root;

  i = 0;
  while (i < nid->n_nodes)
    {
      if (!node_read_value (nid->nodes[i], &reader,
                            i + SET_SEED))
        goto out;

      if (i + 1 == nid->n_nodes)
        NEXT_EXPECTING_FALSE (&reader);
      else
        NEXT_EXPECTING_TRUE (&reader);

      ++i;
    }

  retval = TRUE;

 out:
  return retval;
}

static dbus_bool_t
run_test_delete_values (NodeIterationData *nid)
{
  DBusTypeReader reader;
  dbus_bool_t retval;
  int t;

  _dbus_verbose ("\n");

  retval = FALSE;

  data_block_init_reader_writer (nid->block,
                                 &reader, NULL);

  while ((t = _dbus_type_reader_get_current_type (&reader)) != DBUS_TYPE_INVALID)
    {
      /* Right now, deleting only works on array elements.  We delete
       * all array elements, and then verify that there aren't any
       * left.
       */
      if (t == DBUS_TYPE_ARRAY)
        {
          DBusTypeReader array;
          int n_elements;
          int elem_type;

          _dbus_type_reader_recurse (&reader, &array);
          n_elements = 0;
          while (_dbus_type_reader_get_current_type (&array) != DBUS_TYPE_INVALID)
            {
              n_elements += 1;
              _dbus_type_reader_next (&array);
            }

          /* reset to start of array */
          _dbus_type_reader_recurse (&reader, &array);
          _dbus_verbose ("recursing into deletion loop reader.value_pos = %d array.value_pos = %d array.u.start_pos = %d\n",
                         reader.value_pos, array.value_pos, array.u.array.start_pos);
          while ((elem_type = _dbus_type_reader_get_current_type (&array)) != DBUS_TYPE_INVALID)
            {
              /* We don't want to always delete from the same part of the array. */
              static int cycle = 0;
              int elem;

              _dbus_assert (n_elements > 0);

              elem = cycle;
              if (elem == 3 || elem >= n_elements) /* end of array */
                elem = n_elements - 1;

              _dbus_verbose ("deleting array element %d of %d type %s cycle %d reader pos %d elem pos %d\n",
                             elem, n_elements, _dbus_type_to_string (elem_type),
                             cycle, reader.value_pos, array.value_pos);
              while (elem > 0)
                {
                  if (!_dbus_type_reader_next (&array))
                    _dbus_test_fatal ("should have had another element");
                  --elem;
                }

              if (!_dbus_type_reader_delete (&array, &reader))
                goto out;

              n_elements -= 1;

              /* reset */
              _dbus_type_reader_recurse (&reader, &array);

              if (cycle > 2)
                cycle = 0;
              else
                cycle += 1;
            }
        }
      _dbus_type_reader_next (&reader);
    }

  /* Check that there are no array elements left */
  data_block_init_reader_writer (nid->block,
                                 &reader, NULL);

  while ((t = _dbus_type_reader_get_current_type (&reader)) != DBUS_TYPE_INVALID)
    {
      _dbus_type_reader_next (&reader);
    }

  retval = TRUE;

 out:
  return retval;
}

static dbus_bool_t
run_test_nodes_iteration (void        *data,
                          dbus_bool_t  have_memory)
{
  NodeIterationData *nid = data;
  DBusTypeReader reader;
  DBusTypeWriter writer;
  int i;
  dbus_bool_t retval;

  /* Stuff to do:
   * 1. write the value
   * 2. strcmp-compare with the signature we built
   * 3. read the value
   * 4. type-iterate the signature and the value and see if they are the same type-wise
   */
  retval = FALSE;

  data_block_init_reader_writer (nid->block,
                                 &reader, &writer);

  /* DBusTypeWriter assumes it's writing into an existing signature,
   * so doesn't add nul on its own. We have to do that.
   */
  if (!_dbus_string_insert_byte (&nid->block->signature,
                                 nid->type_offset, '\0'))
    goto out;

  i = 0;
  while (i < nid->n_nodes)
    {
      if (!node_write_value (nid->nodes[i], nid->block, &writer, i))
        goto out;

      ++i;
    }

  if (!_dbus_string_equal_substring (nid->signature, 0, _dbus_string_get_length (nid->signature),
                                     &nid->block->signature, nid->type_offset))
    {
      _dbus_test_fatal ("Expected signature '%s' and got '%s' with initial offset %d",
                  _dbus_string_get_const_data (nid->signature),
                  _dbus_string_get_const_data_len (&nid->block->signature, nid->type_offset, 0),
                  nid->type_offset);
    }

  i = 0;
  while (i < nid->n_nodes)
    {
      if (!node_read_value (nid->nodes[i], &reader, i))
        goto out;

      if (i + 1 == nid->n_nodes)
        NEXT_EXPECTING_FALSE (&reader);
      else
        NEXT_EXPECTING_TRUE (&reader);

      ++i;
    }

  if (n_iterations_expected_this_test <= MAX_ITERATIONS_FOR_EXPENSIVE_TESTS)
    {
      /* this set values test uses code from copy and
       * values_only_write so would ideally be last so you get a
       * simpler test case for problems with copying or values_only
       * writing; but it also needs an already-written DataBlock so it
       * has to go first. Comment it out if it breaks, and see if the
       * later tests also break - debug them first if so.
       */
      if (!run_test_set_values (nid))
        goto out;

      if (!run_test_delete_values (nid))
        goto out;

      if (!run_test_copy (nid))
        goto out;

      if (!run_test_values_only_write (nid))
        goto out;
    }

  /* FIXME type-iterate both signature and value and compare the resulting
   * tree to the node tree perhaps
   */

  retval = TRUE;

 out:

  data_block_reset (nid->block);

  return retval;
}

static void
run_test_nodes_in_one_configuration (TestTypeNode    **nodes,
                                     int               n_nodes,
                                     const DBusString *signature,
                                     int               byte_order,
                                     int               initial_offset)
{
  DataBlock block;
  NodeIterationData nid;

  if (!data_block_init (&block, byte_order, initial_offset))
    _dbus_test_fatal ("no memory");

  nid.signature = signature;
  nid.block = &block;
  nid.type_offset = initial_offset;
  nid.nodes = nodes;
  nid.n_nodes = n_nodes;

  if (TEST_OOM_HANDLING &&
      n_iterations_expected_this_test <= MAX_ITERATIONS_FOR_EXPENSIVE_TESTS)
    {
      _dbus_test_oom_handling ("running test node",
                               run_test_nodes_iteration,
                               &nid);
    }
  else
    {
      if (!run_test_nodes_iteration (&nid, TRUE))
        _dbus_test_fatal ("no memory");
    }

  data_block_free (&block);
}

static void
run_test_nodes (TestTypeNode **nodes,
                int            n_nodes)
{
  int i;
  DBusString signature;

  if (!_dbus_string_init (&signature))
    _dbus_test_fatal ("no memory");

  i = 0;
  while (i < n_nodes)
    {
      if (! node_build_signature (nodes[i], &signature))
        _dbus_test_fatal ("no memory");

      ++i;
    }

  _dbus_verbose (">>> test nodes with signature '%s'\n",
                 _dbus_string_get_const_data (&signature));

  i = 0;
  while (i <= MAX_INITIAL_OFFSET)
    {
      run_test_nodes_in_one_configuration (nodes, n_nodes, &signature,
                                           DBUS_LITTLE_ENDIAN, i);
      run_test_nodes_in_one_configuration (nodes, n_nodes, &signature,
                                           DBUS_BIG_ENDIAN, i);

      ++i;
    }

  n_iterations_completed_this_test += 1;
  n_iterations_completed_total += 1;

  if (n_iterations_completed_this_test == n_iterations_expected_this_test)
    {
      fprintf (stderr, " 100%% %d this test (%d cumulative)\n",
               n_iterations_completed_this_test,
               n_iterations_completed_total);
    }
  /* this happens to turn out well with mod == 1 */
  else if ((n_iterations_completed_this_test %
            (int)(n_iterations_expected_this_test / 10.0)) == 1)
    {
      fprintf (stderr, " %d%% ", (int) (n_iterations_completed_this_test / (double) n_iterations_expected_this_test * 100));
    }

  _dbus_string_free (&signature);
}

#define N_VALUES (N_BASICS * N_CONTAINERS + N_BASICS)

static TestTypeNode*
value_generator (int *ip)
{
  int i = *ip;
  const TestTypeNodeClass *child_klass;
  const TestTypeNodeClass *container_klass;
  TestTypeNode *child;
  TestTypeNode *node;

  _dbus_assert (i <= N_VALUES);

  if (i == N_VALUES)
    {
      return NULL;
    }
  else if (i < N_BASICS)
    {
      node = node_new (basic_nodes[i]);
    }
  else
    {
      /* imagine an array:
       * container 0 of basic 0
       * container 0 of basic 1
       * container 0 of basic 2
       * container 1 of basic 0
       * container 1 of basic 1
       * container 1 of basic 2
       */
      i -= N_BASICS;

      container_klass = container_nodes[i / N_BASICS];
      child_klass = basic_nodes[i % N_BASICS];

      node = node_new (container_klass);
      child = node_new (child_klass);

      node_append_child (node, child);
    }

  *ip += 1; /* increment the generator */

  return node;
}

static void
build_body (TestTypeNode **nodes,
            int            n_nodes,
            int            byte_order,
            DBusString    *signature,
            DBusString    *body)
{
  int i;
  DataBlock block;
  DBusTypeReader reader;
  DBusTypeWriter writer;

  i = 0;
  while (i < n_nodes)
    {
      if (! node_build_signature (nodes[i], signature))
        _dbus_test_fatal ("no memory");

      ++i;
    }

  if (!data_block_init (&block, byte_order, 0))
    _dbus_test_fatal ("no memory");

  data_block_init_reader_writer (&block,
                                 &reader, &writer);

  /* DBusTypeWriter assumes it's writing into an existing signature,
   * so doesn't add nul on its own. We have to do that.
   */
  if (!_dbus_string_insert_byte (&block.signature,
                                 0, '\0'))
    _dbus_test_fatal ("no memory");

  i = 0;
  while (i < n_nodes)
    {
      if (!node_write_value (nodes[i], &block, &writer, i))
        _dbus_test_fatal ("no memory");

      ++i;
    }

  if (!_dbus_string_copy_len (&block.body, 0,
                              _dbus_string_get_length (&block.body) - N_FENCE_BYTES,
                              body, 0))
    _dbus_test_fatal ("oom");

  data_block_free (&block);
}

dbus_bool_t
_dbus_test_generate_bodies (int           sequence,
                            int           byte_order,
                            DBusString   *signature,
                            DBusString   *body)
{
  TestTypeNode *nodes[1];
  int i;
  int n_nodes;

  nodes[0] = value_generator (&sequence);

  if (nodes[0] == NULL)
    return FALSE;

  n_nodes = 1;

  build_body (nodes, n_nodes, byte_order, signature, body);


  i = 0;
  while (i < n_nodes)
    {
      node_destroy (nodes[i]);
      ++i;
    }

  return TRUE;
}

static void
make_and_run_values_inside_container (const TestTypeNodeClass *container_klass,
                                      int                      n_nested)
{
  TestTypeNode *root;
  TestTypeNode *container;
  TestTypeNode *child;
  int i;

  root = node_new (container_klass);
  container = root;
  for (i = 1; i < n_nested; i++)
    {
      child = node_new (container_klass);
      node_append_child (container, child);
      container = child;
    }

  /* container should now be the most-nested container */

  i = 0;
  while ((child = value_generator (&i)))
    {
      node_append_child (container, child);

      run_test_nodes (&root, 1);

      _dbus_list_clear (&((TestTypeNodeContainer*)container)->children);
      node_destroy (child);
    }

  node_destroy (root);
}

static void
start_next_test (const char *description,
                 int         expected)
{
  n_iterations_completed_this_test = 0;
  n_iterations_expected_this_test = expected;

  fprintf (stderr, ">>> >>> %s %d iterations\n",
           description,
           n_iterations_expected_this_test);
}

static void
make_and_run_test_nodes (void)
{
  int i, j, k, m;

  /* We try to do this in order of "complicatedness" so that test
   * failures tend to show up in the simplest test case that
   * demonstrates the failure.  There are also some tests that run
   * more than once for this reason, first while going through simple
   * cases, second while going through a broader range of complex
   * cases.
   */
  /* Each basic node. The basic nodes should include:
   *
   * - each fixed-size type (in such a way that it has different values each time,
   *                         so we can tell if we mix two of them up)
   * - strings of various lengths
   * - object path
   * - signature
   */
  /* Each container node. The container nodes should include:
   *
   *  struct with 1 and 2 copies of the contained item
   *  array with 0, 1, 2 copies of the contained item
   *  variant
   */
  /*  Let a "value" be a basic node, or a container containing a single basic node.
   *  Let n_values be the number of such values i.e. (n_container * n_basic + n_basic)
   *  When iterating through all values to make combinations, do the basic types
   *  first and the containers second.
   */
  /* Each item is shown with its number of iterations to complete so
   * we can keep a handle on this unit test
   */

  /* FIXME test just an empty body, no types at all */

  start_next_test ("Each value by itself", N_VALUES);
  {
    TestTypeNode *node;
    i = 0;
    while ((node = value_generator (&i)))
      {
        run_test_nodes (&node, 1);

        node_destroy (node);
      }
  }

  start_next_test ("Each value by itself with arrays as blocks", N_VALUES);
  arrays_write_fixed_in_blocks = TRUE;
  {
    TestTypeNode *node;
    i = 0;
    while ((node = value_generator (&i)))
      {
        run_test_nodes (&node, 1);

        node_destroy (node);
      }
  }
  arrays_write_fixed_in_blocks = FALSE;

  start_next_test ("All values in one big toplevel", 1);
  {
    TestTypeNode *nodes[N_VALUES];
    TestTypeNode *node;

    i = 0;
    while ((node = value_generator (&i)))
      {
        nodes[i - 1] = node;
      }

    run_test_nodes (nodes, N_VALUES);

    for (i = 0; i < N_VALUES; i++)
      node_destroy (nodes[i]);
  }

  start_next_test ("Each value,value pair combination as toplevel, in both orders",
                   N_VALUES * N_VALUES);
  {
    TestTypeNode *nodes[2];

    i = 0;
    while ((nodes[0] = value_generator (&i)))
      {
        j = 0;
        while ((nodes[1] = value_generator (&j)))
          {
            run_test_nodes (nodes, 2);

            node_destroy (nodes[1]);
          }

        node_destroy (nodes[0]);
      }
  }

  start_next_test ("Each container containing each value",
                   N_CONTAINERS * N_VALUES);
  for (i = 0; i < N_CONTAINERS; i++)
    {
      const TestTypeNodeClass *container_klass = container_nodes[i];

      make_and_run_values_inside_container (container_klass, 1);
    }

  start_next_test ("Each container containing each value with arrays as blocks",
                   N_CONTAINERS * N_VALUES);
  arrays_write_fixed_in_blocks = TRUE;
  for (i = 0; i < N_CONTAINERS; i++)
    {
      const TestTypeNodeClass *container_klass = container_nodes[i];

      make_and_run_values_inside_container (container_klass, 1);
    }
  arrays_write_fixed_in_blocks = FALSE;

  start_next_test ("Each container of same container of each value",
                   N_CONTAINERS * N_VALUES);
  for (i = 0; i < N_CONTAINERS; i++)
    {
      const TestTypeNodeClass *container_klass = container_nodes[i];

      make_and_run_values_inside_container (container_klass, 2);
    }

  start_next_test ("Each container of same container of same container of each value",
                   N_CONTAINERS * N_VALUES);
  for (i = 0; i < N_CONTAINERS; i++)
    {
      const TestTypeNodeClass *container_klass = container_nodes[i];

      make_and_run_values_inside_container (container_klass, 3);
    }

  start_next_test ("Each value,value pair inside a struct",
                   N_VALUES * N_VALUES);
  {
    TestTypeNode *val1, *val2;
    TestTypeNode *node;

    node = node_new (&struct_1_class);

    i = 0;
    while ((val1 = value_generator (&i)))
      {
        j = 0;
        while ((val2 = value_generator (&j)))
          {
            TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;

            node_append_child (node, val1);
            node_append_child (node, val2);

            run_test_nodes (&node, 1);

            _dbus_list_clear (&container->children);
            node_destroy (val2);
          }
        node_destroy (val1);
      }
    node_destroy (node);
  }

  start_next_test ("All values in one big struct", 1);
  {
    TestTypeNode *node;
    TestTypeNode *child;

    node = node_new (&struct_1_class);

    i = 0;
    while ((child = value_generator (&i)))
      node_append_child (node, child);

    run_test_nodes (&node, 1);

    node_destroy (node);
  }

  start_next_test ("Each value in a large array", N_VALUES);
  {
    TestTypeNode *val;
    TestTypeNode *node;

    node = node_new (&array_9_class);

    i = 0;
    while ((val = value_generator (&i)))
      {
        TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;

        node_append_child (node, val);

        run_test_nodes (&node, 1);

        _dbus_list_clear (&container->children);
        node_destroy (val);
      }

    node_destroy (node);
  }

  if (_dbus_getenv ("DBUS_TEST_SLOW") == NULL ||
      atoi (_dbus_getenv ("DBUS_TEST_SLOW")) < 1)
    {
      fprintf (stderr, "skipping remaining marshal-recursive tests, "
          "run with DBUS_TEST_SLOW=1 (or more) to enable\n");
      goto out;
    }

  start_next_test ("Each container of each container of each value",
                   N_CONTAINERS * N_CONTAINERS * N_VALUES);
  for (i = 0; i < N_CONTAINERS; i++)
    {
      const TestTypeNodeClass *outer_container_klass = container_nodes[i];
      TestTypeNode *outer_container = node_new (outer_container_klass);

      for (j = 0; j < N_CONTAINERS; j++)
        {
          TestTypeNode *child;
          const TestTypeNodeClass *inner_container_klass = container_nodes[j];
          TestTypeNode *inner_container = node_new (inner_container_klass);

          node_append_child (outer_container, inner_container);

          m = 0;
          while ((child = value_generator (&m)))
            {
              node_append_child (inner_container, child);

              run_test_nodes (&outer_container, 1);

              _dbus_list_clear (&((TestTypeNodeContainer*)inner_container)->children);
              node_destroy (child);
            }
          _dbus_list_clear (&((TestTypeNodeContainer*)outer_container)->children);
          node_destroy (inner_container);
        }
      node_destroy (outer_container);
    }

  start_next_test ("Each container of each container of each container of each value",
                   N_CONTAINERS * N_CONTAINERS * N_CONTAINERS * N_VALUES);
  for (i = 0; i < N_CONTAINERS; i++)
    {
      const TestTypeNodeClass *outer_container_klass = container_nodes[i];
      TestTypeNode *outer_container = node_new (outer_container_klass);

      for (j = 0; j < N_CONTAINERS; j++)
        {
          const TestTypeNodeClass *inner_container_klass = container_nodes[j];
          TestTypeNode *inner_container = node_new (inner_container_klass);

          node_append_child (outer_container, inner_container);

          for (k = 0; k < N_CONTAINERS; k++)
            {
              TestTypeNode *child;
              const TestTypeNodeClass *center_container_klass = container_nodes[k];
              TestTypeNode *center_container = node_new (center_container_klass);

              node_append_child (inner_container, center_container);

              m = 0;
              while ((child = value_generator (&m)))
                {
                  node_append_child (center_container, child);

                  run_test_nodes (&outer_container, 1);

                  _dbus_list_clear (&((TestTypeNodeContainer*)center_container)->children);
                  node_destroy (child);
                }
              _dbus_list_clear (&((TestTypeNodeContainer*)inner_container)->children);
              node_destroy (center_container);
            }
          _dbus_list_clear (&((TestTypeNodeContainer*)outer_container)->children);
          node_destroy (inner_container);
        }
      node_destroy (outer_container);
    }

  /* This one takes a really long time (10 minutes on a Core2), so only enable
   * it if you're really sure */
  if (atoi (_dbus_getenv ("DBUS_TEST_SLOW")) < 2)
    {
      fprintf (stderr, "skipping really slow marshal-recursive test, "
          "run with DBUS_TEST_SLOW=2 (or more) to enable\n");
      goto out;
    }

  start_next_test ("Each value,value,value triplet combination as toplevel, in all orders",
                   N_VALUES * N_VALUES * N_VALUES);
  {
    TestTypeNode *nodes[3];

    i = 0;
    while ((nodes[0] = value_generator (&i)))
      {
        j = 0;
        while ((nodes[1] = value_generator (&j)))
          {
            k = 0;
            while ((nodes[2] = value_generator (&k)))
              {
                run_test_nodes (nodes, 3);

                node_destroy (nodes[2]);
              }
            node_destroy (nodes[1]);
          }
        node_destroy (nodes[0]);
      }
  }

out:
  fprintf (stderr, "%d total iterations of recursive marshaling tests\n",
           n_iterations_completed_total);
  fprintf (stderr, "each iteration ran at initial offsets 0 through %d in both big and little endian\n",
           MAX_INITIAL_OFFSET);
  fprintf (stderr, "out of memory handling %s tested\n",
           TEST_OOM_HANDLING ? "was" : "was not");
}

dbus_bool_t
_dbus_marshal_recursive_test (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  make_and_run_test_nodes ();

  return TRUE;
}

/*
 *
 *
 *         Implementations of each type node class
 *
 *
 *
 */
#define MAX_MULTI_COUNT 5

#define SAMPLE_INT16           1234
#define SAMPLE_INT16_ALTERNATE 6785
static dbus_uint16_t
uint16_from_seed (unsigned int seed)
{
  /* Generate an integer value that's predictable from seed.  We could
   * just use seed itself, but that would only ever touch one byte of
   * the int so would miss some kinds of bug.
   */
  static const dbus_uint16_t v_of_seed[5] = {
    SAMPLE_INT16,
    SAMPLE_INT16_ALTERNATE,
    _DBUS_UINT16_MAX,
    _DBUS_INT16_MAX,
    1
  };

  dbus_uint16_t v = v_of_seed[seed % _DBUS_N_ELEMENTS(v_of_seed)];

  if (seed > 1)
    v *= seed; /* wraps around eventually, which is fine */

  return v;
}

static dbus_bool_t
uint16_write_value (TestTypeNode   *node,
                    DataBlock      *block,
                    DBusTypeWriter *writer,
                    unsigned int    seed)
{
  /* also used for int16 */
  dbus_uint16_t v;

  v = uint16_from_seed (seed);

  return _dbus_type_writer_write_basic (writer,
                                        node->klass->typecode,
                                        &v);
}

static dbus_bool_t
uint16_read_value (TestTypeNode   *node,
                   DBusTypeReader *reader,
                   unsigned int    seed)
{
  /* also used for int16 */
  dbus_uint16_t v;

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_basic (reader, &v);

  _dbus_assert (v == uint16_from_seed (seed));

  return TRUE;
}

static dbus_bool_t
uint16_set_value (TestTypeNode   *node,
                  DBusTypeReader *reader,
                  DBusTypeReader *realign_root,
                  unsigned int    seed)
{
  /* also used for int16 */
  dbus_uint16_t v;

  v = uint16_from_seed (seed);

  return _dbus_type_reader_set_basic (reader,
                                      &v,
                                      realign_root);
}

static dbus_bool_t
uint16_write_multi (TestTypeNode   *node,
                    DataBlock      *block,
                    DBusTypeWriter *writer,
                    unsigned int    seed,
                    int             count)
{
  /* also used for int16 */
  dbus_uint16_t values[MAX_MULTI_COUNT];
  dbus_uint16_t *v_ARRAY_UINT16 = values;
  int i;

  _dbus_assert (count <= MAX_MULTI_COUNT);

  for (i = 0; i < count; ++i)
    values[i] = uint16_from_seed (seed + i);

  return _dbus_type_writer_write_fixed_multi (writer,
                                              node->klass->typecode,
                                              &v_ARRAY_UINT16, count);
}

static dbus_bool_t
uint16_read_multi (TestTypeNode   *node,
                   DBusTypeReader *reader,
                   unsigned int    seed,
                   int             count)
{
  /* also used for int16 */
  dbus_uint16_t *values;
  int n_elements;
  int i;

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_fixed_multi (reader,
                                      (const void **) &values,
                                      &n_elements);

  if (n_elements != count)
    _dbus_warn ("got %d elements expected %d", n_elements, count);
  _dbus_assert (n_elements == count);

  for (i = 0; i < count; i++)
    _dbus_assert ((_dbus_unpack_uint16 (reader->byte_order,
                                        (const unsigned char*)values + (i * 2))) ==
                  uint16_from_seed (seed + i));

  return TRUE;
}


#define SAMPLE_INT32           12345678
#define SAMPLE_INT32_ALTERNATE 53781429
static dbus_uint32_t
uint32_from_seed (unsigned int seed)
{
  /* Generate an integer value that's predictable from seed.  We could
   * just use seed itself, but that would only ever touch one byte of
   * the int so would miss some kinds of bug.
   */
  static const dbus_uint32_t v_of_seed[5] = {
    SAMPLE_INT32,
    SAMPLE_INT32_ALTERNATE,
    _DBUS_UINT32_MAX,
    _DBUS_INT32_MAX,
    1
  };

  dbus_uint32_t v = v_of_seed[seed % _DBUS_N_ELEMENTS(v_of_seed)];

  if (seed > 1)
    v *= seed; /* wraps around eventually, which is fine */

  return v;
}

static dbus_bool_t
uint32_write_value (TestTypeNode   *node,
                    DataBlock      *block,
                    DBusTypeWriter *writer,
                    unsigned int    seed)
{
  /* also used for int32 */
  dbus_uint32_t v;

  v = uint32_from_seed (seed);

  return _dbus_type_writer_write_basic (writer,
                                        node->klass->typecode,
                                        &v);
}

static dbus_bool_t
uint32_read_value (TestTypeNode   *node,
                   DBusTypeReader *reader,
                   unsigned int    seed)
{
  /* also used for int32 */
  dbus_uint32_t v;

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_basic (reader, &v);

  _dbus_assert (v == uint32_from_seed (seed));

  return TRUE;
}

static dbus_bool_t
uint32_set_value (TestTypeNode   *node,
                  DBusTypeReader *reader,
                  DBusTypeReader *realign_root,
                  unsigned int    seed)
{
  /* also used for int32 */
  dbus_uint32_t v;

  v = uint32_from_seed (seed);

  return _dbus_type_reader_set_basic (reader,
                                      &v,
                                      realign_root);
}

static dbus_bool_t
uint32_write_multi (TestTypeNode   *node,
                    DataBlock      *block,
                    DBusTypeWriter *writer,
                    unsigned int    seed,
                    int             count)
{
  /* also used for int32 */
  dbus_uint32_t values[MAX_MULTI_COUNT];
  dbus_uint32_t *v_ARRAY_UINT32 = values;
  int i;

  _dbus_assert (count <= MAX_MULTI_COUNT);

  for (i = 0; i < count; ++i)
    values[i] = uint32_from_seed (seed + i);

  return _dbus_type_writer_write_fixed_multi (writer,
                                              node->klass->typecode,
                                              &v_ARRAY_UINT32, count);
}

static dbus_bool_t
uint32_read_multi (TestTypeNode   *node,
                   DBusTypeReader *reader,
                   unsigned int    seed,
                   int             count)
{
  /* also used for int32 */
  dbus_uint32_t *values;
  int n_elements;
  int i;

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_fixed_multi (reader,
                                      (const void **) &values,
                                      &n_elements);

  if (n_elements != count)
    _dbus_warn ("got %d elements expected %d", n_elements, count);
  _dbus_assert (n_elements == count);

  for (i = 0; i < count; i++)
    _dbus_assert ((_dbus_unpack_uint32 (reader->byte_order,
                                        (const unsigned char*)values + (i * 4))) ==
                  uint32_from_seed (seed + i));

  return TRUE;
}

static dbus_uint64_t
uint64_from_seed (unsigned int seed)
{
  dbus_uint32_t v32;
  dbus_uint64_t v;

  v32 = uint32_from_seed (seed);

  /* In the original implementation, everything was signed, and v was
   * set to - ((dbus_int32_t) ~ v32), but in two's complement that's just
   * v32 + 1 with wraparound. */
  v = (v32 + 1) & 0xFFFFFFFF;

  v |= (((dbus_uint64_t) v32) << 32);

  return v;
}

static dbus_bool_t
uint64_write_value (TestTypeNode   *node,
                    DataBlock      *block,
                    DBusTypeWriter *writer,
                    unsigned int    seed)
{
  /* also used for int64 */
  dbus_uint64_t v;

  v = uint64_from_seed (seed);

  return _dbus_type_writer_write_basic (writer,
                                        node->klass->typecode,
                                        &v);
}

static dbus_bool_t
uint64_read_value (TestTypeNode   *node,
                   DBusTypeReader *reader,
                   unsigned int    seed)
{
  /* also used for int64 */
  dbus_uint64_t v;

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_basic (reader, &v);

  _dbus_assert (v == uint64_from_seed (seed));

  return TRUE;
}

static dbus_bool_t
uint64_set_value (TestTypeNode   *node,
                  DBusTypeReader *reader,
                  DBusTypeReader *realign_root,
                  unsigned int    seed)
{
  /* also used for int64 */
  dbus_uint64_t v;

  v = uint64_from_seed (seed);

  return _dbus_type_reader_set_basic (reader,
                                      &v,
                                      realign_root);
}

#define MAX_SAMPLE_STRING_LEN 10
static void
string_from_seed (char         *buf,
                  int           len,
                  unsigned int  seed)
{
  int i;
  unsigned char v;

  /* Callers use a buffer of length MAX_SAMPLE_STRING_LEN + 1, which is
   * enough for MAX_SAMPLE_STRING_LEN bytes of actual string payload,
   * plus the NUL terminator */
  _dbus_assert (len + 2 <= MAX_SAMPLE_STRING_LEN);

  /* vary the length slightly, though we also have multiple string
   * value types for this, varying it here tests the set_value code
   */
  switch (seed % 3)
    {
    default:
      /* don't alter it */
      break;
    case 1:
      len += 2;
      break;
    case 2:
      len -= 2;
      break;
    }
  if (len < 0)
    len = 0;

  v = (unsigned char) ('A' + seed);

  i = 0;
  while (i < len)
    {
      if (v < 'A' || v > 'z')
        v = 'A';

      buf[i] = v;

      v += 1;
      ++i;
    }

  buf[i] = '\0';
}

static dbus_bool_t
string_write_value (TestTypeNode   *node,
                    DataBlock      *block,
                    DBusTypeWriter *writer,
                    unsigned int    seed)
{
  char buf[MAX_SAMPLE_STRING_LEN + 1]="";
  const char *v_string = buf;


  string_from_seed (buf, node->klass->subclass_detail,
                    seed);

  return _dbus_type_writer_write_basic (writer,
                                        node->klass->typecode,
                                        &v_string);
}

static dbus_bool_t
string_read_value (TestTypeNode   *node,
                   DBusTypeReader *reader,
                   unsigned int    seed)
{
  const char *v;
  char buf[MAX_SAMPLE_STRING_LEN + 1];
  v = buf;

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_basic (reader,
                                (const char **) &v);

  string_from_seed (buf, node->klass->subclass_detail,
                    seed);

  if (strcmp (buf, v) != 0)
    _dbus_test_fatal ("read string '%s' expected '%s'", v, buf);

  return TRUE;
}

static dbus_bool_t
string_set_value (TestTypeNode   *node,
                  DBusTypeReader *reader,
                  DBusTypeReader *realign_root,
                  unsigned int    seed)
{
  char buf[MAX_SAMPLE_STRING_LEN + 1];
  const char *v_string = buf;

  string_from_seed (buf, node->klass->subclass_detail,
                    seed);

#if RECURSIVE_MARSHAL_WRITE_TRACE
 {
   const char *old;
   _dbus_type_reader_read_basic (reader, &old);
   _dbus_verbose ("SETTING new string '%s' len %d in place of '%s' len %d\n",
                  v_string, strlen (v_string), old, strlen (old));
 }
#endif

  return _dbus_type_reader_set_basic (reader,
                                      &v_string,
                                      realign_root);
}

#define BOOL_FROM_SEED(seed) ((dbus_bool_t)((seed) % 2))

static dbus_bool_t
bool_write_value (TestTypeNode   *node,
                  DataBlock      *block,
                  DBusTypeWriter *writer,
                  unsigned int    seed)
{
  dbus_bool_t v;

  v = BOOL_FROM_SEED (seed);

  return _dbus_type_writer_write_basic (writer,
                                        node->klass->typecode,
                                        &v);
}

static dbus_bool_t
bool_read_value (TestTypeNode   *node,
                 DBusTypeReader *reader,
                 unsigned int    seed)
{
  dbus_bool_t v;

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_basic (reader,
                                (unsigned char*) &v);

  _dbus_assert (v == BOOL_FROM_SEED (seed));

  return TRUE;
}

static dbus_bool_t
bool_set_value (TestTypeNode   *node,
                DBusTypeReader *reader,
                DBusTypeReader *realign_root,
                unsigned int    seed)
{
  dbus_bool_t v;

  v = BOOL_FROM_SEED (seed);

  return _dbus_type_reader_set_basic (reader,
                                      &v,
                                      realign_root);
}

#define BYTE_FROM_SEED(seed) ((unsigned char) uint32_from_seed (seed))

static dbus_bool_t
byte_write_value (TestTypeNode   *node,
                  DataBlock      *block,
                  DBusTypeWriter *writer,
                  unsigned int    seed)
{
  unsigned char v;

  v = BYTE_FROM_SEED (seed);

  return _dbus_type_writer_write_basic (writer,
                                        node->klass->typecode,
                                        &v);
}

static dbus_bool_t
byte_read_value (TestTypeNode   *node,
                 DBusTypeReader *reader,
                 unsigned int    seed)
{
  unsigned char v;

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_basic (reader,
                                (unsigned char*) &v);

  _dbus_assert (v == BYTE_FROM_SEED (seed));

  return TRUE;
}


static dbus_bool_t
byte_set_value (TestTypeNode   *node,
                DBusTypeReader *reader,
                DBusTypeReader *realign_root,
                unsigned int    seed)
{
  unsigned char v;

  v = BYTE_FROM_SEED (seed);

  return _dbus_type_reader_set_basic (reader,
                                      &v,
                                      realign_root);
}

static double
double_from_seed (unsigned int seed)
{
  return SAMPLE_INT32 * ((double) (int) seed) + 0.3;
}

static dbus_bool_t
double_write_value (TestTypeNode   *node,
                    DataBlock      *block,
                    DBusTypeWriter *writer,
                    unsigned int    seed)
{
  double v;

  v = double_from_seed (seed);

  return _dbus_type_writer_write_basic (writer,
                                        node->klass->typecode,
                                        &v);
}

static dbus_bool_t
double_read_value (TestTypeNode   *node,
                   DBusTypeReader *reader,
                   unsigned int    seed)
{
  double v;
  double expected;

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_basic (reader, &v);

  expected = double_from_seed (seed);

  if (!_DBUS_DOUBLES_BITWISE_EQUAL (v, expected))
    _dbus_test_fatal ("Expected double %g got %g\n bits = 0x%" PRIx64 " vs.\n bits = 0x%" PRIx64,
                      expected, v,
                      *(dbus_uint64_t*)(char*)&expected,
                      *(dbus_uint64_t*)(char*)&v);

  return TRUE;
}

static dbus_bool_t
double_set_value (TestTypeNode   *node,
                DBusTypeReader *reader,
                DBusTypeReader *realign_root,
                unsigned int    seed)
{
  double v;

  v = double_from_seed (seed);

  return _dbus_type_reader_set_basic (reader,
                                      &v,
                                      realign_root);
}

#define MAX_SAMPLE_OBJECT_PATH_LEN 10
static void
object_path_from_seed (char          *buf,
                       unsigned int   seed)
{
  int i;
  unsigned char v;
  int len;

  len = seed % 9;
  _dbus_assert (len < MAX_SAMPLE_OBJECT_PATH_LEN);

  v = (unsigned char) ('A' + seed);

  if (len < 2)
    {
      buf[0] = '/';
      i = 1;
    }
  else
    {
      i = 0;
      while (i + 1 < len)
        {
          if (v < 'A' || v > 'z')
            v = 'A';

          buf[i] = '/';
          ++i;
          buf[i] = v;
          ++i;

          v += 1;
        }
    }

  buf[i] = '\0';
}

static dbus_bool_t
object_path_write_value (TestTypeNode   *node,
                         DataBlock      *block,
                         DBusTypeWriter *writer,
                         unsigned int    seed)
{
  char buf[MAX_SAMPLE_OBJECT_PATH_LEN + 1];
  const char *v_string = buf;

  object_path_from_seed (buf, seed);

  return _dbus_type_writer_write_basic (writer,
                                        node->klass->typecode,
                                        &v_string);
}

static dbus_bool_t
object_path_read_value (TestTypeNode   *node,
                        DBusTypeReader *reader,
                        unsigned int    seed)
{
  const char *v;
  char buf[MAX_SAMPLE_OBJECT_PATH_LEN + 1];

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_basic (reader, &v);

  object_path_from_seed (buf, seed);

  if (strcmp (buf, v) != 0)
    _dbus_test_fatal ("read object path '%s' expected '%s'", v, buf);

  return TRUE;
}

static dbus_bool_t
object_path_set_value (TestTypeNode   *node,
                       DBusTypeReader *reader,
                       DBusTypeReader *realign_root,
                       unsigned int    seed)
{
  char buf[MAX_SAMPLE_OBJECT_PATH_LEN + 1];
  const char *v_string = buf;

  object_path_from_seed (buf, seed);

  return _dbus_type_reader_set_basic (reader,
                                      &v_string,
                                      realign_root);
}

#define MAX_SAMPLE_SIGNATURE_LEN 10
static void
signature_from_seed (char         *buf,
                     unsigned int  seed)
{
  /* try to avoid ascending, descending, or alternating length to help find bugs */
  const char *sample_signatures[] = {
    "asax",
    "",
    "asau(xxxx)",
    "x",
    "ai",
    "a(ii)"
  };

  strcpy (buf, sample_signatures[seed % _DBUS_N_ELEMENTS(sample_signatures)]);
}

static dbus_bool_t
signature_write_value (TestTypeNode   *node,
                       DataBlock      *block,
                       DBusTypeWriter *writer,
                       unsigned int    seed)
{
  char buf[MAX_SAMPLE_SIGNATURE_LEN + 1];
  const char *v_string = buf;

  signature_from_seed (buf, seed);

  return _dbus_type_writer_write_basic (writer,
                                        node->klass->typecode,
                                        &v_string);
}

static dbus_bool_t
signature_read_value (TestTypeNode   *node,
                      DBusTypeReader *reader,
                      unsigned int    seed)
{
  const char *v;
  char buf[MAX_SAMPLE_SIGNATURE_LEN + 1];

  check_expected_type (reader, node->klass->typecode);

  _dbus_type_reader_read_basic (reader, &v);

  signature_from_seed (buf, seed);

  if (strcmp (buf, v) != 0)
    _dbus_test_fatal ("read signature value '%s' expected '%s'", v, buf);

  return TRUE;
}


static dbus_bool_t
signature_set_value (TestTypeNode   *node,
                     DBusTypeReader *reader,
                     DBusTypeReader *realign_root,
                     unsigned int    seed)
{
  char buf[MAX_SAMPLE_SIGNATURE_LEN + 1];
  const char *v_string = buf;

  signature_from_seed (buf, seed);

  return _dbus_type_reader_set_basic (reader,
                                      &v_string,
                                      realign_root);
}

static dbus_bool_t
struct_write_value (TestTypeNode   *node,
                    DataBlock      *block,
                    DBusTypeWriter *writer,
                    unsigned int    seed)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  DataBlockState saved;
  DBusTypeWriter sub;
  int i;
  int n_copies;

  n_copies = node->klass->subclass_detail;

  _dbus_assert (container->children != NULL);

  data_block_save (block, &saved);

  if (!_dbus_type_writer_recurse (writer, DBUS_TYPE_STRUCT,
                                  NULL, 0,
                                  &sub))
    return FALSE;

  i = 0;
  while (i < n_copies)
    {
      DBusList *link;

      link = _dbus_list_get_first_link (&container->children);
      while (link != NULL)
        {
          TestTypeNode *child = link->data;
          DBusList *next = _dbus_list_get_next_link (&container->children, link);

          if (!node_write_value (child, block, &sub, seed + i))
            {
              data_block_restore (block, &saved);
              return FALSE;
            }

          link = next;
        }

      ++i;
    }

  if (!_dbus_type_writer_unrecurse (writer, &sub))
    {
      data_block_restore (block, &saved);
      return FALSE;
    }

  return TRUE;
}

static dbus_bool_t
struct_read_or_set_value (TestTypeNode   *node,
                          DBusTypeReader *reader,
                          DBusTypeReader *realign_root,
                          unsigned int    seed)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  DBusTypeReader sub;
  int i;
  int n_copies;

  n_copies = node->klass->subclass_detail;

  check_expected_type (reader, DBUS_TYPE_STRUCT);

  _dbus_type_reader_recurse (reader, &sub);

  i = 0;
  while (i < n_copies)
    {
      DBusList *link;

      link = _dbus_list_get_first_link (&container->children);
      while (link != NULL)
        {
          TestTypeNode *child = link->data;
          DBusList *next = _dbus_list_get_next_link (&container->children, link);

          if (realign_root == NULL)
            {
              if (!node_read_value (child, &sub, seed + i))
                return FALSE;
            }
          else
            {
              if (!node_set_value (child, &sub, realign_root, seed + i))
                return FALSE;
            }

          if (i == (n_copies - 1) && next == NULL)
            NEXT_EXPECTING_FALSE (&sub);
          else
            NEXT_EXPECTING_TRUE (&sub);

          link = next;
        }

      ++i;
    }

  return TRUE;
}

static dbus_bool_t
struct_read_value (TestTypeNode   *node,
                   DBusTypeReader *reader,
                   unsigned int    seed)
{
  return struct_read_or_set_value (node, reader, NULL, seed);
}

static dbus_bool_t
struct_set_value (TestTypeNode   *node,
                  DBusTypeReader *reader,
                  DBusTypeReader *realign_root,
                  unsigned int    seed)
{
  return struct_read_or_set_value (node, reader, realign_root, seed);
}

static dbus_bool_t
struct_build_signature (TestTypeNode   *node,
                        DBusString     *str)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  int i;
  int orig_len;
  int n_copies;

  n_copies = node->klass->subclass_detail;

  orig_len = _dbus_string_get_length (str);

  if (!_dbus_string_append_byte (str, DBUS_STRUCT_BEGIN_CHAR))
    goto oom;

  i = 0;
  while (i < n_copies)
    {
      DBusList *link;

      link = _dbus_list_get_first_link (&container->children);
      while (link != NULL)
        {
          TestTypeNode *child = link->data;
          DBusList *next = _dbus_list_get_next_link (&container->children, link);

          if (!node_build_signature (child, str))
            goto oom;

          link = next;
        }

      ++i;
    }

  if (!_dbus_string_append_byte (str, DBUS_STRUCT_END_CHAR))
    goto oom;

  return TRUE;

 oom:
  _dbus_string_set_length (str, orig_len);
  return FALSE;
}

static dbus_bool_t
array_write_value (TestTypeNode   *node,
                   DataBlock      *block,
                   DBusTypeWriter *writer,
                   unsigned int    seed)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  DataBlockState saved;
  DBusTypeWriter sub;
  DBusString element_signature;
  int i;
  int n_copies;
  int element_type;
  TestTypeNode *child;

  n_copies = node->klass->subclass_detail;

  _dbus_assert (container->children != NULL);

  data_block_save (block, &saved);

  if (!_dbus_string_init (&element_signature))
    return FALSE;

  child = _dbus_list_get_first (&container->children);

  if (!node_build_signature (child,
                             &element_signature))
    goto oom;

  element_type = _dbus_first_type_in_signature (&element_signature, 0);

  if (!_dbus_type_writer_recurse (writer, DBUS_TYPE_ARRAY,
                                  &element_signature, 0,
                                  &sub))
    goto oom;

  if (arrays_write_fixed_in_blocks &&
      dbus_type_is_fixed (element_type) &&
      child->klass->write_multi)
    {
      if (!node_write_multi (child, block, &sub, seed, n_copies))
        goto oom;
    }
  else
    {
      i = 0;
      while (i < n_copies)
        {
          DBusList *link;

          link = _dbus_list_get_first_link (&container->children);
          while (link != NULL)
            {
              TestTypeNode *child2 = link->data;
              DBusList *next = _dbus_list_get_next_link (&container->children, link);

              if (!node_write_value (child2, block, &sub, seed + i))
                goto oom;

              link = next;
            }

          ++i;
        }
    }

  if (!_dbus_type_writer_unrecurse (writer, &sub))
    goto oom;

  _dbus_string_free (&element_signature);
  return TRUE;

 oom:
  data_block_restore (block, &saved);
  _dbus_string_free (&element_signature);
  return FALSE;
}

static dbus_bool_t
array_read_or_set_value (TestTypeNode   *node,
                         DBusTypeReader *reader,
                         DBusTypeReader *realign_root,
                         unsigned int    seed)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  DBusTypeReader sub;
  int i;
  int n_copies;
  TestTypeNode *child;

  n_copies = node->klass->subclass_detail;

  check_expected_type (reader, DBUS_TYPE_ARRAY);

  child = _dbus_list_get_first (&container->children);

  if (n_copies > 0)
    {
      _dbus_type_reader_recurse (reader, &sub);

      if (realign_root == NULL && arrays_write_fixed_in_blocks &&
          dbus_type_is_fixed (_dbus_type_reader_get_element_type (reader)) &&
          child->klass->read_multi)
        {
          if (!node_read_multi (child, &sub, seed, n_copies))
            return FALSE;
        }
      else
        {
          i = 0;
          while (i < n_copies)
            {
              DBusList *link;

              link = _dbus_list_get_first_link (&container->children);
              while (link != NULL)
                {
                  TestTypeNode *child2 = link->data;
                  DBusList *next = _dbus_list_get_next_link (&container->children, link);

                  _dbus_assert (child2->klass->typecode ==
                                _dbus_type_reader_get_element_type (reader));

                  if (realign_root == NULL)
                    {
                      if (!node_read_value (child2, &sub, seed + i))
                        return FALSE;
                    }
                  else
                    {
                      if (!node_set_value (child2, &sub, realign_root, seed + i))
                        return FALSE;
                    }

                  if (i == (n_copies - 1) && next == NULL)
                    NEXT_EXPECTING_FALSE (&sub);
                  else
                    NEXT_EXPECTING_TRUE (&sub);

                  link = next;
                }

              ++i;
            }
        }
    }

  return TRUE;
}

static dbus_bool_t
array_read_value (TestTypeNode   *node,
                  DBusTypeReader *reader,
                  unsigned int    seed)
{
  return array_read_or_set_value (node, reader, NULL, seed);
}

static dbus_bool_t
array_set_value (TestTypeNode   *node,
                 DBusTypeReader *reader,
                 DBusTypeReader *realign_root,
                 unsigned int    seed)
{
  return array_read_or_set_value (node, reader, realign_root, seed);
}

static dbus_bool_t
array_build_signature (TestTypeNode   *node,
                       DBusString     *str)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  int orig_len;

  orig_len = _dbus_string_get_length (str);

  if (!_dbus_string_append_byte (str, DBUS_TYPE_ARRAY))
    goto oom;

  if (!node_build_signature (_dbus_list_get_first (&container->children),
                             str))
    goto oom;

  return TRUE;

 oom:
  _dbus_string_set_length (str, orig_len);
  return FALSE;
}

 /* 10 is random just to add another seed that we use in the suite */
#define VARIANT_SEED 10

static dbus_bool_t
variant_write_value (TestTypeNode   *node,
                     DataBlock      *block,
                     DBusTypeWriter *writer,
                     unsigned int    seed)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  DataBlockState saved;
  DBusTypeWriter sub;
  DBusString content_signature;
  TestTypeNode *child;

  _dbus_assert (container->children != NULL);
  _dbus_assert (_dbus_list_length_is_one (&container->children));

  child = _dbus_list_get_first (&container->children);

  data_block_save (block, &saved);

  if (!_dbus_string_init (&content_signature))
    return FALSE;

  if (!node_build_signature (child,
                             &content_signature))
    goto oom;

  if (!_dbus_type_writer_recurse (writer, DBUS_TYPE_VARIANT,
                                  &content_signature, 0,
                                  &sub))
    goto oom;

  if (!node_write_value (child, block, &sub, seed + VARIANT_SEED))
    goto oom;

  if (!_dbus_type_writer_unrecurse (writer, &sub))
    goto oom;

  _dbus_string_free (&content_signature);
  return TRUE;

 oom:
  data_block_restore (block, &saved);
  _dbus_string_free (&content_signature);
  return FALSE;
}

static dbus_bool_t
variant_read_or_set_value (TestTypeNode   *node,
                           DBusTypeReader *reader,
                           DBusTypeReader *realign_root,
                           unsigned int    seed)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  DBusTypeReader sub;
  TestTypeNode *child;

  _dbus_assert (container->children != NULL);
  _dbus_assert (_dbus_list_length_is_one (&container->children));

  child = _dbus_list_get_first (&container->children);

  check_expected_type (reader, DBUS_TYPE_VARIANT);

  _dbus_type_reader_recurse (reader, &sub);

  if (realign_root == NULL)
    {
      if (!node_read_value (child, &sub, seed + VARIANT_SEED))
        return FALSE;
    }
  else
    {
      if (!node_set_value (child, &sub, realign_root, seed + VARIANT_SEED))
        return FALSE;
    }

  NEXT_EXPECTING_FALSE (&sub);

  return TRUE;
}

static dbus_bool_t
variant_read_value (TestTypeNode   *node,
                    DBusTypeReader *reader,
                    unsigned int    seed)
{
  return variant_read_or_set_value (node, reader, NULL, seed);
}

static dbus_bool_t
variant_set_value (TestTypeNode   *node,
                   DBusTypeReader *reader,
                   DBusTypeReader *realign_root,
                   unsigned int    seed)
{
  return variant_read_or_set_value (node, reader, realign_root, seed);
}

static dbus_bool_t
dict_write_value (TestTypeNode   *node,
                  DataBlock      *block,
                  DBusTypeWriter *writer,
                  unsigned int    seed)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  DataBlockState saved;
  DBusTypeWriter sub;
  DBusString entry_value_signature;
  DBusString dict_entry_signature;
  int i;
  int n_entries;
  TestTypeNode *child;

  n_entries = node->klass->subclass_detail;

  _dbus_assert (container->children != NULL);

  data_block_save (block, &saved);

  if (!_dbus_string_init (&entry_value_signature))
    return FALSE;

  if (!_dbus_string_init (&dict_entry_signature))
    {
      _dbus_string_free (&entry_value_signature);
      return FALSE;
    }

  child = _dbus_list_get_first (&container->children);

  if (!node_build_signature (child,
                             &entry_value_signature))
    goto oom;

  if (!_dbus_string_append (&dict_entry_signature,
                            DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
                            DBUS_TYPE_INT32_AS_STRING))
    goto oom;

  if (!_dbus_string_copy (&entry_value_signature, 0,
                          &dict_entry_signature,
                          _dbus_string_get_length (&dict_entry_signature)))
    goto oom;

  if (!_dbus_string_append_byte (&dict_entry_signature,
                                 DBUS_DICT_ENTRY_END_CHAR))
    goto oom;

  if (!_dbus_type_writer_recurse (writer, DBUS_TYPE_ARRAY,
                                  &dict_entry_signature, 0,
                                  &sub))
    goto oom;

  i = 0;
  while (i < n_entries)
    {
      DBusTypeWriter entry_sub;
      dbus_int32_t key;

      if (!_dbus_type_writer_recurse (&sub, DBUS_TYPE_DICT_ENTRY,
                                      NULL, 0,
                                      &entry_sub))
        goto oom;

      key = (dbus_int32_t) uint32_from_seed (seed + i);

      if (!_dbus_type_writer_write_basic (&entry_sub,
                                          DBUS_TYPE_INT32,
                                          &key))
        goto oom;

      if (!node_write_value (child, block, &entry_sub, seed + i))
        goto oom;

      if (!_dbus_type_writer_unrecurse (&sub, &entry_sub))
        goto oom;

      ++i;
    }

  if (!_dbus_type_writer_unrecurse (writer, &sub))
    goto oom;

  _dbus_string_free (&entry_value_signature);
  _dbus_string_free (&dict_entry_signature);
  return TRUE;

 oom:
  data_block_restore (block, &saved);
  _dbus_string_free (&entry_value_signature);
  _dbus_string_free (&dict_entry_signature);
  return FALSE;
}

static dbus_bool_t
dict_read_or_set_value (TestTypeNode   *node,
                        DBusTypeReader *reader,
                        DBusTypeReader *realign_root,
                        unsigned int    seed)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  DBusTypeReader sub;
  int i;
  int n_entries;
  TestTypeNode *child;

  n_entries = node->klass->subclass_detail;

  check_expected_type (reader, DBUS_TYPE_ARRAY);

  child = _dbus_list_get_first (&container->children);

  if (n_entries > 0)
    {
      _dbus_type_reader_recurse (reader, &sub);

      check_expected_type (&sub, DBUS_TYPE_DICT_ENTRY);

      i = 0;
      while (i < n_entries)
        {
          DBusTypeReader entry_sub;

          check_expected_type (&sub, DBUS_TYPE_DICT_ENTRY);

          _dbus_type_reader_recurse (&sub, &entry_sub);

          if (realign_root == NULL)
            {
              dbus_int32_t v;

              check_expected_type (&entry_sub, DBUS_TYPE_INT32);

              _dbus_type_reader_read_basic (&entry_sub, &v);

              _dbus_assert (v == (dbus_int32_t) uint32_from_seed (seed + i));

              NEXT_EXPECTING_TRUE (&entry_sub);

              if (!node_read_value (child, &entry_sub, seed + i))
                return FALSE;

              NEXT_EXPECTING_FALSE (&entry_sub);
            }
          else
            {
              dbus_int32_t v;

              v = (dbus_int32_t) uint32_from_seed (seed + i);

              if (!_dbus_type_reader_set_basic (&entry_sub,
                                                &v,
                                                realign_root))
                return FALSE;

              NEXT_EXPECTING_TRUE (&entry_sub);

              if (!node_set_value (child, &entry_sub, realign_root, seed + i))
                return FALSE;

              NEXT_EXPECTING_FALSE (&entry_sub);
            }

          if (i == (n_entries - 1))
            NEXT_EXPECTING_FALSE (&sub);
          else
            NEXT_EXPECTING_TRUE (&sub);

          ++i;
        }
    }

  return TRUE;
}

static dbus_bool_t
dict_read_value (TestTypeNode   *node,
                 DBusTypeReader *reader,
                 unsigned int    seed)
{
  return dict_read_or_set_value (node, reader, NULL, seed);
}

static dbus_bool_t
dict_set_value (TestTypeNode   *node,
                DBusTypeReader *reader,
                DBusTypeReader *realign_root,
                unsigned int    seed)
{
  return dict_read_or_set_value (node, reader, realign_root, seed);
}

static dbus_bool_t
dict_build_signature (TestTypeNode   *node,
                      DBusString     *str)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  int orig_len;

  orig_len = _dbus_string_get_length (str);

  if (!_dbus_string_append_byte (str, DBUS_TYPE_ARRAY))
    goto oom;

  if (!_dbus_string_append (str, DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_INT32_AS_STRING))
    goto oom;

  if (!node_build_signature (_dbus_list_get_first (&container->children),
                             str))
    goto oom;

  if (!_dbus_string_append_byte (str, DBUS_DICT_ENTRY_END_CHAR))
    goto oom;

  return TRUE;

 oom:
  _dbus_string_set_length (str, orig_len);
  return FALSE;
}

static void
container_destroy (TestTypeNode *node)
{
  TestTypeNodeContainer *container = (TestTypeNodeContainer*) node;
  DBusList *link;

  link = _dbus_list_get_first_link (&container->children);
  while (link != NULL)
    {
      TestTypeNode *child = link->data;
      DBusList *next = _dbus_list_get_next_link (&container->children, link);

      node_destroy (child);

      _dbus_list_free_link (link);

      link = next;
    }
}

#endif /* !DOXYGEN_SHOULD_SKIP_THIS */

#endif /* DBUS_ENABLE_EMBEDDED_TESTS */
