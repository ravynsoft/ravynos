#include <config.h>

/* Specification.  */
#include "test-moo-sub2.h"

#include <stdio.h>

#pragma implementation

void
sub2::begin_indent (sub2_t x)
{
  x->indent++;
}

void
sub2::end_indent (sub2_t x)
{
  x->indent--;
}
