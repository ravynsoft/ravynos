static int
one (void)
{
  return 1;
}

static int
minus_one (void)
{
  return -1;
}

void * foo_ifunc (void) __asm__ ("foo");
__asm__(".type foo, %gnu_indirect_function");

void *
foo_ifunc (void)
{
  return one;
}

void * bar_ifunc (void) __asm__ ("bar");
__asm__(".type bar, %gnu_indirect_function");

void *
bar_ifunc (void)
{
  return minus_one;
}
