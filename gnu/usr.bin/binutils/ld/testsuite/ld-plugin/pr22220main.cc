#include <stdio.h>
#include "pr22220.h"

int main()
{
  if (boo() == goo())
    {
      printf ("PASS\n");
      return 0;
    }
  return 1;
}
