extern int foo;

static void
__attribute__ ((unused, constructor))
set_foo (void)
{
  foo = 30;
}
