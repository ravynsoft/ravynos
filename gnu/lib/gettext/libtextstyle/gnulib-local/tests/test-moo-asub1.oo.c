#include <config.h>

/* Specification.  */
#include "test-moo-asub1.h"

#pragma implementation

int asub1::write (asub1_t x, void *buf, size_t len)
{
  fwrite (buf, 1, len, x->fp);
  return 0;
}

void asub1::do_free (asub1_t x)
{
  free (x);
}
