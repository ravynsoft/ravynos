extern int foo_p;

void *
get_foo (void)
{
  return (void *) ((long) &foo_p + foo_p);
}
