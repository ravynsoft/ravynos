#include <stdio.h>

void
foo (void)
{
  printf ("MAIN\n");
}

__asm__ (".symver foo,foo@FOO");
#ifdef __alpha__
__asm__ ("foo_alias = foo");
#else
__asm__ (".set foo_alias,foo");
#endif
__asm__ (".global foo_alias");
#if defined __powerpc64__ && defined _CALL_AIXDESC && !defined _CALL_LINUX
__asm__ (".symver .foo,.foo@FOO");
__asm__ (".set .foo_alias,.foo");
__asm__ (".global .foo_alias");
#endif
