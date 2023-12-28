void alt (void) { }

void foo (void);
void * foo_ifunc (void) __asm__ ("foo");
__asm__(".type foo, %gnu_indirect_function");
__asm__(".weak foo");

void *
foo_ifunc (void)
{
  return alt;
}
