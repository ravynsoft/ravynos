#include <stdio.h>
#include <stdlib.h>

void
test (void)
{
  static int count;
  if (count)
    printf("PASS\n");
  count++;
}

void
foo (void (*bar) (void))
{
  if (bar != test)
    abort ();
  bar ();
  test ();
}
