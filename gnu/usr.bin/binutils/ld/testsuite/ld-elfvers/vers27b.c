#include "vers.h"
void foo () {}
__asm__ (".hidden " SYMPFX(foo));
