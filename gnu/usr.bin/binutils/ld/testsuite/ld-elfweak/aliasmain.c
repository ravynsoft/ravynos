#include <stdio.h>

extern int *psym1, *psym2;
extern int strongsym;

int
main (void)
{
  printf ("value via psym1: %d, via psym2: %d, strong %d\n",
	  *psym1, *psym2, strongsym);
  return 0;
}
