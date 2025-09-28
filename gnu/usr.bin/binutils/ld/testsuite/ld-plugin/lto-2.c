#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int
main (int argc, char **argv)
{
  int d = atoi (argv[1]);
  printf ("%f\n", sin (d));
  return 0;
}
