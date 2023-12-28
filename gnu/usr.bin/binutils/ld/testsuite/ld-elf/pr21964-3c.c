#include <stdio.h>

extern int foo1 (void);
extern int foo2 (void);
extern int foo3 (void);

static int my_var __attribute__((used, section("__verbose"))) = 6;

int
main ()
{
  if (foo1 () == 0
      && foo2 () == 0
      && foo3 () == 0)
    printf ("PASS\n");
  return 0;
}
