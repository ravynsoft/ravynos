#include <stdio.h>

static void
bar (void)
{
  printf ("bar 3\n");
}

void *
bar3_p (void)
{
  return bar;
}
