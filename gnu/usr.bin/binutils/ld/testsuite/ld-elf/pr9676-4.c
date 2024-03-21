extern int bar (void);
extern int foo (void);

int
x (void)
{
  foo ();
  return bar ();
}
