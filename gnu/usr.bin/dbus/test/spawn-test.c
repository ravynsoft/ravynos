#include <config.h>
#include <dbus/dbus.h>

#include <dbus/dbus-sysdeps.h>
#include <dbus/dbus-spawn.h>
#include <stdio.h>

static void
setup_func (void *data)
{
  printf ("entering setup func.\n");
}

int
main (int argc, char **argv)
{
  char **argv_copy;
  int i;
  DBusError error = DBUS_ERROR_INIT;
  
  if (argc < 2)
    {
      fprintf (stderr, "You need to specify a program to launch.\n");

      return -1;
    }

  argv_copy = dbus_new (char *, argc);
  for (i = 0; i < argc - 1; i++)
    argv_copy [i] = argv[i + 1];
  argv_copy[argc - 1] = NULL;
  
  if (!_dbus_spawn_async_with_babysitter (NULL, argv_copy[0], argv_copy, NULL,
                                          DBUS_SPAWN_NONE, setup_func, NULL,
                                          &error))
    {
      fprintf (stderr, "Could not launch application: \"%s\"\n",
	       error.message);
    }

  dbus_free(argv_copy);
  dbus_shutdown ();
  return 0;
}
