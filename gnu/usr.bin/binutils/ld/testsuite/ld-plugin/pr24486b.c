extern void bar (void) __attribute__((weak));

void
foo (void)
{
  if (bar)
    bar ();
}
