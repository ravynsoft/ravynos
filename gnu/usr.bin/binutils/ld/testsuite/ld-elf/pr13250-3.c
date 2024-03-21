#include <stdio.h>
#include <stdlib.h>

int common1[1];
char common2[2];

extern int bar ();

int
main ()
{
  int i;
  if (bar () != -1)
    abort ();
  if (common1[0] != -1)
    abort ();
  for (i = 0; i < sizeof (common2)/ sizeof (common2[0]); i++)
    if (common2[i] != 0)
      abort ();
  printf ("PASS\n");
  return 0;
}
