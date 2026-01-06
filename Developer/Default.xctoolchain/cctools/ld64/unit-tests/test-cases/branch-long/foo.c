
int x = 1;
int y = 2;

__attribute__((weak))
void myweak1()
{
}

int foo()
{
  myweak1();
  return 1;
}

int foo1()
{
  return x;
}

int foo2()
{
  return y;
}

