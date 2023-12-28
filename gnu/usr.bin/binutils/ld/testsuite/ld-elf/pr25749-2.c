#include <stdio.h>
#include <stdint.h>

extern intptr_t size;

int
main ()
{
  if (size == 289 /* size of this file */)
    printf ("PASS\n");
#ifdef __GLIBC_PREREQ
# if !__GLIBC_PREREQ (2, 28)
  else
    printf ("PASS (incorrect result due to glibc bug)\n");
# endif
#endif
  return 0;
}
