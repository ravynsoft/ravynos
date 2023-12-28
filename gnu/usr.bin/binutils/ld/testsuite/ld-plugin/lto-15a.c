#include <stdio.h>

int main(int argc, char **argv)
{
  printf ("PASS\n");
  return (int) ((unsigned long long) argc / argv[0][0]);
}
