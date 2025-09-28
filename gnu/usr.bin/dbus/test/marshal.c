/* Simple sanity-check for D-Bus message serialization.
 *
 * Author: Simon McVittie <simon.mcvittie@collabora.co.uk>
 * Copyright © 2010-2011 Nokia Corporation
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
#include <string.h>

#include <dbus/dbus.h>

#include "test-utils-glib.h"

typedef struct {
    DBusError e;
} Fixture;

static void
assert_no_error (const DBusError *e)
{
  if (G_UNLIKELY (dbus_error_is_set (e)))
    g_error ("expected success but got error: %s: %s", e->name, e->message);
}

static void
setup (Fixture *f,
    gconstpointer arg G_GNUC_UNUSED)
{
  dbus_error_init (&f->e);
}

/* this is meant to be obviously correct, not efficient! */
static guint32
get_uint32 (const gchar *blob,
    gsize offset,
    char endian)
{
  if (endian == 'l')
    {
      return
        blob[offset] |
        (blob[offset + 1] << 8) |
        (blob[offset + 2] << 16) |
        (blob[offset + 3] << 24);
    }
  else if (endian == 'B')
    {
      return
        (blob[offset] << 24) |
        (blob[offset + 1] << 16) |
        (blob[offset + 2] << 8) |
        blob[offset + 3];
    }
  else
    {
      g_assert_not_reached ();
    }
}

#define BLOB_LENGTH (sizeof (le_blob) - 1)
#define OFFSET_BODY_LENGTH (4)
#define OFFSET_SERIAL (8)

const gchar le_blob[] =
    /* byte 0 */
    /* yyyyuu fixed headers */
    "l"                     /* little-endian */
    "\2"                    /* reply (which is the simplest message) */
    "\2"                    /* no auto-starting */
    "\1"                    /* D-Bus version = 1 */
    /* byte 4 */
    "\4\0\0\0"              /* bytes in body = 4 */
    /* byte 8 */
    "\x78\x56\x34\x12"      /* serial number = 0x12345678 */
    /* byte 12 */
    /* a(uv) variable headers start here */
    "\x0f\0\0\0"            /* bytes in array of variable headers = 15 */
                            /* pad to 8-byte boundary = nothing */
    /* byte 16 */
    "\5"                    /* in reply to: */
        "\1u\0"             /* variant signature = u */
                            /* pad to 4-byte boundary = nothing */
        "\x12\xef\xcd\xab"  /* 0xabcdef12 */
                            /* pad to 8-byte boundary = nothing */
    /* byte 24 */
    "\x08"                  /* signature: */
        "\1g\0"             /* variant signature = g */
        "\1u\0"             /* 1 byte, u, NUL (no alignment needed) */
        "\0"                /* pad to 8-byte boundary for body */
    /* body; byte 32 */
    "\xef\xbe\xad\xde"      /* 0xdeadbeef */
    ;

const gchar be_blob[] =
    /* byte 0 */
    /* yyyyuu fixed headers */
    "B"                     /* big-endian */
    "\2"                    /* reply (which is the simplest message) */
    "\2"                    /* no auto-starting */
    "\1"                    /* D-Bus version = 1 */
    /* byte 4 */
    "\0\0\0\4"              /* bytes in body = 4 */
    /* byte 8 */
    "\x12\x34\x56\x78"      /* serial number = 0x12345678 */
    /* byte 12 */
    /* a(uv) variable headers start here */
    "\0\0\0\x0f"            /* bytes in array of variable headers = 15 */
                            /* pad to 8-byte boundary = nothing */
    /* byte 16 */
    "\5"                    /* in reply to: */
        "\1u\0"             /* variant signature = u */
                            /* pad to 4-byte boundary = nothing */
        "\xab\xcd\xef\x12"  /* 0xabcdef12 */
                            /* pad to 8-byte boundary = nothing */
    /* byte 24 */
    "\x08"                  /* signature: */
        "\1g\0"             /* variant signature = g */
        "\1u\0"             /* 1 byte, u, NUL (no alignment needed) */
        "\0"                /* pad to 8-byte boundary for body */
    /* body; byte 32 */
    "\xde\xad\xbe\xef"      /* 0xdeadbeef */
    ;

