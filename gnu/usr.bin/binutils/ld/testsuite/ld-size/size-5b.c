extern __thread char bar[];
extern char size_of_bar __asm__ ("bar@SIZE");

char *bar_size_1 = &size_of_bar;
static char *bar_size_2 = &size_of_bar;

char *
bar_size1 (void)
{
  bar[2] = 3;
  return bar_size_1;
}

char *
bar_size2 (void)
{
  return bar_size_2;
}

extern __thread char foo[];
extern char size_of_foo __asm__ ("foo@SIZE");

char *foo_size_1 = &size_of_foo;
static char *foo_size_2 = &size_of_foo;

char *
foo_size1 (void)
{
  foo[3] = 4;
  return foo_size_1;
}

char *
foo_size2 (void)
{
  return foo_size_2;
}
