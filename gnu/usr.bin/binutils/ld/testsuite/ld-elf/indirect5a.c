extern void foo (long *);
long bar;

int
main (void)
{
  foo (&bar);
  return 0;
}
