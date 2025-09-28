int foo = 0;
extern int foo_alias __attribute__ ((weak, alias ("foo")));

void
bar (void)
{
  foo = -1;
}

