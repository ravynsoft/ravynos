#include <stdio.h>

__thread char bar[10];
__thread char foo[20] = { 1 } ;
extern int bar_size1 (void);
extern int bar_size2 (void);
extern int foo_size1 (void);
extern int foo_size2 (void);

int
main ()
{
  int size;
  
  size = bar_size1 ();
  if (bar[2] == 3 && size == sizeof (bar) && bar_size2 () == size)
    printf ("OK\n");

  size = foo_size1 ();
  if (foo[3] == 4 && size == sizeof (foo) && foo_size2 () == size)
    printf ("OK\n");

  return 0;
}
