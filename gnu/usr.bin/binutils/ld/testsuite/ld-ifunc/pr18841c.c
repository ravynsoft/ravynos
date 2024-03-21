void foo() __attribute__((ifunc("resolve_foo")));

static void foo_impl() {}
extern void zoo(void);
void (*pz)(void) = zoo;

void test()
{
  void (*pg)(void) = foo;
  pg();
}

static void* resolve_foo()
{
  pz();
  return foo_impl;
}
