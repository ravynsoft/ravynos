#include <stdio.h>

char bar[10];
char foo[20] = { 1 } ;
extern int bar_size1 (void);
extern int bar_size2 (void);
extern int foo_size1 (void);
extern int foo_size2 (void);

int
main ()
{
  int size;
  
  size = bar_size1 ();
  if (size == sizeof (bar) && bar_size2 () == size)
    printf ("OK\n");

  size = foo_size1 ();
  if (size == sizeof (foo) && foo_size2 () == size)
    printf ("OK\n");

  return 0;
}
