__attribute__((section(".data.a")))
static int int_from_a_1 = 0x11223344;

__attribute__((section(".data.rel.ro.a")))
int *p_int_from_a_2 = &int_from_a_1;

const char *hello (void);

const char *
hello (void)
{
  return "XXXHello, world!" + 3;
}
