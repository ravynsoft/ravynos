/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-send.c  Utility program to send messages from the command line
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dbus/dbus.h>
#include "dbus/dbus-internals.h"

#ifndef HAVE_STRTOLL
#undef strtoll
#define strtoll mystrtoll
#include "strtoll.c"
#endif

#ifndef HAVE_STRTOULL
#undef strtoull
#define strtoull mystrtoull
#include "strtoull.c"
#endif

#ifdef DBUS_WINCE
#ifndef strdup
#define strdup _strdup
#endif
#endif

#include "dbus-print-message.h"

static const char *appname;

static void usage (int ecode) _DBUS_GNUC_NORETURN;

static void
usage (int ecode)
{
  fprintf (stderr, "Usage: %s [--help] [--system | --session | --bus=ADDRESS | --peer=ADDRESS] [--sender=NAME] [--dest=NAME] [--type=TYPE] [--print-reply[=literal]] [--reply-timeout=MSEC] <destination object path> <message name> [contents ...]\n", appname);
  exit (ecode);
}

/* Abort on any allocation failure; there is nothing else we can do. */
static void
handle_oom (dbus_bool_t success)
{
  if (!success)
    {
      fprintf (stderr, "%s: Ran out of memory\n", appname);
      exit (1);
    }
}

static void
append_arg (DBusMessageIter *iter, int type, const char *value)
{
  dbus_uint16_t uint16;
  dbus_int16_t int16;
  dbus_uint32_t uint32;
  dbus_int32_t int32;
  dbus_uint64_t uint64;
  dbus_int64_t int64;
  double d;
  unsigned char byte;
  dbus_bool_t v_BOOLEAN;
  dbus_bool_t ret;

  switch (type)
    {
    case DBUS_TYPE_BYTE:
      byte = strtoul (value, NULL, 0);
      ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_BYTE, &byte);
      break;

    case DBUS_TYPE_DOUBLE:
      d = strtod (value, NULL);
      ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_DOUBLE, &d);
      break;

    case DBUS_TYPE_INT16:
      int16 = strtol (value, NULL, 0);
      ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_INT16, &int16);
      break;

    case DBUS_TYPE_UINT16:
      uint16 = strtoul (value, NULL, 0);
      ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT16, &uint16);
      break;

    case DBUS_TYPE_INT32:
      int32 = strtol (value, NULL, 0);
      ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_INT32, &int32);
      break;

    case DBUS_TYPE_UINT32:
      uint32 = strtoul (value, NULL, 0);
      ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT32, &uint32);
      break;

    case DBUS_TYPE_INT64:
      int64 = strtoll (value, NULL, 0);
      ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_INT64, &int64);
      break;

    case DBUS_TYPE_UINT64:
      uint64 = strtoull (value, NULL, 0);
      ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT64, &uint64);
      break;

    case DBUS_TYPE_STRING:
      ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &value);
      break;

    case DBUS_TYPE_OBJECT_PATH:
      ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_OBJECT_PATH, &value);
      break;

    case DBUS_TYPE_BOOLEAN:
      if (strcmp (value, "true") == 0)
	{
	  v_BOOLEAN = TRUE;
	  ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_BOOLEAN, &v_BOOLEAN);
	}
      else if (strcmp (value, "false") == 0)
	{
	  v_BOOLEAN = FALSE;
	  ret = dbus_message_iter_append_basic (iter, DBUS_TYPE_BOOLEAN, &v_BOOLEAN);
	}
      else
	{
	  fprintf (stderr, "%s: Expected \"true\" or \"false\" instead of \"%s\"\n", appname, value);
	  exit (1);
	}
      break;

    default:
      fprintf (stderr, "%s: Unsupported data type %c\n", appname, (char) type);
      exit (1);
    }

  handle_oom (ret);
}

