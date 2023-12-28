#include <stdio.h>

extern int protected;
extern int get_protected (void);
extern int* get_protected_p (void);

int
main ()
{

  if (protected == get_protected ()
      && &protected == get_protected_p ())
    printf ("PASS\n");

  return 0;
}
