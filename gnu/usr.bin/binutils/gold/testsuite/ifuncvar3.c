/* Test global variable initialized to hidden STT_GNU_IFUNC symbol.  */

#include <assert.h>

extern void bar (void);
extern int didit;

int
main (void)
{
  bar ();
  assert (didit == 1);
  return 0;
}
