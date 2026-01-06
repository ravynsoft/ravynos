

void foo1() {}
void foo3() {}


__attribute__((weak)) void foo2() {}
__attribute__((weak)) void foo4() {}


void wildcheck() {}
void willnot() {}



__attribute__((weak)) void patterncheck() {}
__attribute__((weak)) void patnot() {}
