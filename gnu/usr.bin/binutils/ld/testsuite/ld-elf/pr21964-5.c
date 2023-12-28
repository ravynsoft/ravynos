#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

extern int foo (void);

extern int __start___verbose[];
extern int __stop___verbose[];
int bar (void)
{
  static int my_var __attribute__((section("__verbose"), used)) = 6;
  int *ptr;
  ptr = (int*) dlsym(RTLD_DEFAULT, "__start___verbose");
  if (!ptr || *ptr != 6)
    return -1;
  return 0;
}

int main()
{
  if (bar () == 0)
    printf ("PASS\n");

  return 0;
}
