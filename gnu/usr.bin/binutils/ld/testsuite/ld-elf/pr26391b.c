#include <stdio.h>

static void
bar (void)
{
  printf ("bar 1\n");
}

void *
bar1_p (void)
{
  return bar;
}
