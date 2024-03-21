int foo __attribute__ ((section ("_data_foo"))) = 0;
extern int foo_alias __attribute__ ((weak, alias ("foo")));
extern char __start__data_foo;
__asm__ (".type __start__data_foo,%object");
int x1 = 1;
int x2 = 2;

char *
bar ()
{
  foo = -1;
  return &__start__data_foo;
}
