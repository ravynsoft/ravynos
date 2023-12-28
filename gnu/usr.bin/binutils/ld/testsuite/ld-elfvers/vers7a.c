/*
 * Test supplied by Ulrich.  Verify that we can correctly force 'a'
 * to local scope.
 */
#include "vers.h"

int
__a_internal (int e)
{
  return e + 10;
}

int
__b_internal (int e)
{
  return e + 42;
}

FUNC_SYMVER(__a_internal, hide_a@@VERS_1);
FUNC_SYMVER(__b_internal, show_b@@VERS_1);
