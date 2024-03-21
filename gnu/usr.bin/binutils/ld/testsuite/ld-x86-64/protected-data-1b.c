#include <stdio.h>

#include "protected-data-1.h"

int protected_data_1b = 3;

int
main (void)
{
  int res = 0;

  /* Check if we get the same address for the protected data symbol.  */
  if (&protected_data_1a != protected_data_1a_p ())
    {
      puts ("'protected_data_1a' in main and shared library doesn't have same address");
      res = 1;
    }

  protected_data_1a = -1;
  if (check_protected_data_1a (-1))
    {
      puts ("'protected_data_1a' in main and shared library doesn't have same value");
      res = 1;
    }

  set_protected_data_1a (-3);
  if (protected_data_1a != -3)
    {
      puts ("'protected_data_1a' in main and shared library doesn't have same value");
      res = 1;
    }

  /* Check if we get the different addresses for the protected data
     symbol.  */
  if (&protected_data_1b == protected_data_1b_p ())
    {
      puts ("'protected_data_1b' in main and shared library has same address");
      res = 1;
    }

  protected_data_1b = -10;
  if (check_protected_data_1b (2))
    {
      puts ("'protected_data_1b' in main and shared library has same address");
      res = 1;
    }

  set_protected_data_1b (-30);
  if (protected_data_1b != -10)
    {
      puts ("'protected_data_1b' in main and shared library has same address");
      res = 1;
    }

  if (!res)
    puts ("PASS");

  return res;
}
