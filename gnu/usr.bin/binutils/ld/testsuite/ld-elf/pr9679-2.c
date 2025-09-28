extern int foo (void) __attribute__((weak,__visibility__ ("hidden")));

int
foo (void)
{
  return 1;
}

int
bar (void)
{
  return foo ();
}
