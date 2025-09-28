int foo (int (*bar)(void));

int foo (int (*bar)(void))
{
  return bar() == 0x55 ? 0 : 1;
}
