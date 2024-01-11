/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-monitor.c  Utility program to monitor messages on the bus
 *
 * Copyright (C) 2003 Philip Blundell <philb@gnu.org>
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

#include "dbus/dbus-connection-internal.h"
#include "dbus/dbus-internals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DBUS_WIN
#include <winsock2.h>
#undef interface
#else
#include <sys/time.h>
#endif

#include <time.h>

#include "dbus-print-message.h"
#include "tool-common.h"

#define EAVESDROPPING_RULE "eavesdrop=true"

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

/* http://www.tcpdump.org/linktypes.html */
#define LINKTYPE_DBUS 231

static DBusHandlerResult
monitor_filter_func (DBusConnection     *connection,
                     DBusMessage        *message,
                     void               *user_data)
{
  long sec = 0, usec = 0;

  _dbus_get_real_time (&sec, &usec);

  print_message (message, FALSE, sec, usec);
  
  if (dbus_message_is_signal (message,
                              DBUS_INTERFACE_LOCAL,
                              "Disconnected"))
    exit (0);

  /* Monitors must not allow libdbus to reply to messages, so we eat
   * the message. See bug 1719.
   */
  return DBUS_HANDLER_RESULT_HANDLED;
}

#define TRAP_NULL_STRING(str) ((str) ? (str) : "<none>")

typedef enum
{
  PROFILE_ATTRIBUTE_FLAG_SERIAL = 1,
  PROFILE_ATTRIBUTE_FLAG_REPLY_SERIAL = 2,
  PROFILE_ATTRIBUTE_FLAG_SENDER = 4,
  PROFILE_ATTRIBUTE_FLAG_DESTINATION = 8,
  PROFILE_ATTRIBUTE_FLAG_PATH = 16,
  PROFILE_ATTRIBUTE_FLAG_INTERFACE = 32,
  PROFILE_ATTRIBUTE_FLAG_MEMBER = 64,
  PROFILE_ATTRIBUTE_FLAG_ERROR_NAME = 128
} ProfileAttributeFlags;

static void
profile_print_headers (void)
{
  printf ("#type\ttimestamp\tserial\tsender\tdestination\tpath\tinterface\tmember\n");
  printf ("#\t\t\t\t\tin_reply_to\n");
}

static void
profile_print_with_attrs (const char *type, DBusMessage *message,
  long sec, long usec, ProfileAttributeFlags attrs)
{
  printf ("%s\t%ld.%06ld", type, sec, usec);

  if (attrs & PROFILE_ATTRIBUTE_FLAG_SERIAL)
    printf ("\t%u", dbus_message_get_serial (message));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_SENDER)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_sender (message)));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_DESTINATION)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_destination (message)));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_REPLY_SERIAL)
    printf ("\t%u", dbus_message_get_reply_serial (message));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_PATH)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_path (message)));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_INTERFACE)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_interface (message)));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_MEMBER)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_member (message)));

  if (attrs & PROFILE_ATTRIBUTE_FLAG_ERROR_NAME)
    printf ("\t%s", TRAP_NULL_STRING (dbus_message_get_error_name (message)));

  printf ("\n");
}

static void
print_message_profile (DBusMessage *message)
{
  static dbus_bool_t first = TRUE;
  long sec = 0, usec = 0;

  if (first)
    {
      profile_print_headers ();
      first = FALSE;
    }

  _dbus_get_real_time (&sec, &usec);

  switch (dbus_message_get_type (message))
    {
      case DBUS_MESSAGE_TYPE_METHOD_CALL:
        profile_print_with_attrs ("mc", message, sec, usec,
          PROFILE_ATTRIBUTE_FLAG_SERIAL |
          PROFILE_ATTRIBUTE_FLAG_SENDER |
          PROFILE_ATTRIBUTE_FLAG_DESTINATION |
          PROFILE_ATTRIBUTE_FLAG_PATH |
          PROFILE_ATTRIBUTE_FLAG_INTERFACE |
          PROFILE_ATTRIBUTE_FLAG_MEMBER);
        break;
      case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        profile_print_with_attrs ("mr", message, sec, usec,
          PROFILE_ATTRIBUTE_FLAG_SERIAL |
          PROFILE_ATTRIBUTE_FLAG_SENDER |
          PROFILE_ATTRIBUTE_FLAG_DESTINATION |
          PROFILE_ATTRIBUTE_FLAG_REPLY_SERIAL);
        break;
      case DBUS_MESSAGE_TYPE_ERROR:
        profile_print_with_attrs ("err", message, sec, usec,
          PROFILE_ATTRIBUTE_FLAG_SERIAL |
          PROFILE_ATTRIBUTE_FLAG_SENDER |
          PROFILE_ATTRIBUTE_FLAG_DESTINATION |
          PROFILE_ATTRIBUTE_FLAG_REPLY_SERIAL);
        break;
      case DBUS_MESSAGE_TYPE_SIGNAL:
        profile_print_with_attrs ("sig", message, sec, usec,
          PROFILE_ATTRIBUTE_FLAG_SERIAL |
          PROFILE_ATTRIBUTE_FLAG_SENDER |
          PROFILE_ATTRIBUTE_FLAG_DESTINATION |
          PROFILE_ATTRIBUTE_FLAG_PATH |
          PROFILE_ATTRIBUTE_FLAG_INTERFACE |
          PROFILE_ATTRIBUTE_FLAG_MEMBER);
        break;
      default:
        printf ("%s\t%ld.%06ld", "tun", sec, usec);
        break;
    }
}

