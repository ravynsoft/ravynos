#include <stdio.h>

static void
bar (void)
{
  printf ("bar 2\n");
}

void *
bar2_p (void)
{
  return bar;
}
