#include <stdio.h>

extern int (*func_p) (void);
extern int func (void);
extern void foo (void);


void
bar (void)
{
  if (func_p != &func || func_p () != 0xbadbeef)
    __builtin_abort ();
}

int
main ()
{
  func_p = &func;
  foo ();
  bar ();
  printf ("PASS\n");
  return 0;
}
