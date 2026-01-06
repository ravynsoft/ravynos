
extern int foo1(void);

int foo3(void)
{
  return 42;
}

int main()
{
  int i = foo1();
  if (i == 42)
    return 0;
  else
    return 1;
}
