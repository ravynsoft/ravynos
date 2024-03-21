#include <stdio.h>

#ifdef HAS_INT128
volatile __int128 a = 42;
volatile __int128 b = 1;
#else
volatile long long a = 42;
volatile long long b = 1;
#endif

int
main (void)
{
  if (((int) (a / b)) == 42)
    printf ("PASS\n");
  return 0;
}
