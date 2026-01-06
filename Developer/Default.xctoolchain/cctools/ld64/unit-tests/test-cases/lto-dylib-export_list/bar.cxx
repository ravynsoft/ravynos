#include <stdio.h>
#include "foo.h"

static void bar1() {}

void bar_test() {
  printf("%p\n", &bar1);
  printf("%p\n", &Foo::doit);
  printf("%p\n", &Foo::doit2);
}

