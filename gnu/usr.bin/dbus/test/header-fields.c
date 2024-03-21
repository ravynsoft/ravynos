/* Unit tests for detailed header field manipulation
 *
 * Copyright © 2017 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <config.h>

#include <glib.h>

#include <dbus/dbus.h>
#include "dbus/dbus-internals.h"
#include "dbus/dbus-marshal-recursive.h"
#include "dbus/dbus-message-private.h"
#include "dbus/dbus-string.h"
#include "dbus/dbus-test-tap.h"
#include "test-utils-glib.h"

typedef struct {
    const gchar *mode;
    TestMainContext *ctx;
    DBusConnection *left_conn;
    DBusConnection *right_conn;
    GPid daemon_pid;
    gchar *address;
    GQueue held_messages;
    gboolean skip;
} Fixture;

static DBusHandlerResult
hold_filter (DBusConnection *connection,
             DBusMessage *message,
             void *user_data)
{
  Fixture *f = user_data;

  if (dbus_message_get_type (message) != DBUS_MESSAGE_TYPE_METHOD_CALL)
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

  g_queue_push_tail (&f->held_messages, dbus_message_ref (message));

  return DBUS_HANDLER_RESULT_HANDLED;
}

static void
setup_dbus_daemon (Fixture *f,
                   gconstpointer context G_GNUC_UNUSED)
{
  f->address = test_get_dbus_daemon (NULL, TEST_USER_ME, NULL, &f->daemon_pid);

  if (f->address == NULL)
    {
      f->skip = TRUE;
      return;
    }
}

static void
teardown_dbus_daemon (Fixture *f,
                      gconstpointer context G_GNUC_UNUSED)
{
  if (f->daemon_pid != 0)
    {
      test_kill_pid (f->daemon_pid);
      g_spawn_close_pid (f->daemon_pid);
      f->daemon_pid = 0;
    }

  g_clear_pointer (&f->address, g_free);
}

/* Offset to byte order from start of header */
#define BYTE_ORDER_OFFSET    0
/* Offset to version from start of header */
#define VERSION_OFFSET       3
/* Offset to fields array length from start of header, in protocol v1 */
#define FIELDS_ARRAY_LENGTH_OFFSET 12
/** Offset to first field in header, in protocol v1 */
#define FIRST_FIELD_OFFSET 16

/* Offset from start of _dbus_header_signature_str to the signature of
 * the fields array */
#define FIELDS_ARRAY_SIGNATURE_OFFSET 6

/* A byte that is not a DBUS_HEADER_FIELD_* */
#define NOT_A_HEADER_FIELD 123

_DBUS_STRING_DEFINE_STATIC(_dbus_header_signature_str, DBUS_HEADER_SIGNATURE);

static void
string_overwrite_n (DBusString *str,
                    int         start,
                    const void *bytes,
                    int         len)
{
  unsigned char *data = _dbus_string_get_udata_len (str, start, len);

  g_assert (data != NULL);
  memcpy (data, bytes, len);
}

static void
steal_reply_cb (DBusPendingCall *pc,
                void *data)
{
  DBusMessage **message_p = data;

  g_assert (message_p != NULL);
  g_assert (*message_p == NULL);
  *message_p = dbus_pending_call_steal_reply (pc);
  g_assert (*message_p != NULL);
}

/*
 * Test the handling of unknown header fields.
 *
 * Return TRUE if the right thing happens, but the right thing might include
 * OOM.
 */
