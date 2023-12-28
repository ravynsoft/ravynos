int
foo (void)
{
  return 0;
}

int
main ()
{
  return foo ();
}

extern int __real_foo (void);

int
__wrap_foo (void)
{
  return __real_foo ();
}
