extern void bar () __attribute__((weak));

int
main (void)
{
  if (bar)
    bar ();
  return 0;
}
