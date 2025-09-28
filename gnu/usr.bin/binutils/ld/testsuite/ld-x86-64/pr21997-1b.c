#include <stdio.h>

extern int protected;
extern int get_protected (void);

int
main ()
{
  if (protected == get_protected ())
    printf ("PASS\n");

  return 0;
}
