extern int foo (void) __attribute__((section (".gnu.linkonce.t.1"), weak));

int
foo (void)
{
  return 1;
}
