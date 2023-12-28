#include <stdio.h>

#include "protected-func-1.h"

int
protected_func_1b (void)
{
  return 3;
}

int
main (void)
{
  int res = 0;

  /* Check if we get the same address for the protected function symbol.  */
  if (protected_func_1a != protected_func_1a_p ())
    {
      puts ("'protected_func_1a' in main and shared library doesn't have same address");
      res = 1;
    }

  /* Check if we get the different addresses for the protected function
     symbol.  */
  if (protected_func_1b == protected_func_1b_p ())
    {
      puts ("'protected_func_1b' in main and shared library has same address");
      res = 1;
    }

  if (!res)
    puts ("PASS");

  return res;
}
