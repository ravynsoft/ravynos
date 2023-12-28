#include <stdlib.h>
#include <stdio.h>

int foo = -1;

extern void bar ();

int
main (int argc, char **argv)
{
  bar ();
  printf ("OK\n");
  return 0;
}