static dbus_bool_t
test_weird_header_field (void        *user_data,
                         dbus_bool_t  have_memory)
{
  Fixture *f = user_data;
  const char *body = "hello";
  const char *new_body = NULL;
  DBusError error = DBUS_ERROR_INIT;
  DBusMessage *original = NULL;
  DBusMessage *modified = NULL;
  DBusMessage *filtered = NULL;
  DBusMessage *relayed = NULL;
  DBusMessage *reply = NULL;
  DBusPendingCall *pc = NULL;
  char *blob = NULL;
  int blob_len;
  DBusString modified_blob = _DBUS_STRING_INIT_INVALID;
  /* This is the serialization of a struct (uv), assumed to be at an
   * 8-byte boundary. */
  unsigned char weird_header[8] = {
      NOT_A_HEADER_FIELD,   /*< type code */
      1,                    /*< length of signature */
      'u',                  /*< signature: uint32 */
      '\0',                 /*< end of signature */
      /* no padding required */
      '\x12',               /*< uint32 0x12345678 (BE) or 0x78563412 (LE) */
      '\x34',
      '\x56',
      '\x78'
  };
  int bytes_needed;
  DBusTypeReader reader;
  DBusTypeReader array;
  gboolean added_hold_filter = FALSE;
  GError *gerror = NULL;

  if (f->skip)
    return TRUE;

  /* We'd normally do this in setup_dbus_daemon(), but then we couldn't
   * allocate memory that we want freed by dbus_shutdown(). */
  g_assert_cmpint (_dbus_get_malloc_blocks_outstanding (), ==, 0);
  f->ctx = test_main_context_try_get ();

  if (f->ctx == NULL)
    {
      g_assert_false (have_memory);
      goto out;
    }

  f->left_conn = test_try_connect_to_bus (f->ctx, f->address, &gerror);

  if (f->left_conn == NULL)
    {
      g_assert_error (gerror, G_DBUS_ERROR, G_DBUS_ERROR_NO_MEMORY);
      g_assert_false (have_memory);
      goto out;
    }

  f->right_conn = test_try_connect_to_bus (f->ctx, f->address, &gerror);

  if (f->right_conn == NULL)
    {
      g_assert_error (gerror, G_DBUS_ERROR, G_DBUS_ERROR_NO_MEMORY);
      g_assert_false (have_memory);
      goto out;
    }

  if (!dbus_connection_add_filter (f->right_conn, hold_filter, f, NULL))
    {
      g_assert_false (have_memory);
      goto out;
    }

  added_hold_filter = TRUE;

  original = dbus_message_new_method_call (dbus_bus_get_unique_name (f->right_conn),
                                           "/com/example/Path",
                                           "com.example.Interface",
                                           "Method");

  if (original == NULL ||
      !dbus_message_append_args (original,
                                 DBUS_TYPE_STRING, &body,
                                 DBUS_TYPE_INVALID))
    {
      g_assert_false (have_memory);
      goto out;
    }

  /* Messages with serial number 0 can't be demarshalled. */
  dbus_message_set_serial (original, 42);

  if (!dbus_message_marshal (original, &blob, &blob_len))
    {
      g_assert_false (have_memory);
      goto out;
    }

  /* We will add up to 8 bytes, so preallocate that much. */
  if (!_dbus_string_init_preallocated (&modified_blob, blob_len + 8) ||
      !_dbus_string_append_len (&modified_blob, blob, blob_len))
    {
      g_assert_false (have_memory);
      goto out;
    }

  /* If these are false then we need to change our byte-twiddling */
  g_assert_cmpint (blob_len, >, FIRST_FIELD_OFFSET);
  g_assert_cmpint (blob[VERSION_OFFSET], ==, 1);
  g_assert_cmpint (blob[BYTE_ORDER_OFFSET], ==, DBUS_COMPILER_BYTE_ORDER);

  if (f->mode == NULL)
    {
      /* Do nothing: don't insert a weird header field at all */
    }
  else if (g_str_equal (f->mode, "change") || g_str_equal (f->mode, "multi"))
    {
      /* Replace the interface (which is optional anyway) with the
       * weird header field */

      _dbus_type_reader_init (&reader, DBUS_COMPILER_BYTE_ORDER,
                              &_dbus_header_signature_str,
                              FIELDS_ARRAY_SIGNATURE_OFFSET,
                              &modified_blob,
                              FIELDS_ARRAY_LENGTH_OFFSET);
      _dbus_type_reader_recurse (&reader, &array);

      while (_dbus_type_reader_get_current_type (&array) !=
             DBUS_TYPE_INVALID)
        {
          DBusTypeReader sub;
          unsigned char field_code;

          _dbus_type_reader_recurse (&array, &sub);

          g_assert_cmpint (_dbus_type_reader_get_current_type (&sub),
                           ==, DBUS_TYPE_BYTE);
          _dbus_type_reader_read_basic (&sub, &field_code);

          if (field_code == DBUS_HEADER_FIELD_INTERFACE)
            {
              _dbus_string_set_byte (&modified_blob,
                                     _dbus_type_reader_get_value_pos (&sub),
                                     NOT_A_HEADER_FIELD);
              break;
            }

          _dbus_type_reader_next (&array);
        }

      if (g_str_equal (f->mode, "multi"))
        {
          dbus_uint32_t header_fields_length;
          unsigned int i;

          memcpy (&header_fields_length, &blob[FIELDS_ARRAY_LENGTH_OFFSET], 4);

          /* Same as prepend, twice */
          for (i = 1; i <= 2; i++)
            {
              weird_header[0] = NOT_A_HEADER_FIELD - i;

              if (!_dbus_string_insert_8_aligned (&modified_blob,
                                                  FIRST_FIELD_OFFSET,
                                                  weird_header))
                {
                  g_assert_false (have_memory);
                  goto out;
                }

              header_fields_length += 8;
            }

          /* Same as append, twice (see below) */
          header_fields_length = _DBUS_ALIGN_VALUE (header_fields_length, 8);
          g_assert_cmpint (header_fields_length % 8, ==, 0);

          for (i = 1; i <= 2; i++)
            {
              weird_header[0] = NOT_A_HEADER_FIELD + i;

              if (!_dbus_string_insert_8_aligned (&modified_blob,
                                                  (FIRST_FIELD_OFFSET +
                                                   header_fields_length),
                                                  weird_header))
                {
                  g_assert_false (have_memory);
                  goto out;
                }

              header_fields_length += 8;
            }

          string_overwrite_n (&modified_blob, FIELDS_ARRAY_LENGTH_OFFSET,
                              &header_fields_length, 4);
        }
    }
  else if (g_str_equal (f->mode, "prepend"))
    {
      dbus_uint32_t header_fields_length;

      memcpy (&header_fields_length, &blob[FIELDS_ARRAY_LENGTH_OFFSET], 4);

      /* Insert a weird header field at the beginning of the fields
       * array. We do this by byte manipulation rather than by using a
       * DBusTypeReader, because we eventually want to get rid of the
       * counterintuitive ability for a DBusTypeReader to write to the
       * message: https://bugs.freedesktop.org/show_bug.cgi?id=38288 */
      if (!_dbus_string_insert_8_aligned (&modified_blob,
                                          FIRST_FIELD_OFFSET,
                                          weird_header))
        {
          g_assert_false (have_memory);
          goto out;
        }

      header_fields_length += 8;
      string_overwrite_n (&modified_blob, FIELDS_ARRAY_LENGTH_OFFSET,
                          &header_fields_length, 4);
    }
  else if (g_str_equal (f->mode, "append"))
    {
      dbus_uint32_t header_fields_length;

      memcpy (&header_fields_length, &blob[FIELDS_ARRAY_LENGTH_OFFSET], 4);

      /* Insert a weird header field at the end of the fields
       * array, after the padding (which was previously the padding between
       * header and body, and is now the padding between the last-but-one
       * header field and the new header field). For simplicity, we've
       * used a weird header field that does not ever need to be followed
       * by padding itself.
       *
       * Old:
       *  | 0   | 1   | 2   | 3   | 4   | 5   | 6   | 7    |
       *  | ... 16-byte fixed-length part ...              |
       *  | ... 16-byte fixed-length part ...              |
       * [A] first header field ...                        |
       *  | last header field    [B] padding              [C]
       * [C] body...
       *
       * New:
       *  | 0   | 1   | 2   | 3   | 4   | 5   | 6   | 7    |
       *  | ... 16-byte fixed-length part ...              |
       *  | ... 16-byte fixed-length part ...              |
       * [A] first header field ...                        |
       *  | previously-last field [B] padding             [C]
       *  | weird header field, exactly 8 bytes long    [B'=C']
       * [B'=C'] body...
       *
       */
      header_fields_length = _DBUS_ALIGN_VALUE (header_fields_length, 8);
      g_assert_cmpint (header_fields_length % 8, ==, 0);

      if (!_dbus_string_insert_8_aligned (&modified_blob,
                                          (FIRST_FIELD_OFFSET +
                                           header_fields_length),
                                          weird_header))
        {
          g_assert_false (have_memory);
          goto out;
        }

      header_fields_length += 8;
      string_overwrite_n (&modified_blob, FIELDS_ARRAY_LENGTH_OFFSET,
                          &header_fields_length, 4);
    }
  else
    {
      g_assert_not_reached ();
    }

  /* OK, now we've hacked up the message, compare it with the original. */
  bytes_needed = dbus_message_demarshal_bytes_needed (_dbus_string_get_const_data (&modified_blob),
                                                      _dbus_string_get_length (&modified_blob));

  if (f->mode == NULL || g_str_equal (f->mode, "change"))
    {
      /* We edited the message in-place so its effective length didn't
       * change */
      g_assert_cmpint (bytes_needed, ==, blob_len);
    }
  else if (g_str_equal (f->mode, "multi"))
    {
      g_assert_cmpint (bytes_needed, ==, blob_len + 32);
    }
  else
    {
      g_assert_cmpint (bytes_needed, ==, blob_len + 8);
    }

  g_assert_cmpint (_dbus_string_get_length (&modified_blob), ==,
                   bytes_needed);
  modified = dbus_message_demarshal (_dbus_string_get_const_data (&modified_blob),
                                     _dbus_string_get_length (&modified_blob),
                                     &error);

  if (modified == NULL)
    {
      g_assert_cmpstr (error.name, ==, DBUS_ERROR_NO_MEMORY);
      g_assert_false (have_memory);
      goto out;
    }

  /* The modified message has the same fields, except possibly the
   * interface. */
  g_assert_cmpint (dbus_message_get_type (modified), ==,
                   dbus_message_get_type (original));
  g_assert_cmpstr (dbus_message_get_path (modified), ==,
                   dbus_message_get_path (original));
  g_assert_cmpstr (dbus_message_get_member (modified), ==,
                   dbus_message_get_member (original));
  g_assert_cmpstr (dbus_message_get_error_name (modified), ==,
                   dbus_message_get_error_name (original));
  g_assert_cmpstr (dbus_message_get_destination (modified), ==,
                   dbus_message_get_destination (original));
  g_assert_cmpstr (dbus_message_get_sender (modified), ==,
                   dbus_message_get_sender (original));
  g_assert_cmpstr (dbus_message_get_signature (modified), ==,
                   dbus_message_get_signature (original));
  g_assert_cmpint (dbus_message_get_no_reply (modified), ==,
                   dbus_message_get_no_reply (original));
  g_assert_cmpint (dbus_message_get_serial (modified), ==,
                   dbus_message_get_serial (original));
  g_assert_cmpint (dbus_message_get_reply_serial (modified), ==,
                   dbus_message_get_reply_serial (original));
  g_assert_cmpint (dbus_message_get_auto_start (modified), ==,
                   dbus_message_get_auto_start (original));
  g_assert_cmpint (dbus_message_get_allow_interactive_authorization (modified),
                   ==,
                   dbus_message_get_allow_interactive_authorization (original));

  if (dbus_message_get_args (modified, &error,
                             DBUS_TYPE_STRING, &new_body,
                             DBUS_TYPE_INVALID))
    {
      g_assert_cmpstr (new_body, ==, body);
    }
  else
    {
      g_assert_cmpstr (error.name, ==, DBUS_ERROR_NO_MEMORY);
      g_assert_false (have_memory);
      goto out;
    }

  if (f->mode != NULL &&
      (g_str_equal (f->mode, "change") || g_str_equal (f->mode, "multi")))
    {
      /* We edited the interface field in-place to turn it into the
       * unknown field, so it doesn't have an interface any more. */
      g_assert_cmpstr (dbus_message_get_interface (modified), ==, NULL);
    }
  else
    {
      /* We didn't change the interface. */
      g_assert_cmpstr (dbus_message_get_interface (modified), ==,
                       dbus_message_get_interface (original));
    }

  /* Copy the modified message so we can filter it. */
  filtered = dbus_message_demarshal (_dbus_string_get_const_data (&modified_blob),
                                     _dbus_string_get_length (&modified_blob),
                                     &error);

  if (filtered == NULL)
    {
      g_assert_cmpstr (error.name, ==, DBUS_ERROR_NO_MEMORY);
      g_assert_false (have_memory);
      goto out;
    }

  if (!_dbus_message_remove_unknown_fields (filtered))
    {
      g_assert_false (have_memory);
      goto out;
    }

  /* All known headers are the same as in the modified message that was
   * deserialized from the same blob */
  g_assert_cmpint (dbus_message_get_type (filtered), ==,
                   dbus_message_get_type (modified));
  g_assert_cmpstr (dbus_message_get_path (filtered), ==,
                   dbus_message_get_path (modified));
  g_assert_cmpstr (dbus_message_get_member (filtered), ==,
                   dbus_message_get_member (modified));
  g_assert_cmpstr (dbus_message_get_error_name (filtered), ==,
                   dbus_message_get_error_name (modified));
  g_assert_cmpstr (dbus_message_get_destination (filtered), ==,
                   dbus_message_get_destination (modified));
  g_assert_cmpstr (dbus_message_get_sender (filtered), ==,
                   dbus_message_get_sender (modified));
  g_assert_cmpstr (dbus_message_get_signature (filtered), ==,
                   dbus_message_get_signature (modified));
  g_assert_cmpint (dbus_message_get_no_reply (filtered), ==,
                   dbus_message_get_no_reply (modified));
  g_assert_cmpint (dbus_message_get_serial (filtered), ==,
                   dbus_message_get_serial (modified));
  g_assert_cmpint (dbus_message_get_reply_serial (filtered), ==,
                   dbus_message_get_reply_serial (modified));
  g_assert_cmpint (dbus_message_get_auto_start (filtered), ==,
                   dbus_message_get_auto_start (modified));
  g_assert_cmpint (dbus_message_get_allow_interactive_authorization (modified),
                   ==,
                   dbus_message_get_allow_interactive_authorization (original));

  /* The body is also the same */
  if (dbus_message_get_args (filtered, &error,
                             DBUS_TYPE_STRING, &new_body,
                             DBUS_TYPE_INVALID))
    {
      g_assert_cmpstr (new_body, ==, body);
    }
  else
    {
      g_assert_cmpstr (error.name, ==, DBUS_ERROR_NO_MEMORY);
      g_assert_false (have_memory);
      goto out;
    }

  /* We can't use _dbus_header_get_field_raw() because it asserts that
   * the field in question is in range. */
  _dbus_type_reader_init (&reader, DBUS_COMPILER_BYTE_ORDER,
                          &_dbus_header_signature_str,
                          FIELDS_ARRAY_SIGNATURE_OFFSET,
                          &filtered->header.data,
                          FIELDS_ARRAY_LENGTH_OFFSET);
  _dbus_type_reader_recurse (&reader, &array);

  while (_dbus_type_reader_get_current_type (&array) != DBUS_TYPE_INVALID)
    {
      DBusTypeReader sub;
      unsigned char field_code;

      _dbus_type_reader_recurse (&array, &sub);

      g_assert_cmpint (_dbus_type_reader_get_current_type (&sub),
                       ==, DBUS_TYPE_BYTE);
      _dbus_type_reader_read_basic (&sub, &field_code);

      g_assert_cmpuint (field_code, !=, NOT_A_HEADER_FIELD - 2);
      g_assert_cmpuint (field_code, !=, NOT_A_HEADER_FIELD - 1);
      g_assert_cmpuint (field_code, !=, NOT_A_HEADER_FIELD);
      g_assert_cmpuint (field_code, !=, NOT_A_HEADER_FIELD + 1);
      g_assert_cmpuint (field_code, !=, NOT_A_HEADER_FIELD + 2);

      _dbus_type_reader_next (&array);
    }

  /* Sending a message through the dbus-daemon currently preserves
   * unknown header fields, but it should not.
   * https://bugs.freedesktop.org/show_bug.cgi?id=100317 */

  /* We have to use send_with_reply because if we don't, we won't know
   * if the message was dropped on the floor due to out-of-memory. */
  if (!dbus_connection_send_with_reply (f->left_conn, modified, &pc, -1))
    {
      g_assert_false (have_memory);
      goto out;
    }

  if (dbus_pending_call_get_completed (pc))
    {
      steal_reply_cb (pc, &reply);
    }
  else if (!dbus_pending_call_set_notify (pc, steal_reply_cb, &reply, NULL))
    {
      dbus_pending_call_cancel (pc);
      g_assert_false (have_memory);
      goto out;
    }

  while (g_queue_get_length (&f->held_messages) < 1 && reply == NULL)
    test_main_context_iterate (f->ctx, TRUE);

  if (reply != NULL)
    {
      dbus_set_error_from_message (&error, reply);
      g_assert_cmpstr (error.name, ==, DBUS_ERROR_NO_MEMORY);
      g_assert_false (have_memory);
      goto out;
    }

  relayed = g_queue_pop_head (&f->held_messages);
  g_assert_cmpint (g_queue_get_length (&f->held_messages), ==, 0);

  /* The message relayed through the dbus-daemon has the same known
   * fields and content as the one we sent, except that it has a sender
   * and a serial number. */
  g_assert_cmpint (dbus_message_get_type (relayed), ==,
                   dbus_message_get_type (modified));
  g_assert_cmpstr (dbus_message_get_path (relayed), ==,
                   dbus_message_get_path (modified));
  g_assert_cmpstr (dbus_message_get_member (relayed), ==,
                   dbus_message_get_member (modified));
  g_assert_cmpstr (dbus_message_get_error_name (relayed), ==,
                   dbus_message_get_error_name (modified));
  g_assert_cmpstr (dbus_message_get_destination (relayed), ==,
                   dbus_message_get_destination (modified));
  g_assert_cmpstr (dbus_message_get_sender (relayed), ==,
                   dbus_bus_get_unique_name (f->left_conn));
  g_assert_cmpstr (dbus_message_get_signature (relayed), ==,
                   dbus_message_get_signature (modified));
  g_assert_cmpint (dbus_message_get_no_reply (relayed), ==,
                   dbus_message_get_no_reply (modified));
  g_assert_cmpint (dbus_message_get_serial (relayed), !=, 0);
  g_assert_cmpint (dbus_message_get_reply_serial (relayed), ==,
                   dbus_message_get_reply_serial (modified));
  g_assert_cmpint (dbus_message_get_auto_start (relayed), ==,
                   dbus_message_get_auto_start (modified));
  g_assert_cmpint (dbus_message_get_allow_interactive_authorization (modified),
                   ==,
                   dbus_message_get_allow_interactive_authorization (original));

  if (dbus_message_get_args (relayed, &error,
                             DBUS_TYPE_STRING, &new_body,
                             DBUS_TYPE_INVALID))
    {
      g_assert_cmpstr (new_body, ==, body);
    }
  else
    {
      g_assert_cmpstr (error.name, ==, DBUS_ERROR_NO_MEMORY);
      g_assert_false (have_memory);
      goto out;
    }

  _dbus_type_reader_init (&reader, DBUS_COMPILER_BYTE_ORDER,
                          &_dbus_header_signature_str,
                          FIELDS_ARRAY_SIGNATURE_OFFSET,
                          &relayed->header.data,
                          FIELDS_ARRAY_LENGTH_OFFSET);
  _dbus_type_reader_recurse (&reader, &array);

  while (_dbus_type_reader_get_current_type (&array) != DBUS_TYPE_INVALID)
    {
      DBusTypeReader sub;
      unsigned char field_code;

      _dbus_type_reader_recurse (&array, &sub);

      g_assert_cmpint (_dbus_type_reader_get_current_type (&sub),
                       ==, DBUS_TYPE_BYTE);
      _dbus_type_reader_read_basic (&sub, &field_code);

      g_assert_cmpuint (field_code, !=, NOT_A_HEADER_FIELD - 2);
      g_assert_cmpuint (field_code, !=, NOT_A_HEADER_FIELD - 1);
      g_assert_cmpuint (field_code, !=, NOT_A_HEADER_FIELD);
      g_assert_cmpuint (field_code, !=, NOT_A_HEADER_FIELD + 1);
      g_assert_cmpuint (field_code, !=, NOT_A_HEADER_FIELD + 2);

      _dbus_type_reader_next (&array);
    }

  /* On success, we don't actually reply: it's one more thing that
   * could hit OOM */

out:
  g_clear_error (&gerror);
  _dbus_string_free (&modified_blob);
  dbus_free (blob);

  if (pc != NULL)
    dbus_pending_call_cancel (pc);

  dbus_clear_pending_call (&pc);
  dbus_clear_message (&reply);
  dbus_clear_message (&relayed);
  dbus_clear_message (&filtered);
  dbus_clear_message (&modified);
  dbus_clear_message (&original);
  dbus_error_free (&error);

  /* We'd normally do this in teardown_dbus_daemon(), but for this test
   * we want to do it here so we can dbus_shutdown(). */
  if (f->left_conn != NULL)
    {
      dbus_connection_close (f->left_conn);
      test_connection_shutdown (f->ctx, f->left_conn);
    }

  if (f->right_conn != NULL)
    {
      GList *link;

      if (added_hold_filter)
        dbus_connection_remove_filter (f->right_conn, hold_filter, f);

      for (link = f->held_messages.head; link != NULL; link = link->next)
        dbus_message_unref (link->data);

      g_queue_clear (&f->held_messages);

      dbus_connection_close (f->right_conn);
      test_connection_shutdown (f->ctx, f->right_conn);
    }

  dbus_clear_connection (&f->left_conn);
  dbus_clear_connection (&f->right_conn);
  g_clear_pointer (&f->ctx, test_main_context_unref);

  dbus_shutdown ();
  g_assert_cmpint (_dbus_get_malloc_blocks_outstanding (), ==, 0);

  return !g_test_failed ();
}

