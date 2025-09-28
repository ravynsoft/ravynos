#include <config.h>

/* Specification.  */
#include "test-moo-root.h"

#include <stdio.h>

#pragma implementation

int root::write (root_t x, void *buf, size_t len)
{
  fwrite (buf, 1, len, stdout);
  return 0;
}

void root::do_free (root_t x)
{
  free (x);
}
