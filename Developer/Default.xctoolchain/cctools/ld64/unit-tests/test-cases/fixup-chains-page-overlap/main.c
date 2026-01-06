#include <stdio.h>
#include <assert.h>

extern void* arr[2];

int main() {
  printf("main: %p, arr[0]: %p, arr[1]: %p\n", &main, arr[0], arr[1]);
  assert(&main == arr[1]);
  return 0;
}
