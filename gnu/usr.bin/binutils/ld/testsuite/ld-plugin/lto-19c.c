#include "lto-19.h"

int
main ()
{
  struct re_dfa_t *dfa = xregcomp ();
  rpl_regfree (dfa);
  return 0;
}
