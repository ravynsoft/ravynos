#include <stdio.h>
#include "foo.h"

static void foo1() {}

void foo_test() {
  printf("%p\n", &foo1);
  printf("%p\n", &Foo::doit);
  printf("%p\n", &Foo::doit2);
}

