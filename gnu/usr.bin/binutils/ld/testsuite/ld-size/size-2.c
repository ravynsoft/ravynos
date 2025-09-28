#include <stdio.h>

extern int bar_size;
extern char *get_bar (int, int);

int
main ()
{
  char *bar = get_bar (2, 20);
  if (bar_size == 10 && bar[2] == 20)
    printf ("OK\n");

  return 0;
}
