int foo[2];
int bar[2] = { -1, -1 };

int *
foo_p (void)
{
  return foo;
}

int *
bar_p (void)
{
  return bar;
}
