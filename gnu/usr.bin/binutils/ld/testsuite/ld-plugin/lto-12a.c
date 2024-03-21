#include <stdio.h>
#include <stdlib.h>

extern int value;

int
main (int argc, char **argv)
{
  int n = 10 * (argc + 1);
  char *p = malloc (n);
  __builtin_memcpy (p, argv[0], n);
  if (value != -1)
    abort ();
  printf ("OK\n");
  return 0;
}