static void
append_array (DBusMessageIter *iter, int type, const char *value)
{
  const char *val;
  char *dupval = strdup (value);

  handle_oom (dupval != NULL);

  val = strtok (dupval, ",");
  while (val != NULL)
    {
      append_arg (iter, type, val);
      val = strtok (NULL, ",");
    }
  free (dupval);
}

static void
append_dict (DBusMessageIter *iter, int keytype, int valtype, const char *value)
{
  const char *val;
  char *dupval = strdup (value);

  handle_oom (dupval != NULL);

  val = strtok (dupval, ",");
  while (val != NULL)
    {
      DBusMessageIter subiter;
      
      handle_oom (dbus_message_iter_open_container (iter,
                                                    DBUS_TYPE_DICT_ENTRY,
                                                    NULL,
                                                    &subiter));

      append_arg (&subiter, keytype, val);
      val = strtok (NULL, ",");
      if (val == NULL)
	{
	  fprintf (stderr, "%s: Malformed dictionary\n", appname);
	  exit (1);
	}
      append_arg (&subiter, valtype, val);

      handle_oom (dbus_message_iter_close_container (iter, &subiter));
      val = strtok (NULL, ",");
    } 
  free (dupval);
}

static int
type_from_name (const char *arg)
{
  int type;
  if (!strcmp (arg, "string"))
    type = DBUS_TYPE_STRING;
  else if (!strcmp (arg, "int16"))
    type = DBUS_TYPE_INT16;
  else if (!strcmp (arg, "uint16"))
    type = DBUS_TYPE_UINT16;
  else if (!strcmp (arg, "int32"))
    type = DBUS_TYPE_INT32;
  else if (!strcmp (arg, "uint32"))
    type = DBUS_TYPE_UINT32;
  else if (!strcmp (arg, "int64"))
    type = DBUS_TYPE_INT64;
  else if (!strcmp (arg, "uint64"))
    type = DBUS_TYPE_UINT64;
  else if (!strcmp (arg, "double"))
    type = DBUS_TYPE_DOUBLE;
  else if (!strcmp (arg, "byte"))
    type = DBUS_TYPE_BYTE;
  else if (!strcmp (arg, "boolean"))
    type = DBUS_TYPE_BOOLEAN;
  else if (!strcmp (arg, "objpath"))
    type = DBUS_TYPE_OBJECT_PATH;
  else
    {
      fprintf (stderr, "%s: Unknown type \"%s\"\n", appname, arg);
      exit (1);
    }
  return type;
}

