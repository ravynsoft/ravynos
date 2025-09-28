extern int foo;
extern int foo2;

static void
__attribute__ ((unused, constructor))
set_foo (void)
{
  foo = foo2;
}
