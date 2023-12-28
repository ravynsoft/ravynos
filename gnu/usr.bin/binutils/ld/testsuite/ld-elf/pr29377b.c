extern void foo (void);

void (*foo_p) (void);

int
main ()
{
  foo_p = foo;
  return 0;
}
