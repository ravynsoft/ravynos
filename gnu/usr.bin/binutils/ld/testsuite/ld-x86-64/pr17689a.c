#include <stdio.h>

char *bar = "PASS";
extern char *bar_alias __attribute__ ((weak, alias ("bar")));

void
foo (char *x)
{
  printf ("%s\n", x);
}
