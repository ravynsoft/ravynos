#include <stdio.h>

void foo (void) __attribute__ ((noinline, visibility ("hidden")));

void foo (void) {
  printf("foo called.\n");
}

void bar(void);

int main(void) {
  foo();
  bar();
  return 0;
}
