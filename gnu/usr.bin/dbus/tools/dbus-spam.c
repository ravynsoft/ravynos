/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-spam.c - a plain libdbus message-sender, loosely based on dbus-send
 *
 * Copyright © 2003 Philip Blundell <philb@gnu.org>
 * Copyright © 2011 Nokia Corporation
 * Copyright © 2014 Collabora Ltd.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dbus/dbus.h>

#include "test-tool.h"
#include "tool-common.h"

static dbus_bool_t ignore_errors = FALSE;

static void usage (int ecode) _DBUS_GNUC_NORETURN;

static void
usage (int ecode)
{
  fprintf (stderr,
           "Usage: dbus-test-tool spam [OPTIONS]\n"
           "\n"
           "Repeatedly call com.example.Spam() on the given D-Bus service.\n"
           "\n"
           "Options:\n"
           "\n"
           "    --session     use the session bus (default)\n"
           "    --system      use the system bus\n"
           "\n"
           "    --ignore-errors    ignore errors\n"
           "    --dest=NAME   call methods on NAME (default " DBUS_SERVICE_DBUS ")\n"
           "\n"
           "    --count=N     send N messages (default 1)\n"
           "    --queue=N     queue up N messages at a time (default 1)\n"
           "    --flood       send all messages immediately\n"
           "    --no-reply    set the NO_REPLY flag (implies --flood)\n"
           "    --messages-per-conn=N   after sending messages-per-conn, wait\n"
           "                  for the pending replies if any, then reconnect\n"
           "                  (default: don't reconnect)\n"
           "\n"
           "    --string      send payload as a string (default)\n"
           "    --bytes       send payload as a byte-array\n"
           "    --empty       send an empty payload\n"
           "\n"
           "    --payload=S   use S as payload (default \"hello, world!\")\n"
           "    --stdin       read payload from stdin, until EOF\n"
           "    --message-stdin   read a complete D-Bus message from stdin\n"
           "    --random-size read whitespace-separated ASCII decimal\n"
           "                  payload sizes from stdin and pick one randomly\n"
           "                  for each message\n"
           "\n"
           "    --seed=SEED   seed for srand (default is time())\n"
           "\n"
           );
  exit (ecode);
}

static void
pc_notify (DBusPendingCall *pc,
           void            *data)
{
  DBusMessage *message;
  int *received_p = data;
  DBusError error;

  dbus_error_init (&error);

  message = dbus_pending_call_steal_reply (pc);

  if (!ignore_errors && dbus_message_get_type (message) == DBUS_MESSAGE_TYPE_ERROR)
    {
      dbus_set_error_from_message (&error, message);
      fprintf (stderr, "Failed to receive reply #%d: %s: %s\n", *received_p,
               error.name, error.message);
    }
  else
    {
      VERBOSE (stderr, "received message #%d\n", *received_p);
    }

  (*received_p)++;
}

static void
consume_stdin (char   **payload_p,
               size_t  *len_p)
{
  const size_t BLOCK_SIZE = 4096;
  size_t len = BLOCK_SIZE;
  size_t pos = 0;
  size_t n;
  char *buf;

  buf = dbus_malloc (len);

  if (buf == NULL)
    tool_oom ("reading payload from stdin");

  while (1)
    {
      if (len - pos < BLOCK_SIZE)
        {
          char *tmp = dbus_realloc (buf, len + BLOCK_SIZE);

          if (tmp == NULL)
            tool_oom ("reading payload from stdin");

          buf = tmp;
          len += BLOCK_SIZE;
        }

      n = fread (buf + pos, 1, len - pos, stdin);

      if (n <= 0)
        {
          /* EOF or error - treat as EOF */
          break;
        }

      pos += n;
    }

  *len_p = pos;
  *payload_p = buf;
}

