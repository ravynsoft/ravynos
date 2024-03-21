#include "lto-19.h"

struct re_dfa_t *
xregcomp (void)
{
  return rpl_regcomp ();
}
