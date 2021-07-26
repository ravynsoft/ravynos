#include <stdio.h>

int foo() {
	__block id x;
    void (^hello)(void) = ^(void) {
        printf("hello is running, %p\n", x);
    };
    printf("1\n");
    hello();
    printf("2\n");
    hello = 0;    // Here ARC is releasing the block, that's why we don't see '3' printed.
    printf("3\n");
    return 0;
}
int main() {
  return foo();
}
