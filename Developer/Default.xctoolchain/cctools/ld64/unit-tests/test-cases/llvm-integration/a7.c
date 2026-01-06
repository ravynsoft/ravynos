extern int foo3(void);

int foo1(void)
{
  return foo3();
}

int foo2(void)
{
  return 42;
}
