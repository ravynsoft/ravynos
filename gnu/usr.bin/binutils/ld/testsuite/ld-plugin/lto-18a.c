#include <stdio.h>

extern int select ();
extern int f1 (int);
extern int f2 (int);

int main (void)
{
  switch (select ())
    {
    case 1:
      printf ("%d\n", f1 (3));
      break;
    case 2:
      printf ("%d\n", f2 (4));
      break;
    default:
      printf ("%d\n", f2 (0));
      break;
    }
  return 0;
}
