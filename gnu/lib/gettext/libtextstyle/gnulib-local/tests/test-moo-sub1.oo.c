#include <config.h>

/* Specification.  */
#include "test-moo-sub1.h"

#pragma implementation

/* Test an override.  */
int sub1::write (sub1_t x, void *buf, size_t len)
{
  fwrite (buf, 1, len, x->fp);
  return 0;
}
