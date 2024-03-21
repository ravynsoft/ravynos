int foo (int) __attribute__ ((ifunc ("resolve_foo")));
extern void abort (void);

static int
foo_impl (int x)
{
  return x;
}

void
bar (void)
{
  int (*f)(int) = foo;

  if (foo (5) != 5)
    abort ();

  if (f (42) != 42)
    abort ();
}

void *
resolve_foo (void)
{
  return (void *) foo_impl;
}
