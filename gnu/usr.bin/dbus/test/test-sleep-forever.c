/* This is a process that just sleeps infinitely. */

#include <config.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef DBUS_WIN
# include <windows.h>
# define sleep Sleep
#endif


int
main (int argc, char **argv)
{
  while (1)
    sleep (10000000);

  return 1;
}