static void
test_endian (Fixture *f,
    gconstpointer arg)
{
  const gchar *blob = arg;
  char *output;
  DBusMessage *m;
  int len;
  dbus_uint32_t u;
  dbus_bool_t ok;

  g_assert_cmpuint ((guint) sizeof (le_blob), ==, (guint) sizeof (be_blob));

  g_assert_cmpuint (get_uint32 (blob, OFFSET_BODY_LENGTH, blob[0]), ==, 4);
  g_assert_cmpuint (get_uint32 (blob, OFFSET_SERIAL, blob[0]), ==,
      0x12345678u);

  len = dbus_message_demarshal_bytes_needed (blob, sizeof (le_blob));
  /* everything in the string except the implicit "\0" at the end is part of
   * the message */
  g_assert_cmpint (len, ==, BLOB_LENGTH);

  m = dbus_message_demarshal (blob, sizeof (le_blob), &f->e);
  assert_no_error (&f->e);
  g_assert (m != NULL);

  g_assert_cmpuint (dbus_message_get_serial (m), ==, 0x12345678u);
  g_assert_cmpuint (dbus_message_get_reply_serial (m), ==, 0xabcdef12u);
  g_assert_cmpstr (dbus_message_get_signature (m), ==, "u");

  /* Implementation detail: appending to the message results in it being
   * byteswapped into compiler byte order, which exposed a bug in libdbus,
   * fd.o #38120. (If that changes, this test might not exercise that
   * particular bug but will still be valid.) */
  u = 0xdecafbadu;
  ok = dbus_message_append_args (m,
      DBUS_TYPE_UINT32, &u,
      DBUS_TYPE_INVALID);
  g_assert (ok);

  dbus_message_marshal (m, &output, &len);

  g_assert (output[0] == 'l' || output[0] == 'B');
  /* the single-byte fields are unaffected, even if the endianness was
   * swapped */
  g_assert_cmpint (output[1], ==, blob[1]);
  g_assert_cmpint (output[2], ==, blob[2]);
  g_assert_cmpint (output[3], ==, blob[3]);
  /* the length and serial are in the new endianness, the length has expanded
   * to 8, and the serial is correct */
  g_assert_cmpuint (get_uint32 (output, OFFSET_BODY_LENGTH, output[0]), ==, 8);
  g_assert_cmpuint (get_uint32 (output, OFFSET_SERIAL, output[0]), ==,
      0x12345678u);
  /* the second "u" in the signature replaced a padding byte, so only
   * the length of the body changed */
  g_assert_cmpint (len, ==, BLOB_LENGTH + 4);

  dbus_clear_message (&m);
  dbus_free (output);
}

static void
test_needed (Fixture *f,
    gconstpointer arg)
{
  const gchar *blob = arg;

  /* We need at least 16 bytes to know how long the message is - that's just
   * a fact of the D-Bus protocol. */
  g_assert_cmpint (
      dbus_message_demarshal_bytes_needed (blob, 0), ==, 0);
  g_assert_cmpint (
      dbus_message_demarshal_bytes_needed (blob, 15), ==, 0);
  /* This is enough that we should be able to tell how much we need. */
  g_assert_cmpint (
      dbus_message_demarshal_bytes_needed (blob, 16), ==, BLOB_LENGTH);
  /* The header is 32 bytes long (here), so that's another interesting
   * boundary. */
  g_assert_cmpint (
      dbus_message_demarshal_bytes_needed (blob, 31), ==, BLOB_LENGTH);
  g_assert_cmpint (
      dbus_message_demarshal_bytes_needed (blob, 32), ==, BLOB_LENGTH);
  g_assert_cmpint (
      dbus_message_demarshal_bytes_needed (blob, 33), ==, BLOB_LENGTH);
  g_assert_cmpint (
      dbus_message_demarshal_bytes_needed (blob, BLOB_LENGTH - 1), ==,
      BLOB_LENGTH);
  g_assert_cmpint (
      dbus_message_demarshal_bytes_needed (blob, BLOB_LENGTH), ==,
      BLOB_LENGTH);
  g_assert_cmpint (
      dbus_message_demarshal_bytes_needed (blob, sizeof (be_blob)), ==,
      BLOB_LENGTH);
}

static void
teardown (Fixture *f,
    gconstpointer arg G_GNUC_UNUSED)
{
  dbus_error_free (&f->e);
}

int
main (int argc,
    char **argv)
{
  int ret;
  char *aligned_le_blob;
  char *aligned_be_blob;

  test_init (&argc, &argv);

  /* We have to pass in a buffer that's at least "default aligned",
   * i.e.  on GNU systems to 8 or 16.  The linker may have only given
   * us byte-alignment for the char[] static variables.
   */
  aligned_le_blob = g_malloc (sizeof (le_blob));
  memcpy (aligned_le_blob, le_blob, sizeof (le_blob));
  aligned_be_blob = g_malloc (sizeof (be_blob));
  memcpy (aligned_be_blob, be_blob, sizeof (be_blob));  

  g_test_add ("/demarshal/le", Fixture, aligned_le_blob, setup, test_endian, teardown);
  g_test_add ("/demarshal/be", Fixture, aligned_be_blob, setup, test_endian, teardown);
  g_test_add ("/demarshal/needed/le", Fixture, aligned_le_blob, setup, test_needed,
      teardown);
  g_test_add ("/demarshal/needed/be", Fixture, aligned_be_blob, setup, test_needed,
      teardown);

  ret = g_test_run ();
  g_free (aligned_le_blob);
  g_free (aligned_be_blob);
  dbus_shutdown ();
  return ret;
}
