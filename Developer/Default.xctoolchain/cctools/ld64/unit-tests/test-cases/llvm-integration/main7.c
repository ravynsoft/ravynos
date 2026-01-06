extern int foo1(void);

int main(void)
{
  int i = foo1();
  if (i == 42)
    return 0;
  else
    return 1;
}
