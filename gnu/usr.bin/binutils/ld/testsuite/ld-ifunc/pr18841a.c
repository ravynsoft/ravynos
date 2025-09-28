#include <stdio.h>

extern void test(void);

void zoo(){}

int main()
{
  test();
  printf("OK\n");
  return 0;
}
