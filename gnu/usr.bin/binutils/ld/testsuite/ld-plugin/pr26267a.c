#include <stdio.h>

int counter;
extern void foo (void);
extern void xxx (void);

void
bar (void)
{
}

int
main(void)
{
  bar ();
  foo ();
  xxx ();
  if (counter == 1)
    printf ("PASS\n");
  return 0;
}
