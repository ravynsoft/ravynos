#include <stdio.h>

int global_var;
extern void abort ();

int main(void)
{
  if (global_var != 20)
    abort ();
  printf ("PASS\n");
  return 0;
}
