extern void foo (long *);
long bar = 1;

int
main (void)
{
  foo (&bar);
  return 0;
}
