#include <stdlib.h>
#include <stdio.h>

__thread int bar = 301;

extern int *test1 (int);
extern int *test2 (int);
extern int *test3 (int);

int
main ()
{
  int *p;
  p = test1 (30);
  if (*p != 30)
    abort ();
  *p = 40;
  test1 (40);
  p = test2 (301);
  if (*p != 301)
    abort ();
  if (p != &bar)
    abort ();
  *p = 40;
  test2 (40);
  p = test3 (40);
  if (*p != 40)
    abort ();
  *p = 50;
  test3 (50);
  puts ("PASS");
  return 0;
}
