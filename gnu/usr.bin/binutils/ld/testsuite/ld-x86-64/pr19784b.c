int foo (int x) __attribute__ ((ifunc ("resolve_foo")));

static int foo_impl(int x)
{
  return x;
}

void *resolve_foo (void)
{
  return (void *) foo_impl;
}
