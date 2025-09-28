int foo() { return 42; }

int bar() { return 0; }

#pragma GCC visibility push(hidden)
int baz() { return 1; }
#pragma GCC visibility pop
