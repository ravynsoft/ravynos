/* Test whether .symver x, x@foo
   causes relocations against x within the same shared library
   to become dynamic relocations against x@foo.  */
#include "vers.h"

int x = 12;
SYMVER(x, x@VERS.0);
