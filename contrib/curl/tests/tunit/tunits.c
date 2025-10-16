/* !checksrc! disable COPYRIGHT all */

#include "first.h"

#include "tool1394.c"
#include "tool1604.c"
#include "tool1621.c"

const struct entry_s s_entries[] = {
  {"tool1394", test_tool1394},
  {"tool1604", test_tool1604},
  {"tool1621", test_tool1621},
  {NULL, NULL}
};

#include "first.c"
