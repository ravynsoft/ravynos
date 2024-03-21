#include <stdlib.h>
#include "moo.h"

/* Define an abstract root class. */
struct aroot
{
methods:
  int write (aroot_t x, void *buf, size_t len);
  void do_free (aroot_t x);
};
