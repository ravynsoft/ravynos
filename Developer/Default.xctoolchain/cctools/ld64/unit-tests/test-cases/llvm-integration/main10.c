#include "b10.h"

int main()
{
  struct my_struct *mh = &my_hooks;

  mh->f();

  return 0;
}
