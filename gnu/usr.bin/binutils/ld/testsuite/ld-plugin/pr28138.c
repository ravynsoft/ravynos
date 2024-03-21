#include <stdio.h>

extern int a7(void);

int
a0(void)
{
  return 0;
}

int
main()
{
  if (a7() == 7)
    {
      printf ("PASS\n");
      return 0;
    }
  return 1;
}