int
main (int argc, char *argv[])
{
  DBusConnection *connection;
  DBusError error;
  DBusMessage *message;
  dbus_bool_t print_reply;
  dbus_bool_t print_reply_literal;
  int reply_timeout;
  DBusMessageIter iter;
  int i;
  DBusBusType type = DBUS_BUS_SESSION;
  const char *dest = NULL;
  const char *name = NULL;
  const char *path = NULL;
  int message_type = DBUS_MESSAGE_TYPE_SIGNAL;
  const char *type_str = NULL;
  const char *address = NULL;
  const char *sender = NULL;
  int is_bus = FALSE;
  int session_or_system = FALSE;

  appname = argv[0];
  
  if (argc < 3)
    usage (1);

  print_reply = FALSE;
  print_reply_literal = FALSE;
  reply_timeout = -1;
  
  for (i = 1; i < argc && name == NULL; i++)
    {
      char *arg = argv[i];

      if (strcmp (arg, "--system") == 0)
        {
	  type = DBUS_BUS_SYSTEM;
          session_or_system = TRUE;
        }
      else if (strcmp (arg, "--session") == 0)
        {
	  type = DBUS_BUS_SESSION;
          session_or_system = TRUE;
        }
      else if ((strstr (arg, "--bus=") == arg) || (strstr (arg, "--peer=") == arg) || (strstr (arg, "--address=") == arg))
        {
          /* Check for peer first, to avoid the GCC -Wduplicated-branches
           * warning.
           */
          if (arg[2] == 'p') /* peer */
            {
              is_bus = FALSE;
            }
          else if (arg[2] == 'b') /* bus */
            {
              is_bus = TRUE;
            }
          else /* address; keeping backwards compatibility */
            {
              is_bus = FALSE;
            }

          address = strchr (arg, '=') + 1;

          if (address[0] == '\0')
            {
              fprintf (stderr, "\"--peer=\" and \"--bus=\" require an ADDRESS\n");
              usage (1);
            }
        }
      else if (strstr (arg, "--sender=") == arg)
        {
          sender = strchr (arg, '=') + 1;

          if (sender[0] == '\0')
            {
              fprintf (stderr, "\"--sender=\" requires a NAME\n");
              usage (1);
            }
        }
      else if (strncmp (arg, "--print-reply", 13) == 0)
	{
	  print_reply = TRUE;
	  message_type = DBUS_MESSAGE_TYPE_METHOD_CALL;
	  if (strcmp (arg + 13, "=literal") == 0)
	    print_reply_literal = TRUE;
	  else if (*(arg + 13) != '\0')
	    {
	      fprintf (stderr, "invalid value (%s) of \"--print-reply\"\n", arg + 13);
	      usage (1);
	    }
	}
      else if (strstr (arg, "--reply-timeout=") == arg)
	{
	  if (*(strchr (arg, '=') + 1) == '\0')
	    {
	      fprintf (stderr, "\"--reply-timeout=\" requires an MSEC\n");
	      usage (1);
	    }
	  reply_timeout = strtol (strchr (arg, '=') + 1,
				  NULL, 10);
	  if (reply_timeout <= 0)
	    {
	      fprintf (stderr, "invalid value (%s) of \"--reply-timeout\"\n",
	               strchr (arg, '=') + 1);
	      usage (1);
	    }
	}
      else if (strstr (arg, "--dest=") == arg)
	{
	  if (*(strchr (arg, '=') + 1) == '\0')
	    {
	      fprintf (stderr, "\"--dest=\" requires an NAME\n");
	      usage (1);
	    }
	  dest = strchr (arg, '=') + 1;
	}
      else if (strstr (arg, "--type=") == arg)
	type_str = strchr (arg, '=') + 1;
      else if (!strcmp(arg, "--help"))
	usage (0);
      else if (arg[0] == '-')
	usage (1);
      else if (path == NULL)
        path = arg;
      else /* name == NULL guaranteed by the 'while' loop */
        name = arg;
    }

  if (name == NULL)
    usage (1);

  if (session_or_system &&
      (address != NULL))
    {
      fprintf (stderr, "\"--peer\" and \"--bus\" may not be used with \"--system\" or \"--session\"\n");
      usage (1);
    }

  if (sender != NULL && address != NULL && !is_bus)
    {
      fprintf (stderr, "\"--peer\" may not be used with \"--sender\"\n");
      exit (1);
    }

  if (type_str != NULL)
    {
      message_type = dbus_message_type_from_string (type_str);
      if (!(message_type == DBUS_MESSAGE_TYPE_METHOD_CALL ||
            message_type == DBUS_MESSAGE_TYPE_SIGNAL))
        {
          fprintf (stderr, "Message type \"%s\" is not supported\n",
                   type_str);
          exit (1);
        }
    }

  dbus_error_init (&error);

  if (dest && !dbus_validate_bus_name (dest, &error))
    {
      fprintf (stderr, "invalid value (%s) of \"--dest\"\n", dest);
      dbus_error_free (&error);
      usage (1);
    }

  if (sender && !dbus_validate_bus_name (sender, &error))
    {
      fprintf (stderr, "invalid value (%s) of \"--sender\"\n", sender);
      dbus_error_free (&error);
      usage (1);
    }

  if (!dbus_validate_path (path, &error))
    {
      fprintf (stderr, "%s\n", error.message);
      dbus_error_free (&error);
      exit (1);
    }

  if (address != NULL)
    {
      connection = dbus_connection_open (address, &error);
    }
  else
    {
      connection = dbus_bus_get (type, &error);
    }

  if (connection == NULL)
    {
      fprintf (stderr, "Failed to open connection to \"%s\" message bus: %s\n",
               (address != NULL) ? address :
                 ((type == DBUS_BUS_SYSTEM) ? "system" : "session"),
               error.message);
      dbus_error_free (&error);
      exit (1);
    }
  else if ((address != NULL) && is_bus)
    {
      if (!dbus_bus_register (connection, &error))
        {
          fprintf (stderr, "Failed to register on connection to \"%s\" message bus: %s\n",
                   address, error.message);
          dbus_error_free (&error);
          exit (1);
        }
    }

  if (sender != NULL)
    {
      int ret = dbus_bus_request_name (connection, sender, DBUS_NAME_FLAG_DO_NOT_QUEUE, &error);
      switch (ret)
        {
        case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
          /* success */
          break;
        case DBUS_REQUEST_NAME_REPLY_EXISTS:
          fprintf (stderr, "Requested name \"%s\" already has owner\n", sender);
          exit (1);
        case -1:
          fprintf (stderr, "Failed to request sender name \"%s\": %s\n", sender, error.message);
          dbus_error_free (&error);
          exit (1);
        default:
          /* This should be unreachable if the bus is compliant */
          fprintf (stderr, "Failed to request sender name \"%s\": unexpected result code %d\n",
                   sender, ret);
          exit (1);
        }
    }

  if (message_type == DBUS_MESSAGE_TYPE_METHOD_CALL)
    {
      char *last_dot;

      last_dot = strrchr (name, '.');
      if (last_dot == NULL)
        {
          fprintf (stderr, "Must use org.mydomain.Interface.Method notation, no dot in \"%s\"\n",
                   name);
          exit (1);
        }
      *last_dot = '\0';

      if (!dbus_validate_interface (name, &error))
        {
          /* Typically this is "Interface name was not valid: \"xxx\""
           * so we don't need to prefix anything special */
          fprintf (stderr, "%s\n", error.message);
          dbus_error_free (&error);
          exit (1);
        }

      if (!dbus_validate_member (last_dot + 1, &error))
        {
          fprintf (stderr, "Invalid method name: %s\n", error.message);
          dbus_error_free (&error);
          exit (1);
        }

      message = dbus_message_new_method_call (NULL,
                                              path,
                                              name,
                                              last_dot + 1);
      handle_oom (message != NULL);
      dbus_message_set_auto_start (message, TRUE);
    }
  else if (message_type == DBUS_MESSAGE_TYPE_SIGNAL)
    {
      char *last_dot;

      last_dot = strrchr (name, '.');
      if (last_dot == NULL)
        {
          fprintf (stderr, "Must use org.mydomain.Interface.Signal notation, no dot in \"%s\"\n",
                   name);
          exit (1);
        }
      *last_dot = '\0';

      if (!dbus_validate_interface (name, &error))
        {
          fprintf (stderr, "%s\n", error.message);
          dbus_error_free (&error);
          exit (1);
        }

      if (!dbus_validate_member (last_dot + 1, &error))
        {
          fprintf (stderr, "Invalid signal name: %s\n", error.message);
          dbus_error_free (&error);
          exit (1);
        }

      message = dbus_message_new_signal (path, name, last_dot + 1);
      handle_oom (message != NULL);
    }
  else
    {
      fprintf (stderr, "Internal error, unknown message type\n");
      exit (1);
    }

  if (message == NULL)
    {
      fprintf (stderr, "Couldn't allocate D-Bus message\n");
      exit (1);
    }

  if (dest && !dbus_message_set_destination (message, dest))
    {
      fprintf (stderr, "Not enough memory\n");
      exit (1);
    }
  
  dbus_message_iter_init_append (message, &iter);

  while (i < argc)
    {
      char *arg;
      char *c;
      int type2;
      int secondary_type;
      int container_type;
      DBusMessageIter *target_iter;
      DBusMessageIter container_iter;

      type2 = DBUS_TYPE_INVALID;
      secondary_type = DBUS_TYPE_INVALID;
      arg = argv[i++];
      c = strchr (arg, ':');

      if (c == NULL)
	{
	  fprintf (stderr, "%s: Data item \"%s\" is badly formed\n", argv[0], arg);
	  exit (1);
	}

      *(c++) = 0;

      container_type = DBUS_TYPE_INVALID;

      if (strcmp (arg, "variant") == 0)
	container_type = DBUS_TYPE_VARIANT;
      else if (strcmp (arg, "array") == 0)
	container_type = DBUS_TYPE_ARRAY;
      else if (strcmp (arg, "dict") == 0)
	container_type = DBUS_TYPE_DICT_ENTRY;

      if (container_type != DBUS_TYPE_INVALID)
	{
	  arg = c;
	  c = strchr (arg, ':');
	  if (c == NULL)
	    {
	      fprintf (stderr, "%s: Data item \"%s\" is badly formed\n", argv[0], arg);
	      exit (1);
	    }
	  *(c++) = 0;
	}

      if (arg[0] == 0)
	type2 = DBUS_TYPE_STRING;
      else
	type2 = type_from_name (arg);

      if (container_type == DBUS_TYPE_DICT_ENTRY)
	{
	  char sig[5];
	  arg = c;
	  c = strchr (c, ':');
	  if (c == NULL)
	    {
	      fprintf (stderr, "%s: Data item \"%s\" is badly formed\n", argv[0], arg);
	      exit (1);
	    }
	  *(c++) = 0;
	  secondary_type = type_from_name (arg);
	  sig[0] = DBUS_DICT_ENTRY_BEGIN_CHAR;
	  sig[1] = type2;
	  sig[2] = secondary_type;
	  sig[3] = DBUS_DICT_ENTRY_END_CHAR;
	  sig[4] = '\0';
          handle_oom (dbus_message_iter_open_container (&iter,
                                                        DBUS_TYPE_ARRAY,
                                                        sig,
                                                        &container_iter));
	  target_iter = &container_iter;
	}
      else if (container_type != DBUS_TYPE_INVALID)
	{
	  char sig[2];
	  sig[0] = type2;
	  sig[1] = '\0';
          handle_oom (dbus_message_iter_open_container (&iter,
                                                        container_type,
                                                        sig,
                                                        &container_iter));
	  target_iter = &container_iter;
	}
      else
	target_iter = &iter;

      if (container_type == DBUS_TYPE_ARRAY)
	{
	  append_array (target_iter, type2, c);
	}
      else if (container_type == DBUS_TYPE_DICT_ENTRY)
	{
	  _dbus_assert (secondary_type != DBUS_TYPE_INVALID);
	  append_dict (target_iter, type2, secondary_type, c);
	}
      else
	append_arg (target_iter, type2, c);

      if (container_type != DBUS_TYPE_INVALID)
	{
          handle_oom (dbus_message_iter_close_container (&iter,
                                                         &container_iter));
	}
    }

  if (print_reply)
    {
      DBusMessage *reply;

      dbus_error_init (&error);
      reply = dbus_connection_send_with_reply_and_block (connection,
                                                         message, reply_timeout,
                                                         &error);
      if (dbus_error_is_set (&error))
        {
          fprintf (stderr, "Error %s: %s\n",
		   error.name,
                   error.message);
          exit (1);
        }

      if (reply)
        {
          long sec, usec;

          _dbus_get_real_time (&sec, &usec);
          print_message (reply, print_reply_literal, sec, usec);
          dbus_message_unref (reply);
        }
    }
  else
    {
      dbus_connection_send (connection, message, NULL);
      dbus_connection_flush (connection);
    }

  dbus_message_unref (message);

  dbus_connection_unref (connection);

  exit (0);
}