static DBusHandlerResult
profile_filter_func (DBusConnection     *connection,
                     DBusMessage        *message,
                     void               *user_data)
{
  print_message_profile (message);

  if (dbus_message_is_signal (message,
                              DBUS_INTERFACE_LOCAL,
                              "Disconnected"))
    exit (0);

  return DBUS_HANDLER_RESULT_HANDLED;
}

typedef enum {
    BINARY_MODE_NOT,
    BINARY_MODE_RAW,
    BINARY_MODE_PCAP
} BinaryMode;

static DBusHandlerResult
binary_filter_func (DBusConnection *connection,
                    DBusMessage    *message,
                    void           *user_data)
{
  BinaryMode mode = _DBUS_POINTER_TO_INT (user_data);
  char *blob;
  int len;

  /* It would be nice if we could do a zero-copy "peek" one day, but libdbus
   * is so copy-happy that this isn't really a big deal.
   */
  if (!dbus_message_marshal (message, &blob, &len))
    tool_oom ("retrieving message");

  switch (mode)
    {
      case BINARY_MODE_PCAP:
          {
            long tv_sec, tv_usec;
            /* seconds, microseconds, bytes captured (possibly truncated),
             * original length.
             * http://wiki.wireshark.org/Development/LibpcapFileFormat
             */
            dbus_uint32_t header[4] = { 0, 0, len, len };

            /* If this gets padded then we'd need to write it out in pieces */
            _DBUS_STATIC_ASSERT (sizeof (header) == 16);

            _dbus_get_real_time (&tv_sec, &tv_usec);
            header[0] = tv_sec;
            header[1] = tv_usec;

            if (!tool_write_all (STDOUT_FILENO, header, sizeof (header)))
              {
                perror ("dbus-monitor: write");
                exit (1);
              }
          }
        break;

      case BINARY_MODE_NOT:
        _dbus_assert_not_reached ("wrong filter function");
        break;

      case BINARY_MODE_RAW:
      default:
        /* nothing special, just the raw message stream */
        break;
    }

  if (!tool_write_all (STDOUT_FILENO, blob, len))
    {
      perror ("dbus-monitor: write");
      exit (1);
    }

  dbus_free (blob);

  if (dbus_message_is_signal (message,
                              DBUS_INTERFACE_LOCAL,
                              "Disconnected"))
    exit (0);

  return DBUS_HANDLER_RESULT_HANDLED;
}

static void usage (char *name, int ecode) _DBUS_GNUC_NORETURN;

static void
usage (char *name, int ecode)
{
  fprintf (stderr, "Usage: %s [--system | --session | --address ADDRESS] [--monitor | --profile | --pcap | --binary ] [watch expressions]\n", name);
  exit (ecode);
}

static void
only_one_type (dbus_bool_t *seen_bus_type,
               char        *name)
{
  if (*seen_bus_type)
    {
      fprintf (stderr, "I only support monitoring one bus at a time!\n");
      usage (name, 1);
    }
  else
    {
      *seen_bus_type = TRUE;
    }
}

