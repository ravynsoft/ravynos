#include "vers.h"

void
foo ()
{
}

FUNC_SYMVER(foo, foo@VERS.0);
