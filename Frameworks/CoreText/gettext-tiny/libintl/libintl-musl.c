#include <stdio.h>

/* trick configure tests checking for gnu libintl, as in the copy included in gdb */
const char *_nl_expand_alias () { return NULL; }
int _nl_msg_cat_cntr = 0;
