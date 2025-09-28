#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

struct s { int n; double d[]; };

int main(void)
{
  int m = getchar ();
  struct s *p = malloc (offsetof (struct s, d)
                        + m * sizeof (double));
  p->d[0] = 0.0;
  return p->d != (double *) NULL;
}
