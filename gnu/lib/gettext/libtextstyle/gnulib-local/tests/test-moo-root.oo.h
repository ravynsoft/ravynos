#include <stdlib.h>
#include "moo.h"

/* Define a root class. */
struct root
{
methods:
  int write (root_t x, void *buf, size_t len);
  void do_free (root_t x);
};
