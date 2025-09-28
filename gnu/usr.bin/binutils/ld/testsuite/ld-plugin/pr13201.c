#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
  double x;
  if (argc > 1)
    x = atof (argv[1]);
  else
    x = 3;
  x = sin (x);
  if (x > 0)
    printf("OK\n");
  return 0;
}