static dbus_bool_t
become_monitor (DBusConnection *connection,
    int numFilters,
    const char * const *filters)
{
  DBusError error = DBUS_ERROR_INIT;
  DBusMessage *m;
  DBusMessage *r;
  int i;
  dbus_uint32_t zero = 0;
  DBusMessageIter appender, array_appender;

  m = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
      DBUS_PATH_DBUS, DBUS_INTERFACE_MONITORING, "BecomeMonitor");

  if (m == NULL)
    tool_oom ("becoming a monitor");

  dbus_message_iter_init_append (m, &appender);

  if (!dbus_message_iter_open_container (&appender, DBUS_TYPE_ARRAY, "s",
        &array_appender))
    tool_oom ("opening string array");

  for (i = 0; i < numFilters; i++)
    {
      if (!dbus_message_iter_append_basic (&array_appender, DBUS_TYPE_STRING,
            &filters[i]))
        tool_oom ("adding filter to array");
    }

  if (!dbus_message_iter_close_container (&appender, &array_appender) ||
      !dbus_message_iter_append_basic (&appender, DBUS_TYPE_UINT32, &zero))
    tool_oom ("finishing arguments");

  r = dbus_connection_send_with_reply_and_block (connection, m, -1, &error);

  if (r != NULL)
    {
      dbus_message_unref (r);
    }
  else if (dbus_error_has_name (&error, DBUS_ERROR_UNKNOWN_INTERFACE))
    {
      fprintf (stderr, "dbus-monitor: unable to enable new-style monitoring, "
          "your dbus-daemon is too old. Falling back to eavesdropping.\n");
      dbus_error_free (&error);
    }
  else
    {
      fprintf (stderr, "dbus-monitor: unable to enable new-style monitoring: "
          "%s: \"%s\". Falling back to eavesdropping.\n",
          error.name, error.message);
      dbus_error_free (&error);
    }

  dbus_message_unref (m);

  return (r != NULL);
}