int
dbus_test_tool_spam (int argc, char **argv)
{
  DBusConnection *connection = NULL;
  DBusError error = DBUS_ERROR_INIT;
  DBusBusType type = DBUS_BUS_SESSION;
  const char *destination = DBUS_SERVICE_DBUS;
  int i;
  int count = 1;
  int sent = 0;
  unsigned int sent_in_this_conn = 0;
  int received = 0;
  unsigned int received_before_this_conn = 0;
  int queue_len = 1;
  const char *payload = NULL;
  char *payload_buf = NULL;
  size_t payload_len;
  int payload_type = DBUS_TYPE_STRING;
  DBusMessage *template = NULL;
  dbus_bool_t flood = FALSE;
  dbus_bool_t no_reply = FALSE;
  unsigned int messages_per_conn = 0;
  unsigned int seed = time (NULL);
  int n_random_sizes = 0;
  unsigned int *random_sizes = NULL;

  /* argv[1] is the tool name, so start from 2 */

  for (i = 2; i < argc; i++)
    {
      const char *arg = argv[i];

      if (payload != NULL &&
          (strstr (arg, "--payload=") == arg ||
           strcmp (arg, "--stdin") == 0 ||
           strcmp (arg, "--message-stdin") == 0 ||
           strcmp (arg, "--random-size") == 0))
        {
          fprintf (stderr, "At most one of --payload, --stdin, --message-stdin "
                           "and --random-size may be specified\n\n");
          usage (2);
        }

      if (strcmp (arg, "--system") == 0)
        {
          type = DBUS_BUS_SYSTEM;
        }
      else if (strcmp (arg, "--session") == 0)
        {
          type = DBUS_BUS_SESSION;
        }
      else if (strstr (arg, "--count=") == arg)
        {
          count = atoi (arg + strlen ("--count="));

          if (count < 1)
            usage (2);
        }
      else if (strcmp (arg, "--ignore-errors") == 0)
        {
          ignore_errors = TRUE;
        }
      else if (strstr (arg, "--dest=") == arg)
        {
          destination = arg + strlen ("--dest=");
        }
      else if (strstr (arg, "--payload=") == arg)
        {
          payload = arg + strlen ("--payload=");
        }
      else if (strcmp (arg, "--stdin") == 0)
        {
          consume_stdin (&payload_buf, &payload_len);
          payload = payload_buf;
        }
      else if (strcmp (arg, "--message-stdin") == 0)
        {
          consume_stdin (&payload_buf, &payload_len);
          payload = payload_buf;
          template = dbus_message_demarshal (payload, payload_len, &error);

          if (template == NULL)
            {
              fprintf (stderr, "Unable to demarshal template message: %s: %s",
                       error.name, error.message);
              exit (1);
            }

          if (dbus_message_get_type (template) != DBUS_MESSAGE_TYPE_METHOD_CALL)
            {
              fprintf (stderr, "Template message must be a method call\n");
              exit (1);
            }
        }
      else if (strcmp (arg, "--random-size") == 0)
        {
          unsigned int len, max = 0;
          int j, consumed = 0;
          const char *p;

          consume_stdin (&payload_buf, &payload_len);

          for (p = payload_buf; p < payload_buf + payload_len; p += consumed)
            {
              /* the space character matches any (or no) whitespace */
              if (sscanf (p, " %u %n", &len, &consumed) == 0)
                break;

              n_random_sizes++;
            }

          random_sizes = dbus_new0 (unsigned int, n_random_sizes);

          if (random_sizes == NULL)
            tool_oom ("allocating array of message lengths");

          for (p = payload_buf, j = 0;
              p < payload_buf + payload_len && j < n_random_sizes;
              p += consumed, j++)
            {
              sscanf (p, " %u %n", &len, &consumed);
              random_sizes[j] = len;

              if (len > max)
                max = len;
            }

          dbus_free (payload_buf);
          payload_len = max + 1;
          payload_buf = dbus_new (char, payload_len);
          payload = payload_buf;

          if (payload_buf == NULL)
            tool_oom ("allocating maximum-sized payload");

          memset (payload_buf, 'X', payload_len);
          payload_buf[payload_len - 1] = '\0';
        }
      else if (strcmp (arg, "--empty") == 0)
        {
          payload_type = DBUS_TYPE_INVALID;
        }
      else if (strcmp (arg, "--string") == 0)
        {
          payload_type = DBUS_TYPE_STRING;
        }
      else if (strcmp (arg, "--bytes") == 0)
        {
          payload_type = DBUS_TYPE_ARRAY;
        }
      else if (strcmp (arg, "--flood") == 0)
        {
          if (queue_len > 1)
            usage (2);

          if (messages_per_conn > 0)
            usage (2);

          flood = TRUE;
          queue_len = -1;
        }
      else if (strcmp (arg, "--no-reply") == 0)
        {
          if (queue_len > 1)
            usage (2);

          queue_len = -1;
          no_reply = TRUE;
        }
      else if (strstr (arg, "--queue=") == arg)
        {
          if (flood || no_reply)
            usage (2);

          queue_len = atoi (arg + strlen ("--queue="));

          if (queue_len < 1)
            usage (2);
        }
      else if (strstr (arg, "--seed=") == arg)
        {
          seed = strtoul (arg + strlen ("--seed="), NULL, 10);
        }
      else if (strstr (arg, "--messages-per-conn=") == arg)
        {
          messages_per_conn = atoi (arg + strlen ("--messages-per-conn="));

          if (messages_per_conn > 0 && flood)
            usage (2);
        }
      else
        {
          usage (2);
        }
    }

  srand (seed);

  if (payload == NULL)
    {
      payload = "hello, world!";
      payload_len = strlen (payload);
    }

  VERBOSE (stderr, "Will send up to %d messages, with up to %d queued, max %d per connection\n",
           count, queue_len, messages_per_conn);

  while (no_reply ? sent < count : received < count)
    {
      /* Connect?
       * - In the first iteration
       *  or
       * - When messages_per_conn messages have been sent and no replies are being waited for
       */
      if (connection == NULL ||
          (messages_per_conn > 0  && sent_in_this_conn == messages_per_conn &&
           (no_reply || received - received_before_this_conn == messages_per_conn)))
        {
          if (connection != NULL)
            {
              dbus_connection_flush (connection);
              dbus_connection_close (connection);
              dbus_connection_unref (connection);
            }

          VERBOSE (stderr, "New connection.\n");
          connection = dbus_bus_get_private (type, &error);

          if (connection == NULL)
            {
              fprintf (stderr, "Failed to connect to bus: %s: %s\n",
                       error.name, error.message);
              dbus_error_free (&error);
              dbus_free (random_sizes);
              dbus_free (payload_buf);
              return 1;
            }

          sent_in_this_conn = 0;
          received_before_this_conn = received;
        }

      /* Send another message? Only if we don't exceed the 3 limits:
       * - total amount of messages
       * - messages sent on this connection
       * - queue
       */
      while (sent < count &&
             (messages_per_conn == 0 || sent_in_this_conn < messages_per_conn) &&
             (queue_len == -1 || sent_in_this_conn < queue_len + received - received_before_this_conn))
        {
          DBusMessage *message;

          if (template != NULL)
            {
              message = dbus_message_copy (template);

              if (message == NULL)
                tool_oom ("copying message");

              dbus_message_set_no_reply (message, no_reply);
            }
          else
            {
              dbus_bool_t mem;
              unsigned int len = 0;

              message = dbus_message_new_method_call (destination,
                                                      "/",
                                                      "com.example",
                                                      "Spam");

              if (message == NULL)
                tool_oom ("allocating message");

              dbus_message_set_no_reply (message, no_reply);

              switch (payload_type)
                {
                  case DBUS_TYPE_STRING:
                    if (random_sizes != NULL)
                      {
                        /* this isn't fair, strictly speaking - the first few
                         * are a bit more likely to be chosen, unless
                         * RAND_MAX is divisible by n_random_sizes - but it's
                         * good enough for traffic-generation */
                        len = random_sizes[rand () % n_random_sizes];
                        payload_buf[len] = '\0';
                      }

                    mem = dbus_message_append_args (message,
                                                    DBUS_TYPE_STRING, &payload,
                                                    DBUS_TYPE_INVALID);

                    if (random_sizes != NULL)
                      {
                        /* undo the truncation above */
                        payload_buf[len] = 'X';
                      }

                    break;

                  case DBUS_TYPE_ARRAY:
                    len = payload_len;

                    /* as above, not strictly fair, but close enough */
                    if (random_sizes != NULL)
                      len = random_sizes[rand () % n_random_sizes];

                    mem = dbus_message_append_args (message,
                                                    DBUS_TYPE_ARRAY,
                                                      DBUS_TYPE_BYTE,
                                                      &payload,
                                                      (dbus_uint32_t) len,
                                                    DBUS_TYPE_INVALID);
                    break;

                  default:
                    mem = TRUE;
                }

              if (!mem)
                tool_oom ("building message");
            }

          if (no_reply)
            {
              if (!dbus_connection_send (connection, message, NULL))
                tool_oom ("sending message");

              VERBOSE (stderr, "sent message #%d\n", sent);
              sent++;
              sent_in_this_conn++;
            }
          else
            {
              DBusPendingCall *pc;

              if (!dbus_connection_send_with_reply (connection,
                                                    message,
                                                    &pc,
                                                    DBUS_TIMEOUT_INFINITE))
                tool_oom ("sending message");

              VERBOSE (stderr, "sent message #%d\n", sent);
              sent++;
              sent_in_this_conn++;

              if (pc == NULL)
                tool_oom ("sending message");

              if (dbus_pending_call_get_completed (pc))
                pc_notify (pc, &received);
              else if (!dbus_pending_call_set_notify (pc, pc_notify, &received,
                                                      NULL))
                tool_oom ("setting pending call notifier");

              dbus_pending_call_unref (pc);
            }

          dbus_message_unref (message);
        }

      if (!dbus_connection_read_write_dispatch (connection, -1))
        {
          fprintf (stderr, "Disconnected from bus\n");
          exit (1);
        }
    }

  if (connection != NULL)
    {
      dbus_connection_flush (connection);
      dbus_connection_close (connection);
      dbus_connection_unref (connection);
    }

  VERBOSE (stderr, "Done\n");

  dbus_free (payload_buf);
  dbus_free (random_sizes);

  if (template != NULL)
    dbus_message_unref (template);

  dbus_shutdown ();
  return 0;
}
