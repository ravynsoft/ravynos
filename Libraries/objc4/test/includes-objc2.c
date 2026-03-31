// TEST_CFLAGS -D__OBJC2__

// Verify that all headers can be included in any language, even if
// the client is C code that defined __OBJC2__.

// This is the definition that Instruments uses in its build.
#if defined(__OBJC2__)
#undef __OBJC2__
#endif
#define __OBJC2__ 1

#define NAME "includes-objc2.c"
#include "includes.c"
