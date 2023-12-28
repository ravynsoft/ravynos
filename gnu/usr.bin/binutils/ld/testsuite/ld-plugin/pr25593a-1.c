extern void foo (void);
extern void bar (void);
extern void xxx (void);

int
main (void)
{
  xxx ();
  foo ();
  bar ();
  return 0;
}
