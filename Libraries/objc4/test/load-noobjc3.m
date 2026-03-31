#include "test.h"

extern int state;

__attribute__((constructor))
static void ctor(void)
{
    state = 1;
}
