#include <stdio.h>

int protected = 42;
extern int *get_protected_ptr (void);

void
test()
{
  if (&protected == get_protected_ptr ())
    printf ("PASS\n");
}
