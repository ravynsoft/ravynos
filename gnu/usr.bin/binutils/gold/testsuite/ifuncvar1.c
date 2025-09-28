/* Test global variable initialized to hidden STT_GNU_IFUNC symbol.  */

int didit;

extern void doit (void);

void
doit (void)
{
  didit = 1;
}

void (*get_foo (void)) (void) __asm__ ("foo");
__asm__ (".type foo, %gnu_indirect_function");
__asm__ (".hidden foo");

void (*get_foo (void)) (void)
{
  return &doit;
}
