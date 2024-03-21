static int
ifunc (void)
{
  return 0xbadbeef;
}

void func(void) __attribute__((ifunc("resolve_func")));

static void *
resolve_func (void)
{
  return ifunc;
}
