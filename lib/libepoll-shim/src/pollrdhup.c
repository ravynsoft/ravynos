#define _GNU_SOURCE
#include <stdio.h>
#include <poll.h>
int main() {
  printf("0x%x", POLLRDHUP);
  return 0;
}