int
main (int argc, char *argv[])
{
  DBusConnection *connection;
  DBusError error;
  DBusBusType type = DBUS_BUS_SESSION;
  DBusHandleMessageFunction filter_func = monitor_filter_func;
  char *address = NULL;
  dbus_bool_t seen_bus_type = FALSE;
  BinaryMode binary_mode = BINARY_MODE_NOT;
  int i = 0, j = 0, numFilters = 0;
  char **filters = NULL;

  /* Set stdout to be unbuffered; this is basically so that if people
   * do dbus-monitor > file, then send SIGINT via Control-C, they
   * don't lose the last chunk of messages.
   */

#ifdef DBUS_WIN
  setvbuf (stdout, NULL, _IONBF, 0);
#else
  setvbuf (stdout, NULL, _IOLBF, 0);
#endif

  for (i = 1; i < argc; i++)
    {
      char *arg = argv[i];

      if (!strcmp (arg, "--system"))
        {
          only_one_type (&seen_bus_type, argv[0]);
          type = DBUS_BUS_SYSTEM;
        }
      else if (!strcmp (arg, "--session"))
        {
          only_one_type (&seen_bus_type, argv[0]);
          type = DBUS_BUS_SESSION;
        }
      else if (!strcmp (arg, "--address"))
        {
          only_one_type (&seen_bus_type, argv[0]);

          if (i+1 < argc)
            {
              address = argv[i+1];
              i++;
            }
          else
            usage (argv[0], 1);
        }
      else if (!strcmp (arg, "--help"))
        usage (argv[0], 0);
      else if (!strcmp (arg, "--monitor"))
        {
          filter_func = monitor_filter_func;
          binary_mode = BINARY_MODE_NOT;
        }
      else if (!strcmp (arg, "--profile"))
        {
          filter_func = profile_filter_func;
          binary_mode = BINARY_MODE_NOT;
        }
      else if (!strcmp (arg, "--binary"))
        {
          filter_func = binary_filter_func;
          binary_mode = BINARY_MODE_RAW;
        }
      else if (!strcmp (arg, "--pcap"))
        {
          filter_func = binary_filter_func;
          binary_mode = BINARY_MODE_PCAP;
        }
      else if (!strcmp (arg, "--"))
        continue;
      else if (arg[0] == '-')
        usage (argv[0], 1);
      else {
          unsigned int filter_len;
          numFilters++;
          /* Prepend a rule (and a comma) to enable the monitor to eavesdrop.
           * Prepending allows the user to add eavesdrop=false at command line
           * in order to disable eavesdropping when needed */
          filter_len = strlen (EAVESDROPPING_RULE) + 1 + strlen (arg) + 1;

          filters = (char **) realloc (filters, numFilters * sizeof (char *));
          if (filters == NULL)
            tool_oom ("adding a new filter slot");
          filters[j] = (char *) malloc (filter_len);
          if (filters[j] == NULL)
            tool_oom ("adding a new filter");
          snprintf (filters[j], filter_len, "%s,%s", EAVESDROPPING_RULE, arg);
          j++;
      }
    }

  dbus_error_init (&error);
  
  if (address != NULL)
    {
      connection = dbus_connection_open (address, &error);
      if (connection)
        {
          if (!dbus_bus_register (connection, &error))
            {
              fprintf (stderr, "Failed to register connection to bus at %s: %s\n",
                       address, error.message);
              dbus_error_free (&error);
              exit (1);
            }
        }
    }
  else
    connection = dbus_bus_get (type, &error);
  if (connection == NULL)
    {
      const char *where;
      if (address != NULL)
        where = address;
      else
        {
          switch (type)
            {
            case DBUS_BUS_SYSTEM:
              where = "system bus";
              break;
            case DBUS_BUS_SESSION:
              where = "session bus";
              break;
            case DBUS_BUS_STARTER:
            default:
              /* We don't set type to anything else */
              _dbus_assert_not_reached ("impossible bus type");
            }
        }
      fprintf (stderr, "Failed to open connection to %s: %s\n",
               where,
               error.message);
      dbus_error_free (&error);
      exit (1);
    }

  /* Receive o.fd.Peer messages as normal messages, rather than having
   * libdbus handle them internally, which is the wrong thing for
   * a monitor */
  _dbus_connection_set_builtin_filters_enabled (connection, FALSE);

  if (!dbus_connection_add_filter (connection, filter_func,
                                   _DBUS_INT_TO_POINTER (binary_mode), NULL))
    {
      fprintf (stderr, "Couldn't add filter!\n");
      exit (1);
    }

  if (become_monitor (connection, numFilters,
                      (const char * const *) filters))
    {
      /* no more preparation needed */
    }
  else if (numFilters)
    {
      size_t offset = 0;
      for (i = 0; i < j; i++)
        {
          dbus_bus_add_match (connection, filters[i] + offset, &error);
          if (dbus_error_is_set (&error) && i == 0 && offset == 0)
            {
              /* We might be talking to a pre-1.5.6 dbus-daemon
              * which wouldn't understand eavesdrop=true.
              * If this works, carry on with offset > 0
              * on the remaining iterations. */
              offset = strlen (EAVESDROPPING_RULE) + 1;
              dbus_error_free (&error);
              dbus_bus_add_match (connection, filters[i] + offset, &error);
            }

          if (dbus_error_is_set (&error))
            {
              fprintf (stderr, "Failed to setup match \"%s\": %s\n",
                       filters[i], error.message);
              dbus_error_free (&error);
              exit (1);
            }
          free(filters[i]);
        }
    }
  else
    {
      dbus_bus_add_match (connection,
                          EAVESDROPPING_RULE,
                          &error);
      if (dbus_error_is_set (&error))
        {
          dbus_error_free (&error);
          dbus_bus_add_match (connection,
                              "",
                              &error);
          if (dbus_error_is_set (&error))
            goto lose;
        }
    }

  switch (binary_mode)
    {
      case BINARY_MODE_NOT:
      case BINARY_MODE_RAW:
      default:
        /* no special header needed */
        break;

      case BINARY_MODE_PCAP:
          {
            /* We're not using libpcap because the file format is simple
             * enough not to need it.
             * http://wiki.wireshark.org/Development/LibpcapFileFormat */
            struct {
                dbus_uint32_t magic;
                dbus_uint16_t major_version;
                dbus_uint16_t minor_version;
                dbus_int32_t timezone;
                dbus_uint32_t precision;
                dbus_uint32_t max_length;
                dbus_uint32_t link_type;
            } header = {
                0xA1B2C3D4U,  /* magic number */
                2, 4,         /* v2.4 */
                0,            /* capture in GMT */
                0,            /* no opinion on timestamp precision */
                (1 << 27),    /* D-Bus spec says so */
                LINKTYPE_DBUS
            };

            /* Assert that there is no padding */
            _DBUS_STATIC_ASSERT (sizeof (header) == 24);

            if (!tool_write_all (STDOUT_FILENO, &header, sizeof (header)))
              {
                perror ("dbus-monitor: write");
                exit (1);
              }
          }
        break;
    }

  while (dbus_connection_read_write_dispatch(connection, -1))
    ;
  exit (0);
 lose:
  fprintf (stderr, "Error: %s\n", error.message);
  exit (1);
}
