#include <stdio.h>

char *myweakfunc(void) __attribute__((weak)) ;

int main(int argc, char **argv)
{
  if (myweakfunc)
    printf ("found myweakfunc %s\n", myweakfunc());
  else
    printf("Weak func not found\n");
  return 0;
}

