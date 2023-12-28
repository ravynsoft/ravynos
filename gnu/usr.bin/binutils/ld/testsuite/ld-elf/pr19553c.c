#include <stdio.h>

void
foo (void)
{
  printf ("pr19553c\n");
}

__asm__ (".symver foo,foo@FOO");
#if defined __powerpc64__ && defined _CALL_AIXDESC && !defined _CALL_LINUX
__asm__ (".symver .foo,.foo@FOO");
#endif
