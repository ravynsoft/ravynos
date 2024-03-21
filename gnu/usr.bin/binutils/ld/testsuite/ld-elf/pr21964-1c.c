#include <stdio.h>

extern int foo1 (void);
extern int foo2 (void);

extern int __start___verbose[];
extern int __stop___verbose[];
static int my_var __attribute__((used, section("__verbose"))) = 6;
int
bar (void)
{
  if (& __start___verbose[0] == & __stop___verbose[0])
    return -1;

  if (__start___verbose[0] != 6)
    return -2;
  else
    return 0;
}

int
main ()
{
  if (bar () == 0
      && foo1 () == 0
      && foo2 () == 0)
    printf ("PASS\n");
  return 0;
}
