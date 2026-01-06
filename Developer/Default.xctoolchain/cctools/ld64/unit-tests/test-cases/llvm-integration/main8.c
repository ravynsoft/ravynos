extern  int foo1(void);
extern  void foo2(void);

int main()
{
  int i = foo1();
  if (i == 42)
    return 0;
  else
    return 1;
}
