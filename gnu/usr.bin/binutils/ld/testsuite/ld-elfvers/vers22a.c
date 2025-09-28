#include "vers.h"

FUNC_SYMVER(_old_bar, bar@VERS.0);

void
_old_bar () 
{
}
