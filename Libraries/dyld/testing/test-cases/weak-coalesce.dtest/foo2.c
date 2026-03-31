#include <stdio.h>

#include "test_support.h"

#include "base.h"

int							coal1 = 2;  // note: this is not weak and therefore should win
int __attribute__((weak))	coal2 = 2;

static __attribute__((constructor))
void myinit(int argc, const char* argv[], const char* envp[], const char* apple[])
{
    LOG("myinit() in foo1.c");
    baseVerifyCoal1("in foo2", &coal1);
    baseVerifyCoal2("in foo2", &coal2);
}
