#include <stdio.h>

extern int __attribute__ ((weak)) fun (void);

int
main (void)
{
  if (&fun != 0)
    fun ();
  else
    printf ("Weak undefined\n");
  return 0;
}
