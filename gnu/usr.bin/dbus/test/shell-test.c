#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include <dbus/dbus-internals.h>
#include <dbus/dbus-list.h>
#include <dbus/dbus-memory.h>
#include <dbus/dbus-shell.h>
#include <dbus/dbus-string.h>
#include <dbus/dbus-sysdeps.h>

static int test_num = 0;
static int num_failed = 0;

static dbus_bool_t
test_command_line_internal (dbus_bool_t should_work,
    const char *arg1,
    va_list var_args)
{
  int i, original_argc, shell_argc;
  char **shell_argv;
  char **original_argv;
  char *command_line, *tmp;
  DBusString str;
  DBusList *list = NULL, *node;
  DBusError error;

  if (!_dbus_list_append (&list, (char *)arg1))
    return FALSE;

  do
    {
      tmp = va_arg (var_args, char *);
      if (!tmp)
        break;
      if (!_dbus_list_append (&list, tmp))
        {
          _dbus_list_clear (&list);
          return FALSE;
        }
    } while (tmp);

  original_argc = _dbus_list_get_length (&list);
  original_argv = dbus_new (char *, original_argc);
  if (!_dbus_string_init (&str))
    {
      _dbus_list_clear (&list);
      dbus_free (original_argv);
      return FALSE;
    }

  for (i = 0, node = _dbus_list_get_first_link (&list); i < original_argc && node;
       i++, node = _dbus_list_get_next_link (&list, node))
    {
      original_argv[i] = node->data;
      if ((i > 0 && !_dbus_string_append_byte (&str, ' ')) ||
          !_dbus_string_append (&str, original_argv[i]))
        {
          _dbus_list_clear (&list);
          dbus_free (original_argv);
          _dbus_string_free (&str);
          return FALSE;
        }
    }

  _dbus_list_clear (&list);
  command_line = _dbus_string_get_data (&str);
  fprintf (stderr, "# Testing command line '%s'\n", command_line);

  dbus_error_init (&error);
  if (!_dbus_shell_parse_argv (command_line, &shell_argc, &shell_argv, &error))
    {
      fprintf (stderr, "# Error%s parsing command line: %s\n",
          should_work ? "" : " (as expected)",
          error.message ? error.message : "");
      dbus_free (original_argv);
      _dbus_string_free (&str);
      return !should_work;
    }
  else
    {
      if (shell_argc != original_argc)
        {
          fprintf (stderr, "# Number of arguments returned (%d) don't match original (%d)\n",
                  shell_argc, original_argc);
          dbus_free (original_argv);
          dbus_free_string_array (shell_argv);
          return FALSE;
        }
      fprintf (stderr, "# Number of arguments: %d\n", shell_argc);
      for (i = 0; i < shell_argc; i++)
        {
          char *unquoted;

          unquoted = _dbus_shell_unquote (original_argv[i]);
          if (strcmp (unquoted ? unquoted : "",
                      shell_argv[i] ? shell_argv[i] : ""))
            {
              fprintf (stderr, "Position %d, returned argument (%s) does not match original (%s)\n",
                      i, shell_argv[i], unquoted);
              dbus_free (unquoted);
              dbus_free (original_argv);
              dbus_free_string_array (shell_argv);
              return FALSE;
            }
          dbus_free (unquoted);
          if (shell_argv[i])
            fprintf (stderr, "# Argument %d = %s\n", i, shell_argv[i]);
        }

      dbus_free_string_array (shell_argv);
    }

  _dbus_string_free (&str);
  dbus_free (original_argv);

  if (!should_work)
    {
      fprintf (stderr, "# Expected an error\n");
      return FALSE;
    }

  return TRUE;
}

static void
test_command_line (const char *arg1, ...)
{
  va_list var_args;

  va_start (var_args, arg1);

  if (test_command_line_internal (TRUE, arg1, var_args))
    {
      printf ("ok %d\n", ++test_num);
    }
  else
    {
      printf ("not ok %d\n", ++test_num);
      num_failed++;
    }

  va_end (var_args);
}

static void
test_command_line_fails (const char *arg1, ...)
{
  va_list var_args;

  va_start (var_args, arg1);

  if (test_command_line_internal (FALSE, arg1, var_args))
    {
      printf ("ok %d\n", ++test_num);
    }
  else
    {
      printf ("not ok %d\n", ++test_num);
      num_failed++;
    }

  va_end (var_args);
}

/* This test outputs TAP syntax: http://testanything.org/ */
int
main (int argc, char **argv)
{
  test_command_line ("command", "-s", "--force-shutdown", "\"a string\"", "123", NULL);
  test_command_line ("command", "-s", NULL);
  test_command_line ("/opt/gnome/bin/service-start", NULL);
  test_command_line ("grep", "-l", "-r", "-i", "'whatever'", "files*.c", NULL);
  test_command_line ("/home/boston/johnp/devel-local/dbus/test/test-segfault", NULL);
  test_command_line ("ls", "-l", "-a", "--colors", NULL);
  test_command_line ("rsync-to-server", NULL);
  test_command_line ("test-segfault", "--no-segfault", NULL);
  test_command_line ("evolution", "mailto:pepe@cuco.com", NULL);
  test_command_line ("run", "\"a \n multiline\"", NULL);
  test_command_line_fails ("ls", "\"a wrong string'", NULL);

  /* Tell the TAP driver that we have done all the tests we plan to do.
   * This is how it can distinguish between an unexpected exit and
   * successful completion. */
  printf ("1..%d\n", test_num);

  dbus_shutdown ();
  return (num_failed != 0);
}
