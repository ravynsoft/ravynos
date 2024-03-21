#include <stdio.h>

extern int copy;
extern int* get_copy_p (void);

int main()
{
  if (&copy == get_copy_p ())
    printf ("PASS\n");

  return 0;
}
