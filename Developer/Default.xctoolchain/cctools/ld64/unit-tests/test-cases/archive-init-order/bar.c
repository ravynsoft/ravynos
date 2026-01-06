int bar() { return 0; }

__attribute__((constructor))
void bar_init() { }
