#include <stdio.h>
#include <stdlib.h>
#include "lto-19.h"

static const int utf8_sb_map[4] = { 0x12, 0x34, 0x56, 0x78 };

struct re_dfa_t *
rpl_regcomp ()
{
  struct re_dfa_t *dfa = malloc (sizeof (struct re_dfa_t));
  dfa->sb_char = utf8_sb_map;
  return dfa;
}

void
rpl_regfree (struct re_dfa_t *dfa)
{
  puts (dfa->sb_char == utf8_sb_map ? "PASS" : "FAIL");
}
