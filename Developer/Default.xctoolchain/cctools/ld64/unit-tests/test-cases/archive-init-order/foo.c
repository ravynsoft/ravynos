int foo() { return 1; }

__attribute__((constructor))
void foo_init() { }
