#include <stdio.h>

int Y;
extern int X __attribute__((visibility("hidden")));
void foo() {
  printf ("%d\n", X); 
}
