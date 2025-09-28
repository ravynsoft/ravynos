void test(void);

extern int protected;

int *
get_protected_ptr (void)
{
  return &protected;
}

int main()
{
  test();
  return 0;
}
