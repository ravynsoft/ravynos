extern int foo (void) __attribute__((section (".gnu.linkonce.t.1"), weak,
				     __visibility__ ("hidden")));

int
foo (void)
{
  return 1;
}

int
foo2 (void)
{
  return 1;
}
