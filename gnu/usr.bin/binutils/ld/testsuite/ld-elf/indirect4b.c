#include <stdio.h>

void
foo2 (void)
{
  printf ("MAIN2\n");
}

__asm__ (".symver foo2,foo@@FOO2");
#if defined __powerpc64__ && defined _CALL_AIXDESC && !defined _CALL_LINUX
__asm__ (".symver .foo2,.foo@@FOO2");
#endif

void
foo1 (void)
{
  printf ("MAIN1\n");
}

__asm__ (".symver foo1,foo@FOO1");
#if defined __powerpc64__ && defined _CALL_AIXDESC && !defined _CALL_LINUX
__asm__ (".symver .foo1,.foo@FOO1");
#endif
