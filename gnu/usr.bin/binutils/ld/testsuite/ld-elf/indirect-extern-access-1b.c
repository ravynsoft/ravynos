#include <stdio.h>

extern int indirect_extern_access;

int
main (void)
{
  if (indirect_extern_access == 1)
    puts ("PASS");

  return 0;
}
