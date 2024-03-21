#include <stdio.h>
#include <stdlib.h>

extern int * get_gd (void);
extern void set_gd (int);
extern int test_gd (int);
extern int * get_ld (void);
extern void set_ld (int);
extern int test_ld (int);

int
main ()
{
  int *p;
 
  p = get_gd ();
  set_gd (3);
  if (*p != 3 || !test_gd (3))
    abort ();

  p = get_ld ();
  set_ld (4);
  if (*p != 4 || !test_ld (4))
    abort ();

  printf ("PASS\n");

  return 0;
}
