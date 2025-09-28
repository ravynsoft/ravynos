#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern char bss_in_mbind0;
extern char data_in_mbind3;

int
main (void)
{
  if (((intptr_t) &bss_in_mbind0 & (0x4000 - 1)) != 0)
    abort ();
  if (((intptr_t) &data_in_mbind3 & (0x4000 - 1)) != 0)
    abort ();
  printf ("PASS\n");
  return 0;
}