typedef struct
{
  const gchar *name;
  DBusTestMemoryFunction function;
  const gchar *mode;
} OOMTestCase;

static void
test_oom_wrapper (Fixture *f,
                  gconstpointer data)
{
  const OOMTestCase *test = data;

  f->mode = test->mode;

  if (g_test_slow ())
    {
      /* When we say slow, we mean it. */
      test_timeout_reset (30);
    }
  else
    {
      test_timeout_reset (1);
    }

  if (!_dbus_test_oom_handling (test->name, test->function, f))
    {
      g_test_message ("OOM test failed");
      g_test_fail ();
    }
}

static GQueue *test_cases_to_free = NULL;

static void
add_oom_test (const gchar *name,
              DBusTestMemoryFunction function,
              const gchar *mode)
{
  /* By using GLib memory allocation here, we avoid being affected by
   * dbus_shutdown() or contributing to
   * _dbus_get_malloc_blocks_outstanding() */
  OOMTestCase *test_case = g_new0 (OOMTestCase, 1);

  test_case->name = name;
  test_case->function = function;
  test_case->mode = mode;
  g_test_add (name, Fixture, test_case, setup_dbus_daemon,
              test_oom_wrapper, teardown_dbus_daemon);
  g_queue_push_tail (test_cases_to_free, test_case);
}

int
main (int argc,
      char **argv)
{
  int ret;

  test_init (&argc, &argv);

  /* Normally we test up to 4 consecutive malloc failures, but that's
   * painfully slow here. */
  if (g_getenv ("DBUS_TEST_MALLOC_FAILURES") == NULL)
    {
      if (!g_test_slow ())
        g_setenv ("DBUS_TEST_MALLOC_FAILURES", "2", TRUE);
    }

  test_cases_to_free = g_queue_new ();
  add_oom_test ("/message/weird-header-field/none", test_weird_header_field,
                NULL);
  add_oom_test ("/message/weird-header-field/append", test_weird_header_field,
                "append");
  add_oom_test ("/message/weird-header-field/change", test_weird_header_field,
                "change");
  add_oom_test ("/message/weird-header-field/prepend", test_weird_header_field,
                "prepend");
  add_oom_test ("/message/weird-header-field/multi", test_weird_header_field,
                "multi");

  ret = g_test_run ();

  g_queue_free_full (test_cases_to_free, g_free);
  dbus_shutdown ();
  return ret;
}
