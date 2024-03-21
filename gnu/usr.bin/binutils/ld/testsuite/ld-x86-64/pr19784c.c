#include <stdio.h>

extern void abort (void);
extern int foo (int) __attribute__ ((visibility("hidden")));

void bar(void)
{
  if (foo (5) != 5)
    abort ();
  printf("PASS\n");
}
