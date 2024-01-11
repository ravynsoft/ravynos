/* This is simply a process that segfaults */
#include <config.h>
#include <stdlib.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "disable-crash-handling.h"

int
main (int argc, char **argv)
{
  char *p;  

  _dbus_disable_crash_handling ();

#ifdef HAVE_RAISE
  raise (SIGSEGV);
#endif
  p = NULL;
  *p = 'a';
  
  return 0;
}
