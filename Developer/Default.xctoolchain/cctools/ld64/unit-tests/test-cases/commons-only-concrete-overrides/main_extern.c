#include <stdio.h>

extern int foo;
extern int bar;

int main() {
  printf("foo: %d\n", foo);
  printf("bar: %d\n", bar);
  return 0;
}
