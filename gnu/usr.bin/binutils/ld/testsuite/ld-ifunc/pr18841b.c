void foo() __attribute__((ifunc("resolve_foo")));

static void foo_impl() {}

extern void abort (void);
void test()
{
  void (*pg)(void) = foo;
  if (pg != foo_impl)
    abort ();
  pg();
}

static void* resolve_foo()
{
  extern void zoo(void);

  void (*pz)(void) = zoo;
  pz();
  return foo_impl;
}
