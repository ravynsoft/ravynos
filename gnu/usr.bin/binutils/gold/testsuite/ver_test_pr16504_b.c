void
new_foo(void);
__asm__(".symver new_foo,foo@VER2");

static void (*resolve_foo(void)) (void)
{
  return new_foo;
}

void foo(void) __attribute__((ifunc("resolve_foo")));
