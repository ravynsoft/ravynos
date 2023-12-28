#include <stdio.h>

extern void indirect_extern_access (void);
extern void *indirect_extern_access_p (void);

int
main (void)
{
  if (&indirect_extern_access == indirect_extern_access_p ())
    puts ("PASS");

  return 0;
}
